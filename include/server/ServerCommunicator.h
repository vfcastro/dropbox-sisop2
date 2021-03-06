#ifndef __SC_H__
#define __SC_H__

#define MAX_OPEN_SESSIONS 2

#include <pthread.h>
#include <utility>	// std::pair
#include <map>		// std::map
#include <queue>	// std::queue
#include <list>		// std::tuple
#include "../../include/common/Message.h"
#include "../../include/server/ReplicaManager.h"
using namespace std;

struct ServerCommunicator {
  std::string host;
  unsigned int port;
  unsigned int backlog;
  int sockfd;

  pthread_t listenThread;
  
  //acceptedThreads é um mapeamento thread_t -> socketfd
  //é usado pelas threads de conexao para envio/recebimento de msgs nos sockets
  std::map<pthread_t,int> acceptedThreads;
  pthread_mutex_t acceptedThreadsLock;

  //a cada conexao do cliente retorna-se  um id;
  int connectionId;
  pthread_mutex_t connectionIdLock;

  //mapeamento thread_t -> connectionId
  //é usado pelas threads para identificar qual a connectionId em questao
  std::map<pthread_t,int> threadConnId; 
  pthread_mutex_t threadConnIdLock;

  //mapeamento username -> connectionId
  std::map< std::string , std::pair<int,int> > userSessions;
  pthread_mutex_t userSessionsLock;

  //mapeamento connectionId -> clientAddress/clientPort
  std::map<int,std::pair<std::string,unsigned int>> clientAddress;
  pthread_mutex_t clientAddressLock;

  //mapeamento connectionId -> fila de msgs para envio ao cliente
  std::map<int,std::queue<Message*>> sendQueue;
  pthread_mutex_t sendQueueLock;

  //ReplicaManager do servidor
  struct ReplicaManager *rm;
	  
};

void ServerCommunicator_init(ServerCommunicator *sc, ReplicaManager *rm, unsigned int port, unsigned int backlog);
void ServerCommunicator_start(ServerCommunicator *sc);
void* ServerCommunicator_listen(void* sc);
void* ServerCommunicator_accept(void* sc);
void ServerCommunicator_receiveFromServer(ServerCommunicator *sc, int sockfd, int type_msg, int port);
void ServerCommunicator_receive(ServerCommunicator *sc, int sockfd);
void ServerCommunicator_send(ServerCommunicator *sc, int sockfd, int connectionId);
void ServerCommunicator_exit(ServerCommunicator *sc, Message *msg);
void ServerCommunicator_updateOpenSendConn(ServerCommunicator *sc, Message *msg, int sockfd);
void ServerCommunicator_updateOpenRecvConn(ServerCommunicator *sc, Message *msg, int sockfd);

#endif
