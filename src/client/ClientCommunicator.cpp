#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                      
#include "../../include/common/Message.h"
#include "../../include/client/ClientCommunicator.h"
#include "../../include/client/ClientProcessor.h"

void ClientCommunicator_init(ClientCommunicator *cc, std::string username, std::string server, unsigned int port) {
	std::cout << "ClientCommunicator_init(): START\n";

	if(username.size() > MAX_USERNAME_SIZE) {
		std::cerr << "ClientCommunicator_init(): username too long. Max" << MAX_USERNAME_SIZE << "characters\n";
		exit(-1);
	}
		
	memcpy((void*)cc->username,(void*)std::string(username).c_str(),MAX_USERNAME_SIZE);
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

	int recvsockfd; 
    if ((recvsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "ClientCommunicator_init(): ERROR opening recv socket\n";
		exit(-1);
	}

	int enable = 1;
	if (setsockopt(sendsockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	    std::cout << "ClientCommunicator_init(): setsockopt(SO_REUSEADDR) failed\n";
	    exit(-1);
	}
 
 	if (setsockopt(recvsockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
 	    std::cout << "ClientCommunicator_init(): setsockopt(SO_REUSEADDR) failed\n";
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

	cc->sendsockfd = sendsockfd;
	cc->recvsockfd = recvsockfd;

	// Solicita abertura de conexao de envio
	Message *msg = Message_create(OPEN_SEND_CONN,0,std::string(cc->username).c_str(),std::string().c_str(),0);
	if(Message_send(msg,cc->sendsockfd) != -1) {
		std::cout << "ClientCommunicator_init(): sent msg OPEN_SEND_CONN\n";
	    if(Message_recv(msg,cc->sendsockfd) != -1) { 
	    	if(msg->type == OK) {
				std::cout << "ClientCommunicator_init(): recv OK for OPEN_SEND_CONN\n";
			}
			else {
				std::cerr << "ClientCommunicator_init(): ERROR recv OK for OPEN_SEND_CONN\n";
				exit(-1);
			}
		}
		else {
			std::cerr <<  "ClientCommunicator_init(): ERROR recv OK for OPEN_SEND_CONN\n";
			exit(-1);
		}
	}
	else {
		std::cout << "ClientCommunicator_init(): ERROR sent msg OPEN_SEND_CONN\n";
		exit(-1);
	}
	//free(msg);

	// Solicita abertura de conexao de recebimento passando conectionId recebido da OPEN_SEND_CONN no campo seqn
	int connectionId = msg->seqn;
	msg = Message_create(OPEN_RECV_CONN,connectionId,std::string(cc->username).c_str(),std::string().c_str(),0);
	if(Message_send(msg,cc->recvsockfd) != -1) {
		std::cout << "ClientCommunicator_init(): sent msg OPEN_RECV_CONN\n";
	    if(Message_recv(msg,cc->recvsockfd) != -1) { 
	    	if(msg->type == OK)
				std::cout << "ClientCommunicator_init(): recv OK for OPEN_RECV_CONN\n";
			else {
				std::cerr << "ClientCommunicator_init(): ERROR recv OK for OPEN_RECV_CONN\n";
				exit(-1);
			}
		}
		else {
			std::cerr <<  "ClientCommunicator_init(): ERROR recv OK for OPEN_RECV_CONN\n";
			exit(-1);
		}
	}
	else {
		std::cout << "ClientCommunicator_init(): ERROR sent msg OPEN_RECV_CONN\n";
		exit(-1);
	}

	//free(msg);
	std::cout << "ClientCommunicator_init(): send and recv sockets connected to server\n";
	std::cout << "ClientCommunicator_init(): END\n";
}


void ClientCommunicator_start(ClientCommunicator *cc) {
	std::cout << "ClientCommunicator_start(): START\n";

	// start receive thread
	pthread_create(&(cc->recvThread),0,ClientCommunicator_receive,(void*)cc);


	std::cout << "ClientCommunicator_start(): END\n";
}

void* ClientCommunicator_receive(void *cc) {
	ClientCommunicator *c = (ClientCommunicator*)cc; 
	std::cout << "ClientCommunicator_receive(): STARTED thread " << pthread_self() << "\n";
	std::cout << "ClientCommunicator_receive(): WAITING for msg on fd " << c->recvsockfd << "\n";

    Message *msg = (Message*)malloc(sizeof(Message));
    while(Message_recv(msg,c->recvsockfd) != -1) {
        ClientProcessor_dispatch(c,msg);
    }

    free(msg);
	std::cout << "ClientCommunicator_receive(): END receive for msg on fd " << c->recvsockfd << "\n";
}

void ClientCommunicator_openSession(ClientCommunicator *cc) {
	std::cout << "ClientCommunicator_openSession(): START\n";

	Message *msg = Message_create(OPEN_SESSION,0,std::string(cc->username).c_str(),std::string().c_str(),0);
	if(Message_send(msg,cc->sendsockfd) == -1){
		std::cerr << "ClientCommunicator_openSession(): ERROR sending OPEN_SESSION\n";
		exit(-1);
	}
	std::cout << "ClientCommunicator_openSession(): sent msg OPEN_SESSION\n";

	if(Message_recv(msg,cc->sendsockfd) == -1) {
		std::cerr << "ClientCommunicator_openSession(): ERROR recv OK for OPEN_SESSION\n";
		exit(-1);
	}

	if(msg->type == NOK) {
		std::cerr << "ClientCommunicator_openSession(): ERROR recv NOK for OPEN_SESSION\n";	
    	exit(-1);
    }

	std::cout << "ClientCommunicator_openSession(): recv OK for OPEN_SESSION\n";

	std::cout << "ClientCommunicator_openSession(): END\n";
}













