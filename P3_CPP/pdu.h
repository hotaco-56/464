#ifndef PDU_H
#define PDU_H

#ifndef __DEBUG_
#define __PRINTF_DBG (void)
#else
#define __PRINTF_DBG printf
#endif

#include "inttypes.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>

// #include "gethostbyname.h"
#include "networks.h"
// #include "safeUtil.h"
#include "checksum.h"

// a. Flag < 32 if not specified in this list cannot be used, if you create your own flag use ≥ 32
// b. Flag = 1 Setup packet (rcopy to server) – optional
// c. Flag = 2 Setup response packet (server to rcopy) - optional
// d. Flag = 3 Data packet
// e. Flag = 4 (Not used this quarter)
// f. Flag = 5 RR packet
// g. Flag = 6 SREJ packet
// h. Flag = 7 Packet contains the file name/buffer-size/window-size (rcopy to server)
// i. Flag = 8 Packet contains the response to the filename packet (server to rcopy)
#define SETUP          1U
#define SETUP_ACK      2U
#define DATA           3U
#define RR             5U
#define SREJ           6U
#define FILENAME       7U
#define FILENAME_ACK   8U

#define MAX_PAYLOAD_SIZE 1400UL
#define HEADER_SIZE 7U
#define MAX_PDU_SIZE (HEADER_SIZE + MAX_PAYLOAD_SIZE)

class PDU
{
private:
    typedef struct {
        uint32_t seqNum;
        uint16_t chksum;
        uint8_t flag;
    } __attribute__((packed)) Header;
     

    Header _header = {};
    unsigned char _payload[MAX_PAYLOAD_SIZE] = {0};
    uint16_t _payloadLen = 0;

public:
    PDU();
    PDU(unsigned char* PDU, uint16_t pduLen);
    ~PDU();
    void calcChksum(uint16_t pduLen);
    inline void clearChksum(void) { _header.chksum = 0; }
    inline uint16_t getChksum(void) { return _header.chksum; }

    inline void setSeqNum(uint32_t n) { _header.seqNum = htonl(n); } // in network order
    inline uint32_t getSeqNum(void) { return _header.seqNum; } // in network order

    inline void setFlag(uint8_t flag) { _header.flag = flag; }
    inline uint8_t getFlag(void) { return _header.flag;  }

    inline void addPayload(unsigned char* payload, int16_t size) 
    {
        memcpy(_payload + _payloadLen, payload, size);
        _payloadLen += size;
    }
    inline unsigned char* getPayload() { return _payload; }
    inline uint16_t getPayloadLen() { return _payloadLen; }
    inline uint16_t getPDULen() { return HEADER_SIZE + _payloadLen; }
    inline void headerCpy(unsigned char* dest) { memcpy(dest, &_header, HEADER_SIZE); }
    inline void payloadCpy(unsigned char* dest) { memcpy(dest, _payload, _payloadLen); }
};

#endif