#include <stdexcept>

#include <fmt/core.h>

#include "engine.hpp"
#include "uv/error.hpp"

namespace whatmud {

Engine::Engine() : L() {
  int res = uv_loop_init(&m_loop);
  uv::check_error(res, "Could not create event loop");
}

Engine::~Engine() {
  if (uv_loop_alive(&m_loop)) {
    int res = uv_loop_close(&m_loop);
    uv::check_error(res, "Could not close event loop");
  }
}

void Engine::run() {
  int res = uv_run(&m_loop, UV_RUN_DEFAULT);
  uv::check_error(res, "Error running event loop");
}

} // namespace whatmud
