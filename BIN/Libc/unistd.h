#ifndef UNISTD_H
#define UNISTD_H 1

#include "../LibAxpApi/AxpApi.h"

#define SIGBUF_SIZE                        4096

// Signal Type Constants
#define SIGNAL_TYPE_KILL                   0xFFFFFFFF
#define SIGNAL_INFO_SECOND_EXECUTION_ITERATION 0xFFFFFFFE
#define SIGNAL_TYPE_SIGNAL_BUFFER_TEST     0xDEADBEEF
#define SIGNAL_TX_AVAIL                    0xEFFFFFFF
#define SIGNAL_POLL_OFFSET                 0xDFFFFFFF
#define SIGNAL_INFO_REQUEST_OFFSET         0xDFFFFFFE
#define SIGNAL_TX_GATE_OFFSET              0xCFFFFFFF
#define SIGNAL_SIGBUF_MANIPULATE           0xAFFFFFFF

// Signal Buffer Memory Layout Offsets
#define SIGBUF_OFFSET_TYPE                 0x0000  // 4 bytes
#define SIGBUF_OFFSET_PARAM0               0x0004  // 8 bytes
#define SIGBUF_OFFSET_PARAM1               0x000C  // 8 bytes
#define SIGBUF_OFFSET_RESERVED             0x0014  // 10 bytes
#define SIGBUF_OFFSET_TX                   0x0020  // TX buffer starts here
#define SIGBUF_TX_INFO_AREA_SIZE           512     // First 512 bytes of TX for info
#define SIGBUF_TX_POLL_OFFSET              (SIGBUF_OFFSET_TX + SIGBUF_TX_INFO_AREA_SIZE)

// Signal Buffer Struct
typedef struct {
    uint32_t SignalSignature;
    uint64_t PARM0;
    uint64_t PARM1;
    uint8_t  Rsv0[10];
    uint8_t  TxBuf[SIGBUF_SIZE - 32];
} __attribute__((packed)) SigBufMem;

#endif /* UNISTD_H */
