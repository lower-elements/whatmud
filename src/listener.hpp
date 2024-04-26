#ifndef WHATMUD_LISTENER_HPP
#define WHATMUD_LISTENER_HPP

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

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
  std::shared_ptr<spdlog::logger> m_log;
  struct sockaddr_storage m_listen_addr;
  Engine *m_engine;
};

class TcpListener : public Listener, protected uv::TCP {
public:
  TcpListener(Engine *engine, const char *ip = "::", int port = 4000);
  virtual ~TcpListener() = default;

  virtual void listen() override;

  void onNewConnection();
};

} // namespace whatmud

#endif
