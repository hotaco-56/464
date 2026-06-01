#ifndef PDU_H
#define PDU_H

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

#include "vars.h"

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
#define FILENAME_ERR   32U
#define INVALID        33U
#define TEARDOWN       34U
#define TEARDOWN_ACK   35U
#define FIN            36U

#define MAX_PAYLOAD_SIZE 1400UL
#define HEADER_SIZE 7U
#define MAX_PDU_SIZE (HEADER_SIZE + MAX_PAYLOAD_SIZE)

typedef struct {
    // must be in this order
    seqNum_t seqNum = 0;
    uint16_t chksum = 0;
    uint8_t flag = INVALID;
    unsigned char payload[MAX_PAYLOAD_SIZE] = {0};
    uint16_t pduLen = 7; // the receiver will write to this so we know the actual pdu size
    bool acked = false;
    bool valid = false;
} __attribute__((packed)) PDU_T;

class PDU
{
private:
    PDU_T _pdu = {};
    uint16_t _payloadLen = 0;

    // copy data to header section of pdu
    inline void headerCpy(unsigned char* dest) { memcpy(dest, &_pdu, HEADER_SIZE); }
    // copy data to payload section of pdu
    inline void payloadCpy(unsigned char* dest) { memcpy(dest + HEADER_SIZE, _pdu.payload, _payloadLen); } 

public:
    PDU();
    PDU(unsigned char* PDU, uint16_t pduLen);
    ~PDU();

    inline uint16_t calcChksum() { return (uint16_t)in_cksum((unsigned short*)&_pdu, getPDULen()); }
    inline void clearChksum(void) { _pdu.chksum = 0; }
    inline uint16_t getChksum(void) const { return _pdu.chksum; }

    inline void setSeqNum(uint32_t n) { _pdu.seqNum = htonl(n); } // in network order
    inline seqNum_t getSeqNum(void) const { return _pdu.seqNum; } // in network order

    inline void setFlag(uint8_t flag) { _pdu.flag = flag; }
    inline uint8_t getFlag(void) const { return _pdu.flag;  }

    inline void addPayload(unsigned char* payload, int16_t size) 
    {
        memcpy(_pdu.payload + _payloadLen, payload, size);
        _payloadLen += size;
    }
    inline unsigned char* getPayload() { return _pdu.payload; }
    inline uint16_t getPayloadLen() const { return _payloadLen; }
    
    inline PDU_T* createPDU() { 
        _pdu.valid = true;
        _pdu.pduLen = getPDULen();
        if (calcChksum() == 0)
            return &_pdu;
        _pdu.chksum = calcChksum();
        return &_pdu; 
    }
    inline uint16_t getPDULen() const { return HEADER_SIZE + _payloadLen; }
};

#endif