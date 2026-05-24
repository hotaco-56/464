#include "buffer.h"

FIFOBuffer::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    _pduBuffer = (PDU*)malloc(size * sizeof(PDU));
}

FIFOBuffer::~FIFOBuffer()
{
    free(_pduBuffer);
}

void FIFOBuffer::shiftBuffer()
{
    uint32_t pduSize = sizeof(PDU);
    PDU* firstPDUAddr = _pduBuffer + ((_bufferSize-1) * pduSize); // first is last

    for (PDU* currPDU = firstPDUAddr; currPDU > _pduBuffer; currPDU -= pduSize) {
        *currPDU = *(currPDU - pduSize); // copy assignment
    }
}
