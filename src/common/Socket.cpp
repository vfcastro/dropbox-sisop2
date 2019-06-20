#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                      
#include "../../include/common/Socket.h"

int Socket_openSocket(std::string server, unsigned int port) {
	
	struct hostent *sv = gethostbyname(server.c_str());

	if (sv == NULL) {
        std::cerr << "Socket_openSocket(): ERROR, no such host\n";
        return -1;
    }

	int sockfd; 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "Socket_openSocket(): ERROR opening send socket\n";
		return -1;
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	    std::cout << "Socket_openSocket(): setsockopt(SO_REUSEADDR) failed\n";
	    return -1;
	}

	struct sockaddr_in serv_addr; 
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)sv->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        std::cerr << "Socket_openSocket(): error connecting to send socket\n";
		return -1;
	}

    return sockfd;
}