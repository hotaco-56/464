#include "buffer.h"

template <typename Datatype>
FIFOBuffer<Datatype>::FIFOBuffer(uint32_t size)
{
    _bufferSize = size;
    _buffer = new Datatype[size];
}

template <typename Datatype>
FIFOBuffer<Datatype>::~FIFOBuffer()
{
    delete[] _buffer;
}

template <typename Datatype>
void FIFOBuffer<Datatype>::add(Datatype data)
{
    shiftBuffer();
    _buffer[0] = data;
}

template <typename Datatype>
void FIFOBuffer<Datatype>::shiftBuffer()
{
    if (_bufferSize <= 1)
        return;

    for (uint32_t i = _bufferSize - 1; i > 0; --i) {
        _buffer[i] = _buffer[i - 1];
    }
}

template class FIFOBuffer<PDU>;