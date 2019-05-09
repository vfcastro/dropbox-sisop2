#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "support.h"
#include "serialize.h"

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address);
    
    char *resposta = (char*)malloc(sizeof(char)*50);
    strcpy(resposta, "Recebi seu Pacote");
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
        perror("socket failed");
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, 
                                                  &opt, sizeof(opt))){ 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
    
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0){ 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0){ 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

        Pacote p_recebe;

        void *buf2rcv = (void *) malloc(sizeof(Pacote));

        int first = 1;
        int fd;

        long unsigned int bytes_recv = 0;
        int num_pkg_recv = 0;

        while((valread = read(new_socket, buf2rcv, sizeof(Pacote)))){

            bytes_recv += sizeof(Pacote);
            num_pkg_recv++;
            // transforma sequencia de bytes recebida de volta em struct
            p_recebe = unserialize_pacote(buf2rcv);

            printf("Recebendo Pacote:\n");
            printf("\tcomando: %d\n", p_recebe.cmd);
            printf("\ttamanho msg: %d\n", p_recebe.tamanho);   
            printf("\tnome arquivo: %s\n", p_recebe.nome);
            printf("\tmsg (%ld): %s\n", strlen(p_recebe.msg), p_recebe.msg);

            // No primeiro pacote recebido, cria o arquivo
            if(first == 1){
                if ((fd=open(p_recebe.nome,O_CREAT|O_WRONLY,0600))==-1){
                    perror("open fail");
                    exit (3);
                }
                first = 0;
            }

            if((write(fd, p_recebe.msg, p_recebe.tamanho)) == -1){
                perror("write fail");
                exit (6);
            }

            send(new_socket , resposta , strlen(resposta) , 0 ); 

            printf("\n===================\n");
        }
        close(fd);

    printf("\n#### Stats #####\n");
    printf("Numero de Bytes Recebidos: %lu\n", bytes_recv);
    printf("Numero Pacotes Recebidos: %d\n", num_pkg_recv);


    return 0; 
} 
