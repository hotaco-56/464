#define DEBUG

#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <stdint.h>

#define MAX_HANDLE_LEN 100

typedef struct
{
    char handle[MAX_HANDLE_LEN];
    int socketNum;
} ClientInfo;

extern ClientInfo *clientsTable;
extern volatile uint32_t num_clients;

void addClientToTable(int socketNum, char *handle);
void removeClientFromTable(int socketNum);

#endif