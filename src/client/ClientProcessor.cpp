#include <iostream>
#include "../../include/client/ClientProcessor.h"

void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg) {

	std::cout << "ClientProcessor_dispatch(): START\n";

	switch(msg->type) {
	    case FILE_CLOSE_WRITE:
	          std::cout << "ClientProcessor_dispatch(): recv FILE_CLOSE_WRITE from user " << msg->username << "\n";
	    break;
		case DELETE_FILE:
	          std::cout << "ClientProcessor_dispatch(): recv DELETE_FILE from user " << msg->username << "\n";
	    break;


	}

	std::cout << "ClientProcessor_dispatch(): END\n";
}
