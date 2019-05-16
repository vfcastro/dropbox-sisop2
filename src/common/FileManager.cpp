#include <dirent.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <experimental/filesystem>
#include "../../include/common/FileManager.h"
#include "../../include/common/Message.h"
#include <fcntl.h>
#include <unistd.h>

int FileManager_createDir(char* name) {
    // Creating a directory
    if (mkdir(name, 0777) == -1) {
		if(errno != EEXIST) {
	        std::cerr << "FileManager_createDir(): ERROR " << std::strerror(errno) << std::endl;
    	    return -1;
		}
	}
	else
		return 1;
}


int FileManager_openDir(char* name) {
	DIR* dir = opendir(name);
	if(dir) {
		closedir(dir);
		return 1;
	}
	else
		return -1;
}

int FileManager_removeFile(std::string name){
	return 0;
}
int FileManager_renameFile(char* oldname, char* newname);

int FileManager_getFileSize(char* name) {
	struct stat st;
	if(stat(name,&st) == -1){
		std::cerr<<"FileManager_getFileSize("<<name<<") ERROR \n";
		return -1;
	}
	else
		return st.st_size;
}

int FileManager_readFile(int fd, char* buffer);
int FileManager_writeFile(int fd, char* buffer);
int FileManager_createFile(char* name);

int FileManager_sendFile(std::string path, Message *msg, int socket){
	int f;

	if((f = open(path.c_str(), O_RDONLY)) == -1){
		std::cerr << "FileManager_sendFile(): ERROR opening file " << path << "\n";
		return -1;
	}

	int bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);

	while(bytes_recv){
		std::cout << "FileManager_sendFile(): read " << bytes_recv << " bytes from file " << path << "\n";
		msg->seqn = bytes_recv;
		Message_send(msg,socket);
		bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	}

	close(f);

	msg->type = END;
	Message_send(msg, socket);

	return 0;
}

int FileManager_sendFile2Queue(ServerCommunicator *sc, std::string path, Message *msg, int connectionId){
	int f;

	std::string full_path("./sync_dir_");
	full_path.append(msg->username).append("/").append(path);

	pthread_mutex_lock(&sc->sendQueueLock);

	// Necessita criar uma nova mensagem, se não o endereço na fila de mensagens vai ser igual pra todas
	Message *msg_to_send = Message_create(S2C_PROPAGATE, 1 , msg->username, path.c_str());

	// Envia mensagem avisando que vai ter arquivo pra sincronizar
	sc->sendQueue.at(connectionId).push(msg_to_send);

	if((f = open(full_path.c_str(), O_RDONLY)) == -1){
		std::cerr << "FileManager_sendFile(): ERROR opening file " << full_path << "\n";
		return -1;
	}

	msg_to_send = Message_create(S2C_PROPAGATE, 0 , msg->username, std::string().c_str());
	int bytes_recv = read(f, msg_to_send->payload, MAX_PAYLOAD_SIZE);

	while(bytes_recv){
		// std::cout << "$$$$$$$$$$$$$$$$$$$$: " << msg_to_send->payload << "\n";

		std::cout << "FileManager_sendFile(): read " << bytes_recv << " bytes from file " << full_path << "\n";
		msg_to_send->seqn = bytes_recv;

		// std::cout<<"Enviando arquivos na fila | msg_to_send->type: " << msg_to_send->type << "\n";
		sc->sendQueue.at(connectionId).push(msg_to_send);

		msg_to_send = Message_create(S2C_PROPAGATE, 0 , msg->username, std::string().c_str());
		bzero(msg_to_send->payload, MAX_PAYLOAD_SIZE);
		bytes_recv = read(f, msg_to_send->payload, MAX_PAYLOAD_SIZE);
	}

	close(f);

	msg_to_send = Message_create(END, 0 , msg->username, std::string().c_str());
	bzero(msg_to_send->payload, MAX_PAYLOAD_SIZE);

	// std::cout<<"Enviando arquivo na fila | msg_to_send->type: " << msg_to_send->type << "\n";
	sc->sendQueue.at(connectionId).push(msg_to_send);
	pthread_mutex_unlock(&sc->sendQueueLock);

	return 0;
}

// Recebe um arquivo do servidor ou cliente, dado um socket e um path
int FileManager_receiveFile(std::string path, Message *msg, int socket){
	int f = open((char*)path.c_str(), O_CREAT|O_WRONLY, 0600);

	if(f == -1){
		std::cerr << "FileManager_receiveFile(): ERROR creating file " << path << "\n";
		return -1;
	}

	while(Message_recv(msg,socket) != -1) {
		// Verifica se tipo = END, se sim, para de escrever
		if(msg->type == END){
			return 0;
		}

		// std::cout << "FileManager_receiveFile(): recv payload with " << msg->seqn << " bytes\n";
		
		if(write(f, (const void *)msg->payload, msg->seqn) == -1){
			exit(6);
		}
	}

	close(f);
	return 0;

}