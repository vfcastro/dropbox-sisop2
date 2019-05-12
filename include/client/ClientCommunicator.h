#ifndef __CC_H__
#define __CC_H__

#include <string>
#include <pthread.h>
#include "../../include/common/Message.h"

struct ClientCommunicator {
  std::string username;
  std::string server;
  unsigned int port;

  int sendsockfd;
  int recvsockfd;
  pthread_t recvThread;

};

void ClientCommunicator_init(ClientCommunicator *cc, std::string username, std::string server, unsigned int port);
void ClientCommunicator_start(ClientCommunicator *cc);
void* ClientCommunicator_receive(void *cc);

#endif
