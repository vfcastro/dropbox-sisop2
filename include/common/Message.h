#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MAX_UNREAD_BYTES 4096 

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
#define DELETE_FILE 8
#define UPLOAD_FILE_CMD 9
#define DOWNLOAD_FILE_CMD 10 
#define LIST_SERVER_CMD 11
#define S2C_PROPAGATE 12
#define GET_SYNC_DIR 13
#define END_SYNC 14
#define USER_EXIT 15


struct Message {
	unsigned int type;
	unsigned int seqn; // 0 if the last, > 0 otherwise
	char username[MAX_USERNAME_SIZE];
	char payload[MAX_PAYLOAD_SIZE];
};

Message* Message_create(unsigned int type, unsigned int seqn, const char *username, const char *payload);
void Message_marshall(Message *msg_destino, Message *pacote_recebido);
void Message_unmarshall(Message *msg_destino, Message *pacote_recebido);
int Message_send(Message *msg, int sockfd);
int Message_recv(Message *msg, int sockfd);

#endif
