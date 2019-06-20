#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                      
#include "../../include/client/ClientFrontend.h"
#include "../../include/common/Socket.h"

void ClientFrontend_init(ClientCommunicator *cc, std::string server, unsigned int port) {
   
    int sendsockfd = Socket_openSocket(server,port);
    if(sendsockfd == -1)
        exit(-1);

    int recvsockfd = Socket_openSocket(server,port);
    if(sendsockfd == -1)
        exit(-1);

	cc->server = std::string(server);
	cc->port = port;
	cc->sendsockfd = sendsockfd;
	cc->recvsockfd = recvsockfd;

}

void ClientFrontend_start(ClientCommunicator *cc);