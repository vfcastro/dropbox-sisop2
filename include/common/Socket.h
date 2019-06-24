#ifndef __SOCKET_H__
#define __SOCKET_H__

int Socket_openSocket(std::string server, unsigned int port);
std::string Socket_getClientIP(int sockfd);
std::string Socket_getServerIP();

#endif
