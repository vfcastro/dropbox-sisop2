#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MAX_PAYLOAD_SIZE 500
#define MAX_USERNAME_SIZE 100

// Message types:
#define OK 0
#define NOK 1
#define OPEN_SEND_CONN 2
#define OPEN_RECV_CONN 3
#define OPEN_SESSION 4
#define CREATE_FILE 5
#define FILE_CLOSE_WRITE 6
#define END 7

struct Message {
	unsigned int type;
	unsigned int seqn; // 0 if the last, > 0 otherwise
	char username[MAX_USERNAME_SIZE];
	char payload[MAX_PAYLOAD_SIZE];
	unsigned int size;
};

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload, int size);
void Message_marshall(Message *msg, void *buffer);
void Message_unmarshall(Message *msg, void *buffer);
int Message_send(Message *msg, int sockfd);
int Message_recv(Message *msg, int sockfd);

#endif
