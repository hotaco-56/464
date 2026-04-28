#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "clientMessageHandler.h"
#include "clientInfo.h"
#include "pduhandler.h"
#include "pollLib.h"
#include "ctype.h"
#include "messageFlags.h"

void sendMessagePDU(int socketNum, int dataBufferLen, uint8_t *dataBuffer)
{
    int sent = 0;
    char handle[MAX_HANDLE_LEN];
    findHandle(handle, dataBuffer, dataBufferLen);
    
    size_t handleLen = strlen(handle);
    int messageLen = dataBufferLen - 3 - handleLen;
    uint8_t pduMessageLen = messageLen + handleLen + 1;
    uint8_t *msgPDU = malloc(pduMessageLen);
    
    //send handleLen, handle, and message in pdu
    msgPDU[0] = handleLen;
    memcpy(msgPDU + 1, handle, handleLen);
    memcpy(msgPDU + 1 + handleLen, dataBuffer + 3 + handleLen, messageLen);
    
    sent = sendPDU(socketNum, msgPDU, pduMessageLen, FLAG_MESSAGE);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }

    free(msgPDU);
}

void sendListPDU(int socketNum)
{
    int sent = 0;
    
    sent = sendPDU(socketNum, NULL, 0, FLAG_LIST_REQUEST);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }
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