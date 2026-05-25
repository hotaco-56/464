#include "window.h"

Window::Window(uint32_t windowSize, uint32_t bufferSize) : _buffer(bufferSize), _size(windowSize)
{
}

Window::~Window()
{
}