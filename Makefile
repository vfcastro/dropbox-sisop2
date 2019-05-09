SERVER_SCR=./src/server/
SERVER_INCLUDE=./include/server/

CLIENT_SCR=./src/client/
CLIENT_INCLUDE=./include/client/

COMMOM_SCR=./src/common/
COMMOM_INCLUDE=./include/common/

BIN_DIR=./bin/

all: server client

 server:
	g++ $(SERVER_SCR)*.cpp -o $(BIN_DIR)server -lpthread

client:
	g++ $(CLIENT_SCR)*.cpp -o $(BIN_DIR)client -lpthread

clean:
	rm -rf $(BIN_DIR)*
