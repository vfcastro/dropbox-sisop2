#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include "../../include/client/ClientSync.h"
#include "../../include/common/FileManager.h"

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
	// checa se sync_dir_usename existe no current work dir
	// e cria se não houver
	if(FileManager_createDir((char*)cs->sync_dir.c_str()) == -1) {
		std::cerr << "ClientSync_get_sync_dir(): ERROR creating dir " << cs->sync_dir << "\n";
		exit(-1);
	}
	
	// Dispara thread de SYNC
	pthread_create(&(cs->syncThread),0,ClientSync_sync,(void*)cs);
}

void* ClientSync_sync(void *cs) {
	std::cout << "ClientSync_sync() thread START\n";
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
	wd = inotify_add_watch(fd, sync_dir, IN_CREATE | IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO); 
	
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
				free(oldname);
			  }
            } 

	        if ( event->mask & IN_DELETE) {
	          if (event->mask & IN_ISDIR)
	            printf( "The directory %s was IN_DELETE.\n", event->name );       
	          else
	            printf( "The file %s was IN_DELETE with WD %d\n", event->name, event->wd );       
	        }  
 
	        i += EVENT_SIZE + event->len;
	      }
	    }
	  }
	
	/* Clean up*/
	inotify_rm_watch( fd, wd );
	close( fd );


	std::cout << "ClientSync_sync() thread END\n";
}
