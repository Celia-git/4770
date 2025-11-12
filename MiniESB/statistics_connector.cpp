#include <zmq.h>
#include <boost/process.hpp>
#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>

namespace bp = boost::process;

// Converts /proc/meminfo line into a CSV-formatted message
int adapter_command_to_csv(const std::string& line, std::string& converted) {
    std::istringstream iss(line);
    std::string key, value, unit;
    iss >> key >> value >> unit;

    if (key != "MemAvailable:") {
        return 0; // Skip irrelevant lines
    }

    time_t epoch = time(nullptr);
    std::ostringstream oss;
    oss << "MSG_MEMSTAT," << epoch << "," << value;
    converted = oss.str();
    return converted.size();
}

int main() {
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);
    if (zmq_connect(publisher, "tcp://localhost:5555") != 0) {
        std::cerr << "Failed to connect to message queue" << std::endl;
        return 1;
    }

    // Create a subprocess to continuously read memory info
    bp::ipstream child_stdout;
    bp::child child("/bin/bash", 
        bp::args={"-c", "while [ 1 ]; do grep Available /proc/meminfo; sleep 1; done"}, 
        bp::std_out > child_stdout);

    std::cout << "Statistics Connector started" << std::endl;

    std::string line;
    while (std::getline(child_stdout, line)) {
        std::string msg;
        if (adapter_command_to_csv(line, msg)) {
            zmq_send(publisher, msg.c_str(), msg.size(), 0);
            std::cout << "Sent: " << msg << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    child.wait();
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}
