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

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"

#define MAXBUF 80

class UDPServer
{
private:
    const int clientAddrLen = sizeof(struct sockaddr_in6);

    float _errRate;
    int _portNumber;
    int _socketNum;

    struct sockaddr_in6 client;

    void recvFilenamePDU();
public:
    UDPServer(float errRate = 0.0f, int portNumber = 0) : 
        _errRate(errRate),
        _portNumber(portNumber),
        _socketNum(udpServerSetup(portNumber))
    {
    }
    ~UDPServer();
    void run();
    inline int getSocketNum(void) { return _socketNum; }
};


#endif