#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"
#include "cpe464.h" // for sendErr_init()

#include "vars.h"
#include "window.h"

class UDPServer
{
private:
    const int _clientAddrLen = sizeof(struct sockaddr_in6);

    float _errRate;
    int _portNumber;
    int _socketNum;

    windowSize_t _windowSize = 0;
    buffSize_t _bufferSize = 0;
    char _toFilename[MAX_FILENAME_LEN + 1];
    seqNum_t _expectedSequenceNum = 0;

    struct sockaddr_in6 _client;

    bool _isChild = false;

    std::ofstream _toFile;

    void openToFile(char* toFilename);
    void setup();
    void recvPDU(); // returns pdu flag
    void parseFilenamePDU(PDU pdu);
    void sendFilenameAck();
    void sendFilenameErr();
    void sendRR();
    void runInternal();
public:
    UDPServer(float err, int port);
    ~UDPServer();
    void run();
    inline int getSocketNum(void) { return _socketNum; }
};


#endif