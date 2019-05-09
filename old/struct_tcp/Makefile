EXECLT = ./cliente/client
EXESVR = ./servidor/server

CC = g++

program: cliente server

cliente: client.o support.o serialize.o
	$(CC) -Wall -o $(EXECLT) support.o serialize.o client.o

server: server.o support.o serialize.o
	$(CC) -Wall -o $(EXESVR) support.o serialize.o server.o

client.o: client.c
	$(CC) -Wall -c client.c

server.o: server.c
	$(CC) -Wall -c server.c

support.o: support.c
	$(CC) -Wall -c support.c

serialize.o: serialize.c
	$(CC) -Wall -c serialize.c

clean:
	rm -f *.o $(EXESVR) $(EXECLT)