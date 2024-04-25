#include "uv/prepare.hpp"
#include "uv/error.hpp"

namespace whatmud::uv {

Prepare::Prepare(uv_loop_t *loop) { uv_prepare_init(loop, &m_handle); }

Prepare::~Prepare() { stop(); }

void Prepare::start(uv_prepare_cb cb) {
  int res = uv_prepare_start(&m_handle, cb);
  uv::check_error(res, "Could not start prepare handle");
}

void Prepare::stop() { uv_prepare_stop(&m_handle); }

} // namespace whatmud::uv
