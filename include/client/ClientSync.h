#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

struct ClientSync {
	// thread disparada para sync
	pthread_t syncThread;
	// sync_dir criado na ClientSync_init
	std::string sync_dir;
	
	// referencia para o ClientCommunicator, para o modulo de sync puder
	// acessar o sendsockfd para enviar mensagens de criacao de arquivo, etc
	ClientCommunicator *cc;
};

void ClientSync_init(ClientSync *cs, ClientCommunicator *cc);
void ClientSync_get_sync_dir(ClientSync *cs);
void* ClientSync_sync(void * cs);
void ClientSync_onCloseWrite(ClientSync *cs, char *name);
void ClientSync_onDelete(ClientSync *cs, char *name);
void ClientSync_onRename(ClientSync *cs, char *oldname, char* newname);
