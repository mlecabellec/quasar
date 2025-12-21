#include "tsync_hw_func.h"
#include "tsync_hw_nonkts_func.h"
#include "ddtsync.h"
#include "binary_coded_decimal.h"
#include "tsync_gpio_stamp_queue.h"
#include "tsync_comm_transaction.h"
#include "tsync_comm_fifo.h"
#include "tsync_comm_validation.h"

#define LOG_MATCH_TIME_REGISTER_VALUES 0
#define LOG_SET_AND_GET 0

typedef enum {
    DIRECTION_GET,
    DIRECTION_SET
} trans_direction_t;

typedef enum {
    MATCH_LO = 0,
    MATCH_HI = 1
} match_level_t;


typedef struct {
    uint16_t subSecLow;
    uint16_t subSecHigh;
    uint16_t superSecLow;
    uint16_t superSecMid;
    uint16_t superSecHigh;
} gpio_match_time_t;

void logTsyncInterruptWord(TPRO_INSTANCE_T *hw, char *message, uint16_t interruptWord) {
    unsigned int setBitCount = 0;
    const uint16_t one = 1;
    TPRO_PRINT("%s   0x%04x\n", message, interruptWord);

#define CHECK_INT_BIT(index, string) if (interruptWord & (one<<(index))) { TPRO_PRINT("  bit %2u: %s\n", index, #string ); setBitCount++; }

    CHECK_INT_BIT(0, 1PPS received );
    CHECK_INT_BIT(1, Timing System status );
    CHECK_INT_BIT(2, Host / uC bus FIFO empty );
    CHECK_INT_BIT(3, Host / uC bus FIFO overflow );
    CHECK_INT_BIT(4, uC / host bus FIFO data available );
    CHECK_INT_BIT(5, uC / host bus FIFO overflow );
    CHECK_INT_BIT(6, GPIO input 0 event );
    CHECK_INT_BIT(7, GPIO input 1 event );
    CHECK_INT_BIT(8, GPIO input 2 event );
    CHECK_INT_BIT(9, GPIO input 3 event );
    CHECK_INT_BIT(10, Time stamp occurred );
    CHECK_INT_BIT(11, GPIO output 0 event );
    CHECK_INT_BIT(12, GPIO output 1 event );
    CHECK_INT_BIT(13, GPIO output 2 event );
    CHECK_INT_BIT(14, GPIO output 3 event );
    CHECK_INT_BIT(15, Keypad event);
    TPRO_PRINT("  %u bits total.\n", setBitCount);
}

TSYNC_ERROR doyTimeFromTimestamp(TSYNCD_HWTimeObj *doyTime,
                                 gpio_timestamp_t *stampTime) {
    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;
    uint32_t bcdYear;

    doyTime->bSync = (stampTime->subSecHigh & 0x8000) ? 1 : 0;

    doyTime->time.ns =
        (((stampTime->subSecHigh & 0xfff) << 16) | stampTime->subSecLow) * 5;

    bcdWholeSeconds = stampTime->superSecLow & 0xff;
    doyTime->time.seconds = uintFromBCD(bcdWholeSeconds, 2);

    bcdMinute = (stampTime->superSecLow >> 8) & 0xff;
    doyTime->time.minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = stampTime->superSecMidLow & 0xff;
    doyTime->time.hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((stampTime->superSecMidHigh & 0x0f) << 8) |
        ((stampTime->superSecMidLow>>8) & 0xff);
    doyTime->time.doy = uintFromBCD(bcdDayOfYear, 3);

    bcdYear =
        ((stampTime->superSecHigh << 12) & 0xf000) |
        ((stampTime->superSecMidHigh >> 4) & 0x0fff);
    doyTime->time.years = uintFromBCD(bcdYear, 4);

    return TSYNC_SUCCESS;
}

TSYNC_ERROR doyTimeFromGPIOTime(TSYNCD_TimeObj *doyTime,
                                gpio_match_time_t *matchTime) {
    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;

    bcdWholeSeconds = matchTime->superSecLow & 0xff;
    doyTime->seconds = uintFromBCD(bcdWholeSeconds, 2);

    doyTime->ns = ((matchTime->subSecHigh << 16) | matchTime->subSecLow) * 5;

    bcdMinute = (matchTime->superSecLow >> 8) & 0xff;
    doyTime->minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = matchTime->superSecMid & 0xff;
    doyTime->hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((matchTime->superSecHigh & 0x0f) << 8) |
        ((matchTime->superSecMid>>8) & 0xff);
    doyTime->doy = uintFromBCD(bcdDayOfYear, 3);

    doyTime->years = 0;

    return TSYNC_SUCCESS;
}

TSYNC_ERROR gpioTimeFromDOYTime(gpio_match_time_t *match_time,
                                TSYNCD_TimeObj *doyTime) {

    unsigned int fiveNanos;
    unsigned int bcdWholeSeconds, bcdMinutes, bcdHours, bcdDays;

    fiveNanos = doyTime->ns / 5;

    match_time->subSecHigh = fiveNanos >> 16;
    match_time->subSecLow = fiveNanos & 0xffff;

    bcdWholeSeconds = bcdFromUint(doyTime->seconds);
    bcdMinutes = bcdFromUint(doyTime->minutes);
    bcdHours = bcdFromUint(doyTime->hours);
    bcdDays = bcdFromUint(doyTime->doy);

    match_time->superSecLow = (bcdMinutes << 8) | bcdWholeSeconds;
    match_time->superSecMid = ((bcdDays & 0xff) << 8) | bcdHours;
    match_time->superSecHigh = bcdDays >> 8;

    return TSYNC_SUCCESS;
}

TSYNC_ERROR matchTimeRegisterOffset(OD_PIN iGpio,
                                    match_level_t level,
                                    unsigned int *offset) {

#define OFFSET(testIndex, testLevel, resultRegister)    \
    if ((iGpio == testIndex) && (level == testLevel)) { \
        *offset = resultRegister;                       \
        status = TSYNC_SUCCESS;                         \
    }

    TSYNC_ERROR status = TSYNC_DRV_INVALID_INDEX;

    OFFSET(OD_PIN_0, MATCH_HI, tsync_gpio_output_0_high_match_offset);
    OFFSET(OD_PIN_0, MATCH_LO, tsync_gpio_output_0_low_match_offset);
    OFFSET(OD_PIN_1, MATCH_HI, tsync_gpio_output_1_high_match_offset);
    OFFSET(OD_PIN_1, MATCH_LO, tsync_gpio_output_1_low_match_offset);
    OFFSET(OD_PIN_2, MATCH_HI, tsync_gpio_output_2_high_match_offset);
    OFFSET(OD_PIN_2, MATCH_LO, tsync_gpio_output_2_low_match_offset);
    OFFSET(OD_PIN_3, MATCH_HI, tsync_gpio_output_3_high_match_offset);
    OFFSET(OD_PIN_3, MATCH_LO, tsync_gpio_output_3_low_match_offset);

#undef OFFSET

    return status;
}

TSYNC_ERROR intBitFromTypeAndIndex(INT_TYPE intType, uint32_t index,
                                   unsigned int *bit) {
    switch (intType) {

        /* DO NOT ADD A SEMICOLON AFTER break!
         *
         * Editors will want to reindent the ONE_TO_ONE lines if they
         * don't have trailing semicolons, but if I leave the
         * semicolons there and at the end of break, Sun Studio
         * considers the "statement" between the two semicolons to be
         * unreachable code.  */
#define ONE_TO_ONE(typeName, result)            \
    case typeName:                              \
        *bit = result;                          \
        break

        ONE_TO_ONE(INT_1PPS, 0);
        ONE_TO_ONE(INT_SVC_REQ, 1);
        ONE_TO_ONE(INT_LCL_UC_FIFO_EMPTY, 2);
        ONE_TO_ONE(INT_LCL_UC_FIFO_OVER, 3);
        ONE_TO_ONE(INT_UC_LCL_FIFO_DATA, 4);
        ONE_TO_ONE(INT_UC_LCL_FIFO_OVER, 5);
        ONE_TO_ONE(INT_TMSTMP, 10);

#undef ONE_TO_ONE

    case INT_GPIO_IN:
        if (index >= ID_PIN_NUM) { return TSYNC_DRV_INVALID_INDEX; }
        *bit = 6 + index;
        break;

    case INT_GPIO_OUT:
        if (index >= OD_PIN_NUM) { return TSYNC_DRV_INVALID_INDEX; }
        *bit = 11 + index;
        break;

    default:
        return TSYNC_DRV_INVALID_INDEX;
    };
    return TSYNC_SUCCESS;
}

TSYNC_ERROR get_hw_sys_time(TPRO_INSTANCE_T *hw,
                            uint8_t *outPayload,
                            uint32_t maxOutLength,
                            uint32_t *actualOutLength) {

    TSYNCD_HWTimeObj hwTime;

#define shortsPerSuperSecond  4
#define shortsPerSubSecond 2

    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;
    uint32_t bcdYear;

    uint16_t superSec[shortsPerSuperSecond];
    uint16_t nanoSubSec[shortsPerSubSecond];

    const unsigned int outLength = sizeof(TSYNCD_HWTimeObj);
    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    /*****************************************
     * Get the data from the board
     */
//    if (hw->options == TSYNC_PCIe)

        TSYNC_COPY_FROM_IO32(superSec, tsync_super_second_offset,
                             shortsPerSuperSecond / 2);
        TSYNC_COPY_FROM_IO32(nanoSubSec, tsync_nano_sub_second_offset,
                             shortsPerSubSecond / 2);



    /*****************************************
     * Fill out the time struct
     */

    hwTime.bSync = (nanoSubSec[1] & 0x8000) ? 1 : 0;

    hwTime.time.ns = (((nanoSubSec[1] & 0xfff) << 16) | nanoSubSec[0]) * 5;

    bcdWholeSeconds = superSec[0] & 0xff;
    hwTime.time.seconds = uintFromBCD(bcdWholeSeconds, 2);

    bcdMinute = (superSec[0] >> 8) & 0xff;
    hwTime.time.minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = superSec[1] & 0xff;
    hwTime.time.hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((superSec[2] & 0x0f) << 8) |
        ((superSec[1]>>8) & 0xff);
    hwTime.time.doy = uintFromBCD(bcdDayOfYear, 3);

    bcdYear =
        ((superSec[3] << 12) & 0xf000) |
        ((superSec[2] >> 4) & 0x0fff);
    hwTime.time.years = uintFromBCD(bcdYear, 4);

    memcpy(outPayload, &hwTime, outLength);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
#undef shortsPerSuperSecond
#undef shortsPerSubSecond
} /* end of get_hw_sys_time */

TSYNC_ERROR get_hw_sec_time(TPRO_INSTANCE_T *hw,
                            uint8_t *outPayload,
                            uint32_t maxOutLength,
                            uint32_t *actualOutLength) {

    TSYNCD_HWTimeSecondsObj hwTime;

#define shortsPerSuperSecond  2
#define shortsPerSubSecond  2

    uint16_t superSec[shortsPerSuperSecond];
    uint16_t nanoSubSec[shortsPerSubSecond];

    const unsigned int outLength = sizeof(TSYNCD_HWTimeSecondsObj);
    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    /*****************************************
     * Get the data from the board
     */

//    if (hw->options == TSYNC_PCIe)

        TSYNC_COPY_FROM_IO32(superSec, tsync_super_second_binary_offset,
                             shortsPerSuperSecond / 2);
        TSYNC_COPY_FROM_IO32(nanoSubSec, tsync_nano_sub_second_offset,
                             shortsPerSubSecond / 2);



    hwTime.bSync = (nanoSubSec[1] & 0x8000) ? 1 : 0;
    hwTime.time.ns = (((nanoSubSec[1] & 0xfff) << 16) | nanoSubSec[0]) * 5;
    hwTime.time.seconds = (superSec[1] << 16) | superSec[0];

    memcpy(outPayload, &hwTime, outLength);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
#undef shortsPerSuperSecond
#undef shortsPerSubSecond
} /* end of get_hw_sec_time */


TSYNC_ERROR get_hw_timestamp_enabled(TPRO_INSTANCE_T *hw,
                                     uint8_t *outPayload,
                                     uint32_t maxOutLength,
                                     uint32_t *actualOutLength) {

    uint16_t timestampStatus;
    TSYNCD_Boolean enabled = TDB_FALSE;

    const unsigned int outLength = sizeof(TSYNCD_Boolean);
    if (maxOutLength < outLength) {
        TPRO_PRINT("get_hw_timestamp_enabled generates %u bytes, max %u.\n",
                   outLength, maxOutLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    timestampStatus =
        TSYNC_READ_REG(tsync_timestamp_status_offset);

    if (timestampStatus & 0x1) {
        enabled = TDB_TRUE;
    }
    else {
        enabled = TDB_FALSE;
    }

    memcpy(outPayload, &enabled, outLength);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
} /* end of get_hw_timestamp_enabled */

TSYNC_ERROR get_hw_timestamp_count(TPRO_INSTANCE_T *hw,


                                   uint8_t *inPayload,
                                   uint32_t inLength,
                                   uint8_t *outPayload,
                                   uint32_t maxOutLength,
                                   uint32_t *actualOutLength) {
    TMSTMP_SRC source;
    uint32_t count;
    TSYNC_ERROR gpioResult;

    const unsigned int expectedInLength = sizeof(TMSTMP_SRC);
    const unsigned int outLength = sizeof(uint32_t);

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_timestamp_count expects %u bytes of inPayload, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        TPRO_PRINT("get_hw_timestamp_count generates %u bytes of outPayload, got %u max.\n",
                   outLength, maxOutLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&source, inPayload, expectedInLength);

    gpioResult =
        gpioQueueCount(hw, source, &count);
    if (gpioResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return gpioResult;
    }

    memcpy(outPayload, &count, outLength);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
} /* end of get_hw_timestamp_count */

TSYNC_ERROR get_hw_timestamp_data(TPRO_INSTANCE_T *hw,
                                  uint8_t *inPayload,
                                  uint32_t inLength,
                                  uint8_t *outPayload,
                                  uint32_t maxOutLength,
                                  uint32_t *actualOutLength) {
    TMSTMP_SRC source;
    uint32_t count, iStamp;
    TSYNC_ERROR getCountResult, removeResult;

    const unsigned int expectedInLength = sizeof(TMSTMP_SRC);
    const unsigned int outLength = sizeof(TSYNCD_HWTimeDataObj);

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_timestamp_data expects %u bytes of inPayload, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        TPRO_PRINT("get_hw_timestamp_data generates %u bytes of outPayload, got %u max.\n",
                   outLength, maxOutLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&source, inPayload, expectedInLength);
    getCountResult =
        gpioQueueCount(hw, source, &count);
    if (getCountResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return getCountResult;
    }

    for (iStamp = 0; iStamp < TSYNC_TIMESTAMP_DATA_NUM; iStamp++) {
        TSYNCD_HWTimeObj *outStamp = (TSYNCD_HWTimeObj*) outPayload;
        outStamp += iStamp;

        if (iStamp < count) {
            gpio_timestamp_t packedStamp;

            removeResult = gpioQueueRemove(hw, source, &packedStamp);
            if (removeResult != TSYNC_SUCCESS) {
                *actualOutLength = 0;
                return removeResult;
            }

            doyTimeFromTimestamp(outStamp, &packedStamp);
        }
        else {
            memset(outStamp, 0, sizeof(TSYNCD_HWTimeObj));
        }
    }

    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
} /* end of get_hw_timestamp_data */

TSYNC_ERROR get_hw_timestamp_single(TPRO_INSTANCE_T *hw,
                                    uint8_t *inPayload,
                                    uint32_t inLength,
                                    uint8_t *outPayload,
                                    uint32_t maxOutLength,
                                    uint32_t *actualOutLength) {
    TMSTMP_SRC source;
    uint32_t count;
    TSYNC_ERROR getCountResult, removeResult;

    const unsigned int expectedInLength = sizeof(TMSTMP_SRC);
    const unsigned int outLength = sizeof(TSYNCD_HWTimeObj);

    gpio_timestamp_t packedStamp;

    TSYNCD_HWTimeObj *outPayloadAsTime = (TSYNCD_HWTimeObj*)outPayload;

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_timestamp_single expects %u bytes of inPayload, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        TPRO_PRINT("get_hw_timestamp_single generates %u bytes of outPayload, got %u max.\n",
                   outLength, maxOutLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&source, inPayload, expectedInLength);
    getCountResult =
        gpioQueueCount(hw, source, &count);
    if (getCountResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return getCountResult;
    }

    removeResult = gpioQueueRemove(hw, source, &packedStamp);
    if (removeResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return removeResult;
    }

    doyTimeFromTimestamp(outPayloadAsTime, &packedStamp);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
}

TSYNC_ERROR get_hw_gpo_match(match_level_t level,
                            TPRO_INSTANCE_T *hw,
                            uint8_t *inPayload,
                            uint32_t inLength,
                            uint8_t *outPayload,
                            uint32_t maxOutLength,
                            uint32_t *actualOutLength) {

    const unsigned int expectedInLength = sizeof(OD_PIN);
    const unsigned int outLength = sizeof(TSYNCD_TimeObj);

    OD_PIN iGpio;
    TSYNCD_TimeObj time;
    unsigned int registerOffset;
    gpio_match_time_t gpioMatchTime;

    TSYNC_ERROR status;

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_gpo_match(%s) expected %u bytes in, got %u.\n",
                   ((level == MATCH_HI) ? "hi": "lo"), expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        TPRO_PRINT("get_hw_gpo_match(%s) generates %u bytes, got room for %u.\n",
                   ((level == MATCH_HI) ? "hi": "lo"), outLength, maxOutLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&iGpio, inPayload, expectedInLength);
    status = matchTimeRegisterOffset(iGpio, level, &registerOffset);
    if (status != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return status;
    }

    gpioMatchTime.subSecLow = TSYNC_READ_REG(registerOffset + (sizeof(uint16_t) * 0));
    gpioMatchTime.subSecHigh = TSYNC_READ_REG( registerOffset + (sizeof(uint16_t) * 1));
    gpioMatchTime.superSecLow = TSYNC_READ_REG( registerOffset + (sizeof(uint16_t) * 2));
    gpioMatchTime.superSecMid = TSYNC_READ_REG( registerOffset + (sizeof(uint16_t) * 3));
    gpioMatchTime.superSecHigh = TSYNC_READ_REG( registerOffset + (sizeof(uint16_t) * 4));

#if LOG_MATCH_TIME_REGISTER_VALUES
    TPRO_PRINT("get_hw_gpo_match: sub %04x %04x, super %04x %04x %04x\n",
               gpioMatchTime.subSecHigh,
               gpioMatchTime.subSecLow,
               gpioMatchTime.superSecHigh,
               gpioMatchTime.superSecMid,
               gpioMatchTime.superSecLow);
#endif

    status = doyTimeFromGPIOTime(&time, &gpioMatchTime);
    if (status != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return status;
    }

    memcpy(outPayload, &time, outLength);
    *actualOutLength = outLength;

    return TSYNC_SUCCESS;

} /* end of get_hw_gpo_match */

TSYNC_ERROR get_hw_int_mask(TPRO_INSTANCE_T *hw,
                            uint8_t *inPayload,
                            uint32_t inLength,
                            uint8_t *outPayload,
                            uint32_t maxOutLength,
                            uint32_t *actualOutLength) {
    const unsigned int expectedInLength = sizeof(TSYNCD_InterruptMaskGetObj);
    const unsigned int outLength = sizeof(TSYNCD_Boolean);

    uint16_t boardMask = TSYNC_READ_REG(tsync_interrupt_mask_offset);
    unsigned int whichBit = 0;
    uint16_t bitMask;
    TSYNC_ERROR whichBitResult;

    TSYNCD_Boolean maskEnabled = TDB_TRUE;

    TSYNCD_InterruptMaskGetObj getObj;

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_int_mask expected %u bytes in, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&getObj, inPayload, expectedInLength);

    whichBitResult =
        intBitFromTypeAndIndex(getObj.intType, getObj.index, &whichBit);
    if (whichBitResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return TSYNC_DRV_INVALID_INDEX;
    }

    bitMask = 1 << whichBit;
    if ((boardMask & bitMask) != 0) {
        maskEnabled = TDB_TRUE;
    }
    else {
        maskEnabled = TDB_FALSE;
    }

    memcpy(outPayload, &maskEnabled, outLength);
    *actualOutLength = outLength;

    return TSYNC_SUCCESS;
} /* end of get_hw_int_mask */


TSYNC_ERROR get_hw_fpga_id(TPRO_INSTANCE_T *hw,
                           uint8_t *outPayload,
                           uint32_t maxOutLength,
                           uint32_t *actualOutLength) {

    TSYNCD_FPGAInfoObj fpgaInfo;

    const unsigned int outLength = sizeof(TSYNCD_FPGAInfoObj);;
    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    /*****************************************
     * Get the data from the board
     */

    fpgaInfo.id = TSYNC_READ_REG(tsync_fpga_status_offset);
    fpgaInfo.rev =TSYNC_READ_REG(tsync_fpga_revision_id_offset);

    memcpy(outPayload, &fpgaInfo, outLength);
    *actualOutLength = outLength;
    return TSYNC_SUCCESS;
} /* end of get_hw_fpga_id */


TSYNC_ERROR get_hw_int_cnt(TPRO_INSTANCE_T *hw,
                            uint8_t *inPayload,
                            uint32_t inLength,
                            uint8_t *outPayload,
                            uint32_t maxOutLength,
                            uint32_t *actualOutLength) {

    const unsigned int expectedInLength = sizeof(TSYNCD_InterruptMaskGetObj);
    const unsigned int outLength = sizeof(uint32_t);

    uint32_t intCnt = 0;
    unsigned int whichBit = 0;
    TSYNC_ERROR whichBitResult;

    TSYNCD_InterruptMaskGetObj getObj;

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_int_cnt expected %u bytes in, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&getObj, inPayload, expectedInLength);

    whichBitResult =
        intBitFromTypeAndIndex(getObj.intType, getObj.index, &whichBit);
    if (whichBitResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return TSYNC_DRV_INVALID_INDEX;
    }

    intCnt = hw->intCounter[whichBit];
 
    memcpy(outPayload, &intCnt, outLength);
    *actualOutLength = outLength;

    return TSYNC_SUCCESS;

} /* end of get_hw_int_cnt */


TSYNC_ERROR get_hw_int_ts(TPRO_INSTANCE_T *hw,
                          uint8_t *inPayload,
                          uint32_t inLength,
                          uint8_t *outPayload,
                          uint32_t maxOutLength,
                          uint32_t *actualOutLength) {

    const unsigned int expectedInLength = sizeof(TSYNCD_InterruptMaskGetObj);
    const unsigned int outLength = sizeof(TSYNCD_TimeSecondsObj);

    unsigned int whichBit    = 0;
    TSYNC_ERROR whichBitResult;

    TSYNCD_InterruptMaskGetObj getObj;

    if (inLength < expectedInLength) {
        TPRO_PRINT("get_hw_int_ts expected %u bytes in, got %u.\n",
                   expectedInLength, inLength);
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    if (maxOutLength < outLength) {
        *actualOutLength = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&getObj, inPayload, expectedInLength);

    whichBitResult =
        intBitFromTypeAndIndex(getObj.intType, getObj.index, &whichBit);
    if (whichBitResult != TSYNC_SUCCESS) {
        *actualOutLength = 0;
        return TSYNC_DRV_INVALID_INDEX;
    }
 
    memcpy(outPayload, &(hw->intTime[whichBit]), outLength);
    *actualOutLength = outLength;

    return TSYNC_SUCCESS;

} /* end of get_hw_int_ts */


TSYNC_ERROR get_hw_temperature(TPRO_INSTANCE_T *hw,
                                uint8_t  *outPayload,
                                uint32_t maxOutLength,
                                uint32_t *actualResultBytes) {
                            
    const unsigned int outLength = sizeof(uint16_t);
    uint16_t readVal = 0;

    if (maxOutLength < outLength) {
        *actualResultBytes = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    readVal = TSYNC_READ_REG(tsync_temperature_offset);
    
    memcpy(outPayload, &readVal, outLength);
    *actualResultBytes = outLength;

    return TSYNC_SUCCESS;
    
} /* end of get_hw_temperature */

TSYNC_ERROR get_hw_temperature_ss(TPRO_INSTANCE_T *hw,
                                uint8_t  *outPayload,
                                uint32_t maxOutLength,
                                uint32_t *actualResultBytes) {
                            
    const unsigned int outLength = sizeof(uint16_t);
    uint16_t readVal = 0;

    if (maxOutLength < outLength) {
        *actualResultBytes = 0;
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    readVal = TSYNC_READ_REG(tsync_temperature_offset_ss);
    
    memcpy(outPayload, &readVal, outLength);
    *actualResultBytes = outLength;

    return TSYNC_SUCCESS;
    
} /* end of get_hw_temperature_ss */


TSYNC_ERROR set_hw_timestamp_enabled(TPRO_INSTANCE_T *hw,
                                     uint8_t *inPayload,
                                     uint32_t inLength) {

    const unsigned int expectedInLength = sizeof(TSYNCD_Boolean);
    uint16_t boardStatus;

    TSYNCD_Boolean enable;

    if (inLength < expectedInLength) {
        TPRO_PRINT("set_hw_timestamp_enabled expects %u bytes of inPayload, got %u.\n",
                   expectedInLength, inLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&enable, inPayload, expectedInLength);

    boardStatus = TSYNC_READ_REG(tsync_timestamp_status_offset);

    if (enable == TDB_TRUE) {
        boardStatus |= 0x1;
    }
    else {
        boardStatus &= ~0x1;
    }

    TSYNC_WRITE_REG(boardStatus, tsync_timestamp_status_offset);
    return TSYNC_SUCCESS;
} /* end of set_hw_timestamp_enabled */

TSYNC_ERROR set_hw_timestamp_req(TPRO_INSTANCE_T *hw) {
    TSYNC_WRITE_REG(0x3, tsync_timestamp_status_offset);
    return TSYNC_SUCCESS;
}

TSYNC_ERROR set_hw_timestamp_clr(TPRO_INSTANCE_T *hw,
                                 uint8_t *inPayload,
                                 uint32_t inLength) {

    TMSTMP_SRC source;
    TSYNC_ERROR initResult;

    const unsigned int expectedInLength = sizeof(TMSTMP_SRC);

    if (inLength < expectedInLength) {
        TPRO_PRINT("set_hw_timestamp_clr expects %u bytes of inPayload, got %u.\n",
                   expectedInLength, inLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&source, inPayload, expectedInLength);

    initResult = gpioQueueInit(hw, source);

    return initResult;
}

TSYNC_ERROR set_hw_gpo_match(match_level_t level,
                             TPRO_INSTANCE_T *hw,
                             uint8_t *inPayload,
                             uint32_t inLength) {

    const unsigned int expectedInLength = sizeof(TSYNCD_MatchTimeObj);

    TSYNCD_MatchTimeObj inMatch;
    unsigned int registerOffset;
    gpio_match_time_t gpioMatchTime;

    TSYNC_ERROR status;

    if (inLength < expectedInLength) {
        TPRO_PRINT("set_hw_gpo_match(%s) expected %u bytes in, got %u.\n",
                   ((level == MATCH_HI) ? "hi": "lo"), expectedInLength, inLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&inMatch, inPayload, expectedInLength);
    status = matchTimeRegisterOffset(inMatch.index, level, &registerOffset);
    if (status != TSYNC_SUCCESS) {
        return status;
    }

    status = gpioTimeFromDOYTime(&gpioMatchTime, &inMatch.time);
    if (status != TSYNC_SUCCESS) {
        return status;
    }

#if LOG_MATCH_TIME_REGISTER_VALUES
    TPRO_PRINT("set_hw_gpo_match: sub %04x %04x, super %04x %04x %04x\n",
               gpioMatchTime.subSecHigh,
               gpioMatchTime.subSecLow,
               gpioMatchTime.superSecHigh,
               gpioMatchTime.superSecMid,
               gpioMatchTime.superSecLow);
#endif

    TSYNC_WRITE_REG(gpioMatchTime.subSecLow, registerOffset + (sizeof(uint16_t) * 0));
    TSYNC_WRITE_REG(gpioMatchTime.subSecHigh, registerOffset + (sizeof(uint16_t) * 1));
    TSYNC_WRITE_REG(gpioMatchTime.superSecLow, registerOffset + (sizeof(uint16_t) * 2));
    TSYNC_WRITE_REG(gpioMatchTime.superSecMid, registerOffset + (sizeof(uint16_t) * 3));
    TSYNC_WRITE_REG(gpioMatchTime.superSecHigh, registerOffset + (sizeof(uint16_t) * 4));


    return TSYNC_SUCCESS;
}

TSYNC_ERROR set_hw_int_mask_by_index(TPRO_INSTANCE_T *hw,
                                     unsigned int interruptIndex,
                                     TSYNCD_Boolean enableMask) {

    uint16_t boardMask;
    uint16_t bitMask;

    boardMask = TSYNC_READ_REG(tsync_interrupt_mask_offset);
    bitMask = 1 << interruptIndex;
    if (enableMask == TDB_TRUE) {
        boardMask |= bitMask;
    }
    else {
        boardMask &= ~bitMask;
    }

    TSYNC_WRITE_REG(boardMask, tsync_interrupt_mask_offset);

    return TSYNC_SUCCESS;
}/* end of set_hw_int_mask_by_index */


TSYNC_ERROR set_hw_int_mask(TPRO_INSTANCE_T *hw,
                            uint8_t *inPayload,
                            uint32_t inLength) {
    const unsigned int expectedInLength = sizeof(TSYNCD_InterruptMaskSetObj);

    unsigned int whichBit = 0;
    TSYNC_ERROR whichBitResult;

    TSYNCD_InterruptMaskSetObj setObj;

    if (inLength < expectedInLength) {
        TPRO_PRINT("set_hw_int_mask expected %u bytes in, got %u.\n",
                   expectedInLength, inLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&setObj, inPayload, expectedInLength);

    whichBitResult =
        intBitFromTypeAndIndex(setObj.intType, setObj.index, &whichBit);
    if (whichBitResult != TSYNC_SUCCESS) {
        return TSYNC_DRV_INVALID_INDEX;
    }

    return set_hw_int_mask_by_index(hw, whichBit, setObj.bEnable);
}

TSYNC_ERROR set_hw_int_cnt_clr(TPRO_INSTANCE_T *hw,
                            uint8_t *inPayload,
                            uint32_t inLength)  {

    const unsigned int expectedInLength = sizeof(TSYNCD_InterruptMaskGetObj);

    unsigned int whichBit = 0;
    TSYNC_ERROR whichBitResult;

    TSYNCD_InterruptMaskGetObj getObj;

    if (inLength < expectedInLength) {
        TPRO_PRINT("set_hw_int_cnt_clr expected %u bytes in, got %u.\n",
                   expectedInLength, inLength);
        return TSYNC_DRV_BUFFER_OVERFLOW;
    }

    memcpy(&getObj, inPayload, expectedInLength);

    whichBitResult =
        intBitFromTypeAndIndex(getObj.intType, getObj.index, &whichBit);
    if (whichBitResult != TSYNC_SUCCESS) {
        return TSYNC_DRV_INVALID_INDEX;
    }

    hw->intCounter[whichBit] = 0 ; // clear interrupt counter
 

    return TSYNC_SUCCESS;

} /* end of set_hw_int_cnt_clr */




TSYNC_ERROR hw_trans(trans_direction_t direction,
                     uint16_t iid,
                     TPRO_INSTANCE_T *hw,
                     uint8_t *inPayload,
                     uint32_t inLength,
                     uint8_t *outPayload,
                     uint32_t maxOutLength,
                     uint32_t *actualOutLength) {
    ioctl_trans localTransaction;

    uint16_t *inputBuffer = hw->userRequestTransactionBuffer;
    uint16_t *outputBuffer = hw->userResultTransactionBuffer;

    uint8_t *bufferPos8;
    uint16_t *bufferPos16;
    uint32_t *bufferPos32;

    unsigned char localTransactionResult;

    /*************************************************************
     * fill in the HW transaction
     */
    bufferPos16 = inputBuffer;
    *bufferPos16++ = BIG_END_16_IID(iid);
    *bufferPos16++ = (direction == DIRECTION_GET) ? BIG_END_16(0x8000)
                                                  : BIG_END_16(0x8002);

    bufferPos32 = (uint32_t*) bufferPos16;
    *bufferPos32++ = BIG_END_32(inLength);

    bufferPos8 = (uint8_t*) bufferPos32;
    memcpy(bufferPos8, inPayload, inLength);
    bufferPos8 += inLength;

    /***********************************************************************
     * Do the local transaction
     */

    localTransaction.inBuffer = (uint8_t*) inputBuffer;
    localTransaction.inBufferLength =
        (uintptr_t) bufferPos8 - (uintptr_t) inputBuffer;
    localTransaction.outBuffer = (uint8_t*) outputBuffer;
    localTransaction.maxOutBufferLength = maxOutLength;

    localTransactionResult =
        tsyncLocalTransaction(hw, &localTransaction);

    if (localTransactionResult != 0) {
        TPRO_PRINT("localTransactionResult %d " SRC_LOCATION_FORMAT_STRING,
                   localTransactionResult, SRC_LOCATION_VARIABLES);
        *actualOutLength = localTransaction.actOutBufferLength;
        return localTransaction.status;
    }

    /*********************************************************
     * Check for errors, copy returned payload
     */

    bufferPos32 = (uint32_t*) outputBuffer;
    *actualOutLength = BIG_END_32(bufferPos32[1]);
    memcpy(outPayload, &outputBuffer[4], *actualOutLength);
    if (outputBuffer[1] & BIG_END_16(0x1)) {
        return TSYNC_DRV_FW_TRANS_ERROR;
    }

    return localTransaction.status;
}

TSYNC_ERROR hw_handle_get(TPRO_INSTANCE_T *hw,
                          ioctl_trans_hw *transaction) {

    TSYNC_ERROR transactionResult = TSYNC_DRV_INVALID_INDEX;
    uint32_t actualResultBytes = 0;

    /*****************************************
     * What is the transaction?
     */

#if LOG_SET_AND_GET
    TPRO_PRINT("Dest: %04x, iid %04x.  inLength %u, maxOutLength %u.\n",
               transaction->dest, transaction->iid,
               transaction->inLength,
               transaction->maxOutLength);
#endif

    if (transaction->dest == DEST_ID_FW) {
        transactionResult = hw_trans(DIRECTION_GET,
                                     transaction->iid,
                                     hw,
                                     transaction->inPayload,
                                     transaction->inLength,
                                     transaction->outPayload,
                                     transaction->maxOutLength,
                                     &actualResultBytes);
    }
    else if (transaction->dest == DEST_ID_HW) {
        switch(transaction->iid) {

        case HW_SYS_TIME:
            transactionResult =
                get_hw_sys_time(hw, transaction->outPayload,
                                transaction->maxOutLength,
                                &actualResultBytes);
            break;

        case HW_SEC_TIME:
            transactionResult =
                get_hw_sec_time(hw, transaction->outPayload,
                                transaction->maxOutLength,
                                &actualResultBytes);
            break;

        case HW_TMSTMP_EN:
            transactionResult =
                get_hw_timestamp_enabled(hw, transaction->outPayload,
                                         transaction->maxOutLength,
                                         &actualResultBytes);
            break;

        case HW_TMSTMP_CNT:
            transactionResult =
                get_hw_timestamp_count(hw,
                                       transaction->inPayload,
                                       transaction->inLength,
                                       transaction->outPayload,
                                       transaction->maxOutLength,
                                       &actualResultBytes);
            break;

        case HW_TMSTMP_DATA:
            transactionResult =
                get_hw_timestamp_data(hw,
                                      transaction->inPayload,
                                      transaction->inLength,
                                      transaction->outPayload,
                                      transaction->maxOutLength,
                                      &actualResultBytes);
            break;

        case HW_TMSTMP_SINGLE:
            transactionResult =
                get_hw_timestamp_single(hw,
                                        transaction->inPayload,
                                        transaction->inLength,
                                        transaction->outPayload,
                                        transaction->maxOutLength,
                                        &actualResultBytes);
            break;

        case HW_GPO_MTCH_HI:
            transactionResult =
                get_hw_gpo_match(MATCH_HI,
                                 hw,
                                 transaction->inPayload,
                                 transaction->inLength,
                                 transaction->outPayload,
                                 transaction->maxOutLength,
                                 &actualResultBytes);
            break;

        case HW_GPO_MTCH_LO:
            transactionResult =
                get_hw_gpo_match(MATCH_LO,
                                 hw,
                                 transaction->inPayload,
                                 transaction->inLength,
                                 transaction->outPayload,
                                 transaction->maxOutLength,
                                 &actualResultBytes);
            break;

        case HW_FPGA_ID:
            transactionResult =
                get_hw_fpga_id(hw, transaction->outPayload,
                               transaction->maxOutLength,
                               &actualResultBytes);
            break;

        case HW_INT_MASK:
            transactionResult =
                get_hw_int_mask(hw,
                                transaction->inPayload,
                                transaction->inLength,
                                transaction->outPayload,
                                transaction->maxOutLength,
                                &actualResultBytes);
            break;

        case HW_INT_CNT:
            transactionResult =
                get_hw_int_cnt(hw,
                                transaction->inPayload,
                                transaction->inLength,
                                transaction->outPayload,
                                transaction->maxOutLength,
                                &actualResultBytes);
            break;

        case HW_INT_TS:
            transactionResult =
                get_hw_int_ts(hw,
                              transaction->inPayload,
                              transaction->inLength,
                              transaction->outPayload,
                              transaction->maxOutLength,
                              &actualResultBytes);
            break;

        case HW_KTS_TEMPERATURE:
            transactionResult =
                get_hw_temperature(hw,
                              transaction->outPayload,
                              transaction->maxOutLength,
                              &actualResultBytes);
            break;

        case HW_KTS_TEMPERATURE_SS:
            transactionResult =
                get_hw_temperature_ss(hw,
                              transaction->outPayload,
                              transaction->maxOutLength,
                              &actualResultBytes);
            break;

        default:
            transactionResult = TSYNC_DRV_INVALID_INDEX;
            actualResultBytes = 0;
            break;

        };
    }
    else if (transaction->dest == DEST_ID_HW_NONKTS) {
        transactionResult = hw_nonkts_get_trans(transaction->iid,
                                                hw,
                                                transaction->inPayload,
                                                transaction->inLength,
                                                transaction->outPayload,
                                                transaction->maxOutLength,
                                                &actualResultBytes);
    }

    transaction->status = transactionResult;
    transaction->actualOutLength = actualResultBytes;

#if LOG_SET_AND_GET
    TPRO_PRINT("  actual out length: %u, result %s\n",
               actualResultBytes, tsync_strerror(transactionResult));
#endif
    return TSYNC_SUCCESS;
}

TSYNC_ERROR hw_handle_set(TPRO_INSTANCE_T *hw,
                          ioctl_trans_hw *transaction) {

    TSYNC_ERROR transactionResult = TSYNC_DRV_INVALID_INDEX;
    uint32_t actualResultBytes = 0;

    /*****************************************
     * What is the transaction?
     */

#if LOG_SET_AND_GET
    TPRO_PRINT("Dest: %04x, iid %04x.  inLength %u, maxOutLength %u.\n",
               transaction->dest, transaction->iid,
               transaction->inLength,
               transaction->maxOutLength);
#endif

    if (transaction->dest == DEST_ID_FW) {
        transactionResult = hw_trans(DIRECTION_SET,
                                     transaction->iid,
                                     hw,
                                     transaction->inPayload,
                                     transaction->inLength,
                                     transaction->outPayload,
                                     transaction->maxOutLength,
                                     &actualResultBytes);
    }
    else if (transaction->dest == DEST_ID_HW) {
        switch(transaction->iid) {

        case HW_TMSTMP_EN:
            transactionResult =
                set_hw_timestamp_enabled(hw, transaction->inPayload,
                                         transaction->inLength);
            actualResultBytes = 0;
            break;

        case HW_TMSTMP_REQ:
            transactionResult =
                set_hw_timestamp_req(hw);
            actualResultBytes = 0;
            break;

        case HW_TMSTMP_CLR:
            transactionResult =
                set_hw_timestamp_clr(hw,
                                     transaction->inPayload,
                                     transaction->inLength);
            actualResultBytes = 0;
            break;

        case HW_GPO_MTCH_HI:
            transactionResult =
                set_hw_gpo_match(MATCH_HI,
                                 hw,
                                 transaction->inPayload,
                                 transaction->inLength);
            actualResultBytes = 0;
            break;
        case HW_GPO_MTCH_LO:
            transactionResult =
                set_hw_gpo_match(MATCH_LO,
                                 hw,
                                 transaction->inPayload,
                                 transaction->inLength);
            actualResultBytes = 0;
            break;

        case HW_INT_MASK:
            transactionResult =
                set_hw_int_mask(hw,
                                transaction->inPayload,
                                transaction->inLength);
            actualResultBytes = 0;
            break;

        case HW_INT_CNT_CLR:
            transactionResult =
                set_hw_int_cnt_clr(hw,
                                transaction->inPayload,
                                transaction->inLength);
            actualResultBytes = 0;
            break;

        default:
            transactionResult = TSYNC_DRV_INVALID_INDEX;
            actualResultBytes = 0;
            break;

        };
    }
    else if (transaction->dest == DEST_ID_HW_NONKTS) {
        transactionResult = hw_nonkts_set_trans(transaction->iid,
                                                hw,
                                                transaction->inPayload,
                                                transaction->inLength,
                                                transaction->outPayload,
                                                transaction->maxOutLength,
                                                &actualResultBytes);
    }

    transaction->status = transactionResult;
    transaction->actualOutLength = actualResultBytes;

#if LOG_SET_AND_GET
    TPRO_PRINT("  actual out length: %u, result %s\n",
               actualResultBytes, tsync_strerror(transactionResult));
#endif
    return TSYNC_SUCCESS;
}

uint16_t tsync_read_interrupt_status(TPRO_INSTANCE_T *hw) {
    uint16_t status;
    uint16_t statusMask;

    status = TSYNC_READ_REG(tsync_interrupt_status_offset);
    statusMask = TSYNC_READ_REG(tsync_interrupt_mask_offset);
    status &= ~statusMask;

    TSYNC_WRITE_REG(status, tsync_interrupt_status_offset);

    return status;
}


TSYNC_ERROR tsync_initialize_board(TPRO_INSTANCE_T *hw) {
    TSYNC_ERROR healthRequestResult;
    int iQueue;
    int iCheck, nChecks = 2;

    /* Clear out the FIFOs */
    tsyncFlushFifos(hw);

    // block all interrupts
    TSYNC_WRITE_REG(0xffff, tsync_interrupt_mask_offset);

    // clear all outstanding interrupts
    TSYNC_WRITE_REG(0xffff, tsync_interrupt_status_offset);

    for (iQueue = 0; iQueue < TMSTMP_SRC_COUNT; iQueue++) {
        gpioQueueInit(hw, iQueue);
    }

    /* Do a transaction health query to make sure we're in good shape.
     * Do a second transaction health query to make sure the board's
     * sequence number isn't stuck at the first value, potentially
     * confusing things on the next attach. */
    for (iCheck = 0; iCheck < nChecks; iCheck++) {
      healthRequestResult = tsync_check_board_connection_health(hw);
      if (healthRequestResult != TSYNC_SUCCESS) {
        TPRO_PRINT("tsync_check_board_connection_health attempt %u/%u returned %s. "
		   SRC_LOCATION_FORMAT_STRING,
                   iCheck, nChecks, tsync_strerror(healthRequestResult),
		   SRC_LOCATION_VARIABLES);
        return healthRequestResult;
      }
    }

    return TSYNC_SUCCESS;
}
