#include "udpClient.h"

UDPClient::UDPClient(UDPClientArgs& args) : 
	_args(args),
	_socketNum(setupUdpClientToServer(&_server, args.remoteMachine, args.remotePort)),
	_window(_args.windowSize)
{
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

	if (!setup())
		return;

	// start data transfer
	__PRINTF_DBG("============= DATA TRANS. ==============\n");
	while (!_window.closed()) {
		auto buffer = std::make_unique<unsigned char[]>(_args.bufferSize);

		_fromFile.read((char*)(buffer.get()), _args.bufferSize);
		std::streamsize bytesRead = _fromFile.gcount();

		__PRINTF_DBG("Read %d byes from %s\n", (int)bytesRead, _args.fromFilename);
		if (bytesRead > 0) {
			PDU dataPDU;
			dataPDU.setFlag(DATA);
			dataPDU.setSeqNum(++_pduSeqNum);
			dataPDU.addPayload((unsigned char*)buffer.get(), bytesRead);
			auto* pdu = dataPDU.createPDU();

			_window.update(dataPDU);
			safeSendto(_socketNum, pdu, dataPDU.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
		}
		else {
			__PRINTF_DBG("EOF reached\n");
			break;
		}

		if (pollCall(0) == _socketNum)
			recvPDU();
	}
	__PRINTF_DBG("Window closed\n");
	while (_window.closed()) {
		if (pollCall(1000) == -1) {
			__PRINTF_DBG("Timeout occured\n");
		}
		else {
			recvPDU();
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
			if (flag == FILENAME_ERR) {
				this->~UDPClient();
				exit(1);
			}
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

	_pduSeqNum = ntohl(pdu.getSeqNum()) + 1;

	if (pdu.calcChksum() != 0) {
		__PRINTF_DBG("Bad checksum on: seqNum %d\n", pdu.getSeqNum());
		return 0;
	}

	switch (pdu.getFlag())
	{
		case FILENAME_ACK:
		{
			__PRINTF_DBG("Received FILENAME_ACK pdu\n");
			unsigned char* payload = pdu.getPayload();
			int newPort = 0;
			memcpy(&newPort, payload, pdu.getPayloadLen());
			_args.remotePort = newPort;
			_socketNum = setupUdpClientToServer(&_server, _args.remoteMachine, _args.remotePort);
			__PRINTF_DBG("Connected to new socket with port: %d\n", _args.remotePort);
			break;
		}
		case FILENAME_ERR:
		{
			__PRINTF_DBG("Received FILENAME_ERR pdu\n");
			this->~UDPClient();
			exit(1);
			break;
		}
		case RR:
		{
			__PRINTF_DBG("Received RR pdu\n");
			break;
		}
		case SREJ:
		{
			__PRINTF_DBG("Received SREJ pdu\n");
			break;
		}
		default:
			break;
	}

	return pdu.getFlag();
}

/*
	sends 32-bit header + window-size + buffer-size + to-filename
	max size : 7 + 2 + 2 + 100 = 111
*/
void UDPClient::sendFilenamePDU()
{
	__PRINTF_DBG("Sending FILENAME pdu\n");
	PDU pdu;
	pdu.setFlag(FILENAME);
	pdu.setSeqNum(0);
	pdu.addPayload((unsigned char*)&_args.windowSize, sizeof(_args.windowSize));
	pdu.addPayload((unsigned char*)&_args.bufferSize, sizeof(_args.bufferSize));
	pdu.addPayload((unsigned char*)_args.toFilename, strlen(_args.toFilename));
	auto* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
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