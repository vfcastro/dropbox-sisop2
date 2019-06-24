#ifndef __CF_H__
#define __CF_H__

#ifndef __CC_H__
#include "../../include/client/ClientCommunicator.h"
#endif

#ifndef __SOCKET_H__
#include "../../include/common/Socket.h"
#endif

void ClientFrontend_init(ClientCommunicator *cc, std::string server, unsigned int port, unsigned int frontend_port);
void* ClientFrontend_receive(void* cc);

#endif
