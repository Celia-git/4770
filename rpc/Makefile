CC = cc
CXX = g++
CFLAGS = -g -I/usr/include/tirpc
LDFLAGS = -ltirpc -lnsl
SQLITE_FLAGS = -lsqlite3

# Source files
RPC_X = msg.x
CLIENT_SRCS = msg_clnt.c msg_client.cpp
SERVER_SRCS = msg_svc.c msg_proc.cpp
SQLITE_SRC = main.cpp

# Object files
CLIENT_OBJS = msg_clnt.o msg_client.o
SERVER_OBJS = msg_svc.o msg_proc.o
SQLITE_OBJ = main.o

# Executables
CLIENT_EXE = msg_client
SERVER_EXE = msg_server
SQLITE_EXE = main

.PHONY: all clean test

all: $(SERVER_EXE) $(CLIENT_EXE) $(SQLITE_EXE)

# Generate RPC code
%.h %.c: $(RPC_X)
	rpcgen -N -C $<

# Server build
msg_svc.o: msg_svc.c msg.h
	$(CC) $(CFLAGS) -c -o $@ msg_svc.c

msg_proc.o: msg_proc.cpp msg.h
	$(CXX) $(CFLAGS) -c -o $@ msg_proc.cpp

$(SERVER_EXE): $(SERVER_OBJS)
	$(CXX) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)

# Client build
msg_clnt.o: msg_clnt.c msg.h
	$(CC) $(CFLAGS) -c -o $@ msg_clnt.c

msg_client.o: msg_client.cpp msg.h
	$(CXX) $(CFLAGS) -c -o $@ msg_client.cpp

$(CLIENT_EXE): $(CLIENT_OBJS)
	$(CXX) $(CFLAGS) -o $@ $(CLIENT_OBJS) $(LDFLAGS)

# SQLite (middleware) build
main.o: $(SQLITE_SRC)
	$(CXX) $(CFLAGS) -c -o $@ $(SQLITE_SRC)

$(SQLITE_EXE): $(SQLITE_OBJ)
	$(CXX) $(CFLAGS) -o $@ $(SQLITE_OBJ) $(SQLITE_FLAGS)

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(SQLITE_OBJ) $(CLIENT_EXE) $(SERVER_EXE) $(SQLITE_EXE) msg.h msg_svc.c msg_clnt.c

test: $(SQLITE_EXE)
	@echo "Running middleware test..."
	@output=$$(./$(SQLITE_EXE) credit A12345 100); \
	echo "$$output"; \
	if echo "$$output" | grep -q "success\|credited\|updated"; then \
		echo "Test Passed: credit command executed successfully."; \
	else \
		echo "Test Failed: expected success message not found."; \
		exit 1; \
	fi
