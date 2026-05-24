#include "pdu.h"

PDU::PDU()
{
}

PDU::PDU(unsigned char* pdu, uint16_t pduLen) : _payloadLen(pduLen - HEADER_SIZE)
{
    memcpy(&_header, pdu, HEADER_SIZE);
    memcpy(_payload, pdu+HEADER_SIZE, _payloadLen);
}

PDU::~PDU()
{
}
