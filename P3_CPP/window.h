#ifndef WINDOW_H
#define WINDOW_H

#include <inttypes.h>
#include <string.h>

#include "buffer.h"

class Window
{
private:
    Buffer _buffer;
    uint32_t _lower = 1;
    uint32_t _current = 1;
    uint32_t _size;
    uint32_t _upper;

    void slide();
public:
    Window();
    Window(windowSize_t windowSize);
    ~Window();

    uint32_t getLowestUnacked() { return _lower; }
    void init(windowSize_t);
    PDU_T get(seqNum_t);
    void update(PDU_T pdu, uint16_t pduLen);
    void ack(seqNum_t seqNum);
    bool isAcked();
    bool contains(seqNum_t);
    void clear(seqNum_t);

    inline bool isClosed() const { return _current >= _upper; }
};

#endif