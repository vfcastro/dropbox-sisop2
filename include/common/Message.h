#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MAX_PAYLOAD_SIZE 100
#define MAX_USERNAME_SIZE 20

// Message types:
#define OK 0
#define NOK 1
#define OPEN_SEND_CONN 2
#define OPEN_RECV_CONN 3

struct Message {
	unsigned int type;
	unsigned int seqn; // 0 if the last, > 0 otherwise
	char username[MAX_USERNAME_SIZE];
	char payload[MAX_PAYLOAD_SIZE];
};

void Message_marshall(Message *msg, void *buffer);
void Message_unmarshall(Message *msg, void *buffer);

#endif
