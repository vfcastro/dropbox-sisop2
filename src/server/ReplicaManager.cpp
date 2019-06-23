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
	while(Message_recv(msg,sockfd) != -1) {

		if(msg->type == OK){
			pthread_exit(NULL);
		}

		ReplicaManager_dispatch(rm,msg,sockfd);
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
    s->threadConnId.insert(std::pair<pthread_t,int>(pthread_self(),s->connectionId));
    
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