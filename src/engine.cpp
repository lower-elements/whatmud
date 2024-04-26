#include <stdexcept>

#include "spdlog/spdlog.h"
#include <fmt/core.h>
#include <spdlog/cfg/helpers.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "connection.hpp"
#include "engine.hpp"
#include "lua/helpers.hpp"
#include "uv/error.hpp"

namespace whatmud {

// Forward declarations:
int l_listen(lua_State *L);

Engine::Engine(const char *game_dir)
    : m_log(spdlog::stderr_color_st("engine")), m_loop(), m_game_dir(game_dir),
      m_listeners(), L() {
  registerLuaBuiltins();
  loadGameCode();
  setLogLevel();
}

void Engine::registerLuaBuiltins() {
  // // Register Lua configuration functions
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_listen, 1);
  lua_setglobal(L, "listen");
}

void Engine::loadGameCode() {
  std::string init_script(fmt::format("{}/init.lua", m_game_dir));
  m_log->info("Loading game init script from {}", init_script);
  int res = luaL_loadfilex(L, init_script.c_str(), "t");
  if (res != LUA_OK) {
    throw lua::Error(L, "Could not load game init script");
  }
  lua_call(L, 0, 0);
}

void Engine::setLogLevel() {
  // Set log level from Lua config
  lua_getglobal(L, "log_level");
  if (lua_isstring(L, -1)) {
    std::string log_level;
    lua::get(L, -1, log_level);
    spdlog::cfg::helpers::load_levels(log_level);
  } else if (!lua_isnil(L, -1)) {
    m_log->warn(
        "Unknown type for global `log_level`: expected string or nil got {}",
        luaL_typename(L, -1));
  }
  lua_pop(L, 1);
}

void Engine::listen(std::unique_ptr<Listener> &&listener) {
  listener->listen();
  m_listeners.emplace_back(std::move(listener));
}

void Engine::run() {
  // Warn the user if no listeners were created
  if (m_listeners.empty()) {
    m_log->warn(
        "No listeners created, did you forget to call listen() in {}/init.lua?",
        m_game_dir);
  }

  m_loop.run();
}

int l_listen(lua_State *L) {
  const char *ip = luaL_optstring(L, 1, "::");
  int port = (int)luaL_optinteger(L, 2, 4000);

  lua_pushvalue(L, lua_upvalueindex(1));
  Engine *engine = reinterpret_cast<Engine *>(lua_touserdata(L, -1));
  engine->listen(std::make_unique<TcpListener>(engine, ip, port));

  return 0;
}

} // namespace whatmud
