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

private:
  telnet_t *m_telnet;

  friend void forwardEvent(telnet_t *telnet, telnet_event_t *event,
                           void *user_data);
};

} // namespace whatmud

#endif
