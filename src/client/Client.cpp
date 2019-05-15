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

	//Init Communicator
	ClientCommunicator cc;
	ClientCommunicator_init(&cc,username,server,port);
	std::cout << "main(): ClientCommunicator_init() finished\n";

	//Inicia ClientSync
	ClientSync cs;
	ClientSync_init(&cs,&cc);

	//Abre thread do client para receber do server
	ClientCommunicator_start(&cc);
	std::cout << "main(): ClientCommunicator_start() finished\n";

	//Abre sessão com server
	ClientCommunicator_openSession(&cc);

	//Baixa sync_dir se necessario e dispara thread de watch do sync_dir
	ClientSync_get_sync_dir(&cs);


	ClientInterface_start(&cc);
	std::cout << "main(): ClientInterface_start() finished\n";
	std::cout << "main() END\n";

	return 0;
}
