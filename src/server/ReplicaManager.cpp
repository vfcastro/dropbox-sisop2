#include <unistd.h>
#include <ctime>
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

    // Se não for primário, fica testando se o primário está vivo
    if(r->primary == 0){
        ReplicaManager_heartBeater(r);
    }

}

void ReplicaManager_heartBeater(ReplicaManager *rm){

    int sockfd;
    int primary_connected = 0;

    // Tenta uma primeira conexao com o servidor
    if((sockfd = Socket_openSocket(rm->primary_host, rm->primary_port)) == -1){
        std::cerr << "ReplicaManager_heartBeater(): ERROR primary server does not respond" << "\n";
        
        ReplicaManager_election(rm);            
    }

    primary_connected = 1;

    Message *msg = Message_create(HEARTBEAT, 0, std::string().c_str(),std::string().c_str());

    std::cout << "ReplicaManager_heartBeater(): connected to primary server" << "\n";
    
    while(primary_connected){
        if(Message_send(msg, sockfd) == -1){
            primary_connected = 0;
            break;        
        }

        std::cout << "ReplicaManager_heartBeater(): primary server is alive" << "\n";
        
        sleep(2);
    }
    
    ReplicaManager_election(rm);

}

void ReplicaManager_election(ReplicaManager *rm){
    std::cout << "ReplicaManager_election(): starting a new election" << "\n";
    
    Message *msg = Message_create(ELECTION, rm->sc->port, std::string().c_str(),std::string().c_str());
    Message *msg_recv = Message_create(ELECTION, rm->sc->port, std::string().c_str(),std::string().c_str());
    
    int sockfd;
    int existe_maior = 0;

    // Inicia uma eleição
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin(); it != rm->backups.end(); ++it){   

        sockfd = it->second;

        // Se porta desse servidor é maior que a sua, envia mensagem de eleicao
        if(it->first.second > rm->sc->port){
            existe_maior = 1;

            // Envia eleição pra server_i
            if(Message_send(msg, sockfd) == -1){
                std::cerr << "ReplicaManager_election(): Can't send ELECTION to  " << it->first.second << "\n";
                continue;        
            }

            std::cout << "ReplicaManager_election(): ELECTION send to " << it->first.second << ", waiting for answer\n";

            // Espera resposta de server_i
            if(Message_recv(msg_recv, sockfd) == -1){
                std::cerr << "ReplicaManager_election(): Can't receive answer from " << it->first.second << "\n";
                continue;
            }

            // Se a mensagem recebida for um OK, então se retira da eleição
            if(msg_recv->type == OK){
                std::cout << "ReplicaManager_election(): I am " << rm->sc->port << " and I'm out of election, " << it->first.second << " is best than me\n";
                break;
            }
        }
    }

    // Como não existe maior, se elege e avisa geral
    if(!existe_maior){
        ReplicaManager_iamTheLeader(rm);
    }     
}

void ReplicaManager_updateLeader(ReplicaManager *rm, int elected){
    std::cout << "ReplicaManager_updateLeader(): " << rm->elected << " is the elected.\n";
    rm->elected = elected;
}

void ReplicaManager_iamTheLeader(ReplicaManager *rm){
    std::cout << "ReplicaManager_iamTheLeader(): sending COORDINATOR for everyone" << "\n";
    
    Message *msg = Message_create(COORDINATOR, rm->sc->port, std::string().c_str(),std::string().c_str());
    int sockfd;

    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin(); it != rm->backups.end(); ++it){   

        sockfd = it->second;

        if(Message_send(msg, sockfd) == -1){
            std::cerr << "ReplicaManager_iamTheLeader(): ERROR sending COORDINATOR to " << it->first.second << "\n";
            continue;
        }

        std::cout << "ReplicaManager_iamTheLeader(): COORDINATOR send to " << it->first.second << "\n";
    }

    std::cout << "ReplicaManager_iamTheLeader(): I am the leader now" << "\n";
    rm->elected = rm->sc->port;
}

void ReplicaManager_sendMessageToBackups(ServerCommunicator *sc, Message *msg) {
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = sc->rm->backups.begin() ; it != sc->rm->backups.end(); ++it) 
    {   
        std::cout << "ReplicaManager_sendMessageToBackups: sending msg to " << it->first.first << ":" << it->first.second << " on socketfd " << it->second << "\n";
        Message_send(msg,it->second);
    }

}