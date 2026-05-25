#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"
#include "vars.h"

template <typename Datatype>
class FIFOBuffer
{
private:
    uint32_t _bufferSize;
    Datatype* _buffer;
    void shiftBuffer();
public:
    FIFOBuffer(uint32_t size);
    ~FIFOBuffer();
    void add(Datatype data);
    inline Datatype get(uint32_t i) { return _buffer[i]; }
};

#endif