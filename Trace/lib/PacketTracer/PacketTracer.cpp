#include "PacketTracer.h"

std::string portToAscii(uint16_t pn);
std::string protocolToAscii(uint16_t pn);
void print_tcp_flags(uint8_t flag);

PacketTracer::PacketTracer() {}

void PacketTracer::sniffPackets(const char* fname)
{
    char errbuff[PCAP_ERRBUF_SIZE];
    pcap_t* packet;
    packet = pcap_open_offline(fname, errbuff);

    if (packet == NULL) {
        std::cout << errbuff << std::endl;
        return;
    }

    const u_char* data;
    struct pcap_pkthdr* header;

    uint32_t packetCount = 0;
    while (pcap_next_ex(packet, &header, &data) != PCAP_ERROR_BREAK) {
        printf("\nPacket number: %d  Packet Len: %u\n",++packetCount,header->len);

        if (pcap_datalink(packet) != DLT_EN10MB)
            return;

        processEthHeader(data);
        printEthHeader();

        switch (getEthType())
        {
        case ETH_P_ARP:
            processARPHeader(_ethHeaderEndAddr);
            printARPHeader();
            break;

        case ETH_P_IP:
            processIPHeader(_ethHeaderEndAddr);
            printIPHeader();

            switch (getIPProtocol())
            {
            case 1:
                processICMPHeader(_ipHeaderEndAddr);
                printICMPHeader();
                break;
            case 6:
                processTCPHeader(_ipHeaderEndAddr);
                printTCPHeader();
                break;
            case 17:
                processUDPHeader(_ipHeaderEndAddr);
                printUDPHeader();
                break;
            
            default:
                break;
            }

            break;
        
        default:
            break;
        }
    }

    pcap_close(packet);
}

void PacketTracer::processEthHeader(const u_char* data)
{
    memcpy(_ethHeader.dstMac, data, 6);
    memcpy(_ethHeader.srcMac, data+6, 6);
    memcpy(_ethHeader.type, data+12, 2);

    _ethHeaderEndAddr = data + 14;
}

void PacketTracer::processIPHeader(const u_char* data)
{
    memcpy(_ipHeader.VER_IHL, data, 1);
    memcpy(_ipHeader.totalLen, data+2, 2);
    memcpy(_ipHeader.ttl, data+8, 1);
    memcpy(_ipHeader.protocol, data+9, 1);
    memcpy(_ipHeader.headerChecksum, data+10, 2);
    memcpy(_ipHeader.srcAddr, data+12, 4);
    memcpy(_ipHeader.dstAddr, data+16, 4);

    _ipHeaderEndAddr = data + 20 + ((getIPHeaderLen() - 5) * 4);
}

void PacketTracer::processARPHeader(const u_char* data)
{
    memcpy(_arpHeader.opcode, data+6, 2);
    memcpy(_arpHeader.senderMac, data+8, 6);
    memcpy(_arpHeader.senderIP, data+14, 4);
    memcpy(_arpHeader.targetMac, data+18, 6);
    memcpy(_arpHeader.targetIP, data+24, 4);
}

void PacketTracer::processTCPHeader(const u_char* data)
{
    memcpy(_tcpHeader.srcPort, data, sizeof(_tcpHeader.srcPort));
    memcpy(_tcpHeader.dstPort, data+2, 2);
    memcpy(_tcpHeader.seqNum, data+4, 4);
    memcpy(_tcpHeader.ackNum, data+8, 4);
    memcpy(_tcpHeader.flags, data+13, 1);
    memcpy(_tcpHeader.winSize, data+14, 2);
    memcpy(_tcpHeader.checksum, data+16, 2);

    memcpy(_tcpPseudoHeader.srcAddr, _ipHeader.srcAddr, sizeof(_tcpPseudoHeader.srcAddr));
    memcpy(_tcpPseudoHeader.dstAddr, _ipHeader.dstAddr, sizeof(_tcpPseudoHeader.dstAddr));
    memset(_tcpPseudoHeader.zeroes, 0x00, sizeof(_tcpPseudoHeader.zeroes));
    memcpy(_tcpPseudoHeader.protocol, _ipHeader.protocol, sizeof(_tcpPseudoHeader.protocol));

    uint16_t tcp_len = htons(getIPPDULen() - (getIPHeaderLen() * 4));
    memcpy(_tcpPseudoHeader.tcpLen, &tcp_len, 2);
}

void PacketTracer::processUDPHeader(const u_char* data)
{
    memcpy(_udpHeader.srcPort, data, 2);
    memcpy(_udpHeader.dstPort, data+2, 2);
}

void PacketTracer::processICMPHeader(const u_char* data)
{
    memcpy(_icmpHeader.type, data, 1);
}

void PacketTracer::printUDPHeader()
{
    std::cout << "\n\tUDP Header\n";
    std::cout << "\t\tSource Port:  " << portToAscii(getUDPSrcPort()) << std::endl;
    std::cout << "\t\tDest Port:  " << portToAscii(getUDPDstPort()) << std::endl;
}

void PacketTracer::printEthHeader()
{
    std::cout << "\n\tEthernet Header\n";
    std::cout << "\t\tDest MAC: " << getEthDstMAC() << std::endl;
    std::cout << "\t\tSource MAC: " << getEthSrcMAC() << std::endl;

    if (getEthType() == ETH_P_IP)
        std::cout << "\t\tType: IP\n";
    else if (getEthType() == ETH_P_ARP)
        std::cout << "\t\tType: ARP\n";
}

void PacketTracer::printIPHeader()
{
    std::cout << "\n\tIP Header\n";
    std::cout << "\t\tIP PDU Len: " << getIPPDULen() << std::endl;
    std::cout << "\t\tHeader Len (bytes): " << getIPHeaderLen() * 4 << std::endl;
    std::cout << "\t\tTTL: " << (uint16_t)getIPTTL() << std::endl;
    std::cout << "\t\tProtocol: " << protocolToAscii((uint16_t)getIPProtocol()) << std::endl;
    std::cout << "\t\tChecksum: " << (cmpIPHeaderChecksum(_ethHeaderEndAddr) ? "Correct ":"Incorrect ");
    printf("(0x%04x)\n", ntohs(getIPHeaderChecksum()));
    std::cout << "\t\tSender IP: " << getIPSrcAddr() << std::endl;
    std::cout << "\t\tDest IP: " << getIPDstAddr() << std::endl;

}

void PacketTracer::printARPHeader()
{
    std::cout << "\n\tARP header\n";
    std::cout << "\t\tOpcode: " << (getARPOpcode() == 1 ? "Request":"Reply")  << std::endl;
    std::cout << "\t\tSender MAC: " << getARPSenderMAC() << std::endl;
    std::cout << "\t\tSender IP: " << getARPSenderIP() << std::endl;
    std::cout << "\t\tTarget MAC: " << getARPTargetMAC() << std::endl;
    std::cout << "\t\tTarget IP: " << getARPTargetIP() << std::endl << std::endl;
}

void PacketTracer::printTCPHeader()
{
    std::cout << "\n\tTCP Header\n";
    std::cout << "\t\tSegment Length: " << getTCPLen() << std::endl;
    std::cout << "\t\tSource Port:  " << portToAscii(getTCPSrcPort()) << std::endl;
    std::cout << "\t\tDest Port:  " << portToAscii(getTCPDstPort()) << std::endl;
    std::cout << "\t\tSequence Number: " << getTCPSeqNum() << std::endl;
    std::cout << "\t\tACK Number: " << getTCPAckNum() << std::endl;
    print_tcp_flags(getTCPFlags());
    std::cout << "\t\tWindow Size: " << getTCPWinSize() << std::endl;

    std::cout << "\t\tChecksum: " << (cmpTCPChecksum(_ipHeaderEndAddr) ? "Correct ":"Incorrect ");
    printf("(0x%04x)\n", ntohs(getTCPChecksum()));
}

void PacketTracer::printICMPHeader()
{
    std::cout << "\n\tICMP Header\n";
    std::string type = "";
    if (getICMPType() == 0)
        type = "Reply";
    else if (getICMPType() == 8)
        type = "Request";
    else
        type = std::to_string(getICMPType());

    std::cout << "\t\tType: " << type << std::endl;
}

bool PacketTracer::cmpTCPChecksum(const u_char* data)
{
    uint8_t* tcp_phdrStart = (uint8_t*)data - 12;
    memcpy(tcp_phdrStart, _tcpPseudoHeader.srcAddr, 4);
    memcpy(tcp_phdrStart+4, _tcpPseudoHeader.dstAddr, 4);
    memcpy(tcp_phdrStart+8, _tcpPseudoHeader.zeroes, 1);
    memcpy(tcp_phdrStart+9, _tcpPseudoHeader.protocol, 1);
    memcpy(tcp_phdrStart+10, _tcpPseudoHeader.tcpLen, 2);
    uint16_t received_chksum = getTCPChecksum();
    *(uint16_t*)(data+16) = 0x0000; /*zero checksum*/
    uint16_t computed_chksum = in_cksum((unsigned short*)(tcp_phdrStart), 12 + getTCPLen());

    return (received_chksum == computed_chksum);
}

bool PacketTracer::cmpIPHeaderChecksum(const u_char* data)
{
    uint16_t received_chksum = getIPHeaderChecksum(); 
    *(uint16_t*)(data+10) = 0x0000; /*zero checksum*/
    uint16_t computed_chksum = in_cksum((unsigned short*)(data), getIPHeaderLen() * 4);
    
    return (received_chksum == computed_chksum);
}

std::string portToAscii(uint16_t pn)
{
    switch (pn)
    {
    case 80:
        return "HTTP";
    case 53:
        return "DNS";
    default:
        return std::to_string(pn);
    }
}

std::string protocolToAscii(uint16_t pn)
{
    switch (pn)
    {
    case 1:
        return "ICMP";
    case 6:
        return "TCP";
    case 17:
        return "UDP";
    default:
        return "Unknown";
    }
}

void print_tcp_flags(uint8_t flag)
{
    std::cout << "\t\tSYN Flag: " << ((flag & 0x02) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tRST Flag: " << ((flag & 0x04) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tFIN Flag: " << ((flag & 0x01) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tACK Flag: " << ((flag & 0x10) ? "Yes" : "No") << std::endl;
}