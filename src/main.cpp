#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <uv.h>

#include "lua/helpers.hpp"

namespace lua = whatmud::lua;

int main() {
  lua::State L;
  spdlog::info("Libuv version {:x}", uv_version());
  spdlog::info("Lua version {}", (int)lua_version(L));
  spdlog::info("SQLite3 version {}", sqlite3_libversion());
  return 0;
}
