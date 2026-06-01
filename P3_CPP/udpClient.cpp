#include "udpClient.h"

UDPClient::UDPClient(UDPClientArgs& args) : 
	_args(args),
	_socketNum(setupUdpClientToServer(&_server, args.remoteMachine, args.remotePort)),
	_window(_args.windowSize)
{
	setupPollSet();
	addToPollSet(_socketNum);

	#ifdef __SEND_ERR_
	sendErr_init((double)_args.errorRate, DROP_ON, FLIP_ON, __SEND_ERR_DBG_, RSEED_ON);
	#else
	sendErr_init((double)_args.errorRate, DROP_OFF, FLIP_OFF, __SEND_ERR_DBG_, RSEED_OFF);
	#endif
}

UDPClient::~UDPClient()
{
	printf("Client terminated\n");
    close(_socketNum);
}

void UDPClient::run()
{
	// try open fromFile
	openFromFile();
	if (!_fromFile)
		return;

	// send filename
	if (!setup())
		return;

	// start data transfer
	__PRINTF_DBG("============= DATA TRANS. ==============\n");
	bool transferFinished = false;
	while (!(transferFinished && _window.isAcked())) {
		if (!_window.isClosed() && !transferFinished) {
			std::streamsize bytesSent = sendDataPDU();

			if (pollCall(0) == _socketNum)
				recvPDU();

			if (bytesSent == 0) {
				transferFinished = true;
				__PRINTF_DBG("EOF reached\n");
				lastDataPDUSeqNum = _pduSeqNum;
			}
		}
		else  {
			__PRINTF_DBG("Window closed\n");
			if (pollCall(1000) != _socketNum) {
				resendLowestPDU();
				_timeoutcount++;
			}
			else if (!_window.isAcked()){
				recvPDU();
				_timeoutcount = 0;
			}
		}
	}
	__PRINTF_DBG("Starting Teardown\n");
	teardown();
}

void UDPClient::teardown()
{
	uint8_t count = 0;
	while(count < MAX_RETRANSMIT_COUNT) {
		sendTeardownPDU();

		if (pollCall(1000) != _socketNum) {
			sendTeardownPDU();
			count++;
		}
		else {
			if (recvPDU() == TEARDOWN_ACK) {
				sendFinPDU();
				return;
			}
		}
	}	
}

bool UDPClient::setup()
{
	uint8_t retransmitCount = 0;
	while (retransmitCount < MAX_RETRANSMIT_COUNT) {
		sendFilenamePDU();
		if (pollCall(1000)  == _socketNum) {
			uint8_t flag = recvPDU();
			if (flag == FILENAME_ACK)
				return true;
			else
				break;
		}
		retransmitCount++;
	}
	__PRINTF_DBG("Setup failed: retransmit count: %d\n", retransmitCount);
	return false;
}

uint8_t UDPClient::recvPDU()
{
	unsigned char data[MAX_PDU_SIZE] = {0};
	int dataLen = 0;
	dataLen = safeRecvfrom(_socketNum, data, MAX_PDU_SIZE, 0, (struct sockaddr*) &_server, &_serverAddrLen);
	PDU pdu(data, dataLen);

	if (pdu.calcChksum() != 0) {
		__PRINTF_DBG("Bad checksum on: seqNum %d\n", pdu.getSeqNum());
		return 0;
	}

	switch (pdu.getFlag())
	{
		case FILENAME_ACK:
		{
			__PRINTF_DBG("FILENAME_ACK PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum());
			break;
		}
		case FILENAME_ERR:
		{
			__PRINTF_DBG("FILENAME_ERR PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum());

			this->~UDPClient();
			exit(1);
			break;
		}
		case RR:
		{
			uint32_t rrVal = 0;
			memcpy(&rrVal, pdu.getPayload(), sizeof(uint32_t));
			rrVal = ntohl(rrVal);
			__PRINTF_DBG("RR PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n\trr: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum(),
				rrVal);

			_window.ack(rrVal);
			
			break;
		}
		case SREJ:
		{
			__PRINTF_DBG("Received SREJ pdu\n");
			uint32_t srejVal = 0;
			memcpy(&srejVal, pdu.getPayload(), sizeof(uint32_t));
			srejVal = ntohl(srejVal);
			__PRINTF_DBG("SREJ PDU receved\n");
			__PRINTF_DBG("\tPDULen: %d\n\tPayloadLen: %d\n", pdu.getPDULen(), pdu.getPayloadLen());
			__PRINTF_DBG("\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n\tsrej: %d\n", 
				pdu.getFlag(),
				ntohl(pdu.getSeqNum()), 
				pdu.getChksum(),
				srejVal);

			sendDataPDU(srejVal);
			break;
		}
		default:
			break;
	}

	return pdu.getFlag();
}

void UDPClient::sendFinPDU()
{
	PDU pdu;
	pdu.setFlag(FIN);
	pdu.setSeqNum(_pduSeqNum++);
	unsigned char padding = 0;
	pdu.addPayload(&padding, 1);
	auto* data = pdu.createPDU();

	__PRINTF_DBG("FIN PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		pdu.getFlag(), 
		ntohl(pdu.getSeqNum()), 
		pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
}

void UDPClient::sendTeardownPDU()
{
	PDU pdu;
	pdu.setFlag(TEARDOWN);
	pdu.setSeqNum(_pduSeqNum++);
	pdu.addPayload((unsigned char*)&lastDataPDUSeqNum, sizeof(seqNum_t));
	auto* data = pdu.createPDU();

	__PRINTF_DBG("TEARDOWN PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		pdu.getFlag(), 
		ntohl(pdu.getSeqNum()), 
		pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
}

void UDPClient::resendLowestPDU()
{
	seqNum_t lowest = _window.getLowestUnacked();
	PDU_T dataPDU = _window.get(lowest);
	__PRINTF_DBG("DATA PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		dataPDU.flag, 
		ntohl(dataPDU.seqNum), 
		dataPDU.chksum);
	safeSendto(_socketNum, &dataPDU, dataPDU.pduLen, 0, (struct sockaddr*) &_server, _serverAddrLen);
}

void UDPClient::sendDataPDU(seqNum_t seqNum)
{
	if (!_window.contains(seqNum))
		return;
	PDU_T dataPDU = _window.get(seqNum);
	__PRINTF_DBG("SREJ requested %u, slot contains %u, valid=%d\n", seqNum, ntohl(dataPDU.seqNum), dataPDU.valid);

	__PRINTF_DBG("DATA PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		dataPDU.flag, 
		ntohl(dataPDU.seqNum), 
		dataPDU.chksum);
	safeSendto(_socketNum, &dataPDU, dataPDU.pduLen, 0, (struct sockaddr*) &_server, _serverAddrLen);
}

std::streamsize UDPClient::sendDataPDU()
{
	auto buffer = std::make_unique<unsigned char[]>(_args.bufferSize);

	_fromFile.read((char*)(buffer.get()), _args.bufferSize);
	std::streamsize bytesRead = _fromFile.gcount();

	if (bytesRead > 0) {
		PDU dataPDU;
		dataPDU.setFlag(DATA);
		dataPDU.setSeqNum(_pduSeqNum++);
		dataPDU.addPayload((unsigned char*)buffer.get(), bytesRead);
		auto* pdu = dataPDU.createPDU();
		_window.update(*pdu, dataPDU.getPDULen());

		__PRINTF_DBG("DATA PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
			dataPDU.getFlag(), 
			ntohl(dataPDU.getSeqNum()), 
			dataPDU.getChksum());
		safeSendto(_socketNum, pdu, dataPDU.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
	}

	return bytesRead;
}

/*
	sends 32-bit header + window-size + buffer-size + to-filename
	max size : 7 + 2 + 2 + 100 = 111
*/
void UDPClient::sendFilenamePDU()
{
	PDU pdu;
	pdu.setFlag(FILENAME);
	pdu.setSeqNum(_pduSeqNum++);
	pdu.addPayload((unsigned char*)&_args.windowSize, sizeof(_args.windowSize));
	pdu.addPayload((unsigned char*)&_args.bufferSize, sizeof(_args.bufferSize));
	pdu.addPayload((unsigned char*)_args.toFilename, strlen(_args.toFilename));
	auto* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME PDU SENT:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", 
		pdu.getFlag(), 
		ntohl(pdu.getSeqNum()), 
		pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
}

void UDPClient::openFromFile()
{
    if (_fromFile.is_open()) {
        _fromFile.close();
    }

    _fromFile.clear(); // clear any previous fail state

    _fromFile.open(_args.fromFilename, std::ios::binary);

    if (!_fromFile) {
        printf("Failed to open file %s\n", _args.fromFilename);
    }
}