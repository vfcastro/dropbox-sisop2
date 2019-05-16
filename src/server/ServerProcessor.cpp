#include <iostream>
#include <dirent.h>
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
	// std::cout << "ServerProcessor_dispatch(): START\n";
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
		
		case GET_SYNC_DIR:
			ServerProcessor_getSync(sc,msg);
		break;

		case USER_EXIT:
			ServerProcessor_exitCommand(sc,msg);
		break;

		case OK:
			std:cout << "";
		break;
		
		default:
			std::cout<<"ERROR MSG TYPE INVALID: " << msg->type << "\n";
		break;

	}

	// std::cout << "ServerProcessor_dispatch(): END\n";
}

void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg)  {
	// std::cout << "ServerProcessor_openSession(): recv OPEN_SESSION from client " << msg->username << "\n";
	
	pthread_mutex_lock(&sc->acceptedThreadsLock);	
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	pthread_mutex_unlock(&sc->acceptedThreadsLock);


	//Checar se existe sync_dir_<username>
	//Se não houve, criar
	std::string sync_dir(SYNC_DIR_BASE_NAME);
	sync_dir.append(msg->username);
	std::cerr<<"New Session for: " << msg->username << "\n";

	if(FileManager_createDir((char*)sync_dir.c_str()) == -1) {
		std::cerr<<"ServerProcessor_openSession(): ERROR creating user dir " << sync_dir << "\n";
		msg->type = NOK;
		Message_send(msg,sockfd);
		return;
	}

	msg->type = OK;
	if(Message_send(msg,sockfd) == -1) {
		std::cerr << "ServerProcessor_openSession(): ERROR sending OK for OPEN_SESSION to client " << msg->username << "\n";
		return;
	}

	// std::cout << "ServerProcessor_openSession(): sent OK for OPEN_SESSION to client " << msg->username << "\n";
}

void ServerProcessor_onCloseWrite(ServerCommunicator *sc, Message *msg) {
	// std::cout << "ServerProcessor_onCloseWrite(): recv FILE_CLOSE_WRITE from client " << msg->username << "\n";

	pthread_mutex_lock(&sc->acceptedThreadsLock);
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	pthread_mutex_unlock(&sc->acceptedThreadsLock);

	//Recupera o connectionId desta conexao
	pthread_mutex_lock(&sc->connectionIdLock);
	int connectionId = sc->threadConnId.find((pthread_self()))->second;	
	pthread_mutex_unlock(&sc->connectionIdLock);

	//Recupera o sync_dir_<username>
	std::string sync_dir(SYNC_DIR_BASE_NAME);
	sync_dir.append(msg->username);

	//Monta o path com o nome do arquivo recebido no payload da msg
	std::string path(sync_dir);
	path.append("/").append(msg->payload);
	std::string filename(msg->payload);
	// std::cout << "ServerProcessor_onCloseWrite(): creating file " << path << "\n";

	// Envia um OK para o cliente
	msg->type = OK;
	Message_send(msg,sockfd);
	
	// Começa o recebimento do arquivo
	if(FileManager_receiveFile(path, msg, sockfd) == -1){
		std::cerr<<"ServerProcessor_onCloseWrite(): Error Receive File\n";
	}

	// Propaga o arquivo para o restante dos dispositivos
	ServerProcessor_propagateFiles(sc, connectionId, msg, filename, 1);

	// std::cout << "ServerProcessor_onCloseWrite(): END recv FILE_CLOSE_WRITE from client " << msg->username << "\n";
}

// Mode = 0: todos usuarios
// mode = 1: apenas os restantes
void ServerProcessor_propagateFiles(ServerCommunicator *sc, int connectionId, Message *msg, std::string filename, int mode){
	std::pair<int,int> connection_ids;
	
	pthread_mutex_lock(&sc->userSessionsLock);
	connection_ids = sc->userSessions.find(msg->username)->second;
	pthread_mutex_unlock(&sc->userSessionsLock);

	int connectionId_toSend;

	if(connection_ids.second == 0){
		return;
	}else{
		// Se existir dois dispositivos do mesmo usuario
		if(mode == 0){
				
				// Propaga pra todos
				FileManager_sendFile2Queue(sc, filename, msg, connection_ids.first);
				FileManager_sendFile2Queue(sc, filename, msg, connection_ids.second);

		}else if(mode == 1){
			if(connection_ids.first == connectionId){

				connectionId_toSend = connection_ids.second;

			}else if(connection_ids.second == connectionId){

				connectionId_toSend = connection_ids.first;
			}

			// Propagada pra connectionId_toSend
			FileManager_sendFile2Queue(sc, filename, msg, connectionId_toSend);
		}
	}
}

void ServerProcessor_propagateDelete(ServerCommunicator *sc, int connectionId, Message *msg, std::string filename){
	std::pair<int,int> connection_ids;
		
	pthread_mutex_lock(&sc->userSessionsLock);
	connection_ids = sc->userSessions.find(msg->username)->second;
	pthread_mutex_unlock(&sc->userSessionsLock);

	int connectionId_toSend;

	if(connection_ids.second == 0){
		return;
	}else{
		if(connection_ids.first == connectionId){
			connectionId_toSend = connection_ids.second;

		}else if(connection_ids.second == connectionId){
			connectionId_toSend = connection_ids.first;;
		}

		pthread_mutex_lock(&sc->sendQueueLock);

		// Necessita criar uma nova mensagem, se não o endereço na fila de mensagens vai ser igual pra todas
		Message *msg_to_send = Message_create(DELETE_FILE, 1 , msg->username, filename.c_str());

		// Envia mensagem avisando pra excluir arquivo
		sc->sendQueue.at(connectionId_toSend).push(msg_to_send);

		pthread_mutex_unlock(&sc->sendQueueLock);
	}
}

void ServerProcessor_uploadCommand(ServerCommunicator *sc, Message *msg){
	// std::cout << "ServerProcessor_uploadCommand(): recv UPLOAD_FILE_CMD from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	std::string path("./sync_dir_");
	path.append(msg->username).append("/");

	path.append(msg->payload);
	std::string filename(msg->payload);
	// std::cout << "ServerProcessor_uploadCommand(): creating file " << path << "\n";

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

		// std::cout << "ServerProcessor_uploadCommand(): recv payload with " << msg->seqn << " bytes\n";
		
		if(write(f,(const void *)msg->payload, msg->seqn) == -1){
			exit(6);
		}
		
		msg->type = OK;
	}

	//Recupera o connectionId desta conexao
	pthread_mutex_lock(&sc->connectionIdLock);
	int connectionId = sc->threadConnId.find((pthread_self()))->second;	
	pthread_mutex_unlock(&sc->connectionIdLock);

	ServerProcessor_propagateFiles(sc, connectionId, msg, filename, 0);

	close(f);

	// std::cout << "ServerProcessor_uploadCommand(): END recv UPLOAD_FILE_CMD from client " << msg->username << "\n";
}

void ServerProcessor_downloadCommand(ServerCommunicator *sc, Message *msg) {
	// std::cout << "ServerProcessor_downloadCommand(): recv DOWNLOAD_FILE_CMD from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	std::string path("./sync_dir_");
	path.append(msg->username).append("/");

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
		// std::cout << "ServerProcessor_downloadCommand(): read " << bytes_recv << " bytes from file " << path << "\n";
		msg->seqn = bytes_recv;
		Message_send(msg, sockfd);
		bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	}

	msg->type = END;
	Message_send(msg, sockfd);

	close(f);

	// std::cout << "ServerProcessor_downloadCommand(): END recv DOWNLOAD_FILE_CMD from client " << msg->username << "\n";
}

void ServerProcessor_onDelete(ServerCommunicator *sc, Message *msg){
	// std::cout << "ServerProcessor_onDelete(): recv DELETE_FILE from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;

	std::string path("./sync_dir_");
	path.append(msg->username).append("/");

	path.append(msg->payload);

	std::string filename(msg->payload);

	// std::cout << "ServerProcessor_onDelete(): removing file " << path << "\n";

	const char *c = path.c_str();

    int Removed=std::remove(c);

	msg->type = OK;
	Message_send(msg,sockfd);

	//Recupera o connectionId desta conexao
	pthread_mutex_lock(&sc->connectionIdLock);
	int connectionId = sc->threadConnId.find((pthread_self()))->second;	
	pthread_mutex_unlock(&sc->connectionIdLock);
	
	ServerProcessor_propagateDelete(sc, connectionId, msg, filename);
	// std::cout << "ServerProcessor_onDelete(): END recv DELETE_FILE from client " << msg->username << "\n";

}

void ServerProcessor_deleteCommand(ServerCommunicator *sc, Message *msg){
	return;
}

void ServerProcessor_listServerCommand(ServerCommunicator *sc, Message *msg){
	// std::cout << "ServerProcessor_listServerCommand(): START";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	std::string path("./sync_dir_");
	path.append(msg->username).append("/");

	char output[MAX_PAYLOAD_SIZE];

	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> filelist;

	if((dir = opendir(path.c_str())) != NULL) {
  		while ((ent = readdir (dir)) != NULL) {
  			if((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0))
    			filelist.push_back(ent->d_name);
  		}
 		closedir (dir);
	} else {
	
	perror ("");
	
	}

	msg->type = OK;
	Message_send(msg, sockfd);

	FILE *fp;
	
	for (int i = 0; i < filelist.size(); ++i){
		std::string cmd("stat --printf='M: %y | A: %x | C: %w ' ");
		cmd.append(path);
		cmd.append(filelist[i]);

		std::string payload("");
		payload.append(filelist[i]);
		payload.append(" | ");

		fp = popen(cmd.c_str(), "r");
		if (fp == NULL) {
			printf("Failed to run command\n" );
			exit(1);
	  	}

	  	fgets(output, MAX_PAYLOAD_SIZE-1, fp);
	  	payload.append(output);

	  	strcpy(msg->payload, payload.c_str());
	  	msg->seqn = MAX_PAYLOAD_SIZE;
	  	Message_send(msg, sockfd);

	  	/* close */
	  	pclose(fp);
  	}

  	msg->type = END;
	Message_send(msg, sockfd);
}

void ServerProcessor_getSync(ServerCommunicator *sc, Message *msg){
	// std::cout << "ServerProcessor_getSync(): recv GET_SYNC_DIR from client " << msg->username << "\n";
	int sockfd = sc->acceptedThreads.find(pthread_self())->second;
	
	std::string path("./sync_dir_");
	path.append(msg->username).append("/");

	char output[MAX_PAYLOAD_SIZE];

	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> filelist;

	if((dir = opendir(path.c_str())) != NULL) {
  		while ((ent = readdir (dir)) != NULL) {
  			if((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)){
    			filelist.push_back(ent->d_name);
  			}
  		}
 		closedir (dir);
	} else {
	
	perror ("");
	
	}
	
	
	if(filelist.size() != 0){

		// Avisa o cliente que vai comecar enviar
		msg->type = OK;
		Message_send(msg, sockfd);

		for (int i = 0; i < filelist.size(); i++){
			std::string path2(path);
			path2.append(filelist[i]);
			
			if(i == filelist.size()){
				msg->type = END;
			}

			msg->type = OK;
			// Envia o nome do arquivo
			strcpy(msg->payload, filelist[i].c_str());
			Message_send(msg, sockfd);

			FileManager_sendFile(path2, msg, sockfd);

			if(i == filelist.size()-1){
				msg->type = END_SYNC;
				Message_send(msg, sockfd);
			}else{
				msg->type = END;
				Message_send(msg, sockfd);
			}
	  	} 
	}else{
		// Avisa o cliente que não há arquivos
		msg->type = NOK;
		Message_send(msg, sockfd);
	}

}

void ServerProcessor_exitCommand(ServerCommunicator *sc, Message *msg){
	std::cout << "Session closed: " << msg->username << "\n";
/*	std::pair<int,int> connection_ids;
	
	pthread_mutex_lock(&sc->userSessionsLock);
	connection_ids = sc->userSessions.find(msg->username)->second;
	
	//Recupera o connectionId desta conexao
	pthread_mutex_lock(&sc->connectionIdLock);
	int connectionId = sc->threadConnId.find((pthread_self()))->second;	
	if(connection_ids.first == connectionId) {
		if(connection_ids.second == 0)
			sc->userSessions.at(msg->username).first = 0;
		else
		{
			sc->userSessions.at(msg->username).first = connection_ids.second;
			sc->userSessions.at(msg->username).second = 0;
		}		
	}
	else
	{
		sc->userSessions.at(msg->username).second = 0;
	}
	pthread_mutex_unlock(&sc->connectionIdLock);
	pthread_mutex_unlock(&sc->userSessionsLock);
	*/	

}