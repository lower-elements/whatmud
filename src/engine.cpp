#include <stdexcept>

#include "spdlog/spdlog.h"
#include <fmt/core.h>

#include "engine.hpp"
#include "uv/error.hpp"

namespace whatmud {

Engine::Engine() : L(), m_loop(), m_server_socket(m_loop.asLoop(), AF_INET) {
  struct sockaddr_in addr;
  int res = uv_ip4_addr("0.0.0.0", 9009, &addr);
  uv::check_error(res, "Could not create address for server socket");
  m_server_socket.bind((const struct sockaddr *)&addr);
  m_server_socket.listen(32, [](uv_stream_t *server, int status) {
    uv::check_error(status, "Could not listen for incoming connections");
    spdlog::info("New connection!");
  });
}

Engine::~Engine() {}

void Engine::run() { m_loop.run(); }

} // namespace whatmud
