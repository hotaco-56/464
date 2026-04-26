#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "messageHandler.h"
#include "clientInfo.h"
#include "pduhandler.h"
#include "pollLib.h"
#include "ctype.h"
#include "messageFlags.h"

#define MAXBUF 1024
#define PDU_MESSAGE 1

void termClient(int clientSocket) 
{
    removeFromPollSet(clientSocket);
    removeClientFromTable(clientSocket);
    close(clientSocket);
}

void recvFromClient(int clientSocket)
{
    uint8_t dataBuffer[MAXBUF];
    int messageLen = 0;
    uint8_t handleLen = 0;

    if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0) {
        perror("recv call");
        exit(-1);
    }
    if (messageLen > 0) {
        switch (dataBuffer[0])
        {
        case PDU_MESSAGE:
        {
            handleLen = dataBuffer[1];
            char handle[MAX_HANDLE_LEN];
            memcpy(handle, &dataBuffer[2], handleLen);
            handle[handleLen] = '\0';
            printf("Socket %d: Received message from %s: %s\n", clientSocket, handle, &dataBuffer[2 + handleLen]);
            int recipientSocket;
            getClientSocketFromHandle(handle, &recipientSocket);
            if (recipientSocket < 0) {
                printf("Handle %s not found, message not sent\n", handle);
            } else {
                sendToClient(recipientSocket, &dataBuffer[handleLen + 2], messageLen-handleLen-2); //only send message, not flag or handle
            }
            break;
        }
        default:
            break;
        }
    } else {
        printf("Socket %d: Connection closed by other side\n", clientSocket);
        termClient(clientSocket);
    }
}

void getClientSocketFromHandle(char* handle, int* socketNum) 
{
    for (uint16_t i = 0; i < num_clients; i++) {
        if (strcmp(clientsTable[i].handle, handle) == 0) {
            *socketNum = clientsTable[i].socketNum;
            return;
        }
    }
    *socketNum = -1;
}

void sendToClient(int clientSocket, uint8_t * buffer, int lengthOfData)
{
    int sent = 0;
    sent = sendPDU(clientSocket, buffer, lengthOfData);
    if (sent < 0) {
        perror("send call");
        exit(-1);
    }
    printf("Socket %d: Sent, Length: %d msg: %s\n", clientSocket, sent, buffer);
}