#include <iostream>
#include <string>
#include "../../include/server/ServerCommunicator.h"

int main (int argc, char **argv)
{
	std::cout << "main(): START\n";
	unsigned int port = std::stoul(argv[1]);
	unsigned int backlog = 200;

	ServerCommunicator sc;
	ServerCommunicator_init(&sc,port,backlog);
	std::cout << "main(): ServerCommunicator_init() finished\n";
	std::cout << "main(): ServerCommunicator_start() called\n";
	ServerCommunicator_start(&sc);
	std::cout << "main(): ServerCommunicator_start() finished\n";
	std::cout << "main() END\n";
	
	return 0;
}
