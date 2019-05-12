#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MAX_PAYLOAD_SIZE 10
#define MAX_USERNAME_SIZE 10

// Message types:
#define OK 0
#define NOK 1
#define OPEN_SEND_CONN 2
#define OPEN_RECV_CONN 3
#define OPEN_SESSION 4
#define CREATE_FILE 5

struct Message {
	unsigned int type;
	unsigned int seqn; // 0 if the last, > 0 otherwise
	char *username;
	char *payload;
};

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload);
void Message_marshall(Message *msg, void *buffer);
void Message_unmarshall(Message *msg, void *buffer);
int Message_send(Message *msg, int sockfd);
int Message_recv(Message *msg, int sockfd);

#endif
