#include "buffer.h"

FIFOBuffer::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    _pduBuffer =  new PDU[size];
}

FIFOBuffer::~FIFOBuffer()
{
    delete[] _pduBuffer;
}

void FIFOBuffer::add(PDU pdu)
{
    shiftBuffer();
    _pduBuffer[0] = pdu;
}

void FIFOBuffer::shiftBuffer()
{
    if (_bufferSize <= 1)
        return;

    for (uint32_t i = _bufferSize - 1; i > 0; --i) {
        _pduBuffer[i] = _pduBuffer[i - 1];
    }
}
