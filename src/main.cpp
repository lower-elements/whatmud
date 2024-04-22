#include <fmt/core.h>
#include <lua.hpp>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <uv.h>

int main() {
  lua_State *L = luaL_newstate();
  spdlog::info("Libuv version {:x}", uv_version());
  spdlog::info("Lua version {}", (int)lua_version(L));
  spdlog::info("SQLite3 version {}", sqlite3_libversion());
  lua_close(L);
  return 0;
}
