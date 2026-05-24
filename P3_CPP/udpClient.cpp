#include "udpClient.h"

void UDPClient::retransmitCallback(void (*send)())
{
	bool ackReceived = false;
	int retransmitCount = 0;
	while (!ackReceived)
	{
		if (++retransmitCount > 10)
			this->~UDPClient();
		send();
	}
}

UDPClient::UDPClient(UDPClientArgs& args) : 
	args(args),
	_socketNum(setupUdpClientToServer(&_server, args.remoteMachine, args.remotePort))
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

	// send filename packet
	sendFilenamePDU();
	if (pollCall(1000) == _socketNum) { //poll for ack
		recvPDU();
	}
}

void UDPClient::recvPDU()
{
	unsigned char data[MAX_PDU_SIZE] = {0};
	int dataLen = 0;
	dataLen = safeRecvfrom(_socketNum, data, MAX_PDU_SIZE, 0, (struct sockaddr*) &_server, &serverAddrLen);

	PDU pdu(data, dataLen);

	switch (pdu.getFlag())
	{
		case FILENAME_ACK:
		{
			__PRINTF_DBG("Received FILENAME_ACK pdu\n");
			break;
		}
	
		default:
			break;
	}
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
	pdu.addPayload((unsigned char*)&args.windowSize, sizeof(args.windowSize));
	pdu.addPayload((unsigned char*)&args.bufferSize, sizeof(args.bufferSize));
	pdu.addPayload((unsigned char*)args.toFilename, strlen(args.toFilename));
	unsigned char* data = pdu.getPDU();

	__PRINTF_DBG("FILENAME PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), pdu.getSeqNum(), pdu.getChksum());
	safeSendto(_socketNum, data, pdu.getPDULen(), 0, (struct sockaddr*) &_server, serverAddrLen);
}

void UDPClient::sendDataPDU()
{
	__PRINTF_DBG("sending data pdu\n");
}

std::ifstream UDPClient::openFromFile()
{
	std::ifstream fromFile(args.fromFilename);

	if (!fromFile) {
        printf("File %s not found\n", args.fromFilename);
		this->~UDPClient();
		exit(1);
    }

	return fromFile;
}
