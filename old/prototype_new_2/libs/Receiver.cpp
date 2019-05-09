#include <iostream>
#include <chrono>
#include <thread>

class Receiver{
public:
    Receiver(){};

    int MainLoop(){
        int timeSleeping=10000;
        while(1){
            std::this_thread::sleep_for(std::chrono::milliseconds(timeSleeping));
            std::cout<<"Receiver thread checking if something was received from the server..."<<std::endl;
        }
    }


private:


protected:


};
