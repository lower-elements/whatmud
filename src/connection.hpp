#ifndef WHATMUD_CONNECTION_HPP
#define WHATMUD_CONNECTION_HPP

#include <cstring>
#include <sstream>
#include <stddef.h>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/core.h>
#include <libtelnet.h>
#include <spdlog/spdlog.h>

#include "engine.hpp"
#include "features.hpp"
#include "uv/check.hpp"
#include "uv/tcp.hpp"

namespace whatmud {

class Connection : protected uv::TCP {
public:
  Connection(Engine *engine);
  ~Connection();

  void accept(uv_stream_t *server_sock);

  telnet_t *getTelnet() { return m_telnet; }
  const telnet_t *getTelnet() const { return m_telnet; }

  /**
   * Send data to the client.
   * Data is sent asynchronously by libuv.
   */
  void send(const char *buf, std::size_t size) {
    telnet_send(m_telnet, buf, size);
  }
  void send(const char *str) { send(str, std::strlen(str)); }
  void send(const std::string &str) { send(str.c_str(), str.size()); }
  void send(std::string_view str) { send(str.data(), str.size()); }

protected: // Event handlers
           // Called for each libtelnet event
  void onEvent(telnet_event_t &ev);
  // Called when the client closes the connection
  void onEof();
  // Called when data needs to be sent to the client
  void onSend(const char *buf, std::size_t size);
  // Called when data is received from the client
  void onRecv(const char *buf, std::size_t size);
  // Called for each message line
  void onMessage(const std::string &msg);

  // Called when the client requests to turn on a feature
  void onClientWill(unsigned char telopt);
  // Called when the client notifies us of a feature being turned off
  void onClientWont(unsigned char telopt);
  // Called when the client requests us to turn on a feature
  void onClientDo(unsigned char telopt);
  // Called when the client tells us to turn off a feature
  void onClientDont(unsigned char telopt);

  // Called when the client sends subnegotiation data
  void onClientSubNegotiate(unsigned char telopt, std::string_view data);

private:
  // Receive buffer, used to buffer message lines
  std::stringstream m_recv_buf;
  // Message processor, checks for and handles messages
  // We do this in a check handler instead of when data is received for more
  // fair distribution of CPU time per client
  uv::Check m_msg_proc;
  // Pointer back to the Engine
  Engine *m_engine;
  // Libtelnet state tracker
  telnet_t *m_telnet;
  // Features supported by this client
  Features m_features{};
  // Whether this client is still connected
  bool m_connected : 1 = true;

  static std::shared_ptr<spdlog::logger> m_log;

  // Friend functions used to call event handler member methods
  friend void onRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
  friend void forwardEvent(telnet_t *telnet, telnet_event_t *event,
                           void *user_data);
};

} // namespace whatmud

#endif
