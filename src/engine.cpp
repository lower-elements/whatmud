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

static_assert(LUA_EXTRASPACE >= sizeof(void *),
              "Lua must be compiled with LUA_EXTRASPACE = sizeof(void*)");

// Forward declarations:
int l_listen(lua_State *L);

Engine::Engine(const char *game_dir)
    : m_log(spdlog::stderr_color_st("engine")), m_loop(), m_game_dir(game_dir),
      m_listeners(), L() {
  registerLuaBuiltins();
  loadGameCode();
  setLogLevel();
  loadClientHandler();
}

void Engine::registerLuaBuiltins() {
  // Put a pointer to the engine in the main thread's extra space
  auto extraspace = reinterpret_cast<Engine **>(lua_getextraspace(L.get()));
  *extraspace = this;

  // Create the connections table
  lua_newtable(L);
  lua_setfield(L, LUA_REGISTRYINDEX, "connections");

  // Register Lua configuration functions
  lua_pushcfunction(L, l_listen);
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

void Engine::loadClientHandler() {
  // Get name of client handler script to run
  lua_getglobal(L, "client_handler");
  if (!lua_isstring(L, -1)) {
    throw std::runtime_error(fmt::format(
        "Expected global 'client_handler' to be nil or string, got {}",
        luaL_typename(L, -1)));
  }
  std::string_view name;
  lua::get(L, -1, name);

  // Load the chunk
  requireFrom(name);

  // Store in registry
  lua_setfield(L, LUA_REGISTRYINDEX, "client_handler");
  lua_pop(L, 1);
}

void Engine::requireFrom(std::string_view name) {
  // Get the package.searchpath function
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "searchpath");
  lua_replace(L, -2);

  // Push name of script
  lua::push(L, name);

  // Push path to search
  std::string path(fmt::format("{0}/?.lua;{0}/?/init.lua", m_game_dir));
  lua::push(L, path);

  lua_call(L, 2, 2);
  // Check if the file was found
  if (lua_isnil(L, -2)) {
    lua_replace(L, -2);
    throw lua::Error(L, fmt::format("No module {}", name));
  }
  lua_pop(L, 1);
  // Search was successful, get script path
  const char *script_path;
  lua::get(L, -1, script_path);
  // Load chunk
  int res = luaL_loadfilex(L, script_path, "t");
  lua_replace(L, -2);
  if (res != LUA_OK) {
    throw lua::Error(L, "Could not load chunk");
  }
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

  Engine *engine = *reinterpret_cast<Engine **>(lua_getextraspace(L));
  engine->listen(std::make_unique<TcpListener>(engine, ip, port));

  return 0;
}

} // namespace whatmud
