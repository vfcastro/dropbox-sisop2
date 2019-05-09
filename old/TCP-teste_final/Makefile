EXECLT = client
EXESVR = server

CC = g++

program: cliente server

cliente: client.o
	$(CC) -Wall -o $(EXECLT) client.o	

server: server.o
	$(CC) -Wall -o $(EXESVR) server.o -lpthread
clean:
	rm -f *.o $(EXESVR) $(EXECLT)