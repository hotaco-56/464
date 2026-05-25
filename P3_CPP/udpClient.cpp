#include "udpClient.h"

UDPClient::UDPClient(UDPClientArgs& args) : 
	_args(args),
	_socketNum(setupUdpClientToServer(&_server, args.remoteMachine, args.remotePort)),
	_window(_args.windowSize, _args.bufferSize)
{
	addToPollSet(_socketNum);
}

UDPClient::~UDPClient()
{
	printf("Client terminated\n");
    close(_socketNum);
}

void UDPClient::run()
{
	std::ifstream fromFile = openFromFile();

	if (!setup())
		return;

	// start data transfer
	sendFilenamePDU();
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
	unsigned char* data = pdu.createPDU();

	__PRINTF_DBG("FILENAME PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), pdu.getSeqNum(), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_server, _serverAddrLen);
}

void UDPClient::sendDataPDU()
{
	__PRINTF_DBG("sending data pdu\n");
}

std::ifstream UDPClient::openFromFile()
{
	std::ifstream fromFile(_args.fromFilename);

	if (!fromFile) {
        printf("File %s not found\n", _args.fromFilename);
		this->~UDPClient();
		exit(1);
    }

	return fromFile;
}
