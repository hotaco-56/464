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
	char buffer[args.bufferSize+1];
	std::ifstream fromFile = openFromFile();

	// send setup packet
	pdu.setFlag(SETUP);
	pdu.setSequenceNum(0);
	pdu.calcChksum();
	safeSendto(socketNum, buffer, HEADER_SIZE, 0, (struct sockaddr*) &server, serverAddrLen);
}



std::ifstream UDPClient::openFromFile()
{
	std::ifstream fromFile(args.fromFilename);

	if (!fromFile) {
        printf("File %s not found\n", args.fromFilename);
		this->~UDPClient();
		exit(1);
    }
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