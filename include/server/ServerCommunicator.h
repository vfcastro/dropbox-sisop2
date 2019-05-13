#ifndef __SC_H__
#define __SC_H__

#define MAX_OPEN_SESSIONS 2

#include <pthread.h>
#include <utility>	// std::pair
#include <map>		// std::map
#include <queue>	// std::queue
#include <list>		// std::tuple
#include "../../include/common/Message.h"
using namespace std;

struct ServerCommunicator {
  unsigned int port;
  unsigned int backlog;
  int sockfd;
  pthread_t listenThread;
  
  //acceptedThreads é um mapeamento thread_t -> socketfd
  //é usado pelas threads de conexao para envio/recebimento de msgs nos sockets
  std::map<pthread_t,int> acceptedThreads;

  //a cada conexao do cliente retorna-se  um id;
  int connectionId;

  //mapeamento thread_t -> connectionId
  //é usado pelas threads para identificar qual a connectionId em questao
  std::map<pthread_t,int> threadConnId; 

  //mapeamento connectionId -> fila de msgs para envio ao cliente
  std::map<int,std::queue<Message>> sendQueue;
	  
};

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog);
void ServerCommunicator_start(ServerCommunicator *sc);
void* ServerCommunicator_listen(void* sc);
void* ServerCommunicator_accept(void* sc);
void ServerCommunicator_receive(ServerCommunicator *sc, int sockfd);
void ServerCommunicator_send(ServerCommunicator *sc, int sockfd, int connectionId);
#endif
