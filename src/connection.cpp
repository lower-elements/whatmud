#include <stdexcept>

#include <spdlog/spdlog.h>

#include "connection.hpp"
#include "uv/error.hpp"

namespace whatmud {

// Forward declarations:
void forwardEvent(telnet_t *telnet, telnet_event_t *event, void *user_data);
void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
static void allocBuffer(uv_handle_t *handle, std::size_t suggested_size,
                        uv_buf_t *buf);

static const telnet_telopt_t TELNET_OPTS[]{{-1, 0, 0}};

Connection::Connection(uv_loop_t *loop)
    : uv::TCP(loop), m_recv_buf(std::ios::in | std::ios::out), m_msg_proc(loop),
      m_telnet(telnet_init(TELNET_OPTS, forwardEvent, 0, this)) {
  if (m_telnet == nullptr) {
    throw std::runtime_error("Could not create telnet state tracker");
  }
  setData(this);
  m_recv_buf.exceptions(std::stringstream::badbit);
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
    onSend(ev.data.buffer, ev.data.size);
    break;
  case TELNET_EV_DATA:
    onRecv(ev.data.buffer, ev.data.size);
    break;
  default:
    spdlog::debug("Ignoring telnet event with type {}", ev.type);
    break;
  }
}

void Connection::onEof() {
  spdlog::info("Connection got EOF, closing");
  close([](uv_handle_t *handle) {
    Connection *conn = reinterpret_cast<Connection *>(handle->data);
    delete conn;
  });
}

void Connection::onSend(const char *buf, std::size_t size) {
  // Copy the data into a libuv buffer
  uv_buf_t writebuf;
  writebuf = uv_buf_init(new char[size], size);
  std::memcpy(writebuf.base, buf, size);

  // Start a write request
  // `req` is deleted by the write callback, as is the buffer memory
  uv_write_t *req = new uv_write_t;
  req->data = writebuf.base;
  write(req, &writebuf, [](uv_write_t *req, int status) {
    char *writebuf = reinterpret_cast<char *>(req->data);
    delete[] writebuf;
    delete req;
    uv::check_error(status, "Write error");
  });
}

void Connection::onRecv(const char *buf, std::size_t size) {
  m_recv_buf.write(buf, size);
  m_msg_proc.start([](uv_check_t *handle) {
    Connection *conn = reinterpret_cast<Connection *>(handle->data);
    std::string msg;
    while (!std::getline(conn->m_recv_buf, msg).eof()) {
      conn->onMessage(msg);
    }
    conn->m_recv_buf.clear();
    uv_check_stop(handle);
  });
}

void Connection::onMessage(const std::string &msg) {
  spdlog::info("Got message: {}", msg);
  send(msg + '\n');
}

void forwardEvent(telnet_t *telnet, telnet_event_t *event, void *user_data) {
  (void)telnet;
  Connection *conn = reinterpret_cast<Connection *>(user_data);
  conn->onEvent(*event);
}

void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
  Connection *conn = reinterpret_cast<Connection *>(handle->data);

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
  *buf = uv_buf_init(new char[suggested_size], suggested_size);
}

} // namespace whatmud
