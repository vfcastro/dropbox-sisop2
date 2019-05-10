#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../include/server/ServerCommunicator.h"

void ServerCommunicator_init(ServerCommunicator *sc, unsigned int port, unsigned int backlog) {
	std::cout << "ServerCommunicator_init(): START\n";
	sc->port = port;
	sc->backlog = backlog;

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
	std::cout << "ServerCommunicator_listen(): START\n";
	listen(((ServerCommunicator*)sc)->sockfd, ((ServerCommunicator*)sc)->backlog);
	std::cout << "ServerCommunicator_listen(): WAITING for conections\n";
	
	socklen_t clilen = sizeof(struct sockaddr_in);
	struct sockddr *connection = NULL;
	int *newsockfd = NULL;

	while(1) {
		connection = (struct sockaddr *)malloc(clilen);
		newsockfd = (int*)malloc(sizeof(int));                        		
		pthread_t *acceptThread = (pthread_t *)malloc(sizeof(pthread_t));
		
		if ((*newsockfd = accept(((ServerCommunicator*)sc)->sockfd, connection, &clilen)) == -1) {
			std::cout << "ServerCommunicator_listen(): ERROR on accept\n";
			free(connection);
			free(newsockfd);
			free(acceptThread);
		}
		else {
			//pthread_create(acceptThread,0,ServerCommunicator_accept,(void*)sc);
			std::cout << "ServerCommunicator_listen(): acceptThread created\n";
		}
	}
}
