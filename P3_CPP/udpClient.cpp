#include "udpClient.h"

UDPClient::UDPClient(UDPClientArgs& args) : 
	args(args),
	socketNum(setupUdpClientToServer(&server, args.remoteMachine, args.remotePort))
{
}

UDPClient::~UDPClient()
{
	printf("Client terminated\n");
    close(socketNum);
}

void UDPClient::run()
{
	std::ifstream fromFile = openFromFile();

	// send filename packet
	sendFilenamePDU();
	pollCall(1000);
}

void UDPClient::sendFilenamePDU()
{
	__PRINTF_DBG("sending filename pdu\n");
	unsigned char buffer[args.bufferSize+1];
	PDU pdu;
	pdu.setFlag(FILENAME);
	pdu.setSeqNum(0);
	pdu.calcChksum(HEADER_SIZE);
	pdu.headerCpy(buffer);
	pdu.setPayload((unsigned char*)args.fromFilename, strlen(args.fromFilename));
	__PRINTF_DBG("Filename PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), pdu.getSeqNum(), pdu.getChksum());
	safeSendto(socketNum, buffer, pdu.getPDULen(), 0, (struct sockaddr*) &server, serverAddrLen);
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

void UDPClient::talkToServer()
{
	int serverAddrLen = sizeof(struct sockaddr_in6);
	char * ipString = NULL;
	int dataLen = 0; 
	char buffer[args.bufferSize+1];
	
	buffer[0] = '\0';
	while (buffer[0] != '.')
	{
		dataLen = readFromStdin(buffer);

		printf("Sending: %s with len: %d\n", buffer,dataLen);
	
		safeSendto(socketNum, buffer, dataLen, 0, (struct sockaddr *) &server, serverAddrLen);
		safeRecvfrom(socketNum, buffer, args.bufferSize, 0, (struct sockaddr *) &server, &serverAddrLen);
		
		// print out bytes received
		ipString = ipAddressToString(&server);
		printf("Server with ip: %s and port %d said it received %s\n", ipString, ntohs(server.sin6_port), buffer);
	}
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