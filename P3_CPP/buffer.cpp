#include "buffer.h"

FIFOBuffer::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    _buffer = (PDU_T*)malloc(size * sizeof(PDU_T));
}

FIFOBuffer::~FIFOBuffer()
{
    free(_buffer);
}

void FIFOBuffer::add(PDU_T data)
{
    uint32_t index = ntohl(data.seqNum) % _bufferSize;
    if (_buffer[index].valid == true) 
        return;
    _buffer[index] = data;
}

void FIFOBuffer::clear(uint32_t index)
{
    _buffer[index % _bufferSize].valid = false;
}

void FIFOBuffer::shiftBuffer()
{
    if (_bufferSize <= 1)
        return;

    for (uint32_t i = _bufferSize - 1; i > 0; --i) {
        _buffer[i] = _buffer[i - 1];
    }
}
