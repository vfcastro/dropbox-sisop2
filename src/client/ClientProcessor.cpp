#include <iostream>
#include "../../include/client/ClientProcessor.h"

void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg) {

	std::cout << "ClientProcessor_dispatch(): START\n";
	
	switch(msg->type) {
	    case CREATE_FILE:
	          std::cout << "ClientProcessor_dispatch(): recv CREATE_FILE from server " << msg->username << "\n";
	    break;
	
	
	}
	
	std::cout << "ClientProcessor_dispatch(): END\n";
}
