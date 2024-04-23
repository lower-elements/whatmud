#include "uv/stream.hpp"
#include "uv/error.hpp"

namespace whatmud::uv {

void Stream::shutdown(uv_shutdown_t *req, uv_shutdown_cb cb) {
  int res = uv_shutdown(req, asStream(), cb);
  uv::check_error(res, "Could not initiate shutdown request");
}

void Stream::listen(int backlog, uv_connection_cb cb) {
  int res = uv_listen(asStream(), backlog, cb);
  uv::check_error(res, "Could not initiate shutdown request");
}

void Stream::accept(uv_stream_t *client) {
  int res = uv_accept(asStream(), client);
  uv::check_error(res);
}

void Stream::readStart(uv_alloc_cb alloccb, uv_read_cb readcb) {
  int res = uv_read_start(asStream(), alloccb, readcb);
  uv::check_error(res);
}

bool Stream::readStop() {
  int closeAfterPending = uv_read_stop(asStream());
  return closeAfterPending != 0;
}

void Stream::write(uv_write_t *req, const uv_buf_t bufs[], unsigned int nbufs,
                   uv_write_cb cb) {
  int res = uv_write(req, asStream(), bufs, nbufs, cb);
  uv::check_error(res);
}

} // namespace whatmud::uv
