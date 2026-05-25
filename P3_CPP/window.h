#ifndef WINDOW_H
#define WINDOW_H

#include <inttypes.h>
#include <string.h>

#include "buffer.h"

class Window
{
private:
    FIFOBuffer _buffer;
    uint32_t _lower = 0;
    uint32_t _current = 0;
    uint32_t _size;
    uint32_t _upper;

public:
    Window(uint32_t windowSize, uint32_t bufferSize);
    ~Window();

    void get(uint32_t);
    void update(PDU pdu);

    inline bool closed() const { return _current == _upper; }
};

#endif