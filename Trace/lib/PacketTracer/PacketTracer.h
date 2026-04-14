#ifndef TRACE_H
#define TRACE_H

#include <pcap/pcap.h>
#include <stdio.h>
#include <netinet/ether.h>
#include <string.h>
#include <cstdlib>
#include "Checksum/checksum.h"

class PacketTracer
{
public:
    PacketTracer();
    void sniffPackets(const char* fname);
    void printEthHeader(void);
    void printIPHeader(void);
    void printARPHeader(void);
    void printTCPHeader(void);
    void printUDPHeader(void);
private:
    void processEthHeader(const u_char* data);
    void processIPHeader(const u_char* data);
    void processARPHeader(const u_char* data);
    void processUDPHeader(const u_char* data);
    void processTCPHeader(const u_char* data);

    uint16_t getEthType(void) {return ntohs( *(uint16_t*)(ethHeader.type) );};
    char* getEthDstMAC(void) {return ether_ntoa( (ether_addr*)(ethHeader.dstMac) ); };
    char* getEthSrcMAC(void) {return ether_ntoa( (ether_addr*)(ethHeader.srcMac) ); };

    uint16_t getIPPDULen(void) {return ntohs( *((uint16_t*)(ipHeader.totalLen)) );};
    uint16_t getIPHeaderLen(void) {return *(uint8_t*)(ipHeader.VER_IHL) & 0x0F;};
    uint8_t getIPTTL(void) {return *(uint8_t*)(ipHeader.ttl);};
    uint8_t getIPProtocol(void) {return *(uint8_t*)(ipHeader.protocol);};
    uint16_t getIPHeaderChecksum(void) {return *(uint16_t*)(ipHeader.headerChecksum);};

    uint16_t getARPOpcode(void) {return ntohs( *((uint16_t*)(arpHeader.opcode)) );};
    char* getARPSenderMAC(void) {return ether_ntoa((ether_addr*)arpHeader.senderMac); };
    char* getARPSenderIP(void) {return inet_ntoa( *(in_addr*)arpHeader.senderIP); };
    char* getARPTargetMAC(void) {return ether_ntoa((ether_addr*)arpHeader.targetMac); };
    char* getARPTargetIP(void) {return inet_ntoa( *(in_addr*)arpHeader.targetIP); };

    bool checkIPHeaderChecksum(const u_char* data);

    struct __attribute__((packed)) tcp_pseudo_hdr {
        uint32_t srcAddr;
        uint32_t dstAddr;
        uint8_t zeroes;
        uint8_t protocol;
        uint16_t tcpLen;
    };
    tcp_pseudo_hdr tcpPseudoHeader;

    struct eth_header {
        u_char dstMac[6];
        u_char srcMac[6];
        u_char type[2];
    };
    eth_header ethHeader; 

    struct ip_header {
        u_char VER_IHL[1];
        u_char totalLen[2];
        u_char ttl[1];
        u_char protocol[1];
        u_char headerChecksum[2];
        u_char srcAddr[4];
        u_char dstAddr[4];
    };
    ip_header ipHeader;

    struct arp_header {
        u_char opcode[2];
        u_char senderMac[6];
        u_char senderIP[4];
        u_char targetMac[6];
        u_char targetIP[4];
    };
    arp_header arpHeader;

    const u_char* ipHeaderEndAddr;
    const u_char* ethHeaderEndAddr;
};
    

#endif