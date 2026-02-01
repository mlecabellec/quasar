/*******************************************************************************
**
**  Module  : tpro_func32.c
**  Date    : 02/04/08
**  Purpose : This driver provides an interface to the TPRO-PCI
**
**  Copyright(C) 2006 Spectracom Corporation. All Rights Reserved.
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
#include <asm/delay.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <asm/io.h>

#include "ddtpro.h"
#include "tsyncpci.h"
#include "tpro_func.h"
#include "os_wait.h"
#include "binary_coded_decimal.h"

/*******************************************************************************
**          Public function pointer table
*******************************************************************************/
FuncTable_t tsyncpci_32bit =
{
    tproGetAltitude32,
    tproGetDate32,
    tproGetEvent32,
    tproGetFirmware32,
    tproInitializeBoard32,
    tproGetLatitude32,
    tproGetLongitude32,
    tproGetSatInfo32,
    tproGetTime32,
    tproGetNtpTime32,
    tproResetFirmware32,
    tproSetHeartbeat32,
    tproSetMatchTime32,
    tproSetPropDelayCorr32,
    tproSetTime32,
    tproSetYear32,
    tproSimEvent32,
    tproSynchControl32,
    tproSynchStatus32,
    tproPeek32,
    tproPoke32,
    tproGetFpgaVersion32,
    tproToggleInterrupt32,
    tproGet32,
    tproSet32,
    tproWait32,
    tproWaitTo32,


    CMD_STAT_REG_32,
    FIFO_INT_CTRL_REG_32,
    CLR_FLAGS_REG_32
};


/*******************************************************************************
**          Private Funtion Prototypes
*******************************************************************************/
unsigned char flushFIFO32     (unsigned short base);
unsigned char readFIFO32      (unsigned short  base,
                               unsigned int  *response,
                               unsigned char   numWords);
void          sendCmd32       (unsigned short base, unsigned char cmd);
void          sendDays32      (unsigned short base, unsigned char *str);
void          sendHours32     (unsigned short base, unsigned char *str);
void          sendMatchSecs32 (unsigned short base, unsigned char *str);
void          sendMinutes32   (unsigned short base, unsigned char *str);
void          sendProp32      (unsigned short base, unsigned char *str);
void          sendSeconds32   (unsigned short base, unsigned char *str);

/***************************************************************************
**          Private Function Definitions
***************************************************************************/

/*******************************************************************************
** Function: flushFIFO32
*******************************************************************************/
unsigned char flushFIFO32 (unsigned short base)
{
    /* Check to see if there is data in the fifo */
    if (!(PEEK_REG_32(base, CMD_STAT_REG_32) & 0x1)) {
        /* flush fifo */
        while (1) {
            volatile unsigned int fifo;

            fifo = PEEK_REG_32(base, FIFO_INT_CTRL_REG_32);

            /* check to see if FIFO is empty */
            if (fifo = PEEK_REG_32(base, CMD_STAT_REG_32), fifo & 0x1) break;
        }
    }

    return (0);

} /* End - flushFIFO32() */

/*******************************************************************************
** Function: readFIFO32
*******************************************************************************/
unsigned char readFIFO32(unsigned short  base,
                         unsigned int  *response,
                         unsigned char   numWords)
{
    while (numWords--) {
        /* wait for data to be ready */
        while ( (PEEK_REG_32 (base, CMD_STAT_REG_32) & 0x1) == 1) {
            udelay (1);
        }

        /* read word */
        *response++ = PEEK_REG_32(base, FIFO_INT_CTRL_REG_32) & 0xFF;
    }

    return (0);

} /* End - readFIFO32() */

/*******************************************************************************
** Function: sendCmd32
*******************************************************************************/
void sendCmd32(unsigned short base, unsigned char cmd)
{
    /* Send command to command port */
    POKE_REG_32 (base, CMD_STAT_REG_32, cmd);

    /*  Wait (delay) for at least 100uS */
    udelay (100);

} // End - sendCmd32() */

/*******************************************************************************
** Function: sendDays32
*******************************************************************************/
void sendDays32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x50, 0x60, 0x70};
    unsigned char k;

    /* Send set day commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendDays32() */

/*******************************************************************************
** Function: sendHours32
*******************************************************************************/
void sendHours32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x80, 0x90};
    unsigned char k;

    /* Send set hour commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendHours32() */

/*******************************************************************************
** Function: sendMatchSecs32
*******************************************************************************/
void sendMatchSecs32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xC0, 0xD0, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0};
    unsigned char k;

    /* Send set match second commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        if (k == 2) sendCmd32 (base, 0xE1);  /* send days-seconds */
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendMatchSecs32() */

/*******************************************************************************
** Function: sendMinutes32
*******************************************************************************/
void sendMinutes32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xA0, 0xB0};
    unsigned char k;

    /* Send set min commands */

    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendMinutes32() */

/*******************************************************************************
** Function: sendProp32
*******************************************************************************/
void sendProp32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x30, 0x20, 0x10, 0x00};
    unsigned char k;

    /* Send set propagation commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendProp32() */

/*******************************************************************************
** Function: sendSeconds32
*******************************************************************************/
void sendSeconds32(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xC0, 0xD0};
    unsigned char k;

    /* Send set second commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd32 (base, cmdVec[k] | (*str++));
    }

} /* End - sendSeconds32() */

/*******************************************************************************
**          Public Function Definitions
*******************************************************************************/

/*******************************************************************************
**
** Function:    tproToggleInterrupt32()
** Description: toggle an interrupt flag
**
** Parameters:
**     IN: *hw   - Handle
**         mask  -  interrupt mask to write to ICR register
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproToggleInterrupt32 (tsyncpci_dev_t *hw, unsigned char mask)
{
    POKE_REG_32(hw->ioAddr, FIFO_INT_CTRL_REG_32, mask);
    return (0);
}

/*******************************************************************************
**
** Function:    tproGetAltitude32()
** Description: Get altitude from the TPRO device
**
** Parameters:
**     IN: *hw   - Handle
**         *Altp - Pointer the Altitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetAltitude32(tsyncpci_dev_t *hw, AltObj *Altp)
{
    unsigned int responseVec[FIFO_RESPONSE_LENGTH];

    /* flush fifo */
    flushFIFO32(hw->ioAddr);

    /* send the get altitude command */
    sendCmd32(hw->ioAddr, 0x5D);

    /* get response from card */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* verify it is the altitude response */
    if ((responseVec[0] & 0xFF) != 0x5D ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5D ) return (1);

    /* calculate meters field */
    Altp->meters = uintFromBCDArray32HighLSN(&responseVec[3], 7);

    return (0);

} /* End - tproGetAltitude32() */

/*******************************************************************************
**
** Function:    tproGetDate32()
** Description: Gets the date in Gregorian format from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Datep - Pointer to date result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetDate32(tsyncpci_dev_t *hw, DateObj *Datep)
{
  unsigned int responseVec [FIFO_RESPONSE_LENGTH];

  /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
  flushFIFO32(hw->ioAddr);

  /* send get date command */
  sendCmd32(hw->ioAddr, 0x5D);

  /* verify valid data in fifo */
  while ((PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32) & 0x1));

  /* get response from card */
  readFIFO32(hw->ioAddr, responseVec, 10);

  /* verify its the get date response */
  if ( (responseVec[0] & 0xFF) != 0x5D ) return (1);
  if ( (responseVec[1] & 0xFF) != 0x5D ) return (1);

  /* calculate year field */
  Datep->year = (unsigned short)((MID_LO_NIBBLE (responseVec[8]) * 1000) +
                                 (LO_NIBBLE     (responseVec[8]) * 100)  +
                                 (MID_LO_NIBBLE (responseVec[7]) * 10)   +
                                 (LO_NIBBLE     (responseVec[7]) * 1) );

  /* calculate month field */
  Datep->month = uintFromBCD(responseVec[9], 2);

  /* calculate day field */
  Datep->day   = uintFromBCD(responseVec[2], 2);

  return (0);

} /* End - tproGetDate32() */


/*******************************************************************************
**
** Function:    tproGetEvent32()
** Description: Reads the time-tagged event information from fifo.
**
** Parameters:
**     IN: *hw    - Handle
**         *Waitp - Pointer to event information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetEvent32(tsyncpci_dev_t *hw, WaitObj *Waitp)
{
    unsigned short *ptr = (unsigned short *) &hw->fifoVector [hw->tailIndex];

    Waitp->days = uintFromBCDArray16(&ptr[2], 3);
    Waitp->hours = uintFromBCD(ptr[4], 2);
    Waitp->minutes = uintFromBCD(ptr[5], 2);
    Waitp->seconds = uintFromBCDArray16(&ptr[6], 8);

    /* increment tail index */
    hw->tailIndex = ((hw->tailIndex + 10) % (sizeof(hw->fifoVector)/sizeof(hw->fifoVector[0])));

    return (0);

} /* End - tproGetEvent32() */

/*******************************************************************************
**
** Function:    tproGetFirmware32()
** Description: Get Firmware ID from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *firm - Pointer to firmware ID
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetFirmware32(tsyncpci_dev_t *hw, unsigned char *firm)
{
    unsigned int responseVec [FIFO_RESPONSE_LENGTH];
    unsigned char counter;

    volatile unsigned int val;

    /* flush fifo */
    flushFIFO32(hw->ioAddr);

    /* send cmd */
    POKE_REG_32(hw->ioAddr, CMD_STAT_REG_32, 0xE9);

    /* verify valid data in fifo */
    while (val = PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32), val & 0x1);

    /* get firmware */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* verify response */
    if (responseVec[0] != 0xE9) return (1);
    if (responseVec[1] != 0xE9) return (1);

    /* copy firmware id */
    for (counter=2; counter < 6; counter++) {
        *firm++ = (unsigned char) (responseVec[counter] & 0xFF);
    }

    return (0);

} /* End - tproGetFirmware32() */

/*******************************************************************************
**
** Function:    tproGetLatitude32()
** Description: Get Latitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to latitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetLatitude32(tsyncpci_dev_t *hw, LatObj *Latp)
{
    unsigned int responseVec [FIFO_RESPONSE_LENGTH];

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO32(hw->ioAddr);

    /* send get latitude command */
    sendCmd32(hw->ioAddr, 0x5F);

    /* get response from card */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* verify its the get latitude response */
    if ((responseVec[0] & 0xFF) != 0x5F ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5F ) return (1);

    Latp->degrees = uintFromBCDArray32(&responseVec[2], 3);
    Latp->minutes = uintFromBCDArray32(&responseVec[4], 6);

    return (0);

} /* End - tproGetLatitude32() */

/*******************************************************************************
**
** Function:    tproGetLongitude32()
** Description: Get Longitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to longitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetLongitude32(tsyncpci_dev_t *hw, LatObj *Longp)
{
    unsigned int responseVec [FIFO_RESPONSE_LENGTH];
    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO32(hw->ioAddr);

    /* send get longitude command */
    sendCmd32(hw->ioAddr, 0x5E);

    /* get response from card */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* verify its the get longitude response */
    if ((responseVec[0] & 0xFF) != 0x5E ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5E ) return (1);

    Longp->degrees = uintFromBCDArray32(&responseVec[2], 3);
    Longp->minutes = uintFromBCDArray32(&responseVec[4], 6);

    return (0);

} /* End - tproGetLongitutde32() */

/*******************************************************************************
**
** Function:    tproInitializeBoard32()
** Description: Initializes TPRO registers upon board creation.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproInitializeBoard32(tsyncpci_dev_t *hw)
{
    /*
    **  initialize semaphores
    */
    /* heartbeat */
    os_waitInit(&hw->heart_wait);
    atomic_set(&hw->heartFlag, 0);

    /* match time */
    os_waitInit(&hw->match_wait);
    atomic_set(&hw->matchFlag, 0);

    /* external time tagged events */
    os_waitInit(&hw->event_wait);
    atomic_set(&hw->eventFlag, 0);

    /* disable interrupts */
    POKE_REG_32(hw->ioAddr, FIFO_INT_CTRL_REG_32, 0x0);

    /* flush FIFO */
    flushFIFO32(hw->ioAddr);

    /* clear pending match and heartbeat flags */
    if (PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32) & 0x8)
        POKE_REG_32(hw->ioAddr, CLR_FLAGS_REG_32, 0x8);
    if (PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32) & 0x10)
        POKE_REG_32(hw->ioAddr, CLR_FLAGS_REG_32, 0x10);

    return (0);

} /* End - tproInitializeBoard32() */

/*******************************************************************************
**
** Function:    tproGetSatInfo32()
** Description: Get satellite information from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Satp - Pointer to satellite
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetSatInfo32(tsyncpci_dev_t *hw, SatObj *Satp)
{
    unsigned int responseVec [FIFO_RESPONSE_LENGTH];

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO32(hw->ioAddr);

    /* send get sattelite command */
    sendCmd32(hw->ioAddr, 0x5C);

    /* get response from card */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* verify its the get satellite response */
    if ((responseVec[0] & 0xFF) != 0x5C ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5C ) return (1);

    /* parse response */
    Satp->satsTracked = (unsigned char) (LO_NIBBLE (responseVec[7]));

    return (0);

} /* End - tproGetSatInfo32() */

/*******************************************************************************
**
** Function:    tproGetTime32()
** Description: Get Time information from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetTime32(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    unsigned int word, word2;

    /* Read Time */
    /* low register contains seconds -> uSeconds */
    word  = PEEK_REG_32(hw->ioAddr, TIME_REG_LOW_32);
    /* high register contains minutes, hours, days, flags */
    word2 = PEEK_REG_32(hw->ioAddr, TIME_REG_HIGH_32);

    /* Convert seconds */
    Timep->secsDouble = uintFromBCD(word, 8);

    /* Convert minutes */
    Timep->minutes = uintFromBCD(word2, 2);

    /* Convert hours */
    Timep->hours   = uintFromBCD(word2 >> 8, 2);

    /* Convert days */
    Timep->days = uintFromBCD(word2 >> 16, 3);

    Timep->flags = UPWORD_HI_NIBBLE(word2);

    return (0);

} /* End - tproGetTime32() */

/*******************************************************************************
**
** Function:    tproGetNtpTime32()
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
unsigned char tproGetNtpTime32(tsyncpci_dev_t *hw, NtpTimeObj *Timep)
{
    unsigned int responseVec[FIFO_RESPONSE_LENGTH];
    unsigned int word, word2, word3;

    /*
     * get time and convert to timeval. Note that this should be changed to
     * support year 2038's rollover
     */
    Timep->tv = ktime_to_timeval(ktime_get_real());

    /* Read seconds */
    word  = PEEK_REG_32(hw->ioAddr, CLK_MSECS_USECS_REG);
    word2 = PEEK_REG_32(hw->ioAddr, CLK_SECS_MSECS_REG);
    word3 = PEEK_REG_32(hw->ioAddr, CLK_MSECS_USECS_REG);

    /* If it appears that a rollover happened... */
    if (word > word3)
    {
        /*
         * get time and convert to timeval. Note that this should be changed to
         * support year 2038's rollover
         */
        Timep->tv = ktime_to_timeval(ktime_get_real());

        /* Re-read seconds */
        word  = PEEK_REG_32(hw->ioAddr, CLK_MSECS_USECS_REG);
        word2 = PEEK_REG_32(hw->ioAddr, CLK_SECS_MSECS_REG);
    }

    Timep->timeObj.secsDouble =((LO_NIBBLE(word)      * 1)       +
                                (MID_LO_NIBBLE(word)  * 10)      +
                                (MID_HI_NIBBLE(word)  * 100)     +
                                (HI_NIBBLE(word)      * 1000)    +
                                (LO_NIBBLE(word2)     * 10000)   +
                                (MID_LO_NIBBLE(word2) * 100000)  +
                                (MID_HI_NIBBLE(word2) * 1000000) +
                                (HI_NIBBLE(word2)     * 10000000) );

    /* Read minutes and hours */
    word = PEEK_REG_32(hw->ioAddr, CLK_HRS_MINS_REG);

    Timep->timeObj.minutes =((LO_NIBBLE(word)     * 1) +
                             (MID_LO_NIBBLE(word) * 10) );

    Timep->timeObj.hours   =((MID_HI_NIBBLE(word) * 1) +
                             (HI_NIBBLE(word)     * 10) );

    /* Read days */
    word = PEEK_REG_32 (hw->ioAddr, CLK_DAYS_REG);

    Timep->timeObj.days =((LO_NIBBLE(word)     * 1)  +
                          (MID_LO_NIBBLE(word) * 10) +
                          (MID_HI_NIBBLE(word) * 100) );

    /* disable fifo not empty interrupt */
    POKE_REG_32(hw->ioAddr, FIFO_INT_CTRL_REG, 0x0);

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO32(hw->ioAddr);

    /* send get date command */
    sendCmd32(hw->ioAddr, 0x5D);

    /* verify valid data in fifo */
    while ((PEEK_REG_32(hw->ioAddr, CMD_STAT_REG) & 0x1));

    /* get response from card */
    readFIFO32(hw->ioAddr, responseVec, 10);

    /* re-enable fifo not empty interrupt */
    POKE_REG_32(hw->ioAddr, FIFO_INT_CTRL_REG, 0xE0);

    /* verify its the get date response */
    if ( (responseVec[0] & 0xFF) != 0x5D ) return (1);
    if ( (responseVec[1] & 0xFF) != 0x5D ) return (1);

    /* calculate year field */
    Timep->timeObj.year = (unsigned short)((MID_LO_NIBBLE (responseVec[8]) * 1000) +
                                   (LO_NIBBLE     (responseVec[8]) * 100)  +
                                   (MID_LO_NIBBLE (responseVec[7]) * 10)   +
                                   (LO_NIBBLE     (responseVec[7]) * 1) );
    return (0);

} /* End - tproGetNtpTime32() */

/*******************************************************************************
**
** Function:    tproGetFpgaVersion32()
** Description: get the FPGA version from the board.
**
** Parameters:
**     IN: *hw    - Handle
**         *fpagVer - Pointer to the char array.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetFpgaVersion32(tsyncpci_dev_t *hw, unsigned char *fpgaVer)
{
    unsigned int word;

    word  = PEEK_REG_32(hw->ioAddr, FPGA_REV_32);

    /* save lower upper byte bits 8-15 */
    fpgaVer[0] = (word >> 8) & 0xff;

    /* save lowest byte bits 0-7 */
    fpgaVer[1] = (word & 0xff);

    return (0);

} /* End - tproGetFpgaVersion32() */

/*******************************************************************************
**
** Function:    tproResetFirmware32()
** Description: Reset Firmware on the TPRO device.
**
** Parameters:
**     IN: *hw - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproResetFirmware32(tsyncpci_dev_t *hw)
{
    /* reset firmware for diagnostic purposes only!! */
    POKE_REG_32(hw->ioAddr, CMD_STAT_REG_32, 0x4F);

    return (0);

} /* End - tproResetfirmware32() */

/*******************************************************************************
**
** Function:    tproSetHeartbeat32()
** Description: Configures the heartbeat output on the TPRO device.
**
** Parameters:
**     IN: *hw     - Handle
**         *Heartp - Pointer to heartbeat information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetHeartbeat32(tsyncpci_dev_t *hw, HeartObj *Heartp)
{
    /*
    **  send heartbeat commands
    */

    /* clear holding register */
    sendCmd32(hw->ioAddr, 0xF0);

    /* send heartbeat values */
    sendCmd32(hw->ioAddr, ((Heartp->divNumber >> 12) & 0xF) | 0xA0);
    sendCmd32(hw->ioAddr, ((Heartp->divNumber >> 8)  & 0xF) | 0xB0);
    sendCmd32(hw->ioAddr, ((Heartp->divNumber >> 4)  & 0xF) | 0xC0);
    sendCmd32(hw->ioAddr, ((Heartp->divNumber >> 0)  & 0xF) | 0xD0);

    /* send hearbeat characteristics */
    sendCmd32(hw->ioAddr, Heartp->signalType + Heartp->outputType);

    return (0);

} /* End - tproSetHeartbeat32() */

/*******************************************************************************
**
** Function:    tproSetMatchTime32()
** Description: Configures the match start/stop time feature.
**
** Parameters:
**     IN: *hw     - Handle
**         *Matchp - Pointer to start/stop information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetMatchTime32(tsyncpci_dev_t *hw, MatchObj *Matchp)
{
    unsigned char timeStr[10];

    /* clear existing match condition if not already satisfied */
    if (PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32) & 0x8) {
        POKE_REG_32(hw->ioAddr, CLR_FLAGS_REG_32, 0x8);
    }

    /* clear time register */
    sendCmd32(hw->ioAddr, 0xF0);

    /* set days */
    nibblePerByteFromUint(Matchp->days, timeStr, 3);
    sendDays32(hw->ioAddr, timeStr);

    /* set hours */
    nibblePerByteFromUint(Matchp->hours, timeStr, 2);
    sendHours32(hw->ioAddr, timeStr);

    /* set minutes */
    nibblePerByteFromUint(Matchp->minutes, timeStr, 2);
    sendMinutes32(hw->ioAddr, timeStr);

    /* set match seconds */
    nibblePerByteFromUint(Matchp->seconds, timeStr, 8);
    sendMatchSecs32(hw->ioAddr, timeStr);

    /* send match type command */
    sendCmd32(hw->ioAddr, 0xE2 + Matchp->matchType);

    return (0);

} /* End - tproSetMatchTime32() */

/*******************************************************************************
**
** Function:    tproSetPropDelayCorr32()
** Description: Sets the propagation delay correction factor.
**
** Parameters:
**     IN: *hw           - Handle
**         *microseconds - Pointer to propagation delay correction
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetPropDelayCorr32(tsyncpci_dev_t *hw, int *microseconds)
{
    char propStr[10];
    int msecs;

    /* get number of microseconds */
    msecs = *microseconds;

    /* if delay value is less than 0 -- send special values */
    if (msecs < 0) msecs += 10000;

    /* send propagation delay commands */
    sendCmd32(hw->ioAddr, 0xF0);

    nibblePerByteFromUint(msecs, propStr, 4);
    sendProp32(hw->ioAddr, (unsigned char *)propStr);
    sendCmd32(hw->ioAddr, 0xE0);

    return (0);

} /* End - tproSetPropDelayCorr32() */

/*******************************************************************************
**
** Function:    tproSetTime32()
** Description: Sets the TPRO device's IRIG-B generator to a given time.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetTime32(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    char timeStr[10];

    /* clear time register */
    sendCmd32(hw->ioAddr, 0xF0);

    /* set days */
    nibblePerByteFromUint(Timep->days, timeStr, 3);
    sendDays32(hw->ioAddr, (unsigned char *)timeStr);

    /* set hours */
    nibblePerByteFromUint(Timep->hours, timeStr, 2);
    sendHours32(hw->ioAddr, (unsigned char *)timeStr);

    /* set minutes */
    nibblePerByteFromUint(Timep->minutes, timeStr, 2);
    sendMinutes32(hw->ioAddr, (unsigned char *)timeStr);

    /*  set seconds */
    nibblePerByteFromUint((unsigned int) Timep->seconds, timeStr, 2);
    sendSeconds32(hw->ioAddr, (unsigned char *)timeStr);

    /* get the clock rolling */
    if (Timep->syncOption) {
        sendCmd32(hw->ioAddr, 0x4C); /* arm clock */
    }
    else {
        sendCmd32(hw->ioAddr, 0xE0); /* set clock time */
    }

    return (0);

} /* End - tproSetTime32() */

/*******************************************************************************
**
** Function:    tproSetYear32()
** Description: Sets the year on the TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *year - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetYear32(tsyncpci_dev_t *hw, unsigned short *year)
{
    unsigned char cmdVec[] = {0x60, 0x70, 0x80, 0x90};
    char yrString [5];
    unsigned char k;

    /* set year */
    nibblePerByteFromUint(*year, yrString, 4);

    for (k=0; k < 4; k++) {
        sendCmd32(hw->ioAddr, cmdVec[k] | (yrString[k]));
    }

    sendCmd32(hw->ioAddr, 0xEA);

    return (0);

} /* End - tproSetYear32() */

/*******************************************************************************
**
** Function:    tproSimEvent32()
** Description: Routine will simulate external time tag event.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSimEvent32(tsyncpci_dev_t *hw)
{
    /* simulate event */
    POKE_REG_32(hw->ioAddr, SIM_TAG_REG_32, 0xFF);

    return (0);

} /* End - tproSimEvent32() */

/*******************************************************************************
**
** Function:    tproSynchControl32()
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
unsigned char tproSynchControl32(tsyncpci_dev_t *hw, unsigned char *enbp)
{
    unsigned char enable = *enbp;

    /* if enable synch */
    if (enable == 1) {
        *enbp = 0xEE;
        sendCmd32(hw->ioAddr, 0x4D);
    }
    else {
        *enbp = 0xFF;
        sendCmd32(hw->ioAddr, 0x4E);
    }

    return (0);

} /* End - tproSynchControl32() */

/*******************************************************************************
**
** Function:    tproSynchStatus32()
** Description: Routine returns the state of the Synch Flag.
**
** Parameters:
**     IN: *hw   - Handle
**         *enbp - (1) to enable synchronization
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSynchStatus32(tsyncpci_dev_t *hw, unsigned char *stat)
{
    /* check synch status register */
    *stat = (PEEK_REG_32(hw->ioAddr, CMD_STAT_REG_32) & 0x4) >> 2;

    return (0);

} /* End - tproSynchStatus32() */

/*******************************************************************************
**
** Function:    tproPeek32()
** Description: Routine returns the long word value from the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproPeek32 (tsyncpci_dev_t *hw, MemObj *Mem)
{
    Mem->l_value = PEEK_REG_32(hw->ioAddr, Mem->offset);
    Mem->value = (unsigned short)Mem->l_value;

    return (0);
} /* End - tproPeek32() */

/*******************************************************************************
**
** Function:    tproPoke32()
** Description: Routine pokes the value into the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproPoke32 (tsyncpci_dev_t *hw, MemObj *Mem)
{
    POKE_REG_32 (hw->ioAddr, Mem->offset, Mem->l_value);

    return (0);
} /* End - tproPoke() */

/*******************************************************************************
**
** Function:    tproGet32()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGet32 (tsyncpci_dev_t *hw, int *ioctlArg)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproGet32() */

/*******************************************************************************
**
** Function:    tproSet32()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSet32 (tsyncpci_dev_t *hw, int *ioctlArg)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproSet32() */

/*******************************************************************************
**
** Function:    tproWait32()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproWait32 (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproWait32() */

/*******************************************************************************
**
** Function:    tproWaitTo32()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproWaitTo32 (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproWaitTo32() */
