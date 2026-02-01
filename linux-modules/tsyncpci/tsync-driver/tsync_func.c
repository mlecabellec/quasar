/*******************************************************************************
**
**  Module  : tsync_func.c
**  Date    : 06/23/08
**  Purpose : This driver provides an interface to the TSync-PCIe
**
**  Copyright(C) 2008 Spectracom Corporation. All Rights Reserved.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
********************************************************************************
**
**     04/03/06 Updated from Linux (2.4.24) to Linux (2.6.11.12)
**
*******************************************************************************/
#include "os_wait.h"
#include "ddtsync.h"
#include "tsync_gpio_stamp_queue.h"
#include "binary_coded_decimal.h"
#include "tsync_types.h"
#include "tsync_comm_validation.h"
#include "tsync_comm_transaction.h"
#include "tsync_comm_fifo.h"
#include "tsync_hw_func.h"

#include "tpro_func.h"
#include "tsync_driver_helpers.h"

#include <linux/slab.h>

#define TSYNC_FUNC_MAX_BUFFER_WORDS 128
#define TSYNC_FUNC_SAT_INFO_MAX_BUFFER_WORDS (24 * 24 + 16)
#define TSYNC_SHORTS_PER_SUPER_SECOND 4
#define TSYNC_SHORTS_PER_SUB_SECOND 2
/*******************************************************************************
**          Public function pointer table
*******************************************************************************/
FuncTable_t tsyncpci_tsync =
{
    tsyncGetAltitude,
    tsyncGetDate,
    tsyncGetEvent,
    tsyncGetFirmware,
    tsyncInitializeBoard,
    tsyncGetLatitude,
    tsyncGetLongitude,
    tsyncGetSatInfo,
    tsyncGetTime,
    tsyncGetNtpTime,
    tsyncResetFirmware,
    tsyncSetHeartbeat,
    tsyncSetMatchTime,
    tsyncSetPropDelayCorr,
    tsyncSetTime,
    tsyncSetYear,
    tsyncSimEvent,
    tsyncSynchControl,
    tsyncSynchStatus,
    tsyncPeek,
    tsyncPoke,
    tsyncGetFpgaVersion,
    tsyncToggleInterrupt,
    tsyncGet,
    tsyncSet,
    tsyncWait,
    tsyncWaitTo,


    CMD_STAT_REG_TSYNC,
    FIFO_INT_CTRL_REG_32,
    CLR_FLAGS_REG_TSYNC
};


/*******************************************************************************
**          Private Function Prototypes
*******************************************************************************/
const int MICROSECONDS_PER_SECOND = 1000000;
const int NANOSEC_PER_MICROSEC = 1000;


const int maxTransactionBufferWords = MAX_TRANSACTION_BUFFER_WORDS;

/***************************************************************************
**          Private Function Definitions
***************************************************************************/
void logTransactionError(void *buffer, const char *transactionID) {
    uint32_t *bufferAs32 = (uint32_t*)buffer;

    uint32_t rawHARC, rawRC;
    TSYNC_ERROR harc, rc;

    rawHARC = BIG_END_32(bufferAs32[2]);
    harc = TSYNC_ERROR_FROM_CLASS_ERROR(EC_BOARD_RC, rawHARC);
    TPRO_PRINT("%s %s transaction failed:\n", transactionID,
        ((BIG_END_16(((uint16_t*)buffer)[1]) & 0x2) ? "set" : "get"));
    TPRO_PRINT("  HA rc: %u, %s\n", rawHARC, tsync_strerror(harc));
    if (BIG_END_32(bufferAs32[1]) > sizeof(uint32_t)) {
        rawRC = BIG_END_32(bufferAs32[3]);
        rc = TSYNC_ERROR_FROM_CLASS_ERROR(EC_BOARD_OPT, rawRC);
        TPRO_PRINT("     rc: %u, %s\n", rawRC, tsync_strerror(rc));
    }
}

uint8_t isLeapYear(unsigned int year) {

    /* Zero is a magical, invalid year which can only be achieved by
     * resetting the firmware. */
    if (year == 0) return 0;

    if (year % 400 == 0) return 1;
    if (year % 100 == 0) return 0;
    if (year % 4 == 0) return 1;

    return 0;
}

/*******************************************************************************
**          Public Function Definitions
*******************************************************************************/

/*******************************************************************************
**
** Function:    tproToggleInterrupt()
** Description: toggle an interrupt flag
**
** Parameters:
**     IN: *hw   - Handle
**         mask  -  interrupt mask to write to ICR register
**
**     RETURNS: (0) Success
**
*******************************************************************************/
#define SET_BIT(bitMask, whichBit) bitMask |= 1<<(whichBit);
#define CLEAR_BIT(bitMask, whichBit) bitMask &= ((1<< (whichBit)) ^ 0xffff);
#define IS_BIT_SET(bitMask, whichBit) ((bitMask & (1 << (whichBit))) != 0)

/* WARNING: the bits of the interrupt control port on the TPRO boards
 * have the opposite sense from the TSYNC Host Bus Interrupt Mask.
 * The TPRO considers a set bit to mean "the interrupt is allowed",
 * while the TSYNC considers a set bit to be "interrupt is
 * blocked." */
#define MAP_BIT(legacyMask, legacyWhichBit, tsyncMask, tsyncWhichBit)   \
 if (IS_BIT_SET(legacyMask, legacyWhichBit)) {                          \
    CLEAR_BIT(tsyncMask, tsyncWhichBit);                                \
 }                                                                      \
 else {                                                                 \
    SET_BIT(tsyncMask, tsyncWhichBit);                                 \
 }                                                                      \

unsigned char tsyncToggleInterrupt (tsyncpci_dev_t *hw, unsigned char mask)
{

    uint16_t oldMask = TSYNC_READ_REG(tsync_interrupt_mask_offset);
    uint16_t newMask = oldMask;

    // Legacy: Heartbeat.  TSync: GPIO Output 0
    MAP_BIT(mask, 5, newMask, 11);

    // Legacy: Match.  TSync: GPIO Output 1
    MAP_BIT(mask, 6, newMask, 12);

    // Legacy: FIFO Ready.  Tsync: Time stamp occurred
    MAP_BIT(mask, 7, newMask, 10);

    TSYNC_WRITE_REG(newMask, tsync_interrupt_mask_offset);
    return (0);
}

/*******************************************************************************
**
** Function:    tsyncGetAltitude()
** Description: Get altitude from the TPRO device
**
** Parameters:
**     IN: *hw   - Handle
**         *Altp - Pointer the Altitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetAltitude(tsyncpci_dev_t *hw, AltObj *Altp)
{

    return (1);

} /* End - tsyncGetAltitude() */


/*******************************************************************************
**
** Function:    tsyncGetDate()
** Description: Gets the date in Gregorian format from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Datep - Pointer to date result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetDate(tsyncpci_dev_t *hw, DateObj *Datep)
{
  return (1);
} /* End - tsyncGetDate() */


/*******************************************************************************
**
** Function:    tsyncGetEvent()
** Description: Reads the time-tagged event information from fifo.
**
** Parameters:
**     IN: *hw    - Handle
**         *Waitp - Pointer to event information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetEvent(tsyncpci_dev_t *hw, WaitObj *Waitp)
{
    TSYNC_ERROR queueStatus;
    gpio_timestamp_t timestamp;


    unsigned int wholeSeconds;

    uint32_t subSecondsNano;
    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;

    queueStatus = gpioQueueRemove(hw, TMSTMP_SRC_HOST_AND_GPI_0, &timestamp);
    if (queueStatus != TSYNC_SUCCESS) {
        TPRO_PRINT("gpioQueueRemove returned %s (%s:%u)\n",
                   tsync_strerror(queueStatus),
                   __FILE__, __LINE__);
        return 1;
    }

    bcdWholeSeconds = timestamp.superSecLow & 0xff;
    wholeSeconds = uintFromBCD(bcdWholeSeconds, 2);

    subSecondsNano = ((timestamp.subSecHigh << 16) | timestamp.subSecLow) * 5;

    Waitp->seconds =
        (wholeSeconds * MICROSECONDS_PER_SECOND) +
        (subSecondsNano/NANOSEC_PER_MICROSEC);

    bcdMinute = (timestamp.superSecLow >> 8) & 0xff;
    Waitp->minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = timestamp.superSecMidLow & 0xff;
    Waitp->hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((timestamp.superSecMidHigh & 0x0f) << 8) |
        ((timestamp.superSecMidLow>>8) & 0xff);
    Waitp->days = uintFromBCD(bcdDayOfYear, 3);

    return (0);
} /* End - tsyncGetEvent() */

/*******************************************************************************
**
** Function:    tsyncGetFirmware()
** Description: Get Firmware ID from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *firm - Pointer to firmware ID
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetFirmware(tsyncpci_dev_t *hw, unsigned char *firm)
{
    return 1;
} /* End - tsyncGetFirmware() */

/*******************************************************************************
**
** Function:    tsyncGetLatitude()
** Description: Get Latitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to latitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetLatitude(tsyncpci_dev_t *hw, LatObj *Latp)
{
    return (1);

} /* End - tsyncGetLatitude() */

/*******************************************************************************
**
** Function:    tsyncGetLongitude()
** Description: Get Longitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to longitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetLongitude(tsyncpci_dev_t *hw, LatObj *Longp)
{
    return (1);
} /* End - tsyncGetLongitutde() */

/*******************************************************************************
**
** Function:    tsyncInitializeBoard()
** Description: Initializes TPRO registers upon board creation.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncInitializeBoard(tsyncpci_dev_t *hw)
{
    int iInterrupt;

    TSYNC_ERROR initializationResult;

    /* heartbeat */
    os_waitInit(&hw->heart_wait);
    atomic_set(&hw->heartFlag, 0);

    /* match time */
    os_waitInit(&hw->match_wait);
    atomic_set(&hw->matchFlag, 0);

    /* external time tagged events */
    os_waitInit(&hw->event_wait);
    atomic_set(&hw->eventFlag, 0);

    for (iInterrupt = 0; iInterrupt < TSYNC_INTERRUPT_COUNT; iInterrupt++) {
        os_waitInit(&hw->tsyncInterrupts[iInterrupt].waitQueue);
        atomic_set(&hw->tsyncInterrupts[iInterrupt].flag, 0);
    }

    initializationResult = tsync_initialize_board(hw);

    return (initializationResult == TSYNC_SUCCESS) ? 0 : 1;

} /* End - tsyncInitializeBoard() */

/*******************************************************************************
**
** Function:    tsyncGetSatInfo()
** Description: Get satellite information from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Satp - Pointer to satellite
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetSatInfo(tsyncpci_dev_t *hw, SatObj *Satp)
{
    uint16_t *buffer;
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;
    uint32_t *bufferPos32;
    uint32_t nBytes;
    ioctl_trans localTransaction;
    int transactionResult;
    unsigned char ret = 0;

    buffer = kzalloc(TSYNC_FUNC_SAT_INFO_MAX_BUFFER_WORDS, GFP_KERNEL);
    if (!buffer) {
        TPRO_PRINT("%s: failed to allocate buffer\n", __func__);
        ret = 1;
        goto end;
    }

    /* The transaction: */
    bufferPos8 = (uint8_t*) buffer;
    *bufferPos8++ = 0x29; // GR identifier
    *bufferPos8++ = 0x06; // Fix data item

    bufferPos16 = (uint16_t*) bufferPos8;
    *bufferPos16++ = BIG_END_16(0x0); // Control: get

    bufferPos32 = (uint32_t*) bufferPos16;
    *bufferPos32++ = BIG_END_32(0); // payload length

    nBytes = (uintptr_t) bufferPos32 - (uintptr_t) buffer;

    localTransaction.inBuffer = (uint8_t*) buffer;
    localTransaction.inBufferLength = nBytes;
    localTransaction.outBuffer = (uint8_t*) buffer;
    localTransaction.maxOutBufferLength = sizeof(buffer);

    transactionResult = tsyncLocalTransaction(hw, &localTransaction);
    if (transactionResult != 0) {
        TPRO_PRINT("tsyncLocalTransaction returned %u (%s:%u)\n",
                   transactionResult, __FILE__, __LINE__);
        ret = 1;
        goto end;
    }

    if (BIG_END_16(buffer[1]) & 0x1) {
        logTransactionError(buffer, "GR_CA_FIX_DATA");
        ret = 1;
        goto end;
    }

    bufferPos32 = (uint32_t*) buffer;

    Satp->satsTracked = BIG_END_32(bufferPos32[2]);
    Satp->satsView = Satp->satsTracked;

end:
    kfree(buffer);
    return (ret);

} /* End - tsyncGetSatInfo() */

/*******************************************************************************
**
** Function:    tsyncGetTime()
** Description: Get Time information from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetTime(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    unsigned int wholeSeconds;

    uint32_t subSecondsNano;
    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;

    uint16_t superSec[TSYNC_SHORTS_PER_SUPER_SECOND];
    uint16_t nanoSubSec[TSYNC_SHORTS_PER_SUB_SECOND];

    /*****************************************
     * Get the data from the board
     */

    TSYNC_COPY_FROM_IO(superSec, tsync_super_second_offset,
                       TSYNC_SHORTS_PER_SUPER_SECOND);
    TSYNC_COPY_FROM_IO(nanoSubSec, tsync_nano_sub_second_offset,
                       TSYNC_SHORTS_PER_SUB_SECOND);

    /*****************************************
     * Fill out the time struct
     */

    bcdWholeSeconds = superSec[0] & 0xff;
    wholeSeconds = uintFromBCD(bcdWholeSeconds, 2);

    subSecondsNano = ((nanoSubSec[1] << 16) | nanoSubSec[0]) * 5;

    Timep->secsDouble =
        (wholeSeconds * MICROSECONDS_PER_SECOND) +
        (subSecondsNano/NANOSEC_PER_MICROSEC);
    Timep->seconds = wholeSeconds;

    bcdMinute = (superSec[0] >> 8) & 0xff;
    Timep->minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = superSec[1] & 0xff;
    Timep->hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((superSec[2] & 0x0f) << 8) |
        ((superSec[1]>>8) & 0xff);
    Timep->days = uintFromBCD(bcdDayOfYear, 3);


    return (0);

} /* End - tsyncGetTime() */

/*******************************************************************************
**
** Function:    tsyncGetNtpTime()
** Description: Get Time information from the TPRO device and from the linux
**              kernel.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetNtpTime(tsyncpci_dev_t *hw, NtpTimeObj *Timep)
{
    unsigned int wholeSeconds;
    uint32_t subSecondsNano;
    uint32_t bcdWholeSeconds;
    uint32_t bcdMinute, bcdHour;
    uint32_t bcdDayOfYear;
    uint32_t bcdYear;

    uint16_t superSec[TSYNC_SHORTS_PER_SUPER_SECOND];
    uint16_t nanoSubSec[TSYNC_SHORTS_PER_SUB_SECOND];

    /*
     * get time and convert to timeval. Note that this should be changed to
     * support year 2038's rollover
     */
    Timep->tv = ktime_to_timeval(ktime_get_real());

    /*****************************************
     * Get the data from the board
     */

    TSYNC_COPY_FROM_IO(superSec, tsync_super_second_offset,
                       TSYNC_SHORTS_PER_SUPER_SECOND);
    TSYNC_COPY_FROM_IO(nanoSubSec, tsync_nano_sub_second_offset,
                       TSYNC_SHORTS_PER_SUB_SECOND);

    /*****************************************
     * Fill out the time struct
     */

    bcdWholeSeconds = superSec[0] & 0xff;
    wholeSeconds = uintFromBCD(bcdWholeSeconds, 2);

    subSecondsNano = (((nanoSubSec[1] & 0xFFF) << 16) | nanoSubSec[0]) * 5;

    Timep->timeObj.flags = (nanoSubSec[1] & 0x8000) ? 4 : 0;

    Timep->timeObj.secsDouble =
        (wholeSeconds * MICROSECONDS_PER_SECOND) +
        (subSecondsNano/NANOSEC_PER_MICROSEC);
    Timep->timeObj.seconds = wholeSeconds;

    bcdMinute = (superSec[0] >> 8) & 0xff;
    Timep->timeObj.minutes = uintFromBCD(bcdMinute, 2);

    bcdHour = superSec[1] & 0xff;
    Timep->timeObj.hours = uintFromBCD(bcdHour, 2);

    bcdDayOfYear =
        ((superSec[2] & 0x0f) << 8) |
        ((superSec[1]>>8) & 0xff);
    Timep->timeObj.days = uintFromBCD(bcdDayOfYear, 3);

    bcdYear =
        ((superSec[3] & 0x0f) << 12) |
        ((superSec[2]>>4) & 0xfff);
    Timep->timeObj.year = uintFromBCD(bcdYear, 4);


    return (0);

} /* End - tsyncGetNtpTime() */

/*******************************************************************************
**
** Function:    tsyncGetFpgaVersion()
** Description: get the FPGA version from the board.
**
** Parameters:
**     IN: *hw    - Handle
**         *fpagVer - Pointer to the char array.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncGetFpgaVersion(tsyncpci_dev_t *hw, unsigned char *fpgaVer)
{
    return 1;
} /* End - tsyncGetFpgaVersion() */

/*******************************************************************************
**
** Function:    tsyncResetFirmware()
** Description: Reset Firmware on the TPRO device.
**
** Parameters:
**     IN: *hw - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncResetFirmware(tsyncpci_dev_t *hw) {

    uint16_t buffer[TSYNC_FUNC_MAX_BUFFER_WORDS];
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;
    uint32_t *bufferPos32;


    uint32_t nBytes;

    ioctl_trans localTransaction;
    int transactionResult;

    TPRO_PRINT("Resetting the firmware.\n");

    /* The transaction: */
    bufferPos8 = (uint8_t*) buffer;
    *bufferPos8++ = 0x25;
    *bufferPos8++ = 0x08;

    bufferPos16 = (uint16_t*) bufferPos8;
    *bufferPos16++ = BIG_END_16(0x2); // Control: set

    bufferPos32 = (uint32_t*) bufferPos16;
    *bufferPos32++ = BIG_END_32(sizeof(uint32_t)); // payload length

    *bufferPos32++ = 0; // SS_RESET_BRD

    nBytes = (uintptr_t) bufferPos32 - (uintptr_t) buffer;

    localTransaction.inBuffer = (uint8_t*) buffer;
    localTransaction.inBufferLength = nBytes;
    localTransaction.outBuffer = (uint8_t*) buffer;
    localTransaction.maxOutBufferLength = sizeof(buffer);

    transactionResult = tsyncLocalTransaction(hw, &localTransaction);
    if (transactionResult != 0) {
        TPRO_PRINT("tsyncLocalTransaction returned %u (%s:%u)\n",
                   transactionResult, __FILE__, __LINE__);
        return localTransaction.status;
    }

    if (BIG_END_16(buffer[1]) & 0x1) {
        logTransactionError(buffer, "SS_CA_RESET");
        return 1;
    }


    return (0);

} /* End - tsyncResetfirmware() */

/*******************************************************************************
**
** Function:    tsyncSetHeartbeat()
** Description: Configures the heartbeat output on the TPRO device.
**
** Parameters:
**     IN: *hw     - Handle
**         *Heartp - Pointer to heartbeat information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSetHeartbeat(tsyncpci_dev_t *hw, HeartObj *Heartp)
{
    uint16_t buffer[TSYNC_FUNC_MAX_BUFFER_WORDS];
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;
    uint32_t *bufferPos32;

    uint32_t nBytes;

    ioctl_trans localTransaction;
    int transactionResult;

    uint32_t us3Period = Heartp->divNumber;
    uint32_t nsPeriod, nsPulseWidth;

    nsPeriod = (us3Period/3) * NANOSEC_PER_MICROSEC;
    nsPulseWidth = nsPeriod / 2;

    /* The transaction: */
    bufferPos8 = (uint8_t*) buffer;
    *bufferPos8++ = 0x39; // GO identifier
    *bufferPos8++ = 0x07; // Square Wave command

    bufferPos16 = (uint16_t*) bufferPos8;
    *bufferPos16++ = BIG_END_16(0x2); // Control: set

    bufferPos32 = (uint32_t*) bufferPos16;
    *bufferPos32++ = BIG_END_32(20); // payload length

    *bufferPos32++ = 0; // GPO Index
    *bufferPos32++ = 0; // GO_SQUARE.offset
    *bufferPos32++ = BIG_END_32(nsPeriod); // GO_SQUARE.per
    *bufferPos32++ = BIG_END_32(nsPulseWidth);
    *bufferPos32++ = 0; // EDGE_FALLING

    nBytes = (uintptr_t) bufferPos32 - (uintptr_t) buffer;


    localTransaction.inBuffer = (uint8_t*) buffer;
    localTransaction.inBufferLength = nBytes;
    localTransaction.outBuffer = (uint8_t*) buffer;
    localTransaction.maxOutBufferLength = sizeof(buffer);

    transactionResult = tsyncLocalTransaction(hw, &localTransaction);
    if (transactionResult != 0) {
        TPRO_PRINT("tsyncLocalTransaction returned %u (%s:%u)\n",
                   transactionResult, __FILE__, __LINE__);
        return 1;
    }

    if (BIG_END_16(buffer[1]) & 0x1) {
        logTransactionError(buffer, "GO_SQUARE");
        return 1;
    }

    return (0);

} /* End - tsyncSetHeartbeat() */

/*******************************************************************************
**
** Function:    tsyncSetMatchTime()
** Description: Configures the match start/stop time feature.
**
** Parameters:
**     IN: *hw     - Handle
**         *Matchp - Pointer to start/stop information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSetMatchTime(tsyncpci_dev_t *hw, MatchObj *Matchp)
{
    unsigned int matchOffset;

    unsigned int wholeSeconds, microSeconds, fiveNanos;
    unsigned int bcdWholeSeconds, bcdMinutes, bcdHours, bcdDays;

    uint16_t subSecLow, subSecHigh;
    uint16_t superSecLow, superSecMid, superSecHigh;

    switch(Matchp->matchType) {
    case 0:
        matchOffset = tsync_gpio_output_1_high_match_offset;
        break;

    case 1:
        matchOffset = tsync_gpio_output_1_low_match_offset;
        break;

    default:
        return 1;
    }

    wholeSeconds = Matchp->seconds / MICROSECONDS_PER_SECOND;
    microSeconds = Matchp->seconds % MICROSECONDS_PER_SECOND;
    fiveNanos = microSeconds * NANOSEC_PER_MICROSEC / 5;

    subSecHigh = fiveNanos >> 16;
    subSecLow = fiveNanos & 0xffff;

    bcdWholeSeconds = bcdFromUint(wholeSeconds);
    bcdMinutes = bcdFromUint(Matchp->minutes);
    bcdHours = bcdFromUint(Matchp->hours);
    bcdDays = bcdFromUint(Matchp->days);

    superSecLow = (bcdMinutes << 8) | bcdWholeSeconds;
    superSecMid = ((bcdDays & 0xff) << 8) | bcdHours;
    superSecHigh = bcdDays >> 8;

    TSYNC_WRITE_REG(subSecLow, matchOffset + (sizeof(uint16_t) * 0));
    TSYNC_WRITE_REG(subSecHigh, matchOffset + (sizeof(uint16_t) * 1));
    TSYNC_WRITE_REG(superSecLow, matchOffset + (sizeof(uint16_t) * 2));
    TSYNC_WRITE_REG(superSecMid, matchOffset + (sizeof(uint16_t) * 3));
    TSYNC_WRITE_REG(superSecHigh, matchOffset + (sizeof(uint16_t) * 4));

    return (0);
} /* End - tsyncSetMatchTime() */

/*******************************************************************************
**
** Function:    tsyncSetPropDelayCorr()
** Description: Sets the propagation delay correction factor.
**
** Parameters:
**     IN: *hw           - Handle
**         *microseconds - Pointer to propagation delay correction
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSetPropDelayCorr(tsyncpci_dev_t *hw, int *microseconds)
{
    return 1;
} /* End - tsyncSetPropDelayCorr() */

/*******************************************************************************
**
** Function:    tsyncSetTime()
** Description: Sets the TPRO device's IRIG-B generator to a given time.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSetTime(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    return (1);
} /* End - tsyncSetTime() */

/*******************************************************************************
**
** Function:    tsyncSetYear()
** Description: Sets the year on the TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *year - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSetYear(tsyncpci_dev_t *hw, unsigned short *year) {
    return (1);

} /* End - tsyncSetYear() */

/*******************************************************************************
**
** Function:    tsyncSimEvent()
** Description: Routine will simulate external time tag event.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSimEvent(tsyncpci_dev_t *hw)
{


    TSYNC_WRITE_REG(0x3, tsync_timestamp_status_offset);

    return (0);

} /* End - tsyncSimEvent() */

/*******************************************************************************
**
** Function:    tsyncSynchControl()
** Description: Routine determines whether to synchronize the on-board generator
**              to the TPRO's input or to disable synchronization.
**
** Parameters:
**     IN: *hw   - Handle
**         *enbp - (1) to enable synchronization
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSynchControl(tsyncpci_dev_t *hw, unsigned char *enbp)
{
    return 1;
} /* End - tsyncSynchControl() */

/*******************************************************************************
**
** Function:    tsyncSynchStatus()
** Description: Routine returns the state of the Synch Flag.
**
** Parameters:
**     IN: *hw   - Handle
**         *enbp - (1) to enable synchronization
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncSynchStatus(tsyncpci_dev_t *hw, unsigned char *stat)
{
    uint16_t buffer[TSYNC_FUNC_MAX_BUFFER_WORDS];
    uint8_t *bufferPos8;
    uint16_t *bufferPos16;
    uint32_t *bufferPos32;

    uint32_t nBytes;

    ioctl_trans localTransaction;
    int localTransactionResult;

    /* The transaction: */
    bufferPos8 = (uint8_t*) buffer;
    *bufferPos8++ = 0x25; // SS identifier
    *bufferPos8++ = 0x03; // Sync item

    bufferPos16 = (uint16_t*) bufferPos8;
    *bufferPos16++ = BIG_END_16(0x0); // Control: get

    bufferPos32 = (uint32_t*) bufferPos16;
    *bufferPos32++ = BIG_END_32(0); // payload length

    nBytes = (uintptr_t) bufferPos32 - (uintptr_t) buffer;

    localTransaction.inBuffer = (uint8_t*) buffer;
    localTransaction.inBufferLength = nBytes;
    localTransaction.outBuffer = (uint8_t*) buffer;
    localTransaction.maxOutBufferLength = sizeof(buffer);

    localTransactionResult = tsyncLocalTransaction(hw, &localTransaction);
    if (localTransactionResult != 0) {
        TPRO_PRINT("tsyncLocalTransaction returned %u, error %u (%s:%u)\n",
                   localTransactionResult, localTransaction.status,
                   __FILE__, __LINE__);
        return 1;
    }

    if (BIG_END_16(buffer[1]) & 0x1) {
        logTransactionError(buffer, "SS_CA_SYNC");
        return 1;
    }

    *stat = BIG_END_16(buffer[5]);

    return (0);

} /* End - tsyncSynchStatus() */

/*******************************************************************************
**
** Function:    tsyncPeek()
** Description: Routine returns the long word value from the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncPeek (tsyncpci_dev_t *hw, MemObj *Mem)
{
    Mem->value = TSYNC_READ_REG(Mem->offset);
    Mem->l_value = Mem->value;

    return (0);
} /* End - tsyncPeek() */

/*******************************************************************************
**
** Function:    tsyncPoke()
** Description: Routine pokes the value into the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tsyncPoke (tsyncpci_dev_t *hw, MemObj *Mem)
{
    TSYNC_WRITE_REG(Mem->value, Mem->offset);

    return (0);
} /* End - tsyncPoke() */

/******************************************************************************
**
** Function:    tsyncGet()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
******************************************************************************/
unsigned char tsyncGet (tsyncpci_dev_t *hw, int *ioctlArg) {

    uint8_t *inputBuffer = hw->localHWRequestBuffer;
    uint8_t *outputBuffer = hw->localHWResponseBuffer;

    uint32_t maxResultBytes, actualResultBytes;

    TSYNC_ERROR transactionResult = TSYNC_SUCCESS;

    ioctl_trans_di diTransaction;
    ioctl_trans_hw localTransaction;

    caddr_t userInPayloadSrc, userOutPayloadDst;
    unsigned int copyLength;

    /********************************************************
     * copy the user-space DI transaction fields, but not payload, to
     * driver space.
     */

    copyLength = sizeof(diTransaction) - DI_PAYLOADS_STARTER_LENGTH;
    if (!TSYNC_ACCESS_OK(VERIFY_READ, (void*)ioctlArg, copyLength )) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_from_user(&diTransaction, ioctlArg, copyLength);

    /********************************************************
     * copy the user-space input payload to driver space.
     */

    if (diTransaction.inLength > MAX_HW_REQUEST_LENGTH) {
        TPRO_PRINT("too much data to read in: %u vs. %zu (%s:%u)\n",
                   diTransaction.inLength,
                   MAX_HW_REQUEST_LENGTH * sizeof(uint16_t),
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_BUFFER_OVERFLOW;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    maxResultBytes = MAX_HW_RESPONSE_LENGTH;
    if (diTransaction.maxOutLength < MAX_HW_RESPONSE_LENGTH) {
        maxResultBytes = diTransaction.maxOutLength;
    }

    userInPayloadSrc = (caddr_t)ioctlArg;
    userInPayloadSrc += offsetof(ioctl_trans_di, payloads);
    userInPayloadSrc += diTransaction.inPayloadOffset;

    copyLength = diTransaction.inLength;

    if (!TSYNC_ACCESS_OK(VERIFY_READ, (void*)userInPayloadSrc, copyLength )) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_from_user(inputBuffer, userInPayloadSrc, copyLength);

    /***************************************************************
     * Fill in localTransaction and invoke the handler
     */

    localTransaction.dest = diTransaction.dest;
    localTransaction.iid = diTransaction.iid;
    localTransaction.inPayload = inputBuffer;
    localTransaction.inLength = diTransaction.inLength;
    localTransaction.outPayload = outputBuffer;
    localTransaction.maxOutLength = maxResultBytes;

    transactionResult = hw_handle_get(hw, &localTransaction);
    actualResultBytes = localTransaction.actualOutLength;


    /*************************************************************
     * Copy response payload back to user
     */
    if (actualResultBytes > maxResultBytes) {
        TPRO_PRINT("output overflow: %u vs. %u (%s:%u)\n",
                   actualResultBytes, maxResultBytes,
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_BUFFER_OVERFLOW;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    userOutPayloadDst = (caddr_t)ioctlArg;
    userOutPayloadDst += offsetof(ioctl_trans_di, payloads);
    userOutPayloadDst += diTransaction.outPayloadOffset;

    if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                   (void*)userOutPayloadDst,
                   actualResultBytes)) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   actualResultBytes,
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_to_user(userOutPayloadDst, outputBuffer,
                 actualResultBytes);

    diTransaction.actualOutLength = actualResultBytes;
    diTransaction.status = localTransaction.status;

 fxnExit:
    /*************************************************************
     * We always try to copy the ioctl_trans_di back so the library
     * and application at least have diTransaction.status and
     * diTransaction.actualOutLength.
     */
    copyLength = sizeof(diTransaction) - DI_PAYLOADS_STARTER_LENGTH;
    if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                   (void*)ioctlArg,
                   copyLength)) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        return 1;
    }

    copyLength = copy_to_user(ioctlArg, &diTransaction, copyLength);

    return 0;
} /* End - tsyncGet */

/*****************************************************************************
**
** Function:    tsyncSet()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*****************************************************************************/
unsigned char tsyncSet (tsyncpci_dev_t *hw, int *ioctlArg) {

    uint8_t *inputBuffer = hw->localHWRequestBuffer;
    uint8_t *outputBuffer = hw->localHWResponseBuffer;

    uint32_t maxResultBytes, actualResultBytes;

    TSYNC_ERROR transactionResult = TSYNC_SUCCESS;

    ioctl_trans_di diTransaction;
    ioctl_trans_hw localTransaction;

    caddr_t userInPayloadSrc, userOutPayloadDst;
    unsigned int copyLength;

    /********************************************************
     * copy the user-space DI transaction fields, but not payload, to
     * driver space.
     */

    copyLength = sizeof(diTransaction) - DI_PAYLOADS_STARTER_LENGTH;
    if (!TSYNC_ACCESS_OK(VERIFY_READ, (void*)ioctlArg, copyLength )) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_from_user(&diTransaction, ioctlArg, copyLength);

    /********************************************************
     * copy the user-space input payload to driver space.
     */

    if (diTransaction.inLength > MAX_HW_REQUEST_LENGTH) {
        TPRO_PRINT("too much data to read in: %u vs. %zu (%s:%u)\n",
                   diTransaction.inLength,
                   MAX_HW_REQUEST_LENGTH * sizeof(uint16_t),
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_BUFFER_OVERFLOW;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    maxResultBytes = MAX_HW_RESPONSE_LENGTH;
    if (diTransaction.maxOutLength < MAX_HW_RESPONSE_LENGTH) {
        maxResultBytes = diTransaction.maxOutLength;
    }

    userInPayloadSrc = (caddr_t)ioctlArg;
    userInPayloadSrc += offsetof(ioctl_trans_di, payloads);
    userInPayloadSrc += diTransaction.inPayloadOffset;

    copyLength = diTransaction.inLength;

    if (!TSYNC_ACCESS_OK(VERIFY_READ, (void*)userInPayloadSrc, copyLength )) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_from_user(inputBuffer, userInPayloadSrc, copyLength);

    /***************************************************************
     * Fill in localTransaction and invoke the handler
     */

    localTransaction.dest = diTransaction.dest;
    localTransaction.iid = diTransaction.iid;
    localTransaction.inPayload = inputBuffer;
    localTransaction.inLength = diTransaction.inLength;
    localTransaction.outPayload = outputBuffer;
    localTransaction.maxOutLength = maxResultBytes;

    transactionResult = hw_handle_set(hw, &localTransaction);
    actualResultBytes = localTransaction.actualOutLength;


    /*************************************************************
     * Copy response payload back to user
     */
    if (actualResultBytes > maxResultBytes) {
        TPRO_PRINT("output overflow: %u vs. %u (%s:%u)\n",
                   actualResultBytes, maxResultBytes,
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_BUFFER_OVERFLOW;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    userOutPayloadDst = (caddr_t)ioctlArg;
    userOutPayloadDst += offsetof(ioctl_trans_di, payloads);
    userOutPayloadDst += diTransaction.outPayloadOffset;

    if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                   (void*)userOutPayloadDst,
                   actualResultBytes)) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   actualResultBytes,
                   __FILE__, __LINE__);
        diTransaction.status = TSYNC_DRV_USERSPACE_ACCESS;
        diTransaction.actualOutLength = 0;
        goto fxnExit;
    }

    copyLength = copy_to_user(userOutPayloadDst, outputBuffer,
                 actualResultBytes);

    diTransaction.actualOutLength = actualResultBytes;
    diTransaction.status = localTransaction.status;

 fxnExit:
    /*************************************************************
     * We always try to copy the ioctl_trans_di back so the library
     * and application at least have diTransaction.status and
     * diTransaction.actualOutLength.
     */
    copyLength = sizeof(diTransaction) - DI_PAYLOADS_STARTER_LENGTH;
    if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                   (void*)ioctlArg,
                   copyLength)) {
        TPRO_PRINT("Access not OK for %u bytes. (%s:%u)\n",
                   copyLength, __FILE__, __LINE__);
        return 1;
    }

    copyLength = copy_to_user(ioctlArg, &diTransaction, copyLength);

    return 0;
} /* End - tsyncSet */

/***************************************************************************
**
** Function:    tsyncWait()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
***************************************************************************/
unsigned char tsyncWait (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction) {

    int interruptIndex;

    interrupt_context_t *context;
    unsigned char status = 0;
    int waitResult;
    TSYNC_ERROR error;

    transaction->status = TSYNC_SUCCESS;

    error =
        intBitFromTypeAndIndex(transaction->intType,
                               transaction->index,
                               &interruptIndex);
    if (error != TSYNC_SUCCESS) {
        TPRO_PRINT("intBitFromTypeAndIndex returned %s (%s:%u)\n",
                   tsync_strerror(error), __FILE__, __LINE__);
        transaction->status = error;
        return -1;
    }

    context = &hw->tsyncInterrupts[interruptIndex];

    TPRO_PRINT("Waiting on int %u . . . \n", interruptIndex);

    /* clear flag condition */
    atomic_set (&context->flag, 0);

    /* wait on semaphore */
    waitResult = os_waitPend(&context->waitQueue,
                         100000,
                         &context->flag);

    /* return on timeout or waitaphore error */
    if (waitResult == OS_WAIT_STATUS_TIMEOUT) {
        TPRO_PRINT("  timeout\n");
        transaction->status = TSYNC_DRV_INT_WAIT_TIMEOUT;
    }

    if (waitResult == OS_WAIT_STATUS_NG) {
        TPRO_PRINT("  NG\n");
        transaction->status = TSYNC_DRV_INT_WAIT_ERROR;
        status = -1;
    }

    TPRO_PRINT("Done waiting on int %u.\n", interruptIndex);
    return status;
}

/***************************************************************************
**
** Function:    tsyncWaitTo()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
***************************************************************************/
unsigned char tsyncWaitTo (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction) {

    int interruptIndex;

    interrupt_context_t *context;
    unsigned char status = 0;
    int waitResult;
    int timeout;
    TSYNC_ERROR error;

    transaction->status = TSYNC_SUCCESS;

    timeout = transaction->to;

    error =
        intBitFromTypeAndIndex(transaction->intType,
                               transaction->index,
                               &interruptIndex);
    if (error != TSYNC_SUCCESS) {
        TPRO_PRINT("intBitFromTypeAndIndex returned %s (%s:%u)\n",
                   tsync_strerror(error), __FILE__, __LINE__);
        transaction->status = error;
        return -1;
    }

    context = &hw->tsyncInterrupts[interruptIndex];

    TPRO_PRINT("Waiting on int %u . . . \n", interruptIndex);

    /* clear flag condition */
    atomic_set (&context->flag, 0);

    /* wait on semaphore */
    waitResult = os_waitPend(&context->waitQueue,
                         timeout,
                         &context->flag);

    /* return on timeout or waitaphore error */
    if (waitResult == OS_WAIT_STATUS_TIMEOUT) {
        TPRO_PRINT("  timeout\n");
        transaction->status = TSYNC_DRV_INT_WAIT_TIMEOUT;
    }

    if (waitResult == OS_WAIT_STATUS_NG) {
        TPRO_PRINT("  NG\n");
        transaction->status = TSYNC_DRV_INT_WAIT_ERROR;
        status = -1;
    }

    TPRO_PRINT("Done waiting on int %u.\n", interruptIndex);
    return status;
}
