#ifndef __SUPPORT__
#define __SUPPORT__

#define BUFFERT 5
#define PORT 5005 

typedef struct dados_envio{
	char nome[60];
	int cmd;
	int tamanho;
	char msg[BUFFERT];
} Pacote;

#endif