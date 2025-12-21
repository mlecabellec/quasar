/*============================================================================*
 * FILE:                      B U S A P I . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2016 BY ABACO SYSTEMS, INC. 
 *      ALL RIGHTS RESERVED.
 *
 *      THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *      COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *      THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *      OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *      AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *      SOFTWARE IS HEREBY TRANSFERRED.
 *
 *      THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *      NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    Header file for BusTools API structures.  This is the file
 *              which users should include in their programs.
 *
 *===========================================================================*/

/* $Revision:  8.28 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  01/18/1998   Added support for Version 2.05 PC-1553 microcode.V2.43.ajh
  04/24/1998   Changed _BM_MessageRead() and _InterMessageGap() to properly
               support RT 31 non-broadcast and SA 31 non-mode-code.V2.44.ajh
  05/07/1998   Changed mode to support RT-RT messages.V2.45.ajh
  06/26/1998   Added support for IP-1553 Version 2.0 firmware.V2.51.ajh
  07/23/1998   Added Win32 thread support, aperiodic message support. V2.60.ajh
  07/27/1998   Removed PC-1553 Version 1.03 microcode support.V2.61.ajh
  07/29/1998   Fixed incorrect PC-1553 V2.05 RT interrupt status word.V2.70.ajh
  09/16/1998   Added support for the PCI1553 dual-channel board.V2.81.ajh
  01/10/1999   Changed for PCI-1553 V2.50 release firmware.V3.00.ajh
  01/14/1999   Changed for IP-1553 V2.48 WCS release; V2.50 PROM.V3.02.ajh
  02/03/1999   Added support for the ISA-1553.V3.03.ajh
  04/21/1999   Fixed memory mapping problem with PC-1553 at E000:0000.V3.05.ajh
  04/30/1999   Added BusTools_DumpMemory() function.V3.06.ajh
  05/19/1999   Updated function list.V3.06.ajh
  06/04/1999   Incorporated PCI-1553 V2.54 firmware which fixed trigger in/out
               V3.10.ajh
  08/21/1999   Added new functions to support V3.20 API changes.V3.20.ajh
  11/04/1999   Added Playback and 45-bit time tag support.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  06/01/2000   Changed to Version 2.50 WCS for IP-1553 PROM V2/3.V4.04.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  12/08/2000   Added BusTools_InternalBit, BusTools_TwoBoardWrap and
               BusTools_CableWrap functions. V4.26.ajh.
  01/05/2001   Added support for the PCC-1553.V4.30.ajh
  03/28/2001   Added bus loading calculations and the defining structure
               (API_BUS_STATS).V4.32.ajh
  04/18/2001   Added WCS/LPU V3.20 for non-IP-1553.  Adds trigger on data.V4.38.ajh
  04/26/2001   Changed to WCS/LPU Version 3.21.  Changed API_RT_CBUFBROAD to
               include a count of mbuf's to allocate.V4.39.ajh
  05/07/2001   Added VME-1553 Support V4.40. RHC
  08/31/2001   Modified struct api_bus_stats to add Accumulated Interrupt Status
               per [RT][TR][SA][BUS].V4.41.ajh
  10/25/2001   broke busapi.h into busapi.h and target_defines.h
  01/07/2002   Added support for Quad-PMC and Dual Channel IP V4.46 rhc
  03/15/2002   Added support for IRIG V4.48 rhc
  04/19/2002   Remove support for PC-1553 and PC-1773
  06/05/2002   Update for the v4.52 release
  10/31/2002   Add C++ defines _LABVIEW_ define
  01/27/2003   Add QPCI-1553 Support
  10/01/2003   Add support for 32-Bit memory access
  10/01/2003   BM Trigger update.
  05/26/2004   Add PCCard-D1553 support, BusTools_API_GetBaseAddr, BusTools_API_OpenChannel, 
               BusTools_FindDevice, BusTools_ListDevices, BusTools_API_Reset, 
               BusTools_SetPolling, BusTools_GetCardnum, BusTools_GetFWRevision, 
               BusTools_GetPulse, BusTools_GetChannelStatus, BusTools_IRIG_SetBias
  08/02/2004   Add support for the QCP-1553
  08/02/2004   Add 3 new BC functions
  08/18/2005   Add BusTools_Playback_check and BusTools_GetDevInfo
  02/13/2006   Add BusTools_RS485_TX_Enable, BusTools_RS485_TX_Set_data, BusTools_RS485_Read_Regs, 
               BusTools_ExtTrigEnable.
  02/13/2006   Finalize for API 5.5
  07/27/2006   API 5.54.BusTools_GetBoardType,
  09/06/2006   Add convenience functions support (BusTools_GetCSCRegs, BusTools_BoardHasIRIG, 
               BusTools_BoardIsMultiFunction, BusTools_GetChannelCount)
  09/06/2006   Add Support for AMC-1553
  09/06/2006   Add timeout notice to API_INT_FIFO
  12/07/2006   5.62 Release
  06/06/2007   Release 5.8.  New 4.11 F/W, BM user data.
  12/14/2007   Release 5.9 4.19 F/W add R15-EC, Add BusTools_PCI_Reset, BusTools_DMA_Setup
  04/22/2008   Add support for R15-AMC.
  09/10/2008   Update API_BM_TBUF structure for interrupt status triggers
  12/10/2008   Updated for the 6.10 update
   3/15/2009   Update for the 6.20 release
  06/29/2009   Add support for the RXMC-1553
  10/20/2009   Add support for RPCIe-1553 and Channel Sharing
  01/04/2010   Add support for RXMC-1553 v2 with multiple outputs
  07/02/2010   Add BTU64BIT and BT_VOID.
  12/02/2010   Add 64-Bit Windows Driver Support
  05/14/2012   Major modification for F/W 6.0, R15-USB.
  08/06/2012   Add support for RAR15-XMC-XT multi-protocol board
  11/22/2013   ATM only release
  01/23/2014   Update API_BC_MBUF
  01/24/2014   Update the BM Init defines
  11/05/2014   Add BusTools_RT_MessageBufferNext() API. Changed parameter cardnum to flag for
               BusTools_InterMessageGap2. 
  02/26/2015   Added error codes for semaphore, event, and USB protocol. bch
  09/07/2016   Added support for new hardware MPCIE
  03/13/2017   Added support for new hardware R15-USB-MON
  10/12/2017   Added support for the VxWorks 7 OS  
 */

#ifndef _BUSAPI_H_
#define _BUSAPI_H_

/**********************************************************************
*  API revision number
***********************************************************************/

#define API_REV    828         /* Release version number */
#define API_VER    "8.28"      /* Release version for the resource file */
#define API_RC_VER 8,28,0,0    /* Release version for the resource file */
#define API_TYPE   "Release "  /* Release type is BETA or RELEASE   */
#define API_MISC ""
#define API_PRODUCTS "R15-USB/R15-USB-MON/R15-PMC/RAR15-XMC-XT/RAR15-XMC-FIO/RXMC/RQVME2/R15-AMC/RPCIe/LPCIe/MPCIE/R15-EC/QPCI/QPCX/QPMC/QPM/QVME/Q014P/QcP/PCCard-D1553" 
#define API_FIRMWARE "6.17/6.15/6.11/6.09/6.08/6.05/6.04/6.03/6.02/5.18/4.66/4.40"
#define API_OS_SUPPORT "64-Bit & 32-Bit Windows/Linux/VxWorks 7 & 6/Integrity/QNX/LynxOS"

/**************************************************************************
*  Create API-specific typedef's.  These are used whenever it is
*  necessary to create structures or variables which have specific
*  lengths, such as structures which are copied directly to the
*  hardware, or when it is desirable to create structures, data values,
*  or function references which have consistent lengths regardless of
*  the operating environment.
**************************************************************************/

#include "cei_types.h"

/* The standard BusTools 32 bit unsigned integer is defined here. */
typedef CEI_UINT32 BT_U32BIT;

/* The standard BusTools 16 bit unsigned integer is defined here. */
typedef CEI_UINT16 BT_U16BIT;

/* The standard BusTools 8 bit unsigned integer is defined here.  */
typedef CEI_UCHAR BT_U8BIT;

/* The standard BusTools 16/32 bit unsigned integer is defined here.
  This length can be chosen by the compiler for best efficiency.  */
typedef CEI_UINT BT_UINT;

/* The standard BusTools 16/32 bit signed integer is defined here.
   This length can be chosen by the compiler for best efficiency. */
typedef CEI_INT BT_INT;

/* The standard BusTools 64 bit unsigned signed integer is defined here.*/
typedef CEI_UINT64 BT_U64BIT;
   
/* The standard BusTools void is defined here. */
typedef CEI_VOID BT_VOID;


/*---------------------------------------------------------------------------*
 *                Compiler and Operating System Specific Constants
 *---------------------------------------------------------------------------*/
 
#include "target_defines.h"

#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif

/*---------------------------------------------------------------------------*/

/**************************************************************************
*  Card types, carrier types, bus types and slot mapping supported by
*   this API, specified by the user via arguements to the call to the
*   BusTools_API_InitExtended() function.  Note that not all combinations
*   are supported!
***************************************************************************/
/* Define the platforms which are supported:                              */
#define PLATFORM_PC    0x00  /* This is the standard PC/AT architecture   */
#define PLATFORM_USER  0x04  /* This is a user-defined mapping platform   */
#define PLATFORM_LVRT  0x05  /* This is a LabView RT remote platform      */

/* Define the card types which are supported by the API version:          */
#define QPMC1553    0x110    /* This is the QPMC-1553 native PMC board    */
#define QPCI1553    0x160    /* This is the QPCI-1553 Native PCI board    */
#define Q1041553P   0x180    /* This is a PC\104 4-Ch PCI board           */
#define QVME1553    0x190    /* This is a Quad Channel VME-1553 New Arch  */
#define RQVME2      0x190    /* This is a Quad Channel VME-1553 Newer Arch*/
#define PCCD1553    0x200    /* This is a Dual Channel PCCard-D1553       */
#define QCP1553     0x210    /* This is the QCP-1553 board                */
#define QPCX1553    0x220    /* This is the QPCX-1553 Native PCI board    */
#define R15EC       0x230    /* This is the R15-EC express card board     */
#define QPM1553     0x110    /* This is the QPM-1553 (QPMC1553 variant)   */
#define R15AMC      0x240    /* This is the R15-AMC (QPM1553 variant)     */
#define RPCIe1553   0x250    /* This is the RPCIe-1553 PCI Express native */
#define RXMC1553    0x260    /* This is the RXMC-1553                     */ 
#define R15XMC2     0x300    /* RXMC2-1553                                */
#define LPCIE1553   0x320    /* This is the low profile PCI express       */
#define RAR15XMCXT  0x360    /* This is the RAR15XMC extended Temp XMC    */
#define RAR15XMCIT  0x360    /* This is the RAR15XMC Industrial Temp XMC  */
#define RAR15XMCFIO 0x360    /* This is the RAR15XMC Front I/O XMC        */
#define R15PMC      0x380    /* This is the R15-PMC                       */
#define MPCIE1553   0x400    /* This is the R15-MPCIE                     */
#define R15USB      0x3000   /* This is the R15-USB                       */ 


 
/* Define the IP, et.al. Carriers which are supported:                    */
#define NATIVE      0x00     /* Native board, no carrier                  */
#define NATIVE_24   0x00     /* Native VME-1553 mapping A24 space         */
#define NATIVE_32   0x04     /* Native VME-1553 mapping A32 space         */

/* Special NI defines */
#define NI_A32_MAP_16 0x1000000
#define NI_A32_MAP_32 0x2000000

/* Define the native channels                                             */
#define CHANNEL_1   0     /* This is the first slot or channel            */
#define CHANNEL_2   1     /* This is the second slot or channel           */
#define CHANNEL_3   2     /* This is the third slot or channel            */
#define CHANNEL_4   3     /* This is the fourth slot or channel           */

/* Define the IP Carrier Memory Maps which are supported:                 */
#define CARRIER_MAP_DEFAULT  0x00   /* Default carrier memory map         */
#define CARRIER_MAP_A32      0x10   /* Native VME - A32 Addressing        */
#define CARRIER_MAP_A24      0x11   /* Native VME - A24 Addressing        */
#define CARRIER_MAP_A16      0x12   /* Native VME - A16 Addressing        */
#define CARRIER_MAP_A24S     0x13   /* Native VME - A24 VxWorks SUP DATA Addressing */
#define CARRIER_MAP_A32S     0x14   /* Native VME - A32 VxWorks SUP DATA Addressing */
#define CARRIER_MAP_A24SP    0x15   /* Native VME - A24 VxWorks SUP PGM Addressing */
#define CARRIER_MAP_A32SP    0x16   /* Native VME - A32 VxWorks SUP PGM Addressing */
#define CARRIER_MAP_A32UP    0x17   /* Native VME - A32 VxWorks USR PGM Addressing */ 
#define CARRIER_MAP_A24UP    0x18   /* Native VME - A24 VxWorks USR PGM Addressing */ 

/****************************************************************************
*  General definitions for 1553 access
****************************************************************************/
#define WAIT    1
#define NOWAIT  0

#define API_DEMO_MODE        0x0000
#define API_SW_INTERRUPT     0x0001
#define API_HW_INTERRUPT     0x0002
#define API_HW_ONLY_INT      0x0003
#define API_INT_MASK         0x003f

#define API_B_MODE           0x0000
#define API_A_MODE           0x0080 

#define API_MANUAL_INT       0x0020
#define API_CHANNEL_MAP      0x0040

#define API_MODE_MASK        0x00f0
#define API_NO_MEM_TST       0x0800
#define API_CONT_ON_MEM_FAIL 0x0400

/**********************************************************************
* Define Discrete Trigger
**********************************************************************/
#define TRIGGER_OUT_DIS_7 0x1
#define TRIGGER_OUT_DIS_8 0x2
#define TRIGGER_OUT_NONE  0x0
#define TRIGGER_IN_NONE   0x0
#define TRIGGER_IN_485    0x1
#define TRIGGER_IN_DIS_7  0x2
#define TRIGGER_IN_DIS_8  0x3

#define RS485_TXEN_REG    0x0
#define RS485_TXDA_REG    0x1
#define RS485_RXDA_REG    0x2

#define EXT_TRIG_ENABLE   1
#define EXT_TRIG_DISABLE  0

#define DISCRETE_GROUND   0
#define DISCRETE_OFF      0
#define DISCRETE_ON       1
#define DISCRETE_PULL_UP  1


#define TRIG_DEDICATED  0
#define DISCRETE        1
#define EIA485          2
#define PIO             3

/* 1760 Hardwire address options */
#define LATCH_DATA   0
#define CURRENT_DATA 1

/********************************************************
*  Memory Regions
********************************************************/
#define RAM        0
#define HWREG      1
#define FILEREG    2
#define RAMREG     2
#define HIF        3
#define RAM32      4
#define RELRAM32   5
#define RELRAM     6
#define TRIGREG    7
#define TTREG      8
#define BMRAM32    9
#define SHRMEM     10

#define SHRMEM_ARINC      0x0
#define SHRMEM_1553       0x400
#define SHRMEM_CHAN_INFO  0x800
#define SHRMEM_CHAN_SIZE  0x200

#define CHAN_INIT         0x0
#define CHAN_STATUS       0x4
#define CHAN_DIS_USE      0x8
#define CHAN_TRIG_USE     0xc
#define CHAN_RT_ADDR      0x10
#define CHAN_MODE         0x14
#define CHAN_TT_MODE      0x18
#define CHAN_TT_INIT      0x1c
#define CHAN_TT_DISP      0x20
#define CHAN_TT_PER       0x24
#define CHAN_TT_LSB       0x28
#define CHAN_TT_MSB       0x2c	

/**********************************************************************
* Define for transformer or direct coupling
**********************************************************************/
#define TRANSFORMER           1
#define DIRECT                0
#define DAC_VALUE             2
#define TRANS_MAX_VOLTS    1980
#define DIRECT_MAX_VOLTS    650


/***************************************************************************
*  VME Definitions
****************************************************************************/
#define NUM_VME_CONFIG_REG    12
#define RTADDR_PARITY_ERROR 0x20
/***************************************************************************
*  Test Bus Definitions for VME/VXI1553 and QPCI-1553 only
****************************************************************************/
#define TEST_BUS_ENABLE  0x1000
#define TEST_BUS_DISABLE ~TEST_BUS_ENABLE

#define PCI_RESET_ENABLE  1
#define PCI_RESET_DISABLE 0

#define VME_RESET_ENABLE  1
#define VME_RESET_DISABLE 0

/***************************************************************************
*  1553 Bus Setting
****************************************************************************/
#define INTERNAL_BUS 1
#define EXTERNAL_BUS 0
 
/**********************************************************************
*   IRIG Definitions
**********************************************************************/
#define IRIG_ENABLE         0x2
#define IRIG_DISABLE        0x0
#define IRIG_EXTERNAL       0x0
#define IRIG_INTERNAL       0x4
#define IRIG_OUT_ENABLE     0x2
#define IRIG_OUT_DISABLE    0x0

/**********************************************************************
*  Mode definitions
**********************************************************************/
#define MULTI_FUNCTION     0
#define SINGLE_FUNCTION    1

#define SA31_MODECODE      1
#define NO_SA31_MODECODE   0
#define BROADCAST          1
#define NO_BROADCAST       0

/*********************************************************************
*   TIME TAG SETTINGS
*********************************************************************/
#define TIME_HWL      0x1
#define TIME_SWL      0x2
#define TIME_IRIG     0x4
#define TIME_AUTO     0x8
#define TIME_EXT_CLK  0x20
#define TIME_EXT_EDGE 0x40
#define TIME_RESET    0x80
#define EXT_RESET_ENABLE  0x1
#define EXT_RESET_DISABLE 0x0

#define V6_BASE_TIME 1000000000  /* 1 Nanosecond Resolution             */
#define V5_BASE_TIME    1000000  /* 1 Microsecond Resolution            */
#define NANO_TO_MICRO      1000  /* convert nanoseconds to microseconds */
/********************************************************
*  Timer Definitions
********************************************************/
#define TIMER_START    0
#define TIMER_RESTART  1
#define TIMER_DEFAULT  10

/********************************************************
*  BC RT BC Definitions
********************************************************/
#define REL_GAP             0x0
#define FRAME_MESSAGING     0x0
#define FIXED_GAP           0xf
#define MSG_SCHD            0x10
#define NO_PRED_LOGIC       0x40
#define FRAME_START_TIMING  0x20
#define MULTIPLE_BC_BUFFERS 0x80
#define MFOVFL_INT_ENA      0x100
#define NO_BC_RETRY         0
#define NO_INTERUPT         0
#define RECEIVE             0
#define TRANSMIT            1

#define BC_BLOCK_NEXT      0
#define BC_BLOCK_LAST      0xffff

#define FRAME_TIME_LOWER_LIMIT 100

#define BC_START          1
#define BC_STOP           0
#define BC_START_TT_RESET 3
#define BC_START_BIT      0xf
#define BC_HALT           2
#define BC_NO_INIT        0
#define BC_AUTO_INC_EN    1
#define BC_AUTO_INC_DIS   0
#define BC_WAIT           1
#define BC_NO_WAIT        0

#define RT_START          1
#define RT_START_BIT      0xf
#define RT_STOP           0
#define RT_NO_INIT        0

#define RTADDR_SELECT1 0x40
#define RTADDR_SELECT2 0x4000
#define RT_SET         0x8000
#define RT_NOCHANGE    0x0
#define RT_EXT_STATUS  0xf

#define BM_START          0x1
#define BM_START_TT_RESET 0x3
#define BM_START_BIT      0xf
#define BM_STOP           0x0
#define BM_ENABLE_BUS     0x1 // (F/W v4.99 and below only)
#define BM_DISABLE_BUS    0x0 // (F/W v4.99 and below only)
#define BM_NO_HW_INT      0x8 // (F/W v6.03 and above only)
#define BM_NO_SEG1_INIT   0x10
#define BM_NO_INIT        0


/*Channel sharing */
#define BC_QUIT    4
#define BM_QUIT    2
#define RT_QUIT    1
#define SHARE_QUIT 8

/********************************************************************
* Temperate Read Locations
********************************************************************/
#define INTERNAL   0     /* is the internal temp sensor of the MAX1668 temp sensor chip */
#define TFPGA      1     /* FPGA sensor                                                 */
#define TZBT       2     /* ZBT sensor                                                  */
#define TCHANNEL   3	 /* Channel sensor (RIO - CH4; FIO - CH1)                       */
#define TBOARD     4     /* Board sensor (mid-board RIO - DC-DC Converter FIO)          */
#define SENSOR0    0
#define SENSOR1    1
#define SENSOR2    2
#define SENSOR3    3
#define SENSOR4    4 
   
#define OVER_TEMP_ALARM 0x100

/********************************************************************
* DMA Defines
********************************************************************/
#define BOARD_TO_HOST 0
#define HOST_TO_BOARD 1

/********************************************************************
* BusTools_SetOption Defines
********************************************************************/
#define NO_MF_OVFL       0x0001  /* No Minor Frame Overflow                       */
#define MON_INV_CMD      0x0002  /* Monitor Invalid command                       */
#define DMP_ON_BM_STP    0x0004  /* Dump on BM Stop                               */
#define BM_TRIG_ON_MSG   0x0008  /* BM trigger on Message                         */
#define NO_HGH_WRD       0x0010  /* No High word error                            */
#define PROC_BM_ON_INT   0x0020  /* Process BM data on Interrupt                  */
#define UNDEF_MC_ILL     0x0040  /* Undefined Mode Codes are treated as Illegal   */

/*********************************************************************
*   Data Conversion
*********************************************************************/
#define DATATYPE_16_SDEC         0
#define DATATYPE_16_UDEC         1
#define DATATYPE_16_HEX          2
#define DATATYPE_16_OCTAL        3
#define DATATYPE_16_BINARY       4
#define DATATYPE_16_BCD          5
#define DATATYPE_16_BCD_2        6
#define DATATYPE_16_USCALE       7
#define DATATYPE_32_SDEC         8
#define DATATYPE_32_UDEC         9
#define DATATYPE_32_IEEE         10
#define DATATYPE_32_HEX          11
#define DATATYPE_32_BCD          12
#define DATATYPE_32_USCALE       13
#define DATATYPE_48_LATLONG      14
#define DATATYPE_16_TRANSLATE    15
#define DATATYPE_32_TRANSLATE    16
#define DATATYPE_16_SSCALE       17
#define DATATYPE_32_SSCALE       18
#define DATATYPE_32_MSWF_SDEC    19
#define DATATYPE_32_MSWF_UDEC    20
#define DATATYPE_32_MSWF_SSCALE  21
#define DATATYPE_32_MSWF_USCALE  22
#define DATATYPE_16_DISCRETE     23
#define DATATYPE_16_UDB          24
#define DATATYPE_32_UDB          25
#define DATATYPE_32_DEC_FLOAT    26
#define DATATYPE_32_1750         27
#define DATATYPE_MAX             28
#define LOOKUP_ITEMS             10 /* Non linear translation table items */

typedef struct data_convert
   {
   BT_U16BIT  wDatatype;       /* datatype code                                        */
   BT_U16BIT  wDecimals;       /* no. of digits to right of dec pt (for scaled format) */
   float fFactor;              /* scale factor                                         */
   BT_U16BIT  wFactortype;     /* factor type: 1->mult 2->div                          */
   float fOffset;              /* offset value (scaled data type only)                 */
   BT_U16BIT  wTranslateItems; /* no of lookup table entries defined                   */
   BT_U16BIT  *uiTranslateRaw; /* raw hex data entries                                 */
   float *fTranslateDis;       /* display data entries                                 */ 
   void* value;	               /* pointer to data words.                               */
   BT_INT status;              /* return code                                          */
   char * string;              /* string pointer                                       */
   }
DATA_CONVERT;

typedef struct devicelist
{
   BT_INT  num_devices;
   BT_UINT device_name[MAX_BTA];
   BT_INT  device_num[MAX_BTA];
   char    name[MAX_BTA][256];
}
DeviceList;

/**********************************************************************
*  1553 command word structure definition (from 1553 spec)
**********************************************************************/

typedef struct bt1553_command
   {
#ifdef NON_INTEL_BIT_FIELDS
   BT_U16BIT rtaddr:5;         /* rt address field              (MSB)  */
   BT_U16BIT tran_rec:1;       /* transmit/receive bit                 */
   BT_U16BIT subaddr:5;        /* subaddress field                     */   
   BT_U16BIT wcount:5;         /* word count or mode code field (LSB)  */
#else  /* INTEL-Compatible bit field ordering                          */
   BT_U16BIT wcount:5;         /* word count or mode code field (LSB)  */
   BT_U16BIT subaddr:5;        /* subaddress field                     */
   BT_U16BIT tran_rec:1;       /* transmit/receive bit                 */
   BT_U16BIT rtaddr:5;         /* rt address field              (MSB)  */
#endif
   }
BT1553_COMMAND;

/**********************************************************************
*  1553 status word structure definition (from 1553 spec)
**********************************************************************/

typedef struct bt1553_status
   {
#ifdef NON_INTEL_BIT_FIELDS
   BT_U16BIT rtaddr:5;         /* rt address field               (MSB) */
   BT_U16BIT me:1;             /* message error                        */ 
   BT_U16BIT inst:1;           /* instrumentation bit                  */
   BT_U16BIT sr:1;             /* service request                      */
   BT_U16BIT res:3;            /* unused bits                          */
   BT_U16BIT bcr:1;            /* broadcast received bit               */
   BT_U16BIT busy:1;           /* busy flag bit                        */ 
   BT_U16BIT sf:1;             /* subsystem flag bit                   */ 
   BT_U16BIT dba:1;            /* dynamic bus acceptance flag bit      */  
   BT_U16BIT tf:1;             /* terminal flag bit              (LSB) */
#else  /* INTEL-Compatible bit field ordering                          */ 
   BT_U16BIT tf:1;             /* terminal flag bit              (LSB) */
   BT_U16BIT dba:1;            /* dynamic bus acceptance flag bit      */ 
   BT_U16BIT sf:1;             /* subsystem flag bit                   */
   BT_U16BIT busy:1;           /* busy flag bit                        */ 
   BT_U16BIT bcr:1;            /* broadcast received bit               */
   BT_U16BIT res:3;            /* unused bits                          */
   BT_U16BIT sr:1;             /* service request                      */
   BT_U16BIT inst:1;           /* instrumentation bit                  */
   BT_U16BIT me:1;             /* message error                        */ 
   BT_U16BIT rtaddr:5;         /* rt address field               (MSB) */
#endif
   }
BT1553_STATUS;

/**********************************************************************
*  Bit definitions for BusTools Interrupt Enable
*  and Interrupt Status entries (8-/16-/32-bit interrupt/status masks)
**********************************************************************/

#define BT1553_INT_HIGH_WORD       0x00000001L // high word error
#define BT1553_INT_INVALID_WORD    0x00000002L // invalid word error    **
#define BT1553_INT_LOW_WORD        0x00000004L // low word error
#define BT1553_INT_INVERTED_SYNC   0x00000008L // inverted sync         **
#define BT1553_INT_MID_BIT         0x00000010L // mid bit error         **
#define BT1553_INT_TWO_BUS         0x00000020L // Two bus error V4.25
#define BT1553_INT_PARITY          0x00000040L // parity error          **
#define BT1553_INT_NON_CONT_DATA   0x00000080L // non-contiguous data   **
#define BT1553_INT_EARLY_RESP      0x00000100L // early response
#define BT1553_INT_LATE_RESP       0x00000200L // late response
#define BT1553_INT_BAD_RTADDR      0x00000400L // incorrect rt address
#define BT1553_INT_CHANNEL         0x00000800L // Bus (0=A, 1=B)
#define BT1553_INT_BUSB            0x00000800L // Bus (0=A, 1=B)
                                   /*0x00001000L*/
#define BT1553_INT_WRONG_BUS       0x00002000L // response on wrong bus
#define BT1553_INT_BIT_COUNT       0x00004000L // bit count error       **
#define BT1553_INT_NO_IMSG_GAP     0x00008000L // no intermessage gap
//      Note: In the above list, "**" means the bit can be set for a
//            data word (vs. a command or status word, or a message),
#define BT1553_INT_END_OF_MESS     0x00010000L // end of message
#define BT1553_INT_BROADCAST       0x00020000L // broadcast message
#define BT1553_INT_RT_RT_FORMAT    0x00040000L // rt-to-rt message format
#define BT1553_INT_RESET_RT        0x00080000L // reset rt
#define BT1553_INT_SELF_TEST       0x00100000L // self test
#define BT1553_INT_MODE_CODE       0x00200000L // message is a Mode Code
#define BT1553_INT_NOCMD           0x00400000L // No command seen by decoder
#define BT1553_INV_RTRT_TX         0x00800000L // Invalid RTRT TX CMD2  
#define BT1553_RTRT_RCV_NRSP       0x01000000L // No response on RCV RT-RT message
#define BT1553_INT_RETRY           0x02000000L // retry NI for BM
#define BT1553_INT_NO_RESP         0x04000000L // no response (for RT-RT, set if EITHER was no response)
#define BT1553_INT_ME_BIT          0x08000000L // 1553 status wd message error bit
#define BT1553_INT_ALT_BUS         0x80000000L // retry on alternate bus.

// *****  SOFTWARE ONLY BITS ***** (not set by the hardware)

#define BT1553_INT_TRIG_BEGIN      0x10000000L // message with trigger begin
#define BT1553_INT_TRIG_END        0x20000000L // message with trigger end
#define BT1553_INT_BM_OVERFLOW     0x40000000L // message at buffer overflow

// Interrupt error bits

#define BT1553_INT_ERROR_BITS (BT1553_INT_HIGH_WORD     | \
   BT1553_INT_INVALID_WORD   | BT1553_INT_NOCMD         | \
   BT1553_INT_BAD_RTADDR     | BT1553_INT_INVERTED_SYNC | \
   BT1553_INT_MID_BIT        | BT1553_INT_TWO_BUS       | \
   BT1553_INT_PARITY         | BT1553_INT_NON_CONT_DATA | \
   BT1553_INT_EARLY_RESP     | BT1553_INT_LATE_RESP     | \
   BT1553_INT_LOW_WORD       | BT1553_INT_BIT_COUNT     | \
   BT1553_INT_WRONG_BUS		 | BT1553_INT_RETRY         | \
   BT1553_INT_NO_IMSG_GAP	 | BT1553_INT_BM_OVERFLOW   | \
   BT1553_INT_ME_BIT         | BT1553_INT_NO_RESP)          /* 0x0bc0e7ff */

/**********************************************************************
*  Response time for hardware BM message buffers
**********************************************************************/

typedef struct bt1553_hw_bmresponse
   {
#ifdef NON_INTEL_BIT_FIELDS
   BT_U16BIT unused:10;  /* unused bits                  (MSB) */
   BT_U16BIT time:6;     /* response time, 0.5 usec LSB  (LSB) */
#else  /* INTEL-Compatible bit field ordering                  */
   BT_U16BIT time:6;     /* response time, 0.5 usec LSB  (LSB) */
   BT_U16BIT unused:10;  /* unused bits                  (MSB) */
#endif
   }
BT1553_HW_BMRESPONSE;

/**********************************************************************
*  General BusTools API 48-bit time structure
**********************************************************************/

typedef struct bt1553_v5time
   {
   BT_U32BIT  microseconds PACKED;  /* microseconds since start */
   BT_U16BIT  topuseconds PACKED;   /* Most significant part of microseconds */
   }
BT1553_V5TIME;

typedef struct bt1553_time
   {
   BT_U32BIT  microseconds PACKED;  // microseconds since start
   BT_U32BIT  topuseconds PACKED;   // Most significant part of microseconds
   }
BT1553_TIME;


#define BT1553_BUFCOUNT  34        /* Number of BC data word +2             */
#define PCI1553_BUFCOUNT 40        /* The PCI-1553 rounds this number up    */

typedef struct device_info
{
   BT_INT    busType;                    // One of BUS_TYPE.
   BT_INT    nchan;                      // number of channeld.
   BT_INT    irig;                       // IRIG option 0=no 1=yes.
   BT_INT    mode;                       // mode 0=single 1=multi.
   BT_INT    memSections;
   BT_UINT   VendorID;                   // Vendor ID if PCI card
   BT_UINT   DeviceID;                   // Device ID if PCI card
   BT_UINT   CSCReg;                     // the CSC register
}DEVICE_INFO;

typedef struct channel_status
{
#ifdef NON_INTEL_BIT_FIELDS
   BT_U32BIT err_info:3;       // Error copde information                          (MSB)
   BT_U32BIT int_fifo_count:6; // Number interrupt threads running
   BT_U32BIT irig_on:1;        // IRIG_B time source
   BT_U32BIT broadcast:1;      // SA-31 Broadcast 0 no 1 yes
   BT_U32BIT SA_31:1;          // SA_31 Mode Code 0 not used 1 used
   BT_U32BIT coupling:1;       // 0=direct 1= xformer
   BT_U32BIT extbus:1;         // External bus off = 0 on = 1;
   BT_U32BIT run_mode:1;       // 1553 mode 0 = B 1 = A
   BT_U32BIT mc_17:1;          // MC-17 enabled
   BT_U32BIT int_mode:4;       // interrupt mode 0 polling 1 H/W int
   BT_U32BIT reserve_run:1;    // Reserved
   BT_U32BIT bm_run:1;         // bm is running
   BT_U32BIT rt_run:1;         // rt is running
   BT_U32BIT bc_run:1;         // bc is running
   BT_U32BIT err_reserve:3;    // word count or mode code field 
   BT_U32BIT byte_cnt_err:1;   // Byte count error on read or write
   BT_U32BIT addr_err:1;       // address error
   BT_U32BIT interr:1;         // interrupt error
   BT_U32BIT wcs_pulse:1;      // WCS heart beat dead
   BT_U32BIT mf_ovfl:1;        // minor frame overflow             (LSB)
#else  /* INTEL-Compatible bit field ordering */
   BT_U32BIT mf_ovfl:1;        // minor frame overflow             (LSB)
   BT_U32BIT wcs_pulse:1;      // WCS heart beat dead
   BT_U32BIT interr:1;         // interrupt error
   BT_U32BIT addr_err:1;       // address error
   BT_U32BIT byte_cnt_err:1;   // Byte count error on read or write
   BT_U32BIT err_reserve:3;    // reserved 
   BT_U32BIT bc_run:1;         // bc is running
   BT_U32BIT rt_run:1;         // rt is running
   BT_U32BIT bm_run:1;         // bm is running
   BT_U32BIT reserve_run:1;    // Reserved
   BT_U32BIT int_mode:4;       // interrupt mode 0 polling 1 H/W int
   BT_U32BIT mc_17:1;          // MC-17 enabled
   BT_U32BIT run_mode:1;       // 1553 mode 0 = B 1 = A
   BT_U32BIT extbus:1;         // External bus off = 0 on = 1;
   BT_U32BIT coupling:1;       // 0=direct 1= xformer              
   BT_U32BIT SA_31:1;          // SA_31 Mode Code 0 not used 1 used
   BT_U32BIT broadcast:1;      // SA-31 Broadcast 0 no 1 yes
   BT_U32BIT irig_on:1;        // IRIG_B time source
   BT_U32BIT int_fifo_count:6; // Number interrupt threads running
   BT_U32BIT err_info:3;       // Error code information                          (MSB)
#endif   
}
API_CHANNEL_STATUS;

//                                            
#define CHAN_STAT_MF_OVFL   0x1                
#define CHAN_MEM_TST_FAIL   0x2                
#define CHAN_STAT_INT_ERR   0x4                 
#define CHAN_STAT_ADDR_ERR  0x8
#define CHAN_STAT_BC_RUN    0x100 
#define CHAN_STAT_RT_RUN    0x200  
#define CHAN_STAT_BM_RUN    0x400 
#define CHAN_STAT_INT_MODE  0xf00 
#define CHAN_STAT_MC_17     0x1000   
#define CHAN_STAT_RUN_MODE  0x2000   
#define CHAN_STAT_EXT_BUS   0x4000  
#define CHAN_STAT_COUPLING  0x8000  
#define CHAN_STAT_SA_31     0x10000  
#define CHAN_STAT_BRDCST    0x20000  

#define FIFO_MASK 0x3f000000
#define CHANNEL_ERROR_MASK 0x4
/**********************************************************************
*  Bus Controller Definitions
**********************************************************************/

/************************************************
*  BC message buffer
************************************************/

typedef struct api_bc_mbuf
   {
   BT_U16BIT messno;            // Message number (0-based) 'FFFF' indicates end of aperiodic list
   BT_U16BIT control;           // Bus Controller Control Word.  Defines this
                                //  message to be either a 1553 data transfer
                                //  msg (BC_CONTROL_MESSAGE) or a list management
                                //  msg (conditional branch or noop).
   BT_U16BIT messno_next;       // Next message number
   BT_U16BIT messno_prev;       // Previous message number (Not used, V2.51 and up)

   // This group is for standard bc messages (not conditional messages)
   // Any given message may NOT be both a 1553 message and a conditional message
   //  at the same time.  Data in this section is ignored for conditional msgs,
   //  and data in the following section is ignored for 1553 messages.
   BT1553_COMMAND mess_command1;       // 1553 command word (Receive for RT-RT msgs)
   BT1553_COMMAND mess_command2;       // 1553 cmd word #2 (Transmit, RT-RT msgs)
   BT_U16BIT errorid;                  // Error injection buffer id number
   BT_U16BIT gap_time;                 // Time from end of current message to start of next
                                       //   of this msg (IP-/ISA-/PCI-1553)
   BT1553_STATUS mess_status1;         // 1553 status word #1 (Transmit for RT-RT and Broadcast RT-RT)
   BT1553_STATUS mess_status2;         // 1553 status word #2 (Receive for RT-RT msgs, NULL for Broadcast RT-RT)
   BT_U32BIT status;                   // Message error status from hardware
   BT_U16BIT data[2][BT1553_BUFCOUNT]; // Data buffers A and B

   // This group is for BC_CONTROL_CONDITION, #2 and #3 list management messages
   // These words are ignored if the message is a 1553 message.
   BT_U32BIT data_value;       // Data value to compare
   BT_U32BIT data_mask;        // Bit mask for compare
   BT_U16BIT address;          // Word number of the previous or specified msg
                               //  to compare for BC_CONTROL_CONDITION or
                               //  BC_CONTROL_CONDITION3 Conditional branch msgs.
   BT_U16BIT messno_branch;    // If '==' branch to this message
   BT_U16BIT messno_compare;   // Msg # containing data word in "address"

   // These variables are only used by the extended BC message function calls.

   BT_U16BIT start_frame;      // Start frame for message scheduling
   BT_U32BIT test_address;     // Byte address of word to be tested by
                               //  BC_CONTROL_CONDITION2 conditional branch msg.
                               //  or repeat rate if start_address is greater than 0.
   BT_U32BIT cond_count_val;   // Conditional counter reload value
   BT_U32BIT cond_counter;     // Conditional counter initial value

   BT_U16BIT data_buf_num1;    // Buffer number of the first BC data buffer
   BT_U16BIT data_buf_num2;    // Buffer number of the second BC data buffer
   BT1553_TIME time_tag;       // Tag Time for  F/W version 3.97 and above
   BT_U32BIT long_gap;         // Extend 24-bit gap time.
   BT_U32BIT next_branch_addr; // Msg address the Conditional Branch executes next
   BT_U16BIT spare;            // Padding for C#
   BT_U16BIT rep_rate;         // Repeat rate for message scheduling.
   }
API_BC_MBUF; 


/************************************************
*  Bit definitions for API_BC_MBUF.control Word.
************************************************/

/* Define the message type */
#define BC_CONTROL_NOP        0x0000   // no-op command (jump .+1)
#define BC_CONTROL_MESSAGE    0x0001   // data message base value
#define BC_CONTROL_BRANCH     0x0002   // branch message type (jump to specified)
#define BC_CONTROL_CONDITION  0x0003   // conditional branch on previous msg
#define BC_CONTROL_CONDITION2 0x0004   // conditional branch on specific addr
#define BC_CONTROL_CONDITION3 0x0006   // cond branch on specified msg and word
#define BC_CONTROL_LAST       0x0005   // last logical message in list; stops the BC
#define BC_CONTROL_HALT       0x8005   // last logical message in list; stops the BC and clears the BC external sync
#define BC_CONTROL_MSG_NOP    0x0007   // Data Message noop to start
#define BC_CONTROL_TYPEMASK   0x0007   // mask for message type

/* These bits are defined for messages */
#define BC_CONTROL_MFRAME_BEG 0x0008   // beginning of a minor frame
#define BC_CONTROL_MFRAME_END 0x0010   // end of a minor frame
#define BC_CONTROL_RTRTFORMAT 0x0020   // rt to rt format 1553 message
#define BC_CONTROL_RETRY      0x0040   // retry enable: Conditions for retrying
                                       //  are setup in the BusTools_BC_Init()
                                       //  function call defined below.
#define BC_CONTROL_INTERRUPT  0x0080   // interrupt enable on this message
#define BC_CONTROL_INTQ_ONLY  0x1000   // do not generate H/W int but add event to queue
#define BC_CONTROL_TIMED_NOP  0x2000   // NOOP that preserves timing
#define BC_CONTROL_EXT_SYNC   0x8000   // clears external sync 

#define BC_CONTROL_BUFFERA    0x0100   // buffer a is active
#define BC_CONTROL_BUFFERB    0x0200   // buffer b is active
#define BC_CONTROL_CHANNELA   0x0400   // output message on Bus a
#define BC_CONTROL_CHANNELB   0x0800   // output message on bus b
#define BC_CONTROL_BUSA       0x0400   // output message on bus a
#define BC_CONTROL_BUSB       0x0800   // output message on bus b

/************************************************
*  External BC Triggering options for the
*   BusTools_BC_Trigger() function.
************************************************/
#define BC_CLR_EXT_SYNC       0x2000
#define BC_TRIGGER_IMMEDIATE  0  /* BC starts running immediately      */
#define BC_TRIGGER_ONESHOT    1  /* BC is triggered by external source */
                                 /*   and free runs after trigger.     */
#define BC_TRIGGER_REPETITIVE -1 /* Each minor frame is started by the */
                                 /*   external trigger.                */
#define BC_TRIGGER_USER       2  /* Allows user to configure triggering user BC_CONTROL_LAST */

#define TIMED_NOOP 0xf
#define NOOP 0x1
#define MSG_OP 0x0      
/************************************************
* BC Retry options
************************************************/
#define RETRY_ALTERNATE_BUS  2
#define RETRY_SAME_BUS       1
#define RETRY_END            0


/********************************************************
*  Retry Enable bit definitions for BusTools_BC_Init()
*   wRetry parameter.  These bits enable the conditons
*   for BC_CONTROL_RETRY retries in the API_BC_MBUF.
********************************************************/
#define BC_RETRY_TF            0x1
#define BC_RETRY_TF_BIT        0x0001   /* Terminal Flag  */
#define BC_RETRY_SSF           0x2
#define BC_RETRY_SSF_BIT       0x0004   /* SubSystem Flag */
#define BC_RETRY_BUSY          0x4
#define BC_RETRY_BUSY_BIT      0x0008   /* Busy Bit Set   */
#define BC_RETRY_SRQ           0x8
#define BC_RETRY_SRQ_BIT       0x0100   /* Service Request*/
#define BC_RETRY_INSTR         0x10
#define BC_RETRY_INSTR_BIT     0x0200   /* Instrumentation*/
#define BC_RETRY_ME            0x20
#define BC_RETRY_ME_BIT        0x0400   /* Message Error  */
#define BC_RETRY_ALTB          0x8888   /* Setup 1 retry on alternate bus */

/***********************************************************************
* Retry on Data Quality Errors Interrupt status word errors
***********************************************************************/
#define BC_RETRY_INV_WRD       0x40
#define BC_RETRY_INV_WRD_BIT   0x00000002  /* Invalid word        */
#define BC_RETRY_INV_SYNC      0x80
#define BC_RETRY_INV_SYNC_BIT  0x00000008  /* Inverted sync       */
#define BC_RETRY_MID_BIT       0x100
#define BC_RETRY_MID_BIT_BIT   0x00000010  /* Mid-bit             */
#define BC_RETRY_TWO_BUS       0x200
#define BC_RETRY_TWO_BUS_BIT   0x00000020  /* Two-bus error       */
#define BC_RETRY_PARITY        0x400
#define BC_RETRY_PARITY_BIT    0x00000040  /* Parity error        */
#define BC_RETRY_CONT_DATA     0x800
#define BC_RETRY_CONT_DATA_BIT 0x00000080  /* Non-contiguous data */
#define BC_RETRY_EARLY_RSP     0x1000
#define BC_RETRY_EARLY_RSP_BIT 0x00000100  /* Early response      */
#define BC_RETRY_LATE_RSP      0x2000
#define BC_RETRY_LATE_RSP_BIT  0x00000200  /* Late response       */
#define BC_RETRY_BAD_ADDR      0x4000
#define BC_RETRY_BAD_ADDR_BIT  0x00000400  /* Bad RT address      */
#define BC_RETRY_WRONG_BUS     0x8000
#define BC_RETRY_WRONG_BUS_BIT 0x00002000  /* Resp on wrong bus   */
#define BC_RETRY_NO_GAP        0x10000
#define BC_RETRY_NO_GAP_BIT    0x00008000  /* No/short gap        */
#define BC_RETRY_NRSP_RCV      0x20000
#define BC_RETRY_NRSP_RCV_BIT  0x00800000  /* No RSP on RT-RT RCV */
#define BC_RETRY_NRSP          0x40000
#define BC_RETRY_NRSP_BIT      0x04000000  /* No response         */ 

#define BC_RETRY_BIT_COUNT 0x40000

#define API_BC_BRANCH_CMD1         0
#define API_BC_BRANCH_CMD2         1
#define API_BRANCH_MSTAT1          2
#define API_BC_BRANCH_MSTAT2       3  
#define API_BC_BRANCH_INT_STATUS   50
#define API_BC_BRANCH_DATA         4 

/********************************************************
*  Bus Monitor Definitions
********************************************************/

#define BM_ADDRESS_COUNT   32
#define BM_SUBADDR_COUNT   32
#define BM_TRANREC_COUNT   2

// Define the number of BM messages which trigger recording to disk/memory.
#define BM_MAX_MSGS_PER_READ_BLOCK   0x1020

/************************************************
*  API BM Control Buffer
************************************************/

typedef struct api_bm_cbuf
   {
   union mode
      {
      BT_U32BIT wcount;           // enabled word counts (bit field)
      BT_U32BIT modecode;         // enabled mode codes
      } t;
   BT_U16BIT pass_count;          // number of passes before interrupt
   }
API_BM_CBUF;

/************************************************
*  BM control buffer "mode_code" bits
************************************************/

#define BM_FILTER_MC_DBC     0x00000001L // dynamic bus control
#define BM_FILTER_MC_SYNC    0x00000002L // synchronize without data word
#define BM_FILTER_MC_TSWS    0x00000004L // transmit status word
#define BM_FILTER_MC_STST    0x00000008L // initiate self test
#define BM_FILTER_MC_XSD     0x00000010L // transmitter shutdown
#define BM_FILTER_MC_OXSD    0x00000020L // override transmitter shutdown
#define BM_FILTER_MC_ITF     0x00000040L // inhibit terminal flag bit
#define BM_FILTER_MC_OITF    0x00000080L // Override inhibit terminal flag bit
#define BM_FILTER_MC_RRT     0x00000100L // Reset remote terminal

#define BM_FILTER_MC_TVWC    0x00010000L // Transmit vector word
#define BM_FILTER_MC_SWO     0x00020000L // Synchronize with data word
#define BM_FILTER_MC_TLC     0x00040000L // Transmit last command
#define BM_FILTER_MC_TBW     0x00080000L // Transmit bit word
#define BM_FILTER_MC_SXSD    0x00100000L // Selected transmitter shutdown
#define BM_FILTER_MC_OSXS    0x00200000L // Override selected transmitter shutdown

/************************************************
*  BM control buffer "control_word" bits
************************************************/

#define BM_CBUF_ENABLE     0x0001
#define BM_CBUF_INTERACT   0x0002
#define BM_CBUF_RECORD     0x0004
#define BM_CBUF_SBUFF      0x0008

/**********************************************************************
*  Response time for API BM message buffers
**********************************************************************/

typedef struct bt1553_bmresponse
   {
   unsigned char time;      // Response time, 0.5 usec LSB (6 bits)
   }
BT1553_BMRESPONSE;

/**********************************************************************
*  BM Message Buffer
*        - API_BM_MBUF: BM data that has been re-formatted into a
*          structure.  If this structure is changed, it may be
*          necessary to change BM_MessageConvert(), which depends on
*          the order of these parameters.  The BM Record file which is
*          created by BusTools contains multiple instances of this
*          structures, one for each message recorded.  The file contains
*          no additional header or trailer.
**********************************************************************/

#define BT1553_MBUFCOUNT  32      /* Data words in api_bm_mbuf struct */

typedef struct api_bm_mbuf
   {
   BT_U32BIT         messno;     // Message number (generated by API, 1-based)
   BT_U32BIT         int_status; // Interrupt status from board
   BT1553_TIME       time;       // Time of message (64-bits, 1 us or 1 ns LSB)
   BT1553_COMMAND    command1;   // 1553 command word #1 (Receive for RT-RT)
   BT_U16BIT         status_c1;  // 1553 command word #1 error status
   BT1553_COMMAND    command2;   // 1553 command word #2 (Transmit for RT-RT)
   BT_U16BIT         status_c2;  // 1553 command word #2 error status

   BT1553_BMRESPONSE response1;  // 1553 response time #1 (byte)
   BT1553_BMRESPONSE response2;  // 1553 response time #2 (byte)
   BT1553_STATUS     status1;    // 1553 status word #1 (Transmit for RT-RT or Broadcast RT-RT) (V4.05)
   BT_U16BIT         status_s1;  // 1553 status word #1 error status

   BT1553_STATUS     status2;    // 1553 status word #2 (Receive for RT-RT, NULL for Broadcast RT-RT) (V4.05)
   BT_U16BIT         status_s2;  // 1553 status word #2 error status

   BT_U16BIT   value[BT1553_MBUFCOUNT];   // 1553 data words
   BT_U16BIT   status[BT1553_MBUFCOUNT];  // 1553 status for data words
   }
API_BM_MBUF;

/**********************************************************************
*  BM Trigger Buffer definition
*
*     4 - Start Trigger Events (only 3 Active) A - B - C - D(Not Used)
*     4 - Stop Trigger Events (only 3 Active) E - F - G - H(Not Used)
* 
*
*     control: dialog box entries for order of events:
*                         START SIDE                  STOP SIDE
*                    0 -> unconditionally             never
*                    1 -> if A                        if E
*                    2 -> if A AND B                  if E AND F
*                    3 -> if A OR B                   if E OR F
*                    4 -> if A WITH B (same message)  if E WITH F (same message) 
*                    5 -> if B armed by A             if F armed by E
*                    6 -> if A WITH B and C           if E WITH F and G
*                    7 -> if A WITH B or C            if E WITH F or G
*                    8 -> if C armed by A WITH B      if G armed by E WITH F
*                    9 -> if A WITH B armed by C      if E WITH F armed by G
*                   10 -> if A and B and C            if E and F and G
*                   11 -> if A or B or C              if E or E or G
*                   Note: WITH means in the same message. AND and OR means
*                         in the same or different message.
*
*     type:    trigger type:
*                    0 -> none
*                    1 -> command
*                    2 -> status
*                    3 -> data
*                    4 -> Not used
*                    5 -> interrupt status lsb
*                    6 -> interrupt status msb
*
*     mask:    16 bit mask (applied prior to compare)
*     value:   16 bit value to compare
*     word:    word number (data trigger - 3) or offset (user offset trigger - 5)
*     count:   number of times event must occur before event_occured bit is set
*
**********************************************************************/

typedef struct api_bm_tbuf
   {
   BT_U16BIT     trig_ext;         // 1 -> external trigger input enabled
                                   // 2 -> external output on every BM interrupt
                                   //  (rest of structure is ignored unless zero)
   BT_U16BIT     trig_err;         // 1 -> trigger on errors
   BT_U16BIT     trig_ext_output;  // 1 -> external output on trigger
                                   // 2 -> repetitive external output on trigger
   BT_U16BIT     control1;         // control of trigger start
   BT_U16BIT     control2;         // control of trigger stop
   struct
      {                 // Start events programmed here
      BT_U16BIT  type;  // Event type which causes trigger
      BT_U16BIT  mask;  // Specified 1553 word compared with "word" under this mask
      BT_U16BIT  value; // Value of the command, status or data word for trigger
      BT_U16BIT  word;  // Word number within message to test
      BT_U16BIT  count; // Number of times event occurs before Start declared
      }
   capture[4];     // Two Start events are supported by the old firmware.
   struct
      {                 // Stop events programmed here
      BT_U16BIT  type;  // Event type which causes trigger
      BT_U16BIT  mask;  // Specified 1553 word compared with "word" under this mask
      BT_U16BIT  value; // Value of the command, status or data word for trigger
      BT_U16BIT  word;  // Word number within message to test
      BT_U16BIT  count; // Number of times event occurs before Stop declared
      }
   stop[4];             // Two Stop events are supported by the old firmware.
   }
API_BM_TBUF;

/**********************************************************************
*  Hardware Interrupt Queue Definitions
**********************************************************************/
#define INT_QUE_PTR_REG    2*(0x40+0x13) // HW interrupt queue pointer

/************************************************
*  Definition of interrupt queue mode/type bits
************************************************/

typedef struct bt1553_intmode
   {
#ifdef NON_INTEL_BIT_FIELDS
   BT_U16BIT unused:7;   // unused                    (MSB)
   BT_U16BIT bc_ctl:1;   // BC control interrupt     0x0100
   BT_U16BIT bm_swap:1;  // BM-Only Buffer Swap*     0x0080
   BT_U16BIT ext_trig:1; // External Trigger         0x0040
   BT_U16BIT bmtrig:1;   // bm trigger has occurred  0x0020
   BT_U16BIT bc:1;       // bc interrupt             0x0010
   BT_U16BIT bm:1;       // bm interrupt             0x0008
   BT_U16BIT rt:1;       // rt interrupt             0x0004
   BT_U16BIT timer:1;    // timer overflow or load   0x0002
   BT_U16BIT iack:1;     // interrupt acknowledge bit (LSB)
#else  /* INTEL-Compatable bit field ordering */
   BT_U16BIT iack:1;     // interrupt acknowledge bit (LSB)
   BT_U16BIT timer:1;    // timer overflow or load   0x0002
   BT_U16BIT rt:1;       // rt interrupt             0x0004
   BT_U16BIT bm:1;       // bm interrupt             0x0008
   BT_U16BIT bc:1;       // bc interrupt             0x0010
   BT_U16BIT bmtrig:1;   // bm trigger has occurred  0x0020
   BT_U16BIT ext_trig:1; // External Trigger         0x0040
   BT_U16BIT bm_swap:1;  // BM-Only Buffer Swap*     0x0080
   BT_U16BIT bc_ctl:1;   // BC control interrupt     0x0100
   BT_U16BIT unused:7;   // unused                    (MSB)
#endif
   }
BT1553_INTMODE;

#define NO_INTERRUPT            0   //No interrupt present
#define TTIMER_LOAD_INTERRUPT   1   //Tag timer load interrupt (BC,RT,BM)
#define TRIGGER_IN_INTERRUPT    2   //Trigger input interrupt (BC,RT)
#define BC_MESSAGE_INTERRUPT    3   //BC Message interrupt (BC)
#define BC_CNTRLWD_INTERRUPT    4   //BC control word interrupt (BC)
#define RT_MESSAGE_INTERRUPT    5   //RT Message interrupt (RT)
#define BM_MESSAGE_INTERRUPT    6   //BM Message interrupt (BM)
#define BM_TRIGGER_INTERRUPT    7   //BM Message and trigger interrupt (BM)
#define MF_OVFL_INTERRUPT       8   //Minor Frame overflow interrupt (BC)   
#define BC_BSY_MFOVFL_INTERRUPT 9   //BC busy minor frame overflow interrupt. BC busy set at start of frame (BC)
#define LP_MF_OVFL_INTERRUPT    10  //BC low priority aperiodic minor frame overflow (BC)
#define HP_MF_OVFL_INTERRUPT    11  //BC high priority aperiodic minor frame overflow (BC)
#define BM_HW_OVRFLW_INTERRUPT  12  //BM overflow detected head=tail

/************************************************
*  Definition of interrupt queue memory block
************************************************/

typedef struct iq_mblock_v6
   {
      BT_U32BIT mode;         //   
      BT_U32BIT msg_ptr;      // points to message that generated the interrupt
   }IQ_MBLOCK_V6;

typedef struct iq_mblock
   {
   union
      {
      BT_U16BIT      modeword;
      BT1553_INTMODE mode;
      } t;                 // Interrupt mode/type bits
   BT_U16BIT msg_ptr;      // points to message that generated the interrupt
   BT_U16BIT nxt_int;      // points to the next interrupt in the queue
   }
IQ_MBLOCK;

/**********************************************************************
*  Error Injection Definitions
**********************************************************************/

#define EI_COUNT 33  /* Number of error injection words supported by SW */
                     /* The HW supports one more, for the 33rd data word*/

/***********************************************
*  Error Injection definition buffer
***********************************************/

typedef struct api_eibuf
   {
   BT_U16BIT buftype;     // error injection buffer type
   struct
      {
      BT_U8BIT etype;     // error code (e.g. EI_NONE, EI_PARITY, etc)
      BT_U8BIT edata;     // error data (if req'd)
      }
      error[EI_COUNT];
   }
API_EIBUF;

typedef struct api_enh_eibuf
   {
   BT_U16BIT buftype;     // error injection buffer type
   struct
      {
      BT_U8BIT etype;     // error code (e.g. EI_NONE, EI_PARITY, etc)
      BT_UINT edata;      // error data (if req'd)
      }
      error[EI_COUNT];
   }
API_ENH_EIBUF;

/***********************************************
*  Error Injection buffer types
***********************************************/

#define EI_BC_REC       0  // error injection for BC on a receive message
#define EI_BC_TRANS     1  // error injection for BC on a transmit message
#define EI_BC_RTTORT    2  // error injection for BC on a RT-TO-RT message
#define EI_RT_REC       3  // error injection for RT on a receive message
#define EI_RT_TRANS     4  // error injection for RT on a transmit message

/***********************************************
*  Error Injection codes coming from user
***********************************************/

#define EI_NONE              0        // "[None]"
#define EI_PARITY            2        // "Parity (RT, BC)"
#define EI_MIDSYNC           3        // "Mid sync zero crossing (RT, BC)"
#define EI_MIDBIT            4        // "Mid bit zero crossing (RT, BC)"
#define EI_BITCOUNT          5        // "Bit count (RT, BC)"
#define EI_WORDCOUNT         6        // "Word count (RT, BC)"
#define EI_SYNC              7        // "Invert Sync (RT, BC)"
#define EI_DATAWORDGAP       8        // "Data word gap (RT, BC)"
#define EI_LATERESPONSE      9        // "Programmable response (RT)"
#define EI_RESPWRONGBUS      11       // "Response wrong bus (RT)"
#define EI_NOINTERMSGGAP     12       // "No intermessage gap (BC)"
#define EI_BADADDR           13       // "Respond to wrong address (RT)"
#define EI_MIDPARITY         14       // "Mid Parity Zero Crossing (RT, BC)
#define EI_BIPHASE           15       // "Bi-phase error (RT, BC)"

/************************************************
*  RT Val Error injection hardware definitions
************************************************/
#define EI_ENH_ZEROXNG       16     // Enhanced Zero-Crossing error
#define EI_BIPHASE_LOW      116     // Biphase error: waveform doesn't go high
#define EI_BIPHASE_HI       117     // Biphase error: waveform doesn't go low
#define EI_BIPHASE_PAR_LOW  118     // Biphase "low" error on the parity bit
#define EI_BIPHASE_PAR_HI   119     // Biphase "high" error on the parity bit
#define EI_SYNC3B           120     // "Sync Error : 111100. errdata = 11.1011. 
#define EI_SYNC37           121     // "Sync Error : 110000  errdata = 11.0111
#define EI_SYNC3E           122     // "Sync Error : 111001  errdata = 11.1110
#define EI_SYNC1F           123     // "Sync Error : 011000  errdata = 01.1111
#define EI_TCENH_ZEROXNG    124
#define EI_BS3_FIXER        125

/****************************************************************************
*  Remote Terminal Definitions
****************************************************************************/

#define RT_ADDRESS_COUNT 32
#define RT_SUBADDR_COUNT 32
#define RT_TRANREC_COUNT 2
#define RT_SUBUNIT_COUNT 2048

/************************************************
*  API RT Address Buffer (read/write)
************************************************/

typedef struct api_rt_abuf
   {
   BT_U16BIT enable_a;             // enable channel a; 1=enable
   BT_U16BIT enable_b;             // enable channel b; 1=enable
   BT_U16BIT inhibit_term_flag;    // inhibit terminal flag plus misc (see below)
   BT_U16BIT status;               // latest RT status word
   BT_U16BIT command;              // latest command word.V4.27
   BT_U16BIT bit_word;             // latest built in test (BIT) word
   }
API_RT_ABUF;

// Define the bits in the inhibit terminal flag control word:
#define RT_ABUF_MON_ENA    0x0001
#define RT_ABUF_DBC_ENA    0x0002  /* Set this bit to enable Dynamic Bus Acceptance       */
#define RT_ABUF_ITF        0x0004  /* Set this bit to inhibit terminal flag               */
#define RT_ABUF_MBUF_BWD   0x0010  /* Select BIT WORD stored in message buffer.           */ 
#define RT_ABUF_EXT_STATUS 0x0100  /* Select Extended status mode for the RT address      */
#define RT_ABUF_DBC_RT_OFF 0x8000  /* Set this bit to turn off a specific RT on valid DBA */ 

#define RT_MONITOR_ENABLE  1
#define RT_MONITOR_DISABLE 0

/***********************************************
*  FLASH data offsets
************************************************/
#define FLASH_SERIAL_NUMBER_OFFSET 0
#define FLASH_BOARD_SPECIFIC_DATA_OFFSET 1

/************************************************
*  API RT Control Buffer - non-broadcast (read/write)
************************************************/
typedef struct api_rt_cbuf
   {
   BT_U32BIT legal_wordcount;      // legal word count bit mask
   }
API_RT_CBUF;

/************************************************
*  API RT Control Buffer - broadcast (read/write)
************************************************/

typedef struct api_rt_cbufbroad
   {
   BT_U32BIT legal_wordcount[31]; // legal word count bits for each RT address,
                                  //   for RT addresses 0 - 31.
   BT_U32BIT mbuf_count;          // Number of message buffers to allocate,
   }                              //  or zero to use the default.V4.39
API_RT_CBUFBROAD;

/***********************************************
*  API RT Message Buffer (read from hardware)
************************************************/

typedef struct api_rt_mbuf_read
   {
   BT_U32BIT        status;          // interrupt status
   BT_U32BIT        reserved;        //
   BT1553_TIME      time;            // BusTools 64-bit time

   // Contents of real 1553 message to/from bus

   BT1553_COMMAND mess_command;    // 1553 command word
   BT1553_STATUS  mess_status;     // 1553 status word
   BT_U16BIT      mess_data[32];   // data words
   BT_U32BIT      spare;           // spare data word for word count errors
   }
API_RT_MBUF_READ;

/************************************************
*  API RT Message Buffer (write to hardware)
************************************************/

typedef struct api_rt_mbuf_write
   {
   BT_U32BIT enable;               // interrupt enable bits
   BT_U32BIT error_inj_id;         // id of error injection buffer
   BT_U16BIT mess_data[32];        // data words
   }
API_RT_MBUF_WRITE;

/**********************************************************************
*  Playback Function Structure Definition and constants.
*  Playback is only supported in 32-bit Windows (95/98/NT/2000)
* This structure contains mis-aligned long's!
**********************************************************************/
typedef struct api_playback_status
   {
   BT_U32BIT tailPointer;
   BT_U16BIT playbackStatus;
   BT_INT    playbackError;
   BT_U32BIT recordsProcessed;
   }
API_PLAYBACK_STATUS;

#define PB_LOGFILE_LEN 512
typedef struct api_playback
   {
   BT1553_TIME timeStart;          // Start Tag Time
   BT1553_TIME timeStop;           // Stop Tag Time
   BT_U32BIT   messageStart;       // Starting message number
   BT_U32BIT   messageStop;        // Ending message number
   BT_INT      filterFlag;         // 0 no filter, 1 = time filter, 2 = message filter
   BT_U32BIT   activeRT;           // Active RTs 0 = do not playback; 1 = use in playback
   BT_INT      Subadress31Flag;    // 0 = SA 31 is not a mode code, 1 = SA31 is mode code
   BT_INT      Rt31Flag;           // 0 = RT 31 is not broadcast, 1 = RT 31 is broadcast
   API_PLAYBACK_STATUS *status;    // Status structure to monitor playback
   char        fileName[PB_LOGFILE_LEN];      // Log file name
   }
API_PLAYBACK;

#define PB_TT_CONVERT    0x0001


#define REGISTER_FUNCTION       1
#define UNREGISTER_FUNCTION     0

/**********************************************************************
*  Interrupt Function Structure Definition and constants.
*  Zero this structure, then setup the function-required fields before
*   calling BusTools_RegisterFunction().
**********************************************************************/
#define MAX_FIFO_LEN     256   /* Size of the event FIFO (power of 2!)*/
#if (MAX_FIFO_LEN - 1) & MAX_FIFO_LEN
#error The MAX_FIFO_LEN parameter is not a power of 2!
#endif 

typedef struct api_int_fifo
   {
   /**************************************************************************
   *  Parameters setup by the user before calling BusTools_RegisterFunction()
   **************************************************************************/
   // Pointer to user interrupt thread function:
   BT_INT (_stdcall *function)(BT_UINT cardnum, struct api_int_fifo *pFIFO);
                          // Function should return API_SUCCESS if thread is to
                          //  continue operation, any other value will cause
                          //  the thread and event object to be distroyed.
   // User-requested thread priority:
   int     iPriority;     // THREAD_PRIORITY
   DWORD   dwMilliseconds;// Thread time-out interval in milliseconds or INFINITE
   // Mask to request startup and shutdown notification:
   BT_INT  iNotification; // CALL_STARTUP if function to be called at creation
                          //  of thread, CALL_SHUTDOWN if function is to be
                          //  called upon distruction of thread.  "OR" both
                          //  together to enable notification on both events.
                          // See "bForceShutdown" and "bForceStartup" below.

   // User variables; not referenced by the API:
   int     nUser[8];      // Spare variables for use by the user.
   void   *pUser[8];      // Spare variables for use by the user.

   /**************************************************************************
   *  Event filter structure.  A one "1' enables the specified event; when
   *   detected the API will place it in the FIFO and call the user function.
   **************************************************************************/
   // Top Level Notification Event Mask:
   BT_UINT  FilterType;    // One or more EVENT_ definitions, "or'ed" together.
   // Event filter mask array.
   //                  rt  tr sa
   BT_UINT  FilterMask[32][2][32];   // The bits in the word form the word count
                                     // mask; bit 0 is 32 words, bit 1 is 1 word,
                                     //       bit 2 is 2 words, etc.  Bit set
                                     //       enables combination.

   /**************************************************************************
   *  Parameters setup by BusTools_RegisterFunction()
   **************************************************************************/
   // Reason codes explain why user function is being called.
   //  If both bForceShutdown and bForceStartup are zero,
   //   function is being called to process events.                       user:
   int     bForceShutdown;// 1 - Thread is being shutdown, -1 complete    (RO) $
   int     bForceStartup; // 1 - Thread is being started, 0 complete      (RO) $
   int     nPtrIndex;     // Index into API pointer table                 (RO) $
   BT_UINT cardnum;       // Card number associated with this thread.     (RO) $
   BT_UINT numEvents;     // Total number of events, including overflows  (RW) $
   BT_UINT queue_oflow;   // Count incremented by API when FIFO overflows (RW) $

   CEI_THREAD hThread;    // Handle to thread                             (RO) $
   CEI_EVENT hEvent;      // Handle to event object                       (RO) $
   CEI_EVENT hkEvent;     // Kernel mode handle to event object           (RO) $
   DWORD   lThreadId;     // ID of new user interrupt thread.             (RO) $

   //  Note that a "$" indicates that the API initializes this parameter.
   /***************************************************************************
   *  FIFO of events, to be processed by user function.  The API enters events
   *  at the head_index, then increments head_index.  The user function should
   *  compare head_index with tail_index.  If equal, the FIFO is empty and the
   *  user function should return to wait for more events (return API_SUCCESS).
   *
   *  If head_index != tail_index, then the user function should process the
   *  FIFO entry indexed by tail_index, increment tail_index by one and
   *  bit-wise "AND" it with mask_index, saving the resulting value back into
   *  tail_index.
   *  The user function should then compare head_index with the updated value
   *  of tail_index, and if not equal, process the next entry and update the
   *  value of tail_index, etc., otherwise return "API_SUCCESS" to wait for
   *  more events.
   ***************************************************************************/
   BT_INT  head_index;  // Index of element being added to queue (0->63)  (RO) $
   BT_INT  tail_index;  // Index of element to be removed from queue      (RW) $
   BT_INT  mask_index;  // Mask for wrapping head and tail pointers       (RO) $
   struct  BT_FIFO      // FIFO structure: events for user to process.         $
   {
      BT_INT  event_type;  // EVENT_ definitions below.
      BT_INT  buffer_off;  // Byte offset of message buffer which caused event
      BT_INT  rtaddress;   // Terminal address of message.  If > 31, error code.
      BT_INT  transrec;    // Non-zero if transmit message, zero for receive
      BT_INT  subaddress;  // Subaddress of message
      BT_INT  wordcount;   // Word count of message; 0-31; 0 indicates 32 words
                           // unless mode code (then indicates mode code number)
      BT_INT  bufferID;    // Buffer ID number or message ID number.
      BT_UINT int_status;  // Contains the INT_STATUS. 
   }
   fifo[MAX_FIFO_LEN];     // FIFO has exactly 256 entries.

   BT_U32BIT  EventMask[32][2][32];  // The bits correspond to the int status words
   BT_U32BIT EventInit;
   CEI_MUTEX mutex;
   BT_UINT timeout;                  //timeout on timed wait
   } API_INT_FIFO;

#define CALL_STARTUP      0x0001   /* Thread created and initialized        */
#define CALL_SHUTDOWN     0x0004   /* Thread shutdown has been requested    */
#define USE_INTERRUPT_MASK 0x12345678
#define NO_ERRORS 0x87654321
#define MAX_REGISTER_FUNCTION  64  /* Max number of registered functions    */

/*****************************************************************************
*  Event filter specification values.  When specified event is detected
*  the API places it in the FIFO and calls the user function.
*****************************************************************************/
#define EVENT_IMMEDIATE    0x00000f  /* unqualified interrupt handled by user  */
#define EVENT_EXT_TRIG     0x000001  /* interrupt on external trigger input    */
#define EVENT_TIMER_WRAP   0x000002  /* Tag Timer overflow or discrete input   */
#define EVENT_RT_MESSAGE   0x000004  /* RT message transacted                  */
#define EVENT_BM_MESSAGE   0x000008  /* BM message transacted                  */
#define EVENT_BC_MESSAGE   0x000010  /* BC message transacted                  */
#define EVENT_BM_TRIG      0x000020  /* BM trigger event (start/stop)          */
#define EVENT_BM_START     0x000040  /* BM started (BusTools_BM_StartStop)V4.01*/
#define EVENT_BM_STOP      0x000080  /* BM stopped (BusTools_BM_StartStop)     */    
#define EVENT_BC_START     0x000100  /* BC started (BusTools_BC_StartStop)     */
#define EVENT_BC_STOP      0x000200  /* BC stopped (BusTools_BC_StartStop)     */
#define EVENT_RT_START     0x000400  /* BC started (BusTools_RT_StartStop)     */
#define EVENT_RT_STOP      0x000800  /* BC stopped (BusTools_RT_StartStop)     */
#define EVENT_RECORDER     0x001000  /* BM recorder buffer has 64K or timeout  */
#define EVENT_MF_OVERFLO   0x002000  /* Minor frame timing overflow            */
#define EVENT_API_OVERFLO  0x004000  /* BM API Recorder buffer overflowed      */
#define EVENT_HW_OVERFLO   0x008000  /* BM HW Recorder buffer overflowed       */
#define EVENT_BC_CONTROL   0x010000  /* BC Control block (NOOP condition stop) */
#define EVENT_LP_MF_OVFL   0x020000
#define EVENT_HP_MF_OVFL   0x040000
#define EVENT_BC_BSY_OVFL  0x080000
#define EVENT_BM_OVRFLW    0x100000

/**********************************************************************
*  Bus Statistics structure.  This structure contains information
*   about bus loading and bus errors.
**********************************************************************/
#if defined(DO_BUS_LOADING)
typedef struct api_bus_stats
   {
   BT_U16BIT StatusWordRt[32];             // Current RT status word [RT] (logical OR of status words)
   BT_U32BIT CommandCountRtBus[32][2];     // Current Command Counts [RT][BUS]
   BT_U32BIT ErrorCountRtBus[32][2];       // Current Error Counts [RT][BUS]
   BT_U32BIT NrCountRtBus[32][2];          // Current No Response Counts [RT][BUS]
   BT_U32BIT MeCountRtBus[32][2];          // Current Message Error Counts [RT][BUS]
   BT_U32BIT ErrorBitMapRt[32];            // Current Error Bit Map [RT]  (logical OR of errors)
   BT_U32BIT CommandCountRtSaBus[32][32][2];// Current Command Counts [RT][SA][BUS]
   BT_U32BIT ErrorCountRtSaBus[32][32][2]; // Current Error Counts [RT][SA][BUS]
   BT_U32BIT NrCountRtSaBus[32][32][2];    // Current No Response Counts [RT][SA][BUS]
   BT_U32BIT MeCountRtSaBus[32][32][2];    // Current Message Error Counts [RT][SA][BUS]
   BT_U32BIT ErrorBitMapRtSa[32][32];      // Current Error Bit Map [RT][SA]  (logical OR of errors)

   BT_U32BIT ErrorsRTtrSABus[32][2][2];    // Current Error Flags [RT][TR][BUS](one bit per SA)
   BT_U32BIT NRRTtrSABus[32][2][2];        // Current No Response Flags [TR][BUS] (one bit per SA)

   BT_U32BIT ActivityRTtrSABus[32][2][2];  // Current Activity Flags [RT][TR][BUS](one bit per SA)
   BT_U32BIT ActivityRTtrBus[2][2];        // Current Activity Flags [TR][BUS] (one bit per RT)

   BT_U32BIT BusCountsRTSA[32][32];        // Current Activity Counts [RT][SA] (API use only)
   BT_U32BIT BusCountsRT[32];              // Current Activity Counts [RT]     (API use only)
   BT_U32BIT BusCounts;                    // Current Activity Counts (all RT) (API use only)

   BT_U8BIT  BusLoadingRTSA[32][32];       // Filtered Half Percent Bus Loading [RT][SA]
   BT_U8BIT  BusLoadingRT[32];             // Filtered Half Percent Bus Loading [RT]
   BT_U32BIT BusLoading;                   // Filtered Half Percent Bus Loading (all RT/SA)
   }
API_BUS_STATS;
#endif

/************************************************
*  MIL-STD-1553 Status Word definitions
************************************************/

#define API_1553_STAT_ME   0x0400  // message error
#define API_1553_STAT_IN   0x0200  // instrumentation
#define API_1553_STAT_SR   0x0100  // service request
#define API_1553_STAT_BR   0x0010  // broadcast command received
#define API_1553_STAT_BY   0x0008  // busy bit
#define API_1553_STAT_SF   0x0004  // subsystem flag
#define API_1553_STAT_DB   0x0002  // dynamic bus acceptance
#define API_1553_STAT_TF   0x0001  // terminal flag

/**********************************************************************
*  Memory block id's for BusTools_GetAddr routine
**********************************************************************/

#define GETADDR_HWREG       1   // Hardware Registers              (1)
#define GETADDR_RAMREG      2   // RAM Registers                   (2)
#define GETADDR_BCMESS      3   // BC Message Buffers             (10)
#define GETADDR_BMFILTER    4   // BM Filter Buffer                (7)
#define GETADDR_BMTRIGGER   5   // BM Trigger Buffer               (3)
#define GETADDR_BMMESSAGE   6   // BM Message Buffers              (9)
#define GETADDR_BMCONTROL   7   // BM Control Buffers              (8)
#define GETADDR_BMDEFCBUF   8   // BM Default Control Buffer       (4)
#define GETADDR_RTADDRESS   9   // RT Address Buffer              (15)
#define GETADDR_RTFILTER    10  // RT Filter Buffer               (16)
#define GETADDR_RTDATA      11  // RT Control and Message Buffers (11)
#define GETADDR_EI          12  // Error Injection data area       (6)
#define GETADDR_IQ          13  // Interrupt queue data area       (5)
#define GETADDR_RTMBUF_DEF  14  // RT Default Message Buffers     (14)
#define GETADDR_RTCBUF_DEF  15  // RT Default Control Buffers     (13)
#define GETADDR_RTCBUF_BRO  16  // RT Broadcast Control Buffers   (12)
#define GETADDR_PCI_RTDATA  17  // RT MBUF's, PCI-1553 only       (17)
#define GETADDR_DIFF_IO     18  // Discrete IO registers          (19)

#define GETADDR_COUNT       18  // Number of address block types

#define DUMP_HWREG      1 << GETADDR_HWREG       // Hardware Registers             
#define DUMP_RAMREG     1 << GETADDR_RAMREG      // RAM Registers             
#define DUMP_BCMESS     1 << GETADDR_BCMESS      // BC Message Buffers            
#define DUMP_BMFILTER   1 << GETADDR_BMFILTER    // BM Filter Buffer              
#define DUMP_BMTRIGGER  1 << GETADDR_BMTRIGGER   // BM Trigger Buffer             
#define DUMP_BMMESSAGE  1 << GETADDR_BMMESSAGE   // BM Message Buffers             
#define DUMP_BMCONTROL  1 << GETADDR_BMCONTROL   // BM Control Buffers             
#define DUMP_BMDEFCBUF  1 << GETADDR_BMDEFCBUF   // BM Default Control Buffer     
#define DUMP_RTADDRESS  1 << GETADDR_RTADDRESS   // RT Address Buffer   
#define DUMP_RTFILTER   1 << GETADDR_RTFILTER    // RT Filter Buffer   
#define DUMP_RTDATA     1 << GETADDR_RTDATA      // RT Control and Message Buffers 
#define DUMP_EI         1 << GETADDR_EI          // Error Injection data area  
#define DUMP_IQ         1 << GETADDR_IQ          // Interrupt queue data area    
#define DUMP_RTMBUF_DEF 1 << GETADDR_RTMBUF_DEF  // RT Default Message Buffers   
#define DUMP_RTCBUF_DEF 1 << GETADDR_RTCBUF_DEF  // RT Default Control Buffers 
#define DUMP_RTCBUF_BRO 1 << GETADDR_RTCBUF_BRO  // RT Broadcast Control Buffers  
#define DUMP_PCI_RTDATA 1 << GETADDR_PCI_RTDATA  // RT MBUF's, PCI-1553 only  
#define DUMP_DIFF_IO    1 << GETADDR_DIFF_IO     // Discrete IO registers  

#define DUMP_ALL 0xffffffff
#define DUMP_RT DUMP_RTFILTER | DUMP_RTADDRESS | DUMP_RTDATA | DUMP_RTMBUF_DEF | DUMP_RTCBUF_DEF | DUMP_RTCBUF_BRO | DUMP_PCI_RTDATA

/**********************************************************************
	Register defines for V6 boards
**********************************************************************/
#define P1_FRONT_IO_CONNECTOR 1
#define P14_REAR_IO_CONNECTOR 2
#define P16_READ_IO_CONNECTOR 4

/************************************************************************
* Global Register WORD offsets  HIF
************************************************************************/
#define GLBREG_CSC_OFFSET      0x0
#define GLBREG_BD_SPECIFIC     0x1
#define GLBREG_RTADDR1         0x2
#define GLBREG_RTADDR2         0x3
#define GLBREG_CH1_OPT         0x4
#define GLBREG_CH2_OPT         0x5 
#define GLBREG_CH3_OPT         0x6 
#define GLBREG_CH4_OPT         0x7 
#define GLBREG_CH5_OPT         0x8 
#define GLBREG_CH6_OPT         0x9 
#define GLBREG_CH7_OPT         0xa 
#define GLBREG_CH8_OPT         0xb
#define GLBREG_BD_SERIAL_NUM   0xc 
#define GLBREG_DISCR_OPT       0xd
#define GLBREG_DIFF_OPT        0xe
#define GLBREG_PIO_OPT         0xf

#define GLBREG_FPGA_REV        0x20
#define GLBREG_INT_STAT        0x21
#define GLBREG_RAMSTART        0x22
#define GLBREG_RAMSIZE         0x23
#define GLBREG_LED_CTL         0x24
#define GLBREG_TMP_SEN_READ    0x25
#define GLBREG_TMP_SEN_WRITE   0x26
#define GLBREG_RESET_ENABLE    0x27
#define GLBREG_DISC_OUT        0x30
#define GLBREG_DSIC_IN         0x31
#define GLBREG_RS485_CTL       0x32
#define GLBREG_RS485_DATA      0x33
#define GLBREG_EXT_RT_ADDR     0x34
#define GLBREG_DAC_CTRL        0x36
#define GLBREG_PIO_CTRL        0x37

#define GLBREG_IRIG_CTRL       0x50
#define GLBREG_IRIG_TOY        0x51
#define GLBREG_RESET_TT        0x52

/************************************************************************
*  V6 Hardware register WORD offsets;  32-BIT word address HWREG
************************************************************************/
/* 0x10 - 0x4F Common Registers  */
#define HWREG_LPU_V6REVISION   0x10  // Read LPU Revision information              (R)
#define HWREG_BUILD_NUMBER     0x11  // Build Number                               (R)
#define HWREG_WCS_V6REVISION   0x12  // Microcode Revision register                (R)
#define HWREG_WCS_BUILD        0x13  // Microcode Build Number                     (R)
#define HWREG_HEART_BEATV6     0x14  // Heart Beat Register                        (R)
/* 0x15 reserved */
#define HWREG_CONTROL          0x16  // 1553 Control Reg. bits 31:0                (R/W)
#define HWREG_1553A_ENABLE     0x18  // Mil-Std-1553A Enables Register             (R/W)
#define HWREG_V6RESPONSE       0x1a  // response time out/late response            (R/W)
/* 0x1b - 0x2f reserved */
#define HWREG_FW_CNTRL         0x30  // Firmware Control Flags                     (R/W)
/* 0x31 - 0x37 Reserved */
/* 0x38 - 0x3f Interrupt Queue Registers */ 
#define HWREG_IQ_BUF_START     0x3a  // Interrupt Queue Buffer Start Pointer       (R/W)
#define HWREG_IQ_BUF_END       0x3b  // Interrupt Queue Puffer End Pointer         (R/W)
#define HWREG_IQ_HEAD_PTR      0x3c  // Interrupt Queue Head Pointer               (R/W)
/* 0x3d - 0x3f Not Used */
/* 0x40 - 0x67 Bus Controller Control Registers */
#define HWREG_BCMSG_PTR        0x40  // BC Message Initial Word Pointer            (R/W)
#define HWREG_BC_INT_ENABLE    0x41  // BC Interrupt Enables                       (R/W)
#define HWREG_MINOR_FRAMEV6    0x42  // BC Minor Frame Register                    (R/W)
/* 0x43 reserved  */
#define HWREG_BC_RETRY         0x44  // BC Retry Enable Bits                       (R/W)
#define HWREG_DQ_RETRY         0x45  // Rettry on Data Quality                     (R/W)
#define HWREG_BC_RETRY_CTL     0x46  // BC Retry Control Buffer                    (R/W)
/* 0x47 Reserved */
#define HWREG_BC_BUS_SWTCH     0x48  // BC Bus Switching offset                    (R/W)
#define HWREG_BC_TRIGGER       0x49  // BC Trigger setup                           (R/W)
#define HWREG_BC_HIGH_PRI_MSG  0x4a  // BC High Priority Aperiodic Message Pointer (R/W)
#define HWREG_BC_LOW_PRI_MSG   0x4b  // BC Low Priority Aperiodic Message Pointer  (R/W)
/* 0x4c reserved  */
#define HWREG_BC_APER_FLAG     0x4d  // BC Aperiodic List Flags                    (R/W)
/* 0x4f - 0x5f Reserved */
/* 0x60 - 0x77 Remote Terminal Control Registers */
#define HWREG_RT_HW_ADDR       0x60  // RT Hard-Wired Address                      (R)
#define HWREG_SRT_V6ADDR       0x61  // RT Address for Single-RT Mode              (R/W)
#define HWREG_RT_ENABLEA       0x62  // RT 31..0 bus A enables                     (R/W)
#define HWREG_RT_ENABLEB       0x63  // RT 31..0 bus B enables                     (R/W)
#define HWREG_RT_FILTER        0x64  // RT Filter Buffer Base Address              (R/W)
/* 0x65 reserved         */
#define HWREG_RT_MSG_PTR_XMT   0x66  // RT Message Transmit Word Pointer           (R/W)
#define HWREG_RT_MSG_PTR_RCV   0x67  // RT Message Receive Word Pointer            (R/W)
/* 0x68 - 0x74 Reserved  */
#define HWREG_BCR_REG          0x74  // BCR Register                               (R/W)                              
#define HWREG_ME_REG           0x75  // ME Register                                (R/W)
/* 0x76 - 0x77 Reserved */
#define HWREG_BSY_REG          0x78  // BSY Register                               (R/W)         
/* 0x76 - 0x7f Reserved  */
#define HWREG_RT_ABUF_ADDR     0x80  // 0x080 - 0xbf RT Abuf 
/***************************************************************************************
 * 0xc0 - 0xd3 Bus Monitor Control Regiseters                                          *
 ***************************************************************************************/
#define HWREG_BM_BUF_START     0xc0  // BM Start of Buffer Pointer A               (R/W)
#define HWREG_BM_BUF_END       0xc1  // BM End of Buffer Pointer A                 (R/W)
#define HWREG_BM_HEAD_PTR      0xc2  // BM Head Pointer                            (R/W)
#define HWREG_BM_TAIL_PTR      0xc3  // BM Tail Pointer                            (R/W)
#define HWREG_BM_FILTER        0xc4  // BM Filter Buffer Base Address              (R/W)
#define HWREG_BM_TRIGGER       0xc5  // BM Trigger Buffer Pointer                  (R/W)
#define HWREG_BM_INT_ENABLE    0xc6  // BM Interrupt enable                        (R/W)
/* 0xc7 - 0xd0 Reserved */
#define HWREG_BM_OVFL_CTR      0xd1  // BM overflow counter (words) register       (R/W)
#define HWREG_BM_CTRL          0xd2  // Enable/Disable BM hardware interrupts      (R/W)
/***************************************************************************************/
/* 0x90 - 0x93 Interrupt Queue Registers */
#define HWREG_PB_V6CONTROL     0xd4  // Playback Control Register                  (RW)
/* 0xd6 - 0xd7 Reserved */
#define PB_START_V6POINTER     0xd8  // Address of Playback Data Buffer
#define PB_END_V6POINTER       0xd9  // End of Playback Data Buffer
#define PB_TAIL_V6POINTER      0xda  // Current FW loc in PB Data Buffer
#define PB_HEAD_V6POINTER      0xdb  // Addr where host is adding data
#define PB_THRSHLD_V6CNT       0xdc  // Interrupt counter used by FW
#define PB_INT_V6STATUS        0xdd  // Number of FW interrupts generated
#define PB_INT_V6THRSHLD       0xde  // Number of words between FW interrupts
#define PB_CURRENT_V6WORD      0xdf  // Reserved for the Playback firmware

#define PB_SET   0x80000000
#define PB_CLEAR 0x40000000
/* 0xe0 - 0xff Reserved */

#define HWREG_V6COUNT 0x100


/* V6 Discrete Registers offset */
#define V6_DISCRETE_OUT 0x30
#define V6_DISCRETE_IN  0x31
#define V6_RS485_CTRL   0x32
#define V6_RS485_DATA   0x33
#define V6_EXT_RTADDR   0x34
#define V6_DAC_CTRL     0x36
#define V6_TRIGGER_CTRL 0x0

/* BC message buffer offsets              */
#define BC_MESSNO_OFFSET             0    //0
#define BC_NUM_BUFFERS_OFFSET        4    //4
#define BC_CTLWD_OFFSET              8    //8
#define BC_MSG_CMD1_OFFSET          12    //c 
#define BC_MSG_CMD2_OFFSET          14    //e   
#define BC_MSG_EI_OFFSET            16    //10  
#define BC_MSG_DATA_OFFSET          20    //14
#define BC_MSG_GAP_TIME_OFFSET      24    //18
#define BC_MSG_REP_RATE_OFFSET      28    //1c
#define BC_MSG_STR_FRAME_OFFSET     30    //1e
#define BC_BRNCH_MSG_OFFSET         24    //18
#define BC_MSG_NEXT_OFFSET          32    //20 

/* BC Data byte offsets                    */
#define BC_DATA_MSG_OFFSET           0
#define BC_DATA_NEXT_OFFSET          4
#define BC_DATA_TTIME_OFFSET         8
#define BC_DATA_STATUS_OFFSET       16
#define BC_DATA_MSTAT1_OFFSET       20
#define BC_DATA_MSTAT2_OFFSET       22
#define BC_DATA_DATA_OFFSET         24

/* BC Control data offset                 */
#define BC_CTRLDATA_MSG_OFFSET       0
#define BC_CTRLDATA_NXT_DATA_OFFSET  4
#define BC_CTRLDATA_TTIME_OFFSET     8
#define BC_CTRLDATA_TST_WRD_OFFSET  16
#define BC_CTRLDATA_DATA_OFFSET     20
#define BC_CTRLDATA_BIT_MSK_OFFSET  24
#define BC_CTRLDATA_COND_CTR_OFFSET 28
#define BC_CTRLDATA_CTR_VAL_OFFSET  32
#define BC_CTRLDATA_NXT_MSG_ADDR    36
#define BC_CTRLDATA_RESERVED        40

/* BM Buffer word offset defines */
#define BM_HEADER_OFFSET             0
#define BM_INT_STATUS_OFFSET         4
#define BM_TIME_TAG_OFFSET           8
#define BM_CMD1_OFFSET              16
#define BM_CMD2_OFFSET              20
#define BM_RESP1_OFFSET             24
#define BM_STAT1_OFFSET             28
#define BM_RESP2_OFFSET             32
#define BM_STAT2_OFFSET             36
#define BM_DATA_START               40

/* RT Message buffer word offset.  */
#define RT_NXT_MSG_OFFSET            0
#define RT_EI_PTR_OFFSET             4
#define RT_ENABLE_OFFSET             8
#define RT_INT_STATUS_OFFSET        12
#define RT_TIME_TAG_OFFSET          16
#define RT_CMD_OFFSET               24
#define RT_STATUS_OFFSET            26
#define RT_DATA_OFFSET              28
#define RT_EXT_STAT_OFFSET          92 
 
/* Parameters for BusTools_TimeTagMode() function:                                       */
#define API_TT_MICRO    0x0 /* Microsecond timetag default                               */
#define API_TT_NANO     0x10/* Nanosecond resolution                                     */
#define API_TT_DEFAULT  -1  /* TTDisplay format unchanged                                */
#define API_TTD_RELM     0  /* TTDisplay format relative to midnight time                */
#define API_TTD_IRIG     1  /* TTDisplay format IRIG time                                */
#define API_TTD_DATE     2  /* TTDisplay format DATE time                                */
#define API_TTD_RELM_NS  API_TTD_RELM | API_TT_NANO /* Relative to midnight time         */ 
#define API_TTD_IRIG_NS  API_TTD_IRIG | API_TT_NANO /* TTDisplay format IRIG time        */     
#define API_TTD_DATE_NS  API_TTD_DATE | API_TT_NANO /* TTDisplay format DATE time        */

#define API_TTI_ZERO     0  /* TTInit TT = 0 at start of recording                       */
#define API_TTI_DAY      1  /* TTInit TT = time since midnight (Host clock)              */
#define API_TTI_IRIG     2  /* TTInit TT = day of year (IRIG) (Host clock)               */
#define API_TTI_EXT      3  /* TTInit TT = externally-supplied time (DLL_NAME)           */
#define API_TTI_IRIG64   4  /* TTInit TT = time since midnight (Host clock) 64BIT TT     */
#define API_TTI_DAY64    5  /* TTInit TT = time since midnight (Host clock)              */

#define API_TTM_FREE     0  /* TTMode TT is free running                                 */
#define API_TTM_RESET    1  /* TTMode TT is reset to zero by external input              */
#define API_TTM_SYNC     2  /* TTMode TT is sync'ed to external input pulse              */
#define API_TTM_RELOD    3  /* TTMode TT is reset to previous value by pulse             */
#define API_TTM_IRIG     4  /* TTMode TT is reset to the external or internal IRIG value */
#define API_TTM_AUTO     5  /* TTMode TT is automatically incremented on external pulse  */
#define API_TTM_XCLK     6  /* TTMode TT is generated from external 1 Mhz clock          */

#define GMTIME           0
#define LOCALTIME        1
#define USER_TIME        0x10
#define SYSTEM_TIME      -1

/**********************************************************************
*  Function Prototypes
**********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

NOMANGLE BT_INT CCONV BusTools_API_Close(
   BT_UINT cardnum);

NOMANGLE BT_INT CCONV BusTools_API_Reset(
   BT_UINT, BT_UINT);

NOMANGLE BT_INT CCONV API_Init(
   BT_UINT,       // (i) card number (0 - 3)
   BT_U32BIT,     // (i) base address of board memory area
   BT_UINT,       // (i) io address of board
   BT_UINT * );   // (io) 0 -> software driver, 1 -> hardware driver,
                  //      2 -> HW interrupt enable

NOMANGLE BT_INT CCONV BusTools_API_InitExtended(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_U32BIT base_address,  // (i) base address of board/carrier memory area
   BT_UINT   ioaddr,        // (i) board/carrier io address
   BT_UINT   * flag,        // (io) 0->software driver, 1->hardware driver, 2->Enable HW interrupts
   BT_UINT   platform,      // (i) execution platform: PLATFORM_PC, PLATFORM_VMIC
   BT_UINT   boardType,     // (i) IP1553, IP1553SF, IP1553MF, PCI1553 or ISA1553
   BT_UINT   carrier,       // (i) NATIVE, IP1553_ISA, IP1553_PCI, IP1553_VME,
                            //     IP1553_CP, IP1553_VXI.
   BT_UINT   slot,          // (i) SLOT_A, SLOT_B, SLOT_C, SLOT_D, etc.
   BT_UINT   mapping);      // (i) carrier memory map: CARRIER_MAP_DEFAULT

NOMANGLE BT_INT CCONV BusTools_API_OpenChannel(
  BT_UINT   *chnd,          // (o) card number (0 - 16)
  BT_UINT   mode,           // (i) operational mode
  BT_INT    devid,          // (i) Installation device number
  BT_UINT   channel);       // (i) channel 1 - 4.

#ifdef SHARE_CHANNEL
NOMANGLE BT_INT CCONV BusTools_API_ShareChannel(
  BT_UINT   cardnum);         // (o) channel 0 - 15.

NOMANGLE BT_INT CCONV BusTools_API_JoinChannel(
  BT_UINT *chnd, 
  BT_UINT device,
  BT_UINT channel);

NOMANGLE BT_INT CCONV BusTools_API_QuitChannel(BT_UINT cardnum, BT_UINT qFlag);
#endif //#ifdef SHARE_CHANNEL 

NOMANGLE BT_INT CCONV BusTools_API_OpenDeviceChannel(
  BT_UINT   *chnd,          // (o) card number (0 - 16)
  BT_UINT   mode,           // (i) operational mode
  BT_INT    devid,          // (i) Installation device number
  BT_UINT   channel);       // (i) channel 1 - 4.

NOMANGLE BT_INT CCONV BusTools_FindDevice(
  BT_INT   device_type,     // (i) 
  BT_INT   instance);       // (i) device instance (1 - 16)

NOMANGLE BT_INT CCONV BusTools_ListDevices(
  DeviceList  *list);       // (i) pointer to DeviceList Structure

NOMANGLE BT_INT CCONV BusTools_GetCardnum(void);

#ifdef _USER_INIT_
// This routine is not a part of the API, rather it is a function supplied by
//  the user, which is called by BusTools_API_Initxxx() to get the board addresses.
NOMANGLE BT_INT CCONV BusTools_API_InitExternal(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_UINT   wOpen,         // (i) 1->open device, 0->close device
   BT_U32BIT phys_addr,     // (i) base address of board/carrier memory area
   BT_U32BIT wIOaddress,    // (i) board/carrier io address
   BT_UINT   cardType,      // (i) IP1553, IP1553SF, IP1553MF, PCI1553 or ISA1553
   BT_UINT   carrier,       // (i) NATIVE, IP1553_ISA, IP1553_PCI, IP1553_VME,
                            //     IP1553_CP, IP1553_VXI.
   BT_UINT   slot,          // (i) SLOT_A, SLOT_B, SLOT_C, SLOT_D, etc.
   void * PageAddr[4]);     // (o) array of board addresses.
#endif //_USER_INIT_

NOMANGLE BT_INT CCONV BusTools_SetDumpPath(
   char * dpath);

NOMANGLE BT_INT CCONV BusTools_DumpMemory(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U32BIT region_mask,   // (i) mask of memory regions to dump (bit 1 = region 1)
   char * file_name,        // (i) pointer to name of output file to create
   char * heading);         // (i) pointer to message to display in file

NOMANGLE BT_INT CCONV BusTools_GetAddr(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_UINT   memtype,       // (i) memory region
   BT_U32BIT * start,       // (o) starting address of requested memory
   BT_U32BIT * end);        // (o) ending address of requested memory

NOMANGLE BT_INT CCONV BusTools_Checksum1760(
   BT_U16BIT *mbuf, 
   BT_INT wdcnt);

NOMANGLE BT_INT CCONV BusTools_ExtTrigIntEnable(
   BT_UINT cardnum,         // (i) card number
   BT_UINT flag);           // (i) EXT_TRIG_ENABLE
                            //     EXT_TRIG_DISABLE

NOMANGLE BT_INT CCONV BusTools_ExtTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT pwidth);       // (i) Trigger pulse width  

NOMANGLE char * CCONV BusTools_GetAddrName(
   BT_UINT memtype);        // (i) memory block type (e.g. API_MEM_REGISTERS)


NOMANGLE BT_INT CCONV BusTools_GetChannelStatus(
   BT_UINT cardnum,         // (i) card number
   API_CHANNEL_STATUS * cstat);  // (o) pointer to heart beat count

NOMANGLE BT_INT CCONV BusTools_GetCSCRegs(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U16BIT *csc,
   BT_U16BIT *acr);

NOMANGLE BT_INT CCONV BusTools_BoardIsUSBMon(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_BoardHasIRIG(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_BoardIsV6(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_BoardIsMultiFunction(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_GetBoardType(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_GetChannelCount(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_DumpHWRegisters(
   BT_UINT cardnum,         // (i) card number
   void * buff);

NOMANGLE BT_INT CCONV BusTools_DumpRAMRegisters(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT * buff);

NOMANGLE BT_INT CCONV BusTools_GetRevision(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT * ucode_rev,     // (o) microcode revision of card
   BT_UINT * api_rev);      // (o) API revision number

NOMANGLE BT_INT CCONV BusTools_GetFWRevision(
   BT_UINT cardnum,         // (i) card number
   float * wrev,            // (o) pointer to microcode revision number
   float * lrev,
   BT_INT   * build);       // (o) pointer to lpu revision number

NOMANGLE BT_INT CCONV BusTools_GetPulse(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT * beat);       // (o) pointer to heart beat count

NOMANGLE BT_INT CCONV BusTools_GetSerialNumber(
   BT_UINT cardnum,                     // (i) card number
   BT_U32BIT * serial_number);           // (o) pointer to heart beat count

NOMANGLE BT_INT CCONV BusTools_ReadFlashConfigData(BT_UINT cardnum, BT_U16BIT * fdata);

NOMANGLE BT_INT CCONV BusTools_MemoryAlloc(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_UINT   segnum,        // (i) segment from which to allocate space
   BT_U32BIT bcount,        // (i) number of bytes to write (must be even)
   BT_U32BIT * addr);       // (o) pointer to allocated region on board

NOMANGLE BT_INT CCONV BusTools_MemoryAlloc32(
   BT_UINT cardnum,         // (i) card number
   BT_UINT segnum,          // (i) Segment to allocate memory in (not used)
   BT_U32BIT bcount,        // (i) number of bytes to be allocated (even)
   BT_U32BIT * addr);       // (o) Allocated buffer address in BT hardware

NOMANGLE BT_INT CCONV BusTools_MemoryAvailable(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT *bytes);       // (o) available Memory

NOMANGLE BT_INT CCONV BusTools_MemoryRead(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) card address of data to read (must be even)
   BT_U32BIT bcount,        // (i) number of bytes to read (must be even)
   VOID * buff);            // (o) pointer to user's buffer

NOMANGLE BT_INT CCONV BusTools_MemoryRead2(
   BT_UINT cardnum,          // (i) card number
   BT_INT  region,           // (i) Region to read HIF, HWREG, RAMREG, RAM
   BT_U32BIT start,          // (i) start address or register
   BT_U32BIT count,          // (i) bytes/registers to read
   void * buff);             // (o) Pointer to user's buffer

NOMANGLE BT_INT CCONV BusTools_SharedMemoryRead(
   BT_UINT cardnum,          // (i) card number
   BT_U32BIT addr,           // (i) byte address
   BT_U32BIT count,          // (i) word count
   BT_U32BIT *buff);         // (o) Pointer to user's buffer

NOMANGLE BT_INT CCONV BusTools_MemoryWrite(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) card address of data to write (must be even)
   BT_U32BIT bcount,        // (i) number of bytes to write (must be even)
   VOID * buff);            // (o) pointer to user's buffer

NOMANGLE BT_INT CCONV BusTools_MemoryWrite2(
   BT_UINT cardnum,         // (i) card number
   BT_INT  region,          // (i) Regions to read HIF, HWREG, RAMREG, RAM
   BT_U32BIT addr,          // (i) BYTE address in BT hardware to begin writing
   BT_U32BIT bcount,        // (i) EVEN number of bytes to write
   void * buff);            // (i) Data Buffer

NOMANGLE BT_INT CCONV BusTools_SharedMemoryWrite(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr,          // (i) address in BT hardware to begin writing
   BT_U32BIT wcount,        // (i) number of word to write
   BT_U32BIT *buff);        // (i) Data Buffer

NOMANGLE BT_INT CCONV BusTools_RegisterFunction(
   BT_UINT cardnum,         // (i) card number (0 - based)
   API_INT_FIFO *sIntFIFO,  // (i) pointer to user-supplied FIFO structure
   BT_UINT wFlag);          // (i) REGISTER_FUNCTION -> 1 -> create thread,
                            //     UNREGISTER_FUNCTION-> 0 -> distroy thread
NOMANGLE BT_INT CCONV BusTools_Set1553Mode(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT rtmode);       // (i) Mode flag bit map by RT address
                            //     bit 0=RT0, bit1=RT1 ... bit 31=RT31
                            //     0=1553B
                            //     1=1553A

NOMANGLE BT_INT CCONV BusTools_SetBroadcast(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT bcast);          // (i) flag, non-zero to enable broadcast

NOMANGLE BT_INT CCONV BusTools_SetExternalSync(BT_UINT cardnum,
   BT_UINT flag);           // (i) 0 -> disable external sync, 1-> enable sync

NOMANGLE BT_INT CCONV BusTools_SetInternalBus(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT busflag);        // (i) 0 -> external bus, 1 -> internal bus

NOMANGLE BT_INT CCONV BusTools_SetNRLRTimeout(
   BT_UINT   cardnum,        // (i) card number
   BT_UINT   wTimeout1,      // (i) no response time out in microseconds
   BT_UINT   wTimeout2);     // (i) late response time out in microseconds
   

NOMANGLE BT_INT CCONV BusTools_SetTestBus(
   BT_UINT cardnum,         // (i) card number
   BT_UINT busflag);        // (i) TEST_BUS_ENABLE enables the test bus
                            //     TEST_BUS_DISABLE disalbe the test bus

NOMANGLE BT_INT CCONV BusTools_V6_SetDiscreteBlock(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disValue);     // (i) discrete values 0-ground 1-pull-up to 3.3v or for input
                            //     bit encoded with bit 1 = discrete 1; bit 2 = discrete 2....
                            //     bit 0 is not used.

NOMANGLE BT_INT CCONV BusTools_RS485_V6Ctrl(
   BT_UINT cardnum,         // (i) card number
   BT_UINT dchan,           // (i) diff channel (1 - 8)
   BT_UINT ddata,           // (i) Data. Set this bit for 485+ pin = 1 and 485- pin = 0. 
                            //     Clear the bit for 485+ pin = 0 and 485- pin = 1.
   BT_UINT dtxen,           // (i) Transmit Enable. Set the bit for the 485 device to transmit
                            //     Clear the bit for the device to receive.
   BT_UINT dterm);          // (i) Termination. Setting this bit enables a 120 ohm termination within the 485 transceiver. 
                            // (i) Clear this bit for no termination. This feature is board specific.

NOMANGLE BT_INT CCONV BusTools_RS485_V6Data(
   BT_UINT cardnum,         // (i) card number
   BT_INT  difFlag,         // (i) Set for RS485_TX_DATA for transmit data
                            //     Set for RS485_RX_DATA for receive data
   BT_U32BIT *ddata);       // (i) RS485 Data (BIT 0 not used)

NOMANGLE BT_INT CCONV BusTools_DiscreteSetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disFlag,       // (i) discrete flags
   BT_U32BIT mask);         // (i) mask

NOMANGLE BT_INT CCONV BusTools_DiscreteGetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT *disDirValue); // (o) discrete values

NOMANGLE BT_INT CCONV BusTools_DiscreteRead(
   BT_UINT cardnum,         // (i) card number
   BT_INT disSel,           // Discrete select = 0 all
   BT_U32BIT *disValue);

NOMANGLE BT_INT CCONV BusTools_DiscreteReadRegister(
   BT_UINT cardnum,         // (i) card number
   BT_INT regnum,           // (i) register number
   BT_U32BIT *disValue);    // (o) discrete values

NOMANGLE BT_INT CCONV BusTools_DiscreteWrite(
   BT_UINT cardnum,         // (i) card number
   BT_INT  disSel,          // Discrete select = 0 all
   BT_UINT disFlag);        // (I) 0 reset the discrete 1 set the discrete

NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag);         // (i) TRIGGER_OUT_DIS_7 use Discrete 7 as trigger out
                            //     TRIGGER_OUT_DIS_8 use Discrete 8 as trigger out
                            //     TRIGGER_OUT_DIS_NONE Use neither

NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag);        // (i) 0=none 1-31 discrete -1 to -31 differential

NOMANGLE BT_INT CCONV BusTools_DiffTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT  chflag,          // (i) 0 = disconnect 1 = connect channel
   BT_INT  diffen);         // (i) 0 = disable 1 = enable differential output

NOMANGLE BT_INT CCONV BusTools_RS485_TX_Enable(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT enable,        // (i) discrete flags
   BT_U16BIT mask);         // (i) mask

NOMANGLE BT_INT CCONV BusTools_RS485_Set_TX_Data(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT rsdata,        // (i) transmit data pattern
   BT_U16BIT mask);         // (i) mask

NOMANGLE BT_INT CCONV BusTools_RS485_ReadRegs(
   BT_UINT cardnum,         // (i) card number
   BT_INT  regval,          // (i) RS 485 register to read
                            //     RS485_TXEN_REG
                            //     RS485_TXDA_REG
                            //     RS485_RXDA_REG
   BT_U32BIT *rsdata);      // (o) RS 485 register data

NOMANGLE BT_INT CCONV BusTools_PIO_GetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT *pioDirValue); // (o) discrete values 

NOMANGLE BT_INT CCONV BusTools_PIO_Read(
   BT_UINT cardnum,         // (i) card number
   BT_INT pioSel,           // (i) Discrete select = 0 all
   BT_U32BIT *pioValue);    // (o) discrete values

NOMANGLE BT_INT CCONV BusTools_PIO_SetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disSet,        // (i) discrete flags
   BT_U32BIT mask);         // (i) mask

NOMANGLE BT_INT CCONV BusTools_PIO_Write(
   BT_UINT cardnum,         // (i) card number
   BT_INT  pioSel,          // (i) Discrete select 0 = all
   BT_UINT pioFlag);        // (i) 0 reset the discrete 1 set the discrete

NOMANGLE BT_INT CCONV BusTools_SetMultipleExtTrig(
   BT_UINT cardnum,         // (i) card number
   BT_INT  trigOpt,			// (i) PIO,DISCRETE,EIA485
   BT_UINT tvalue,          // (i) trigger channel (1 - 12)
   BT_INT  enableFlag);     // (i) ENABLE_TRIG or DISABLE_TRIG

NOMANGLE BT_INT CCONV BusTools_GetValidDiscrete(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT * disSetting); // (i) returns valid discrete

NOMANGLE BT_INT CCONV BusTools_GetValidPio(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT * pioSetting); // (i) returns valid PIO

NOMANGLE BT_INT CCONV BusTools_GetValidDiff(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT * diffSetting);// (i) returns valid Differential

NOMANGLE BT_INT CCONV BusTools_GetTermEnable(
   BT_UINT cardnum,          // (i) card number
   BT_U32BIT *tEnable);      // (o) Enable setting

NOMANGLE BT_INT CCONV BusTools_SetTermEnable(
   BT_UINT cardnum,          // (i) card number
   BT_U16BIT  tEnable);      // (i) enable/disable setting 

NOMANGLE BT_INT CCONV BusTools_SetV6TrigIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT  trigOpt,			// (i) PIO,DISCRETE,EIA485
   BT_UINT tvalue);         // (i) trigger channel (1 - 18) 

NOMANGLE BT_INT CCONV BusTools_SetV6TrigOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT  trigOpt,			// (i) PIO,DISCRETE,EIA485   
   BT_UINT tvalue);         // (i) trigger channel (1 - 18)

NOMANGLE BT_INT CCONV BusTools_GetDevInfo(
   BT_UINT device,          // (i) card number
   DEVICE_INFO *pInfo);

NOMANGLE BT_INT CCONV BusTools_SetOptions(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT intflag,         // (i)
   BT_UINT resettimer,      // (i)
   BT_UINT trig_on_sync,    // (i)
   BT_UINT enable_rt);      // (i)

NOMANGLE BT_INT CCONV BusTools_PCI_Reset(
   BT_INT cardnum, 
   BT_UINT reset_flag);

NOMANGLE BT_INT CCONV BusTools_VME_Reset(
   BT_INT cardnum, 
   BT_UINT reset_flag);

NOMANGLE BT_INT CCONV BusTools_SetPolling(
   BT_UINT polling);

NOMANGLE BT_INT CCONV BusTools_SetSa31(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT sa31);           // (i) flag, non-zero to enable SA31 as a mode code

NOMANGLE BT_INT CCONV BusTools_SetVoltage(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT voltage,         // (i) integer value of volts*100
   BT_UINT voltflag);       // (i) 0 -> direct coupling, 1 -> transformer coupling

NOMANGLE char * CCONV BusTools_StatusGetString(
   BT_INT status);          // (i) BusTools error status

/**********************************************************************
*  BC Function Prototypes
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicRun(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_UINT   messageid,     // Number of BC message which begins list
   BT_UINT   Hipriority,    // 1 -> Hi Priority msgs, 0 -> Low Priority msgs
   BT_UINT   WaitFlag,      // 1 -> Wait for BC to complete executing msgs
   BT_UINT   WaitTime);     // Timeout in seconds(16-bit) or milliseconds(32-bit)

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicTest(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_UINT   Hipriority);   // 1 -> Hi Priority msgs, 0 -> Low Priority msgs

NOMANGLE BT_INT CCONV BusTools_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf Msg Schd = 0x10
                            //  Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers);  // (i) number of BC message buffers ( 1 or 2  for legacy )

NOMANGLE BT_INT CCONV BusTools_BC_RetryInit(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_U16BIT *bc_retry);    // (i) retry option buffer

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT * flag);         // (o) non-zero if BC is running

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning2(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_BC_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT count);          // (i) number of BC messages to allocate

NOMANGLE BT_INT CCONV BusTools_BC_MessageBlockAlloc(
   BT_UINT cardnum,         // (i) card number (0 based)
   BT_UINT bufID,           // (i) BC_BUFFER_NEXT or BC_BUFFER_LAST
   BT_UINT count);          // (i) Number of BC data buffer linked to the
                            //     message buffer

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetaddr(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT messageid,       // (i) index of BC message to convert to an address
   BT_U32BIT * addr);       // (o) board address of specified message

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetid(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) board address of BC message
   BT_UINT * messageid);    // (o) index ob BC message

NOMANGLE BT_INT CCONV BusTools_BC_MessageNoop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT messageid,       // (i) index of BC message to convert to/from NOOP
   BT_UINT NoopFlag,        // (i) non-zero to convert message to NOOP
   BT_UINT WaitFlag);       // (i) non-zero to wait for BC to finish BC message

NOMANGLE BT_INT CCONV BusTools_BC_ControlWordUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_U16BIT wControlWord,  // (i) New Control Word settings
   BT_UINT WaitFlag);       // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
NOMANGLE BT_INT CCONV BusTools_BC_ControlWordRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message
   BT_U16BIT * api_control_word); // (i) Pointer variable to hold control word

NOMANGLE BT_INT CCONV BusTools_BC_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT messno,          // (i) index of BC message to read
   API_BC_MBUF * api_message); // (o) user's buffer to write message into

NOMANGLE BT_INT CCONV BusTools_BC_MessageBufferRead(
   BT_UINT cardnum,          // (i) card number (0 based)
   BT_U32BIT addr,           // (i) address of BC Data buffer
   API_BC_MBUF * api_message);  // (i) Pointer to buffer to receive msg

NOMANGLE BT_INT CCONV BusTools_BC_MessageReadData(   
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT messno,          // (i) index of BC message to read
   BT_U16BIT * buffer);     // (o) user's buffer to write message data into

NOMANGLE BT_INT CCONV BusTools_BC_MessageReadDataBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) number of the BC message
   BT_UINT   buffer_id,     // (i) Buffer ID 0=A 1=B
   BT_U16BIT * buffer);     // (o) pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdate(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT mblock_id,       // (i) index of BC message to update
   BT_U16BIT * buffer);     // (i) pointer to data to copy to BC message

NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_ReadDataBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bufaddr,       // (i) Buffer address 
   BT_U16BIT * buffer);     // (o) pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_DataBufferWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_DataBufferUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT buf_addr,      // (i) Address of the data buffer
   BT_UINT   dcount,        // (i) data count
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_SelectBufferRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_UINT   buf_num,       // (i) Data buffer number
   BT_U16BIT * buffer);     // (i) Pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_SelectBufferUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_UINT   buf_num,       // (i) Data buffer number
   BT_UINT   dcount,        // (i) Data count
   BT_U16BIT * buffer);     // (i) Pointer to user's data buffer

NOMANGLE BT_INT CCONV BusTools_BC_GetBufferCount(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_U32BIT * count);      // (i) Pointer to count

NOMANGLE BT_INT CCONV BusTools_BC_MessageWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT messno,          // (i) index of BC message to write to
   API_BC_MBUF * api_message); // (i) pointer to user-specified BC message

NOMANGLE BT_INT CCONV BusTools_BC_AutoIncrMessageData(
   BT_INT cardnum,          // (i) card number (0 - based)
   BT_INT messno,           // (i) Message buffer number
   BT_INT data_wrd,         // (i) Data word to increment (0 - 31)
   BT_U16BIT start,         // (i) Start value (0 - 65535)
   BT_U16BIT incr,          // (i) Increment value (0 - 512)
   BT_INT rate,             // (i) Increment rate  (0 - 512)   
   BT_U16BIT max,           // (i) Maximum value   (0 - 65535)
   BT_INT sflag);           // (i) Start/Stop flag 0 = stop; 1 = start

NOMANGLE BT_INT CCONV BusTools_BC_SetFrameRate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT frame);         // (i) New Frame Time in uSecs

NOMANGLE BT_INT CCONV BusTools_BC_Start(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT message_num);    // (i) message number to start BC at

NOMANGLE BT_INT CCONV BusTools_BC_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT flag);           // (i) 1 -> start BC (at message 0), 0 -> stop BC

NOMANGLE BT_INT CCONV BusTools_BC_Trigger(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_INT    trigger_mode);

NOMANGLE BT_INT CCONV BusTools_BC_Checksum1760(API_BC_MBUF *mbuf, 
                                               BT_U16BIT *cksum);

/**********************************************************************
*  BIT Function Prototypes
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_InternalBit(
   BT_UINT cardnum,        // (i) Card number of open board to test
   BT_INT  NumMessages);   // (i) Number of messages to test

NOMANGLE BT_INT CCONV BusTools_BIT_TwoBoardWrap(
   BT_UINT FirstCard,      // (i) Card number of first open board in wrap test
   BT_UINT SecondCard,     // (i) Card number of second open board in wrap test
   BT_INT  TestPrimary,    // (i) Set TRUE if we should test the primary bus
   BT_INT  TestSecondary,  // (i) Set TRUE if we should test the secondary bus
   BT_INT  NumMessages,    // (i) Number of messages to test
   BT_INT  RT_addr,        // (i) RT address to test
   BT_INT  RT_subaddr);    // (i) RT subaddress to test

NOMANGLE BT_INT CCONV BusTools_BIT_CableWrap(
   BT_UINT cardnum,        // (i) Card number of open board to test
   BT_INT  NumMessages);   // (i) Number of messages to test

NOMANGLE BT_INT CCONV BusTools_BIT_StructureAlignmentCheck(
   BT_INT flag);

NOMANGLE BT_INT CCONV BusTools_BIT_StructureAlignmentCheckV6(
   BT_INT flag);

/**********************************************************************
*  BM Function Prototypes
**********************************************************************/
NOMANGLE BT_INT CCONV BusTools_BM_GetUserData(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT   *data1,
   BT_U16BIT   *data2);

NOMANGLE BT_INT CCONV BusTools_BM_SetUserData(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT   data1,
   BT_U16BIT   data2);

NOMANGLE BT_INT CCONV BusTools_BM_FilterRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   API_BM_CBUF * cbuf);

NOMANGLE BT_INT CCONV BusTools_BM_FilterWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   API_BM_CBUF * cbuf);

NOMANGLE BT_INT CCONV BusTools_BM_Init(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT bm_ctrl1,
   BT_UINT bm_ctrl2);

NOMANGLE BT_INT CCONV BusTools_BM_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT mbuf_count,
   BT_UINT * mbuf_actual,
   BT_U32BIT enable);

NOMANGLE BT_INT CCONV BusTools_BM_MessageGetaddr(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT mbuf_id,         // (i) BM buffer id (0 - based)
   BT_U32BIT * addr);       // (o) BT address of buffer

NOMANGLE BT_INT CCONV BusTools_BM_MessageGetid(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) BT address of buffer
   BT_UINT * messageid);    // (o) BM buffer id (0 - based)

NOMANGLE BT_INT CCONV BusTools_BM_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT mbuf_id,
   API_BM_MBUF * mbuf);

NOMANGLE BT_INT CCONV BusTools_BM_MessageReadBlock(
   BT_UINT cardnum,         // (i) card number (0 - based)
   API_BM_MBUF * api_mbuf,  // (i) address of caller's array of BM_MBUF's
   BT_UINT size,            // (i) number of BM_MBUF's in array
   BT_UINT curpos,          // (i) next avail location in array
   BT_UINT * ret_count);    // (o) number of BM_MBUF's transferred

NOMANGLE BT_INT CCONV BusTools_BM_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT flag);           // (i) 1 -> Start BM, 0 -> Stop BM

NOMANGLE BT_INT CCONV BusTools_BM_TriggerWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   API_BM_TBUF * tbuf);     // (i) pointer to BM trigger buffer data

NOMANGLE BT_U32BIT CCONV BusTools_InterMessageGap(
   API_BM_MBUF * first,     // (i) address of first message buffer
   API_BM_MBUF * secnd);    // (i) address of second message buffer

NOMANGLE BT_U32BIT CCONV BusTools_InterMessageGap2(
   BT_UINT       flag,
   API_BM_MBUF * first,
   API_BM_MBUF * second);

NOMANGLE BT_INT CCONV BusTools_BM_SetRT_RT_INT(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT iflag);          // 0 = int on command1 1 = int on command2

/**********************************************************************
*  EI Function Prototypes
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_EbufRead(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,        // (i) number of error injection buffer to read
   API_EIBUF * ebuf);       // (o) pointer to resulting buffer values

NOMANGLE BT_INT CCONV BusTools_EI_EbufWrite(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,        // (i) number of error injection buffer to write
   API_EIBUF * ebuf);       // (i) pointer to buffer values to be written to HW

NOMANGLE BT_INT CCONV BusTools_EI_EbufWriteENH(
   BT_UINT cardnum,         // (i) card number (0 - based) 
   BT_UINT errorid,         // (i) number of error injection buffer to write 
   API_ENH_EIBUF *ebuf);    // (i) pointer to buffer values to be written to HW

NOMANGLE BT_INT CCONV BusTools_EI_EnhEbufWrite( //DEPRECATED
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT errorid,         // (i) number of error injection buffer to write
   BT_UINT enhData,         // (i) enhanced data pointer to EI_COUNT (33) size array
   API_EIBUF * ebuf);       // (i) pointer to buffer values to be written to HW

NOMANGLE BT_INT CCONV BusTools_EI_Getaddr(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,        // (i) number of error injection buffer
   BT_U32BIT * addr);       // (o) address of specified error injection buffer

NOMANGLE BT_INT CCONV BusTools_EI_Getid(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) address of error injection buffer
   BT_UINT * errorid);      // (o) number of specified error injection buffer

/**********************************************************************
*  Error count routines
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_ErrorCountClear(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_ErrorCountGet(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U32BIT * count,       // (o) number of Interrupt on End Of Messages
   BT_U32BIT * buf);        // (o) pointer to output buffer of 32 elements


/**********************************************************************
*  Flash Function Prototypes
**********************************************************************/
NOMANGLE BT_INT CCONV BusTools_FirmwareReload(
   BT_UINT cardnum);       // (i) card number (0 - based)
NOMANGLE BT_INT CCONV BusTools_FlashLogErase(
   BT_UINT cardnum,        // (i) card number (0 - based)
   BT_INT sector);         // flash block (1-4)
NOMANGLE BT_INT CCONV BusTools_FlashLogWrite(
   BT_UINT cardnum,        // (i) card number (0 - based)
   BT_INT sector,          // flash block (1-4)
   BT_INT pagenum,         // Page number within a flash block (0-255)
   BT_U8BIT *pageData);    // Pointer to the input data of 256 bytes
NOMANGLE BT_INT CCONV BusTools_FlashLogRead(
   BT_UINT cardnum,        // (i) card number (0 - based)
   BT_INT sector,          // flash block (1-4)
   BT_UINT paddr,          // offset to each flash block (0 to < 64K)
   BT_UINT bcount,         // number of bytes to read (1-64K)
   BT_U8BIT *rData);       // Pointer to the output data of bcount bytes
/**********************************************************************
*  RT Function Prototypes
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_RT_AbufRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddress,       // (i) RT address (0 - based)
   API_RT_ABUF * abuf);     // (o) pointer to resulting RT_ABUF

NOMANGLE BT_INT CCONV BusTools_RT_AbufWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddress,       // (i) RT address (0 - based)
   API_RT_ABUF * abuf);     // (i) pointer to RT_ABUF to be written to HW

NOMANGLE BT_INT CCONV BusTools_RT_MonitorEnable(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddress,       // (i) RT address (0 - based)
   BT_UINT mode);           // (i) RT Monitor Enble = 1 disable =0;

NOMANGLE BT_INT CCONV BusTools_RT_CbufbroadRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   API_RT_CBUFBROAD * apicbuf); // (i) pointer to RT CBUF (Broadcast)

NOMANGLE BT_INT CCONV BusTools_RT_CbufbroadWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   API_RT_CBUFBROAD * apicbuf); // (i) pointer to RT CBUF

NOMANGLE BT_INT CCONV BusTools_RT_CbufRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT * mbuf_count,    // (o) number of RT MBUF's allocated
   API_RT_CBUF * apicbuf);  // (o) pointer to resulting RT CBUF

NOMANGLE BT_INT CCONV BusTools_RT_CbufWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_INT  mbuf_count,      // if negative, one pass through buffers only
   API_RT_CBUF * apicbuf);  // (i) pointer to API RT control buf

NOMANGLE BT_INT CCONV BusTools_RT_Init(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT testflag);       // (i) flag; must be zero for normal operation!

NOMANGLE BT_INT CCONV BusTools_RT_MessageGetaddr(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT mbuf_id,         // (i) RT MBUF number
   BT_U32BIT * mbuf_offset);// (o) address of specified RT MBUF

NOMANGLE BT_INT CCONV BusTools_RT_MessageGetid(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_U32BIT addr,          // (i) Byte address of RT MBUF to locate
   BT_UINT * rtaddr,        // (o) RT address (0 - based)
   BT_UINT * subaddr,       // (o) RT subaddress (0 - based)
   BT_UINT * tr,            // (o) Transmit/Receive flag (1->rt transmit)
   BT_UINT * mbuf_id);      // (o) RT MBUF number

NOMANGLE BT_INT CCONV BusTools_RT_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT mbuf_id,         // (i) RT MBUF number
   API_RT_MBUF_READ * mbuf);

NOMANGLE BT_INT CCONV BusTools_RT_MessageBufferNext(
   BT_UINT cardnum,         // (i) card number
   BT_UINT rtaddr,          // (i) RT Terminal Address of message to read
   BT_UINT subaddr,         // (i) RT Subaddress of message to read
   BT_UINT tr,              // (i) Receive or Transmit buffer is to be read
   BT_UINT *mbuf_id         // (i) ID number of message buffer (0-based)
   );

NOMANGLE BT_INT CCONV BusTools_RT_MessageWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT mbuf_id,         // (i) RT MBUF number
   API_RT_MBUF_WRITE * mbuf);

NOMANGLE BT_INT CCONV BusTools_RT_AutoIncrMessageData(
   BT_INT cardnum,          // (i) card number (0 - based)
   BT_INT rtaddr,           // (i) RT address (0 - based)
   BT_INT subaddr,          // (i) RT subaddress (0 - based)
   BT_INT data_wrd,         // (i) data word to increment (0 - 31)
   BT_U16BIT start,         // (i) start value (0 - 65535)
   BT_U16BIT incr,          // (i) increment value (0 - 512)
   BT_INT rate,             // (i) increment rate (0 - 512)
   BT_U16BIT max,           // (i) maximum value (0 - 65535)
   BT_INT sflag);           // (i) start/stop flag 0 = stop; 1 = start

NOMANGLE BT_INT CCONV BusTools_RT_MessageWriteStatusWord(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   BT_UINT subaddr,         // (i) RT subaddress (0 - based)
   BT_UINT tr,              // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT mbuf_id,         // (i) RT MBUF number
   BT_U16BIT wStatusWord,   // (i) 1553 status word to be used by buffer
   BT_UINT wFlag);          // (i) 1 -> Enable status word processing, 0-> disable

NOMANGLE BT_INT CCONV BusTools_RT_MessageWriteDef(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT rtaddr,          // (i) RT address (0 - based)
   API_RT_MBUF_WRITE * mbuf);

NOMANGLE BT_INT CCONV BusTools_RT_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT flag);           // (i) 1 -> start RT, 0 -> stop

NOMANGLE BT_INT CCONV BusTools_RT_GetRTAddr(
    BT_UINT cardnum,        // (i) card number
    BT_INT *rtaddr);        // (o) hardwire RT address lines value

NOMANGLE BT_INT CCONV BusTools_RT_GetRTAddr1760(
    BT_UINT cardnum,        // (i) card number
    BT_UINT aflag,          // (i) 0   - return latched data
                            //     >=1 - read current data.
    BT_INT *rtaddr);        // (o) hardwire RT address lines value

#ifdef _USER_DLL_
/**********************************************************************
*  User DLL's are only supported under Windows.
**********************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_LoadUserDLL(
   BT_UINT cardnum,         // (i) - Card number to open/close
   const char * szDLLName); // (i) - Pointer to the comma-delimited list of DLL names
#endif
/**********************************************************************
*  PLAYBACK function prototypes are only defined for 32-bit Windows.
**********************************************************************/
#if defined (_PLAYBACK_)
NOMANGLE BT_INT CCONV BusTools_Playback(
   BT_UINT cardnum,         // (i) card number (0 - based)
   API_PLAYBACK);

NOMANGLE BT_INT CCONV BusTools_Playback_Stop(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_Playback_Check(
   char *,BT_UINT);
#endif

/**********************************************************************
*  TIME function prototypes
**********************************************************************/

NOMANGLE void  CCONV BusTools_TimeGetString(
   BT1553_TIME * curtime,   // (i) pointer to time structure to convert
   char * string);          // (o) pointer to resulting character string

NOMANGLE void CCONV BusTools_TimeGetFmtString(
   BT_INT        tFormat,   // (i) Format assignment (API_TT_DEFAULT, API_TTD_RELM, API_TTD_IRIG, API_TTD_DATE
                            //                        API_TTD_RELM_NS, API_TTD_IRIG_NS, API_TTD_DATE_NS)
   BT1553_TIME * curtime,   // (i) Time to be converted to string value.
   char        * string);   // (i/o) Pointer to store resulting time string.

NOMANGLE BT_INT CCONV BusTools_TimeTagInit(
   BT_UINT cardnum);        // (i) card number (0 - based)

NOMANGLE BT_INT CCONV BusTools_TimeTagMode(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_INT    TTDisplay,     // (i) time tag display format
   BT_INT    TTInit,        // (i) time tag counter initialization mode
   BT_INT    TTMode,        // (i) time tag timer operation mode
   char     *DLLname,       // (i) name of the DLL containing time function
   BT_U32BIT TTPeriod,      // (i) period of external TTL sync input
   BT_U32BIT lParm1,        // (i) spare parm1, set to zero.
   BT_U32BIT lParm2);       // (i) spare parm2, set to zero.

NOMANGLE BT_INT CCONV BusTools_GetTimeTagMode(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_INT    *TTDisplay,    // (o  Pointer to time tag display format
   BT_INT    *TTInit,       // (o) Pointer to time tag counter initialization mode
   BT_INT    *TTMode,       // (o) Pointer to time tag timer operation mode
   BT_U32BIT *TTPeriod);    // (o) Pointer to period of external TTL sync input

NOMANGLE BT_U32BIT CCONV BusTools_RelAddr(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr);

NOMANGLE BT_U32BIT CCONV BusTools_RamAddr(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT addr);

NOMANGLE BT_INT CCONV BusTools_TimeTagRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT1553_TIME * timetag);  // (o) pointer to resulting time tag structure

NOMANGLE BT_INT CCONV BusTools_TimeTagWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT1553_TIME  * timetag,  // (i) pointer to time tag structure
   BT_INT         flag);    // (i) flag->0 just load the TT register,
                            //     flag->1 load the TT register into the counter

NOMANGLE BT_INT CCONV BusTools_TimeTagReset(
   BT_UINT cardnum,         // (i) card number (0 - based)card number (0 - based)
   BT_UINT tflag);          // (i) EXT_RESET_ENABLE (1) EXT_RESET_DISABLE (0)

// This is the prototype of the user-supplied external time tag
//  initialization function.
#if !defined(_CVI_) && !defined(_LVRT_)
NOMANGLE BT_INT CCONV BusTools_TimeTagGet(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT1553_TIME * timetag);  // (i) pointer to time tag structure
#endif

#ifndef _CVI_
NOMANGLE BT_INT CCONV BusTools_RT_ReadNextMessage(
   int cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_RT_MBUF_READ *pRT_mbuf);
#endif

NOMANGLE BT_INT CCONV BusTools_RT_ReadLastMessage(
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_RT_MBUF_READ *pRT_mbuf);

NOMANGLE BT_INT CCONV BusTools_RT_ReadLastMessageBlock(
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr,
   BT_UINT *mcount,
   API_RT_MBUF_READ *pRT_mbuf);

NOMANGLE BT_INT CCONV BusTools_RT_Checksum1760(
   API_RT_MBUF_WRITE *mbuf, 
   BT_U16BIT *cksum, 
   int wdcnt);

#ifndef _CVI_   
NOMANGLE BT_INT CCONV BusTools_BM_ReadNextMessage(
   int cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);
#endif

NOMANGLE BT_INT CCONV BusTools_BM_ReadLastMessage(
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV BusTools_BM_ReadLastMessageBlock(
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV BusTools_BM_Checksum1760(
   API_BM_MBUF mbuf, 
   BT_U16BIT *cksum);

#ifndef _CVI_
NOMANGLE BT_INT CCONV BusTools_BC_ReadNextMessage(
   int cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BC_MBUF *pBC_mbuf);
#endif

NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessage(
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessageBlock(
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf);

#ifdef INCLUDE_VME_VXI_1553
NOMANGLE BT_INT CCONV BusTools_ReadVMEConfig(
   BT_INT    cardnum,      
   BT_U16BIT *vdata);

NOMANGLE BT_INT CCONV BusTools_WriteVMEConfig(
   BT_INT    cardnum,
   BT_UINT   offset,
   BT_U16BIT vdata);
#endif

NOMANGLE BT_INT CCONV BusTools_IRIG_Calibration(
   BT_UINT cardnum, BT_INT flag);

NOMANGLE BT_INT CCONV BusTools_IRIG_Config(
   BT_UINT cardnum,     // (i) card number
   BT_UINT intFlag,     // Internal IRIG or External IRIG
   BT_UINT outFlag);    // Output Internal IRIG 

NOMANGLE BT_INT CCONV BusTools_IRIG_SetTime(
   BT_UINT cardnum,     // (i) card number
   BT_U64BIT  timedate,
   BT_U32BIT IRIGTime);

NOMANGLE BT_INT CCONV BusTools_IRIG_SetBias(
   BT_UINT cardnum,     // (i) card number
   BT_INT bias);        // (i) value to ajust IRIG time

NOMANGLE BT_INT CCONV BusTools_IRIG_Valid(
   BT_UINT cardnum);

NOMANGLE BT_INT CCONV BusTools_API_GetBaseAddr(
  BT_UINT cardnum,
  unsigned long * baseAddress);

NOMANGLE char * CCONV BusTools_DataGetString(
   DATA_CONVERT *cdat); 

NOMANGLE BT_INT CCONV BusTools_1760_DataRead(
   BT_UINT cardnum, BT_U32BIT **saData, 
   BT_U32BIT *saEnable);

NOMANGLE BT_INT CCONV BusTools_1760_DataWrite(
   BT_UINT cardnum, BT_U16BIT **saData, 
   BT_U32BIT saEnable);

NOMANGLE BT_INT CCONV BusTools_DMA_Setup(
   BT_UINT cardnum,
   BT_UINT dma_channel,
   BT_U32BIT host_addr, 
   BT_U32BIT board_addr,
   BT_U32BIT byte_count,
   BT_U32BIT dma_flag); 

NOMANGLE BT_INT CCONV BusTools_DMA_Start(
  BT_UINT cardnum,
  BT_U32BIT dma_channel); 

NOMANGLE BT_INT CCONV BusTools_DMA_Setup2(
   BT_UINT cardnum, 
   BT_UINT dma_channel, 
   BT_VOID* host_addr,
   BT_U32BIT board_addr, 
   BT_U32BIT byte_count,
   BT_U32BIT dma_flag);

NOMANGLE BT_INT CCONV BusTools_DMA_Transfer(
   BT_UINT cardnum,
   BT_UINT dma_channel);

NOMANGLE BT_INT CCONV BusTools_DMA_Init(
   BT_UINT cardnum,
   BT_UINT dma_channel,
   BT_U32BIT size,
   BT_U32BIT flag);

NOMANGLE BT_INT CCONV BusTools_DMA_Close(
   BT_UINT cardnum,
   BT_UINT dma_channel);

NOMANGLE BT_INT CCONV BusTools_DMA_Read(
   BT_UINT cardnum,
   BT_UINT dma_channel,
   BT_UINT board_addr,
   BT_UINT byte_count,
   BT_UINT* buff);

NOMANGLE BT_INT CCONV BusTools_ReadBoardTemp(
   BT_INT cardnum,         // (i) card number
   BT_UINT location,       // (i) temp location
   BT_INT *tmp);           // (o) temperature

#ifdef __cplusplus
}
#endif

/*============================================================================*
 *  Function prototypes for the BusTools API functions which are defined in a
 *  user interface DLL.  These functions are called by the API (if they exist)
 *  whenever the associated API function is called.
 *===========================================================================*/
#if defined(_USER_DLL_)
// Function prototypes of the functions in the user interface dlls.
// Called by BusTools_API_Init() and BusTools_API_InitExtended():
NOMANGLE BT_INT CCONV UsrAPI_Init(
   BT_UINT   cardnum,        // (i) card number (0 - based)
   BT_U32BIT *base_address,  // (i) base address of board/carrier memory area
   BT_U32BIT *ioaddr,        // (i) board/carrier io address
   BT_UINT   *flag,          // (io) 0->emulation driver, 1->hardware driver, 2->Enable HW interrupts
   BT_UINT   *platform,      // (i) execution platform: PLATFORM_PC, PLATFORM_VMIC
   BT_UINT   *boardType,     // (i) IP1553, IP1553SF, IP1553MF, PCI1553 or ISA1553
   BT_UINT   *carrier,       // (i) NATIVE, IP1553_ISA, IP1553_PCI, IP1553_VME,
                             //     IP1553_CP, IP1553_VXI.
   BT_UINT   *slot,          // (i) SLOT_A, SLOT_B, SLOT_C, SLOT_D, etc.
   BT_UINT   *mapping);      // (i) carrier memory map: CARRIER_MAP_DEFAULT

// Called by BusTools_API_Close():
NOMANGLE BT_INT CCONV UsrAPI_Close(BT_UINT cardnum);

// Called by BusTools_BC_MessageAlloc():
NOMANGLE BT_INT CCONV UsrBC_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *count);         // (i) number of BC messages to allocate

// Called by BusTools_BC_MessageRead():
NOMANGLE BT_INT CCONV UsrBC_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *messno,         // (i) index of BC message to read
   API_BC_MBUF *api_message); // (o) user's buffer to write message into

// Called by BusTools_BC_MessageUpdate():
NOMANGLE BT_INT CCONV UsrBC_MessageUpdate(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mblock_id,      // (i) index of BC message to update
   BT_U16BIT *buffer);      // (i) pointer to data to copy to BC message

// Called by BusTools_BC_MessageWrite():
NOMANGLE BT_INT CCONV UsrBC_MessageWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *messno,         // (i) index of BC message to write to
   API_BC_MBUF *api_message); // (i) pointer to user-specified BC message

// Called by BusTools_BC_StartStop():
NOMANGLE BT_INT CCONV UsrBC_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> start BC (at message 0), 0 -> stop BC

// Called by BusTools_BM_MessageAlloc():
NOMANGLE BT_INT CCONV UsrBM_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mbuf_count,
   BT_UINT *mbuf_actual,
   BT_U32BIT *enable);

// Called by BusTools_BM_MessageRead():
NOMANGLE BT_INT CCONV UsrBM_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mbuf_id,
   API_BM_MBUF *mbuf);

// Called by BusTools_BM_StartStop():
NOMANGLE BT_INT CCONV UsrBM_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> Start BM, 0 -> Stop BM

// Called by BusTools_RT_CbufWrite():
NOMANGLE BT_INT CCONV UsrRT_CbufWrite(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *rtaddr,         // (i) RT address (0 - based)
   BT_UINT *subaddr,        // (i) RT subaddress (0 - based)
   BT_UINT *tr,             // (i) Transmit/Receive flag (1->rt transmit)
   BT_INT  *mbuf_count,     // if negative, one pass through buffers only
   API_RT_CBUF *apicbuf);   // (i) pointer to API RT control buf

// Called by BusTools_RT_MessageRead():
NOMANGLE BT_INT CCONV UsrRT_MessageRead(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *rtaddr,         // (i) RT address (0 - based)
   BT_UINT *subaddr,        // (i) RT subaddress (0 - based)
   BT_UINT *tr,             // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT *mbuf_id,        // (i) RT MBUF number
   API_RT_MBUF_READ *mbuf);

// Called by BusTools_RT_StartStop():
NOMANGLE BT_INT CCONV UsrRT_StartStop(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> start RT, 0 -> stop
#endif

#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)        /* Align structure elements to 1 byte boundry */
#endif

//---------------------------------------------------------------------------*
// Error reporting constants
//---------------------------------------------------------------------------*
/***********************************************************************
* BusTools Low Level Device Driver error return constants
***********************************************************************/
#ifndef _LOW_LEVEL_ERRORS_
#define _LOW_LEVEL_ERRORS_
#define BTD_OK                0     // CEI-INFO -- success
#define BTD_ERR_PARAM         1     // CEI-WARN -- invalid parameter
#define BTD_ERR_NOACCESS      2     // CEI-ERROR -- unable to map/access adapter
#define BTD_ERR_INUSE         3     // CEI-WARN -- adapter already in use
#define BTD_ERR_BADADDR       4     // CEI-ERROR -- invalid address
#define BTD_ERR_NODETECT      5     // CEI-ERROR -- I/O or Config register ID invalid, board detect fail
#define BTD_ERR_NOTSETUP      7     // CEI_ERROR -- adapter has not been setup
#define BTD_ERR_FPGALOAD      8     // CEI-ERROR -- FPGA load failure
#define BTD_ERR_NOMEMORY      10    // CEI-ERROR -- error allocating memory in SW version
#define BTD_ERR_BADADDRMAP    12    // CEI-ERROR -- bad initial mapping of address
#define BTD_ERR_BADEXTMEM     13    // CEI-ERROR -- bad extended memory mapping
#define BTD_ERR_BADBOARDTYPE  14    // CEI-ERROR -- Unknown board type
#define BTD_ERR_BADWCS        15    // CEI-ERROR -- Verify failure reading Writable Control Store
#define BTD_NO_PLATFORM       18    // CEI-ERROR -- Platform specified unknown or not supported
#define BTD_BAD_MANUFACTURER  19    // CEI-ERROR -- IP ID PROM Manufacturer code not 0x79
#define BTD_BAD_MODEL         20    // CEI-ERROR -- IP ID PROM Model number not 0x0C, 0x0D, 0x0E or 0x0F
#define BTD_BAD_SERIAL_PROM   21    // CEI-ERROR -- IP Serial PROM needs update, no support for this version
#define BTD_NEW_SERIAL_PROM   22    // CEI-ERROR -- Serial PROM too new, not supported by this software
#define BTD_CHAN_NOT_PRESENT  23    // CEI-WARN -- Channel not present (on multi-channel board)
#define BTD_NON_SUPPORT       24    // CEI-WARN -- Bus/Carrier/OS combination not supported by API
#define BTD_BAD_HW_INTERRUPT  25    // CEI-ERROR -- Hardware interrupt number bad or not defined in registry
#define BTD_FPGA_NOT_CLEAR    26    // CEI-ERROR -- The FPGA configuration failed to clear
#define BTD_BAD_CONF_FILE     29    // CEI-ERROR -- Unable to open ceidev.conf
#define BTD_NO_DRV_MOD        30    // CEI-ERROR -- No Driver Module found
#define BTD_IOCTL_DEV_ERR     31    // CEI-ERROR -- Error in ioctl get device
#define BTD_IOCTL_SET_REG     32    // CEI-ERROR -- Error in ioctl set region
#define BTD_IOCTL_REG_SIZE    33    // CEI-ERROR -- Error in getting ioctl region size
#define BTD_IOCTL_GET_REG     34    // CEI-ERROR -- Error in ioctl get region
#define BTD_BAD_SIZE          35    // CEI-ERROR -- Region size is 0
#define BTD_BAD_PROC_ID       36    // CEI-INFO -- Unable to set process ID
#define BTD_HASH_ERR          37    // CEI-INFO -- Unable to setup hash table
#define BTD_NO_HASH_ENTRY     38    // CEI-INFO -- No hash table entry found
#define BTD_WRONG_BOARD       39    // CEI-INFO -- Wrong board type for command
#define BTD_MODE_MISMATCH     40    // CEI-INFO -- IPD1553 mismatch in the mode.
#define BTD_IRIG_NO_LOW_PEAK  41    // CEI-INFO -- No lower peak on IRIG DAC calibration
#define BTD_IRIG_NO_HIGH_PEAK 42    // CEI-INFO -- No upper peak on IRIG DAC calibration
#define BTD_IRIG_LEVEL_ERR    43    // CEI-WARN -- Delta between MAX and MIN DAC peak values less than required
#define BTD_IRIG_NO_SIGNAL    44    // CEI-INFO -- No IRIG Signal Detected 
#define BTD_RTADDR_PARITY     45    // CEI-ERROR -- Parity Error on Hardwired RT address lines
#define BTD_BAD_BYTE_COUNT    47    // CEI-ERROR -- Byte count not on 4 byte boundary
#define BTD_TIMER_FAIL        48    // CEI-ERROR -- failed to create a timer

/**********************************************************************
*  Error return codes from LOWLEVEL routines
**********************************************************************/
#define BTD_ERR_NOWINRT       50    // CEI-ERROR -- WinRT driver not loaded/started
#define BTD_ERR_BADREGISTER   51    // CEI-ERROR -- WinRT parameters don't match registry
#define BTD_ERR_BADOPEN       52    // CEI-ERROR -- WinRT device open failed
#define BTD_UNKNOWN_BUS       53    // CEI-ERROR -- Bus is not PCI, ISA or VME
#define BTD_BAD_LL_VERSION    54    // CEI-ERROR -- Unsupported lowlevel driver installed
#define BTD_BAD_INT_EVENT     55    // CEI-ERROR -- Unable to create interrupt event
#define BTD_ISR_SETUP_ERROR   56    // CEI-ERROR -- Error setting up the ISR driver
#define BTD_CREATE_ISR_THREAD 57    // CEI-ERROR -- Error creating the ISR thread
#define BTD_NO_REGIONS_TO_MAP 58    // CEI-ERROR -- No regions requested in call to vbtMapBoardAddresses
#define BTD_RESOURCE_ERR      60    // CEI-ERROR -- Integrity Resource Error
#define BTD_READ_IODEV_ERR    61    // CEI-ERROR -- Integrity IO Device Read Error
#define BTD_MEMREG_ERR        62    // CEI-ERROR -- Integrity error getting memory region
#define BTD_MEM_MAP_ERR       63    // CEI-ERROR -- Integrity Memory Mapping error
#define BTD_CLK_RATE_NOT_SET  64    // CEI-ERROR -- Error setting clk rate

#define BTD_LL_CLOSE_ERR      65    // CEI_ERROR -- Failed to close 1553 device
#define BTD_LL_OPEN_ERR       66    // CEI_ERROR -- Failed to open 1553 device
#define BTD_LL_LINK_ERR       67    // CEI_ERROR -- Failure linking to installation library (vbtOpen1553Channel)


/**************************************************************************
* Error code from BTVXIMAP.C
**************************************************************************/
#define BTD_VIOPEN_FAIL       70    // CEI-ERROR -- viOpen Error
#define BTD_VIMAPADDRESS_FAIL 71    // CEI-ERROR -- viMapAddress Error
#define BTD_VIOPENDEFAULTRM   72    // CEI-ERROR -- viOpenDefaultRM Error
#define BTD_VIUNMAP_ERR       73    // CEI-ERROR -- viUnMapAddress Error

#define BTD_SEM_CREATE        80    // Error Creating semaphore
#define BTD_TASK_CREATE       81    // Error spawning task
/* Event Wait status  negative for error condition 0 for success and postive for timeout */
#define BTD_EVENT_WAIT_FAILED    -82   // CEI-ERROR -- Event Wait Failure
#define BTD_EVENT_WAIT_ABANDONED -83   // CEI-ERROR -- Event Wait Abandoned
#define BTD_EVENT_WAIT_TIMEOUT   84    // CEI-INFO -- Timeout on Event Wait
#define BTD_EVENT_WAIT_UNKNOWN   -85   // CEI-ERROR -- Unknown Event Error
#define BTD_EVENT_SIGNAL_ERR  86    // CEI-ERROR -- Error Occurred During Event Signal
#define BTD_SET_PRIORITY_ERR  87    // CEI-ERROR -- Error Setting Thread Priority
#define BTD_THRD_CREATE_FAIL  88    // CEI-ERROR -- Thread Create Failure

#define BTD_CLOSE_ERR         90    // CEI-ERROR -- Failed to close 1553 device
#define BTD_OPEN_ERR          91    // CEI-ERROR -- Failed to open 1553 device
#define BTD_VBT_OPEN_ERR      92    // CEI-ERROR -- Failure in vbtOpen1553Channel
#define BTD_FIND_DEV_ERR      93    // CEI_ERROR -- Failure in BusTools_FindDevice
#define BTD_LIST_DEV_ERR      94    // CEI_ERROR -- Failure in BusTools_ListDevices

#define BTD_ERR_SEM_OBJ       97    // CEI-ERROR -- Failed to open/close/destroy a semaphore
#define BTD_ERR_SEM_OPER      98    // CEI-ERROR -- Failed to lock/unlock a semaphore
#define BTD_ERR_EVENT_OBJ     99    // CEI-ERROR -- Failed to create/destroy an event object
#define BTD_ERR_SHMEM_OBJ     100   // failed to create/destroy shared memory mapping object
#define BTD_ERR_SHMEM_MAP     101   // failed to map/unmap shared memory block
#define BTD_ERR_SHMEM_ATTR    102   // failed to obtain shared memory block attributes
#define BTD_ERR_MUTEX_OBJ     103   // failed to create/destroy mutex object
#define BTD_ERR_MUTEX_ATTR    104   // failed to update mutex attributes
#define BTD_ERR_MUTEX_LOCK    105   // failed to acquire/release mutex
#define BTD_ERR_COND_OBJ      106   // failed to create/destroy condition variable object
#define BTD_ERR_COND_ATTR     107   // failed to update condition variable attributes
#define BTD_ERR_COND_OPER     108   // failed to wait/signal condition variable
#define BTD_ERR_IPC_KEY       109   // failed to obtain unique IPC key
#define BTD_ERR_CFGLOCK_OBJ   110   // failed to create/destroy config lock object
#define BTD_ERR_CFGLOCK_ATTR  111   // failed to get/set config lock attributes
#define BTD_ERR_CFGLOCK_LOCK  112   // failed to acquire/release config lock
#define BTD_ERR_READ_PCI_CFG  113   // failed to read PCI configuration register
#define BTD_ERR_LOAD_CEIINST  114   // failed to load cei_install library

#define BTD_ERR_USB_PROT      115   // CEI-ERROR -- Failure in the USB protocol
#define BTD_ERR_USB_OPER      116   // CEI-ERROR -- Failure in the USB operation
#define BTD_ERR_FIFO_OPER     117   // CEI-ERROR -- Failure in the FIFO operation

#endif //_LOW_LEVEL_ERRORS_


/**********************************************************************
*  Error return codes from BusTools API routines
**********************************************************************/
#define API_SUCCESS              0     // CEI-INFO -- No error detected
#define API_FEATURE_SUPPORT      120   // CEI-INFO -- Feature supported by board
#define API_CONTINUE             121   // CEI-INFO -- API function should continue execution normally
#define API_RETURN_SUCCESS       122   // CEI-INFO -- API function should return immediately with API_SUCCESS
#define API_NEVER_CALL_AGAIN     123   // CEI-INFO -- User function is never to be called again
#define API_INIT_NO_SUPPORT      124   // CEI-WARN -- Cannot initialize board type with this function
#define API_NO_CHANNEL_MAP       125   // CEI_WARN -- Channel mapping not support for current card type
#define API_OVER_TEMP_ALARM      130   // CEI_INFO -- Board Temperature sensor reporting Over Temp
#define API_BUSTOOLS_INT_USED    170   // CEI-WARN -- Interrupt on card already in use
#define API_NULL_PTR             171   // CEI-WARN -- NULL Pointer passed to function
#define API_MAX_CHANNELS_INUSE   180   // CEI-WARN -- Maximum 1553 channels already in use
#define API_CARDNUM_INUSE        181   // CEI-WARN -- cardnum in already in use
#define API_BAD_PRODUCT_LIST     182   // CEI-WARN -- Unable to build the Condor Engineering Product list
#define API_BAD_DEVICE_ID        183   // CEI-WARN -- Bad device ID
#define API_INSTALL_INIT_FAIL    184   // CEI-ERROR -- CEI_INSTALL init failure
#define API_NO_POLLING           190   // CEI-WARN -- Polling is not enabled
#define API_TIMER_ERR            191   // CEI-ERROR -- Error setting up polling timer
#define API_DAC_ERROR            192   // CEI-ERROR -- DAC Operation Error
#define API_TIMEOUT_ERR          193   // CEI-ERROR -- Timeout occurred during timed delay.
#define API_PARAM_CONFLICT       194   // CEI-INFO -- Conflicting parameters passed to function.
#define API_NO_INTERRUPT_SUPPORT 195   // CEI-ERROR -- Interrupts are not support for the board type
#define API_BUSTOOLS_INITED      201   // CEI-WARN -- This card has already been init'ed
#define API_BUSTOOLS_NOTINITED   202   // CEI-WARN -- BusTools API not initialized
#define API_BUSTOOLS_BADCARDNUM  203   // CEI-WARN -- Bad card number specified
#define API_BUSTOOLS_BADCOUPLING 206   // CEI-WARN -- Bad coupling specified in BusTools_SetVoltage
#define API_BUSTOOLS_BADVOLTAGE  207   // CEI-WARN -- Bad voltage specified in BusTools_SetVoltage
#define API_BUSTOOLS_EVENBCOUNT  209   // CEI_ERROR -- Even byte count required for this routine
#define API_BUSTOOLS_BADMEMORY   210   // CEI_ERROR -- BusTools Board Dual-Port Memory Self-Test Failed
#define API_BUSTOOLS_TOO_MANY    211   // CEI-WARN -- Too many user interrupt functions registered
#define API_BUSTOOLS_FIFO_BAD    212   // CEI_ERROR -- User API_INT_FIFO structure corrupted or bad entry
#define API_BUSTOOLS_NO_OBJECT   213   // CEI_ERROR -- Error creating event object or thread
#define API_BUSTOOLS_EVENADDR    214   // CEI_ERROR -- Even start address required for this routine
#define API_BUSTOOLS_NO_FILE     215   // CEI-WARN -- Could not open the specified file
#define API_BUSTOOLS_NO_MEMORY   216   // CEI-WARN -- BusTools_MemoryAlloc request overflows first 64 Kw of board memory
#define API_HW_IQPTR_ERROR       217   // CEI-CRIT-ERR -- Hardware Interrupt Pointer register error.
#define API_BIT_BC_RT_FAIL_PRI   218   // CEI-ERROR -- BIT failure/data error detected on BC-RT primary bus
#define API_BIT_BC_RT_FAIL_SEC   219   // CEI-ERROR -- BIT failure/data error detected on BC-RT secondary bus
#define API_BUSTOOLS_FIFO_DUP    220   // CEI-ERROR -- Specified API_INT_FIFO structure is already in use.V4.35.ajh
#define API_BIT_BM_RT_FAIL_PRI   221   // CEI-ERROR -- BIT failure/data error detected on BM-RT primary bus
#define API_BIT_BM_RT_FAIL_SEC   222   // CEI-ERROR -- BIT failure/data error detected on BM-RT secondary bus
#define API_STRUCT_ALIGN         223   // CEI-ERROR -- Structure Alignment Incorrect.
#define API_BUSTOOLS_DWORD_SIZE  224   // CEI-WARN -- DWORD byte count must be multiple of 4.
#define API_HARDWARE_NOSUPPORT   225   // CEI-WARN -- Function not supported by current hardware
#define API_OUTDATED_FIRMWARE    226   // CEI-WARN -- Firmware version no longer supported, contact factory for upgrade
#define API_NO_OS_SUPPORT        227   // CEI-WARN -- Function not supported by underlying Operating System
#define API_NO_BUILD_SUPPORT     228   // CEI-WARN -- Function not supported by API as built
#define API_CHANNEL_OPEN_OTHER   229   // CEI-WARN -- Board or channel already opened as another cardnum
#define API_SINGLE_FUNCTION_ERR  231   // CEI-WARN -- You have attempted to start multiple functions on a single function board
#define API_CANT_LOAD_USER_DLL   232   // CEI-WARN -- Cannot load specified user DLL
#define API_REGISTERFUNCTION_OFF 233   // CEI-WARN -- RegisterFunction operations not enabled
#define API_DUAL_FUNCTION_ERR    234   // CEI-WARN -- You have attempted to start RT and BC functions on a dual function board
#define API_SINGLE_RT_MODE_ERR   235   // CEI_WARN -- Attempting to start the Bus Controller while in single RT mode
#define API_BAD_PARAM	         240   // CEI-WARN -- Bad parameter for the function call
#define API_EI_BADMSGTYPE        252   // CEI-WARN -- Bad message type specified in EbufWrite
#define API_EI_ILLERRORNO        253   // CEI-WARN -- Error injection buffer num > number of buffers avail
#define API_EI_ILLERRORADDR      254   // CEI-WARN -- Illegal error buffer address
#define API_BAD_ADDR_TYPE        271   // CEI-WARN -- Bad address type for BusTools_GetAddr()
#define API_BAD_DISCRETE         280   // CEI-WARN -- Attempting to configure invalid discrete
#define API_OUTPUT_DISCRETE      281   // CEI-WARN -- Attempting to read from an output
#define API_INPUT_DISCRETE       282   // CEI-WARN -- Attempting to write to an input
#define API_MEM_ALLOC_ERR        283   // CEI-ERROR -- Error allocating memory
#define API_BADDATATYPE	         284   // CEI-WARN -- Bad data type for EU conversion. 
#define API_BADBCDDATA           285   // CEI-WARN -- Bad data for BCD EU conversion.
#define API_BADTRANSLATE         286   // CEI-WARN -- Bad translation table data, for translate EU conversion.
#define API_BADFACTORTYPE        287   // CEI-WARN -- Bad factor type for scaled EU conversion.
#define API_CHANNEL_SHARED       290   // CEI_WARN -- Channel already shared
#define API_CHANNEL_NOTSHARED    291   // CEI_WARN -- Channel not shared
#define API_SINGLE_FUNCTION      295   // CEI-INFO -- Board is single-function
#define API_DUAL_FUNCTION        296   // CEI-INFO -- Board is dual-function
#define API_MULTI_FUNCTION       297   // CEI-INFO -- Board is multi-function
#define API_SINGLE_RT            298   // CEI_INFO -- board is in single RT mode
#define API_TEMP_SENSOR_ERR      299   // CEI_INFO -- Temperature Sensor Error wrong manufacturer ID

#define API_BC_NOTINITED         301   // CEI-WARN -- BC_Init not yet called
#define API_BC_INITED            302   // CEI-WARN -- BC_Init already called
#define API_BC_RUNNING           303   // CEI-WARN -- BC currently running
#define API_BC_NOTRUNNING        304   // CEI-WARN -- BC not currently running
#define API_BC_MEMORY_OFLOW      305   // CEI-ERROR -- BC memory overflow
#define API_BC_ILLEGAL_MBLOCK    306   // CEI-WARN -- BC illegal mem block number specified
#define API_BC_MBLOCK_NOMATCH    307   // CEI-WARN -- BC specified addr is not a BC message block
#define API_BC_MBUF_NOT_ALLOC    308   // CEI-WARN -- BC message buffers have not been allocated
#define API_BC_MBUF_ALLOCD       309   // CEI-WARN -- BC message buffers already allocated
#define API_BC_ILLEGAL_NEXT      310   // CEI-WARN -- BC illegal next message number
#define API_BC_ILLEGAL_PREV      311   // CEI-WARN -- BC illegal prev message number
#define API_BC_ILLEGAL_BRANCH    312   // CEI-WARN -- BC illegal branch message number
#define API_BC_MESS1_COND        313   // CEI-WARN -- BC first message in buffer is conditional
#define API_BC_BAD_COND_ADDR     314   // CEI-WARN -- BC bad address value in conditional message
#define API_BC_BADTIMEOUT1       315   // CEI-WARN -- BC illegal "No Response" timeout
#define API_BC_BADTIMEOUT2       316   // CEI-WARN -- BC illegal "Late Response" timeout
#define API_BC_BADFREQUENCY      317   // CEI-WARN -- BC illegal minor frame frequency
#define API_BC_HALTERROR         318   // CEI-ERROR -- BC error detected during stop, bus is probably unterminated
#define API_BC_ILLEGAL_DBLOCK    320   // CEI-ERROR -- BC Illegal Data buffer number specified
#define API_BC_BOTHBUFFERS       323   // CEI-WARN -- BC cannot specify both buffers
#define API_BC_BOTHBUSES         324   // CEI-WARN -- BC cannot specify both buses
#define API_BC_UPDATEMESSTYPE    326   // CEI-WARN -- BC message update must operate on a message (not branch)
#define API_BC_ILLEGALMESSAGE    327   // CEI-ERROR -- BC message in memory is not legal
#define API_BC_ILLEGALTARGET     328   // CEI-WARN -- BC branch data message number not legal
#define API_BC_NOTMESSAGE        329   // CEI-WARN -- BC msg is not a proper 1553-type message
#define API_BC_NOTNOOP           330   // CEI-WARN -- BC msg is not a proper noop-type message
#define API_BC_APERIODIC_RUNNING 331   // CEI-WARN -- BC Aperiodics still running, cannot start new msg list
#define API_BC_APERIODIC_TIMEOUT 332   // CEI-ERROR -- BC Aperiodic messages did not complete in time
#define API_BC_CANT_NOOP         333   // CEI-WARN -- BC cannot noop or un-noop a noop message
#define API_BC_READ_TIMEOUT      335   // CEI-INFO -- RT timeout when attempting to read data.
#define API_BC_READ_NODATA       336   // CEI-INFO -- No BC data in int queue
#define API_BC_AUTOINC_INUSE     337   // CEI-WARN -- Auto-Increment in use for message
#define API_BC_IS_RUNNING        338   // CEI-INFO -- BC is running
#define API_BC_IS_STOPPED        339   // CEI-INFO -- BC is stopped
#define API_BC_MULTI_BUFFER_ERR  340   // CEI_WARN -- Using wrong function to setup Bus Controller.  Use BusTools_BC_BufferAlloc()"
#define API_BC_BAD_DATA_BUFFER   341   // CEI_WARN -- Data buffer not allocated
#define API_BM_NOTINITED         401   // CEI-WARN -- BM_Init or BM_MessageAlloc not called
#define API_BM_INITED            402   // CEI-WARN -- BM_Init already called
#define API_BM_RUNNING           403   // CEI-WARN -- BM currently running
#define API_BM_NOTRUNNING        404   // CEI-WARN -- BM not currently running
#define API_BM_MEMORY_OFLOW      405   // CEI-ERROR -- BM memory overflow
#define API_BM_ILLEGAL_ADDR      408   // CEI-WARN -- BM illegal RT address specified
#define API_BM_ILLEGAL_SUBADDR   409   // CEI-WARN -- BM illegal subaddress specified
#define API_BM_ILLEGAL_TRANREC   410   // CEI-WARN -- BM illegal trans/rec flag specified
#define API_BM_ILLEGAL_MBUFID    411   // CEI-WARN -- BM illegal mbuf_id for specified subunit
#define API_BM_MBUF_NOMATCH      412   // CEI-ERROR -- BM no match for specified address
#define API_BM_WRAP_AROUND       413   // CEI-ERROR -- BM API message buffer has overflowed, data has been lost
#define API_BM_MSG_ALLOC_CALLED  415   // CEI-WARN -- BM_MessageAlloc has already been called
#define API_BM_HW_WRAP_AROUND    416   // CEI-ERROR -- BM HW message buffer has overflowed, data has been lost
#define API_BM_POINTER_REG_BAD   417   // CEI-ERROR -- BM HW pointer register contents invalid
#define API_BM_READ_TIMEOUT      418   // CEI-INFO -- BM timeout when attempting to read data.
#define API_BM_READ_NODATA       419   // CEI-INFO -- No BM data in int queue
#define API_BM_1760_ERROR        420   // CEI-INFO -- Checksum error on MIL-STD-1760 message.
#define API_BM_BAD_HEAD_PTR      421   // CEI-ERROR -- BM Head pointer not within range. 
#define API_RT_NOTINITED         501   // CEI-WARN -- RT_Init not yet called
#define API_RT_INITED            502   // CEI-WARN -- RT_Init already called
#define API_RT_RUNNING           503   // CEI-WARN -- RT currently running
#define API_RT_NOTRUNNING        504   // CEI-WARN -- RT not currently running
#define API_RT_MEMORY_OFLOW      505   // CEI-ERROR -- RT memory overflow
#define API_RT_CBUF_EXISTS       506   // CEI-ERROR -- RT subunit MBUFs already allocated
#define API_RT_ILLEGAL_ADDR      508   // CEI-WARN -- RT illegal address specified
#define API_RT_ILLEGAL_SUBADDR   509   // CEI-WARN -- RT illegal subaddress specified
#define API_RT_ILLEGAL_TRANREC   510   // CEI-WARN -- RT illegal trans/rec flag specified
#define API_RT_ILLEGAL_MBUFID    511   // CEI-WARN -- RT illegal mbuf_id for specified subunit
#define API_RT_CBUF_BROAD        513   // CEI-WARN -- RT 31 is broadcast only
#define API_RT_CBUF_NOTBROAD     514   // CEI-WARN -- specified rt address is non-bro only
#define API_RT_MBUF_NOMATCH      515   // CEI-WARN -- RT message buffer not found at specified address
#define API_RT_BROADCAST_DISABLE 516   // CEI-WARN -- RT 31 Broadcast is disabled
#define API_RT_SELF_TEST_MODE    517   // CEI-WARN -- RT Self Test Wrap-Around Mode selected, normal operation inhibited
#define API_RT_READ_TIMEOUT      518   // CEI-INFO -- RT timeout when attempting to read data.
#define API_RT_READ_NODATA       519   // CEI-INFO -- No RT data in int queue
#define API_NO_HARDWIRE_RT       520   // CEI-INFO -- RT hardwired address not enabled.
#define API_RT_AUTOINC_INUSE     522   // CEI-WARN -- Auto-Increment already in use for message
#define API_SRT_OVERRIDE         530   // CEI_INFO -- Only one RT runs in sRT mode.  Current RT is overwritten.
#define API_BUSTOOLS_NO_SUPPORT  531   // CEI_INFO -- API function is not spport in this version firmware

#define API_ZEROXNG_SET          600   // CEI-WARN -- Only one zero crossing error can be injected in to a message

#define API_LV_BADARRAY          700   // CEI-ERROR -- LabView array structure not correctly setup
#define API_NO_LV_SUPPORT        701   // CEI-ERROR -- Function not supported in LabView
#define API_PLAYBACK_INIT_ERROR  801   // CEI-ERROR -- Error initializing Playback
#define API_PLAYBACK_BAD_THREAD  802   // CEI-ERROR -- Attempt to create thread failed
#define API_PLAYBACK_BAD_FILE    803   // CEI-ERROR -- File open failed
#define API_PLAYBACK_BAD_EVENT   804   // CEI-ERROR -- Event creation error
#define API_PLAYBACK_BUF_EMPTY   805   // CEI-ERROR -- Playback Buffer empty
#define API_PLAYBACK_BAD_EXIT    806   // CEI-ERROR -- Unexpected Exit from Playback
#define API_PLAYBACK_BAD_MEMORY  807   // CEI-ERROR -- Unable to allocate memory on Host
#define API_PLAYBACK_DISK_READ   808   // CEI-ERROR -- Disk read Error during playback
#define API_PLAYBACK_RUNNING     809   // CEI-WARN -- Playback is already running
#define API_PLAYBACK_BAD_ALLOC   810   // CEI-ERROR -- Failure to allocate enough BusTools Memory for PB
#define API_PLAYBACK_TIME_GAP    811   // CEI-INFO -- There larger gaps in time tags in playback file.
#define API_PLAYBACK_TIME_ORDER  812   // CEI-ERROR -- Time tags in playback file out of sequence.
#define API_PLAYBACK_FILE_ERR    814   // CEI-ERROR -- File is not .BMDX file 


#define API_TIMETAG_BAD_DISPLAY  901   // CEI-WARN -- Unknown or unsupported Time Tag display format
#define API_TIMETAG_BAD_INIT     902   // CEI-WARN -- Unknown Time Tag Initialization method
#define API_TIMETAG_BAD_MODE     903   // CEI-WARN -- Unknown Time Tag Operating Mode
#define API_TIMETAG_NO_DLL       904   // CEI-WARN -- DLL containing BusTools_TimeTagGet() could not be loaded
#define API_TIMETAG_NO_FUNCTION  905   // CEI-WARN -- Could not get the address of the BusTools_TimeTagGet() function
#define API_TIMETAG_USER_ERROR   906   // CEI-WARN -- User function BusTools_TimeTagGet() returned an error
#define API_TIMETAG_WRITE_ERROR  907   // CEI-ERROR -- Cannot write to time tag load register when in API_TM_IRIG mode
#define API_IRIG_NO_SIGNAL       908   // CEI-INFO -- No external IRIG signal present
#define API_TIMETAG_BAD_PERIOD   909   // CEI-WARN -- Time Tag period is not valid

#define API_USB_IF_ERROR         1001  // CEI-CRIT-ERR -- USB Fatal Interface Error
#define API_INSTALL_ERROR        0x80000000   // CEI-ERROR -- Install Error check

#define TST_SA_WRAP     1           // Subaddress wrap around (test mode)
#define RT_RCV          0           // RT Receive data
#define RT_XMT          1           // RT Transmit data

#define MAX_BT_USR_ALLOC_ENTRY	 256  // Maxinum recoring entries of the usr alloc memory
#endif  // #ifndef _BUSAPI_H_
