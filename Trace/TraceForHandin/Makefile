CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra 
CXXWNO = -Wno-register

all: trace

trace: PacketTracer.cpp checksum.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp PacketTracer.cpp checksum.cpp -lpcap
clean:
	rm -f trace
