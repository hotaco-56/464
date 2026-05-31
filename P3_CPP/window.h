#ifndef WINDOW_H
#define WINDOW_H

#include <inttypes.h>
#include <string.h>

#include "buffer.h"

class Window
{
private:
    FIFOBuffer _buffer;
    uint32_t _lower = 1;
    uint32_t _current = 1;
    const uint32_t _size;
    uint32_t _upper;

    void slide();
    uint32_t getLowestUnacked();
public:
    Window(windowSize_t windowSize);
    ~Window();

    PDU_T get(uint32_t);
    void update(PDU_T pdu);
    void ack(seqNum_t seqNum);
    bool isAcked();

    inline bool isClosed() const { return _current >= _upper; }
};

#endif