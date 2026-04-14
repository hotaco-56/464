#ifndef TRACE_H
#define TRACE_H

#include <pcap/pcap.h>
#include <stdio.h>
#include <netinet/ether.h>
#include <string.h>
#include <cstdlib>
#include "Checksum/checksum.h"
#include <iostream>

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
    char* getIPSrcAddr(void) {return inet_ntoa( *(in_addr*)(ipHeader.srcAddr));};
    char* getIPDstAddr(void) {return inet_ntoa( *(in_addr*)(ipHeader.dstAddr));};

    uint16_t getARPOpcode(void) {return ntohs( *((uint16_t*)(arpHeader.opcode)) );};
    char* getARPSenderMAC(void) {return ether_ntoa((ether_addr*)arpHeader.senderMac); };
    char* getARPSenderIP(void) {return inet_ntoa( *(in_addr*)arpHeader.senderIP); };
    char* getARPTargetMAC(void) {return ether_ntoa((ether_addr*)arpHeader.targetMac); };
    char* getARPTargetIP(void) {return inet_ntoa( *(in_addr*)arpHeader.targetIP); };

    uint16_t getTCPLen(void) {return ntohs( *(uint16_t*)(tcpPseudoHeader.tcpLen));};
    uint16_t getTCPSrcPort(void) {return ntohs(*(uint16_t*)(tcpHeader.srcPort));};
    uint16_t getTCPDstPort(void) {return ntohs(*(uint16_t*)(tcpHeader.dstPort));};
    uint32_t getTCPSeqNum(void) {return ntohl(*(uint32_t*)(tcpHeader.seqNum));};
    uint32_t getTCPAckNum(void) {return ntohl(*(uint32_t*)(tcpHeader.ackNum));};
    uint8_t getTCPFlags(void) {return *(uint8_t*)(tcpHeader.flags);};
    uint16_t getTCPWinSize(void) {return ntohs(*(uint16_t*)(tcpHeader.winSize));};
    uint16_t getTCPChecksum(void) {return *(uint16_t*)(tcpHeader.checksum);};

    uint16_t getUDPSrcPort(void) {return ntohs(*(uint16_t*)(udpHeader.srcPort));};
    uint16_t getUDPDstPort(void) {return ntohs(*(uint16_t*)(udpHeader.dstPort));};

    bool cmpIPHeaderChecksum(const u_char* data);
    bool cmpTCPChecksum(const u_char* data);

    struct  tcp_pseudo_hdr {
        u_char srcAddr[4];
        u_char dstAddr[4];
        u_char zeroes[1];
        u_char protocol[1];
        u_char tcpLen[2];
    };
    tcp_pseudo_hdr tcpPseudoHeader;

    struct tcp_header {
        u_char srcPort[2];
        u_char dstPort[2];
        u_char seqNum[4];
        u_char ackNum[4];
        u_char flags[1];
        u_char winSize[2];
        u_char checksum[2];
    };
    tcp_header tcpHeader;

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

    struct udp_header {
        u_char srcPort[2];
        u_char dstPort[2];
    };
    udp_header udpHeader;

    const u_char* ipHeaderEndAddr;
    const u_char* ethHeaderEndAddr;
};
    

#endif