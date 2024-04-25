#ifndef WHATMUD_ENGINE_HPP
#define WHATMUD_ENGINE_HPP

#include <uv.h>

#include "lua/state.hpp"
#include "uv/loop.hpp"
#include "uv/tcp.hpp"

namespace whatmud {

class Engine {
public:
  Engine(const char *game_dir);
  ~Engine();

  void run();

private:
  lua::State L;
  uv::Loop m_loop;
  uv::TCP m_server_socket;
};

} // namespace whatmud

#endif
