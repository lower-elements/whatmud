#ifndef WHATMUD_UV_HANDLE_HPP
#define WHATMUD_UV_HANDLE_HPP

#include <ostream>

#include <uv.h>

namespace whatmud::uv {

class Handle {
public:
  virtual ~Handle() = default;

  virtual uv_handle_t *asHandle() = 0;
  virtual const uv_handle_t *asHandle() const = 0;

  uv_handle_type getType() const { return asHandle()->type; }
  const char *getTypeName() const { return uv_handle_type_name(getType()); }

  template <class T> T *as() {
    if (getType() == T::TYPE) {
      return (T *)asHandle();
    }
    return nullptr;
  }

  uv_loop_t *getLoop() { return asHandle()->loop; }

  void *getData() { return asHandle()->data; }
  void setData(void *data) { asHandle()->data = data; }

  bool isActive() const { return uv_is_active(asHandle()) != 0; }

  bool isClosing() const { return uv_is_closing(asHandle()) != 0; }
  void close(uv_close_cb closecb = nullptr) { uv_close(asHandle(), closecb); }

  bool hasRef() const { return uv_has_ref(asHandle()) != 0; }
  void ref() { uv_ref(asHandle()); }
  void unref() { uv_unref(asHandle()); }

  friend std::ostream &operator<<(std::ostream &out, const Handle &handle);
};

} // namespace whatmud::uv

#endif
