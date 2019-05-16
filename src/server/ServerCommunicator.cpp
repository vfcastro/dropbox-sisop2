#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../include/common/Message.h"
#include "../../include/server/ServerCommunicator.h"
#include "../../include/server/ServerProcessor.h"

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog) {
	std::cout << "ServerCommunicator_init(): START\n";
	sc->port = port;
	sc->backlog = backlog;
	sc->connectionId = 0;

	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    	std::cout << "ServerCommunicator_init(): ERROR opening socket\n";
    	exit(-1);
	}
	sc->sockfd = sockfd;

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	    std::cout << "ServerCommunicator_init(): setsockopt(SO_REUSEADDR) failed\n";
	    exit(-1);
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(sc->port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cout << "ServerCommunicator_init(): ERROR on binding\n";
		exit(-1);
	}


	//Inicializa MUTEXes
	pthread_mutex_init(&sc->acceptedThreadsLock, NULL);
	pthread_mutex_init(&sc->connectionIdLock, NULL);
	pthread_mutex_init(&sc->threadConnIdLock, NULL);
	pthread_mutex_init(&sc->userSessionsLock, NULL);
	pthread_mutex_init(&sc->sendQueueLock, NULL);

	std::cout << "ServerCommunicator_init(): bind OK\n";
	std::cout << "ServerCommunicator_init(): END\n";
}

void ServerCommunicator_start(ServerCommunicator *sc) {
	std::cout << "ServerCommunicator_start(): START\n";
	pthread_create(&sc->listenThread,0,ServerCommunicator_listen,(void*)sc);
	std::cout << "ServerCommunicator_start(): listenThread created\n";
	pthread_join(sc->listenThread,NULL);
}

void* ServerCommunicator_listen(void* sc) {
	// std::cout << "ServerCommunicator_listen(): START\n";
	ServerCommunicator *s = (ServerCommunicator*)sc;

	listen(s->sockfd, s->backlog);
	std::cout << "ServerCommunicator_listen(): WAITING for conections\n";
	
	socklen_t clilen = sizeof(struct sockaddr_in);
	struct sockaddr *connection = NULL;
	int *newsockfd = NULL;

	while(1) {
		connection = (struct sockaddr *)malloc(clilen);
		newsockfd = (int*)malloc(sizeof(int));                        		
		pthread_t *acceptThread = (pthread_t *)malloc(sizeof(pthread_t));
		
		if ((*newsockfd = accept(s->sockfd, connection, &clilen)) == -1) {
			std::cout << "ServerCommunicator_listen(): ERROR on accept\n";
		}
		else {
			
			//MUTEX acceptThreads
			pthread_mutex_lock(&s->acceptedThreadsLock);
			pthread_create(acceptThread,0,ServerCommunicator_accept,(void*)sc);
			s->acceptedThreads.insert(std::pair<pthread_t,int>(*acceptThread,*newsockfd));
			pthread_mutex_unlock(&s->acceptedThreadsLock);

			std::cout << "ServerCommunicator_listen(): acceptThread " << *acceptThread << " fd " << *newsockfd << " created\n";

		}
	
		free(connection);
		free(newsockfd);
		free(acceptThread);
	}
}

void* ServerCommunicator_accept(void* sc) {
	// std::cout << "ServerCommunicator_accept(): START thread " << pthread_self() << "\n";

	ServerCommunicator *s = (ServerCommunicator*)sc;
	
	// adiciona mapeamento thread,sockfd
	// MUTEX acceptedThreadsLock
	pthread_mutex_lock(&s->acceptedThreadsLock);
	int sockfd = s->acceptedThreads.find(pthread_self())->second;
	pthread_mutex_unlock(&s->acceptedThreadsLock);

	Message *msg = (Message*) malloc(sizeof(Message));

	// protocolo de abertura de conexao
	if(Message_recv(msg,sockfd) != -1) {
		if(msg->type == OPEN_SEND_CONN) {
			// std::cout << "ServerCommunicator_accept(): read msg OPEN_SEND_CONN\n";		

			/* SECAO CRITICA */
			//add map thread -> connectionId
			pthread_mutex_lock(&s->connectionIdLock);
			s->connectionId = s->connectionId+1;
			s->threadConnId.insert(std::pair<pthread_t,int>(pthread_self(),s->connectionId));
			
			//tenta criar uma sessao
			if(s->userSessions.insert(std::pair<std::string,std::pair<int,int> >(msg->username,std::pair<int,int>(s->connectionId,0))).second == false) {
				//ja existia pelo uma sessão
				//checa se o segundo slot de sessao esta vazio
				if(s->userSessions.at(msg->username).second == 0)
					//cria segunda sessao
					s->userSessions.at(msg->username).second = s->connectionId;
				else {
					// Limite de sessoes excedido
					msg->type = NOK;
					Message_send(msg,sockfd);
					std::cout<<"ServerCommunicator_accept(): SESSIONS LIMIT EXCEEDED\n";
				}
			}
					

			pthread_mutex_unlock(&s->connectionIdLock);
			/* FINAL SECAO CRITICA */



			// passa-se ao cliente o connId pelo campo seqn
			msg->type = OK;
			msg->seqn = s->connectionId;
			if(Message_send(msg,sockfd) != -1){
				// std::cout << "ServerCommunicator_accept(): sent reply OK\n";
				ServerCommunicator_receive(s,sockfd);
			}
			else
				std::cerr << "ServerCommunicator_accept(): ERROR sent reply OK\n";
		}
		else 
			if (msg->type == OPEN_RECV_CONN) {
				// std::cout << "ServerCommunicator_accept(): read msg OPEN_RECV_CONN\n";		
				msg->type = OK;
				if(Message_send(msg,sockfd) != -1) {
					// std::cout << "ServerCommunicator_accept(): sent reply OK\n";
					// thread deve aguardar por msgs na fila identificada pelo connectionId
					// que vem do campo seqn da msg
					std::queue<Message*> queue;
					pthread_mutex_lock(&s->sendQueueLock);
					s->sendQueue.insert(std::pair<int,std::queue<Message*>>(msg->seqn,queue));
					pthread_mutex_unlock(&s->sendQueueLock);
					ServerCommunicator_send(s,sockfd,msg->seqn);
				}
				else
					std::cerr << "ServerCommunicator_accept(): ERROR sent reply OK\n";
			}
			else 
				std::cerr << "ServerCommunicator_accept(): ERROR recv msg type not OPEN_SEND_CONN nor OPEN_RECV_CONN\n";
	}
	else 
		std::cerr << "ServerCommunicator_accept(): ERROR recv msg\n";
	

	//free(msg);
	close(sockfd);

	pthread_mutex_lock(&s->acceptedThreadsLock);
	s->acceptedThreads.erase(pthread_self());
	pthread_mutex_unlock(&s->acceptedThreadsLock);

	// std::cout << "ServerCommunicator_accept(): ENDED thread " << pthread_self() << "\n"; 
}

void ServerCommunicator_receive(ServerCommunicator *sc, int sockfd) {
	// std::cout << "ServerCommunicator_receive(): WAITING for msg on fd " << sockfd << "\n"; 
	
	Message *msg = (Message*)malloc(sizeof(Message));
	while(Message_recv(msg,sockfd) != -1) {
		ServerProcessor_dispatch(sc,msg);
	}

	//free(msg);
	// std::cout << "ServerCommunicator_receive(): END " << sockfd << "\n"; 
}

void ServerCommunicator_send(ServerCommunicator *sc, int sockfd, int connectionId) {
	// std::cout << "ServerCommunicator_send(): START on connId "<< connectionId <<"\n";
	// Enquanto socket esta aberto, tenta ler evento para enviar ao client
	while(1) {
		// checa se ha msgs na fila identificada por connectionId
		pthread_mutex_lock(&sc->sendQueueLock);
		if(sc->sendQueue.at(connectionId).size() > 0) {
			// std::cout << "ServerCommunicator_send() msg RECEIVED ON QUEUE!\n";
			pthread_mutex_lock(&sc->userSessionsLock);
			
			Message *msg = sc->sendQueue.at(connectionId).front();

			std::cout << "\n\n\n";
			std::cout << "Quantidade da fila: " << sc->sendQueue.at(connectionId).size() << "\n";
			std::cout << "Retirando msg da fila e enviando " << sockfd << "\n";
			std::cout << "\n\n\n";

			if(Message_send(msg, sockfd) == -1){
				std::cerr << "ServerCommunicator_send() Error send msg!\n";
			}
			free(msg);
			sc->sendQueue.at(connectionId).pop();
			pthread_mutex_unlock(&sc->userSessionsLock);
			// std::cout << "ServerCommunicator_send() msg SENT!\n";
		}
		pthread_mutex_unlock(&sc->sendQueueLock);
			
	}

}
