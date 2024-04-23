#include <stdexcept>

#include "connection.hpp"
#include "uv/error.hpp"

namespace whatmud {

static void allocBuffer(uv_handle_t *handle, std::size_t suggested_size,
                        uv_buf_t *buf) {
  (void)handle;
  buf->base = new char[suggested_size];
  buf->len = suggested_size;
}

static void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
  if (nread == 0) {
    return;
  } else if (nread < 0) {
    throw uv::Error((int)nread, "Read error");
  }
  Connection *conn = reinterpret_cast<Connection *>(handle->data);
  telnet_recv(conn->getTelnet(), buf->base, nread);
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

void Connection::onEvent(telnet_event_t &ev) { (void)ev; }

} // namespace whatmud
