#include "pdu.h"

PDU::PDU()
{
}

PDU::PDU(unsigned char* pdu, uint16_t pduLen) : payloadLen(pduLen - HEADER_SIZE)
{
    memcpy(&header, pdu, HEADER_SIZE);
    memcpy(payload, pdu+HEADER_SIZE, payloadLen);
}

PDU::~PDU()
{
}

void PDU::calcChksum(uint16_t pduLen)
{
    unsigned short data[HEADER_SIZE + MAX_PAYLOAD_SIZE];
    headerCpy((unsigned char*) data);
    clearChksum();
    header.chksum = (uint16_t)in_cksum(data, pduLen);
}
