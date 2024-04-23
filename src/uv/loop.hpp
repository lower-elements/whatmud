#ifndef WHATMUD_UV_LOOP_HPP
#define WHATMUD_UV_LOOP_HPP

#include <uv.h>

namespace whatmud::uv {

class Loop {
public:
  Loop();
  ~Loop();

  uv_loop_t *asLoop() { return &m_loop; }
  const uv_loop_t *asLoop() const { return &m_loop; }

  bool isAlive() const { return uv_loop_alive(&m_loop) != 0; }

  void run(uv_run_mode mode = UV_RUN_DEFAULT);

  void stop() { uv_stop(&m_loop); }

private:
  uv_loop_t m_loop;
};
} // namespace whatmud::uv

#endif
