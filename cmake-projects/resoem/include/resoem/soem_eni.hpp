#pragma once

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

/** ENI CoE command structure */
typedef struct ec_enicoecmd {
    /** transition(s) during which command should be sent */
    uint16_t Transition;
    /** complete access flag */
    int CA;
    /** ccs (1 = read, 2 = write) */
    uint8_t Ccs;
    /** object index */
    uint16_t Index;
    /** object subindex */
    uint8_t SubIdx;
    /** timeout in us */
    int Timeout;
    /** size in bytes of parameter buffer */
    int DataSize;
    /** pointer to parameter buffer */
    void *Data;
} ec_enicoecmdt;

/** ENI slave structure */
typedef struct ec_enislave {
    uint16_t Slave;
    uint32_t VendorId;
    uint32_t ProductCode;
    uint32_t RevisionNo;
    ec_enicoecmdt *CoECmds;
    int CoECmdCount;
} ec_enislavet;

/** ENI structure */
typedef struct ec_eni {
    ec_enislavet *slave;
    int slavecount;
} ec_enit;

// Transitions matching SOEM's ec_main.h
#define ECT_ESMTRANS_IP 0x0001
#define ECT_ESMTRANS_PS 0x0002
#define ECT_ESMTRANS_PI 0x0004
#define ECT_ESMTRANS_SP 0x0008
#define ECT_ESMTRANS_SO 0x0010
#define ECT_ESMTRANS_SI 0x0020
#define ECT_ESMTRANS_OS 0x0040
#define ECT_ESMTRANS_OP 0x0080
#define ECT_ESMTRANS_OI 0x0100
#define ECT_ESMTRANS_IB 0x0200
#define ECT_ESMTRANS_BI 0x0400
#define ECT_ESMTRANS_II 0x0800
#define ECT_ESMTRANS_PP 0x1000
#define ECT_ESMTRANS_SS 0x2000

#ifdef __cplusplus
}
#endif
