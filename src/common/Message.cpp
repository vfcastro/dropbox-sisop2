#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../../include/common/Message.h"

void *buffer_geral = (void *)malloc(MAX_UNREAD_BYTES);
int curr_size_buffer_geral = 0;

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
    if((bytes_sent = send(sockfd,(void*)buffer_socket,sizeof(Message),0)) != sizeof(Message)) {
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
    // std::cout << "Message_recv(): START on fd " << sockfd << "\n";
    
    std::cout << "\n\n####### Iniciando Recebimento de um pacote\n";

    Message *pacote;
    void *buffer_socket;
    void *buffer_socket_inicio;

    if((pacote = (Message *)malloc(sizeof(Message))) == NULL){
        std::cerr << "Message_recv(): ERROR malloc pacote\n";
    }

    if((buffer_socket = (void *)malloc(MAX_UNREAD_BYTES)) == NULL){
        std::cerr << "Message_recv(): ERROR malloc buffer_socket\n";
    }

    buffer_socket_inicio = buffer_socket;

    int bytes_recv = -1;

    int contador_bytes_recebidos = 0;

    // Copia o que tem no buffer geral antes de ler no socket
    if(curr_size_buffer_geral > sizeof(Message)){
        std::cout << "\n\n####### Tamanho Buffer Geral igual sizeof(message): " << curr_size_buffer_geral << sizeof(Message) << std::endl;
        memcpy(buffer_socket_inicio, buffer_geral, sizeof(Message));
        curr_size_buffer_geral -= sizeof(Message);
        
        buffer_socket += sizeof(Message);

        // Entra no while tendo lido algo já
        contador_bytes_recebidos = sizeof(Message);

        memcpy(buffer_geral, (buffer_geral+sizeof(Message)), curr_size_buffer_geral);

        std::cout << "\n\n####### Copia no buffer_socket uma mensagem inteira e move no buffer geral\n";
        std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
        std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";

    }else{
        // Se o buffer geral tiver tamanho igual ou menor de uma mensagem
        std::cout << "\n\n#######  Tamanho Buffer Geral: " << curr_size_buffer_geral << std::endl;
        memcpy(buffer_socket_inicio, buffer_geral, curr_size_buffer_geral);
        buffer_socket += curr_size_buffer_geral;
        
        // Entra no while tendo lido algo já
        contador_bytes_recebidos = curr_size_buffer_geral;

        curr_size_buffer_geral = 0;

        bzero(buffer_geral, MAX_UNREAD_BYTES);
        std::cout << "\n\n####### Copia no buffer_socket tudo que tem no buffer geral\n";
        std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
        std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";
    }

    std::cout << "\n\n####### Antes do while\n";
    std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
    std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";
        

    // Enquanto nao recebe um pacote completo, vai lendo do socket. Quando completar uma mensagem inteira, retorna
    // Se receber mais que uma mensagem inteira, guarda no buffer_geral (global)
    while(contador_bytes_recebidos < sizeof(Message)){
        std::cout << "\n\n#######  entrando no while do recv" << std::endl;
        bytes_recv = read(sockfd, buffer_socket_inicio, sizeof(Message));
        
        if(bytes_recv == -1){
            std::cout << "\n\n####### linha 134 free" << std::endl;
            free(buffer_socket_inicio);
            std::cerr << "Message_recv(): recv FAILED on fd "<< sockfd << " bytes_recv: " << bytes_recv << "\n";
            return -1;
        }

        contador_bytes_recebidos += bytes_recv;
        
        std::cout << "\n\n####### Meio do While\n";
        std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
        std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";

        if(contador_bytes_recebidos == sizeof(Message)){
            std::cout << "\n\n####### Bytes Recebidos == Mensagem\n";
            std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
            std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";


            memcpy((Message *)pacote, buffer_socket_inicio, sizeof(Message));
            std::cout << "\n\n####### linha 149 free - recebeu == size message" << std::endl;
            free(buffer_socket_inicio);
            
            std::cout << "\n\n####### Guardou no Pacote\n";
            std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
            std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";
            break;

        }else if(contador_bytes_recebidos > sizeof(Message)){
            std::cout << "\n\n####### Bytes Recebidos > Mensagem\n";
            std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
            std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";

            int restante = contador_bytes_recebidos - sizeof(Message);
            std::cout << "\n\n#######  restante: " << restante << std::endl;            

            memcpy((Message *)pacote, buffer_socket_inicio, sizeof(Message));
            
            std::cout << "\n\n#######  consegui copiar para o pacote: " << std::endl; 

            memcpy(buffer_geral, buffer_socket_inicio + sizeof(Message), restante);
            curr_size_buffer_geral += restante;
            std::cout << "\n\n####### linha 157 free" << std::endl;
            free(buffer_socket_inicio);

            std::cout << "\n\n####### Guardou parte no Pacote e Restante no Buffer Geral\n";
            std::cout << "####### contador_bytes_recebidos: " << contador_bytes_recebidos << "\n";
            std::cout << "####### curr_size_buffer_geral: " << curr_size_buffer_geral << "\n";
            break;
        }

        buffer_socket += bytes_recv;
        std::cout << "\n\n#######  fim no while do recv" << std::endl;
    }
    
    std::cout << "\n\n#######  fora do while do recv" << std::endl;

    Message_unmarshall(msg, pacote);
    std::cout << "\n\n####### linha 166 free depois unmarshall" << std::endl;
    free(pacote);

    std::cout << "\n\n\nMensagem Recebida on fd " << sockfd << "\n";
    std::cout << "msg.type: " << msg->type << "\n";
    std::cout << "msg.seqn: " << msg->seqn << "\n";
    std::cout << "msg.username: " << msg->username << "\n";
    std::cout << "msg.payload: " << msg->payload << "\n";

    if(msg->type > 15 || msg->type < 0){
           
        return -1;
        exit(0);
    }

	//free(buffer_socket);
    // std::cout << "Message_recv(): END on fd " << sockfd << "\n";
    return sizeof(Message);
}
