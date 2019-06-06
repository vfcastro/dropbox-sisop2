#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../../include/common/Message.h"

void *buffer_geral = (void *)malloc(MAX_UNREAD_BYTES);
int curr_size_buffer_geral = 0;

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload) {
	// std::cout << "Message_create(): START\n";
	Message *msg = (Message*) malloc(sizeof(Message));
	msg->type = type;
	msg->seqn = seqn;
	memcpy((void*)msg->username,(void*)username, MAX_USERNAME_SIZE);
	memcpy((void*)msg->payload,(void*)payload, MAX_PAYLOAD_SIZE);

	// std::cout << "Message_create(): type:" << msg->type << " seqn:" << msg->seqn << " username:" << msg->username << " payload:" << msg->payload <<"\n";
	return msg;
}

void Message_unmarshall(Message *msg_destino, Message *pacote_recebido) {
    // std::cout << "Message_unmarshall(): START\n";

	void *address = (void*)pacote_recebido;
   
    memcpy((unsigned int*) &(msg_destino->type), address, sizeof(msg_destino->type));
    address += sizeof(msg_destino->type);
    
    memcpy((unsigned int*) &(msg_destino->seqn), address, sizeof(msg_destino->seqn));
    address += sizeof(msg_destino->seqn);

    memcpy((char*) &(msg_destino->username), address, MAX_USERNAME_SIZE);
    address += MAX_USERNAME_SIZE;

    memcpy((char*) &(msg_destino->payload), address, MAX_PAYLOAD_SIZE);

    bzero(pacote_recebido, MAX_PAYLOAD_SIZE);

    // std::cout << "Message_unmarshall(): END\n";
}

void Message_marshall(Message *msg_destino, Message *pacote_recebido) {
    // std::cout << "Message_marshall(): START\n";

	void *address = (void*)pacote_recebido;
   
    memcpy((unsigned int*) address,(void*) &(msg_destino->type), sizeof(msg_destino->type));
    address += sizeof(msg_destino->type);
    
    memcpy((unsigned int*) address,(void*) &(msg_destino->seqn), sizeof(msg_destino->seqn));
    address += sizeof(msg_destino->seqn);

    memcpy((char*) address,(void*) &(msg_destino->username), MAX_USERNAME_SIZE);
    address += MAX_USERNAME_SIZE;

    memcpy((char*) address,(void*) &(msg_destino->payload), MAX_PAYLOAD_SIZE);

    // std::cout << "Message_marshall(): END\n";
}

int Message_send(Message *msg, int sockfd) {
    // std::cout << "Message_send(): START on fd " << sockfd << "\n";
    Message *buffer_socket = (Message *) malloc(sizeof(Message));
    int bytes_sent;
    
    Message_marshall(msg,buffer_socket);
    if((bytes_sent = send(sockfd,(void*)buffer_socket,sizeof(Message),0)) != sizeof(Message)) {
        std::cerr << "Message_send(): send FAILED on fd " << sockfd << "\n";
        return -1;
    }

    std::cout << "\n\n\nMensagem Enviada on fd " << sockfd << "\n";
    std::cout << "msg.type: " << msg->type << "\n";
    std::cout << "msg.seqn: " << msg->seqn << "\n";
    std::cout << "msg.username: " << msg->username << "\n";
    std::cout << "msg.payload: " << msg->payload << "\n\n\n";

    free(buffer_socket);
    // std::cout << "Message_send(): END on fd " << sockfd << "\n";
    return bytes_sent;
}

int Message_recv(Message *msg, int sockfd) {
    // std::cout << "Message_recv(): START on fd " << sockfd << "\n";
    Message *pacote = (Message *)malloc(sizeof(Message));
    void *buffer_socket = (void *)malloc(MAX_UNREAD_BYTES);

    int bytes_recv = -1;

    int contador_bytes_recebidos = 0;

    // Copia o que tem no buffer geral antes de ler no socket
    if(curr_size_buffer_geral > sizeof(Message)){
        memcpy(buffer_socket, buffer_geral, sizeof(Message));
        curr_size_buffer_geral -= sizeof(Message);
        contador_bytes_recebidos = sizeof(Message);

        memcpy(buffer_geral, (buffer_geral+sizeof(Message)), curr_size_buffer_geral);
    }else{
        memcpy(buffer_socket, buffer_geral, curr_size_buffer_geral);
        curr_size_buffer_geral = 0;
        bzero(buffer_geral, MAX_UNREAD_BYTES);
    }

/home/grad/jcazeredo/Desktop/dropbox-sisop2-master/bin/./client jc localhost 8008
    // Enquanto nao recebe um pacote completo, vai lendo do socket. Quando completar uma mensagem inteira, retorna
    // Se receber mais que uma mensagem inteira, guarda no buffer_geral (global)
    while(contador_bytes_recebidos < sizeof(Message)){
        bytes_recv = read(sockfd, buffer_socket, sizeof(Message));
        
        if(bytes_recv == -1){
            free(buffer_socket);
            std::cerr << "Message_recv(): recv FAILED on fd "<< sockfd << " bytes_recv: " << bytes_recv << "\n";
            return -1;
        }

        contador_bytes_recebidos += bytes_recv;
            
        if(contador_bytes_recebidos == sizeof(Message)){
            memcpy((Message *)pacote, buffer_socket, sizeof(Message));
            free(buffer_socket);
            break;

        }else if(contador_bytes_recebidos > sizeof(Message)){
            int restante = contador_bytes_recebidos - sizeof(Message);

            memcpy((Message *)pacote, buffer_socket, sizeof(Message));
            buffer_socket += sizeof(Message);

            memcpy(buffer_geral, buffer_socket, restante);
            curr_size_buffer_geral += restante;

            free(buffer_socket);
            break;
        }

        buffer_socket += bytes_recv;
    }

    Message_unmarshall(msg, pacote);
    free(pacote);
        
    std::cout << "\n\n\nMensagem Recebida on fd " << sockfd << "\n";
    std::cout << "msg.type: " << msg->type << "\n";
    std::cout << "msg.seqn: " << msg->seqn << "\n";
    std::cout << "msg.username: " << msg->username << "\n";
    std::cout << "msg.payload: " << msg->payload << "\n";

	//free(buffer_socket);
    // std::cout << "Message_recv(): END on fd " << sockfd << "\n";
    return sizeof(Message);
}
