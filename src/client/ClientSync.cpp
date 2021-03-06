#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <limits.h>
#include <fcntl.h>
#include "../../include/client/ClientSync.h"
#include "../../include/common/FileManager.h"
#include "../../include/common/Message.h"

/* Para o INOTIFY */
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void ClientSync_init(ClientSync *cs, ClientCommunicator *cc) {
	cs->sync_dir = std::string("./sync_dir_").append(cc->username);
	cs->cc = cc;
	pthread_mutex_init(&cc->syncFilesLock,NULL);
}

void ClientSync_get_sync_dir(ClientSync *cs) {
	// checa se sync_dir_username existe no current work dir
	// se nao houver, cria e solicita sync_dir ao server
	if(FileManager_openDir((char*)cs->sync_dir.c_str()) == -1) {
		if(FileManager_createDir((char*)cs->sync_dir.c_str()) == -1) {
			std::cerr << "ClientSync_get_sync_dir(): ERROR creating dir " << cs->sync_dir << "\n";
			exit(-1);
		}
	}

	// Solicita todo o sync_dir do server
	ClientSync_sync(cs);

	// Dispara thread de WATCH
	pthread_create(&(cs->syncThread),0,ClientSync_watch,(void*)cs);

	return;
}

void* ClientSync_watch(void *cs) {
	// std::cout << "ClientSync_watch() thread START\n";
	ClientSync *c = (ClientSync*)cs;
	const char *sync_dir = c->sync_dir.c_str();

	// Código de monitoramento do inotify.
	// Sera monitorado o diretorio sync_dir da struct ClientSync
	// quando o inotify identificar algum evento, é chamada a respectiva funcao
	int length, i = 0, wd;
	int fd;
	char buffer[BUF_LEN];

	/* Initialize Inotify*/
	fd = inotify_init();

	//int pauseSync;

	if ( fd < 0 ) {
		perror( "Couldn't initialize inotify");
	}

	/* add watch to starting directory */
	wd = inotify_add_watch(fd, sync_dir, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

	if (wd == -1){
	    printf("Couldn't add watch to %s\n",sync_dir);
	}
	else{
	    printf("Watching:: %s\n\n",sync_dir);
	}

	/* do it forever*/
	while(1){
	    i = 0;
	    length = read( fd, buffer, BUF_LEN );

	    if(length < 0){
	      perror("read");
	    }

	    while(i < length){
	      	struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	      	char* oldname;
	      	
	      	if(event->len){
		        if(event->mask & IN_CLOSE_WRITE){
		          	if(event->mask & IN_ISDIR){
		          	 	// printf("The directory %s was modified.\n", event->name );
		          		printf("");
		        	}
					else{					
							// Checa se arquivo eh de sincronizacao
							pthread_mutex_lock(&c->cc->syncFilesLock);
							if(c->cc->syncFiles.find(event->name) == c->cc->syncFiles.end()) {
								pthread_mutex_unlock(&c->cc->syncFilesLock);	
								ClientSync_onCloseWrite(c,event->name);
							}

							pthread_mutex_unlock(&c->cc->syncFilesLock);
					}
							
				}
			}

		    if(event->mask & IN_MOVED_FROM){
		       	if(event->mask & IN_ISDIR){
		         	printf("");
		       	}else{
			        printf("");
		 				ClientSync_onDelete(c,event->name);
				  	}
		    }

		  	if(event->mask & IN_MOVED_TO){
	         	if (event->mask & IN_ISDIR){
	         		printf("");
	         	}else{
		          	printf("");
			        ClientSync_onCloseWrite(c,event->name);
					free(oldname);
				}
	        }

		    if(event->mask & IN_DELETE) {
			    if (event->mask & IN_ISDIR){
			        printf("");
			    }else{
			        // printf("The file %s was IN_DELETE with WD %d\n", event->name, event->wd );
			        ClientSync_onDelete(c,event->name);
			    }
		    }

		    i += EVENT_SIZE + event->len;
	    }
	}
	  

	inotify_rm_watch(fd, wd);
	close(fd);

	// std::cout << "ClientSync_watch() thread END\n";
}

void ClientSync_sync(ClientSync *cs) {
	std::cout << "GET_SYNC_DIR: Sincronizando arquivos....\n";

	Message *msg = Message_create(GET_SYNC_DIR, 0, cs->cc->username, std::string().c_str());

	// Envia Requisição
	Message_send(msg, ClientCommunicator_getSendSocket(cs->cc));

	// Aguarda Ok ou aviso que nao ha arquivos
	Message_recv(msg, ClientCommunicator_getSendSocket(cs->cc));
	if(msg->type == NOK){
		return;
	}

	int receive_result = 0;
	int type = 0;
	// Recebe nome do arquivo

	while(Message_recv(msg, ClientCommunicator_getSendSocket(cs->cc)) != -1){

		type = msg->type;
		std::string path(cs->sync_dir);
		path.append("/");
		path.append(msg->payload);

		receive_result = FileManager_receiveFile(path, msg, ClientCommunicator_getSendSocket(cs->cc));

		if(receive_result == -1){
			std::cout<<"ClientSync_sync(): Error Send File\n";
			break;
		}

		Message_recv(msg, ClientCommunicator_getSendSocket(cs->cc));
		if(msg->type == END_SYNC){
			break;
		}
	}
	return;

}


//TODO :eliminar codigo duplicado em ClientSync_onCloseWrite e ClientSync_onDelete,deveria ser uma funcao apenas
void ClientSync_onCloseWrite(ClientSync *cs, char *name) {
	std::string path(cs->sync_dir);
	path.append("/").append(name);
	
	Message *msg = Message_create(FILE_CLOSE_WRITE,0,cs->cc->username,(const char *)name);

	// Envia Requisição
	Message_send(msg,ClientCommunicator_getSendSocket(cs->cc));

	// Aguarda Ok
	Message_recv(msg,ClientCommunicator_getSendSocket(cs->cc));

	if(FileManager_sendFile(path, msg, ClientCommunicator_getSendSocket(cs->cc)) == -1){
		std::cout<<"ClientSync_onCloseWrite(): Error Send File\n";
	}
}

void ClientSync_onDelete(ClientSync *cs, char *name) {
	std::string path(cs->sync_dir);
	path.append("/").append(name);
	
	Message *msg = Message_create(DELETE_FILE, 0, cs->cc->username, (const char *)name);
	Message_send(msg,ClientCommunicator_getSendSocket(cs->cc));
	Message_recv(msg,ClientCommunicator_getSendSocket(cs->cc));

}
void ClientSync_onRename(ClientSync *cs, char *oldname, char* newname) {

}
