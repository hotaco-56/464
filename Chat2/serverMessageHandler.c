#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "serverMessageHandler.h"
#include "clientInfo.h"
#include "pduhandler.h"
#include "pollLib.h"
#include "ctype.h"
#include "messageFlags.h"

#define DEBUG_FLAG 1

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
    uint8_t flag = 0;

    if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF, &flag)) < 0) {
        perror("recv call");
        exit(-1);
    }

    printf("Received data from client socket %d, message length: %d, flag: %d\n", clientSocket, messageLen, flag);

    if (messageLen > 0) {
        switch (flag)
        {
        case FLAG_HANDLE_INITIAL:
        {
            uint8_t handleLen = dataBuffer[0];  
            char handle[MAX_HANDLE_LEN];
            memcpy(handle, dataBuffer + 1, handleLen);  
            handle[handleLen] = '\0';
            printf("Socket %d: Received handle: %s\n", clientSocket, handle);

            addToPollSet(clientSocket);
            
            // Check if handle already exists
            int existingSocket = -1;
            getClientSocketFromHandle(handle, &existingSocket);
            
            if (existingSocket > 0) {
                sendPDU(clientSocket, NULL, 1, FLAG_HANDLE_ERROR);
                termClient(clientSocket);
            }
            else {
                addClientToTable(clientSocket, handle);
                sendPDU(clientSocket, NULL, 1, FLAG_HANDLE_ACK);
                printf("Client socket %d registered with handle: %s\n", clientSocket, clientsTable[num_clients - 1].handle);
            }
            break;
        }
        case FLAG_MESSAGE:
        {
            int receiverSocket = 0;
            uint8_t handleLen = dataBuffer[0];
            char handle[MAX_HANDLE_LEN];
            memcpy(handle, &dataBuffer[1], handleLen);
            handle[handleLen] = '\0';
            handleLen += 1; //accounting for space

            uint8_t msgOffset = handleLen + 1;

            printf("Received message for handle: %s\n", handle);
            printf("total buffer: %s\n", dataBuffer);
            printf("Message content: %s\n", dataBuffer + msgOffset);

            uint8_t *msgData = dataBuffer + msgOffset; //this accounts for +1 flag
            uint8_t msgSize = messageLen - msgOffset;

            getClientSocketFromHandle(handle, &receiverSocket);

            printf("Handle is socket %d\n", receiverSocket);

            if (receiverSocket > 0) {
                printf("Sending Message to %s\n: %s", handle, msgData);
                sendPDU(receiverSocket, msgData, msgSize, FLAG_MESSAGE);
            } else {
                printf("handle not found\n");
                sendPDU(clientSocket, NULL, 1, FLAG_MESSAGE_ERROR);
            }
            break;
        }
        case FLAG_LIST_REQUEST:
        {
            printf("received list request\n");
            uint32_t clientCount = num_clients;  
            sendPDU(clientSocket, (uint8_t*)&clientCount, sizeof(clientCount), FLAG_LIST_COUNT);
            
            for (int i = 0; i < num_clients; i++) {
                printf("sending handle %s\n", clientsTable[i].handle);
                uint8_t handleLen = strlen(clientsTable[i].handle);
                uint8_t *msg = malloc(1 + handleLen); 
                msg[0] = handleLen;
                memcpy(msg + 1, clientsTable[i].handle, handleLen);
                sendPDU(clientSocket, msg, 1 + handleLen, FLAG_LIST_HANDLE);
                free(msg);
            }
            
            sendPDU(clientSocket, NULL, 1, FLAG_LIST_END);  
            break;
        }
        case FLAG_MULTICAST:
        {
            printf("recevied multicast request\n");
            uint8_t multicastHosts = dataBuffer[0];
            printf("multicast hosts: %d\n", multicastHosts);

            //find msg
            uint8_t *msgPtr = dataBuffer;
            uint8_t handleLen = 0;
            for (int i = 0; i < multicastHosts; i++) {
                handleLen = dataBuffer[i + handleLen + 1];
                msgPtr += 1 + handleLen;
            }

            //send to hosts
            handleLen = 0;
            uint8_t *handlePtr = dataBuffer + 1;
            for (int i = 0; i < multicastHosts; i++) {
                char handle[MAX_HANDLE_LEN];
                memset(handle, 0, MAX_HANDLE_LEN);
                handleLen = dataBuffer[i + handleLen + 1];
                printf("handle len: %d\n", handleLen);

                memcpy(handle, handlePtr + 1, handleLen);

                handle[handleLen] = '\0';
                printf("handle: %s\n", handle);

                int socketNum = 0;
                getClientSocketFromHandle(handle, &socketNum);

                if (socketNum > 0) {
                    printf("Sending Message to %s\n: %s", handle, msgPtr);
                    sendPDU(socketNum, msgPtr + 1, msgPtr - 1 - dataBuffer, FLAG_MESSAGE);
                } else {
                    printf("handle not found\n");
                    sendPDU(clientSocket, NULL, 1, FLAG_MESSAGE_ERROR);
                }

                handlePtr += handleLen + 1;
            }
            break;
        }
        case FLAG_BROADCAST:
        {
            for (int i = 0; i < num_clients; i++) {
                sendPDU(clientsTable[i].socketNum, dataBuffer, messageLen, FLAG_MESSAGE);
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

