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
	pthread_create(&sc->listenThread,0,ServerCommunicator_accept,(void*)sc);
	std::cout << "ServerCommunicator_start(): listenThread created\n";
	pthread_join(sc->listenThread,NULL);
}

void* ServerCommunicator_accept(void* sc) {
	std::cout << "ServerCommunicator_accept(): START\n";
	listen(((ServerCommunicator*)sc)->sockfd, ((ServerCommunicator*)sc)->backlog);
	std::cout << "ServerCommunicator_accept(): WAITING for conections\n";
	
	while(1) {
		socklen_t clilen = sizeof(struct sockaddr_in);
		struct sockaddr *connection = (struct sockaddr *)malloc(clilen);
		int *newsockfd = (int*)malloc(sizeof(int));                        		
		
		if ((*newsockfd = accept(((ServerCommunicator*)sc)->sockfd, connection, &clilen)) == -1) {
			std::cout << "ServerCommunicator_accept(): ERROR on accept\n";
			free(connection);
		}
		else {
			std::cout << "accept\n";
			//pthread_create(&thread, 0, process, (void *)newsockfd);
			//pthread_detach(thread);
		}
	}
}
