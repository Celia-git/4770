#!/bin/bash

# This script demonstrates usage of the Linda tuple space system.

set -e

SERVER=bin/server
CLIENT=bin/client

# Start the server in the background
$SERVER &
SERVER_PID=$!
sleep 1

echo "Starting test run..."

function client_cmd {
    echo "> $*"
    # echo command, send to client, print result
    (echo "$*"; sleep 1) | $CLIENT
}

# Client 1: write a tuple
client_cmd "out STRING:Hello INT64:35"

# Client 2: read (rd) with wildcard
client_cmd "rd STRING:? INT64:35"

# Client 3: remove (in) tuple with specific match
client_cmd "in STRING:Hello INT64:35"

# Client 4: try remove (blocks), then satisfy with out
(
    (echo "in STRING:? INT64:99"; sleep 2; echo "out STRING:Howdy INT64:99") | $CLIENT
) &

# Wait for background test
sleep 4

echo "Test complete. Stopping server."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null || true

