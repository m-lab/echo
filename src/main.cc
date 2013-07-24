#include <pthread.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include "mlab/mlab.h"
#include "mlab/server_socket.h"
#include "mlab/socket_type.h"

const int MAX_BYTES = 1024;

namespace {
struct ServerConfig {
  SocketType type;
  uint16_t port;
};

const timeval kNoTimeout = {0, 0};

static void* ServerThread(void* config_ptr) {
  ServerConfig* config = (ServerConfig*) config_ptr;

  while (true) {
    mlab::ServerSocket* socket = mlab::ServerSocket::CreateOrDie(
        config->port, config->type, SOCKETFAMILY_IPV6);

    // Disable timeouts.
    setsockopt(socket->raw(), SOL_SOCKET, SO_RCVTIMEO,
               (const char*) &kNoTimeout, sizeof(kNoTimeout));
    setsockopt(socket->raw(), SOL_SOCKET, SO_SNDTIMEO,
               (const char*) &kNoTimeout, sizeof(kNoTimeout));

    std::cout << "[" << config->type << ":" << config->port << "] Up";
    socket->Select();
    socket->Accept();
    std::cout << "[" << config->type << ":" << config->port << "] Ready";

    mlab::Packet p = socket->ReceiveOrDie(MAX_BYTES);
    std::cout << "[" << config->type << ":" << config->port << "] Echoing: "
              << p.buffer() << "\n";
    socket->SendOrDie(p);
    delete socket;
  }
  delete config;

  return NULL;
}
}  // namespace

int main(int argc, const char* argv[]) {
  mlab::Initialize("echo", "0.1");

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <tcp_port> <udp_port>\n";
    return 1;
  }

  std::cout << "Creating TCP server\n";
  ServerConfig* tcp_config = new ServerConfig();
  tcp_config->port = strtoul(argv[1], NULL, 10);
  tcp_config->type = SOCKETTYPE_TCP;

  pthread_t tcp_thread;
  int rc = pthread_create(&tcp_thread, NULL, ServerThread, (void*) tcp_config);
  if (rc != 0) {
    std::cerr << "Failed to create tcp thread: " << strerror(errno)
              << " [" << errno << "]\n";
    return 1;
  }

  std::cout << "Creating UDP server\n";
  ServerConfig* udp_config = new ServerConfig();
  udp_config->port = strtoul(argv[2], NULL, 10);
  udp_config->type = SOCKETTYPE_UDP;

  pthread_t udp_thread;
  rc = pthread_create(&udp_thread, NULL, ServerThread, (void*) udp_config);
  if (rc != 0) {
    std::cerr << "Failed to create udp thread: " << strerror(errno)
              << " [" << errno << "]\n";
    return 1;
  }

  pthread_exit(NULL);
  return 0;
}
