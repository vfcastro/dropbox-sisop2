#include <unistd.h>
#include <iostream>
#include <cstring>
#include "../../include/client/ClientCommunicator.h"
#include "../../include/client/ClientInterface.h"
#include "../../include/client/ClientSync.h"
#include "../../include/client/ClientFrontend.h"

int main (int argc, char **argv)
{	
	system("clear");
	std::cout << "Dropbox Client Starting...\n";
	std::string username = std::string(argv[1]);
	std::string server = std::string(argv[2]);
	unsigned int port = std::stoul(argv[3]);
	unsigned int frontend_port = std::stoul(argv[4]);

	// Inicia os módulos de conexão

	ClientCommunicator cc;
	ClientFrontend_init(&cc,server,port,frontend_port);
	ClientCommunicator_init(&cc,username);
	ClientCommunicator_start(&cc);
	ClientCommunicator_openSession(&cc);

	// Inicia os módulos de monitoramento de pasta
	ClientSync cs;
	ClientSync_init(&cs,&cc);
	ClientSync_get_sync_dir(&cs);


	//sleep só pra esperar o print do inotify vir antes da interface
	sleep(2);

	// Inicia módulo de tratamento de comandos do usuario
	ClientInterface_start(&cc);

	return 0;
}