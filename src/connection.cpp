#include <stdexcept>

#include "connection.hpp"

namespace whatmud {

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
