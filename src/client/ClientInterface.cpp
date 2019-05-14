#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "../../include/client/ClientInterface.h"

void ClientInterface_start(ClientCommunicator *cc) {
	std::cout << "ClientInterface_start(): START\n";
	bool exit = false;
	std::string command;

	while(!exit){
		std::cout << "$ ";
		std::getline(std::cin,command);
		ClientInterface_command(command,&exit);
	}

	std::cout << "ClientInterface_start(): STOP\n";
}

void ClientInterface_command(std::string command,bool* exit){
	std::vector<std::string> cmdAndArg;
	std::istringstream f(command);
	std::string s;

	while(getline(f, s, ' ')){            //separa o commando do argumento e coloca no vetor
		cmdAndArg.push_back(s);
	}

	if(cmdAndArg[0].compare("update")==0){
		if(cmdAndArg.size()>1){
			std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			//CHAMAR AQUI A FUNCAO UPDATE
		}
		else{
			std::cout<<"--formato:"<<cmdAndArg[0]<<"[filepath]"<<std::endl;
		}

	}
	else if(cmdAndArg[0].compare("download")==0){
		if(cmdAndArg.size()>1){
			std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			//CHAMAR AQUI A FUNCAO DOWNLOAD
		}
		else{
			std::cout<<"--formato:"<<cmdAndArg[0]<<"[filename]"<<std::endl;
		}
	}
	else if(cmdAndArg[0].compare("delete")==0){
		if(cmdAndArg.size()>1){
			std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			//CHAMAR AQUI A FUNCAO DELETE
		}
		else{
			std::cout<<"--formato:"<<cmdAndArg[0]<<"[filepath]"<<std::endl;
		}
	}
	else if(cmdAndArg[0].compare("list_server")==0){
		std::cout<<"list_server called"<<std::endl;
		//CHAMAR AQUI list_server
	}
	else if(cmdAndArg[0].compare("exit")==0){
		*exit=true;
		std::cout<<"exiting"<<std::endl;
	}
	else if(cmdAndArg[0].compare("help")==0){
		std::cout<< "Commands Available:"<<std::endl;
		std::cout<< "\tupdate [filepath]"<<std::endl;
		std::cout<< "\tdownload [filepath]"<<std::endl;
		std::cout<< "\tdelete [filepath]"<<std::endl;
		std::cout<< "\tlist_server"<<std::endl;
		std::cout<< "\texit"<<std::endl;
	}
	else {
		std::cout<<"Invalid command"<<std::endl;
	}
}
