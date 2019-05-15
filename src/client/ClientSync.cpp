#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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
	cs->cc = cc;
	cs->sync_dir = std::string("./sync_dir_").append(cc->username);
}

void ClientSync_get_sync_dir(ClientSync *cs) {
	// checa se sync_dir_username existe no current work dir
	// se nao houver, cria e solicita sync_dir ao server
	if(FileManager_openDir((char*)cs->sync_dir.c_str()) == -1) {
		if(FileManager_createDir((char*)cs->sync_dir.c_str()) == -1) {
			std::cerr << "ClientSync_get_sync_dir(): ERROR creating dir " << cs->sync_dir << "\n";
			exit(-1);
		}
		// Solicita todo o sync_dir do server
		ClientSync_sync(cs);
	}

	// Dispara thread de WATCH
	pthread_create(&(cs->syncThread),0,ClientSync_watch,(void*)cs);
}

void* ClientSync_watch(void *cs) {
	std::cout << "ClientSync_watch() thread START\n";
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
	if ( fd < 0 ) {
	  perror( "Couldn't initialize inotify");
	}

	/* add watch to starting directory */
	wd = inotify_add_watch(fd, sync_dir, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

	if (wd == -1)
	  {
	    printf("Couldn't add watch to %s\n",sync_dir);
	  }
	else
	  {
	    printf("Watching:: %s\n",sync_dir);
	  }

	/* do it forever*/
	while(1)
	  {
	    i = 0;
	    length = read( fd, buffer, BUF_LEN );

	    if ( length < 0 ) {
	      perror( "read" );
	    }

	    while ( i < length ) {
	      struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	      char* oldname;
	      if ( event->len ) {
	        if ( event->mask & IN_CLOSE_WRITE) {
	          if (event->mask & IN_ISDIR)
	            printf( "The directory %s was modified.\n", event->name );
	          else
	            printf( "The file %s was IN_CLOSE_WRITE with WD %d\n", event->name, event->wd );
	          	ClientSync_onCloseWrite(c,event->name);
	        }

	        if ( event->mask & IN_MOVED_FROM) {
	          if (event->mask & IN_ISDIR)
	            printf( "The directory %s was IN_MOVED_FROM.\n", event->name );
	          else {
	            printf( "The file %s was IN_MOVED_FROM with WD %d\n", event->name, event->wd );
 				oldname = (char*)malloc(event->len);
 				strcpy(oldname,(const char*)event->name);
			  }
	        }

	  	  if ( event->mask & IN_MOVED_TO) {
              if (event->mask & IN_ISDIR)
                printf( "The directory %s was IN_MOVED_TO.\n", event->name );
              else {
                printf( "The file %s was IN_MOVED_TO to %s\n", oldname, event->name );
	          	ClientSync_onRename(c,oldname,event->name);
				free(oldname);
			  }
            }

	        if ( event->mask & IN_DELETE) {
	          if (event->mask & IN_ISDIR)
	            printf( "The directory %s was IN_DELETE.\n", event->name );
	          else
	            printf( "The file %s was IN_DELETE with WD %d\n", event->name, event->wd );
	          	ClientSync_onDelete(c,event->name);
	        }

	        i += EVENT_SIZE + event->len;
	      }
	    }
	  }

	/* Clean up*/
	inotify_rm_watch( fd, wd );
	close( fd );


	std::cout << "ClientSync_watch() thread END\n";
}

void ClientSync_sync(ClientSync *cs) {
	//TODO: baixar sync_dir do server

}
//TODO :eliminar codigo duplicado em ClientSync_onCloseWrite e ClientSync_onDelete,deveria ser uma funcao apenas
void ClientSync_onCloseWrite(ClientSync *cs, char *name) {
	std::string path(cs->sync_dir);
	path.append("/").append(name);
	int size = FileManager_getFileSize((char*)path.c_str());
	std::cout<<"ClientSync_onCloseWrite(): file: "<<path<<" size: "<<size<<"\n";
	int remainder = size % MAX_PAYLOAD_SIZE;
	int quot = size / MAX_PAYLOAD_SIZE;
	int num_of_messages;
	if(remainder == 0)
		num_of_messages = quot;
	else
		num_of_messages = quot + 1;
	std::cout<<"ClientSync_onCloseWrite(): file: "<<path<<" num_of_messages: "<<num_of_messages<<"\n";

	Message *msg = Message_create(FILE_CLOSE_WRITE,num_of_messages,cs->cc->username,(const char *)name);
	Message_send(msg,cs->cc->sendsockfd);
	Message_recv(msg,cs->cc->sendsockfd);

	int f;

	if((f = open(path.c_str(), O_RDONLY)) == -1){
		std::cerr << "ClientSync_onCloseWrite(): ERROR opening file " << path << "\n";
		return;
	}

	int bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);

	while(bytes_recv){
		std::cout << "ClientSync_onCloseWrite(): read " << bytes_recv << " bytes from file " << path << "\n";
		if(bytes_recv < MAX_PAYLOAD_SIZE)
			msg->payload[bytes_recv] = '\0';
		num_of_messages = num_of_messages - 1;
		msg->seqn = num_of_messages;
		Message_send(msg,cs->cc->sendsockfd);
		bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	}

	msg->type = END;
	Message_send(msg,cs->cc->sendsockfd);
}

void ClientSync_onDelete(ClientSync *cs, char *name) {
	std::string path(cs->sync_dir);
	path.append("/").append(name);
	int size = FileManager_getFileSize((char*)path.c_str());
	std::cout<<"ClientSync_onDelete(): file: "<<path<<" size: "<<size<<"\n";
	int num_of_messages=1;


	std::cout<<"ClientSync_onDelete(): file: "<<path<<" num_of_messages: "<<num_of_messages<<"\n";

	Message *msg = Message_create(DELETE_FILE,num_of_messages,cs->cc->username,(const char *)name);
	Message_send(msg,cs->cc->sendsockfd);
	Message_recv(msg,cs->cc->sendsockfd);


}
void ClientSync_onRename(ClientSync *cs, char *oldname, char* newname) {}
