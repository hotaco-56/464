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
}

UDPClient::~UDPClient()
{
	printf("Client terminated\n");
    close(_socketNum);
}

void UDPClient::run()
{
	std::ifstream fromFile = openFromFile();

	bool ackReceived = false;
	// send filename packet
	sendFilenamePDU();
	pollCall(1000); //poll for ack
}

/*
	sends 32-bit header + window-size + buffer-size + to-filename
	max size : 7 + 2 + 2 + 100 = 111
*/
void UDPClient::sendFilenamePDU()
{
	__PRINTF_DBG("sending filename pdu\n");
	PDU pdu;
	pdu.setFlag(FILENAME);
	pdu.setSeqNum(0);
	pdu.addPayload((unsigned char*)&args.windowSize, sizeof(args.windowSize));
	pdu.addPayload((unsigned char*)&args.bufferSize, sizeof(args.bufferSize));
	pdu.addPayload((unsigned char*)args.toFilename, strlen(args.toFilename));
	unsigned char* data = pdu.getPDU();

	__PRINTF_DBG("Filename PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), pdu.getSeqNum(), pdu.getChksum());
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

int UDPClient::readFromStdin(char * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	printf("Enter data: ");
	while (inputLen < (args.bufferSize - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}