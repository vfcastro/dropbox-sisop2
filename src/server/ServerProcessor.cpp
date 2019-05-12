#include <iostream>
#include <unistd.h>
#include "../../include/server/ServerProcessor.h"

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg) {
	std::cout << "ServerProcessor_dispatch(): START\n";
	switch(msg->type) {
		case OPEN_SESSION:
			ServerProcessor_openSession(sc,msg);
		break;


	}


	
	std::cout << "ServerProcessor_dispatch(): END\n";
}

void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg)  {
	std::cout << "ServerProcessor_openSession(): recv OPEN_SESSION from client " << msg->username << "\n";

	//TODO
	//Checar se existe sync_dir_<username>
	//Se não houve, criar

	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	msg->type = OK;
	if(Message_send(msg,sockfd) == -1)
		std::cerr << "ServerProcessor_openSession(): ERROR sending OK for OPEN_SESSION to client " << msg->username << "\n";
	std::cout << "ServerProcessor_openSession(): sent OK for OPEN_SESSION to client " << msg->username << "\n";

}
