#include "udpServer.h"

void setupSignalHandlers();

UDPServer::~UDPServer()
{
	__PRINTF_DBG("Server deconstructor called\n");
    close(socketNum);
}

void UDPServer::run()
{
	setupSignalHandlers();
	setupPollSet();
	addToPollSet(this->socketNum);

	while(1) {
		recvFilenamePDU();
	}
}

void UDPServer::recvFilenamePDU()
{
	int dataLen = 0;
	unsigned char pdu[MAX_PDU_SIZE];
	dataLen = safeRecvfrom(socketNum, pdu, MAX_PDU_SIZE, 0, (struct sockaddr*) &client, (int*)&clientAddrLen);
	PDU filenamePDU(pdu, dataLen);

	unsigned char* payload = filenamePDU.getPayload();
	char fromFilename[100 + 1];
	uint16_t windowSize = 0;
	uint16_t bufferSize = 0;

	memcpy(&windowSize, payload, sizeof(windowSize));
	payload += 2;
	memcpy(&bufferSize, payload, sizeof(bufferSize));
	payload += 2;
	memcpy(fromFilename, payload, dataLen - 4 - HEADER_SIZE);
	fromFilename[dataLen - 4 - HEADER_SIZE] = '\0';

	__PRINTF_DBG("Recevied filename pdu from client with ");
	#ifdef __DEBUG_
	printIPInfo(&client);
	#endif
	__PRINTF_DBG("\tPDULen: %d \'%s\'\n\tPayloadLen: %d\n", dataLen, pdu, filenamePDU.getPayloadLen());
	__PRINTF_DBG("Filename PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", filenamePDU.getFlag(), filenamePDU.getSeqNum(), filenamePDU.getChksum());
	__PRINTF_DBG("\twindowSize: %d\n\tbufferSize: %d\n\tfileName: %s\n", windowSize, bufferSize, fromFilename);
}

// void UDPServer::processClient()
// {
// 	int dataLen = 0; 
// 	char buffer[MAXBUF + 1];	  
// 	struct sockaddr_in6 client;		
// 	int clientAddrLen = sizeof(client);	
	
// 	buffer[0] = '\0';
// 	while (buffer[0] != '.')
// 	{
// 		dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &client, &clientAddrLen);
	
// 		printf("Received message from client with ");
// 		printIPInfo(&client);
// 		printf();

// 		// just for fun send back to client number of bytes received
// 		sprintf(buffer, "bytes: %d", dataLen);
// 		safeSendto(socketNum, buffer, strlen(buffer)+1, 0, (struct sockaddr *) & client, clientAddrLen);

// 	}
// }

void sigchldHandler(int signo)
{
	pid_t childPID;
    while ((childPID = waitpid(-1, nullptr, WNOHANG)) > 0) {
		  printf("Child %d terminated\n", childPID);
    }
}

void setupSignalHandlers()
{
    struct sigaction sa;

    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(1);
    }
}