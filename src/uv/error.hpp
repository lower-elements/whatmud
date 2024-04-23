#ifndef WHATMUD_UV_ERROR_HPP
#define WHATMUD_UV_ERROR_HPP

#include <exception>
#include <string>

#include <uv.h>

namespace whatmud::uv {

class Error : public std::exception {
public:
  Error(int status, std::string prefix = "Libuv error");
  virtual ~Error() override;

  virtual const char *what() const noexcept override;

private:
  std::string m_msg;
};

void check_error(int status, std::string prefix = "Libuv error");

} // namespace whatmud::uv

#endif
