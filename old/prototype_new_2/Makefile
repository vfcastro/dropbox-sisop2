CLIENT := ./client
SERVER := ./server

LIBS := ./libs/

CLI_LIBS := $(LIBS)ClientInterface.cpp $(LIBS)event_queue.cpp $(LIBS)inotify.cpp $(LIBS)Notifier.cpp 
CLI_LIBS += $(LIBS)RcvAction.cpp $(LIBS)Receiver.cpp $(LIBS)SendAction.cpp

SVR_LIBS := 

program: client server

client:
	clear
	@echo "## Compiling Client... ## \n"
	g++ -pthread client.cpp $(CLI_LIBS) -fpermissive -o client
	@echo "\n"

server:
	@echo "## Compiling Server... ## \n"
	g++ -pthread server.cpp $(SVR_LIBS) -fpermissive -o server

clean:
	clear
	@echo "Removing Files...\n"
	rm -f *.o $(CLIENT) $(SERVER)
