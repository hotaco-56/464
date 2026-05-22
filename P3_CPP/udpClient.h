#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <string>

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pdu.h"

typedef struct {
    char* fromFilename;
    char* toFilename;
    uint8_t windowSize;
    uint8_t bufferSize;
    float errorRate;
    char* remoteMachine;
    int remotePort;
} UDPClientArgs;

class UDPClient
{
private:
    UDPClientArgs args;
    int socketNum;
    struct sockaddr_in6 server;
    PDUs pdu;

    const size_t serverAddrLen = sizeof(struct sockaddr_in6);

    std::ifstream openFromFile();
    void talkToServer();
    int readFromStdin(char * buffer);
public:
    UDPClient(UDPClientArgs& args);
    ~UDPClient();

    void run(void);
};



#endif