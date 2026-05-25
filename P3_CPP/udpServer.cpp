#include "udpServer.h"

UDPServer::UDPServer(float errRate = 0.0f, int portNumber = 0) : 
	_errRate(errRate),
	_portNumber(portNumber),
	_socketNum(udpServerSetup(portNumber))
{
	setupPollSet();
	addToPollSet(_socketNum);
}

UDPServer::~UDPServer()
{
	__PRINTF_DBG("======================= CLEANUP ========================\n");
	if (_isChild)
		__PRINTF_DBG("Child terminated\n");
	else
		__PRINTF_DBG("Server deconstructor called\n");

	__PRINTF_DBG("Closed socket: %d\n", _socketNum);
	__PRINTF_DBG("Closed file: %s\n", _toFilename);
    close(_socketNum);
	_toFile.close();
}

void UDPServer::run()
{
	while(1) {
		pid_t terminatedProcess = waitpid(-1, nullptr, WNOHANG);
		if (terminatedProcess > 0)
			__PRINTF_DBG("Server cleaned up child process: %d\n", terminatedProcess);

		if (pollCall(0) == _socketNum)
			recvPDU();

		if (!_isChild)
			continue;

		// start data transfer
		__PRINTF_DBG("======================= DATA TRANS. ========================\n");
		if (pollCall(1000) == -1)
			__PRINTF_DBG("Timout occured\n");
		return;
	}
}

void UDPServer::recvPDU()
{
	int dataLen = 0;
	unsigned char data[MAX_PDU_SIZE];
	dataLen = safeRecvfrom(_socketNum, data, MAX_PDU_SIZE, 0, (struct sockaddr*) &_client, (int*)&_clientAddrLen);
	PDU pdu(data, dataLen);

	// throw packet if bad checksum
	if (pdu.calcChksum() != 0) {
		__PRINTF_DBG("Bad checksum (%d) on: seqNum %d\n", pdu.calcChksum(), pdu.getSeqNum());
		return;
	}
	
	_pduSeqNum = ntohl(pdu.getSeqNum()) + 1;

	switch (pdu.getFlag())
	{
		case FILENAME: // setup
		{
			__PRINTF_DBG("======================= SETUP ========================\n");
			parseFilenamePDU(pdu);

			// try open toFile
			openToFile(_toFilename);
			if (!_toFile)
				return;

			pid_t pid = 0;
			pid = fork();

			if (pid != 0) { 
				__PRINTF_DBG("Child Process Created: %d\n", pid);
				return;
			}
			else { // in child
				_isChild = true;
				
				// child needs to reconfigure server for new socket
				removeFromPollSet(_socketNum);
				close(_socketNum);
				_socketNum = udpServerSetup(_portNumber);

				sendFilenameAck();
			}
			break;
		}
		
		case DATA:
		{
			break;
		}
		
		default:
			break;
	}
}

void UDPServer::parseFilenamePDU(PDU pdu)
{
	// parse payload
	unsigned char* payload = pdu.getPayload();
	uint16_t fileNameLen = pdu.getPayloadLen() - sizeof(_windowSize) - sizeof(_bufferSize);

	memcpy(&_windowSize, payload, sizeof(_windowSize));
	payload += sizeof(_windowSize);
	memcpy(&_bufferSize, payload, sizeof(_bufferSize));
	payload += sizeof(_bufferSize);
	memcpy(_toFilename, payload, fileNameLen);
	_toFilename[fileNameLen] = '\0';

	__PRINTF_DBG("Received filename pdu from client with ");
	#ifdef __DEBUG_
	printIPInfo(&_client);
	#endif
	__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
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
	pdu.addPayload((unsigned char*)&_portNumber, sizeof(_portNumber));
	unsigned char* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME_ACK PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n\tportNumber: %d\n",
		 pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum(), _portNumber);
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, _clientAddrLen);
}

void UDPServer::sendFilenameErr()
{
	unsigned char padding = (unsigned char)0; // for min pdusize = 8bytes
	__PRINTF_DBG("Sending FILENAME_ERR pdu\n");
	PDU pdu;
	pdu.setFlag(FILENAME_ERR);
	pdu.setSeqNum(_pduSeqNum);
	pdu.addPayload(&padding, 1);
	unsigned char* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME_ERR PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n",
		 pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, _clientAddrLen);
}

void UDPServer::openToFile(char* toFilename)
{
    if (_toFile.is_open()) {
        _toFile.close();
    }

    _toFile.clear(); // clear any previous fail state

    _toFile.open(toFilename, std::ios::binary);

    if (!_toFile) {
        printf("Failed to open file %s\n", toFilename);
        sendFilenameErr();
    }
}