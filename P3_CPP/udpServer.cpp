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
	addToPollSet(_socketNum);

	while(1) {
		recvFilename();
		sendFilenameAck();
	}
}

void UDPServer::recvFilename()
{
	int dataLen = 0;
	unsigned char data[MAX_PDU_SIZE];
	dataLen = safeRecvfrom(_socketNum, data, MAX_PDU_SIZE, 0, (struct sockaddr*) &_client, (int*)&clientAddrLen);
	PDU pdu(data, dataLen);
	_pduSeqNum = ntohl(pdu.getSeqNum()) + 1;

	// parse payload
	unsigned char* payload = pdu.getPayload();

	memcpy(&_windowSize, payload, sizeof(_windowSize));
	payload += 2;
	memcpy(&_bufferSize, payload, sizeof(_bufferSize));
	payload += 2;
	memcpy(_toFilename, payload, dataLen - 4 - HEADER_SIZE);
	_toFilename[dataLen - 4 - HEADER_SIZE] = '\0';

	__PRINTF_DBG("Received filename pdu from client with ");
	#ifdef __DEBUG_
	printIPInfo(&_client);
	#endif
	__PRINTF_DBG("\tPDULen: %d \'%s\'\n\tPayloadLen: %d\n", dataLen, data, pdu.getPayloadLen());
	__PRINTF_DBG("Filename PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		pdu.getFlag(),
		pdu.getSeqNum(), 
		pdu.getChksum());
	__PRINTF_DBG("\twindowSize: %d\n\tbufferSize: %d\n\tfileName: %s\n", _windowSize, _bufferSize, _toFilename);
}

void UDPServer::sendFilenameAck()
{
	__PRINTF_DBG("Sending FILENAME_ACK pdu\n");
	PDU pdu;
	pdu.setFlag(FILENAME_ACK);
	pdu.setSeqNum(_pduSeqNum);
	unsigned char* data = pdu.getPDU();

	__PRINTF_DBG("FILENAME_ACK PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, clientAddrLen);
}

void processNewClient()
{
	__PRINTF_DBG("Processing New Client\n");
	
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