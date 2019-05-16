#include <iostream>
#include <dirent.h>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include "../../include/client/ClientInterface.h"
#include "../../include/common/FileManager.h"

void ClientInterface_start(ClientCommunicator *cc) {
	// std::cout << "ClientInterface_start(): START\n";
	bool exit = false;
	std::string command;

	while(!exit){
		std::cout<< "Commands Available:"<<std::endl;
		std::cout<< "\tupload [filepath]"<<std::endl;
		std::cout<< "\tdownload [filepath]"<<std::endl;
		std::cout<< "\tdelete [filepath]"<<std::endl;
		std::cout<< "\tlist_server"<<std::endl;
		std::cout<< "\tlist_client"<<std::endl;
		std::cout<< "\texit"<<std::endl;
		std::cout << "#: ";
		std::getline(std::cin,command);
		if(!command.empty())
			ClientInterface_command(command, &exit, cc);
	}

	// std::cout << "ClientInterface_start(): STOP\n";
}

void ClientInterface_command(std::string command, bool* exit, ClientCommunicator *cc){
	std::vector<std::string> cmdAndArg;
	std::istringstream f(command);
	std::string s;

	while(getline(f, s, ' ')){
		cmdAndArg.push_back(s);
	}

	if(cmdAndArg[0].compare("upload")==0){
		if(cmdAndArg.size()>1){
			// std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			ClientInterface_upload(cc, cmdAndArg[1]);
		}
		else{
			std::cout<<"--formato:"<<cmdAndArg[0]<<" <filepath>"<<std::endl;
		}

	}
	else if(cmdAndArg[0].compare("download")==0){
		if(cmdAndArg.size()>1){
			// std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			ClientInterface_download(cc, cmdAndArg[1]);
		}
		else{
			std::cout<<"--formato:"<<cmdAndArg[0]<<" <filename>"<<std::endl;
		}
	}
	else if(cmdAndArg[0].compare("delete")==0){
		if(cmdAndArg.size()>1){
			// std::cout<<cmdAndArg[0]<<" "<<cmdAndArg[1]<<" called"<<std::endl;
			ClientInterface_delete(cc, cmdAndArg[1]);
		}
		else{
			std::cout<<"--formato:"<< cmdAndArg[0] <<" <filepath>"<<std::endl;
		}
	}
	else if(cmdAndArg[0].compare("list_server")==0){
		// std::cout<<"list_server called"<<std::endl;
		ClientInterface_listServer(cc);
	}
	else if(cmdAndArg[0].compare("list_client")==0){
		// std::cout<<"list_client called"<<std::endl;
		ClientInterface_listClient(cc);
	}
	else if(cmdAndArg[0].compare("get_sync_dir")==0){
		// std::cout<<"list_client called"<<std::endl;
		ClientInterface_getSyncDir(cc);
	}
	else if(cmdAndArg[0].compare("exit")==0){
		*exit=true;
		std::cout<<"Closing..."<<std::endl;
		ClientInterface_exit(cc);
	}
	else if(cmdAndArg[0].compare("help")==0){
		std::cout<< "Commands Available:"<<std::endl;
		std::cout<< "\tupload [filepath]"<<std::endl;
		std::cout<< "\tdownload [filepath]"<<std::endl;
		std::cout<< "\tdelete [filepath]"<<std::endl;
		std::cout<< "\tlist_server"<<std::endl;
		std::cout<< "\tlist_client"<<std::endl;
		std::cout<< "\texit"<<std::endl;
	}
	else {
		std::cout<<"Invalid command"<<std::endl;
	}
}

void ClientInterface_upload(ClientCommunicator *cc, std::string filepath){
	// std::cout << "ClientInterface_upload(): START for " << filepath << std::endl;

	std::vector<std::string> filename_v;
	std::istringstream fs(filepath);
	std::string s;

	while(getline(fs, s, '/')){
		filename_v.push_back(s);
	}

	std::string filename(filename_v[filename_v.size()-1]);

	Message *msg = Message_create(UPLOAD_FILE_CMD, 0, cc->username, (const char *)filename.c_str());
	// Envia msg dizendo que quer fazer upload
	Message_send(msg, cc->sendsockfd);

	// Aguarda um Ok do server
	if(Message_recv(msg, cc->sendsockfd) == -1){
		std::cerr << "ClientInterface_upload(): ERROR recv -1 ";
	}

	int f;

	if((f = open(filepath.c_str(), O_RDONLY)) == -1){
		std::cerr << "ClientInterface_upload(): ERROR opening file " << filepath << "\n";
		return;
	}

	int bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	
	// Envia arquivo pro servidor
	while(bytes_recv){
		// std::cout << "ClientSync_onCloseWrite(): read " << bytes_recv << " bytes from file " << filepath << "\n";
		msg->seqn = bytes_recv;
		Message_send(msg, cc->sendsockfd);
		bytes_recv = read(f, msg->payload, MAX_PAYLOAD_SIZE);
	}

	msg->type = END;
	Message_send(msg, cc->sendsockfd);

	close(f);

	// std::cout << "ClientInterface_upload(): END for " << filepath << std::endl;
}

void ClientInterface_download(ClientCommunicator *cc, std::string filename){
	// std::cout << "ClientInterface_download(): " << filename << std::endl;

	Message *msg = Message_create(DOWNLOAD_FILE_CMD, 0, cc->username, (const char *)filename.c_str());
	// Envia msg dizendo que quer fazer download
	Message_send(msg, cc->sendsockfd);

	// Aguarda um Ok do server
	if(Message_recv(msg, cc->sendsockfd) == -1){
		std::cerr << "ClientInterface_download(): ERROR recv -1 ";
	}

	// std::cout << "ClientInterface_download(): creating file " << filename << "\n";

	int f = open((char*)filename.c_str(),O_CREAT|O_WRONLY,0600);
	
	if(f == -1){
		std::cerr << "ClientInterface_download(): ERROR creating file " << filename << "\n";
		return;
	}

	//Preenche o arquivo conforme recebimento das mensagens
	while(Message_recv(msg,cc->sendsockfd) != -1) {
		// Verifica se tipo = OK, se sim, para de escrever
		if(msg->type == END){
			break;
		}

		// std::cout << "ClientInterface_download(): recv payload with " << msg->seqn << " bytes\n";
		
		if(write(f,(const void *)msg->payload, msg->seqn) == -1){
			exit(6);
		}
		
		msg->type = OK;
	}

	close(f);


}

void ClientInterface_delete(ClientCommunicator *cc, std::string filename){
	// std::cout<<"ClientInterface_delete(): Delete File " << filename << "\n";

	std::string path("./sync_dir_");
	path.append(cc->username).append("/");
	path.append(filename);
	
	Message *msg = Message_create(DELETE_FILE, 0, cc->username, filename.c_str());
	Message_send(msg, cc->sendsockfd);
	Message_recv(msg, cc->sendsockfd);

	int Removed = std::remove(path.c_str());

}

void ClientInterface_listServer(ClientCommunicator *cc){
	// std::cout << "ClientInterface_listServer(): START";
	std::vector<std::string> list_files;

	Message *msg = Message_create(LIST_SERVER_CMD, 0, cc->username, "");
	// Envia msg dizendo que quer lista
	Message_send(msg, cc->sendsockfd);

	// Aguarda um Ok do server
	if(Message_recv(msg, cc->sendsockfd) == -1){
		std::cerr << "ClientInterface_listServer(): ERROR recv -1 ";
	}

	// Cada pacote recebido é uma linha do "ls -l" executado no server
	while(Message_recv(msg, cc->sendsockfd) != -1) {
		// Verifica se tipo = END, se sim, para de dar read
		if(msg->type == END){
			break;
		}

		// adiciona cada linha num vetor
		// std::cout << "Recebendo: " << msg->payload;
		list_files.push_back(msg->payload);
	}


	std::cout << "\n\nList Server result: " << "\n";
	// imprime as linhas do "ls -l" na tela do cliente
	for (int i = 0; i < list_files.size(); ++i){
		std::cout << list_files[i] << "\n";
	}
	std::cout << "\n\n";
}

void ClientInterface_listClient(ClientCommunicator *cc){
	// std::cout << "ClientInterface_listClient: START\n\n";
	
	std::string path("./sync_dir_");
	path.append(cc->username).append("/");
	char output[MAX_PAYLOAD_SIZE];

	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> filelist;

	if((dir = opendir(path.c_str())) != NULL) {
  		while ((ent = readdir (dir)) != NULL) {
  			if((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0))
    			filelist.push_back(ent->d_name);
  		}
 		closedir (dir);
	} else {
	
	perror ("");
	
	}

	FILE *fp;
	
	std::cout << "\n\nList Client result: " << "\n";

	for (int i = 0; i < filelist.size(); ++i){
		std::string cmd("stat --printf='M: %y | A: %x | C: %w ' ");
		cmd.append(path);
		cmd.append(filelist[i]);

		std::string payload("");
		payload.append(filelist[i]);
		payload.append(" | ");

		fp = popen(cmd.c_str(), "r");
		if (fp == NULL) {
			std::cerr << "Failed to run command \n";
			exit(1);
	  	}

	  	fgets(output, MAX_PAYLOAD_SIZE-1, fp);
	  	payload.append(output);

	    std::cout << payload << "\n";

	  	/* close */
	  	pclose(fp);
  	}
  	std::cout << "\n\n";
}

void ClientInterface_exit(ClientCommunicator *cc){
	// std::cout << "ClientInterface_exit(): START";
	Message *msg = Message_create(USER_EXIT, 0, cc->username, "");
	Message_send(msg,cc->sendsockfd);
	exit(0);
}

void ClientInterface_getSyncDir(ClientCommunicator *cc){
std::cout << "GET_SYNC_DIR: Sincronizando arquivos....\n";
	int socket = cc->sendsockfd;

	Message *msg = Message_create(GET_SYNC_DIR, 0, cc->username, std::string().c_str());

	// Envia Requisição
	Message_send(msg, socket);

	// Aguarda Ok ou aviso que nao ha arquivos
	Message_recv(msg, socket);
	if(msg->type == NOK){
		return;
	}

	int receive_result = 0;
	int type = 0;
	// Recebe nome do arquivo

	while(Message_recv(msg, socket) != -1){

		type = msg->type;

		std::string path("./sync_dir_");
		path.append(cc->username).append("/");
		path.append(msg->payload);

		receive_result = FileManager_receiveFile(path, msg, cc->sendsockfd);

		if(receive_result == -1){
			std::cout<<"ClientInterface_getSyncDir(): Error Send File\n";
			break;
		}

		Message_recv(msg, socket);
		if(msg->type == END_SYNC){
			break;
		}
	}
	return;
}