#include "window.h"

Window::Window(windowSize_t windowSize) : 
    _buffer(windowSize), 
    _size(windowSize), 
    _upper(windowSize + 1)
{
}

Window::~Window()
{
}

void Window::slide()
{
    while(1) {
        PDU_T* pdu = _buffer.get(_lower);

        if (pdu->valid == false) return;
        if (ntohl(pdu->seqNum) != _lower) return;
        if (pdu->acked == false) return;

        _buffer.clear(_lower);
        _lower++;
        _upper++;
    }
}

bool Window::isAcked()
{
    return _lower == _current;
}

void Window::ack(seqNum_t seqNum)
{
    PDU_T* pdu = _buffer.get(seqNum);

    if (pdu->valid == false)
        return;

    if (ntohl(pdu->seqNum) != seqNum) {
        __PRINTF_DBG("invalid\n");
        return;
    }
    __PRINTF_DBG("acking: %d, %d at buffPos %d\n", ntohl(pdu->seqNum), (uint8_t)pdu->acked, seqNum);
    pdu->acked = true;

    __PRINTF_DBG("acked: %d, %d\n", ntohl(pdu->seqNum), (uint8_t)pdu->acked);

    slide();
}

void Window::update(PDU_T pdu)
{
    if (_buffer.add(pdu))
        _current++;
    __PRINTF_DBG("current: %d\n", _current);
}