CXX = g++
CXXFLAGS = -Wall -Wextra $(shell find lib -type d -exec printf -- '-I%s ' {} \;)
CXXWNO = -Wno-register

SRC = $(wildcard src/*.cpp)
LIBSRC = $(shell find lib -name '*.cpp')
HEADERS = $(shell find lib -name '*.h')
TARGET = bin/trace

all: $(TARGET)

$(TARGET): $(SRC) $(LIBSRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CXXWNO) $(SRC) $(LIBSRC) -o $(TARGET) -lpcap

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)