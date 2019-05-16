#include "../../include/common/Message.h"

#ifndef __SC_H__
#include "../../include/server/ServerCommunicator.h"
#endif

#define SYNC_DIR_BASE_NAME "./sync_dir_"

void ServerProcessor_dispatch(ServerCommunicator *sc, Message *msg);
void ServerProcessor_openSession(ServerCommunicator *sc, Message *msg);
void ServerProcessor_onCloseWrite(ServerCommunicator *sc, Message *msg);
void ServerProcessor_onDelete(ServerCommunicator *sc, Message *msg);
void ServerProcessor_uploadCommand(ServerCommunicator *sc, Message *msg);
void ServerProcessor_downloadCommand(ServerCommunicator *sc, Message *msg);
void ServerProcessor_deleteCommand(ServerCommunicator *sc, Message *msg);
void ServerProcessor_listServerCommand(ServerCommunicator *sc, Message *msg);
void ServerProcessor_exitCommand(ServerCommunicator *sc, Message *msg);
void ServerProcessor_propagateFiles(ServerCommunicator *sc, int connectionId, Message *msg, std::string filename, int mode);
void ServerProcessor_getSync(ServerCommunicator *sc, Message *msg);
void ServerProcessor_propagateDelete(ServerCommunicator *sc, int connectionId, Message *msg, std::string filename);