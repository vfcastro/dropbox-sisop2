#include <iostream>
#include <string.h>
#include "../../include/common/Message.h"

void Message_unmarshall(Message *msg, void *buffer) {
    std::cout << "Message_unmarshall(): START\n";

	void *address = buffer;
   
    memcpy((unsigned int*) &(msg->type), address, sizeof(msg->type));
    address += sizeof(msg->type);
    
    memcpy((unsigned int*) &(msg->seqn), address, sizeof(msg->seqn));
    address += sizeof(msg->seqn);

    memcpy((char*) &(msg->username), address, sizeof(msg->username));
    address += sizeof(msg->username);

    memcpy((char*) &(msg->payload), address, sizeof(msg->payload));

    std::cout << "Message_unmarshall(): START\n";
}

void Message_marshall(Message *msg, void *buffer) {
    std::cout << "Message_marshall(): START\n";

	void *address = buffer;
   
    memcpy((unsigned int*) address,(void*) &(msg->type), sizeof(msg->type));
    address += sizeof(msg->type);
    
    memcpy((unsigned int*) address,(void*) &(msg->seqn), sizeof(msg->seqn));
    address += sizeof(msg->seqn);

    memcpy((char*) address,(void*) &(msg->username), sizeof(msg->username));
    address += sizeof(msg->username);

    memcpy((char*) address,(void*) &(msg->payload), sizeof(msg->payload));

    std::cout << "Message_marshall(): START\n";
}
