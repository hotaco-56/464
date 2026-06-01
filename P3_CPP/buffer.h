#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"
#include "vars.h"

class Buffer
{
private:
    uint32_t _bufferSize;
    PDU_T* _buffer;
    void shiftBuffer();
public:
    Buffer(uint32_t size);
    Buffer();
    void init(uint32_t size);
    ~Buffer();
    bool add(PDU_T data);
    void clear(uint32_t index);
    inline PDU_T* get(uint32_t i) { return &_buffer[i % _bufferSize]; }
};

#endif