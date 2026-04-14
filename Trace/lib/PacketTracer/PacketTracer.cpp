#include "PacketTracer.h"

#include <iostream>

uint16_t getEtherType(const u_char*  &data);
// uint16_t getIPType(const u_char* &data, tcp_pseudo_hdr* tcp_phdr);
void processARP(const u_char* &data);
void processICMP(const u_char* &data);
// void processTCP(const u_char* &data, tcp_pseudo_hdr* tcp_phdr);
void processUDP(const u_char* &data);
void print_src_port(uint16_t pn);
void print_dst_port(uint16_t pn);
void print_protocol(uint8_t pn);
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
            processARPHeader(ethHeaderEndAddr);
            printARPHeader();
            break;

        case ETH_P_IP:
            processIPHeader(ethHeaderEndAddr);
            printIPHeader();
            break;
        
        default:
            break;
        }
    }

    pcap_close(packet);
}

void PacketTracer::processEthHeader(const u_char* data)
{
    memcpy(ethHeader.dstMac, data, 6);
    memcpy(ethHeader.srcMac, data+6, 6);
    memcpy(ethHeader.type, data+12, 2);

    ethHeaderEndAddr = data + 14;
}

void PacketTracer::processIPHeader(const u_char* data)
{
    memcpy(ipHeader.VER_IHL, data, 1);
    memcpy(ipHeader.totalLen, data+2, 2);
    memcpy(ipHeader.ttl, data+8, 1);
    memcpy(ipHeader.protocol, data+9, 1);
    memcpy(ipHeader.headerChecksum, data+10, 2);
    memcpy(ipHeader.srcAddr, data+12, 4);
    memcpy(ipHeader.dstAddr, data+16, 4);

    ipHeaderEndAddr = data + 20;
}

void PacketTracer::processARPHeader(const u_char* data)
{
    memcpy(arpHeader.opcode, data+6, 2);
    memcpy(arpHeader.senderMac, data+8, 6);
    memcpy(arpHeader.senderIP, data+14, 4);
    memcpy(arpHeader.targetMac, data+18, 6);
    memcpy(arpHeader.targetIP, data+24, 4);
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
    std::cout << "\t\tProtocol: " << (uint16_t)getIPProtocol() << std::endl;
    
    std::cout << "\t\tChecksum: " 
        << (checkIPHeaderChecksum(ethHeaderEndAddr) ? "Correct ":"Incorrect ") 
        << std::hex
        << "(0x" << ntohs(getIPHeaderChecksum()) << ")\n"
        << std::dec;

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

bool PacketTracer::checkIPHeaderChecksum(const u_char* data)
{
    uint16_t received_chksum = getIPHeaderChecksum(); 
    *(uint16_t*)(data+10) = 0x0000; /*zero checksum*/
    uint16_t computed_chksum = in_cksum((unsigned short*)(data), getIPHeaderLen() * 4);
    
    return (received_chksum == computed_chksum);
}

void processARP(const u_char* &data)
{
    printf("\n\tARP Header\n");

    data += 6;
    u_char opcode[2];
    memcpy(opcode, data, 2);
    data += 2;

    //Opcode
    uint16_t op = ntohs( *((uint16_t*)(opcode)) );
    if (op == 1)
        printf("\t\tOpcode: Request\n");
    else if (op == 2)
        printf("\t\tOpcode: Reply\n");

    //Sender MAC
    u_char sender_mac[6];
    memcpy(sender_mac, data, 6);
    data += 6;
    printf("\t\tSender MAC: %s\n", ether_ntoa((ether_addr*)sender_mac));

    //Sender IP
    u_char sender_ip[4];
    memcpy(sender_ip, data, 4);
    data += 4;
    printf("\t\tSender IP: %s\n", inet_ntoa( *(in_addr*)(sender_ip) ));

    //Target MAC
    u_char target_mac[6];
    memcpy(target_mac, data, 6);
    data += 6;
    printf("\t\tTarget MAC: %s\n", ether_ntoa((ether_addr*)target_mac));

    //Target IP
    u_char target_ip[4];
    memcpy(target_ip, data, 4);
    data += 4;
    printf("\t\tTarget IP: %s\n\n", inet_ntoa( *(in_addr*)(target_ip) ));
}


// void processTCP(const u_char* &data, tcp_pseudo_hdr* tcp_phdr)
// {
//     printf("\n\tTCP Header\n");
    
//     //Segment Length
//     printf("\t\tSegment Length: %u\n", tcp_phdr->tcp_len);

//     //Source Port
//     u_char src_port[2];
//     memcpy(src_port, data, 2);
//     print_src_port(ntohs( *(uint16_t*)(src_port)));

//     //Destination Port
//     u_char dest_port[2];
//     memcpy(dest_port, data+2, 2);
//     print_dst_port(ntohs( *(uint16_t*)(dest_port)));

//     //Sequence Number
//     u_char sequence_num[4];
//     memcpy(sequence_num, data+4, 4);
//     printf("\t\tSequence Number: %u\n", ntohl(*(uint32_t*)(sequence_num)) );

//     //Ack Number
//     u_char ack_num[4];
//     memcpy(ack_num, data+8, 4);
//     printf("\t\tACK Number: %u\n", ntohl(*(uint32_t*)(ack_num)) );
    
//     //Check Flags
//     u_char flags[1];
//     memcpy(flags, data+13, 1);
//     print_tcp_flags(*(uint8_t*)(flags));

//     //Window Size
//     u_char win_size[2];
//     memcpy(win_size, data+14,2);
//     std::cout << "\t\tWindow Size: " << ntohs( *(uint16_t*)(win_size) ) << std::endl;

//     //Checksum
//     u_char chksum[2];
//     mempcpy(chksum, data+16, 2);

//     uint16_t actual_chksum = ntohs(*(uint16_t*)(chksum));
//     *(uint16_t*)(data+16) = 0x0000; /*zero checksum*/

//     memcpy(tcp_phdr, data-13, 12);
//     uint16_t computed_chksum = ntohs(in_cksum((unsigned short*)(data-13), 12 + tcp_phdr->tcp_len));

//     if (actual_chksum == computed_chksum)
//         printf("\t\tChecksum: Correct (0x%04x)\n", actual_chksum);
//     else
//         printf("\t\tChecksum: Incorrect (0x%04x) (0x%04x)\n", actual_chksum, computed_chksum);

// }

void processUDP(const u_char* &data)
{
    printf("\n\tUDP Header\n");

    //Source Port
    u_char src_port[2];
    memcpy(src_port, data, 2);
    print_src_port(ntohs( *(uint16_t*)(src_port)));

    //Destination Port
    u_char dest_port[2];
    memcpy(dest_port, data+2, 2);
    print_dst_port(ntohs( *(uint16_t*)(dest_port)));
}

void print_src_port(uint16_t pn)
{
    if (pn == 80)
        std::cout << "\t\tSource Port:  HTTP\n";
    else if (pn == 53)
        std::cout << "\t\tSource Port:  DNS\n";
    else
        std::cout << "\t\tSource Port:  "<< pn << std::endl;
}

void print_dst_port(uint16_t pn)
{
    if (pn == 80)
        std::cout << "\t\tDest Port:  HTTP\n";
    else if (pn == 53)
        std::cout << "\t\tDest Port:  DNS\n";
    else
        std::cout << "\t\tDest Port:  "<< pn << std::endl;
}

void print_protocol(uint8_t pn)
{
    switch (pn)
    {
    case 1:
        std::cout << "\t\tProtocol: ICMP\n";
        return;
    case 6:
        std::cout << "\t\tProtocol: TCP\n";
        return;
    case 17:
        std::cout << "\t\tProtocol: UDP\n";
        return;
    default:
        return;
    }
}

void print_tcp_flags(uint8_t flag)
{
    std::cout << "\t\tSYN Flag: " << ((flag & 0x02) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tRST Flag: " << ((flag & 0x04) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tFIN Flag: " << ((flag & 0x01) ? "Yes" : "No") << std::endl;
    std::cout << "\t\tACK Flag: " << ((flag & 0x10) ? "Yes" : "No") << std::endl;
}