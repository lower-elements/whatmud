#ifndef WHATMUD_LUA_STATE_HPP
#define WHATMUD_LUA_STATE_HPP

#include <stdexcept>

#include <lua.hpp>

namespace whatmud::lua {

class State {
public:
  State();
  State(lua_State *state) : L(state) {}

  // No copy
  State(const State &) = delete;
  State &operator=(const State &) = delete;

  State(State &&s) : L(s.L) { s.L = nullptr; }
  State &operator=(State &&s) {
    L = s.L;
    s.L = nullptr;
    return *this;
  }

  ~State();

  lua_State *get() & { return L; }

  operator lua_State *() & { return get(); }

private:
  lua_State *L;
};

} // namespace whatmud::lua

#endif
