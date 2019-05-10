#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                      
#include "../../include/client/ClientCommunicator.h"

void ClientCommunicator_init(ClientCommunicator *cc, std::string username, std::string server, unsigned int port) {
	std::cout << "ClientCommunicator_init(): START\n";
	cc->username = std::string(username);
	cc->server = std::string(server);
	cc->port = port;
	
	struct hostent *sv = gethostbyname(cc->server.c_str());

	if (sv == NULL) {
        std::cerr << "ClientCommunicator_init(): ERROR, no such host\n";
        exit(-1);
    }
   
	int sendsockfd; 
    if ((sendsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "ClientCommunicator_init(): ERROR opening send socket\n";
		exit(-1);
	}
	cc->sendsockfd = sendsockfd;

	int recvsockfd; 
    if ((recvsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "ClientCommunicator_init(): ERROR opening recv socket\n";
		exit(-1);
	}
	cc->recvsockfd = recvsockfd;

	int enable = 1;
	if (setsockopt(sendsockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	    std::cout << "ServerCommunicator_init(): setsockopt(SO_REUSEADDR) failed\n";
	    exit(-1);
	}
 
 	if (setsockopt(recvsockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
 	    std::cout << "ServerCommunicator_init(): setsockopt(SO_REUSEADDR) failed\n";
 	    exit(-1);
 	}

	struct sockaddr_in serv_addr; 
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(cc->port);    
	serv_addr.sin_addr = *((struct in_addr *)sv->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
    
	if (connect(sendsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        std::cerr << "ClientCommunicator_init(): error connecting to send socket\n";
		exit(-1);
	}

	if (connect(recvsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        std::cerr << "ClientCommunicator_init(): error connecting to recv socket\n";
		exit(-1);
	}
 
	std::cout << "ClientCommunicator_init(): END\n";
}

