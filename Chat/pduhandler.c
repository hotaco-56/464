#include "pduhandler.h"

uint8_t* createPDU(uint8_t* dataBuffer, int lengthOfData) 
{
    uint8_t* pdu  = (uint8_t*)(malloc(lengthOfData + 2));

    uint16_t header = htons(lengthOfData);
    memcpy(pdu, &header, 2);
    memcpy(pdu+2, dataBuffer, lengthOfData);
    return pdu;
}

int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData)
{
    uint8_t* pdu = createPDU(dataBuffer, lengthOfData);
    ssize_t ret = send(clientSocket, pdu, lengthOfData+2, 0);

    if (ret < 0)
        perror("error");

    free(pdu);
    return ret;
}

int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize)
{
    uint8_t header[2];
    if (recv(socketNumber, header, 2, MSG_WAITALL) <= 0)
        return 0;

    uint16_t pduLen;
    memcpy(&pduLen, header, 2);
    pduLen = ntohs(pduLen);

    if (pduLen > bufferSize) {
        perror("pduLen larger than recv bufferSize");
        return -1;
    }
    ssize_t ret = recv(socketNumber, dataBuffer, pduLen, MSG_WAITALL);

    if (ret < 0)
        perror("error");
    
    return ret;
}