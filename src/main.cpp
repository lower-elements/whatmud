#include "engine.hpp"

namespace lua = whatmud::lua;

int main() {
  whatmud::Engine engine;
  engine.run();
  return 0;
}
