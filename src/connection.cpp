#include <stdexcept>

#include <spdlog/sinks/stdout_color_sinks.h>

#include "connection.hpp"
#include "uv/error.hpp"

// Telnet charset negotiation, not yet in libtelnet
#ifndef TELNET_TELOPT_CHARSET
#define TELNET_TELOPT_CHARSET 42
#define TELNET_CHARSET_REQUEST 1
#define TELNET_CHARSET_ACCEPTED 2
#define TELNET_CHARSET_REJECTED 3
#endif

namespace whatmud {

// Forward declarations:
void forwardEvent(telnet_t *telnet, telnet_event_t *event, void *user_data);
void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
static void allocBuffer(uv_handle_t *handle, std::size_t suggested_size,
                        uv_buf_t *buf);

// Logger for all connection objects
std::shared_ptr<spdlog::logger> Connection::m_log =
    spdlog::stderr_color_mt("connection");

// Telnet options we support, terminated by -1
static const telnet_telopt_t TELNET_OPTS[]{
    {TELNET_TELOPT_SGA, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_CHARSET, TELNET_WILL, TELNET_DONT},
    {-1, 0, 0}};

Connection::Connection(uv_loop_t *loop)
    : uv::TCP(loop), m_recv_buf(std::ios::in | std::ios::out), m_msg_proc(loop),
      m_telnet(telnet_init(TELNET_OPTS, forwardEvent, 0, this)), m_features() {
  if (m_telnet == nullptr) {
    throw std::runtime_error("Could not create telnet state tracker");
  }

  // Makes it easy for event callbacks to reference this Connection object
  setData(this);

  // Tell the receive buffer to throw exceptions for hard IO errors, but not EOF
  // or fail EOF will occur frequently, and is handled directly by the message
  // processor
  m_recv_buf.exceptions(std::stringstream::badbit);
  // Give the message processor a reference to this object
  m_msg_proc.setData(this);
}

void Connection::accept(uv_stream_t *server_sock) {
  int res = uv_accept(server_sock, asStream());
  uv::check_error(res, "Could not accept client connection");
  readStart(allocBuffer, onRead);

  // Initial negotiation
  for (const telnet_telopt_t *opt = TELNET_OPTS; opt->telopt != -1; ++opt) {
    if (opt->us == TELNET_WILL) {
      telnet_negotiate(m_telnet, TELNET_WILL, opt->telopt);
    }
    if (opt->him == TELNET_DO) {
      telnet_negotiate(m_telnet, TELNET_DO, opt->telopt);
    }
  }
}

Connection::~Connection() {
  if (m_telnet) {
    telnet_free(m_telnet);
  }
}

void Connection::onEvent(telnet_event_t &ev) {
  switch (ev.type) {
  case TELNET_EV_SEND:
    // Queue data to be sent
    onSend(ev.data.buffer, ev.data.size);
    break;
  case TELNET_EV_DATA:
    // Buffer data from client
    onRecv(ev.data.buffer, ev.data.size);
    break;

  case TELNET_EV_WARNING:
    m_log->warn("{}:{} - {} - {}", ev.error.file, ev.error.line, ev.error.func,
                ev.error.msg);
    break;
  case TELNET_EV_ERROR:
    m_log->error("{}:{} - {} - {}", ev.error.file, ev.error.line, ev.error.func,
                 ev.error.msg);
    onEof();
    break;

  case TELNET_EV_WILL:
    onClientWill(ev.neg.telopt);
    break;
  case TELNET_EV_WONT:
    onClientWont(ev.neg.telopt);
    break;
  case TELNET_EV_DO:
    onClientDo(ev.neg.telopt);
    break;
  case TELNET_EV_DONT:
    onClientDont(ev.neg.telopt);
    break;

  case TELNET_EV_SUBNEGOTIATION:
    onClientSubNegotiate(ev.sub.telopt,
                         std::string_view(ev.sub.buffer, ev.sub.size));
    break;

  default: // Unknown event
    m_log->debug("Ignoring telnet event with type {}", ev.type);
    break;
  }
}

void Connection::onEof() {
  m_log->info("Closing connection");
  close([](uv_handle_t *handle) {
    // Destroy the Connection object
    Connection *conn = reinterpret_cast<Connection *>(handle->data);
    delete conn;
  });
}

void Connection::onSend(const char *buf, std::size_t size) {
  // Copy the data into a libuv buffer
  uv_buf_t writebuf = uv_buf_init(new char[size], size);
  std::memcpy(writebuf.base, buf, size);

  // Start a write request
  // `req` is deleted by the write callback, as is the buffer memory
  uv_write_t *req = new uv_write_t;
  req->data = writebuf.base;
  write(req, &writebuf, [](uv_write_t *req, int status) {
    char *writebuf = reinterpret_cast<char *>(req->data);
    // Free the no longer needed buffer memory
    delete[] writebuf;
    // Destroy the request
    delete req;
    // Throw any errors
    uv::check_error(status, "Write error");
  });
}

void Connection::onRecv(const char *buf, std::size_t size) {
  // Buffer the received data so it can be read line by line
  m_recv_buf.write(buf, size);
  // We likely have new messages to process
  m_msg_proc.start([](uv_check_t *handle) {
    Connection *conn = reinterpret_cast<Connection *>(handle->data);

    // Process messages line by line
    std::string msg;
    while (!std::getline(conn->m_recv_buf, msg).eof()) {
      conn->onMessage(msg);
    }

    conn->m_recv_buf.clear(); // Clear EOF flag
    uv_check_stop(handle);    // Will be started again on new data
  });
}

void Connection::onMessage(const std::string &msg) {
  m_log->info("Got message: {}", msg);
  send(msg + '\n'); // Echo the message
}

void Connection::onClientWill(unsigned char telopt) {
  m_log->debug("Client will use telnet option {}", telopt);
}

void Connection::onClientWont(unsigned char telopt) {
  m_log->debug("Client won't use telnet option {}", telopt);
}

void Connection::onClientDo(unsigned char telopt) {
  m_log->debug("Client wants telnet option {}", telopt);
  switch (telopt) {
  case TELNET_TELOPT_CHARSET:
    // Request to use UTF-8
    telnet_begin_sb(m_telnet, TELNET_TELOPT_CHARSET);
    telnet_printf(m_telnet, "%c UTF-8", TELNET_CHARSET_REQUEST);
    telnet_finish_sb(m_telnet);
    break;
  }
}

void Connection::onClientDont(unsigned char telopt) {
  m_log->debug("Client doesn't want telnet option {}", telopt);
}

void Connection::onClientSubNegotiate(unsigned char telopt,
                                      std::string_view data) {
  if (telopt != TELNET_TELOPT_CHARSET) {
    return; // Ignore unknown SB
  }
  char status = data.at(0);
  if (status == TELNET_CHARSET_ACCEPTED) {
    m_features.utf8 = true;
  } else if (status == TELNET_CHARSET_REJECTED) {
    m_log->info("Client doesn't accept UTF-8, falling back to ASCII");
    m_features.utf8 = false;
  } else {
    m_log->warn("Unknown charset negotiation data: {}", data);
  }
}

void forwardEvent(telnet_t *telnet, telnet_event_t *event, void *user_data) {
  (void)telnet;
  Connection *conn = reinterpret_cast<Connection *>(user_data);
  conn->onEvent(*event);
}

void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
  Connection *conn = reinterpret_cast<Connection *>(handle->data);

  //  Check status
  if (nread == 0) {
    return; // EAGAIN
  } else if (nread == UV_EOF) {
    conn->readStop();
    conn->onEof();
  } else if (nread < 0) {
    throw uv::Error((int)nread, "Read error");
  } else {
    // Process input with libtelnet
    telnet_recv(conn->getTelnet(), buf->base, nread);

    // Free the buffer
    if (buf->base != nullptr) {
      delete[] buf->base;
    }
  }
}

static void allocBuffer(uv_handle_t *handle, std::size_t suggested_size,
                        uv_buf_t *buf) {
  (void)handle;
  // Todo: Free list for buffers?
  *buf = uv_buf_init(new char[suggested_size], suggested_size);
}

} // namespace whatmud
