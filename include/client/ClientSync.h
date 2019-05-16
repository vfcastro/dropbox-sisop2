#include <set>
#include "../../include/client/ClientCommunicator.h"

struct ClientSync {
	// thread disparada para sync
	pthread_t syncThread;
	
	// sync_dir criado na ClientSync_init
	std::string sync_dir;
	
	//Referencia para o ClientCommunicator
	ClientCommunicator *cc;

};

void ClientSync_init(ClientSync *cs, ClientCommunicator *cc);
void ClientSync_get_sync_dir(ClientSync *cs);
void ClientSync_sync(ClientSync *cs);
void* ClientSync_watch(void * cs);
void ClientSync_onCloseWrite(ClientSync *cs, char *name);
void ClientSync_onDelete(ClientSync *cs, char *name);
void ClientSync_onRename(ClientSync *cs, char *oldname, char* newname);