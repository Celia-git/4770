CC = cc
CXX = g++
CFLAGS = -g -I/usr/include/tirpc
LDFLAGS = -ltirpc -lnsl

# Source files
RPC_X = msg.x
CLIENT_SRCS = msg_clnt.c msg_client.cpp
SERVER_SRCS = msg_svc.c msg_proc.cpp

# Object files
CLIENT_OBJS = msg_clnt.o msg_client.o
SERVER_OBJS = msg_svc.o msg_proc.o

# Executables
CLIENT_EXE = msg_client
SERVER_EXE = msg_server

.PHONY: all clean test

all: $(SERVER_EXE) $(CLIENT_EXE)

# Generate rpc code
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

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXE) $(SERVER_EXE) msg.h msg_svc.c msg_clnt.c

test:
	@echo "Test target is currently empty"
