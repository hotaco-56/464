/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

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
#include <ctype.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pduhandler.h"
#include "clientMessageHandler.h"
#include "clientInfo.h"

#define DEBUG_FLAG 1

typedef struct {
    char *handle;
    char *serverName;
    char *serverPort;
} ClientArgs;

int readFromStdin(uint8_t *buffer);
void checkClientArgs(int argc, char *argv[], ClientArgs *args);

ClientArgs args;

int main(int argc, char *argv[])
{
    int socketNum = 0;
    checkClientArgs(argc, argv, &args);
    socketNum = tcpClientSetup(args.serverName, args.serverPort, DEBUG_FLAG);
    addToPollSet(socketNum);
    addToPollSet(STDIN_FILENO);

    sendHandleInitial(socketNum, args.handle);
    
    while(1) {
        int socket = pollCall(-1);
        if (socket == STDIN_FILENO) {
            uint8_t buffer[MAXBUF];
            int sendLen = 0;
            sendLen = readFromStdin(buffer);
            sendToServer(socketNum, buffer, sendLen);
            memset(buffer, 0, MAXBUF);
        }
        else {
            recvFromServer(socketNum);
        }
    }
    close(socketNum);
    
    return 0;
}

int readFromStdin(uint8_t * buffer)
{
    char aChar = 0;
    int inputLen = 0;        
    
    buffer[0] = '\0';
    while (inputLen < (MAXBUF - 1) && aChar != '\n')
    {
        aChar = getchar();
        if (aChar != '\n')
        {
            buffer[inputLen] = aChar;
            inputLen++;
            if (inputLen >= (MAXBUF - 1)) {
                printf("Too much input! Max is %d characters\n", MAXBUF - 1);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    buffer[inputLen] = '\0';
    inputLen++;
    
    return inputLen;
}

void checkClientArgs(int argc, char *argv[], ClientArgs *args) 
{
    size_t handleLen;

    if (argc != 4) {
        printf("Usage: %s handle server-name server-port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    handleLen = strlen(argv[1]);
    if (handleLen > MAX_HANDLE_LEN) {
        printf("Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    args->handle = argv[1];
    args->serverName = argv[2];
    args->serverPort = argv[3];
}