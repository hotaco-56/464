#include "pdu.h"

PDU::PDU() : payloadLen(0)
{
}

PDU::PDU(unsigned char* pdu, uint16_t pduLen) : payloadLen(pduLen - HEADER_SIZE)
{
    memcpy(&(header.seqNum), pdu, sizeof(header.seqNum));
    pdu += sizeof(header.seqNum);
    memcpy(&(header.chksum), pdu, sizeof(header.chksum));
    pdu += sizeof(header.chksum);
    memcpy(&(header.flag), pdu, sizeof(header.flag));
    pdu += sizeof(header.flag);
    memcpy(payload, pdu, pduLen - HEADER_SIZE);
}

PDU::~PDU()
{
}

void PDU::calcChksum(uint16_t pduLen)
{
    unsigned short data[HEADER_SIZE + MAX_PAYLOAD_SIZE];
    headerCpy((unsigned char*) data);
    header.chksum = (uint16_t)in_cksum(data, pduLen);
}

/*
    copy header to dest
*/
void PDU::headerCpy(unsigned char* dest)
{
    memcpy(dest, &(header.seqNum), sizeof(header.seqNum));
    dest += sizeof(header.seqNum);
    memcpy(dest, &(header.chksum), sizeof(header.chksum));
    dest += sizeof(header.chksum);
    memcpy(dest, &(header.flag), sizeof(header.flag));
    dest += sizeof(header.flag);

}