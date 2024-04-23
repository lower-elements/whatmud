#ifndef WHATMUD_CONNECTION_HPP
#define WHATMUD_CONNECTION_HPP

#include <stddef.h>

#include <libtelnet.h>

#include "uv/tcp.hpp"

namespace whatmud {

class Connection : protected uv::TCP {
public:
  Connection();
  ~Connection();

  void accept(uv::TCP &server_sock);

  telnet_t *getTelnet() { return m_telnet; }
  const telnet_t *getTelnet() const { return m_telnet; }

protected:
  void onEvent(telnet_event_t &ev);
  void onEof();

  void sendData(const char *buf, std::size_t size);

private:
  telnet_t *m_telnet;

  friend void forwardEvent(telnet_t *telnet, telnet_event_t *event,
                           void *user_data);
  void friend onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
};

} // namespace whatmud

#endif
