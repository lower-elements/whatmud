#include <stdexcept>

#include <spdlog/spdlog.h>

#include "connection.hpp"
#include "uv/error.hpp"

namespace whatmud {

static void allocBuffer(uv_handle_t *handle, std::size_t suggested_size,
                        uv_buf_t *buf) {
  (void)handle;
  buf->base = new char[suggested_size];
  buf->len = suggested_size;
}

void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
  Connection *conn = reinterpret_cast<Connection *>(handle->data);

  if (nread == 0) {
    return; // EAGAIN
  } else if (nread == UV_EOF) {
    conn->onEof();
  } else if (nread < 0) {
    throw uv::Error((int)nread, "Read error");
  }

  // Process input with libtelnet
  telnet_recv(conn->getTelnet(), buf->base, nread);

  // Free the buffer
  if (buf->base != nullptr) {
    delete[] buf->base;
  }
}

static const telnet_telopt_t TELNET_OPTS[]{{-1, 0, 0}};

void forwardEvent(telnet_t *telnet, telnet_event_t *event, void *user_data) {
  (void)telnet;
  Connection *conn = reinterpret_cast<Connection *>(user_data);
  conn->onEvent(*event);
}

Connection::Connection()
    : uv::TCP(), m_telnet(telnet_init(TELNET_OPTS, forwardEvent, 0, this)) {
  if (m_telnet == nullptr) {
    throw std::runtime_error("Could not create telnet state tracker");
  }
  setData(this);
  readStart(allocBuffer, onRead);
}

void Connection::accept(uv::TCP &server_sock) {
  server_sock.accept(asStream());
}

Connection::~Connection() {
  if (m_telnet) {
    telnet_free(m_telnet);
  }
}

void Connection::onEvent(telnet_event_t &ev) {
  switch (ev.type) {
  case TELNET_EV_SEND:
    sendData(ev.data.buffer, ev.data.size);
    break;
  default:
    spdlog::debug("Ignoring telnet event with type {}", ev.type);
    break;
  }
}

void Connection::onEof() {
  spdlog::info("Connection got EOF, closing");
  close();
}

void Connection::sendData(const char *buf, std::size_t size) {
  // Copy the data into a libuv buffer
  uv_buf_t writebuf;
  writebuf.base = new char[size];
  std::memcpy(writebuf.base, buf, size);
  writebuf.len = size;

  // Start a write request
  // `req` is deleted by the write callback, as is the buffer memory
  uv_write_t *req = new uv_write_t;
  req->data = writebuf.base;
  write(req, &writebuf, [](uv_write_t *req, int status) {
    char *writebuf = (char *)req->data;
    delete[] writebuf;
    delete req;
    uv::check_error(status, "Write error");
  });
}

} // namespace whatmud
