#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "serialize.h"
#include "support.h"

Pacote unserialize_pacote(void *buffer){
	Pacote p_unserialized;

	void* address = buffer;

	memcpy((int*) &(p_unserialized.cmd), address, sizeof(p_unserialized.cmd));
	address += sizeof(p_unserialized.cmd);

	memcpy((int*) &(p_unserialized.tamanho), address, sizeof(p_unserialized.tamanho));
	address += sizeof(p_unserialized.tamanho);

	memcpy((char*) &(p_unserialized.nome), address, sizeof(p_unserialized.nome));
	address += sizeof(p_unserialized.nome);

	memcpy((char*) &(p_unserialized.msg), address, p_unserialized.tamanho);

	p_unserialized.msg[p_unserialized.tamanho] = '\0';

	return p_unserialized;

}


void* serialize_pacote(Pacote *pkg){
	void *buf_prepare;
	void *address;

	buf_prepare = (void *) malloc(sizeof(Pacote));
	bzero(buf_prepare,sizeof(Pacote));
	address = buf_prepare;

	memcpy((int*) address, (void*) &(pkg->cmd), sizeof(pkg->cmd));
	address += sizeof(pkg->cmd);

	memcpy((int*) address, (void*) &(pkg->tamanho), sizeof(pkg->tamanho));
	address += sizeof(pkg->tamanho);

	memcpy((char*) address, (void*) &(pkg->nome), sizeof(pkg->nome));
	address += sizeof(pkg->nome);

	memcpy((char*) address, (void*) &(pkg->msg), pkg->tamanho);

	return buf_prepare;
}
