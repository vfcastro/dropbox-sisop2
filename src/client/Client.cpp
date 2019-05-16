#include <unistd.h>
#include <iostream>
#include <cstring>
#include "../../include/client/ClientCommunicator.h"
#include "../../include/client/ClientInterface.h"
#include "../../include/client/ClientSync.h"

int main (int argc, char **argv)
{
	std::cout << "main(): START\n";
	std::string username = std::string(argv[1]);
	std::string server = std::string(argv[2]);
	unsigned int port = std::stoul(argv[3]);

	ClientCommunicator cc;
	ClientCommunicator_init(&cc,username,server,port);
	std::cout << "main(): ClientCommunicator_init() finished\n";
	ClientCommunicator_start(&cc);
	std::cout << "main(): ClientCommunicator_start() finished\n";
	ClientCommunicator_openSession(&cc);
	std::cout << "main(): ClientCommunicator_openSession() finished\n";

	ClientSync cs;
	ClientSync_init(&cs,&cc);
	std::cout << "main(): ClientSync_init() finished\n";
	ClientSync_get_sync_dir(&cs);
	std::cout << "main(): ClientSync_get_sync_dir() finished\n";


	ClientInterface_start(&cc);
	std::cout << "main(): ClientInterface_start() finished\n";
	std::cout << "main() END\n";

	return 0;
}