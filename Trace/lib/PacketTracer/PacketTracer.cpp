#include "PacketTracer.h"

#include <iostream>

PacketTracer::PacketTracer() {}

uint8_t getEtherType(const u_char* data, struct pcap_pkthdr* header)
{
    u_char dest_mac[6];
    memcpy(dest_mac, data, 6);
    data += 6;
    printf("\t\tDest MAC: %s\n", ether_ntoa((ether_addr*)dest_mac));

    u_char src_mac[6];
    memcpy(src_mac, data, 6);
    data += 6;
    printf("\t\tSource MAC: %s\n", ether_ntoa((ether_addr*)src_mac));

    

    return 0;
}

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

        if (pcap_datalink(packet) == DLT_EN10MB) {
            printf("\n\tEthernet Header\n");

            getEtherType(data, header);
        }
    }

    pcap_close(packet);
}

