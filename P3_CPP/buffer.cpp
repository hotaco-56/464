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
}

void FIFOBuffer::add(PDU pdu)
{
    
}