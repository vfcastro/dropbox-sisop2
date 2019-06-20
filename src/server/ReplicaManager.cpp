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
        //std::cout << "ReplicaManager_init host: " << hosts_and_ports.at(0) << "\n";
        string host = hosts_and_ports.at(0);
        //std::cout << "ReplicaManager_init port: " << hosts_and_ports.at(1) << "\n";
        unsigned int port = std::stoul(hosts_and_ports.at(1));
        pair<string,unsigned int> host_port (host,port);
        rm->backups.push_back(host_port);
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

    for (std::vector<std::pair<string,unsigned int>>::iterator it = r->backups.begin() ; it != r->backups.end(); ++it)
    {
        int connected = 0;
        while(connected == 0)
        {
            std::cout << "ReplicaManager_connect: trying " << (*it).first << ":" << (*it).second << "\n";
            int sockfd = Socket_openSocket((*it).first,(*it).second);
            if(sockfd != -1)
            {
                std::cout << "ReplicaManager_connect: connected " << (*it).first << ":" << (*it).second << "\n";
                connected = 1;
            }
            else
                sleep(1);            
        }
    }
}