/*============================================================================*
 * FILE:                        B T D R V . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2017 BY ABACO SYSTEMS, INC. 
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
 * FUNCTION:   BusTools API low-level board access component data definitions.
 *
 * DESCRIPTION:  This module defines all of the local data structures needed
 *               to map, load and access the BusTools Boards.  All of the
 *               supported boards are defined here.
 *
 *===========================================================================*/

/* $Revision:  8.22 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  07/23/1999   Split data definitions from BTDRV.C into this file.V3.20.ajh
  01/18/2000   Added definitions for the ISA-1553.V4.00.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc
  02/19/2004   PCCard-D1553 Support
  01/02/2006   portibility mdofication
  11/19/2007   Add function prototype for vbtPageAccessSetupR15EC
  06/29/2009   Add support for the RXMC-1553
  04/14/2011   Add support for the RXMC2-1553 and R15-LPCIE
  05/14/2016   Add support for the RXMC2-1553 and R15-MPCIE
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#ifndef _BT_GLOBALS_
#define EXTERN extern
#else
#define EXTERN
#undef _BT_GLOBALS_
#endif
#include "lowlevel.h"

/* device number to cardnum translation. */
EXTERN int api_device[MAX_BTA];

/* This is the max number of Pointers needed to map the board's memory        */
/*  into the Host Address space.                                              */
#define BT_NUM_PAGES   8           /* 8 regions                               */
/* This parameter is used in memmap.c, and must track what's there!           */
EXTERN LPSTR bt_PageAddr[MAX_BTA][BT_NUM_PAGES]; /* Page addrs for each board */
EXTERN char  bt_UserDLLName[MAX_BTA][255];       /* Name of the User DLL      */
EXTERN int   hw_int_enable[MAX_BTA];             /* Hardware Interrupt enable */

/******************************************************************************/
/* The I/O control register is used to select the Writable Control Store,     */
/*   which is accessed in an address space which parallels the card memory.   */
/*   Whenever the card is reset or powered-up, the WCS must be reloaded.      */
/* The WCS data is written, 16 bits at a time, through the 2 Kbyte window to  */
/*   the associated WCS address.  Since the WCS is 48 bits wide, every 4th    */
/*   word is skipped when writing the WCS data to the card.  The API reads    */
/*   back the WCS to verify correct initialization of the IP.                 */
/*                                                                            */
/* The PCI/ISA-1553 board maps 32 megs of memory, and uses no IO space.  This */
/*   board is flat-mapped; there are no pages.  We only support this board    */
/*   under 32-bit environments since mapping would be tough in 16-bits.       */
/******************************************************************************/

/* Define the shift counts used to convert a board offset into a board frame: */
BT_UINT BT_PCI_MEMORY[MAX_BTA];                    /* Number of 1KB blocks of memory   */
#define BT_VME_MEMORY          1024       /* Number of 1KB blocks of memory   */

/* Define the base address of the RT segment */
#define BT_RT_BASE_PCI     0x00000L   /* Base address of RT-seg 1 PCI board   */
#define BT_RT_BASE_VME     0x00000L   /* Base address of RT-seg 1 PCI board   */

// Host Interface Registers for the PCI/ISA/PCC-1553 boards:
#define HIR_CSC_REG       (0/4)  /* DWORD Offset to Control/Status/Config Reg */
#define HIR_AC_REG         1     /* Additional capabilties register for QPM   */

// Definitions for the default board...byte offsets from the base address:
BT_UINT RAM_SIZE[MAX_BTA];               /* Default RAM size                  */
BT_UINT RAM_OFFSET[MAX_BTA];             /* Offset to 1553 data RAM           */

#define REG_OFFSET          0x30000      /* Offset to start of Register       */ 
#define HWREG_OFFSET        0x0          /* Offset to 1553 H/W Register       */
#define TTREG_OFFSET        0x1000       /* Offset to Time Tag Register       */
#define TRIG_OFFSET         0x2000       /* Offset to Trigger Registers       */
#define REG_SIZE            0x4000       /* Size of all Registers             */ 

#define SHARED_MEM_OFFESET  0x20000
#define SHARED_MEM_SIZE     0x1000

#define SMP_LOCKS_OFFSET    0x10000
#define SMP_LOCKS_SIZE      0x4000

#define COMMON_REG_OFFSET   0x0   /* this is for the v6 F/W                   */  
#define COMMON_REG_SIZE     0x400 

#define RAM_ADDR(a,b) RAM_OFFSET[a] + (CurrentCardSlot[a] * RAM_SIZE[a]) + b
#define REL_ADDR(a,b) b - (RAM_OFFSET[a] + (CurrentCardSlot[a] * RAM_SIZE[a]))

// Definitions for the VME-1553 board...byte offsets from the base address:
#define CHAN1_VME        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_VME        0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_VME        0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_VME        0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_VME     0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_VME       0x00000800    /* Offset to 1553 Control Register    */
#define REG_FILE_VME     0x00001000   /* Offset to Register File             */

// Definitions for the QPMC-1553 board...byte offsets from the base address:
#define CHAN1_QPMC       0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_QPMC       0x00200000   /* Offset to channel 2 specific fncts  */
#define CHAN3_QPMC       0x00400000   /* Offset to channel 3 specific fncts  */
#define CHAN4_QPMC       0x00600000   /* Offset to channel 4 specific fncts  */
#define DATA_RAM_QPMC    0x00100000   /* Offset to 1553 data RAM area        */
#define HW_REG_QPMC      0x00000800   /* Offset to 1553 Control Register     */
#define REG_FILE_QPMC    0x00001000   /* Offset to Register File             */

// Definitions for the PCC-D1553 board...byte offsets from the base address:
#define CHAN1_PCCD        0x00000000   /* Offset to channel 1 specific fncts  */
#define CHAN2_PCCD        0x00001000   /* Offset to channel 2 specific fncts  */
#define DATA_RAM_PCCD     0x00000800   /* Offset to 1553 data RAM area        */
#define HW_REG_PCCD       0x00000200   /* Offset to 1553 Control Register     */
#define REG_FILE_PCCD     0x00000400   /* Offset to Register File             */

/* V5 F/W Page indicies */
#define V5_RAM_PAGE    0
#define V5_HWREG_PAGE  1
#define V5_RAMREG_PAGE 2
#define V5_CSC_PAGE    3 

#define UCA32 0x4000

/* Page indices       */
#define CSC_REG_PAGE  0
#define BASE_PAGE     0
#define SMP_LOCK_PAGE 1
#define SHR_MEM_PAGE  2
#define HWREG_PAGE    3
#define TTREG_PAGE    4
#define TRIG_PAGE     5
#define RAM_PAGE      6
#define BM_SIM_PAGE   7
#define NUM_PAGES     8
 
#define MAX_CHANNEL_COUNT   4
#define CHANNEL_COUNT_MASK  0x07c0       /* Mask in the channel count           */
#define CHANNEL_COUNT_SHIFT 6
#define MODE_MASK 0x0800   
#define MODE_SHIFT 11
#define IRIG_FLAG 0x1000

#define BT_02KBFRAMESHIFT_ISA   (11)      /* PCCD-1553 2KB page frame          */
#define BT_02KBPTRSHIFT_ISA     (23)      /* PCCD-1553 2KB pointer index       */
#define BT_02KBOFFSETMASK_ISA 0x000007FFL /* PCCD-1553 extract 2KB offset      */
#define BT_FRAME_MASK_ISA     0xFFFF   /* Frame bits for the ISA1553 frame reg*/
/******************************************************************************/
/* Module Private (Static) Data Base.  Shared with HWSETUP.C and IPSETUP.C    */
/******************************************************************************/

EXTERN BT_U32BIT bt_OffsetMask[MAX_BTA];  // Extracts page offset from addr.
EXTERN BT_U16BIT bt_FrameShift[MAX_BTA];  // Shift to get Frame Register value.
EXTERN BT_U16BIT bt_PtrShift[MAX_BTA];    // Shift FR for index to addr pointer.
EXTERN BT_U32BIT bt_FrameMask[MAX_BTA];   // Frame register mask.
EXTERN BT_U16BIT bt_FrameReg[MAX_BTA];    // Current frame register setting.

EXTERN char *bt_iobase[MAX_BTA];          // I/O addresses, per board.

EXTERN BT_U32BIT MemoryMapSize[MAX_BTA];  // Size of region mapped by carrier.
EXTERN DEVMAP_T bt_devmap[MAX_BTA];       // Struture for vbtMapBoardAddresses

/******************************************************************************/
/* Module Private Function Definitions, shared with HWSETUP.C                 */
/******************************************************************************/
EXTERN BT_INT vbtBoardAccessSetup(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT phys_addr);

BT_INT vbtMapUserBoardAddress(BT_UINT cardnum, BT_INT wOpenFlag,
                              BT_U32BIT phys_addr, BT_U32BIT io_addr,
                              char * UserDLLName);


#ifdef INCLUDE_VME_VXI_1553
BT_INT vbtPageAccessSetupVME(BT_UINT cardnum, unsigned mem_addr, char *lpbase);
#endif

BT_INT vbtPageAccessSetupQPMC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupQPCX(BT_UINT, char *);
BT_INT vbtPageAccessSetupQCP(BT_UINT, char *);
BT_INT vbtPageAccessSetupQ104(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupR15EC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupRXMC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupR15PMC(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupRXMC2(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupLPCIE(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupMPCIE(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupUSB(BT_UINT cardnum);
BT_INT vbtPageAccessSetupRAR15(BT_UINT cardnum, char *lpbase);
BT_INT vbtPageAccessSetupRAR15XT(BT_UINT cardnum, char *lpbase);

#ifdef INCLUDE_PCCD
BT_INT vbtPageAccessSetupPCCD(BT_UINT cardnum, char *lpbase);
#endif //INCLUDE_PCCD

#if defined(__WIN32__)
void vbtAcquireFrameRegister(
   BT_UINT   cardnum,           // (i) card number.
   BT_UINT   flag);             // (i) 1=Acquire Frame Critical Section,
                                //     0=Release Critical Section.
#else
#define vbtAcquireFrameRegister(p1,p2)
#endif

LPSTR vbtMapPage(
   BT_UINT    cardnum,       // (i) card number
   BT_U32BIT  offset,        // (i) byte address within adapter memory
   BT_U32BIT *pagebytes,     // (o) number of bytes remaining in page
   BT_U16BIT *framereg);     // (o) value of frame register which maps page

void vbtSetFrame(BT_UINT cardnum, BT_U16BIT frame);

BT_U16BIT vbtAcquireFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame);        // (i) actual value to load into frame register

BT_UINT vbtPageDataCopy(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer to copy data to
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory
   BT_UINT   bytesToRead,   // (i) number of bytes to copy
   int       direction);    // (i) 0=read from board, 1=write to board

void vbtReleaseFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame);

#if defined(DEMO_CODE)
BT_INT vbtDemoSetup(BT_UINT cardnum);
BT_INT vbtDemoShutdown(BT_UINT cardnum);
#else
#define vbtDemoSetup(p1)
#define vbtDemoShutdown(p1)
#endif

void get_cardID(CEI_UINT *cndx);
void get_dev_chan(CEI_UINT *dchan);
void get_dev_id(CEI_UINT *devid);

#if defined(DEMO_CODE)
BT_INT vbtDemoSetup(BT_UINT cardnum);
BT_INT vbtDemoShutdown(BT_UINT cardnum);
#else
#define vbtDemoSetup(p1)
#define vbtDemoShutdown(p1)
#endif

// Host Interface Registers for the PCI/ISA/PCC-1553 boards:
#define HIR_CSC_REG       (0/4)  /* DWORD Offset to Control/Status/Config Reg */
#define HIR_AC_REG         1     /* Additional capabilties register for QPM   */
#define HIR_CONFIG_10K    (4/4)  /* DWORD Offset to 10K Conf Load Data Reg    */
#define HIR_PAGE_1        (8/2)  /* WORD  Offset to Page Register Channel 1   */
#define HIR_JUMPER_REV    (10/2) /* WORD  Offset to Jumpers/Revision ID Reg   */
#define HIR_IRQ_ENABLE    (12/2) /* WORD  Offset to Interrupt Enable Register */
#define HIR_PAGE_2        (14/2) /* WORD  Offset to Page Register Channel 2   */

#define HIR_QPAGE_1        (16/2)  /* WORD  Offset to Page Register Channel 1  */
#define HIR_QPAGE_2        (18/2)  /* WORD  Offset to Page Register Channel 2  */
#define HIR_QPAGE_3        (20/2)  /* WORD  Offset to Page Register Channel 3  */
#define HIR_QPAGE_4        (22/2)  /* WORD  Offset to Page Register Channel 4  */
