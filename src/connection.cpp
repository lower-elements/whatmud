#include <stdexcept>

#include <spdlog/sinks/stdout_color_sinks.h>

#include "connection.hpp"
#include "uv/error.hpp"

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
// Currently we haven't implemented any
static const telnet_telopt_t TELNET_OPTS[]{
    {TELNET_TELOPT_BINARY, TELNET_WILL, TELNET_DO},
    {TELNET_TELOPT_SGA, TELNET_WILL, TELNET_DO},
    {-1, 0, 0}};

Connection::Connection(uv_loop_t *loop)
    : uv::TCP(loop), m_recv_buf(std::ios::in | std::ios::out), m_msg_proc(loop),
      m_telnet(telnet_init(TELNET_OPTS, forwardEvent, 0, this)) {
  if (m_telnet == nullptr) {
    throw std::runtime_error("Could not create telnet state tracker");
  }

  // Makes it easy for event callbacks to reference this COnnection object
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
    m_log->debug("Client will use option {}", ev.neg.telopt);
    break;
  case TELNET_EV_DO:
    m_log->debug("Client wants option {}", ev.neg.telopt);
    break;
  case TELNET_EV_WONT:
    m_log->debug("Client won't use option {}", ev.neg.telopt);
    break;
  case TELNET_EV_DONT:
    m_log->debug("Client doesn't want option {}", ev.neg.telopt);
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
