#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 4029
#define BACKLOG 2

int num_connections = 0;

void* process(void * sockfd)
{	
	char buffer[256];
	int *newsockfd = (int *)sockfd;
	int n;

	printf("Conexao Numero %d\n", num_connections);

	// Esse for eh so pra limitar o numero de msgs pra teste
	for (int i = 0; i < 5; ++i){
		bzero(buffer, 256);

		n = read(*newsockfd, buffer, 256);

		if (n < 0) 
			printf("ERROR reading from socket");

		printf("Here is the message: %s\n", buffer);
		
		/* write in the socket */ 
		n = write(*newsockfd,"I got your message", 18);
		if (n < 0) 
			printf("ERROR writing to socket");

	}
	
	printf("Conexao Numero %d Fechada\n", num_connections);

	close(*newsockfd);

	free(newsockfd);

	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr;
	pthread_t thread;
	int *newsockfd = NULL;
	struct sockaddr * connection = NULL;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(sockfd, BACKLOG);	

	printf("Esperando conexoes...\n");
	
	clilen = sizeof(struct sockaddr_in);
	
	while (1){
		
		connection = (struct sockaddr *)malloc(clilen);
		newsockfd = (int*)malloc(sizeof(int));

		if ((*newsockfd = accept(sockfd, connection, &clilen)) == -1){
			printf("ERROR on accept");
			free(connection);
		}
		else{
			num_connections++;
			pthread_create(&thread, 0, process, (void *)newsockfd);
			pthread_detach(thread);
		}
	}
	
	return 0;  
}