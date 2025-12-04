CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror

.PHONY: all clean run-server run-client test format

all: server client

server: server.cpp connection.hpp
	$(CXX) $(CXXFLAGS) -o $@ server.cpp

client: client.cpp connection.hpp
	$(CXX) $(CXXFLAGS) -o $@ client.cpp

test: tests
	./tests

tests: tests.cpp connection.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests.cpp -pthread

run-server: server
	./server

run-client: client
	./client

format:
	clang-format -i *.cpp *.hpp

clean:
	rm -f server client tests
