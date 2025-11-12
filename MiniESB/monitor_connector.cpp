#include <zmq.h>
#include <boost/process.hpp>
#include <iostream>
#include <deque>
#include <sstream>
#include <ctime>
#include <thread>

namespace bp = boost::process;

// Converts incoming CSV string into a Gnuplot-ready data point
std::string adapter_csv_to_plot(const std::string& msg) {
    std::istringstream iss(msg);
    std::string prefix, epoch_str, free_mem_str;
    getline(iss, prefix, ',');
    getline(iss, epoch_str, ',');
    getline(iss, free_mem_str, ',');

    if (prefix != "MSG_MEMSTAT") return "";

    time_t epoch = std::stol(epoch_str);
    tm* t = gmtime(&epoch);
    int seconds_of_day = t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec;

    std::ostringstream oss;
    oss << seconds_of_day << " " << free_mem_str << "\n";
    return oss.str();
}

int main() {
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_SUB);
    if (zmq_connect(subscriber, "tcp://localhost:5556") != 0) {
        std::cerr << "Failed to connect to message queue" << std::endl;
        return 1;
    }

    // Subscribe to all messages
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    std::deque<std::string> history;
    bp::opstream gp_in;
    bp::child gnuplot("gnuplot", bp::std_in < gp_in);

    std::cout << "Monitor Connector started. Launching Gnuplot..." << std::endl;
    gp_in << "set term x11\n" << std::flush;

    while (true) {
        char buffer[256];
        int bytes = zmq_recv(subscriber, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
            continue;
        buffer[bytes] = '\0';
        std::string msg(buffer);

        std::string point = adapter_csv_to_plot(msg);
        if (point.empty()) continue;

        history.push_back(point);
        if (history.size() > 20)
            history.pop_front();

        gp_in << "plot '-' with linespoints title 'Free Memory'\n";
        for (const auto& p : history)
            gp_in << p;
        gp_in << "e\n" << std::flush;

        std::cout << "Plotted new data point" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    gnuplot.wait();
    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
