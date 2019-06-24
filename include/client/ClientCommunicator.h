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
  unsigned int frontend_port;
  int connectionId;

  int sendsockfd;
  int recvsockfd;
  pthread_t recvThread;
  
  //int pauseSync = 0;
  pthread_mutex_t syncFilesLock;

  //lock para acesso aos sockets
  pthread_mutex_t sockfdLock;

  //conjunto de arquivos sendo sincronizados no momento
	std::set<std::string> syncFiles;

  //thread listen caso haja eleicao de novo server primario
  pthread_t frontEndThread;

 };

void ClientCommunicator_init(ClientCommunicator *cc, std::string username);
void ClientCommunicator_start(ClientCommunicator *cc);
void* ClientCommunicator_receive(void *cc);
void ClientCommunicator_openSession(ClientCommunicator *cc);
int ClientCommunicator_getSendSocket(ClientCommunicator *cc);
int ClientCommunicator_getRecvSocket(ClientCommunicator *cc);


#endif
