#ifndef __RM_H__
#define __RM_H__

#include <string>
#include <vector>
#include "../../include/common/Message.h"
#include "../../include/server/ServerCommunicator.h"


struct ReplicaManager {
    // 0 ou 1 (false ou true)
    int primary;
    std::string primary_host;
    unsigned int primary_port;

    // vetor de host e porta dos backups
    std::vector<std::pair<std::string,unsigned int>> backups;

    // referencia ao ServerCommunicator para propagacao do seu estado aos backups
    struct ServerCommunicator *sc;

    // thread de conexao entre servers
    pthread_t connectionThread;
};

void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<std::string> hosts_and_ports);
void* ReplicaManager_connect(void* rm);

#endif