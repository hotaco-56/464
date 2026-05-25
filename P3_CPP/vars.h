#ifndef VARS_H
#define VARS_H

const uint8_t MAX_FILENAME_LEN = 100;
const uint8_t MAX_RETRANSMIT_COUNT = 10;

#define seqNum_t uint32_t
#define windowSize_t uint32_t
#define buffSize_t uint16_t

#ifndef __DEBUG_
#define __PRINTF_DBG (void)
#else
#define __PRINTF_DBG printf
#endif

// #define __SEND_ERR_
#define __SEND_ERR_DBG_ DEBUG_OFF
// #define __SEND_ERR_DBG_ DEBUG_ON

#endif