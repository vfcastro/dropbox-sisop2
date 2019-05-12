#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

struct ClientSync {
	// thread disparada para sync
	pthread_t syncThread;

	// talvez guardar aqui o caminho do sync_dir
	// ou usar sempre o caminho relativo ./ no inotify
	
	// referencia para o ClientCommunicator, para o modulo de sync puder
	// acessar o sendsockfd para enviar mensagens de criacao de arquivo, etc
	ClientCommunicator *cc;
};

void ClientSync_init(ClientSync *cs, ClientCommunicator *cc);
void ClientSync_get_sync_dir(ClientSync *cs);
void* ClientSync_sync(void * cs);
