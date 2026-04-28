#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "pduhandler.h"

#define HEADER_LEN 3

uint8_t* createPDU(uint8_t* dataBuffer, int lengthOfData, uint8_t flag) 
{
    uint8_t* pdu  = (uint8_t*)(malloc(lengthOfData + HEADER_LEN));


    uint16_t header = htons(lengthOfData);
    memcpy(pdu, &header, 2);
    memcpy(pdu + 2, &flag, 1);
    if (dataBuffer == NULL) {
        memset(pdu + 3, 0, lengthOfData);
    } else {
        memcpy(pdu+3, dataBuffer, lengthOfData);
    }
    return pdu;
}

int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData, uint8_t flag)
{
    uint8_t* pdu = createPDU(dataBuffer, lengthOfData, flag);
    ssize_t ret = send(clientSocket, pdu, lengthOfData+HEADER_LEN, 0);

    if (ret < 0)
        perror("error");

    free(pdu);
    return ret;
}

int recvPDU(int socketNum, uint8_t *dataBuffer, int maxLen, uint8_t *flag)
{
    uint8_t header[HEADER_LEN];
    uint16_t messageLen = 0;
    int bytesRead = 0;
    
    bytesRead = recv(socketNum, header, HEADER_LEN, 0);
    if (bytesRead == 0) {
        return 0; // Connection closed
    }
    
    if (bytesRead < 0) {
        return -1;
    }
    
    memcpy(&messageLen, header, 2);
    messageLen = ntohs(messageLen); 
    *flag = header[2];
    
    int dataLen = messageLen;
    if (dataLen > maxLen) {
        dataLen = maxLen;
    }
    
    // Receive data
    bytesRead = recv(socketNum, dataBuffer, dataLen, 0);
    if (bytesRead < 0) {
        return -1;
    }
    
    return messageLen;
}