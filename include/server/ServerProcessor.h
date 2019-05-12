#include "../../include/common/Message.h"

#ifndef __SC_H__
#include "../../include/server/ServerCommunicator.h"
#endif

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg);
