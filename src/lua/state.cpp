#include <cstring>

#include <fmt/core.h>

#include "lua/error.hpp"
#include "lua/state.hpp"

namespace whatmud::lua {

State::State() : L(luaL_newstate()) {
  if (!L) {
    throw std::runtime_error("Could not initialize Lua state");
  }
  lua_atpanic(L, lua::on_error);
  luaL_checkversion(L);
  luaL_openlibs(L);
}

State::~State() {
  if (L != nullptr) {
    lua_close(L);
  }
}

} // namespace whatmud::lua
