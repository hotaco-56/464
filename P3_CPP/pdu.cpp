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

PDU& PDU::operator=(const PDU& other)
{
    this->_header = other._header;
    this->_payloadLen = other._payloadLen;
    memcpy(this->_payload, other._payload, other._payloadLen);
    memcpy(this->_pdu, other._pdu, other.getPDULen());

    return *this;
}