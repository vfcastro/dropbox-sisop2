#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

void ClientInterface_start(ClientCommunicator *cc);
void ClientInterface_command(std::string command,bool* exit);
