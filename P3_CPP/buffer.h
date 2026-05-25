#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"
#include "vars.h"

class FIFOBuffer
{
private:
    uint32_t _bufferSize;
    PDU* _pduBuffer;
    void shiftBuffer();
public:
    FIFOBuffer(uint32_t size);
    ~FIFOBuffer();
    void add(PDU pdu); 
    inline PDU get(uint32_t i) { return _pduBuffer[i]; }
};

#endif