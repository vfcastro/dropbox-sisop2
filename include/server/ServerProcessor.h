#include "../../include/common/Message.h"

#ifndef __SC_H__
#include "../../include/server/ServerCommunicator.h"
#endif

#define SYNC_DIR_BASE_NAME "./sync_dir_"

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg);
void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg);
void ServerProcessor_onCloseWrite(ServerCommunicator *sc, Message *msg);
void ServerProcessor_onDelete(ServerCommunicator *sc, Message *msg);
int ServerProcessor_PayloadSize(char *payload);
