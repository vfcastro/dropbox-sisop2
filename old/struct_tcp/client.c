#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <string.h> 
#include <unistd.h>
#include <fcntl.h>

#include "support.h"
#include "serialize.h"
   
int main(int argc, char const *argv[]) 
{ 
    // struct sockaddr_in address; 
    int sock = 0; 
    struct sockaddr_in serv_addr; 

    char buffer_resposta[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 


	void *buf2snd;
	Pacote p_envio;
    int fd;
    long int num_bytes_read;
    long unsigned int bytes_send = 0;
    int num_pkg_send = 0;

    // fd = file descriptor, abre arquivo a ser enviado
    if ((fd = open(argv[1], O_RDONLY))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}

    // Fica lendo BUFFERT bytes por vez, salva em p_envio.msg
    num_bytes_read = read(fd, p_envio.msg, BUFFERT);
	while(num_bytes_read){
		if(num_bytes_read == -1){
			perror("Read Fails");
			return EXIT_FAILURE;
		}

		strcpy(p_envio.nome, argv[1]);

		p_envio.cmd = 7;
		p_envio.tamanho = num_bytes_read;
        p_envio.msg[p_envio.tamanho] = '\0';

        printf("Enviando o pacote: \n");
        printf("\tcomando: %d\n", p_envio.cmd);
        printf("\ttamanho msg: %d\n", p_envio.tamanho);   
        printf("\tnome arquivo: %s\n", p_envio.nome);
        printf("\tmsg (%ld): %s\n", strlen(p_envio.msg), p_envio.msg);

        // converte uma struct para uma sequencia de bytes
		buf2snd = serialize_pacote(&p_envio);

        // envia struct serializada para server
        send(sock, buf2snd , sizeof(Pacote), 0); 
        bytes_send += sizeof(Pacote);
        num_pkg_send ++;

        // Aguarda resposta do server
	    read(sock, buffer_resposta, 1024); 
        printf("\nResposta do Server: %s\n", buffer_resposta); 

        printf("\n===================\n");
        
        // Lê proximos bytes do arquivo
        num_bytes_read = read(fd, p_envio.msg, BUFFERT);
	}
    close(fd);

    printf("\n#### Stats #####\n");
    printf("Numero de Bytes Enviados: %lu\n", bytes_send);
    printf("Numero Pacotes Enviados: %d\n", num_pkg_send);

    return 0; 
} 


