#include <cassert>
#include <cstring>
#include <stdexcept>

#include <fmt/core.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "connection.hpp"
#include "engine.hpp"
#include "listener.hpp"
#include "lua/helpers.hpp"
#include "uv/error.hpp"

namespace whatmud {

Listener::Listener(Engine *engine, const char *ip, int port)
    : m_log(spdlog::stderr_color_st(fmt::format("listener@{}:{}", ip, port))),
      m_engine(engine) {
  // Parse address
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
  m_log->info("Listening on {}:{}", getListenIP(), getListenPort());

  TCP::listen(32, [](uv_stream_t *handle, int status) {
    TcpListener *listener = reinterpret_cast<TcpListener *>(handle->data);

    // Check for errors
    if (status < 0) {
      throw uv::Error(status,
                      fmt::format("Could not listen for connections on {}:{}",
                                  listener->getListenIP(),
                                  listener->getListenPort()));
    }

    listener->onNewConnection();
  });
}

void TcpListener::onNewConnection() {
  m_log->info("New connection!");

  // Get the Lua state
  lua_State *L = m_engine->getLuaState();

  // Get the connections table
  lua_pushliteral(L, "connections");
  lua_rawget(L, LUA_REGISTRYINDEX);

  // Create a new Connection object
  Connection *conn = lua::new_userdata_uv<Connection>(L, 1, m_engine);
  conn->accept(asStream());

  // Create the coroutine on which the client handler runs
  lua_State *co = lua_newthread(L);

  // Set the coroutine's extra-space to point to the Connection object for
  // convenience
  auto **extraspace = reinterpret_cast<Connection **>(lua_getextraspace(co));
  *extraspace = conn;

  // Set the coroutine as the Connection's first uservalue
  lua_pushvalue(L, -1);
  lua_setiuservalue(L, -3, 1);

  // Create the Connection's _ENV table
  // makeEnvironment() expects the Connection object to be ontop of the stack
  lua_pushvalue(L, -2);
  lua_xmove(L, co, 1);
  // 1 = connection
  Connection::makeEnvironment(co);
  // 1 = _ENV

  // Run the client handler on the coroutine
  lua_pushliteral(co, "client_handler");
  lua_rawget(co, LUA_REGISTRYINDEX);
  lua_insert(co, 1);
  //  1 = handler function, 2 = _ENV
  assert(lua_isfunction(co, 1));
  // Set the environment
  const char *upval_name = lua_setupvalue(co, 1, 1);
  assert(std::strcmp(upval_name, "_ENV") == 0);
  // 1 = handler function
  // Start the handler
  int nresults;
  lua_resume(co, L, 0, &nresults);

  lua_pop(L, 1); // Pop coroutine

  // Add the Connection to the connections table to keep it alive
  lua_pushlightuserdata(L, conn);
  lua_insert(L, -2); // Swap the key and value
  lua_rawset(L, -3);

  // Pop connections table
  lua_pop(L, 1);
}

} // namespace whatmud
