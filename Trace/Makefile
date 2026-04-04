CC = gcc
CFLAGS = -Wall -Wextra -Wno-register -Wno-unused-parameter -Ilib -v

SRC = src/*.cpp
LIBSRC = lib/*.cpp
TARGET = bin/trace

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) $(LIBSRC) -o $(TARGET)

clean:
	rm -f $(TARGET)