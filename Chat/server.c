/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pduhandler.h"
#include "clientInfo.h"
#include "messageHandler.h"

#define DEBUG_FLAG 1

void addNewServerSocket(int mainServerSocket);
void checkServerArgs(int argc, char *argv[], char **port);

void addNewServerSocket(int mainServerSocket) 
{ 
    int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	char handle[MAX_HANDLE_LEN];
	recvPDU(clientSocket, (uint8_t*)handle, MAX_HANDLE_LEN);
	printf("New client connected with handle: %s\n", handle);
    addToPollSet(clientSocket);
    addClientToTable(clientSocket, handle);
}

int main(int argc, char *argv[])
{
    int mainServerSocket = 0;
    char *port;
    
    checkServerArgs(argc, argv, &port);

    clientsTable = (ClientInfo*)malloc(sizeof(ClientInfo) * 100);
    
    mainServerSocket = tcpServerSetup(strtol(port, NULL, 10));
    addToPollSet(mainServerSocket);

    while(1) {
        int socket = pollCall(-1);
        if (socket == mainServerSocket) {
            addNewServerSocket(mainServerSocket);
        }
        else {
            recvFromClient(socket);
        }
    }

    close(mainServerSocket);
    free(clientsTable);
    return 0;
}

void checkServerArgs(int argc, char *argv[], char **port) {
    if (argc == 1) {
        *port = "0";
        return;
    }

    if (argc != 2) {
        printf("Usage: %s [port-number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *port = argv[1];
}
