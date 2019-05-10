#include <pthread.h>
#include <utility>	// std::pair
#include <queue>	// std::queue
#include "../../include/common/Message.h"

struct ServerCommunicator {
  unsigned int port;
  unsigned int backlog;
  int sockfd;
  pthread_t listenThread;
  
  std::queue<std::pair<int*,pthread_t*>> acceptQueue;
  
};

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog);
void ServerCommunicator_start(ServerCommunicator *sc);
void* ServerCommunicator_listen(void* sc);
void* ServerCommunicator_accept(void* sc);
void ServerCommunicator_sendmsg(ServerCommunicator *sc, Message *msg);
Message* ServerCommunicator_recvmsg();
