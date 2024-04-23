#include "engine.hpp"
#include <libtelnet.h>

namespace lua = whatmud::lua;

int main() {
  whatmud::Engine engine;
  engine.run();
  return 0;
}
