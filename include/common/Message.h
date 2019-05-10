#define MAX_PAYLOAD_SIZE

// Message types:
#define SESSION_OPEN 0

struct Message {
	unsigned int type;
	std::string username;
	unsigned int seqn; // 0 if the last, > 0 otherwise
	unsigned int lenght;
	std::string payload;
};
