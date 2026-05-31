#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"
#include "vars.h"

class FIFOBuffer
{
private:
    uint32_t _bufferSize;
    PDU_T* _buffer;
    void shiftBuffer();
public:
    FIFOBuffer(uint32_t size);
    ~FIFOBuffer();
    bool add(PDU_T data);
    void clear(uint32_t index);
    inline PDU_T* get(uint32_t i) { return &_buffer[i % _bufferSize]; }
};

#endif