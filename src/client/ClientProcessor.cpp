#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "../../include/client/ClientProcessor.h"
#include "../../include/common/FileManager.h"


// Avalia qual o tipo de mensagem recebida e encaminha pra funcao adequada
void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg) {

	// std::cout << "ClientProcessor_dispatch(): START\n";

	switch(msg->type) {
      	case FILE_CLOSE_WRITE:
	        // std::cout << "ClientProcessor_dispatch(): recv FILE_CLOSE_WRITE from user " << msg->username << "\n";
	        ClientProcessor_onCloseWrite(cc,msg);
      	break;

		case DELETE_FILE:
	        // std::cout << "ClientProcessor_dispatch(): recv DELETE_FILE from user " << msg->username << "\n";
      		ClientProcessor_receiveDelete(cc, msg);
      	break;

      	case S2C_PROPAGATE:
            // std::cout << "SOU UM CLIENTE E RECEBI MENSAGEM" << msg->username << "\n";
            ClientProcessor_receivePropagate(cc, msg);
      	break;
	}

	// std::cout << "ClientProcessor_dispatch(): END\n";
}

void ClientProcessor_onCloseWrite(ClientCommunicator *cc, Message *msg) {
	// std::cout << "ClientProcessor_onCloseWrite(): recv FILE_CLOSE_WRITE from server " << msg->username << "\n";

	//primeira msg contem o nome do arquivo, cria caso necessario
	std::string path("./sync_dir_");
	path.append(msg->username).append("/");


	path.append(msg->payload);


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

	    // std::cout << "ClientProcessor_onCloseWrite(): recv payload with " << msg->seqn << " bytes\n";

	    if(write(f,(const void *)msg->payload, msg->seqn) == -1){
	        exit(6);
	    }

	}

	
}

void ClientProcessor_receivePropagate(ClientCommunicator *cc, Message *msg) {
   // std::cout << "ClientProcessor_receivePropagate(): recv FILE_CLOSE_WRITE from client " << msg->username << "\n";

   	std::string path("./sync_dir_");
   	path.append(cc->username).append("/");
   	path.append(msg->payload);
   	std::cout << "Recebendo arquivo: " << path << "\n"; 

   	pthread_mutex_lock(&cc->syncFilesLock);

	//Adiciona filename na lista de sicronizacao
	cc->syncFiles.insert(msg->payload);

   	// Começa o recebimento do arquivo
   	if(FileManager_receiveFile(path, msg, cc->recvsockfd) == -1){
      	std::cerr<<"ClientProcessor_receivePropagate(): Error Receive File\n";
   	}

	//Remove filename na lista de sync
	cc->syncFiles.erase(msg->payload);

   	pthread_mutex_unlock(&cc->syncFilesLock);
  }

void ClientProcessor_receiveDelete(ClientCommunicator *cc, Message *msg){
	std::string path("./sync_dir_");
   	path.append(cc->username).append("/");
   	path.append(msg->payload);

   	pthread_mutex_lock(&cc->syncFilesLock);
   	cc->syncFiles.insert(msg->payload);

   	// cout << path << "\n";

   	int Removed = std::remove(path.c_str());

   	cc->syncFiles.erase(msg->payload);
   	pthread_mutex_unlock(&cc->syncFilesLock);
}