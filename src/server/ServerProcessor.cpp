#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "../../include/server/ServerProcessor.h"
#include "../../include/common/FileManager.h"


void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg) {
	std::cout << "ServerProcessor_dispatch(): START\n";
	switch(msg->type) {
		case OPEN_SESSION:
			ServerProcessor_openSession(sc,msg);
		break;

		case FILE_CLOSE_WRITE:
			ServerProcessor_onCloseWrite(sc,msg);
		break;
		case DELETE_FILE:
			ServerProcessor_onDelete(sc,msg);
		break;
		default:
			std::cout<<"ERROR MSG TYPE INVALID"<<std::endl;
			break;

	}


	std::cout << "ServerProcessor_dispatch(): END\n";
}

void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg)  {
	std::cout << "ServerProcessor_openSession(): recv OPEN_SESSION from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;

	//TODO: checar numero de sessoes!



	//Checar se existe sync_dir_<username>
	//Se não houve, criar
	std::string sync_dir(SYNC_DIR_BASE_NAME);
	sync_dir.append(msg->username);
	if(FileManager_createDir((char*)sync_dir.c_str()) == -1) {
		std::cout<<"ServerProcessor_openSession(): ERROR creating user dir " << sync_dir << "\n";
		msg->type = NOK;
		Message_send(msg,sockfd);
		return;
	}

	msg->type = OK;
	if(Message_send(msg,sockfd) == -1) {
		std::cerr << "ServerProcessor_openSession(): ERROR sending OK for OPEN_SESSION to client " << msg->username << "\n";
		return;
	}


	std::cout << "ServerProcessor_openSession(): sent OK for OPEN_SESSION to client " << msg->username << "\n";
}

void ServerProcessor_onCloseWrite(ServerCommunicator *sc, Message *msg) {
	std::cout << "ServerProcessor_onCloseWrite(): recv FILE_CLOSE_WRITE from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;

	//primeira msg contem o nome do arquivo, cria caso necessario

	std::string path("./sync_dir_server/");
	path.append(msg->username).append("/");

	// Verifica se pasta do usuário existe, se não existe, cria
	if(FileManager_openDir((char*)path.c_str()) == -1) {
		if(FileManager_createDir((char*)path.c_str()) == -1) {
				std::cerr << "Server(): ERROR creating " << path << "\n";
				exit(-1);
		}
	}

	path.append(msg->payload);

	std::cout << "ServerProcessor_onCloseWrite(): creating file " << path << "\n";

	int f = open((char*)path.c_str(),O_CREAT|O_WRONLY,0600);

	if(f == -1){
		std::cerr << "ServerProcessor_onCloseWrite(): ERROR creating file " << path << "\n";
		return;
	}

	msg->type = OK;
	Message_send(msg,sockfd);

	//Preenche o arquivo conforme recebimento das mensagens
	while(Message_recv(msg,sockfd) != -1) {
		// Verifica se tipo = OK, se sim, para de escrever
		if(msg->type == END){
			break;
		}

		std::string payload(msg->payload);

		int size_payload = MAX_PAYLOAD_SIZE;
		if(msg->seqn == 0){
			size_payload = ServerProcessor_PayloadSize(msg->payload);

		}

		std::cout << "ServerProcessor_onCloseWrite(): recv payload " << payload << "with " << size_payload << " bytes\n";
		if(write(f,(const void *)msg->payload, size_payload) == -1){
			exit(6);
		}
		msg->type = OK;
	}

	//Recupera o connectionId desta conexao
	int connectionId = sc->threadConnId.find((pthread_self()))->second;
	//Posta msg na fila de envio do respectivo connectionId
	Message *m = (Message*)malloc(sizeof(Message));
	m->type = FILE_CLOSE_WRITE;
	strcpy(m->username,msg->username);
	sc->sendQueue.at(connectionId).push(m);



	close(f);

	std::cout << "ServerProcessor_onCloseWrite(): END recv FILE_CLOSE_WRITE from client " << msg->username << "\n";
}

void ServerProcessor_onDelete(ServerCommunicator *sc, Message *msg){
	std::cout << "ServerProcessor_onDelete(): recv DELETE_FILE from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;

	//primeira msg contem o nome do arquivo, cria caso necessario

	std::string path("./sync_dir_server/");
	path.append(msg->username).append("/");

	// Verifica se pasta do usuário existe, se não existe, cria
	if(FileManager_openDir((char*)path.c_str()) == -1) {
		if(FileManager_createDir((char*)path.c_str()) == -1) {
				std::cerr << "Server(): ERROR creating " << path << "\n";
				exit(-1);
		}
	}

	path.append(msg->payload);

	std::cout << "ServerProcessor_onDelete(): removing file " << path << "\n";

	const char *c = path.c_str();

    int Removed=std::remove(c);


	if(Removed==0){
		std::cout<<"File "<<path <<"removed from "<<msg->username<<"'s"<<" sync_dir"<<std::endl;
	}
	else{
		std::cout<<"File "<<path <<"wasn't inside "<<msg->username<<"'s"<<" sync_dir"<<std::endl;
	}


	msg->type = OK;
	Message_send(msg,sockfd);

	//Recupera o connectionId desta conexao
	int connectionId = sc->threadConnId.find((pthread_self()))->second;
	//Posta msg na fila de envio do respectivo connectionId
	Message *m = (Message*)malloc(sizeof(Message));
	m->type = DELETE_FILE;
	strcpy(m->username,msg->username);
	sc->sendQueue.at(connectionId).push(m);


	std::cout << "ServerProcessor_onDelete(): END recv DELETE_FILE from client " << msg->username << "\n";

}

// Funcao nova
int ServerProcessor_PayloadSize(char *payload){
	int size = 0;

	for (int i = 0; i < MAX_PAYLOAD_SIZE; ++i){
		if(payload[i] != '\0'){
			size++;
		}
	}

	return size;
}
