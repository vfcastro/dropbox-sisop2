#ifndef __RM_H__
#define __RM_H__

#include <string>
#include <map>
#include "../../include/common/Message.h"
#include "../../include/server/ServerCommunicator.h"


struct ReplicaManager {
    // 0 ou 1 (false ou true)
    int primary;
    std::string primary_host;
    unsigned int primary_port;

    int elected;

    // map de host/porta e socket dos backups
    std::map<std::pair<std::string,unsigned int>,int> backups;

    // referencia ao ServerCommunicator para propagacao do seu estado aos backups
    struct ServerCommunicator *sc;

    // thread de conexao entre servers
    pthread_t connectionThread;
};

void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<std::string> hosts_and_ports);
void* ReplicaManager_connect(void* rm);
void ReplicaManager_election(ReplicaManager *rm);
void ReplicaManager_sendMessageToBackups(ServerCommunicator *sc, Message *msg);
void ReplicaManager_heartBeater(ReplicaManager *rm);
void ReplicaManager_iamTheLeader(ReplicaManager *rm);
void ReplicaManager_updateLeader(ReplicaManager *rm, int elected);

#endif