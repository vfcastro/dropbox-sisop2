#include <iostream>
#include <unistd.h>
#include "../../include/server/ServerProcessor.h"
#include "../../include/common/FileManager.h"

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
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	//TODO
	//Checar se existe sync_dir_<username>
	//Se não houve, criar
	std::string sync_dir("./sync_dir_");
	sync_dir.append(msg->username);
	if(FileManager_createDir((char*)sync_dir.c_str()) == -1) {
		std::cout<<"ServerProcessor_openSession(): ERROR creating user dir " << sync_dir << "\n";
		msg->type = NOK;
		Message_send(msg,sockfd);
		return;
	}

	msg->type = OK;
	if(Message_send(msg,sockfd) == -1) {
		std::cerr << "ServerProcessor_openSession(): ERROR sending OK for OPEN_SESSION to client " << msg->username << "\n";
		return;
	}

	std::cout << "ServerProcessor_openSession(): sent OK for OPEN_SESSION to client " << msg->username << "\n";
}
