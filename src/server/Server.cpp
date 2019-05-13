#include <iostream>
#include <string>
#include "../../include/common/FileManager.h"
#include "../../include/server/ServerCommunicator.h"

int main (int argc, char **argv)
{
	std::cout << "main(): START\n";
	unsigned int port = std::stoul(argv[1]);
	unsigned int backlog = 200;

	std::string path("./sync_dir_server/");
	
	// Verifica se existe a pasta "sync_dir_server", se nao tiver, cria
	if(FileManager_openDir((char*)path.c_str()) == -1) {
		if(FileManager_createDir((char*)path.c_str()) == -1) {
			std::cerr << "Server(): ERROR creating " << path << "\n";
			exit(-1);
		}
	}

	ServerCommunicator sc;
	ServerCommunicator_init(&sc,port,backlog);
	std::cout << "main(): ServerCommunicator_init() finished\n";
	std::cout << "main(): ServerCommunicator_start() called\n";
	ServerCommunicator_start(&sc);
	std::cout << "main(): ServerCommunicator_start() finished\n";
	std::cout << "main() END\n";
	
	return 0;
}
