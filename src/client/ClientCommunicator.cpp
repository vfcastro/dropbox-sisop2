#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>                      
#include "../../include/common/Message.h"
#include "../../include/client/ClientCommunicator.h"
#include "../../include/client/ClientProcessor.h"
#include "../../include/client/ClientSync.h"

void ClientCommunicator_init(ClientCommunicator *cc, std::string username) {
	// std::cout << "ClientCommunicator_init(): START\n";

	//cc->pauseSync = 0;
	pthread_mutex_init(&cc->syncFilesLock, NULL);
	pthread_mutex_init(&cc->sockfdLock,NULL);

	if(username.size() > MAX_USERNAME_SIZE) {
		std::cerr << "ClientCommunicator_init(): username too long. Max" << MAX_USERNAME_SIZE << "characters\n";
		exit(-1);
	}		
	memcpy((void*)cc->username,(void*)std::string(username).c_str(),MAX_USERNAME_SIZE);

	// Solicita abertura de conexao de envio
	Message *msg = Message_create(OPEN_SEND_CONN,0,std::string(cc->username).c_str(),std::string().c_str());
	if(Message_send(msg,ClientCommunicator_getSendSocket(cc)) != -1) {
		// std::cout << "ClientCommunicator_init(): sent msg OPEN_SEND_CONN\n";
	    if(Message_recv(msg,ClientCommunicator_getSendSocket(cc)) != -1) { 
	    	if(msg->type == OK) {
	    		std::cout << "";
				// std::cout << "ClientCommunicator_init(): recv OK for OPEN_SEND_CONN\n";
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
	// std::cout << "ClientCommunicator_init(): CONNECTION ID " << connectionId << "\n";
	msg = Message_create(OPEN_RECV_CONN,connectionId,std::string(cc->username).c_str(),std::string().c_str());
	if(Message_send(msg,ClientCommunicator_getRecvSocket(cc)) != -1) {
		// std::cout << "ClientCommunicator_init(): sent msg OPEN_RECV_CONN\n";
	    if(Message_recv(msg,cc->recvsockfd) != -1) { 
	    	if(msg->type == OK){
				std::cout << "";
	    	}
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
		std::cerr << "ClientCommunicator_init(): ERROR sent msg OPEN_RECV_CONN\n";
		exit(-1);
	}

	//free(msg);
	// std::cout << "ClientCommunicator_init(): send and recv sockets connected to server\n";
	// std::cout << "ClientCommunicator_init(): END\n";
}


void ClientCommunicator_start(ClientCommunicator *cc) {
	// std::cout << "ClientCommunicator_start(): START\n";

	// start receive thread
	pthread_create(&(cc->recvThread),0,ClientCommunicator_receive,(void*)cc);


	// std::cout << "ClientCommunicator_start(): END\n";
}

void* ClientCommunicator_receive(void *cc) {
	ClientCommunicator *c = (ClientCommunicator*)cc; 
	// std::cout << "ClientCommunicator_receive(): STARTED thread " << pthread_self() << "\n";
	// std::cout << "ClientCommunicator_receive(): WAITING for msg on fd " << c->recvsockfd << "\n";

    Message *msg;
    
    if((msg = (Message*)malloc(sizeof(Message))) == NULL){
    	std::cerr << "ClientCommunicator_receive(): ERROR malloc msg\n";
    	pthread_exit(NULL);
    }

    while(Message_recv(msg, ClientCommunicator_getRecvSocket(c)) != -1) {
        ClientProcessor_dispatch(c,msg);
    }

    std::cerr << "ClientCommunicator_receive(): Fechando\n";

    free(msg);
	exit(0);
	// std::cout << "ClientCommunicator_receive(): END receive for msg on fd " << c->recvsockfd << "\n";
}

void ClientCommunicator_openSession(ClientCommunicator *cc) {
	// std::cout << "ClientCommunicator_openSession(): START\n";

	Message *msg = Message_create(OPEN_SESSION,0,std::string(cc->username).c_str(),std::string().c_str());
	if(Message_send(msg,ClientCommunicator_getSendSocket(cc)) == -1){
		std::cerr << "ClientCommunicator_openSession(): ERROR sending OPEN_SESSION\n";
		exit(-1);
	}
	// std::cout << "ClientCommunicator_openSession(): sent msg OPEN_SESSION\n";

	if(Message_recv(msg,ClientCommunicator_getSendSocket(cc)) == -1) {
		std::cerr << "ClientCommunicator_openSession(): ERROR recv OK for OPEN_SESSION\n";
		exit(-1);
	}

	if(msg->type == NOK) {
		std::cerr << "ClientCommunicator_openSession(): ERROR recv NOK for OPEN_SESSION\n";	
    	exit(-1);
    }

	// std::cout << "ClientCommunicator_openSession(): recv OK for OPEN_SESSION\n";

	// std::cout << "ClientCommunicator_openSession(): END\n";
}

int ClientCommunicator_getSendSocket(ClientCommunicator *cc) {
	pthread_mutex_lock(&cc->sockfdLock);

	int sendsockfd = cc->sendsockfd;

	pthread_mutex_unlock(&cc->sockfdLock);

	return sendsockfd;
}

int ClientCommunicator_getRecvSocket(ClientCommunicator *cc) {
	pthread_mutex_lock(&cc->sockfdLock);

	int recvsockfd = cc->recvsockfd;

	pthread_mutex_unlock(&cc->sockfdLock);

	return recvsockfd;
}












