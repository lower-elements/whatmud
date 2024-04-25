#ifndef WHATMUD_LISTENER_HPP
#define WHATMUD_LISTENER_HPP

#include <string>

#include "uv/tcp.hpp"

namespace whatmud {

// Forward declarations:
class Engine;

class Listener {
public:
  Listener(Engine *engine, const char *ip, int port);
  virtual ~Listener() = default;

  virtual void listen() = 0;

  struct sockaddr *getListenAddr() {
    return (struct sockaddr *)&m_listen_addr;
  }
  const struct sockaddr *getListenAddr() const {
    return (struct sockaddr *)&m_listen_addr;
  }

  std::string getListenIP() const;
  int getListenPort() const;

protected:
  Engine *m_engine;
  struct sockaddr_storage m_listen_addr;
};

class TcpListener : public Listener, protected uv::TCP {
public:
  TcpListener(Engine *engine, const char *ip = "::", int port = 9009);
  virtual ~TcpListener() = default;

  virtual void listen() override;
};

} // namespace whatmud

#endif
