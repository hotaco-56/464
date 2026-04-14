#include "PacketTracer.h"

#include <iostream>

uint16_t getEtherType(const u_char*  &data);
void processARP(const u_char* &data);

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
        printf("\nPacket Number: %d  Packet Len: %u\n",++packetCount,header->len);

        if (pcap_datalink(packet) != DLT_EN10MB)
            return;

        switch (getEtherType(data))
        {
        case ETH_P_ARP:
            printf("\t\tType: ARP\n");
            processARP(data);
            break;

        case ETH_P_IP:
            break;
        
        default:
            break;
        }
    }

    pcap_close(packet);
}

uint16_t getEtherType(const u_char* &data)
{
    printf("\n\tEthernet Header\n");

    u_char dest_mac[6];
    memcpy(dest_mac, data, 6);
    data += 6;
    printf("\t\tDest MAC: %s\n", ether_ntoa((ether_addr*)dest_mac));

    u_char src_mac[6];
    memcpy(src_mac, data, 6);
    data += 6;
    printf("\t\tSource MAC: %s\n", ether_ntoa((ether_addr*)src_mac));

    u_char type[2];
    memcpy(type, data, 2);
    data += 2;
    return ntohs( *((uint16_t*)(type)) );
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