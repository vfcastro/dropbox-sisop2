#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../../include/common/Message.h"

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload) {
	// std::cout << "Message_create(): START\n";
	Message *msg;

    if((msg = (Message*)malloc(sizeof(Message))) == NULL){
        std::cerr << "Message_create(): ERROR malloc msg\n";
    }

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
    Message *buffer_socket;

    if((buffer_socket = (Message*)malloc(sizeof(Message))) == NULL){
        std::cerr << "Message_send(): ERROR malloc msg\n";
    }

    int bytes_sent;
    
    Message_marshall(msg, buffer_socket);
    if((bytes_sent = send(sockfd,(void*)buffer_socket,sizeof(Message),MSG_NOSIGNAL)) != sizeof(Message)) {
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
    void *buffer = (void*)malloc(sizeof(Message));
    int bytes_read;
    int bytes_pending = sizeof(Message);

    while(bytes_pending > 0)
    {
        bytes_read = read(sockfd,buffer,bytes_pending);

        // Erro na leitura
        if(bytes_read == -1)
        {
            std::cerr << "Message_recv: ERROR reading message from socket" << sockfd << "\n";
            free(buffer);
            return -1;
        }

        bytes_pending -= bytes_read;
        buffer += bytes_read;
    }
    buffer -= sizeof(Message);

    Message_unmarshall(msg, (Message*)buffer);
    free(buffer);

    std::cout << "\n\n\nMensagem Recebida on fd " << sockfd << "\n";
    std::cout << "msg.type: " << msg->type << "\n";
    std::cout << "msg.seqn: " << msg->seqn << "\n";
    std::cout << "msg.username: " << msg->username << "\n";
    std::cout << "msg.payload: " << msg->payload << "\n";

    // std::cout << "Message_recv(): END on fd " << sockfd << "\n";
    return sizeof(Message);
}
