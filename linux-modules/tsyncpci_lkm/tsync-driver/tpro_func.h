/*******************************************************************************
**
**  Module  : tpro_func.h
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
#ifndef _defined_TPRO_FUNC_
#define _defined_TPRO_FUNC_

#include <linux/ktime.h>
#include <linux/hrtimer.h>

#include "tsync_error_codes.h"
#include "ddtsync.h"



/*******************************************************************************
**          Hardware Access Macros
*******************************************************************************/

#define PEEK_REG(base, offs) \
inw ((unsigned short)((base) + (offs)))

#define PEEK_REG_32(base, offs)                    \
inl ((unsigned short)((base) + (offs)))

#define POKE_REG(base, offs, val) \
outw ((unsigned short) (val), (unsigned short)((base) + (offs)))

#define POKE_REG_32(base, offs, val)                                   \
outl ((unsigned short) (val), (unsigned short)((base) + (offs)))

#define UPWORD_HI_NIBBLE(x)     (((x) & 0xF0000000) >> 28)
#define UPWORD_MID_HI_NIBBLE(x) (((x) & 0x0F000000) >> 24)
#define UPWORD_MID_LO_NIBBLE(x) (((x) & 0x00F00000) >> 20)
#define UPWORD_LO_NIBBLE(x)     (((x) & 0x000F0000) >> 16)
#define HI_NIBBLE(x)            (((x) & 0x0000F000) >> 12)
#define MID_HI_NIBBLE(x)        (((x) & 0x00000F00) >> 8)
#define MID_LO_NIBBLE(x)        (((x) & 0x000000F0) >> 4)
#define LO_NIBBLE(x)            (((x) & 0x0000000F) >> 0)
#define CHAR_TO_DEC(val)      ( (val) - '0')

/*******************************************************************************
**          Defines
*******************************************************************************/
#define FIFO_RESPONSE_LENGTH  (10)

/*
** Interrupt control flags
*/
#define DISABLE_INTERRUPTS   (0x00)
#define FIFO_IRQ_ENABLE      (0x80)
#define FIFO_IRQ_DISABLE     (0x7F)
#define MATCH_IRQ_ENABLE     (0x40)
#define MATCH_IRQ_DISABLE    (0xBF)
#define HEART_IRQ_ENABLE     (0x20)
#define HEART_IRQ_DISABLE    (0xDF)

/*
**  TPRO-PCI REGISTER LOCATIONS 16bit offsets
*/
#define FIFO_INT_CTRL_REG    (0x00)
#define CMD_STAT_REG         (0x02)
#define RESET_DEASSERT_REG   (0x04)
#define RESET_ASSERT_REG     (0x06)
#define CLK_DAYS_REG         (0x08)
#define CLK_HRS_MINS_REG     (0x0A)
#define CLK_SECS_MSECS_REG   (0x0C)
#define CLK_MSECS_USECS_REG  (0x0E)
#define CLR_FLAGS_REG        (0x10)
#define SIM_TAG_REG          (0x12)

/*
**  TPRO-PCI REGISTER LOCATIONS 32bit offsets
*/
#define FIFO_INT_CTRL_REG_32  (0x00)
#define CMD_STAT_REG_32       (0x04)
#define RESET_DEASSERT_REG_32 (0x08)
#define RESET_ASSERT_REG_32   (0x0C)
#define TIME_REG_LOW_32       (0x10)
#define TIME_REG_HIGH_32      (0x14)
#define CLR_FLAGS_REG_32      (0x18)
#define SIM_TAG_REG_32        (0x1C)
#define FPGA_REV_32           (0x20)

/*
**  TSYNC-PCIe Register Locations
*/
#define CMD_STAT_REG_TSYNC    (0x0c6)
#define CLR_FLAGS_REG_TSYNC    (0x0c6)

/*******************************************************************************
**          Public Function Prototypes 16bit registers
*******************************************************************************/
unsigned char tproGetAltitude      (tsyncpci_dev_t *hw, AltObj *Altp);
unsigned char tproGetDate          (tsyncpci_dev_t *hw, DateObj *Datep);
unsigned char tproGetEvent         (tsyncpci_dev_t *hw, WaitObj *Waitp);
unsigned char tproGetFirmware      (tsyncpci_dev_t *hw, unsigned char *firm);
unsigned char tproInitializeBoard  (tsyncpci_dev_t *hw);
unsigned char tproGetLatitude      (tsyncpci_dev_t *hw, LatObj *Latp);
unsigned char tproGetLongitude     (tsyncpci_dev_t *hw, LatObj *Longp);
unsigned char tproGetSatInfo       (tsyncpci_dev_t *hw, SatObj *Satp);
unsigned char tproGetTime          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tproGetNtpTime       (tsyncpci_dev_t *hw, NtpTimeObj *Timep);
unsigned char tproResetFirmware    (tsyncpci_dev_t *hw);
unsigned char tproSetHeartbeat     (tsyncpci_dev_t *hw, HeartObj *Heartp);
unsigned char tproSetMatchTime     (tsyncpci_dev_t *hw, MatchObj *Matchp);
unsigned char tproSetPropDelayCorr (tsyncpci_dev_t *hw, int *microseconds);
unsigned char tproSetTime          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tproSetYear          (tsyncpci_dev_t *hw, unsigned short *year);
unsigned char tproSimEvent         (tsyncpci_dev_t *hw);
unsigned char tproSynchControl     (tsyncpci_dev_t *hw, unsigned char *enbp);
unsigned char tproSynchStatus      (tsyncpci_dev_t *hw, unsigned char *stat);
unsigned char tproPeek             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tproPoke             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tproGetFpgaVersion   (tsyncpci_dev_t *hw, unsigned char *fpgaver);
unsigned char tproToggleInterrupt  (tsyncpci_dev_t *hw, unsigned char mask);
unsigned char tproGet              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tproSet              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tproWait             (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction);
unsigned char tproWaitTo           (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction);

/*******************************************************************************
**          Public Function Prototypes 32bit registers
*******************************************************************************/
unsigned char tproGetAltitude32      (tsyncpci_dev_t *hw, AltObj *Altp);
unsigned char tproGetDate32          (tsyncpci_dev_t *hw, DateObj *Datep);
unsigned char tproGetEvent32         (tsyncpci_dev_t *hw, WaitObj *Waitp);
unsigned char tproGetFirmware32      (tsyncpci_dev_t *hw, unsigned char *firm);
unsigned char tproInitializeBoard32  (tsyncpci_dev_t *hw);
unsigned char tproGetLatitude32      (tsyncpci_dev_t *hw, LatObj *Latp);
unsigned char tproGetLongitude32     (tsyncpci_dev_t *hw, LatObj *Longp);
unsigned char tproGetSatInfo32       (tsyncpci_dev_t *hw, SatObj *Satp);
unsigned char tproGetTime32          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tproGetNtpTime32       (tsyncpci_dev_t *hw, NtpTimeObj *Timep);
unsigned char tproResetFirmware32    (tsyncpci_dev_t *hw);
unsigned char tproSetHeartbeat32     (tsyncpci_dev_t *hw, HeartObj *Heartp);
unsigned char tproSetMatchTime32     (tsyncpci_dev_t *hw, MatchObj *Matchp);
unsigned char tproSetPropDelayCorr32 (tsyncpci_dev_t *hw, int *microseconds);
unsigned char tproSetTime32          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tproSetYear32          (tsyncpci_dev_t *hw, unsigned short *year);
unsigned char tproSimEvent32         (tsyncpci_dev_t *hw);
unsigned char tproSynchControl32     (tsyncpci_dev_t *hw, unsigned char *enbp);
unsigned char tproSynchStatus32      (tsyncpci_dev_t *hw, unsigned char *stat);
unsigned char tproPeek32             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tproPoke32             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tproGetFpgaVersion32   (tsyncpci_dev_t *hw, unsigned char *fpgaver);
unsigned char tproToggleInterrupt32  (tsyncpci_dev_t *hw, unsigned char mask);
unsigned char tproGet32              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tproSet32              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tproWait32             (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction);
unsigned char tproWaitTo32           (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction);

/*******************************************************************************
**          Public Function Prototypes TSync boards
*******************************************************************************/
unsigned char tsyncGetAltitude      (tsyncpci_dev_t *hw, AltObj *Altp);
unsigned char tsyncGetDate          (tsyncpci_dev_t *hw, DateObj *Datep);
unsigned char tsyncGetEvent         (tsyncpci_dev_t *hw, WaitObj *Waitp);
unsigned char tsyncGetFirmware      (tsyncpci_dev_t *hw, unsigned char *firm);
unsigned char tsyncInitializeBoard  (tsyncpci_dev_t *hw);
unsigned char tsyncGetLatitude      (tsyncpci_dev_t *hw, LatObj *Latp);
unsigned char tsyncGetLongitude     (tsyncpci_dev_t *hw, LatObj *Longp);
unsigned char tsyncGetSatInfo       (tsyncpci_dev_t *hw, SatObj *Satp);
unsigned char tsyncGetTime          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tsyncGetNtpTime       (tsyncpci_dev_t *hw, NtpTimeObj *Timep);
unsigned char tsyncResetFirmware    (tsyncpci_dev_t *hw);
unsigned char tsyncSetHeartbeat     (tsyncpci_dev_t *hw, HeartObj *Heartp);
unsigned char tsyncSetMatchTime     (tsyncpci_dev_t *hw, MatchObj *Matchp);
unsigned char tsyncSetPropDelayCorr (tsyncpci_dev_t *hw, int *microseconds);
unsigned char tsyncSetTime          (tsyncpci_dev_t *hw, TimeObj *Timep);
unsigned char tsyncSetYear          (tsyncpci_dev_t *hw, unsigned short *year);
unsigned char tsyncSimEvent         (tsyncpci_dev_t *hw);
unsigned char tsyncSynchControl     (tsyncpci_dev_t *hw, unsigned char *enbp);
unsigned char tsyncSynchStatus      (tsyncpci_dev_t *hw, unsigned char *stat);
unsigned char tsyncPeek             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tsyncPoke             (tsyncpci_dev_t *hw, MemObj *Mem);
unsigned char tsyncGetFpgaVersion   (tsyncpci_dev_t *hw, unsigned char *fpgaver);
unsigned char tsyncToggleInterrupt  (tsyncpci_dev_t *hw, unsigned char mask);
unsigned char tsyncGet              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tsyncSet              (tsyncpci_dev_t *hw, int *ioctlArg);
unsigned char tsyncWait             (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction);
unsigned char tsyncWaitTo           (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction);

/*******************************************************************************
**          Structure definition for the Function pointer table.
*******************************************************************************/
typedef struct funcTable_s
{
    unsigned char (*getAltitude)      (tsyncpci_dev_t *hw, AltObj *Altp);
    unsigned char (*getDate)          (tsyncpci_dev_t *hw, DateObj *Datep);
    unsigned char (*getEvent)         (tsyncpci_dev_t *hw, WaitObj *Waitp);
    unsigned char (*getFirmware)      (tsyncpci_dev_t *hw, unsigned char *firm);
    unsigned char (*InitializeBoard)  (tsyncpci_dev_t *hw);
    unsigned char (*getLatitude)      (tsyncpci_dev_t *hw, LatObj *Latp);
    unsigned char (*getLongitude)     (tsyncpci_dev_t *hw, LatObj *Longp);

    unsigned char (*getSatInfo)       (tsyncpci_dev_t *hw, SatObj *Satp);
    unsigned char (*getTime)          (tsyncpci_dev_t *hw, TimeObj *Timep);
    unsigned char (*getNtpTime)       (tsyncpci_dev_t *hw, NtpTimeObj *Timep);
    unsigned char (*resetFirmware)    (tsyncpci_dev_t *hw);
    unsigned char (*setHeartbeat)     (tsyncpci_dev_t *hw, HeartObj *Heartp);
    unsigned char (*setMatchTime)     (tsyncpci_dev_t *hw, MatchObj *Matchp);
    unsigned char (*setPropDelayCorr) (tsyncpci_dev_t *hw, int *microseconds);
    unsigned char (*setTime)          (tsyncpci_dev_t *hw, TimeObj *Timep);
    unsigned char (*setYear)          (tsyncpci_dev_t *hw, unsigned short *year);
    unsigned char (*simEvent)         (tsyncpci_dev_t *hw);
    unsigned char (*synchControl)     (tsyncpci_dev_t *hw, unsigned char *enbp);
    unsigned char (*synchStatus)      (tsyncpci_dev_t *hw, unsigned char *stat);
    unsigned char (*peek)             (tsyncpci_dev_t *hw, MemObj *Mem);
    unsigned char (*poke)             (tsyncpci_dev_t *hw, MemObj *Mem);
    unsigned char (*getFpgaVersion)   (tsyncpci_dev_t *hw, unsigned char *fpgaVer);
    unsigned char (*toggleInterrupt)  (tsyncpci_dev_t *hw, unsigned char mask);
    unsigned char (*get)              (tsyncpci_dev_t *hw, int *ioctlArg);
    unsigned char (*set)              (tsyncpci_dev_t *hw, int *ioctlArg);
    unsigned char (*wait)             (tsyncpci_dev_t *hw, ioctl_trans_di_wait *transaction);
    unsigned char (*waitTo)           (tsyncpci_dev_t *hw, ioctl_trans_di_wait_to *transaction);

    /* define register offsets that are used outside the board specific files */
    int cmdStatOffset;
    int fifoIntCtrlOffset;
    int clrFlagsOffset;

} FuncTable_t;

#endif  /* _defined_TPRO_FUNC_ */
