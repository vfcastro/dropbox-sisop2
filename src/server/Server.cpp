#include <iostream>
#include <string>
#include "../include/ServerCommunicator.h"

int main (int argc, char **argv)
{
	unsigned int port = std::stoul(argv[1]);
	unsigned int backlog = 100;
	ServerCommunicator serverCommunicator(port,backlog);
	std::thread serverCommunicatorThread;

    s.addThread(std::thread(&ServerCommunicator::));
	

}

