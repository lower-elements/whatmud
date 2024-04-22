#ifndef WHATMUD_ENGINE_HPP
#define WHATMUD_ENGINE_HPP

#include <uv.h>

#include "lua/state.hpp"

namespace whatmud {

class Engine {
public:
  Engine();
  ~Engine();

  void run();

private:
  lua::State L;
  uv_loop_t m_loop;
};

} // namespace whatmud

#endif
