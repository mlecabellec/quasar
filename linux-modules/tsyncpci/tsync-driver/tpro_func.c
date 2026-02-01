/*******************************************************************************
**
**  Module  : tpro_func.c
**  Date    : 04/03/06
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
FuncTable_t tsyncpci_16bit =
{
    tproGetAltitude,
    tproGetDate,
    tproGetEvent,
    tproGetFirmware,
    tproInitializeBoard,
    tproGetLatitude,
    tproGetLongitude,
    tproGetSatInfo,
    tproGetTime,
    tproGetNtpTime,
    tproResetFirmware,
    tproSetHeartbeat,
    tproSetMatchTime,
    tproSetPropDelayCorr,
    tproSetTime,
    tproSetYear,
    tproSimEvent,
    tproSynchControl,
    tproSynchStatus,
    tproPeek,
    tproPoke,
    tproGetFpgaVersion,
    tproToggleInterrupt,
    tproGet,
    tproSet,
    tproWait,
    tproWaitTo,

    CMD_STAT_REG,
    FIFO_INT_CTRL_REG,
    CLR_FLAGS_REG
};

/*******************************************************************************
**          Private Funtion Prototypes
*******************************************************************************/
unsigned char flushFIFO     (unsigned short base);
unsigned char readFIFO      (unsigned short  base,
                             unsigned short *response,
                             unsigned char   numWords);
void          sendCmd       (unsigned short base, unsigned char cmd);
void          sendDays      (unsigned short base, unsigned char *str);
void          sendHours     (unsigned short base, unsigned char *str);
void          sendMatchSecs (unsigned short base, unsigned char *str);
void          sendMinutes   (unsigned short base, unsigned char *str);
void          sendProp      (unsigned short base, unsigned char *str);
void          sendSeconds   (unsigned short base, unsigned char *str);

/***************************************************************************
**          Private Function Definitions
***************************************************************************/

/*******************************************************************************
** Function: flushFIFO
*******************************************************************************/
unsigned char flushFIFO (unsigned short base)
{
    /* Check to see if there is data in the fifo */
    if (!(PEEK_REG(base, CMD_STAT_REG) & 0x1)) {
        /* flush fifo */
        while (1) {
            volatile unsigned short fifo;

            fifo = PEEK_REG(base, FIFO_INT_CTRL_REG);

            /* check to see if FIFO is empty */
            if (fifo = PEEK_REG(base, CMD_STAT_REG), fifo & 0x1) break;
        }
    }

    return (0);

} /* End - flushFIFO() */

/*******************************************************************************
** Function: readFIFO
*******************************************************************************/
unsigned char readFIFO(unsigned short  base,
                       unsigned short *response,
                       unsigned char   numWords)
{
    while (numWords--) {
        /* wait for data to be ready */
        while ( (PEEK_REG (base, CMD_STAT_REG) & 0x1) == 1) {
            udelay (1);
        }

        /* read word */
        *response++ = PEEK_REG(base, FIFO_INT_CTRL_REG) & 0xFF;
    }

    return (0);

} /* End - readFIFO() */

/*******************************************************************************
** Function: sendCmd
*******************************************************************************/
void sendCmd(unsigned short base, unsigned char cmd)
{
    /* Send command to command port */
    POKE_REG (base, CMD_STAT_REG, cmd);

    /*  Wait (delay) for at least 100uS */
    udelay (100);

} // End - sendCmd() */

/*******************************************************************************
** Function: sendDays
*******************************************************************************/
void sendDays(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x50, 0x60, 0x70};
    unsigned char k;

    /* Send set day commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendDays() */

/*******************************************************************************
** Function: sendHours
*******************************************************************************/
void sendHours(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x80, 0x90};
    unsigned char k;

    /* Send set hour commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendHours() */

/*******************************************************************************
** Function: sendMatchSecs
*******************************************************************************/
void sendMatchSecs(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xC0, 0xD0, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0};
    unsigned char k;

    /* Send set match second commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        if (k == 2) sendCmd (base, 0xE1);  /* send days-seconds */
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendMatchSecs() */

/*******************************************************************************
** Function: sendMinutes
*******************************************************************************/
void sendMinutes(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xA0, 0xB0};
    unsigned char k;

    /* Send set min commands */

    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendMinutes() */

/*******************************************************************************
** Function: sendProp
*******************************************************************************/
void sendProp(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0x30, 0x20, 0x10, 0x00};
    unsigned char k;

    /* Send set propagation commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendProp() */

/*******************************************************************************
** Function: sendSeconds
*******************************************************************************/
void sendSeconds(unsigned short base, unsigned char *str)
{
    unsigned char cmdVec[] = {0xC0, 0xD0};
    unsigned char k;

    /* Send set second commands */
    for (k=0; k < (sizeof(cmdVec)/sizeof(unsigned char)); k++) {
        sendCmd (base, cmdVec[k] | (*str++));
    }

} /* End - sendSeconds() */

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
unsigned char tproToggleInterrupt (tsyncpci_dev_t *hw, unsigned char mask)
{
    POKE_REG(hw->ioAddr, FIFO_INT_CTRL_REG, mask);
    return (0);
}


/*******************************************************************************
**
** Function:    tproGetAltitude()
** Description: Get altitude from the TPRO device
**
** Parameters:
**     IN: *hw   - Handle
**         *Altp - Pointer the Altitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetAltitude(tsyncpci_dev_t *hw, AltObj *Altp)
{
    unsigned short responseVec[FIFO_RESPONSE_LENGTH];

    /* flush fifo */
    flushFIFO(hw->ioAddr);

    /* send the get altitude command */
    sendCmd(hw->ioAddr, 0x5D);

    /* get response from card */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify it is the altitude response */
    if ((responseVec[0] & 0xFF) != 0x5D ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5D ) return (1);

    /* calculate meters field */
    Altp->meters = (unsigned int)((MID_LO_NIBBLE (responseVec[3]) * 1000000) +
                                  (LO_NIBBLE     (responseVec[3]) * 100000)  +
                                  (MID_LO_NIBBLE (responseVec[4]) * 10000)   +
                                  (LO_NIBBLE     (responseVec[4]) * 1000)    +
                                  (MID_LO_NIBBLE (responseVec[5]) * 100)     +
                                  (LO_NIBBLE     (responseVec[5]) * 10)      +
                                  (MID_LO_NIBBLE (responseVec[6]) * 1) );

    return (0);

} /* End - tproGetAltitude() */

/*******************************************************************************
**
** Function:    tproGetDate()
** Description: Gets the date in Gregorian format from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Datep - Pointer to date result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetDate(tsyncpci_dev_t *hw, DateObj *Datep)
{
  unsigned short responseVec [FIFO_RESPONSE_LENGTH];

  /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
  flushFIFO(hw->ioAddr);

  /* send get date command */
  sendCmd(hw->ioAddr, 0x5D);

  /* verify valid data in fifo */
  while ((PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x1));

  /* get response from card */
  readFIFO(hw->ioAddr, responseVec, 10);

  /* verify its the get date response */
  if ( (responseVec[0] & 0xFF) != 0x5D ) return (1);
  if ( (responseVec[1] & 0xFF) != 0x5D ) return (1);

  /* calculate year field */
  Datep->year = (unsigned short)((MID_LO_NIBBLE (responseVec[8]) * 1000) +
                                 (LO_NIBBLE     (responseVec[8]) * 100)  +
                                 (MID_LO_NIBBLE (responseVec[7]) * 10)   +
                                 (LO_NIBBLE     (responseVec[7]) * 1) );

  /* calculate month field */
  Datep->month = (unsigned char)((MID_LO_NIBBLE (responseVec[9]) * 10) +
                                 (LO_NIBBLE     (responseVec[9]) * 1) );

  /* calculate day field */
  Datep->day   = (unsigned char)((MID_LO_NIBBLE (responseVec[2]) * 10) +
                                 (LO_NIBBLE     (responseVec[2]) * 1) );

  return (0);

} /* End - tproGetDate() */


/*******************************************************************************
**
** Function:    tproGetEvent()
** Description: Reads the time-tagged event information from fifo.
**
** Parameters:
**     IN: *hw    - Handle
**         *Waitp - Pointer to event information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetEvent(tsyncpci_dev_t *hw, WaitObj *Waitp)
{
    unsigned short *ptr = (unsigned short *) &hw->fifoVector [hw->tailIndex];

    /* parse days */
    Waitp->days = (unsigned short)((LO_NIBBLE     (ptr[2]) * 100) +
                                   (MID_LO_NIBBLE (ptr[3]) * 10)  +
                                   (LO_NIBBLE     (ptr[3]) * 1) );

    /* parse hours */
    Waitp->hours = (unsigned char)((MID_LO_NIBBLE (ptr[4]) * 10) +
                                   (LO_NIBBLE     (ptr[4]) * 1) );

    /* parse minutes */
    Waitp->minutes = (unsigned char)((MID_LO_NIBBLE (ptr[5]) * 10) +
                                     (LO_NIBBLE     (ptr[5]) * 1) );

    /* parse seconds */
    Waitp->seconds = (unsigned int)((LO_NIBBLE     (ptr[9]) * 1) +
                                    (MID_LO_NIBBLE (ptr[9]) * 10) +
                                    (LO_NIBBLE     (ptr[8]) * 100) +
                                    (MID_LO_NIBBLE (ptr[8]) * 1000) +
                                    (LO_NIBBLE     (ptr[7]) * 10000) +
                                    (MID_LO_NIBBLE (ptr[7]) * 100000) +
                                    (LO_NIBBLE     (ptr[6]) * 1000000) +
                                    (MID_LO_NIBBLE (ptr[6]) * 10000000) );

    /* increment tail index */
    hw->tailIndex = ((hw->tailIndex + 10) % (sizeof(hw->fifoVector)/sizeof(hw->fifoVector[0])));

    return (0);

} /* End - tproGetEvent() */

/*******************************************************************************
**
** Function:    tproGetFirmware()
** Description: Get Firmware ID from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *firm - Pointer to firmware ID
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetFirmware(tsyncpci_dev_t *hw, unsigned char *firm)
{
    unsigned short responseVec [FIFO_RESPONSE_LENGTH];
    unsigned char counter;

    volatile unsigned short val;

    /* flush fifo */
    flushFIFO(hw->ioAddr);

    /* send cmd */
    POKE_REG(hw->ioAddr, CMD_STAT_REG, 0xE9);

    /* verify valid data in fifo */
    while (val = PEEK_REG(hw->ioAddr, CMD_STAT_REG), val & 0x1);

    /* get firmware */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify response */
    if (responseVec[0] != 0xE9) return (1);
    if (responseVec[1] != 0xE9) return (1);

    /* copy firmware id */
    for (counter=2; counter < 6; counter++) {
        *firm++ = (unsigned char) (responseVec[counter] & 0xFF);
    }

    return (0);

} /* End - tproGetFirmware() */

/*******************************************************************************
**
** Function:    tproGetLatitude()
** Description: Get Latitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to latitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetLatitude(tsyncpci_dev_t *hw, LatObj *Latp)
{
    unsigned short responseVec [FIFO_RESPONSE_LENGTH];

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO(hw->ioAddr);

    /* send get latitude command */
    sendCmd(hw->ioAddr, 0x5F);

    /* get response from card */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify its the get latitude response */
    if ((responseVec[0] & 0xFF) != 0x5F ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5F ) return (1);

    /* calculate degrees */
    Latp->degrees = (unsigned short)((LO_NIBBLE     (responseVec[2]) * 100) +
                                     (MID_LO_NIBBLE (responseVec[3]) * 10)  +
                                     (LO_NIBBLE     (responseVec[3]) * 1) );

    /* calculate minutes */
    Latp->minutes = (unsigned int)((MID_LO_NIBBLE (responseVec[4]) * 100000) +
                                   (LO_NIBBLE     (responseVec[4]) * 10000)  +
                                   (MID_LO_NIBBLE (responseVec[5]) * 1000)   +
                                   (LO_NIBBLE     (responseVec[5]) * 100)    +
                                   (MID_LO_NIBBLE (responseVec[6]) * 10)     +
                                   (LO_NIBBLE     (responseVec[6]) * 1) );

    return (0);

} /* End - tproGetLatitude() */

/*******************************************************************************
**
** Function:    tproGetLongitude()
** Description: Get Longitude from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Latp - Pointer to longitude result
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetLongitude(tsyncpci_dev_t *hw, LatObj *Longp)
{
    unsigned short responseVec [FIFO_RESPONSE_LENGTH];

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO(hw->ioAddr);

    /* send get longitude command */
    sendCmd(hw->ioAddr, 0x5E);

    /* get response from card */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify its the get longitude response */
    if ((responseVec[0] & 0xFF) != 0x5E ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5E ) return (1);

    /* calculate degrees */
    Longp->degrees = (unsigned short)((LO_NIBBLE     (responseVec[2]) * 100) +
                                      (MID_LO_NIBBLE (responseVec[3]) * 10)  +
                                      (LO_NIBBLE     (responseVec[3]) * 1) );

    /* calculate minutes */
    Longp->minutes = (unsigned int)((MID_LO_NIBBLE (responseVec[4]) * 100000) +
                                    (LO_NIBBLE     (responseVec[4]) * 10000)  +
                                    (MID_LO_NIBBLE (responseVec[5]) * 1000)   +
                                    (LO_NIBBLE     (responseVec[5]) * 100)    +
                                    (MID_LO_NIBBLE (responseVec[6]) * 10)     +
                                    (LO_NIBBLE     (responseVec[6]) * 1) );
    return (0);

} /* End - tproGetLongitutde() */

/*******************************************************************************
**
** Function:    tproInitializeBoard()
** Description: Initializes TPRO registers upon board creation.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproInitializeBoard(tsyncpci_dev_t *hw)
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
    POKE_REG(hw->ioAddr, FIFO_INT_CTRL_REG, 0x0);

    /* flush FIFO */
    flushFIFO(hw->ioAddr);

    /* clear pending match and heartbeat flags */
    if (PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x8)
        POKE_REG(hw->ioAddr, CLR_FLAGS_REG, 0x8);
    if (PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x10)
        POKE_REG(hw->ioAddr, CLR_FLAGS_REG, 0x10);

    return (0);

} /* End - tproInitializeBoard() */

/*******************************************************************************
**
** Function:    tproGetSatInfo()
** Description: Get satellite information from TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *Satp - Pointer to satellite
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetSatInfo(tsyncpci_dev_t *hw, SatObj *Satp)
{
    unsigned short responseVec [FIFO_RESPONSE_LENGTH];

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO(hw->ioAddr);

    /* send get sattelite command */
    sendCmd(hw->ioAddr, 0x5C);

    /* get response from card */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify its the get satellite response */
    if ((responseVec[0] & 0xFF) != 0x5C ) return (1);
    if ((responseVec[1] & 0xFF) != 0x5C ) return (1);

    /* parse response */
    Satp->satsTracked = (unsigned char) (LO_NIBBLE (responseVec[7]));

    return (0);

} /* End - tproGetSatInfo() */

/*******************************************************************************
**
** Function:    tproGetTime()
** Description: Get Time information from the TPRO device.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetTime(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    unsigned short word, word2;

    /* Read seconds */
    word  = PEEK_REG(hw->ioAddr, CLK_MSECS_USECS_REG);
    word2 = PEEK_REG(hw->ioAddr, CLK_SECS_MSECS_REG);

    Timep->secsDouble =((LO_NIBBLE(word)      * 1)       +
                        (MID_LO_NIBBLE(word)  * 10)      +
                        (MID_HI_NIBBLE(word)  * 100)     +
                        (HI_NIBBLE(word)      * 1000)    +
                        (LO_NIBBLE(word2)     * 10000)   +
                        (MID_LO_NIBBLE(word2) * 100000)  +
                        (MID_HI_NIBBLE(word2) * 1000000) +
                        (HI_NIBBLE(word2)     * 10000000) );

    /* Read minutes and hours */
    word = PEEK_REG(hw->ioAddr, CLK_HRS_MINS_REG);

    Timep->minutes =((LO_NIBBLE(word)     * 1) +
                     (MID_LO_NIBBLE(word) * 10) );

    Timep->hours   =((MID_HI_NIBBLE(word) * 1) +
                     (HI_NIBBLE(word)     * 10) );

    /* Read days */
    word = PEEK_REG (hw->ioAddr, CLK_DAYS_REG);

    Timep->days =((LO_NIBBLE(word)     * 1)  +
                  (MID_LO_NIBBLE(word) * 10) +
                  (MID_HI_NIBBLE(word) * 100) );

    Timep->flags = 0xffff;
    return (0);

} /* End - tproGetTime() */

/*******************************************************************************
**
** Function:    tproGetNtpTime()
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
unsigned char tproGetNtpTime(tsyncpci_dev_t *hw, NtpTimeObj *Timep)
{
    unsigned short responseVec[FIFO_RESPONSE_LENGTH];
    unsigned short word, word2, word3;

    /*
     * get time and convert to timeval. Note that this should be changed to
     * support year 2038's rollover
     */
    Timep->tv = ktime_to_timeval(ktime_get_real());

    /* Read seconds */
    word  = PEEK_REG(hw->ioAddr, CLK_MSECS_USECS_REG);
    word2 = PEEK_REG(hw->ioAddr, CLK_SECS_MSECS_REG);
    word3 = PEEK_REG(hw->ioAddr, CLK_MSECS_USECS_REG);

    /* If it appears that a rollover happened... */
    if (word > word3)
    {
        /*
         * get time and convert to timeval. Note that this should be changed to
         * support year 2038's rollover
         */
        Timep->tv = ktime_to_timeval(ktime_get_real());

        /* Re-read seconds */
        word  = PEEK_REG(hw->ioAddr, CLK_MSECS_USECS_REG);
        word2 = PEEK_REG(hw->ioAddr, CLK_SECS_MSECS_REG);
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
    word = PEEK_REG(hw->ioAddr, CLK_HRS_MINS_REG);

    Timep->timeObj.minutes =((LO_NIBBLE(word)     * 1) +
                             (MID_LO_NIBBLE(word) * 10) );

    Timep->timeObj.hours   =((MID_HI_NIBBLE(word) * 1) +
                             (HI_NIBBLE(word)     * 10) );

    /* Read days */
    word = PEEK_REG (hw->ioAddr, CLK_DAYS_REG);

    Timep->timeObj.days =((LO_NIBBLE(word)     * 1)  +
                          (MID_LO_NIBBLE(word) * 10) +
                          (MID_HI_NIBBLE(word) * 100) );

    /* reset fifo and wait for the fifo to be empty (fifo empty flag) */
    flushFIFO(hw->ioAddr);

    /* send get date command */
    sendCmd(hw->ioAddr, 0x5D);

    /* verify valid data in fifo */
    while ((PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x1));

    /* get response from card */
    readFIFO(hw->ioAddr, responseVec, 10);

    /* verify its the get date response */
    if ( (responseVec[0] & 0xFF) != 0x5D ) return (1);
    if ( (responseVec[1] & 0xFF) != 0x5D ) return (1);

    /* calculate year field */
    Timep->timeObj.year = (unsigned short)((MID_LO_NIBBLE (responseVec[8]) * 1000) +
                                   (LO_NIBBLE     (responseVec[8]) * 100)  +
                                   (MID_LO_NIBBLE (responseVec[7]) * 10)   +
                                   (LO_NIBBLE     (responseVec[7]) * 1) );
    return (0);

} /* End - tproGetNtpTime() */

/*******************************************************************************
**
** Function:    tproGetFpgaVersion()
** Description: get the FPGA version from the board.
**              This function does nothing but is needed as a placeholder.
**
** Parameters:
**     IN: *hw    - Handle
**         *fpagVer - Pointer to the char array.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGetFpgaVersion(tsyncpci_dev_t *hw, unsigned char *fpgaVer)
{
    /* This does nothing of the 16 bit register boards */
    return (0);

} /* End - tproGetTime32() */


/*******************************************************************************
**
** Function:    tproResetFirmware()
** Description: Reset Firmware on the TPRO device.
**
** Parameters:
**     IN: *hw - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproResetFirmware(tsyncpci_dev_t *hw)
{
    /* reset firmware for diagnostic purposes only!! */
    POKE_REG(hw->ioAddr, CMD_STAT_REG, 0x4F);

    return (0);

} /* End - tproResetfirmware() */

/*******************************************************************************
**
** Function:    tproSetHeartbeat()
** Description: Configures the heartbeat output on the TPRO device.
**
** Parameters:
**     IN: *hw     - Handle
**         *Heartp - Pointer to heartbeat information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetHeartbeat(tsyncpci_dev_t *hw, HeartObj *Heartp)
{
    /*
    **  send heartbeat commands
    */

    /* clear holding register */
    sendCmd(hw->ioAddr, 0xF0);

    /* send heartbeat values */
    sendCmd(hw->ioAddr, ((Heartp->divNumber >> 12) & 0xF) | 0xA0);
    sendCmd(hw->ioAddr, ((Heartp->divNumber >> 8)  & 0xF) | 0xB0);
    sendCmd(hw->ioAddr, ((Heartp->divNumber >> 4)  & 0xF) | 0xC0);
    sendCmd(hw->ioAddr, ((Heartp->divNumber >> 0)  & 0xF) | 0xD0);

    /* send hearbeat characteristics */
    sendCmd(hw->ioAddr, Heartp->signalType + Heartp->outputType);

    return (0);

} /* End - tproSetHeartbeat() */

/*******************************************************************************
**
** Function:    tproSetMatchTime()
** Description: Configures the match start/stop time feature.
**
** Parameters:
**     IN: *hw     - Handle
**         *Matchp - Pointer to start/stop information
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetMatchTime(tsyncpci_dev_t *hw, MatchObj *Matchp)
{
    unsigned char timeStr[10];

    /* clear existing match condition if not already satisfied */
    if (PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x8) {
        POKE_REG(hw->ioAddr, CLR_FLAGS_REG, 0x8);
    }

    /* set days */
    nibblePerByteFromUint(Matchp->days, timeStr, 3);
    sendDays(hw->ioAddr, timeStr);

    /* set hours */
    nibblePerByteFromUint(Matchp->hours, timeStr, 2);
    sendHours(hw->ioAddr, timeStr);

    /* set minutes */
    nibblePerByteFromUint(Matchp->minutes, timeStr, 2);
    sendMinutes(hw->ioAddr, timeStr);

    /* set match seconds */
    nibblePerByteFromUint(Matchp->seconds, timeStr, 8);
    sendMatchSecs(hw->ioAddr, timeStr);

    /* send match type command */
    sendCmd(hw->ioAddr, 0xE2 + Matchp->matchType);

    return (0);

} /* End - tproSetMatchTime() */

/*******************************************************************************
**
** Function:    tproSetPropDelayCorr()
** Description: Sets the propagation delay correction factor.
**
** Parameters:
**     IN: *hw           - Handle
**         *microseconds - Pointer to propagation delay correction
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetPropDelayCorr(tsyncpci_dev_t *hw, int *microseconds)
{
    char propStr[10];
    int msecs;

    /* get number of microseconds */
    msecs = *microseconds;

    /* if delay value is less than 0 -- send special values */
    if (msecs < 0) msecs += 10000;

    /* send propagation delay commands */
    sendCmd(hw->ioAddr, 0xF0);
    nibblePerByteFromUint(msecs, propStr, 4);
    sendProp(hw->ioAddr, (unsigned char *)propStr);
    sendCmd(hw->ioAddr, 0xE0);

    return (0);

} /* End - tproSetPropDelayCorr() */

/*******************************************************************************
**
** Function:    tproSetTime()
** Description: Sets the TPRO device's IRIG-B generator to a given time.
**
** Parameters:
**     IN: *hw    - Handle
**         *Timep - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetTime(tsyncpci_dev_t *hw, TimeObj *Timep)
{
    char timeStr[10];

    /* clear time register */
    sendCmd(hw->ioAddr, 0xF0);

    /* set days */
    nibblePerByteFromUint(Timep->days, timeStr, 3);
    sendDays(hw->ioAddr, (unsigned char *)timeStr);

    /* set hours */
    nibblePerByteFromUint(Timep->hours, timeStr, 2);
    sendHours(hw->ioAddr, (unsigned char *)timeStr);

    /* set minutes */
    nibblePerByteFromUint(Timep->minutes, timeStr, 2);
    sendMinutes(hw->ioAddr, (unsigned char *)timeStr);

    /*  set seconds */
    nibblePerByteFromUint((unsigned int) Timep->seconds, timeStr, 2);
    sendSeconds(hw->ioAddr, (unsigned char *)timeStr);

    /* get the clock rolling */
    if (Timep->syncOption) {
        sendCmd(hw->ioAddr, 0x4C); /* arm clock */
    }
    else {
        sendCmd(hw->ioAddr, 0xE0); /* set clock time */
    }

    return (0);

} /* End - tproSetTime() */

/*******************************************************************************
**
** Function:    tproSetYear()
** Description: Sets the year on the TPRO device.
**
** Parameters:
**     IN: *hw   - Handle
**         *year - Pointer to time value
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSetYear(tsyncpci_dev_t *hw, unsigned short *year)
{
    unsigned char cmdVec[] = {0x60, 0x70, 0x80, 0x90};
    char yrString [5];
    unsigned char k;

    /* set year */
    nibblePerByteFromUint(*year, yrString, 4);

    for (k=0; k < 4; k++) {
        sendCmd(hw->ioAddr, cmdVec[k] | (yrString[k]));
    }

    sendCmd(hw->ioAddr, 0xEA);

    return (0);

} /* End - tproSetYear() */

/*******************************************************************************
**
** Function:    tproSimEvent()
** Description: Routine will simulate external time tag event.
**
** Parameters:
**     IN: *hw   - Handle
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSimEvent(tsyncpci_dev_t *hw)
{
    /* simulate event */
    POKE_REG(hw->ioAddr, SIM_TAG_REG, 0xFF);

    return (0);

} /* End - tproSimEvent() */

/*******************************************************************************
**
** Function:    tproSynchControl()
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
unsigned char tproSynchControl(tsyncpci_dev_t *hw, unsigned char *enbp)
{
    unsigned char enable = *enbp;

    /* if enable synch */
    if (enable == 1) {
        *enbp = 0xEE;
        sendCmd(hw->ioAddr, 0x4D);
    }
    else {
        *enbp = 0xFF;
        sendCmd(hw->ioAddr, 0x4E);
    }

    return (0);

} /* End - tproSynchControl() */

/*******************************************************************************
**
** Function:    tproSynchStatus()
** Description: Routine returns the state of the Synch Flag.
**
** Parameters:
**     IN: *hw   - Handle
**         *enbp - (1) to enable synchronization
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSynchStatus(tsyncpci_dev_t *hw, unsigned char *stat)
{
    /* check synch status register */
    *stat = (PEEK_REG(hw->ioAddr, CMD_STAT_REG) & 0x4) >> 2;

    return (0);

} /* End - tproSynchStatus() */

/*******************************************************************************
**
** Function:    tproPeek()
** Description: Routine returns the word value from the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproPeek (tsyncpci_dev_t *hw, MemObj *Mem)
{
    Mem->value = PEEK_REG(hw->ioAddr, Mem->offset);
    Mem->l_value = Mem->value;

    return (0);
} /* End - tproPeek() */

/*******************************************************************************
**
** Function:    tproPoke()
** Description: Routine pokes the value into the selected register.
**
** Parameters:
**     IN: *hw   - Handle
**         *Mem - Pointer to memory object.
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproPoke (tsyncpci_dev_t *hw, MemObj *Mem)
{
    POKE_REG (hw->ioAddr, Mem->offset, Mem->value);

    return (0);
} /* End - tproPoke() */

/*******************************************************************************
**
** Function:    tproGet()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproGet (tsyncpci_dev_t *hw, int *ioctlArg)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproGet() */

/*******************************************************************************
**
** Function:    tproSet()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproSet (tsyncpci_dev_t *hw, int *ioctlArg)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproSet() */

/*******************************************************************************
**
** Function:    tproWait()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproWait (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproWait() */

/*******************************************************************************
**
** Function:    tproWaitTo()
**
** Parameters:
**     IN: *hw   - Handle
**         *transaction - Pointer to transaction object
**
**     RETURNS: (0) Success
**
*******************************************************************************/
unsigned char tproWaitTo (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction)
{
    /* invalid operation for this board type. */

    return (1);
} /* End - tproWaitTo() */
