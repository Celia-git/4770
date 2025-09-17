# Define the Go programs to build
SERVER_BIN = server
CLIENT_BIN = client

# Define the source files
SERVER_SRC = server.go
CLIENT_SRC = client.go
TEST_SCRIPT = test_cases.sh

# The 'all' target is the default one
all: $(SERVER_BIN) $(CLIENT_BIN)

# Target to build the server executable
$(SERVER_BIN): $(SERVER_SRC)
	go build -o $(SERVER_BIN) $(SERVER_SRC)

# Target to build the client executable
$(CLIENT_BIN): $(CLIENT_SRC)
	go build -o $(CLIENT_BIN) $(CLIENT_SRC)

# Target to clean up compiled executables
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

# This is the important part: a target to run the tests.
# It ensures the binaries are up-to-date before running the test script.
test: all
	./$(TEST_SCRIPT)

.PHONY: all clean test