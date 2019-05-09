#include <pthread.h>

class ServerCommunicator {
private:
  unsigned int port;
  unsigned int backlog;
  int sockfd;
  std::thread th;

public:
  ServerCommunicator(unsigned int port, unsigned int backlog);
  ~ServerCommunicator();
  void start();
};
