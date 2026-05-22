# udpCode makefile
# written by Hugh Smith - Feb 2021

CC = gcc
CFLAGS = -g -Wall -std=gnu99


SRC = networks.c  gethostbyname.c safeUtil.c pollLib.c
OBJS = networks.o gethostbyname.o safeUtil.o pollLib.o

#uncomment next two lines if your using sendtoErr() library
LIBS += libcpe464.2.21.a -lstdc++ -ldl
CFLAGS += -D__LIBCPE464_

all:  udpClient udpServer cleano

udpClient: udpClient.c $(OBJS) 
	$(CC) $(CFLAGS) -o rcopy udpClient.c $(OBJS) $(LIBS)

udpServer: udpServer.c $(OBJS) 
	$(CC) $(CFLAGS) -o server udpServer.c  $(OBJS) $(LIBS)

%.o: %.c *.h 
	gcc -c $(CFLAGS) $< -o $@ 

cleano:
	rm -f *.o

clean:
	rm -f udpServer udpClient *.o

