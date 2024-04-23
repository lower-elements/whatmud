#include "uv/loop.hpp"
#include "uv/error.hpp"

namespace whatmud::uv {

Loop::Loop() {
  int res = uv_loop_init(&m_loop);
  uv::check_error(res, "Could not create event loop");
}

Loop::~Loop() {
  uv_stop(&m_loop);
  int res = uv_loop_close(&m_loop);
  uv::check_error(res, "Could not close event loop");
}

void Loop::run(uv_run_mode mode) {
  int res = uv_run(&m_loop, mode);
  uv::check_error(res, "Could not run event loop");
}

} // namespace whatmud::uv
