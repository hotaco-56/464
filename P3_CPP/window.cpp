#include "window.h"

Window::Window(windowSize_t windowSize) : 
    _buffer(windowSize), 
    _size(windowSize), 
    _upper(windowSize)
{
}

Window::Window() : _buffer() {}

void Window::init(windowSize_t windowSize) 
{
    _buffer.init(windowSize);
    _size = windowSize;
    _upper = windowSize;
}

PDU_T Window::get(seqNum_t seqNum)
{
    return *_buffer.get(seqNum);
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

bool Window::contains(seqNum_t seqNum)
{
    bool containsPDU = false;
    PDU_T pdu = *_buffer.get(seqNum);
    containsPDU = (pdu.valid) && (ntohl(pdu.seqNum) == seqNum);

    __PRINTF_DBG("window %s contain %d, lowest: %d\n", containsPDU ? "does" : "doesn't", seqNum, _lower);

    return containsPDU;
}

void Window::clear(seqNum_t seqNum)
{
    _buffer.clear(seqNum);
}

void Window::ack(seqNum_t seqNum)
{
    while (_lower <= seqNum) {
        PDU_T* pdu = _buffer.get(_lower);
        _buffer.clear(_lower);
        __PRINTF_DBG("acking: %d, %d at buffPos %d\n", ntohl(pdu->seqNum), (uint8_t)pdu->acked, seqNum);
        pdu->acked = true;

        __PRINTF_DBG("acked: %d, %d\n", ntohl(pdu->seqNum), (uint8_t)pdu->acked);
        _lower++;
        _upper++;
    }

}

void Window::update(PDU_T pdu, uint16_t pduLen)
{
    pdu.pduLen = pduLen;
    if (_buffer.add(pdu))
        _current++;
    __PRINTF_DBG("current: %d\n", _current);
}