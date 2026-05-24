#ifndef WINDOW_H
#define WINDOW_H

#include <inttypes.h>
#include <string.h>

#include "buffer.h"

class Window
{
private:
    uint32_t _lower;
    uint32_t _current;
    uint32_t _upper;

    FIFOBuffer _buffer;

public:
    Window(uint32_t bufferSize);
    ~Window();

    void update();
};

#endif