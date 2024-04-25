#include "uv/check.hpp"
#include "uv/error.hpp"

namespace whatmud::uv {

Check::Check(uv_loop_t *loop) { uv_check_init(loop, &m_handle); }

Check::~Check() {}

void Check::start(uv_check_cb cb) {
  int res = uv_check_start(&m_handle, cb);
  uv::check_error(res, "Could not start check handle");
}

void Check::stop() { uv_check_stop(&m_handle); }

} // namespace whatmud::uv
