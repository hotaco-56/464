#include "udpServer.h"

UDPServer::UDPServer(float errRate = 0.0f, int portNumber = 0) : 
	_errRate(errRate),
	_portNumber(portNumber),
	_socketNum(udpServerSetup(portNumber))
{
	setupPollSet();
	addToPollSet(_socketNum);
	#ifdef __SEND_ERR_
	sendErr_init((double)_errRate, DROP_ON, FLIP_ON, __SEND_ERR_DBG_, RSEED_ON);
	#else
	sendErr_init((double)_errRate, DROP_OFF, FLIP_OFF, __SEND_ERR_DBG_, RSEED_OFF);
	#endif
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

		_expectedSeqNum = 0;
		if (pollCall(0) == _socketNum)
			recvPDU();

		if (_isChild)
			break;
	}

	// start data transfer
	__PRINTF_DBG("======================= DATA TRANS. ========================\n");
	int i = 0;
	while (1) {
		if (pollCall(1000) == -1) {
			__PRINTF_DBG("Timeout occured\n");
			if (++i == MAX_RETRANSMIT_COUNT)
				return;
		}
		else {
			__PRINTF_DBG("expected: %d\n", _expectedSeqNum);
			recvPDU();
		}
	}
	return;
}

void UDPServer::processDataPDU(PDU pdu)
{
	uint32_t recvdSeqNum = ntohl(pdu.getSeqNum());
	switch (_state)
	{
		case IN_ORDER:
		{
			if (recvdSeqNum == _expectedSeqNum) {
				__PRINTF_DBG("in order received\n");
				_toFile.write((char*)pdu.getPayload(), pdu.getPayloadLen());
				_toFile.flush();
				sendRR();
				_expectedSeqNum++;
			}
			else if (recvdSeqNum > _expectedSeqNum) {
				__PRINTF_DBG("received %d when expected: %d\n", recvdSeqNum, _expectedSeqNum);
				_state = BUFFERING;
			}
			else if (recvdSeqNum < _expectedSeqNum) {
				__PRINTF_DBG("received %d when expected: %d\n", recvdSeqNum, _expectedSeqNum);
				_state = BUFFERING;
			}
			break;
		}

		case BUFFERING:
		{
			__PRINTF_DBG("buffering\n");
			break;
		}
		
		case FLUSHING:
		{

			break;
		}
		
		default:
			break;
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
	
	switch (pdu.getFlag())
	{
		case FILENAME: // setup
		{
			__PRINTF_DBG("======================= SETUP ========================\n");
			__PRINTF_DBG("FILENAME PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum());
			processFilenamePDU(pdu);
			__PRINTF_DBG("\twindowSize: %d\n\tbufferSize: %d\n\tfileName: %s\n", _windowSize, _bufferSize, _toFilename);
			setup();
			_expectedSeqNum++;
			break;
		}
		
		case DATA:
		{
			__PRINTF_DBG("DATA PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum());

			processDataPDU(pdu);
			break;
		}
		
		default:
			break;
	}
}

void UDPServer::setup()
{
	#ifdef __DEBUG_
	printIPInfo(&_client);
	#endif

	pid_t pid = 0;
	pid = fork();

	if (pid != 0) { 
		__PRINTF_DBG("Child Process Created: %d\n", pid);
		return;
	}
	else { // in child
		_isChild = true;
		// try open toFile
		openToFile(_toFilename);
		if (!_toFile)
			return;
		
		// child needs to reconfigure server for new socket
		#ifdef __SEND_ERR_
		sendErr_init((double)_errRate, DROP_ON, FLIP_ON, __SEND_ERR_DBG_, RSEED_ON);
		#else
		sendErr_init((double)_errRate, DROP_OFF, FLIP_OFF, __SEND_ERR_DBG_, RSEED_OFF);
		#endif

		removeFromPollSet(_socketNum);
		close(_socketNum);
		_socketNum = udpServerSetup(_portNumber);
		addToPollSet(_socketNum);

		sendFilenameAck();
	}
}

void UDPServer::processFilenamePDU(PDU pdu)
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
}

void UDPServer::sendFilenameAck()
{
	unsigned char padding = (unsigned char)0; // for min pdusize = 8bytes
	PDU pdu;
	pdu.setFlag(FILENAME_ACK);
	pdu.setSeqNum(_pduSeqNum++);
	pdu.addPayload(&padding, 1);
	auto* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME_ACK PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n",
		 pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, _clientAddrLen);
}

void UDPServer::sendFilenameErr()
{
	unsigned char padding = (unsigned char)0; // for min pdusize = 8bytes
	PDU pdu;
	pdu.setFlag(FILENAME_ERR);
	pdu.setSeqNum(_pduSeqNum++);
	pdu.addPayload(&padding, 1);
	auto* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME_ERR PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n",
		 pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, _clientAddrLen);
}

void UDPServer::sendRR()
{
	uint32_t rrVal = htonl(_expectedSeqNum);
	PDU pdu;
	pdu.setFlag(RR);
	pdu.setSeqNum(_pduSeqNum++);
	pdu.addPayload((unsigned char*)&rrVal, sizeof(_expectedSeqNum));
	auto* data = pdu.createPDU();

	__PRINTF_DBG("RR PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n\trr: %d\n",
		 pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum(), ntohl(rrVal));
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_client, _clientAddrLen);
}

void UDPServer::openToFile(char* toFilename)
{
    if (_toFile.is_open()) {
        _toFile.close();
    }

    _toFile.clear(); // clear any previous fail state

    _toFile.open(toFilename, std::ios::binary | std::ios::trunc | std::ios::out);

    if (!_toFile) {
        printf("Failed to open file %s\n", toFilename);
        sendFilenameErr();
    }
}