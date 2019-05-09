#include <thread>
#include "libs/ClientInterface.cpp"
#include "libs/Receiver.cpp"
#include "libs/Notifier.cpp"

int main (int argc, char **argv)
{
  	ClientInterface* interfacePtr = new ClientInterface();
  	Receiver* receiverPtr = new Receiver();
  	Notifier* notifierPtr = new Notifier();

  	std::thread clientInterfaceT(&ClientInterface::MainLoop,interfacePtr);
  	std::thread synchDirCheckT(&Notifier::notify,notifierPtr);
  	//essa thread abaixo seria a thread que checa se o cliente recebeu algum comando do server
  	//No momento ela so eh um loop que printa "estou aqui" a cada 10 secs entao deixei comentada
  	//std::thread receiverT(&Receiver::MainLoop,receiverPtr);

  	clientInterfaceT.join();
  	synchDirCheckT.join();
  	//receiverT.join();
  	return 0;
}
