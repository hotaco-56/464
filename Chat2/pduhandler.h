#ifndef PDU_H
#define PDU

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

// Flag definitions
#define FLAG_HANDLE_INITIAL 1
#define FLAG_HANDLE_ACK 2
#define FLAG_HANDLE_ERROR 3
#define FLAG_BROADCAST 4
#define FLAG_MESSAGE 5
#define FLAG_MULTICAST 6
#define FLAG_MESSAGE_ERROR 7
#define FLAG_LIST_REQUEST 10
#define FLAG_LIST_COUNT 11
#define FLAG_LIST_HANDLE 12
#define FLAG_LIST_END 13

#define MAXBUF 1024

int sendPDU(int socketNum, uint8_t *dataBuffer, int lengthOfData, uint8_t flag);
int recvPDU(int socketNum, uint8_t *dataBuffer, int maxLen, uint8_t *flag);

#endif