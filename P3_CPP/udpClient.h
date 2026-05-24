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
#include "pollLib.h"

#include "vars.h"

typedef struct {
    char* fromFilename;
    char* toFilename;
    uint16_t windowSize;
    uint16_t bufferSize;
    float errorRate;
    char* remoteMachine;
    int remotePort;
} UDPClientArgs;

class UDPClient
{
private:
    UDPClientArgs args;

    const size_t serverAddrLen = sizeof(struct sockaddr_in6);

    int _socketNum;
    struct sockaddr_in6 _server;

    uint8_t _retransmitCount = 0;

    std::ifstream openFromFile();
    int readFromStdin(char * buffer);
    void sendFilenamePDU(void);
    void sendDataPDU(void);
    void retransmitCallback(void(*func)());
public:
    UDPClient(UDPClientArgs& args);
    ~UDPClient();

    void run(void);
};



#endif