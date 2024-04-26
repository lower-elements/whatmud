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
  ~Engine();

  uv_loop_t *getLoop() { return m_loop.asLoop(); }
  const uv_loop_t *getLoop() const { return m_loop.asLoop(); }

  void listen(Listener *listener);

  void run();

private:
  std::shared_ptr<spdlog::logger> m_log;
  uv::Loop m_loop;
  std::vector<std::unique_ptr<Listener>> m_listeners;
  lua::State L;
};

} // namespace whatmud

#endif
