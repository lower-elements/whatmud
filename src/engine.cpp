#include <stdexcept>

#include <fmt/core.h>

#include "engine.hpp"

namespace whatmud {

Engine::Engine() : L() {
  int res = uv_loop_init(&m_loop);
  if (res < 0) {
    throw std::runtime_error(
        fmt::format("Could not create libuv loop: {}", uv_strerror(res)));
  }
}

Engine::~Engine() {
  if (uv_loop_alive(&m_loop)) {
    uv_loop_close(&m_loop);
  }
}

void Engine::run() { uv_run(&m_loop, UV_RUN_DEFAULT); }

} // namespace whatmud
