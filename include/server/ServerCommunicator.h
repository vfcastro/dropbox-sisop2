#include <pthread.h>
#include <utility>	// std::pair
#include <map>	// std::map
#include "../../include/common/Message.h"

struct ServerCommunicator {
  unsigned int port;
  unsigned int backlog;
  int sockfd;
  pthread_t listenThread;
  
  std::map<pthread_t,int> acceptedThreads;
  
};

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog);
void ServerCommunicator_start(ServerCommunicator *sc);
void* ServerCommunicator_listen(void* sc);
void* ServerCommunicator_accept(void* sc);
//void ServerCommunicator_write(ServerCommunicator *sc, Message *msg);
//void ServerCommunicator_read(ServerCommunicator *sc, int sockfd);
