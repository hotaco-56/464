CXX = g++
CXXFLAGS = -g -Wall -std=gnu++11 -Wno-register -Wno-unused-value -D__LIBCPE464_

SRC = networks.cpp gethostbyname.cpp safeUtil.cpp pollLib.cpp udpClient.cpp udpServer.cpp pdu.cpp checksum.cpp buffer.cpp window.cpp
OBJS = networks.o gethostbyname.o safeUtil.o pollLib.o udpClient.o udpServer.o pdu.o checksum.o buffer.o window.o

LIBS = ./libcpe464.2.21.a -ldl

all: rcopy server cleano

rcopy: rcopy.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o rcopy rcopy.cpp $(OBJS) $(LIBS)

server: server.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o server server.cpp $(OBJS) $(LIBS)

%.o: %.cpp *.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

cleano:
	rm -f *.o

clean:
	rm -f rcopy server *.o