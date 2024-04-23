#include <cstddef>
#include <cstring>

#include <fmt/core.h>

#include "uv/error.hpp"

namespace whatmud::uv {

Error::Error(int status, std::string prefix) {
  m_msg = std::move(prefix);
  m_msg += ": ";
  auto size = m_msg.size();
  m_msg.resize(size + 512);
  char *errbuf = &m_msg[size];
  size += std::strlen(uv_strerror_r(status, errbuf, 512));
  m_msg.resize(size);
}

Error::~Error() {}

const char *Error::what() const noexcept { return m_msg.c_str(); }

void check_error(int status, std::string prefix) {
  if (status < 0) {
    throw Error(status, std::move(prefix));
  }
}

} // namespace whatmud::uv
