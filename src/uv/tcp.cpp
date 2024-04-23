#include "uv/tcp.hpp"
#include "uv/error.hpp"

namespace whatmud::uv {

TCP::TCP(uv_loop_t *loop) {
  int res = uv_tcp_init(loop, &m_handle);
  uv::check_error(res);
}

TCP::TCP(uv_loop_t *loop, unsigned flags) {
  int res = uv_tcp_init_ex(loop, &m_handle, flags);
  uv::check_error(res);
}

void TCP::open(uv_os_sock_t sock) {
  int res = uv_tcp_open(&m_handle, sock);
  uv::check_error(res);
}

void TCP::setNodelay(bool nodelay) {
  int res = uv_tcp_nodelay(&m_handle, nodelay);
  uv::check_error(res);
}

void TCP::setKeepalive(bool keepalive, unsigned timeout) {
  int res = uv_tcp_keepalive(&m_handle, keepalive, timeout);
  uv::check_error(res);
}

void TCP::bind(const struct sockaddr *addr, unsigned int flags) {
  int res = uv_tcp_bind(&m_handle, addr, flags);
  uv::check_error(res);
}

int TCP::getSockName(struct sockaddr *name) const {
  int namelen;
  int res = uv_tcp_getsockname(&m_handle, name, &namelen);
  uv::check_error(res);
  return namelen;
}

int TCP::getPeerName(struct sockaddr *name) const {
  int namelen;
  int res = uv_tcp_getpeername(&m_handle, name, &namelen);
  uv::check_error(res);
  return namelen;
}

void TCP::connect(uv_connect_t *req, const struct sockaddr *addr,
                  uv_connect_cb cb) {
  int res = uv_tcp_connect(req, &m_handle, addr, cb);
  uv::check_error(res);
}

void TCP::closeReset(uv_close_cb close_cb) {
  int res = uv_tcp_close_reset(&m_handle, close_cb);
  uv::check_error(res);
}

} // namespace whatmud::uv
