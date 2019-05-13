#include "../../include/common/Message.h"

#ifndef __SC_H__
#include "../../include/server/ServerCommunicator.h"
#endif

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg);
void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg);
void ServerProcessor_onCloseWrite(ServerCommunicator *sc, Message *msg);
