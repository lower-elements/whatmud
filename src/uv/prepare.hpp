#ifndef WHATMUD_UV_PREPARE_HPP
#define WHATMUD_UV_PREPARE_HPP

#include "uv/handle.hpp"

namespace whatmud::uv {

class Prepare : public uv::Handle {
public:
  Prepare(uv_loop_t *loop);
  virtual ~Prepare();

  virtual uv_handle_t *asHandle() override {
    return reinterpret_cast<uv_handle_t *>(&m_handle);
  }
  virtual const uv_handle_t *asHandle() const override {
    return reinterpret_cast<const uv_handle_t *>(&m_handle);
  }

  void start(uv_prepare_cb cb);
  void stop();

private:
  uv_prepare_t m_handle;
};

} // namespace whatmud::uv

#endif
