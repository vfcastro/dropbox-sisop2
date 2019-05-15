SERVER_SCR=./src/server/
CLIENT_SCR=./src/client/
COMMON_SCR=./src/common/

BIN_DIR=./bin/
BUILD_DIR=./build/
TEST_DIR=./test/

all: common server client

server:
	g++ -g $(SERVER_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)server -lpthread

client:
	g++ -g $(CLIENT_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)client -lpthread

common:
	mkdir -p $(BUILD_DIR)
	g++ -g -c $(COMMON_SCR)*.cpp -lpthread
	mv *.o $(BUILD_DIR)

test:
	mkdir -p $(TEST_DIR)/server
	mkdir -p $(TEST_DIR)/client-session1
	mkdir -p $(TEST_DIR)/client-session2
	cp $(BIN_DIR)/server $(TEST_DIR)/server
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session1
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session2

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR) $(TEST_DIR)
