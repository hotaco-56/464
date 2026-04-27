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
#include "clientInfo.h"
#include "messageFlags.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_HANDLE_LEN 100

typedef struct {
    char *handle;
    char *serverName;
    char *serverPort;
} ClientArgs;

void sendToServer(int socketNum);
void sendHandleInitial(int socketNum, char *handle);
void recvFromServer(int socketNum);
int readFromStdin(uint8_t *buffer);
void checkClientArgs(int argc, char *argv[], ClientArgs *args);
void processMessage(int socketNum, int messageLen, uint8_t *dataBuffer);
void sendMessagePDU(int socketNum, int messageLen, uint8_t *dataBuffer);
void sendListPDU(int socketNum);
void findHandle(char* handle, uint8_t* dataBuffer, int messageLen);

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
            sendToServer(socketNum);
        }
        else {
            recvFromServer(socketNum);
        }
    }
    
    close(socketNum);
    
    return 0;
}


void recvFromServer(int socketNum)
{
    uint8_t dataBuffer[MAXBUF];
    int messageLen = 0;
    uint8_t flag = 0;
	uint32_t totalClients = 0;

    if ((messageLen = recvPDU(socketNum, dataBuffer, MAXBUF, &flag)) < 0) {
        perror("recv call");
        exit(EXIT_FAILURE);
    }


    if (messageLen > 0) {
		switch (flag)
		{
		case FLAG_MESSAGE:
		{
			printf("Message Good From Socket %d, length: %d Data: %s\n", socketNum, messageLen, dataBuffer);
			break;
		}
		case FLAG_LIST_COUNT:
		{
			memcpy(&totalClients, dataBuffer, messageLen);
			printf("Total clients: %d\n", totalClients);
			break;
		}
		case FLAG_LIST_HANDLE:
		{
			uint8_t handleLen = dataBuffer[0];
			printf("handle len: %d", handleLen);
			char handle[MAX_HANDLE_LEN];
			memcpy(handle, dataBuffer + 1, handleLen);
			handle[handleLen] = '\0';  // NULL terminate
			printf("Handle: %s\n", handle);
			break;
		}
		default:
			break;
		}
    } else {
        printf("Server Terminated\n");
        close(socketNum);
        exit(EXIT_FAILURE);
    }
}

void sendToServer(int socketNum)
{
    uint8_t buffer[MAXBUF];
    int sendLen = 0;
    
    sendLen = readFromStdin(buffer);
    processMessage(socketNum, sendLen, buffer);
	memset(buffer, 0, MAXBUF);
}

void processMessage(int socketNum, int messageLen, uint8_t *dataBuffer) 
{
    if (dataBuffer[0] != '%') {
        printf("usage: %%msgType message\n");
        return;
    }
        
    int cmd = toupper((int)dataBuffer[1]);
    
    switch(cmd) {
        case 'M':
            sendMessagePDU(socketNum, messageLen, dataBuffer);
            break;
        case 'L':
			sendListPDU(socketNum);
            break;
        default:
            break;
    }
}

void sendHandleInitial(int socketNum, char *handle)
{
    int sent = 0;
    uint8_t pdu[MAX_HANDLE_LEN];
    memcpy(pdu, handle, strlen(handle));
    pdu[strlen(handle)] = '\0';
    
    sent = sendPDU(socketNum, pdu, strlen(handle) + 1, FLAG_HANDLE_INITIAL);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }
}

void sendMessagePDU(int socketNum, int dataBufferLen, uint8_t *dataBuffer)
{
    int sent = 0;
    char handle[MAX_HANDLE_LEN];
    findHandle(handle, dataBuffer, dataBufferLen);
    
    size_t handleLen = strlen(handle);
	uint8_t pduMessageLen = 1 + handleLen + dataBufferLen;
    uint8_t *msgPDU = malloc(pduMessageLen);
    
	//send flag,handleLen, handle, and message in pdu
    msgPDU[0] = handleLen;
    memcpy(msgPDU + 1, handle, handleLen);
    memcpy(msgPDU + 1 + handleLen, dataBuffer + 2 + handleLen, dataBufferLen - 2 - handleLen);
    
    sent = sendPDU(socketNum, msgPDU, pduMessageLen, FLAG_MESSAGE);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }

    free(msgPDU);
}

void sendListPDU(int socketNum)
{
	uint8_t sent = 0;
    
    sent = sendPDU(socketNum, (uint8_t*)&sent, 1, FLAG_LIST_REQUEST);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }
}

void findHandle(char* handle, uint8_t* dataBuffer, int messageLen)
{
    int i = 0;
    int handleIndex = 0;
    
    while (i < messageLen && dataBuffer[i] != ' ') {
        i++;
    }
    i++;
    
    while (i < messageLen && dataBuffer[i] != ' ') {
        handle[handleIndex++] = dataBuffer[i++];
    }
    handle[handleIndex] = '\0';
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