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
#include "../../include/server/ServerProcessor.h"
#include "../../include/common/FileManager.h"


void ReplicaManager_init(ReplicaManager *rm, ServerCommunicator *sc, int primary, std::vector<string> hosts_and_ports) {
    pthread_mutex_init(&rm->sendMessageToBackupsLock,NULL);
    
    // populando a struct 
    rm->sc = sc;    
    rm->primary = primary;
    rm->backupStopped = 1;
    pthread_mutex_init(&rm->backupStoppedLock,NULL);
    rm->runningElection = 0;
    pthread_mutex_init(&rm->runningElectionLock,NULL);

    if(primary == 1) {
        rm->primary_host = sc->host;
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
    rm->connectionThread = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(rm->connectionThread,0,ReplicaManager_connect,(void*)rm);
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
                // Ja havia um socket aberto, entao estamos em um eleito.
                // A conexao antiga era entre backups e pode ser fechada apos eleicao
                if(it->second != 0)
                    close(it->second);

                it->second = sockfd;

                if(r->primary == 1)
			    { 
				    Message *msg = Message_create(BACKUP_START,0,std::string().c_str(),std::string().c_str());
                    Message_send(msg,sockfd);
			    }

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
    int connected = 0;
    
    while(rm->primary == 0) 
    {  
        // Tenta uma primeira conexao com o servidor
        if((sockfd = Socket_openSocket(rm->primary_host, rm->primary_port)) == -1){
            std::cerr << "ReplicaManager_heartBeater(): ERROR primary server does not respond" << "\n"; 
            ReplicaManager_election(rm);            
        }
        else
        {
            std::cout << "ReplicaManager_heartBeater(): connected to primary server" << "\n";
            connected = 1;
            Message *msg = Message_create(HEARTBEAT, 0, std::string().c_str(),std::string().c_str());

            while(connected)
            {
                if(Message_send(msg, sockfd) == -1){
                    std::cerr << "ReplicaManager_heartBeater(): ERROR sending HEARTBEAT" << "\n";
                    ReplicaManager_election(rm);
                    connected = 0;
                }
                else
                {
                    if(Message_recv(msg,sockfd) == -1)
                    {
                        std::cerr << "ReplicaManager_heartBeater(): ERROR receiving HEARTBEAT" << "\n";
                        ReplicaManager_election(rm);
                        connected = 0;
                    }
                    else 
                    {
                        std::cout << "ReplicaManager_heartBeater(): primary server is alive" << "\n";
                        sleep(1);
                    }
                }
            }
            close(sockfd);
            free(msg);
            
        }
        sleep(1);
    }    
}

void ReplicaManager_election(ReplicaManager *rm){
    pthread_mutex_lock(&rm->runningElectionLock);
    rm->runningElection = 1;
    std::cout << "ReplicaManager_election(): starting a new election" << "\n";
    
    Message *msg = Message_create(ELECTION, rm->sc->port, std::string().c_str(),std::string(rm->sc->host).c_str());
    Message *msg_recv = Message_create(ELECTION, rm->sc->port, std::string().c_str(),std::string().c_str());
    
    int sockfd;
    int existe_maior = 0;

    // Inicia uma eleição
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin(); it != rm->backups.end(); ++it){   

        sockfd = it->second;

        // Se porta desse servidor é maior que a sua, envia mensagem de eleicao
        if(it->first.second > rm->sc->port){
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
                existe_maior = 1;
                break;
            }
        }
    }

    // Como não existe maior, se elege e avisa geral
    if(!existe_maior){
        int already_primary = rm->primary;
        ReplicaManager_iamTheLeader(rm);

        if(!already_primary)
        {
            pthread_mutex_lock(&rm->backupStoppedLock);
            std::cout << "ReplicaManager_election(): BACKUP STOPPED\n";
            rm->backupStopped = 1;
            pthread_mutex_unlock(&rm->backupStoppedLock);

            ReplicaManager_updateClients(rm);
            
            rm->runningElection = 0;
            pthread_mutex_unlock(&rm->runningElectionLock);
            ReplicaManager_connect((void*)rm);
        }
    }

    rm->runningElection = 0;
    pthread_mutex_unlock(&rm->runningElectionLock);
}

void ReplicaManager_iamTheLeader(ReplicaManager *rm){
    std::cout << "ReplicaManager_iamTheLeader(): I am the leader now" << "\n";
    rm->primary = 1;
    rm->primary_host = rm->sc->host;
    rm->primary_port = rm->sc->port;
    std::cout << "ReplicaManager_iamTheLeader(): " << rm->primary_host << ":" << rm->primary_port << "\n";

    std::cout << "ReplicaManager_iamTheLeader(): sending COORDINATOR for everyone" << "\n";    
    Message *msg = Message_create(COORDINATOR, rm->primary_port, std::string().c_str(),std::string(rm->primary_host).c_str());
    int sockfd;

    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin(); it != rm->backups.end(); ++it){   

        sockfd = it->second;

        if(Message_send(msg, sockfd) == -1){
            std::cerr << "ReplicaManager_iamTheLeader(): ERROR sending COORDINATOR to " << it->first.first << ":" << it->first.second << "\n";
            rm->backups.erase(std::pair<std::string,unsigned int>(it->first.first,it->first.second));
            continue;
        }

        std::cout << "ReplicaManager_iamTheLeader(): COORDINATOR sent to " << it->first.first << ":" << it->first.second << "\n";
    }
}

void ReplicaManager_sendMessageToBackups(ReplicaManager *rm, Message *msg) {
    pthread_mutex_lock(&rm->sendMessageToBackupsLock);
    Message *new_msg = (Message*)malloc(sizeof(Message));
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin() ; it != rm->backups.end(); ++it) 
    {   
        memcpy((void*)new_msg,(void*)msg,sizeof(Message));
        std::cout << "ReplicaManager_sendMessageToBackups: sending msg to " << it->first.first << ":" << it->first.second << " on socketfd " << it->second << "\n";
        Message_send(new_msg,it->second);
        Message_recv(new_msg,it->second);

    }
    free(new_msg);
    pthread_mutex_unlock(&rm->sendMessageToBackupsLock);
}

void ReplicaManager_startBackup(ReplicaManager *rm, int sockfd)
{
    std::cout << "\n\nReplicaManager_startBackup\n\n";
    Message *msg = (Message*)malloc(sizeof(Message));

    pthread_mutex_lock(&rm->backupStoppedLock);
    rm->backupStopped = 0;
    pthread_mutex_unlock(&rm->backupStoppedLock);

    while(!rm->backupStopped)
    {
        if(Message_recv(msg,sockfd) != -1)
        {
            if(msg->type == OK){
                pthread_exit(NULL);
            }
        
            ReplicaManager_dispatch(rm,msg,sockfd);
        }
   
	}
    

	free(msg);
}

void ReplicaManager_dispatch(ReplicaManager *rm, Message *msg, int sockfd)
{
    std::cout << "\n\nReplicaManager_dispatch\n\n";
    switch (msg->type)
    {
    case BACKUP_OPEN_SEND_CONN:
        if(ReplicaManager_openSendConn(rm,msg))
            Message_send(msg,sockfd);
        break;

    case BACKUP_OPEN_RECV_CONN:
        if(ReplicaManager_openRecvConn(rm,msg))
            Message_send(msg,sockfd);
        break;       

    case BACKUP_OPEN_SESSION:
        if(ReplicaManager_openSession(rm,msg))
            Message_send(msg,sockfd);
        break;

    case BACKUP_FILE_CLOSE_WRITE:
        ReplicaManager_receiveFile(rm,msg,sockfd);
        break;
    
    default:
        break;
    }

}

int  ReplicaManager_openSendConn(ReplicaManager *rm, Message *msg)
{
    std::cout << "\n\nReplicaManager_openSendConn\n\n";
    ServerCommunicator *s = rm->sc;

    s->connectionId = s->connectionId+1;

    // recupera client ip/porta e add map clientAddress
    s->clientAddress.insert(
        std::pair<int, std::pair<std::string,unsigned int>>(
                s->connectionId,
                std::pair<std::string,unsigned int>(
                    msg->payload,
                    msg->seqn
                )
        )
    );
    
    //tenta criar uma sessao
    if(s->userSessions.insert(std::pair<std::string,std::pair<int,int> >(msg->username,std::pair<int,int>(s->connectionId,0))).second == false) {
        //ja existia pelo uma sessão
        //checa se o segundo slot de sessao esta vazio
        if(s->userSessions.at(msg->username).second == 0)
            //cria segunda sessao
            s->userSessions.at(msg->username).second = s->connectionId;
    }

    return 1;
}

int  ReplicaManager_openRecvConn(ReplicaManager *rm, Message *msg)
{
    std::cout << "\n\nReplicaManager_openRecvConn\n\n";
    ServerCommunicator *s = rm->sc;
    std::queue<Message*> queue;
    s->sendQueue.insert(std::pair<int,std::queue<Message*>>(msg->seqn,queue));

    return 1;
}

int  ReplicaManager_openSession(ReplicaManager *rm, Message *msg)
{
	//Checar se existe sync_dir_<username>
	//Se não houve, criar
	std::string sync_dir(SYNC_DIR_BASE_NAME);
	sync_dir.append(msg->username);
	std::cout<<"New Session for: " << msg->username << "\n";

	if(FileManager_createDir((char*)sync_dir.c_str()) == -1) {
		std::cerr<<"ReplicaManager_openSession(): ERROR creating user dir " << sync_dir << "\n";
		return -1;
	}
    
    return 1;
}

int  ReplicaManager_receiveFile(ReplicaManager *rm, Message *msg, int sockfd)
{
    //Recupera o sync_dir_<username>
	std::string sync_dir(SYNC_DIR_BASE_NAME);
	sync_dir.append(msg->username);
    //Monta o path com o nome do arquivo recebido no payload da msg
	std::string path(sync_dir);
	path.append("/").append(msg->payload);
	std::cout << "ReplicaManager_receiveFile(): creating file " << path << "\n";

	// Envia um OK para o primario
 	msg->type = OK;
	Message_send(msg,sockfd);

    // Começa o recebimento do arquivo
	if(FileManager_receiveFile(path, msg, sockfd) == -1){
		std::cerr<<"ReplicaManager_receiveFile(): Error Receive File\n";
        return -1;
	}

    return 1;
}

void ReplicaManager_sendFileToBackups(ReplicaManager *rm, std::string path, Message *msg)
{
    pthread_mutex_lock(&rm->sendMessageToBackupsLock);
    Message *new_msg = (Message*)malloc(sizeof(Message));
    for (std::map<std::pair<string,unsigned int>,int>::iterator it = rm->backups.begin() ; it != rm->backups.end(); ++it) 
    {   
        memcpy((void*)new_msg,(void*)msg,sizeof(Message));
        std::cout << "ReplicaManager_sendFileToBackups: sending msg to " << it->first.first << ":" << it->first.second << " on socketfd " << it->second << "\n";
        Message_send(new_msg,it->second);
        Message_recv(new_msg,it->second);

       	if(FileManager_sendFile(path, new_msg, it->second) == -1){
	    	std::cout<<"ReplicaManager_sendFileToBackups(): Error Send File\n";
	    }
    }
    free(new_msg);
    pthread_mutex_unlock(&rm->sendMessageToBackupsLock);
}

void ReplicaManager_updateClients(ReplicaManager *rm)
{
    std:cout << "ReplicaManager_updateClients\n";

    // conecta aos clients e envia ip/porta novos
    for (std::map<int,std::pair<std::string,unsigned int>>::iterator it = rm->sc->clientAddress.begin() ; it != rm->sc->clientAddress.end(); ++it)
    {
        int connected = 0;
        while(connected == 0)
        {
            std::cout << "ReplicaManager_updateClients: trying " << it->second.first << ":" << it->second.second << "\n";
            int sockfd = Socket_openSocket(it->second.first,it->second.second);
            if(sockfd != -1)
            {
			    Message *msg = Message_create(FRONTEND_NEW_SERVER,rm->primary_port,std::string().c_str(),std::string(rm->primary_host).c_str());
                Message_send(msg,sockfd);
                Message_recv(msg,sockfd);

                std::cout << "ReplicaManager_updateClients: FRONTEND_NEW_SERVER sent to " << it->second.first << ":" << it->second.second << "\n";
                connected = 1;
            }
            else
                sleep(1);            
        }
    }



}

void ReplicaManager_receiveHeartBeat(ReplicaManager *rm, Message *msg, int sockfd)
{
    Message_send(msg,sockfd);

    int timeout = 10;
    while(timeout > 0)
    {
        if(Message_recv(msg,sockfd) != -1)
            Message_send(msg,sockfd);
        else
        {
            timeout--;
            sleep(1);
        }
        
    }
    std::cerr << "ReplicaManager_receiveHeartBeat(): TIMEOUT waiting for hearbeat. Probally backup disconnected.\n";
}

void ReplicaManager_receiveElection(ReplicaManager *rm, Message *msg, int sockfd)
{
    std::cout << "ReplicaManager_receiveElection(): Receive ELECTION from " << msg->payload << "\n";

    msg->type = OK;
    if(Message_send(msg, sockfd) == -1){
        std::cerr << "ReplicaManager_receiveElection(): ERROR sending reply\n";
     
    }	

    ReplicaManager_election(rm);
}

void ReplicaManager_receiveCoordinator(ReplicaManager *rm, Message *msg, int sockfd)
{
    std::string host = msg->payload;
    unsigned int port = msg->seqn;
    std::cout << "ReplicaManager_receiveCoordinator(): Receive COORDINATOR from " << host << ":" << port << "\n";
    rm->primary_host = std::string(host);
    rm->primary_port = port;
    rm->backups.erase(std::pair<std::string,unsigned int>(host,port));

    pthread_mutex_lock(&rm->backupStoppedLock);
    rm->backupStopped = 1;
    pthread_mutex_unlock(&rm->backupStoppedLock);

}