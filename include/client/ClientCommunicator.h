#ifndef __CC_H__
#define __CC_H__

#include <string>
#include <pthread.h>
#include <set>
#include "../../include/common/Message.h"

struct ClientCommunicator {
  char username[MAX_USERNAME_SIZE];
  std::string server;
  unsigned int port;

  int sendsockfd;
  int recvsockfd;
  pthread_t recvThread;

  //int pauseSync = 0;
  pthread_mutex_t syncFilesLock;
  //conjunto de arquivos sendo sincronizados no momento
	std::set<std::string> syncFiles;

 };

void ClientCommunicator_init(ClientCommunicator *cc, std::string username, std::string server, unsigned int port);
void ClientCommunicator_start(ClientCommunicator *cc);
void* ClientCommunicator_receive(void *cc);
void ClientCommunicator_openSession(ClientCommunicator *cc);


#endif
