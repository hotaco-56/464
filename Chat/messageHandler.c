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
    uint8_t flag = 0;

    if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF, &flag)) < 0) {
        perror("recv call");
        exit(-1);
    }
    if (messageLen > 0) {
        switch (flag)
        {
        case FLAG_HANDLE_INITIAL:
        {
            uint8_t handleLen = dataBuffer[0];  
            char handle[MAX_HANDLE_LEN];
            memcpy(handle, &dataBuffer[1], handleLen);  
            handle[handleLen] = '\0';
            printf("Socket %d: Received handle: %s\n", clientSocket, handle);
            
            // Check if handle already exists
            int existingSocket = -1;
            getClientSocketFromHandle(handle, &existingSocket);
            
            if (existingSocket > 0) {
                sendPDU(clientSocket, NULL, 0, FLAG_HANDLE_ERROR);
                termClient(clientSocket);
            }
            else {
                addClientToTable(clientSocket, handle);
                sendPDU(clientSocket, NULL, 0, FLAG_HANDLE_ACK);
                printf("Client socket %d registered with handle: %s\n", clientSocket, handle);
            }
            break;
        }
        case FLAG_LIST_REQUEST:
        {
            printf("received list request\n");
            uint32_t clientCount = htonl(num_clients);  
            sendPDU(clientSocket, (uint8_t*)&clientCount, sizeof(uint32_t), FLAG_LIST_COUNT);
            
            for (int i = 0; i < num_clients; i++) {
                printf("sending handle %s\n", clientsTable[i].handle);
                uint8_t handleLen = strlen(clientsTable[i].handle);
                uint8_t *msg = malloc(1 + handleLen); 
                msg[0] = handleLen;
                memcpy(msg + 1, clientsTable[i].handle, handleLen);
                sendPDU(clientSocket, msg, 1 + handleLen, FLAG_LIST_HANDLE);
                free(msg);
            }
            
            sendPDU(clientSocket, NULL, 0, FLAG_LIST_END);  
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