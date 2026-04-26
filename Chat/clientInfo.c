#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "clientInfo.h"

ClientInfo *clientsTable;
uint32_t num_clients = 0;

void addClientToTable(int socketNum, char* handle)
{
    clientsTable[num_clients].socketNum = socketNum;
    strcpy(clientsTable[num_clients].handle, handle);
    num_clients++;
}

void removeClientFromTable(int socketNum)
{
    for (uint16_t i = 0; i < num_clients; i++) {
        if (clientsTable[i].socketNum == socketNum) {
            for (uint16_t j = i; j < num_clients - 1; j++) {
                clientsTable[j] = clientsTable[j + 1];
            }
            num_clients--;
            break;
        }
    }
}