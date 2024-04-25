#include <stdexcept>

#include "spdlog/spdlog.h"
#include <fmt/core.h>
#include <lua.hpp>

#include "connection.hpp"
#include "engine.hpp"
#include "lua/helpers.hpp"
#include "uv/error.hpp"

namespace whatmud {

// Forward declarations:
int l_listen(lua_State *L);

Engine::Engine(const char *dir_name) : L(), m_loop(), m_listeners() {
  // // Register Lua configuration functions
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_listen, 1);
  lua_setglobal(L, "listen");

  // Load game code
  std::string init_script(fmt::format("{}/init.lua", dir_name));
  spdlog::info("Loading game init script from {}", init_script);
  int res = luaL_loadfilex(L, init_script.c_str(), "t");
  if (res != LUA_OK) {
    throw lua::Error(L, "Could not load game init script");
  }
  lua_call(L, 0, 0);

  // Warn the user if no listeners were created
  if (m_listeners.empty()) {
    spdlog::warn("No listeners created, did you forget to call listen() in {}?",
                 init_script);
  }
}

Engine::~Engine() {}

void Engine::listen(Listener *listener) {
  m_listeners.emplace_back(listener);
  listener->listen();
  spdlog::info("Listening on {}:{}", listener->getListenIP(),
               listener->getListenPort());
}

void Engine::run() { m_loop.run(); }

int l_listen(lua_State *L) {
  const char *ip = luaL_optstring(L, 1, "::");
  int port = (int)luaL_optinteger(L, 2, 9009);

  lua_pushvalue(L, lua_upvalueindex(1));
  Engine *engine = reinterpret_cast<Engine *>(lua_touserdata(L, -1));
  engine->listen(new TcpListener(engine, ip, port));

  return 0;
}

} // namespace whatmud
