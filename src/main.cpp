#include <cstdlib>
#include <string_view>

#include <fmt/core.h>
#include <spdlog/cfg/argv.h>

#include "engine.hpp"

namespace lua = whatmud::lua;

[[noreturn]] void usage(char *argv[], const char *errmsg) {
  const char *prog = argv[0] ? argv[0] : "whatmud";
  fmt::print(stderr, "Error: {}\n", errmsg);
  fmt::print(stderr, "Usage: {} [SPDLOG_LEVEL=<log-level>] <game-directory>\n",
             prog);
  std::exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  // Parse command-line arguments
  argv = uv_setup_args(argc, argv);

  const char *game_dir = nullptr;
  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);
    if (arg.starts_with("SPDLOG_LEVEL=")) {
      continue;
    }
    if (game_dir == nullptr) {
      game_dir = argv[i];
    } else {
      usage(argv, "Too many arguments were given");
    }
  }
  if (game_dir == nullptr) {
    usage(argv, "Please specify the directory to load the game from");
  }

  whatmud::Engine engine(game_dir);
  // This has to happen after engine construction to ensure command line params
  // override the Lua config
  spdlog::cfg::load_argv_levels(argc, argv);
  engine.run();
  return EXIT_SUCCESS;
}
