#ifndef TRACE_H
#define TRACE_H

#include <pcap/pcap.h>
#include <stdio.h>
#include <netinet/ether.h>
#include <string.h>
#include <cstdlib>

class PacketTracer
{
public:
    PacketTracer();
    void sniffPackets(const char* fname);
private:
    
};

#endif