#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "../../include/client/ClientProcessor.h"
#include "../../include/common/FileManager.h"

void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg) {

	std::cout << "ClientProcessor_dispatch(): START\n";

	switch(msg->type) {
	    case FILE_CLOSE_WRITE:
	          std::cout << "ClientProcessor_dispatch(): recv FILE_CLOSE_WRITE from user " << msg->username << "\n";
	          ClientProcessor_onCloseWrite(cc,msg);
	    break;
		case DELETE_FILE:
	          std::cout << "ClientProcessor_dispatch(): recv DELETE_FILE from user " << msg->username << "\n";
	    break;


	}

	std::cout << "ClientProcessor_dispatch(): END\n";
}

void ClientProcessor_onCloseWrite(ClientCommunicator *cc, Message *msg) {
   	std::cout << "ClientProcessor_onCloseWrite(): recv FILE_CLOSE_WRITE from server " << msg->username << "\n";


   	//primeira msg contem o nome do arquivo, cria caso necessario
   	std::string path("./sync_dir_");
   	path.append(msg->username).append("/");

   	// Verifica se pasta do usuário existe, se não existe, cria
   	if(FileManager_openDir((char*)path.c_str()) == -1) {
   	    if(FileManager_createDir((char*)path.c_str()) == -1) {
   	            std::cerr << "ClienteProcessor_onCloseWrite(): ERROR creating " << path << "\n";
   	            exit(-1);
   	    }
   	}

   	path.append(msg->payload);

   	std::cout << "ClientProcessor_onCloseWrite(): creating file " << path << "\n";

   	int f = open((char*)path.c_str(),O_CREAT|O_WRONLY,0600);

   	if(f == -1){
   	    std::cerr << "ClientProcessor_onCloseWrite(): ERROR creating file " << path << "\n";
   	    return;
   	}
   	//msg->type = OK;
   	//Message_send(msg,sockfd);

   	//Preenche o arquivo conforme recebimento das mensagens
   	while(Message_recv(msg,cc->recvsockfd) != -1) {
   	    // Verifica se tipo = OK, se sim, para de escrever
   	    if(msg->type == END){
   	        break;
   	    }

   	    std::cout << "ClientProcessor_onCloseWrite(): recv payload with " << msg->seqn << " bytes\n";

   	    if(write(f,(const void *)msg->payload, msg->seqn) == -1){
   	        exit(6);
   	    }

   	}

   	std::cout << "ClientProcessor_onCloseWrite(): END recv FILE_CLOSE_WRITE from client " << msg->username << "\n";

}
