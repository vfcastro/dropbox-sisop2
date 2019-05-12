#ifndef __SC_H__
#define __SC_H__

#include <pthread.h>
#include <utility>	// std::pair
#include <map>	// std::map

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
void ServerCommunicator_receive(ServerCommunicator *sc, int sockfd);
#endif
