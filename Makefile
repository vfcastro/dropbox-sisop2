SERVER_SCR=./src/server/
CLIENT_SCR=./src/client/
COMMON_SCR=./src/common/

BIN_DIR=./bin/
BUILD_DIR=./build/

all: common server client

server:
	g++ -g $(SERVER_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)server -lpthread -std=c++11

client:
	g++ -g $(CLIENT_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)client -lpthread -std=c++11

common:
	g++ -g -c $(COMMON_SCR)*.cpp -lpthread -std=c++11
	mv *.o $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR)*
