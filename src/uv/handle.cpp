#include "uv/handle.hpp"

namespace whatmud::uv {

std::ostream &operator<<(std::ostream &out, const Handle &handle) {
  out << handle.getTypeName() << "@" << &handle;
  if (handle.isActive()) {
    out << " (active)";
  }
  if (handle.isClosing()) {
    out << " (closing)";
  }
  if (handle.hasRef()) {
    out << " (referenced)";
  }
  return out;
}

} // namespace whatmud::uv
