#ifndef WHATMUD_UV_TCP_HPP
#define WHATMUD_UV_TCP_HPP

#include "uv/stream.hpp"

namespace whatmud::uv {

class TCP : public Stream {
public:
  TCP(); // Uninitialised, used when accepting connections
  TCP(uv_loop_t *loop);
  TCP(uv_loop_t *loop, unsigned flags);

  virtual ~TCP() = default;

  virtual uv_stream_t *asStream() override { return (uv_stream_t *)&m_handle; }
  virtual const uv_stream_t *asStream() const override {
    return (uv_stream_t *)&m_handle;
  }

  uv_tcp_t *operator*() { return &m_handle; }
  const uv_tcp_t *operator*() const { return &m_handle; }
  uv_tcp_t *operator->() { return &m_handle; }
  const uv_tcp_t *operator->() const { return &m_handle; }

  void open(uv_os_sock_t sock);

  void setNodelay(bool nodelay);
  void setKeepalive(bool keepalive, unsigned timeout = 0);

  void bind(const struct sockaddr *addr, unsigned int flags = 0);

  int getSockName(struct sockaddr *name) const;
  int getPeerName(struct sockaddr *name) const;

  void connect(uv_connect_t *req, const struct sockaddr *addr,
               uv_connect_cb cb);

  void closeReset(uv_close_cb close_cb);

private:
  uv_tcp_t m_handle;
};

} // namespace whatmud::uv

#endif
