#include <stdexcept>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "connection.hpp"
#include "engine.hpp"
#include "listener.hpp"
#include "uv/error.hpp"

namespace whatmud {

Listener::Listener(Engine *engine, const char *ip, int port)
    : m_engine(engine) {
  // Parse and bind address
  int res = uv_ip4_addr(ip, port, (struct sockaddr_in *)&m_listen_addr);
  if (res < 0) {
    // Try ipv6
    res = uv_ip6_addr(ip, port, (struct sockaddr_in6 *)&m_listen_addr);
  }
  uv::check_error(res, fmt::format("Could not parse ip address: {}", ip));
}

std::string Listener::getListenIP() const {
  std::string ip;
  ip.resize(64);
  int res = uv_ip_name(getListenAddr(), ip.data(), ip.size());
  uv::check_error(res, "Could not convert listener IP address to string");
  ip.erase(std::find(ip.begin(), ip.end(), '\0'), ip.end());
  return ip;
}

int Listener::getListenPort() const {
  switch (m_listen_addr.ss_family) {
  case AF_INET: // IPv4
  {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&m_listen_addr;
    return ntohs(ipv4->sin_port);
  }
  case AF_INET6: // IPv6
  {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&m_listen_addr;
    return ntohs(ipv6->sin6_port);
  }
  default:
    throw std::runtime_error(fmt::format("Unknown listener address family: {}",
                                         m_listen_addr.ss_family));
  }
}

TcpListener::TcpListener(Engine *engine, const char *ip, int port)
    : Listener(engine, ip, port), TCP(engine->getLoop()) {
  setData(this);

  bind(getListenAddr());
}

void TcpListener::listen() {
  TCP::listen(32, [](uv_stream_t *handle, int status) {
    TcpListener *listener = reinterpret_cast<TcpListener *>(handle->data);

    // Check for errors
    if (status < 0) {
      throw uv::Error(status,
                      fmt::format("Could not listen for connections on {}:{}",
                                  listener->getListenIP(),
                                  listener->getListenPort()));
    }

    spdlog::info("New connection!");

    Connection *conn = new Connection(handle->loop);
    conn->accept(handle);
    // Conn deletes itself, for now
  });
}

} // namespace whatmud
