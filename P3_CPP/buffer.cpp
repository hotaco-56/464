#include "buffer.h"

FIFOBuffer::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    pduBuffer = (PDU*)malloc(size * sizeof(PDU));
}

FIFOBuffer::~FIFOBuffer()
{
    free(pduBuffer);
}

void FIFOBuffer::shiftBuffer()
{
    uint32_t pduSize = sizeof(PDU);
    PDU* firstPDUAddr = pduBuffer + (_bufferSize * pduSize);

    for (PDU* currPDU = firstPDUAddr; currPDU > pduBuffer; currPDU -= pduSize) {
        memcpy(currPDU, currPDU - pduSize, pduSize);
    }

    // for (uint32_t i = _bufferSize; i > 0; i--) {
    //     PDU* currPDU = firstPDUAddr - (i * pduSize);
    //     PDU* prevPDU = currPDU - pduSize;
    //     memcpy(currPDU, prevPDU, pduSize);
    // }
}
