#ifndef TRACE_H
#define TRACE_H

#include <pcap/pcap.h>
#include <stdio.h>
#include <netinet/ether.h>
#include <string.h>
#include <cstdlib>
#include "Checksum/checksum.h"
#include <iostream>
#include <iomanip>

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
    void printICMPHeader(void);
private:
    void processEthHeader(const u_char* data);
    void processIPHeader(const u_char* data);
    void processARPHeader(const u_char* data);
    void processUDPHeader(const u_char* data);
    void processTCPHeader(const u_char* data);
    void processICMPHeader(const u_char* data);

    uint16_t getEthType(void) {return ntohs( *(uint16_t*)(_ethHeader.type) );};
    char* getEthDstMAC(void) {return ether_ntoa( (ether_addr*)(_ethHeader.dstMac) ); };
    char* getEthSrcMAC(void) {return ether_ntoa( (ether_addr*)(_ethHeader.srcMac) ); };

    uint16_t getIPPDULen(void) {return ntohs( *((uint16_t*)(_ipHeader.totalLen)) );};
    uint16_t getIPHeaderLen(void) {return *(uint8_t*)(_ipHeader.VER_IHL) & 0x0F;};
    uint8_t getIPTTL(void) {return *(uint8_t*)(_ipHeader.ttl);};
    uint8_t getIPProtocol(void) {return *(uint8_t*)(_ipHeader.protocol);};
    uint16_t getIPHeaderChecksum(void) {return *(uint16_t*)(_ipHeader.headerChecksum);};
    char* getIPSrcAddr(void) {return inet_ntoa( *(in_addr*)(_ipHeader.srcAddr));};
    char* getIPDstAddr(void) {return inet_ntoa( *(in_addr*)(_ipHeader.dstAddr));};

    uint16_t getARPOpcode(void) {return ntohs( *((uint16_t*)(_arpHeader.opcode)) );};
    char* getARPSenderMAC(void) {return ether_ntoa((ether_addr*)_arpHeader.senderMac); };
    char* getARPSenderIP(void) {return inet_ntoa( *(in_addr*)_arpHeader.senderIP); };
    char* getARPTargetMAC(void) {return ether_ntoa((ether_addr*)_arpHeader.targetMac); };
    char* getARPTargetIP(void) {return inet_ntoa( *(in_addr*)_arpHeader.targetIP); };

    uint16_t getTCPLen(void) {return ntohs( *(uint16_t*)(_tcpPseudoHeader.tcpLen));};
    uint16_t getTCPSrcPort(void) {return ntohs(*(uint16_t*)(_tcpHeader.srcPort));};
    uint16_t getTCPDstPort(void) {return ntohs(*(uint16_t*)(_tcpHeader.dstPort));};
    uint32_t getTCPSeqNum(void) {return ntohl(*(uint32_t*)(_tcpHeader.seqNum));};
    uint32_t getTCPAckNum(void) {return ntohl(*(uint32_t*)(_tcpHeader.ackNum));};
    uint8_t getTCPFlags(void) {return *(uint8_t*)(_tcpHeader.flags);};
    uint16_t getTCPWinSize(void) {return ntohs(*(uint16_t*)(_tcpHeader.winSize));};
    uint16_t getTCPChecksum(void) {return *(uint16_t*)(_tcpHeader.checksum);};

    uint16_t getUDPSrcPort(void) {return ntohs(*(uint16_t*)(_udpHeader.srcPort));};
    uint16_t getUDPDstPort(void) {return ntohs(*(uint16_t*)(_udpHeader.dstPort));};

    uint8_t getICMPType(void) {return *(uint8_t*)(_icmpHeader.type);};

    bool cmpIPHeaderChecksum(const u_char* data);
    bool cmpTCPChecksum(const u_char* data);

    struct icmp_header {
        u_char type[1];
    };
    icmp_header _icmpHeader;

    struct  tcp_pseudo_hdr {
        u_char srcAddr[4];
        u_char dstAddr[4];
        u_char zeroes[1];
        u_char protocol[1];
        u_char tcpLen[2];
    };
    tcp_pseudo_hdr _tcpPseudoHeader;

    struct tcp_header {
        u_char srcPort[2];
        u_char dstPort[2];
        u_char seqNum[4];
        u_char ackNum[4];
        u_char flags[1];
        u_char winSize[2];
        u_char checksum[2];
    };
    tcp_header _tcpHeader;

    struct eth_header {
        u_char dstMac[6];
        u_char srcMac[6];
        u_char type[2];
    };
    eth_header _ethHeader; 

    struct ip_header {
        u_char VER_IHL[1];
        u_char totalLen[2];
        u_char ttl[1];
        u_char protocol[1];
        u_char headerChecksum[2];
        u_char srcAddr[4];
        u_char dstAddr[4];
    };
    ip_header _ipHeader;

    struct arp_header {
        u_char opcode[2];
        u_char senderMac[6];
        u_char senderIP[4];
        u_char targetMac[6];
        u_char targetIP[4];
    };
    arp_header _arpHeader;

    struct udp_header {
        u_char srcPort[2];
        u_char dstPort[2];
    };
    udp_header _udpHeader;

    const u_char* _ipHeaderEndAddr;
    const u_char* _ethHeaderEndAddr;
};

#endif