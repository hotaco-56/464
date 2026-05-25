#include "window.h"

Window::Window(windowSize_t windowSize) : 
    _buffer(windowSize), 
    _size(windowSize), 
    _upper(windowSize)
{
}

Window::~Window()
{
}

void Window::get(uint32_t i)
{
    PDU pdu = _buffer.get(i);
	__PRINTF_DBG("FILENAME PDU:\n\tflag: %d\n\tseqNum: %u\n\tchksum: %d\n", pdu.getFlag(), ntohl(pdu.getSeqNum()), pdu.getChksum());
}

void Window::update(PDU pdu)
{
    _buffer.add(pdu);
    _current++;
}