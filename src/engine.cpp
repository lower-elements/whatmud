#include <stdexcept>

#include "spdlog/spdlog.h"
#include <fmt/core.h>

#include "connection.hpp"
#include "engine.hpp"
#include "lua/error.hpp"
#include "uv/error.hpp"

namespace whatmud {

Engine::Engine(const char *dir_name)
    : L(), m_loop(), m_server_socket(m_loop.asLoop(), AF_INET) {
  // Load game code
  std::string init_script(fmt::format("{}/init.lua", dir_name));
  spdlog::info("Loading game init script from {}", init_script);
  int res = luaL_loadfilex(L, init_script.c_str(), "t");
  if (res != LUA_OK) {
    throw lua::Error(L, "Could not load game init script");
  }
  lua_call(L, 0, 0);

  // Bind server socket
  struct sockaddr_in addr;
  res = uv_ip4_addr("0.0.0.0", 9009, &addr);
  uv::check_error(res, "Could not create address for server socket");
  m_server_socket.bind((const struct sockaddr *)&addr);

  // Listen for new connections, and create Connection objects
  m_server_socket.listen(32, [](uv_stream_t *server, int status) {
    uv::check_error(status, "Could not listen for incoming connections");
    spdlog::info("New connection!");
    Connection *conn = new Connection(server->loop);
    conn->accept(server);
  });
}

Engine::~Engine() {}

void Engine::run() { m_loop.run(); }

} // namespace whatmud
