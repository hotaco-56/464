#include "udpServer.h"

void setupSignalHandlers();

UDPServer::~UDPServer()
{
	__PRINTF_DBG("Server deconstructor called\n");
    close(socketNum);
}

void UDPServer::run()
{
	setupSignalHandlers();
	setupPollSet();
	addToPollSet(this->socketNum);

	while(1) {
		int pollSocket = pollCall(-1);
		if (pollSocket == this->socketNum) {
			pid_t forkVal = fork();

			if (forkVal < 0) {
				perror("fork failed");
				this->~UDPServer();
				exit(1);
			}
			else if (forkVal != 0) {
				__PRINTF_DBG("Child created with pid: %d\n", forkVal);
			}
			else if (forkVal == 0) {
				processClient();
				this->~UDPServer();
				return;
			}
		}

	}
}

void UDPServer::processClient()
{
	int dataLen = 0; 
	char buffer[MAXBUF + 1];	  
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);	
	
	buffer[0] = '\0';
	while (buffer[0] != '.')
	{
		dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &client, &clientAddrLen);
	
		printf("Received message from client with ");
		printIPInfo(&client);
		printf(" Len: %d \'%s\'\n", dataLen, buffer);

		// just for fun send back to client number of bytes received
		sprintf(buffer, "bytes: %d", dataLen);
		safeSendto(socketNum, buffer, strlen(buffer)+1, 0, (struct sockaddr *) & client, clientAddrLen);

	}
}

void sigchldHandler(int signo)
{
	pid_t childPID;
    while ((childPID = waitpid(-1, nullptr, WNOHANG)) > 0) {
		  printf("Child %d terminated\n", childPID);
    }
}

void setupSignalHandlers()
{
    struct sigaction sa;

    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(1);
    }
}