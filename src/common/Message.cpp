#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../../include/common/Message.h"

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload) {
	// std::cout << "Message_create(): START\n";
	Message *msg = (Message*) malloc(sizeof(Message));
	msg->type = type;
	msg->seqn = seqn;
	memcpy((void*)msg->username,(void*)username,MAX_USERNAME_SIZE);
	memcpy((void*)msg->payload,(void*)payload,MAX_PAYLOAD_SIZE);

	std::cout << "Message_create(): type:" << msg->type << " seqn:" << msg->seqn << " username:" << msg->username << " payload:" << msg->payload <<"\n";
	return msg;
}

void Message_unmarshall(Message *msg, Message *buffer) {
    // std::cout << "Message_unmarshall(): START\n";

	void *address = (void*)buffer;
   
    memcpy((unsigned int*) &(msg->type), address, sizeof(msg->type));
    address += sizeof(msg->type);
    
    memcpy((unsigned int*) &(msg->seqn), address, sizeof(msg->seqn));
    address += sizeof(msg->seqn);

    memcpy((char*) &(msg->username), address, MAX_USERNAME_SIZE);
    address += MAX_USERNAME_SIZE;

    memcpy((char*) &(msg->payload), address, MAX_PAYLOAD_SIZE);

    bzero(buffer, MAX_PAYLOAD_SIZE);

    // std::cout << "Message_unmarshall(): END\n";
}

void Message_marshall(Message *msg, Message *buffer) {
    // std::cout << "Message_marshall(): START\n";

	void *address = (void*)buffer;
   
    memcpy((unsigned int*) address,(void*) &(msg->type), sizeof(msg->type));
    address += sizeof(msg->type);
    
    memcpy((unsigned int*) address,(void*) &(msg->seqn), sizeof(msg->seqn));
    address += sizeof(msg->seqn);

    memcpy((char*) address,(void*) &(msg->username), MAX_USERNAME_SIZE);
    address += MAX_USERNAME_SIZE;

    memcpy((char*) address,(void*) &(msg->payload), MAX_PAYLOAD_SIZE);

    // std::cout << "Message_marshall(): END\n";
}

int Message_send(Message *msg, int sockfd) {
    // std::cout << "Message_send(): START on fd " << sockfd << "\n";
    Message *buffer = (Message *) malloc(sizeof(Message));
    int bytes_sent;
    
    Message_marshall(msg,buffer);
    if((bytes_sent = send(sockfd,(void*)buffer,sizeof(Message),0)) != sizeof(Message)) {
        std::cerr << "Message_send(): send FAILED on fd " << sockfd << "\n";
        return -1;
    }

    // std::cout << "\n\n\nMensagem Enviada on fd " << sockfd << "\n";
    // std::cout << "msg.type: " << msg->type << "\n";
    // std::cout << "msg.seqn: " << msg->seqn << "\n";
    // std::cout << "msg.username: " << msg->username << "\n";
    // std::cout << "msg.payload: " << msg->payload << "\n\n\n";

    //free(buffer);
    // std::cout << "Message_send(): END on fd " << sockfd << "\n";
    return bytes_sent;
}

int Message_recv(Message *msg, int sockfd) {
    // std::cout << "Message_recv(): START on fd " << sockfd << "\n";
    Message *buffer = (Message *) malloc(sizeof(Message));
    int bytes_recv;

    if((bytes_recv = read(sockfd,(void*)buffer,sizeof(Message))) == -1) {
		free(buffer);
        std::cerr << "Message_recv(): recv FAILED on fd "<< sockfd << " bytes_recv: " << bytes_recv << "\n";
        return -1;
    }
    Message_unmarshall(msg,buffer);
        
    // std::cout << "\n\n\nMensagem Recebida on fd " << sockfd << "\n";
    // std::cout << "msg.type: " << msg->type << "\n";
    // std::cout << "msg.seqn: " << msg->seqn << "\n";
    // std::cout << "msg.username: " << msg->username << "\n";
    // std::cout << "msg.payload: " << msg->payload << "\n";

	//free(buffer);
    // std::cout << "Message_recv(): END on fd " << sockfd << "\n";
    return bytes_recv;
}
