#include <iostream>
#include <sstream>
#include <string>
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
		
		case UPLOAD_FILE_CMD:
			ServerProcessor_uploadCommand(sc,msg);
		break;

		case DOWNLOAD_FILE_CMD:
			ServerProcessor_downloadCommand(sc,msg);
		break;

		case LIST_SERVER_CMD:
			ServerProcessor_listServerCommand(sc,msg);
		break;
		
		default:
			std::cout<<"ERROR MSG TYPE INVALID"<<std::endl;
			break;

	}


	std::cout << "ServerProcessor_dispatch(): END\n";
}

void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg)  {
	std::cout << "ServerProcessor_openSession(): recv OPEN_SESSION from client " << msg->username << "\n";
	
	pthread_mutex_lock(&sc->acceptedThreadsLock);	
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	pthread_mutex_unlock(&sc->acceptedThreadsLock);


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

	pthread_mutex_lock(&sc->acceptedThreadsLock);
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	pthread_mutex_unlock(&sc->acceptedThreadsLock);

	//Recupera o connectionId desta conexao
	int connectionId = sc->threadConnId.find((pthread_self()))->second;	
	
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


	//QUEUE: posta primeira msg na fila de sincronizacao
	sc->sendQueue.at(connectionId).push(msg);

	msg->type = OK;
	Message_send(msg,sockfd);

	//Preenche o arquivo conforme recebimento das mensagens
	while(Message_recv(msg,sockfd) != -1) {
		// Verifica se tipo = OK, se sim, para de escrever
		if(msg->type == END){
			break;
		}

		std::cout << "ServerProcessor_onCloseWrite(): recv payload with " << msg->seqn << " bytes\n";
		
		if(write(f,(const void *)msg->payload, msg->seqn) == -1){
			exit(6);
		}


		//QUEUE: posta msg na fila de sync
		sc->sendQueue.at(connectionId).push(msg);	

	}

	close(f);

	std::cout << "ServerProcessor_onCloseWrite(): END recv FILE_CLOSE_WRITE from client " << msg->username << "\n";
}

void ServerProcessor_uploadCommand(ServerCommunicator *sc, Message *msg){
	std::cout << "ServerProcessor_uploadCommand(): recv UPLOAD_FILE_CMD from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
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

	std::cout << "ServerProcessor_uploadCommand(): creating file " << path << "\n";

	int f = open((char*)path.c_str(),O_CREAT|O_WRONLY,0600);
	
	if(f == -1){
		std::cerr << "ServerProcessor_uploadCommand(): ERROR creating file " << path << "\n";
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

		std::cout << "ServerProcessor_uploadCommand(): recv payload with " << msg->seqn << " bytes\n";
		
		if(write(f,(const void *)msg->payload, msg->seqn) == -1){
			exit(6);
		}
		
		msg->type = OK;
	}

	//Recupera o connectionId desta conexao
	int connectionId = sc->threadConnId.find((pthread_self()))->second;
	
	//Checa para qual conexao enviar a msg
	int idtonotify = 0;
	if(sc->userSessions.at(msg->username).first == connectionId) {
		if(sc->userSessions.at(msg->username).second != 0)
			idtonotify = sc->userSessions.at(msg->username).second;
	}
	else
		idtonotify = sc->userSessions.at(msg->username).first;


	if(idtonotify != 0) {
		//Posta msg na fila de envio do respectivo connectionId
		Message *m = (Message*)malloc(sizeof(Message));
		m->type = FILE_CLOSE_WRITE;
		strcpy(m->username,msg->username);
		sc->sendQueue.at(idtonotify).push(m);
	}
	//Posta msg na fila de envio do respectivo connectionId
	Message *m = (Message*)malloc(sizeof(Message));
	m->type = UPLOAD_FILE_CMD;
	strcpy(m->username,msg->username);
	sc->sendQueue.at(connectionId).push(m);

	close(f);

	std::cout << "ServerProcessor_uploadCommand(): END recv UPLOAD_FILE_CMD from client " << msg->username << "\n";
}

void ServerProcessor_downloadCommand(ServerCommunicator *sc, Message *msg) {
	std::cout << "ServerProcessor_downloadCommand(): recv DOWNLOAD_FILE_CMD from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
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

	int f;

	if((f = open(path.c_str(), O_RDONLY)) == -1){
		std::cerr << "ServerProcessor_downloadCommand(): ERROR opening file " << path << "\n";
		return;
	}

	msg->type = OK;
	
	Message_send(msg, sockfd);

	int bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	
	// Envia arquivo pro cliente
	while(bytes_recv){
		std::cout << "ServerProcessor_downloadCommand(): read " << bytes_recv << " bytes from file " << path << "\n";
		msg->seqn = bytes_recv;
		Message_send(msg, sockfd);
		bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	}

	msg->type = END;
	Message_send(msg, sockfd);

	close(f);

	std::cout << "ServerProcessor_downloadCommand(): END recv DOWNLOAD_FILE_CMD from client " << msg->username << "\n";
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

void ServerProcessor_deleteCommand(ServerCommunicator *sc, Message *msg){
	std::cout << "ServerProcessor_deleteCommand(): START";
}

void ServerProcessor_listServerCommand(ServerCommunicator *sc, Message *msg){
	std::cout << "ServerProcessor_listServerCommand(): START";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	std::string path("./sync_dir_server/");
	path.append(msg->username).append("/");

	// Verifica se pasta do usuário existe, se não existe, cria
	if(FileManager_openDir((char*)path.c_str()) == -1) {
		if(FileManager_createDir((char*)path.c_str()) == -1) {
				std::cerr << "Server(): ERROR creating " << path << "\n";
				exit(-1);
		}
	}

	FILE *fp;
	std::string cmd("ls -l ");
	cmd.append(path);

  	// Executa comando ls
	fp = popen(cmd.c_str(), "r");
	if (fp == NULL) {
	    printf("Failed to run command\n" );
	    exit(1);
	}

	int count = 0;
	
	// Envia OK para o cliente
	msg->type = OK;
	Message_send(msg, sockfd);

	msg->seqn = MAX_PAYLOAD_SIZE;

	while (fgets(msg->payload, MAX_PAYLOAD_SIZE-1, fp) != NULL) {
	  	if(count == 0){
			count = 1;
			continue;
		}

		Message_send(msg, sockfd);
	}

	msg->type = END;
	Message_send(msg, sockfd);

	pclose(fp);

}

void ServerProcessor_exitCommand(ServerCommunicator *sc, Message *msg){
	std::cout << "ServerProcessor_exitCommand(): START";
}

