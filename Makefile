SERVER_SCR=./src/server/
CLIENT_SCR=./src/client/
COMMON_SCR=./src/common/

BIN_DIR=./bin/
BUILD_DIR=./build/

all: common server client

server:
	g++ -g $(SERVER_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)server -lpthread

client:
	g++ -g $(CLIENT_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)client -lpthread

common:
	g++ -g -c $(COMMON_SCR)*.cpp -lpthread
	mv *.o $(BUILD_DIR)

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR)*
