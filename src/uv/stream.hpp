#ifndef WHATMUD_UV_STREAM_HPP
#define WHATMUD_UV_STREAM_HPP

#include <cstddef>

#include <uv.h>

#include "uv/handle.hpp"

namespace whatmud::uv {

class Stream : public Handle {
public:
  virtual ~Stream() = default;

  virtual uv_handle_t *asHandle() override { return (uv_handle_t *)asStream(); }
  virtual const uv_handle_t *asHandle() const override {
    return (uv_handle_t *)asStream();
  }

  virtual uv_stream_t *asStream() = 0;
  virtual const uv_stream_t *asStream() const = 0;

  bool isReadable() const { return uv_is_readable(asStream()) != 0; }
  bool isWritable() const { return uv_is_writable(asStream()) != 0; }

  std::size_t getWriteQueueSize() { return asStream()->write_queue_size; }

  void shutdown(uv_shutdown_t *req, uv_shutdown_cb shutdowncb);

  void listen(int backlog, uv_connection_cb cb);

  void accept(uv_stream_t *client);
  void accept(Stream &client) { accept(client.asStream()); }

  void readStart(uv_alloc_cb alloccb, uv_read_cb readcb);
  bool readStop();

  void write(uv_write_t *req, const uv_buf_t bufs[], unsigned int nbufs,
             uv_write_cb cb);
  void write(uv_write_t *req, const uv_buf_t *buf, uv_write_cb cb) {
    write(req, buf, 1, cb);
  }
};

} // namespace whatmud::uv

#endif
