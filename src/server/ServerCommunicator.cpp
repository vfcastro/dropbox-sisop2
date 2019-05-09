#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/ServerCommunicator.h"

ServerCommunicator::ServerCommunicator(unsigned int port, unsigned int backlog) {
	this->port = port;
	this->backlog = port;

	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    	throw "ServerCommunicator::ServerCommunicator: ERROR opening socket";
	this->sockfd = sockfd;

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(this->port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		throw "ServerCommunicator::ServerCommunicator: ERROR on binding";	
	
}

void ServerCommunicator::start() {
}

void* ServerCommunicator::startThread(void*) {
	//listen(this->sockfd, this->backlog);
	std::cout << "ServerCommunicator::startThread: Esperando conexoes...";
}

ServerCommunicator::~ServerCommunicator() {};


