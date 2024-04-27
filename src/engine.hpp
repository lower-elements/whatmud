#ifndef WHATMUD_ENGINE_HPP
#define WHATMUD_ENGINE_HPP

#include <memory>
#include <vector>

#include <spdlog/spdlog.h>
#include <uv.h>

#include "listener.hpp"
#include "lua/state.hpp"
#include "uv/loop.hpp"
#include "uv/tcp.hpp"

namespace whatmud {

class Engine {
public:
  Engine(const char *game_dir);
  ~Engine() = default;

  uv_loop_t *getLoop() { return m_loop.asLoop(); }
  const uv_loop_t *getLoop() const { return m_loop.asLoop(); }

  lua_State *getLuaState() { return L; }

  void listen(std::unique_ptr<Listener> &&listener);

  void run();

private:
  // Initialisation functions
  void registerLuaBuiltins();
  void loadGameCode();
  void setLogLevel();
  void loadClientHandler();

  // Find and load a Lua script in the game directory
  void requireFrom(std::string_view name);

private:
  std::shared_ptr<spdlog::logger> m_log;
  uv::Loop m_loop;
  std::string m_game_dir;
  std::vector<std::unique_ptr<Listener>> m_listeners;
  lua::State L;
};

} // namespace whatmud

#endif
