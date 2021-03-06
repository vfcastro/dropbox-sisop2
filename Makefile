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
	@echo '#### Usage ####'
	@echo 'Primary:  ./bin/./server <primary_port> 1 <backup1_ip> <backup1_port> <backup2_ip> <backup2_port>'
	@echo 'Backup 1: ./bin/./server <backup1_port> 0 <primary_ip> <primary_port> <backup2_ip> <backup2_port>'
	@echo 'Backup 2: ./bin/./server <backup2_port> 0 <primary_ip> <primary_port> <backup1_ip> <backup1_port>'
	@echo 'Client:   ./bin/./client <username> <primary_ip> <primary_port> <client_reconnection_port>'

common:
	mkdir -p $(BUILD_DIR)
	g++ -g -c $(COMMON_SCR)*.cpp -lpthread -std=c++11
	mv *.o $(BUILD_DIR)

.PHONY: test
test:
	mkdir -p $(TEST_DIR)/server1
	mkdir -p $(TEST_DIR)/server2
	mkdir -p $(TEST_DIR)/server3
	mkdir -p $(TEST_DIR)/client-session1
	mkdir -p $(TEST_DIR)/client-session2
	cp $(BIN_DIR)/server $(TEST_DIR)/server1/server1
	cp $(BIN_DIR)/server $(TEST_DIR)/server2/server2
	cp $(BIN_DIR)/server $(TEST_DIR)/server3/server3
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session1
	cp $(BIN_DIR)/client $(TEST_DIR)/client-session2

test1:
	sh $(TEST_DIR)/test1.sh

test2:
	sh $(TEST_DIR)/test2.sh

test3:
	sh $(TEST_DIR)/test3.sh

test4:
	sh $(TEST_DIR)/test4.sh

clean:
	rm -rf $(BIN_DIR)* $(BUILD_DIR) $(TEST_DIR)/client* $(TEST_DIR)/server*
	clear