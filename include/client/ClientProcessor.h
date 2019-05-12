#include "../../include/common/Message.h"

#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

void ClientProcessor_dispatch(ClientCommunicator *cc, Message *msg);
