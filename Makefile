SERVER_SCR=./src/server/
CLIENT_SCR=./src/client/
COMMON_SCR=./src/common/

BIN_DIR=./bin/
BUILD_DIR=./build/

all: common server client

server:
	g++ $(SERVER_SCR)*.cpp $(BUILD_DIR)Message.o -o $(BIN_DIR)server -lpthread

client:
	g++ $(CLIENT_SCR)*.cpp $(BUILD_DIR)Message.o -o $(BIN_DIR)client -lpthread

common:
	g++ -c $(COMMON_SCR)Message.cpp -o $(BUILD_DIR)Message.o -lpthread

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR)*
