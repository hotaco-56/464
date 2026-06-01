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
#include <memory>

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pdu.h"
#include "pollLib.h"
#include "window.h"
#include "cpe464.h" // for sendErr_init()

#include "vars.h"

typedef struct {
    char* fromFilename;
    char* toFilename;
    windowSize_t windowSize;
    buffSize_t bufferSize;
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
    seqNum_t _pduSeqNum = 0;
	uint8_t _timeoutcount = 0;

    std::ifstream _fromFile;
    seqNum_t lastDataPDUSeqNum = 0;
    void openFromFile();
    void teardown();
    bool setup();
    void sendFinPDU();
    void sendTeardownPDU();
    void sendFilenamePDU();
    std::streamsize sendDataPDU();
    void sendDataPDU(seqNum_t);
    uint8_t recvPDU();
    void resendLowestPDU();
public:
    UDPClient(UDPClientArgs& args);
    ~UDPClient();

    void run(void);
};



#endif