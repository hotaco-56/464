#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>

#include "pdu.h"

class FIFOBuffer
{
private:
    uint32_t _bufferSize;
    PDU* pduBuffer;
    void shiftBuffer();
public:
    FIFOBuffer(uint32_t size);
    ~FIFOBuffer();
    inline void add(PDU* pdu) { memcpy(pduBuffer, pdu, sizeof(PDU)); }
};

#endif