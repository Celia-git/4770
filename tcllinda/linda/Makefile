# Makefile for Linda Tuple Space in Go

.PHONY: all build_server build_client run_server run_client clean test

BINDIR := bin
SERVER := $(BINDIR)/server
CLIENT := $(BINDIR)/client

all: build_server build_client

build_server:
	@mkdir -p $(BINDIR)
	go build -o $(SERVER) server.go

build_client:
	@mkdir -p $(BINDIR)
	go build -o $(CLIENT) client.go

run_server: build_server
	@echo "Starting server:"
	$(SERVER)

run_client: build_client
	@echo "Starting client:"
	$(CLIENT)

test: all
	./test_cases.sh

clean:
	rm -rf $(BINDIR)

