#include <pthread.h>

struct ServerCommunicator {
  unsigned int port;
  unsigned int backlog;
  int sockfd;
  pthread_t listenThread;
  
};

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog);
void ServerCommunicator_start(ServerCommunicator *sc);

void* ServerCommunicator_accept(void* sc);
