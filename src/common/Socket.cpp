#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>                      
#include <ifaddrs.h>
#include <linux/if_link.h>
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

std::string Socket_getClientIP(int sockfd)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
	char str[INET_ADDRSTRLEN];
	getpeername(sockfd, (struct sockaddr *)&addr, &addr_size);
	strcpy(str, inet_ntoa(addr.sin_addr));
	
	return std::string(str);
}

std::string Socket_getServerIP()
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	/* Walk through linked list, maintaining head pointer so we
		can free list later */

	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		/* For an AF_INET* interface address, display the address */

		if (family == AF_INET ) {
			s = getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in) :
											sizeof(struct sockaddr_in6),
					host, NI_MAXHOST,
					NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
		}
	}

	freeifaddrs(ifaddr);
	return std::string(host);
}