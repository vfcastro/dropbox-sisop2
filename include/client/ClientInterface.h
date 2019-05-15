#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

void ClientInterface_start(ClientCommunicator *cc);
void ClientInterface_command(std::string command, bool *exit, ClientCommunicator *cc);
void ClientInterface_upload(ClientCommunicator *cc, std::string filepath);
void ClientInterface_download(ClientCommunicator *cc, std::string filename);
void ClientInterface_delete(ClientCommunicator *cc, std::string filename);
void ClientInterface_listServer(ClientCommunicator *cc);
void ClientInterface_listClient(ClientCommunicator *cc);
void ClientInterface_exit(ClientCommunicator *cc);