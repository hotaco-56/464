/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pduhandler.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

typedef struct {
    char *port;
} ServerArgs;

void recvFromClient(int clientSocket);
void checkServerArgs(int argc, char *argv[], ServerArgs *args);
void termClient(int clientSocket);
void addNewServerSocket(int mainServerSocket) { addToPollSet(tcpAccept(mainServerSocket, DEBUG_FLAG)); }

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	
	ServerArgs args;
	checkServerArgs(argc, argv, &args);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(args.port);

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
	return 0;
}


void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Message Received on Socket %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
		
		// send it back to client (just to test sending is working... e.g. debugging)
		messageLen = sendPDU(clientSocket, dataBuffer, messageLen);
		printf("Socket %d: msg sent: %d bytes, text: %s\n", clientSocket, messageLen, dataBuffer);
	}
	else
	{
		printf("Socket %d: Connection closed by other side\n", clientSocket);
		termClient(clientSocket);
	}

}

void termClient(int clientSocket) {
	removeFromPollSet(clientSocket);
	close(clientSocket);
}

void checkServerArgs(int argc, char *argv[], ServerArgs *args) {
    if (argc == 1) {
        args->port = 0;
        return;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port-number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    args->port = argv[1];
}
