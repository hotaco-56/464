#include "udpServer.h"

void setupSignalHandlers();

UDPServer::~UDPServer()
{
	__PRINTF_DBG("Server deconstructor called\n");
    close(_socketNum);
}

void UDPServer::run()
{
	setupSignalHandlers();
	setupPollSet();
	addToPollSet(this->_socketNum);

	while(1) {
		recvFilenamePDU();
	}
}

void UDPServer::recvFilenamePDU()
{
	int dataLen = 0;
	unsigned char pdu[MAX_PDU_SIZE];
	dataLen = safeRecvfrom(_socketNum, pdu, MAX_PDU_SIZE, 0, (struct sockaddr*) &_client, (int*)&clientAddrLen);
	PDU filenamePDU(pdu, dataLen);

	// parse payload
	unsigned char* payload = filenamePDU.getPayload();

	memcpy(&_windowSize, payload, sizeof(_windowSize));
	payload += 2;
	memcpy(&_bufferSize, payload, sizeof(_bufferSize));
	payload += 2;
	memcpy(_toFilename, payload, dataLen - 4 - HEADER_SIZE);
	_toFilename[dataLen - 4 - HEADER_SIZE] = '\0';

	__PRINTF_DBG("Recevied filename pdu from client with ");
	#ifdef __DEBUG_
	printIPInfo(&_client);
	#endif
	__PRINTF_DBG("\tPDULen: %d \'%s\'\n\tPayloadLen: %d\n", dataLen, pdu, filenamePDU.getPayloadLen());
	__PRINTF_DBG("Filename PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		filenamePDU.getFlag(),
		filenamePDU.getSeqNum(), 
		filenamePDU.getChksum());
	__PRINTF_DBG("\twindowSize: %d\n\tbufferSize: %d\n\tfileName: %s\n", _windowSize, _bufferSize, _toFilename);
}

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