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
#include "window.h"

#include "vars.h"

typedef struct {
    char* fromFilename;
    char* toFilename;
    uint32_t windowSize;
    uint16_t bufferSize;
    float errorRate;
    char* remoteMachine;
    int remotePort;
} UDPClientArgs;

class UDPClient
{
private:
    UDPClientArgs _args;
    int _serverAddrLen = (int)sizeof(struct sockaddr_in6);
    int _socketNum;
    struct sockaddr_in6 _server;

    Window _window;

    std::ifstream openFromFile();
    bool setup();
    void sendFilenamePDU(void);
    void sendDataPDU(void);
    uint8_t recvPDU();
public:
    UDPClient(UDPClientArgs& args);
    ~UDPClient();

    void run(void);
};



#endif