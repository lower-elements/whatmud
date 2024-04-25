#ifndef WHATMUD_CONNECTION_HPP
#define WHATMUD_CONNECTION_HPP

#include <cstring>
#include <stddef.h>
#include <utility>

#include <fmt/core.h>
#include <libtelnet.h>

#include "uv/tcp.hpp"

namespace whatmud {

class Connection : protected uv::TCP {
public:
  Connection(uv_loop_t *loop);
  ~Connection();

  void accept(uv_stream_t *server_sock);

  telnet_t *getTelnet() { return m_telnet; }
  const telnet_t *getTelnet() const { return m_telnet; }

  void send(const char *buf, std::size_t size) {
    telnet_send(m_telnet, buf, size);
  }
  void send(const char *str) { send(str, std::strlen(str)); }
  void send(const std::string &str) { send(str.c_str(), str.size()); }
  void send(std::string_view str) { send(str.data(), str.size()); }
  template <class... Args> void send(Args &&...args) {
    send(fmt::format(std::forward(args...)));
  }

protected:
  void onEvent(telnet_event_t &ev);
  void onEof();

  void sendData(const char *buf, std::size_t size);

private:
  telnet_t *m_telnet;

  friend void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
  friend void forwardEvent(telnet_t *telnet, telnet_event_t *event,
                           void *user_data);
};

} // namespace whatmud

#endif
