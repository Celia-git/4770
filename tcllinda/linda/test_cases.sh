#!/bin/bash

# --- Test Automation Script for Distributed Tuple Space ---
# This script assumes the 'server' and 'client' executables
# are present in the same directory.

# Define the server and client executables
SERVER="./server"
CLIENT="./client"

# A helper function to print colored output
echo_info() {
    echo -e "\033[1;34m$1\033[0m"
}

echo_success() {
    echo -e "\033[1;32m$1\033[0m"
}

echo_error() {
    echo -e "\033[1;31m$1\033[0m"
}

# --- 1. Start the server in the background ---
echo_info "Starting the tuple space server..."
$SERVER > server_log.txt 2>&1 &
SERVER_PID=$!
sleep 1 # Give the server a moment to start up

if kill -0 $SERVER_PID > /dev/null 2>&1; then
    echo_success "Server started with PID: $SERVER_PID"
else
    echo_error "Failed to start the server. Exiting."
    exit 1
fi

# --- 2. Test Basic OUT and RD Operations ---
echo_info "\n--- Test Case 1: OUT followed by RD ---"
echo "Client 1: Writing tuple {STRING {Hello}} {INT64 123}..."
$CLIENT -OUT "{{STRING {Hello}} {INT64 123}}"
sleep 0.5

echo "Client 2: Reading tuple with exact match..."
$CLIENT -RD "{{STRING {Hello}} {INT64 123}}"

# --- 3. Test RD with a Wildcard ---
echo_info "\n--- Test Case 2: RD with a Wildcard ---"
echo "Client 3: Reading tuple using wildcard {STRING {Hello}} {INT64 ?}..."
$CLIENT -RD "{{STRING {Hello}} {INT64 ?}}"

# --- 4. Test IN (Read and Remove) ---
echo_info "\n--- Test Case 3: IN (Read and Remove) ---"
echo "Client 4: Writing a new tuple {STRING {Delete Me}} {INT64 999}..."
$CLIENT -OUT "{{STRING {Delete Me}} {INT64 999}}"
sleep 0.5

echo "Client 5: Performing IN to delete the tuple..."
$CLIENT -IN "{{STRING {Delete Me}} {INT64 999}}"

echo "Client 6: Trying to read the deleted tuple (should fail or block)..."
# This RD command should hang if the IN was successful, or return an empty tuple.
# A timeout is useful here to prevent the script from hanging indefinitely.
# In a real test, you might use a tool like 'timeout' if available.
# For this example, we'll just run it and observe.
$CLIENT -RD "{{STRING {Delete Me}} {INT64 999}}" &
RD_PID=$!
sleep 2
if kill -0 $RD_PID > /dev/null 2>&1; then
    echo_success "The RD command is correctly blocking. Killing it now."
    kill $RD_PID
else
    echo_error "The RD command did not block. The tuple may not have been removed."
fi

# Wait for the background process to finish before proceeding
wait $RD_PID 2>/dev/null

# --- 5. Test Blocking IN with a Concurrent OUT ---
echo_info "\n--- Test Case 4: Blocking IN and Concurrent OUT ---"
echo "Client 7: Performing a blocking IN on {STRING {Blocker}} {INT64 ?}..."
$CLIENT -IN "{{STRING {Blocker}} {INT64 ?}}" &
BLOCKING_PID=$!
sleep 1 # Give the blocking client a moment to connect

echo "Client 8: Writing a tuple that satisfies the blocking IN request..."
$CLIENT -OUT "{{STRING {Blocker}} {INT64 456}}"

sleep 1 # Wait for the blocking client to unblock

if ! kill -0 $BLOCKING_PID > /dev/null 2>&1; then
    echo_success "The blocking IN command successfully unblocked and exited."
else
    echo_error "The blocking IN command did not unblock. Something went wrong."
    kill $BLOCKING_PID
fi

# --- 6. Cleanup ---
echo_info "\n--- Cleaning up ---"
echo "Killing the server with PID: $SERVER_PID"
kill $SERVER_PID
rm -f server_log.txt

echo_success "Test script finished."