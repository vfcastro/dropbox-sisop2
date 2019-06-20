#include <string>
#include <vector>
#include "../../include/common/Message.h"

#ifndef __SC_H__
#include "../../include/server/ServerCommunicator.h"
#endif

struct ReplicaManager {
    // 0 ou 1 (false ou true)
    int primary;
    string primary_host;
    unsigned int primary_port;

    // vetor de host e porta dos backups
    std::vector<std::pair<string,unsigned int>> backups;

    // referencia ao ServerCommunicator para propagacao do seu estado aos backups
    ServerCommunicator *sc;

    // thread de conexao entre servers
    pthread_t connectionThread;
};

void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<string> hosts_and_ports);
void* ReplicaManager_connect(void* rm);