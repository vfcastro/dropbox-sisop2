#include "../../include/common/Message.h"

#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg);
void ClientProcessor_onCloseWrite(ClientCommunicator *cc, Message *msg);
void ClientProcessor_receivePropagate(ClientCommunicator *cc, Message *msg);
void ClientProcessor_receiveDelete(ClientCommunicator *cc, Message *msg);
