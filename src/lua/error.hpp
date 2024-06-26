#ifndef WHATMUD_LUA_ERROR_HPP
#define WHATMUD_LUA_ERROR_HPP

#include <exception>
#include <string>

#include <lua.hpp>

namespace whatmud::lua {

class Error : public std::exception {
public:
  Error(lua_State *L, std::string prefix = "Lua error");
  virtual ~Error() override;

  virtual const char *what() const noexcept override;

private:
  std::string m_msg;
};

int on_error(lua_State *L);

} // namespace whatmud::lua

#endif
