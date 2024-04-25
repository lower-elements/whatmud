#include <cstdlib>

#include <fmt/core.h>

#include "engine.hpp"

namespace lua = whatmud::lua;

int main(int argc, char **argv) {
  argv = uv_setup_args(argc, argv);
  if (argc < 2) {
    const char *prog = argv[0] ? argv[0] : "whatmud";
    fmt::print(stderr, "Error: Please specify a directory containing the game "
                       "you wish to run inside the engine\n");
    fmt::print(stderr, "Usage: {} <game-directory>\n", prog);
    return EXIT_FAILURE;
  }
  whatmud::Engine engine(argv[1]);
  engine.run();
  return EXIT_SUCCESS;
}
