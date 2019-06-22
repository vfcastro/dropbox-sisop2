SERVER_SCR=./src/server/
CLIENT_SCR=./src/client/
COMMON_SCR=./src/common/

BIN_DIR=./bin/
BUILD_DIR=./build/
TEST_DIR=./test/

all: common server client

server:
	g++ -g $(SERVER_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)server -lpthread -std=c++11

client:
	g++ -g $(CLIENT_SCR)*.cpp $(BUILD_DIR)*.o -o $(BIN_DIR)client -lpthread -std=c++11
	@echo ''
	@echo ''
	@echo 'Usage:'
	@echo './bin/./server <port>'
	@echo './bin/./client <username> <ip_address> <port>'

common:
	mkdir -p $(BUILD_DIR)
	g++ -g -c $(COMMON_SCR)*.cpp -lpthread -std=c++11
	mv *.o $(BUILD_DIR)

.PHONY: test
test:
	mkdir -p $(TEST_DIR)/server1
	mkdir -p $(TEST_DIR)/server2
	mkdir -p $(TEST_DIR)/client-session1
	mkdir -p $(TEST_DIR)/client-session2
	cp $(BIN_DIR)/server $(TEST_DIR)/server1
	cp $(BIN_DIR)/server $(TEST_DIR)/server2
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session1
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session2

	sh $(TEST_DIR)/test1.sh

test2:
	sh $(TEST_DIR)/test2.sh

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR) $(TEST_DIR)/client* $(TEST_DIR)/server*
	clear