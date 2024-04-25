#ifndef WHATMUD_UV_CHECK_HPP
#define WHATMUD_UV_CHECK_HPP

#include "uv/handle.hpp"

namespace whatmud::uv {

class Check : public uv::Handle {
public:
  Check(uv_loop_t *loop);
  virtual ~Check();

  virtual uv_handle_t *asHandle() override {
    return reinterpret_cast<uv_handle_t *>(&m_handle);
  }
  virtual const uv_handle_t *asHandle() const override {
    return reinterpret_cast<const uv_handle_t *>(&m_handle);
  }

  void start(uv_check_cb cb);
  void stop();

private:
  uv_check_t m_handle;
};

} // namespace whatmud::uv

#endif
