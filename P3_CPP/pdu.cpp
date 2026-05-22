#include "pdu.h"

PDUs::PDUs(unsigned char* data)
{
    memcpy(&(header.seqNum), data, sizeof(header.seqNum));
    data + sizeof(header.seqNum);
    memcpy(&(header.chksum), data, sizeof(header.chksum));
    data + sizeof(header.chksum);
    memcpy(&(header.flag), data, sizeof(header.flag));
    data + sizeof(header.flag);
}

void PDUs::calcChksum()
{
    
}