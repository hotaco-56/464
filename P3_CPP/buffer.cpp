#include "buffer.h"

FIFOBuffer::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    _buffer = (PDU_T*)malloc(size * sizeof(PDU_T));

    for (uint32_t i = 0; i < _bufferSize; i++) {
        _buffer[i].acked = false;
        _buffer[i].valid = false;
    }
}

FIFOBuffer::~FIFOBuffer()
{
    free(_buffer);
}

bool FIFOBuffer::add(PDU_T data)
{
    uint32_t index = ntohl(data.seqNum) % _bufferSize;
    if (_buffer[index].valid == true) 
        return false;
    _buffer[index] = data;
    return true;
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
