#ifndef S_MESSAGE_HANDLER_H
#define S_MESSAGE_HANDLER_H

#include <stdint.h>


void sendToClient(int clientSocket, uint8_t * buffer, int lengthOfData);
void recvFromClient(int clientSocket);
void termClient(int clientSocket);

#endif