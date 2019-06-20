#include <iostream>
#include <string>
#include <vector>
#include "../../include/common/FileManager.h"
#include "../../include/server/ServerCommunicator.h"
#include "../../include/server/ReplicaManager.h"

int main (int argc, char **argv)
{	
	system("clear");
	// std::cout << "main(): START\n";
	std::cout << "Dropbox Server Starting...\n";
	
	// processando argumentos
	unsigned int port = std::stoul(argv[1]);
	int primary = std::stoi(argv[2]);

	std::vector<string> hosts_and_ports;
	for(char **arg = argv+3; *arg != NULL; arg++)
		hosts_and_ports.push_back(*arg);

	unsigned int backlog = 200;

	ServerCommunicator sc;
	ReplicaManager rm;
	ServerCommunicator_init(&sc,&rm,port,backlog);
	ReplicaManager_init(&rm,&sc,primary,hosts_and_ports);

	// std::cout << "main(): ServerCommunicator_init() finished\n";
	// std::cout << "main(): ServerCommunicator_start() called\n";
	ServerCommunicator_start(&sc);
		
	// std::cout << "main(): ServerCommunicator_start() finished\n";
	// std::cout << "main() END\n";
	return 0;
}
