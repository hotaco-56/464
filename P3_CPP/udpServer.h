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
    /* data */
    float errRate;
    int portNumber;
    int socketNum;
    void processClient();
    void recvFilenamePDU();
public:
    UDPServer(float errRate = 0.0f, int portNumber = 0) : 
        errRate(errRate),
        portNumber(portNumber),
        socketNum(udpServerSetup(portNumber))
    {
    }
    ~UDPServer();
    void run();
    inline int getSocketNum(void) { return socketNum; }
};


#endif