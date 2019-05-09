#include <iostream>
#include <cstring>
#include "SendAction.cpp"

class ClientInterface{
	public:
	    std::string chooseAction;
	    std::string askFilepath;
	    std::string invalidAction;
	    std::string actionChosen;
	    std::string filepath;
	    bool exitCalled;
	    SendAction send_action;

	    ClientInterface(){
	        exitCalled=false;
	        //send_action();
	        chooseAction="Escolha uma acao:\n\t-update\n\t-download\n\t-delete\n\t-list_server\n\t-exit";
	        askFilepath="Forneca o caminho do arquivo:";
	        invalidAction="Acao invalida!\n";
	    };

	    int MainLoop(){
	        while(!exitCalled){
	            	std::cout<<chooseAction<<std::endl;
	            	std::cin>>actionChosen;
	            	exitCalled=ClientInterface::execAction();
	            };
	        exit(0); //tenho que matar todas as outras threads antes de sair
	        return 0;
	    };

	private:
	    //pega a funcao escolhida e executa,se necessario pede o caminho do arquivo
	    bool execAction(){
	        if(actionChosen.compare("update")==0){
	            ClientInterface::AskFilepath();
	            try {
	            	char *filepath_str = new char [filepath.length()+1];
					std::strcpy (filepath_str, filepath.c_str());
	                send_action.update(filepath_str,0);
	            }
	            catch(std::exception& e) {
	                std::cout << "exception caught" << std::endl;
	                std::cout << e.what() << std::endl;
	            }
	        }
	        else if(actionChosen.compare("download")==0){
	            ClientInterface::AskFilepath();
	            try {
	            	char *filepath_str = new char [filepath.length()+1];
					std::strcpy (filepath_str, filepath.c_str());
	                send_action.download(filepath_str);
	            }
	            catch(std::exception& e) {
	                std::cout << "exception caught" << std::endl;
	                std::cout << e.what() << std::endl;
	            }
	        }
	        else if(actionChosen.compare("delete")==0){
	            ClientInterface::AskFilepath();
	            try {
	            	char *filepath_str = new char [filepath.length()+1];
					std::strcpy (filepath_str, filepath.c_str());
	                send_action.deleteF(filepath_str,0);
	            }
	            catch(std::exception& e) {
	                std::cout << "exception caught" << std::endl;
	                std::cout << e.what() << std::endl;
	            }
	        }
	        else if(actionChosen.compare("list_server")==0){
	            send_action.list_server();
	        }
	        else if(actionChosen.compare("exit")==0){
	            send_action.exit();
	            return true;
	        }
	        else {
	            std::cout<<invalidAction<<std::endl;
	        }
	        return false;
	    };

	    void AskFilepath(){
	        std::cout<<askFilepath<<std::endl;
	        std::cin >>filepath;
	    };



	protected:


};