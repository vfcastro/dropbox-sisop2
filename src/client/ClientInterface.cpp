#include <iostream>
#include "../../include/client/ClientInterface.h"

void ClientInterface_start(ClientCommunicator *cc) {
	std::cout << "ClientInterface_start(): START\n";
	bool exit = false;
	std::string command;

	while(!exit){
		std::cout << "$";
		std::cin >> command;
	}


	std::cout << "ClientInterface_start(): STOP\n";
}
