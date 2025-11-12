#include <zmq.h>
#include <iostream>

int main() {
    // Initialize ZeroMQ context
    void* context = zmq_ctx_new();
    if (!context) {
        std::cerr << "Failed to create ZeroMQ context" << std::endl;
        return 1;
    }

    // Create XSUB and XPUB sockets (frontend and backend)
    void* frontend = zmq_socket(context, ZMQ_XSUB);
    void* backend  = zmq_socket(context, ZMQ_XPUB);
    if (!frontend || !backend) {
        std::cerr << "Error creating sockets" << std::endl;
        return 1;
    }

    // Bind sockets to ports
    zmq_bind(frontend, "tcp://*:5555"); // For publishers (Statistics Connector)
    zmq_bind(backend,  "tcp://*:5556"); // For subscribers (Monitor Connector)

    std::cout << "Message Queue started on ports 5555 (frontend) and 5556 (backend)" << std::endl;

    // Acts as a simple forwarder between publisher(s) and subscriber(s)
    zmq_proxy(frontend, backend, nullptr);

    // Close sockets and clean up
    zmq_close(frontend);
    zmq_close(backend);
    zmq_ctx_destroy(context);
    return 0;
}
