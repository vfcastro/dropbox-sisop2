#include <iostream>
#include "../../include/client/ClientSync.h"

void ClientSync_init(ClientSync *cs, ClientCommunicator *cc) {
	cs->cc = cc;

}

void ClientSync_get_sync_dir(ClientSync *cs) {
	// TODO: checa se sync_dir_usename existe no current work dir
	
	// Dispara thread de SYNC
	pthread_create(&(cs->syncThread),0,ClientSync_sync,(void*)cs);


}

void* ClientSync_sync(void *cs) {
	std::cout << "ClientSync_sync() thread START\n";

	// TODO: código de monitoramento do inotify.
	// Sera monitorado ou o diretorio ./ (current dir atual)
	// ou podemos definir algum parametro na struct para isso
	// quando o inotify identificar algum evento, deve ser processado aqui e enviado
	// mensagens para o server via sockets do ClientCommunicator
	while(true){}



	std::cout << "ClientSync_sync() thread END\n";
}
