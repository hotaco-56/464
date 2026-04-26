#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdint.h>

void sendToClient(int clientSocket, uint8_t * buffer, int lengthOfData);
void recvFromClient(int clientSocket);
void getClientSocketFromHandle(char* handle, int* socketNum);
void termClient(int clientSocket);


#endif