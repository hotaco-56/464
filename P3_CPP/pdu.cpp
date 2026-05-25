#include "pdu.h"

PDU::PDU()
{
}

PDU::PDU(unsigned char* pdu, uint16_t pduLen) : _payloadLen(pduLen - HEADER_SIZE)
{
    memcpy(&_pdu.seqNum, pdu, sizeof(seqNum_t));
    memcpy(&_pdu.chksum, pdu + sizeof(seqNum_t), sizeof(_pdu.chksum));
    memcpy(&_pdu.flag, pdu + sizeof(seqNum_t) + sizeof(_pdu.chksum), sizeof(_pdu.flag));
    memcpy(_pdu.payload, pdu+HEADER_SIZE, _payloadLen);
}

PDU::~PDU()
{
}
