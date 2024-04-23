#include "engine.hpp"
#include <libtelnet.h>

namespace lua = whatmud::lua;

int main(int argc, char **argv) {
  argv = uv_setup_args(argc, argv);
  whatmud::Engine engine;
  engine.run();
  return 0;
}
