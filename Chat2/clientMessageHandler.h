#ifndef C_MESSAGE_HANDLER_H
#define C_MESSAGE_HANDLER_H

#include <stdint.h>

void sendHandleInitial(int socketNum, char *handle);
void sendToServer(int clientSocket, uint8_t * buffer, int lengthOfData);
void recvFromServer(int clientSocket);

#endif