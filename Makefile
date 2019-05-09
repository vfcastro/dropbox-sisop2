all:
	g++ src/Server.cpp src/ServerCommunicator.cpp -o bin/server -lpthread
