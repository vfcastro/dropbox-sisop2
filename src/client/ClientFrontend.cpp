#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>                     
#include "../../include/client/ClientFrontend.h"
#include "../../include/common/Socket.h"
#include "../../include/common/Message.h"

void ClientFrontend_init(ClientCommunicator *cc, std::string server, unsigned int port, unsigned int frontend_port) {
   
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
    cc->frontend_port = frontend_port;

    pthread_create(&cc->frontEndThread,0,ClientFrontend_receive,(void*)cc);

}

void* ClientFrontend_receive(void* cc)
{
	ClientCommunicator *c = (ClientCommunicator*)cc;

	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    	std::cerr << "ClientFrontend_receive(): ERROR opening socket\n";
    	exit(-1);
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	    std::cerr << "ClientFrontend_receive(): setsockopt(SO_REUSEADDR) failed\n";
	    exit(-1);
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(c->frontend_port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "ClientFrontend_receive(): ERROR on binding\n";
		exit(-1);
	}

    listen(sockfd, 200);
	std::cout << "ClientFrontend_receive(): WAITING for conections\n";
	
	socklen_t clilen = sizeof(struct sockaddr_in);
	struct sockaddr *connection = NULL;
	int *newsockfd = NULL;

	while(1) {
		connection = (struct sockaddr *)malloc(clilen);
		newsockfd = (int*)malloc(sizeof(int));       
        Message *msg = (Message*)malloc(sizeof(Message));
		Message *new_msg;
		
		if ((*newsockfd = accept(sockfd, connection, &clilen)) == -1) {
			std::cerr << "ClientFrontend_receive(): ERROR on accept\n";
		}
		else {
		    std::cout << "ClientFrontend_receive: SERVER IP: " << Socket_getClientIP(*newsockfd) << "\n";
	
            if(Message_recv(msg,*newsockfd) != -1)
            {
                if(msg->type == FRONTEND_NEW_SERVER)
                {
					pthread_mutex_lock(&c->sockfdLock);
					
					close(c->sendsockfd);
					c->sendsockfd = Socket_openSocket(msg->payload,msg->seqn);
					new_msg = Message_create(FRONTEND_OPEN_SEND_CONN,c->connectionId,std::string(c->username).c_str(),std::string().c_str());
					Message_send(new_msg,c->sendsockfd);
					std::cout << "ClientFrontend_receive: sent FRONTEND_OPEN_SEND_CONN to " << Socket_getClientIP(*newsockfd) << ":" << msg->seqn << "\n";
					Message_recv(new_msg,c->sendsockfd);
					free(new_msg);

					close(c->recvsockfd);
					c->recvsockfd = Socket_openSocket(msg->payload,msg->seqn);
					new_msg = Message_create(FRONTEND_OPEN_RECV_CONN,c->connectionId,std::string(c->username).c_str(),std::string().c_str());
					Message_send(new_msg,c->recvsockfd);
					std::cout << "ClientFrontend_receive: sent FRONTEND_RECV_SEND_CONN to " << Socket_getClientIP(*newsockfd) << ":" << msg->seqn << "\n";
					Message_recv(new_msg,c->recvsockfd);
					free(new_msg);

					pthread_mutex_unlock(&c->sockfdLock);
                }
            }

		}
    
        free(msg);
		free(connection);
		free(newsockfd);
	}


}