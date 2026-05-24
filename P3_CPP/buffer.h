#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"

class FIFOBuffer
{
private:
    uint32_t _bufferSize;
    PDU* _pduBuffer;
    void shiftBuffer();
public:
    FIFOBuffer(uint32_t size);
    ~FIFOBuffer();
    inline void add(PDU* pdu) { memcpy(_pduBuffer, pdu, sizeof(PDU)); }
    inline PDU get(uint32_t i) { return *(_pduBuffer + (sizeof(PDU) * i)); }
};

#endif