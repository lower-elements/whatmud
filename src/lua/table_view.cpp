#include "lua/table_view.hpp"

namespace whatmud::lua {

TableView::TableView(lua_State *state, int index) : L(state), m_index(index) {}

TableView TableView::registry(lua_State *L) {
  return TableView(L, LUA_REGISTRYINDEX);
}

} // namespace whatmud::lua
