#include <unistd.h>
#include <iostream>
#include <cstring>
#include "../../include/client/ClientCommunicator.h"

int main (int argc, char **argv)
{
	std::cout << "main(): START\n";
	std::string username = std::string(argv[1]);
	std::string server = std::string(argv[2]);
	unsigned int port = std::stoul(argv[3]);

	ClientCommunicator cc;
	ClientCommunicator_init(&cc,username,server,port);
/*	std::cout << "main(): ServerCommunicator_init() finished\n";
	std::cout << "main(): ServerCommunicator_start() called\n";
	ServerCommunicator_start(&sc);
	std::cout << "main(): ServerCommunicator_start() finished\n";
	std::cout << "main() END\n";
*/	
	return 0;
}
