#include <iostream>
#include "../../include/server/ServerProcessor.h"

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg) {
	std::cout << "ServerProcessor_dispatch(): START\n";
	
	switch(msg->type) {
		case OPEN_SESSION:
			  std::cout << "ServerProcessor_dispatch(): recv OPEN_SESSION from client " << msg->username << "\n";
		break;


	}


	
	std::cout << "ServerProcessor_dispatch(): END\n";
}
