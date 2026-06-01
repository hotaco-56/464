#include "buffer.h"

Buffer::Buffer(uint32_t size)
{
    _bufferSize = size;
    _buffer = (PDU_T*)malloc(size * sizeof(PDU_T));

    for (uint32_t i = 0; i < _bufferSize; i++) {
        _buffer[i].acked = false;
        _buffer[i].valid = false;
    }
}

Buffer::Buffer() {}

void Buffer::init(uint32_t size)
{
    _bufferSize = size;
    _buffer = (PDU_T*)malloc(size * sizeof(PDU_T));

    for (uint32_t i = 0; i < _bufferSize; i++) {
        _buffer[i].acked = false;
        _buffer[i].valid = false;
    }
}

Buffer::~Buffer()
{
    free(_buffer);
}

bool Buffer::add(PDU_T data)
{
    uint32_t index = ntohl(data.seqNum) % _bufferSize;
    if (_buffer[index].valid == true) 
        return false;
    _buffer[index] = data;
    return true;
}

void Buffer::clear(uint32_t index)
{
    _buffer[index % _bufferSize].valid = false;
}

void Buffer::shiftBuffer()
{
    if (_bufferSize <= 1)
        return;

    for (uint32_t i = _bufferSize - 1; i > 0; --i) {
        _buffer[i] = _buffer[i - 1];
    }
}
