#ifndef WHATMUD_UTIL_HPP
#define WHATMUD_UTIL_HPP

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <memory>
#include <type_traits>
#include <typeindex>

#include <fmt/core.h>

namespace whatmud {

static inline void *align_down(void *ptr, std::uintptr_t alignment) {
  return (void *)((std::uintptr_t)ptr & ~(alignment - 1));
}

static inline void *align_up(void *ptr, std::uintptr_t alignment) {
  return (void *)((std::uintptr_t)align_down(ptr, alignment) + alignment);
}

template <class T> std::size_t type_id() {
  return std::type_index(typeid(T)).hash_code();
}

} // namespace whatmud

#endif
