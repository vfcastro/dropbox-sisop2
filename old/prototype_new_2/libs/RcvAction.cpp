#include <iostream>
#include <string>

class RcvAction{
public:
    RcvAction(){};

    int update(char* filename,void* file){
        std::string temp_str(filename);
        std::cout<<"update("<<temp_str<<") received"<<std::endl;
    }

    int deleteF(char* filename){
        std::string temp_str(filename);
        std::cout<<"delete("<<temp_str<<") received"<<std::endl;
    }

    int list_server(void* result){
        std::cout<<"list_server() received"<<std::endl;
    }


private:


protected:


};
