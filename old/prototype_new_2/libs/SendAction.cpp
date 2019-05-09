#include <iostream>
#include <string>

class SendAction{

public:
    SendAction(){};
    //int updateF(char* filename);
    //int downloadF(char* filename);
    //int deleteF(char* filename);
    //int list_server();
    //int get_sync_dir();
    //int exit();
    int update(char* filepath,bool insideSynchDir){
        //preciso do bool insideSynchDir porque se update for chamado pelo inotify eu so tenho acesso ao nome do arquivo
        //teoricamente o ideal seria adicionar o resto do filepath dentro da funcao do inotify antes de chamar update
        std::string temp_str(filepath);
        std::cout<<"update(" << temp_str << ") called" << std::endl;
    }

    int download(char* filepath){
        std::string temp_str(filepath);
        std::cout<<"download("<<temp_str<<") called"<<std::endl;
    }

    int deleteF(char* filepath,bool insideSynchDir){
        std::string temp_str(filepath);
        std::cout<<"delete("<<temp_str<<") called"<<std::endl;
    }

    int list_server(){
        std::cout<<"list_server() called"<<std::endl;
    }

    int get_sync_dir(){
        std::cout<<"get_sync_dir() called"<<std::endl;
    }

    int exit(){
        std::cout<<"exit() called"<<std::endl;
    }
private:


protected:


};
