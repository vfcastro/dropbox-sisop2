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

    int backupStopped;
    pthread_mutex_t backupStoppedLock;

    int runningElection;
    pthread_mutex_t runningElectionLock;

    // map de host/porta e socket dos backups
    std::map<std::pair<std::string,unsigned int>,int> backups;

    // referencia ao ServerCommunicator para propagacao do seu estado aos backups
    struct ServerCommunicator *sc;

    // thread de conexao entre servers
    pthread_t *connectionThread;

    // mutex para envio de msgs aos backups
    pthread_mutex_t sendMessageToBackupsLock;
};

void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<std::string> hosts_and_ports);
void* ReplicaManager_connect(void* rm);
void ReplicaManager_election(ReplicaManager *rm);
void ReplicaManager_heartBeater(ReplicaManager *rm);
void ReplicaManager_iamTheLeader(ReplicaManager *rm);
void ReplicaManager_sendMessageToBackups(ReplicaManager *rm, Message *msg); 
// Aguarda mensagens do primario
void ReplicaManager_startBackup(ReplicaManager *rm, int sockfd);
// Processa msg vinda do primario
void ReplicaManager_dispatch(ReplicaManager *rm, Message *msg, int sockfd);
int  ReplicaManager_openSendConn(ReplicaManager *rm, Message *msg);
int  ReplicaManager_openRecvConn(ReplicaManager *rm, Message *msg);
int  ReplicaManager_openSession(ReplicaManager *rm, Message *msg);
int  ReplicaManager_receiveFile(ReplicaManager *rm, Message *msg, int sockfd);
int  ReplicaManager_deleteFile(ReplicaManager *rm, Message *msg, int sockfd);
void ReplicaManager_sendFileToBackups(ReplicaManager *rm, std::string path, Message *msg);
void ReplicaManager_sendDeleteToBackups(ReplicaManager *rm, std::string path, Message *msg);
void ReplicaManager_updateClients(ReplicaManager *rm);
void ReplicaManager_receiveHeartBeat(ReplicaManager *sc, Message *msg, int sockfd);
void ReplicaManager_receiveElection(ReplicaManager *sc, Message *msg, int sockfd);
void ReplicaManager_receiveCoordinator(ReplicaManager *sc, Message *msg, int sockfd);

#endif