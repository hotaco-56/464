#include "udpServer.h"

int checkArgs(int argc, char *argv[]);

int main ( int argc, char *argv[] )
{ 
	__PRINTF_DBG("DEBUG_MODE\n");
	int portNumber = 0;
	portNumber = checkArgs(argc, argv);
		
    UDPServer server(atof(argv[1]), portNumber);

	server.run();
	
	return 0;
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;
	
	if (argc > 4 || argc < 2)
	{
		fprintf(stderr, "Usage %s error-rate [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 3)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}
