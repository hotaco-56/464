#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "clientInfo.h"

ClientInfo *clientsTable;
uint32_t num_clients = 0;

void addClientToTable(int socketNum, char* handle)
{
    printf("adding client %s on socket %d to client table\n", handle, socketNum);
    // if (num_clients >= capacity) {
    //     clientsTable = realloc(clientsTable, capacity * sizeof(ClientInfo));
    //     capacity += capacity;
    // }
    
    clientsTable[num_clients].socketNum = socketNum;
    strcpy(clientsTable[num_clients].handle, handle);
    num_clients++;
}

void getClientSocketFromHandle(char* handle, int* socketNum) 
{
    *socketNum = -1;

    printf("looking for handle: %s\n", handle);
    for (uint16_t i = 0; i < num_clients; i++) {
        printf("%s\n", clientsTable[i].handle);
        if (strcmp(clientsTable[i].handle, handle) == 0) {
            *socketNum = clientsTable[i].socketNum;
            return;
        }
    }
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