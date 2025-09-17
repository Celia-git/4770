Assignment 1: Distributed Tuple Space 

Overview

This project, "Listen Linda!", is an implementation of a distributed tuple space system inspired by the Linda coordination language. The system allows remote clients to coordinate by performing operations on a shared tuple space.

Features

    Distributed Tuple Space: A centralized server manages a shared tuple space, allowing multiple clients to interact with it remotely.

    Client-Server Architecture: A TCP server hosts the tuple space and handles requests from remote clients. Clients connect to the server to perform operations.

    Linda Operations:

        out(tuple): Adds a new tuple to the tuple space.

        rd(pattern): Reads a matching tuple from the space without removing it. If multiple tuples match, a random one is returned.

        in(pattern): Reads and removes a matching tuple from the space. If multiple tuples match, a random one is chosen, removed, and returned.

    Wildcard Matching: Tuples are matched against patterns, where a wildcard (? on the command line) can match any value at a given position.

    Blocking Behavior: rd and in operations are blocking. If no matching tuple is found, the client will wait until a matching tuple is added by another client.

    Data Types: Tuples can contain int64, float64, and string data types.

    Concurrency: The server is designed to handle multiple concurrent clients using appropriate concurrency mechanisms.

Implementation Details

The project is implemented in Go and consists of three main components:

    TupleSpace: The core data structure that holds the tuples and provides the out, rd, and in methods.

    server.go: The TCP server that listens on port 8080 (by default) and handles incoming client connections and requests. It deserializes client messages, processes the requests, and sends back responses.

    client.go: A command-line client that connects to the server, formats user requests into a message, and sends them to the server for processing.

Communication Protocol

Client-server communication uses a simple JSON-over-TCP protocol. Messages contain the operation to be performed (out, rd, or in) and the corresponding tuple or pattern.

Concurrency

The Go implementation uses goroutines to handle each client connection concurrently. A mutex is used to ensure safe access to the shared tuple space data structure. A condition variable is used to handle blocking rd and in operations, allowing waiting clients to be notified when a new tuple is added that might satisfy their request.

Prerequisites

    Go 1.18+

Building and Running

    Clone the repository:

    Build the project:

    Start the server:
    Open a terminal and run: ```go run server.go &```

    The server will start and listen on port 8080.

    Run a client:
    Open another terminal and use the client.go executable to interact with the server.

        Add a tuple: ```go run client.go -out '("foo", "bar")' ```

        Read a tuple: ```go run client.go -rd '("foo", ?)```

        Read and remove a tuple: ```go run client.go -in '("foo", ?)'```

Running Test Cases

The test_cases.sh script demonstrates a series of interactions between multiple clients and the server to verify the correct functionality, including blocking behavior and wildcard matching.

To run the tests:

    compile the client and server scripts using ```make all```

    Kill the server if it is running.

    In a separate terminal, run the test script: ```./test_cases.sh```

    This will execute a sequence of client commands and print the output, which can be compared against expected behavior.
    
To run the Makefile:
    
    Kill the server if it is running
    
    ```make clean``` removes any executables.
    
    ```make all``` compiles executables.
    
    ```make test``` runs the test file.

Deliverables

    tuplespace.go: The core TupleSpace implementation.

    server.go: The TCP server.

    client.go: The sample command-line client.

    Makefile: For building and running the project.

    README.md: This file.

    test_cases.sh: A shell script for demonstrating functionality.
