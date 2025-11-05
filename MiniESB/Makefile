CXX = g++
CXXFLAGS = -Wall -O2 -g -std=c++11
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
