#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../../include/common/Message.h"
#include "../../include/common/Socket.h"
#include "../../include/server/ReplicaManager.h"


void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<string> hosts_and_ports) {
    // populando a struct 
    rm->sc = sc;    
    rm->primary = primary;

    if(primary == 1) {
        rm->primary_host = string("localhost");
        rm->primary_port = sc->port;
    }
    else {
        // primeiro host da lista eh o primario
        rm->primary_host = hosts_and_ports.at(0);
        rm->primary_port = std::stoul(hosts_and_ports.at(1));
        hosts_and_ports.erase(hosts_and_ports.begin());
        hosts_and_ports.erase(hosts_and_ports.begin());
    }

    while(!hosts_and_ports.empty()){
        std::cout << "ReplicaManager_init host: " << hosts_and_ports.at(0) << "\n";
        std::string host = hosts_and_ports.at(0);
        std::cout << "ReplicaManager_init port: " << hosts_and_ports.at(1) << "\n";
        unsigned int port = std::stoul(hosts_and_ports.at(1));
        std::pair<string,unsigned int> host_port(host,port);
        rm->backups.insert(std::pair<std::pair<string,unsigned int>,int>(host_port,0));
        hosts_and_ports.erase(hosts_and_ports.begin());
        hosts_and_ports.erase(hosts_and_ports.begin());
    }
    /* ---------------- */

    // Diparando a thread que ira conectar aos demais servers
    pthread_create(&rm->connectionThread,0,ReplicaManager_connect,(void*)rm);
}

void* ReplicaManager_connect(void* rm) {
    std:cout << "ReplicaManager_connect\n";
    ReplicaManager *r = (ReplicaManager*)rm;

    // conecta aos demais servers
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = r->backups.begin() ; it != r->backups.end(); ++it)
    {
        int connected = 0;
        while(connected == 0)
        {
            std::cout << "ReplicaManager_connect: trying " << it->first.first << ":" << it->first.second << "\n";
            int sockfd = Socket_openSocket(it->first.first,it->first.second);
            if(sockfd != -1)
            {
                it->second = sockfd;
                std::cout << "ReplicaManager_connect: connected " << it->first.first << ":" << it->first.second << " on socketfd " << it->second << "\n";
                connected = 1;
            }
            else
                sleep(1);            
        }
    }
}

void ReplicaManager_sendMessageToBackups(ServerCommunicator *sc, Message *msg) {
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = sc->rm->backups.begin() ; it != sc->rm->backups.end(); ++it) 
    {   
        std::cout << "ReplicaManager_sendMessageToBackups: sending msg to " << it->first.first << ":" << it->first.second << " on socketfd " << it->second << "\n";
        Message_send(msg,it->second);
    }

}