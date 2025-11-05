### Project Directory Structure

```
mini-esb/
├── Makefile
├── README.md
├── message_queue.cpp
├── statistics_connector.cpp
├── monitor_connector.cpp
├── adapters.hpp
└── utils.hpp
```

***

### File Responsibilities

#### message_queue.cpp
Implements the ZeroMQ proxy that forwards messages between the publisher and subscriber.  
Key steps:
- Create `ZMQ_XSUB` (frontend) and `ZMQ_XPUB` (backend) sockets.
- Bind to known TCP ports, for example:
  ```
  frontend -> tcp://*:5555
  backend  -> tcp://*:5556
  ```
- Run `zmq_proxy(frontend, backend, NULL)`.

#### statistics_connector.cpp
Implements the system data producer.  
Key steps:
- Creates a `ZMQ_PUB` socket that connects to the message queue’s frontend.
- Uses Boost.Process to run the `/proc/meminfo` monitoring script.
- Reads lines, converts them with `adapter_command_to_csv()`, and publishes them.

#### monitor_connector.cpp
Implements the data consumer and visualizer.  
Key steps:
- Creates a `ZMQ_SUB` socket that connects to the queue’s backend.
- Uses Boost.Process to start Gnuplot and manage input/output streams.
- Maintains an in-memory queue (e.g., `std::deque`) of the last 20 points.
- Invokes `adapter_csv_to_plot()` before sending data to Gnuplot for plotting.

#### adapters.hpp
Contains helper functions:
- `adapter_command_to_csv(line, converted)`
- `adapter_csv_to_plot(line, converted)`

Include necessary headers:
```
#include <string>
#include <ctime>
#include <sstream>
```

#### utils.hpp
Optionally hold reusable setup functions for ZeroMQ context creation or date/time conversion.

***

### Example Makefile

```
CXX = g++
CXXFLAGS = -Wall -O2 -g
LIBS = -lzmq -lboost_system -lboost_filesystem -lboost_program_options -lboost_process

TARGETS = message_queue statistics_connector monitor_connector

all: $(TARGETS)

message_queue: message_queue.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

statistics_connector: statistics_connector.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

monitor_connector: monitor_connector.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

run: all
	@echo "Starting message queue..."
	@./message_queue & \
	 echo "Starting statistics connector..." & \
	 ./statistics_connector & \
	 echo "Starting monitor connector..." & \
	 ./monitor_connector &

killall:
	@pkill -f message_queue || true
	@pkill -f statistics_connector || true
	@pkill -f monitor_connector || true

clean:
	rm -f $(TARGETS) *.o
```

