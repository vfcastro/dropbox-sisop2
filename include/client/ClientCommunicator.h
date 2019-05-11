#include <string>
#include <pthread.h>
#include "../../include/common/Message.h"

struct ClientCommunicator {
  std::string username;
  std::string server;
  unsigned int port;

  int sendsockfd;
  int recvsockfd;
  pthread_t sendthread;
  pthread_t recvthread;

};

void ClientCommunicator_init(ClientCommunicator *cc, std::string username, std::string server, unsigned int port);
void ClientCommunicator_start(ClientCommunicator *cc);
void ClientCommunicator_waitRecv(ClientCommunicator *cc);
