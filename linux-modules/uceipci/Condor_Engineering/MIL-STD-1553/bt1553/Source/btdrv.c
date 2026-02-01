/*============================================================================*
 * FILE:                        B T D R V . C
 *============================================================================*
 *      COPYRIGHT (C) 1998-2019 BY ABACO SYSTEMS, INC. 
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
 * FUNCTION:   BusTools/1553-API Library:
 *             Low-level board access component.
 *             This file is used for all O/S, the difference
 *             is the mapping of the board into the program's address space.
 *             This module performs the actual memory-mapped reads and writes
 *             from/to the BusTools board.
 *
 *             The only user interface to these functions is (and should be)
 *             through the higher-level functions provided by the API.
 *
 *             All routines within this module assume that a successful call
 *             to vbtSetup() has been performed before they are called, and
 *             that their arguements are all valid.  Error checking within
 *             this module is minimal to provide the best possible performance.
 *
 *             This module also supports a simulation or demo mode, which
 *             simulates the operation of a board without having the hardware
 *             actually installed.  This mode is triggered by a flag passed
 *             to the vbtSetup() function by the BusTools_API_Init() or the
 *             BusTools_API_InitExtended() function.
 *
 * DESCRIPTION:  See vbtSetup() for a description of the functions performed
 *               by this module.
 *
 * API ENTRY POINTS: :
 *    vbtSetup             Sets specified adapter for read/write access. 
 *    vbtShutdown          Disables access to specified adapter.
 *    v6GetRegister       Returns the value of specified adapter register.
 *    v6SetRegister
 *    v6ReadRAM              Reads range in 16 bits (1 word) of memory.
 *    v6ReadRAM32            Reads range in 32 bits (2 words) of memory 
 *    v6WriteRAM            Writes data from buffer into adapter memory.
 *    v6WriteRAM32            Writes data from buffer into adapter memory.
 *    v6ReadTimeTag       Reads the time tag counter.
 *    v6WriteTimeTag      Write to the time tag counter.
 *===========================================================================*/

/* $Revision:  8.28 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/31/1999   Finished the hardware interrupt support code.V3.03.ajh
  04/26/1999   Fixed PC-1553 code to support mapping at E000:0000.  Fixed
               lowlevel.c interface problems with PC-1553 when using offset
               device numbers from the BUSAPI32.INI file.V3.05.ajh
  07/23/1999   Moved functions from btdrv.c into hwsetup.c and ipsetup.c.V3.20.ajh
  08/30/1999   Added ability to change polling interval through .ini file.V3.20.ajh
  12/30/1999   Changed code to clean up the conditional compiles.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  02/23/2000   Completed support for the IP-1553 PROM Version 5.V4.01.ajh
  07/31/2000   Fixed error in vbtBoardAccessSetup for the ISA-1553.V4.08.ajh
               Fix vbtSetup to not call GetPrivateProfileInt in Win3.11 to
               read the board address(it returns only 16-bits).V4.09.ajh
  08/19/2000   Modify vbtSetup in DOS mode to set bt_inuse[cardnum].  Modify
               vbtShutdown to handle the ISA-1553 in Win 3.11.V4.11.ajh
  11/06/2000   Fix vbtSetup in TNT mode to initialize DumpOnBMStop[cardnum]
               so the dump function is not automatically called.V4.20.ajh
  01/07/2002   Added supprt for Quad-PMC and Dual Channel IP V4.46 rhc
  10/22/2003   Add support for 32-Bit memory access
  11/25/2003   Fixed SlotOpened problem.
  02/19/2004   BusTools_API_Open and PCCard-D15553
  12/30/2005   Modified for improve portability and common vbtSetup
  12/30/2005   Add vbtRead_iq function for more efficient processing of IQ data.
  08/30/2006   Add AMC-1553 Support
  12/07/2006   Remove platform specific include and initalization.
  11/19/2007   Added code for vbtSetPLXRegister32 and vbtSetPLXRegister8.The 
               functions were added in support of DMA.
  06/29/2009   Add support for the RXMC-1553
  12/03/2010   Move vbtSetDicrete and vbtGetDiscrete from hwsetup.c to btdrv.c
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API  
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
  12/01/2017   Updated bt_iobase pointer cast in vbtSetup for non-Windows cases.
 */

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#define _BT_GLOBALS_  /* This module allocates the memory for the driver      */
#include "btdrv.h"
#include "lowlevel.h"

#if 0
#define debugMessageBox(a, b, c, d)  MessageBox((a), (b), (c), (d))
#else
#define debugMessageBox(a, b, c, d)
#endif

/*---------------------------------------------------------------------------*
 *                    Static Data Base
 *---------------------------------------------------------------------------*/
static int SlotOpened[MAX_BTA][MAX_BTA] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},};   // Contains cardnum+1 if entry open.

/*===========================================================================*
 * EXTERNAL ENTRY POINT:  v b t S e t u p
 *===========================================================================*
 *
 * FUNCTION:    Setup specified adapter for read/write access.
 *
 * DESCRIPTION: Specified adapter is initialized, or the software demo mode
 *              is initialized.
 *
 *      It will return:
 *              BTD_OK if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtSetup(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT phys_addr,     // (i) 32 bit base address of board
   BT_U32BIT ioaddr,        // (i) 32 bit I/O address
   BT_UINT   HWflag)        // (i) hw/sw flag: 0->sw, 1->hw, 2->HW interrupt enable
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int       status;
   int       i;

   AddTrace(cardnum, NVBTSETUP, bt_inuse[cardnum], phys_addr, ioaddr, 0, 0);
   /*******************************************************************
   *  Basic parameter and state verification
   *******************************************************************/
   if ( bt_inuse[cardnum] )
      return BTD_ERR_INUSE;       // Return card already initialized.

   /*******************************************************************
   *  Save the I/O base register address, and set page to 0
   *******************************************************************/
   _HW_WCSRev[cardnum]  = 0;           //
#ifdef __WIN32__
   bt_iobase[cardnum] = (char *)((CEI_UINT64)ioaddr);        // Used to setup the page frame reg. - extraneous CEI_UINT64 cast for Wp64 warning
#else
   bt_iobase[cardnum] = (char *)((CEI_NATIVE_ULONG)ioaddr);  // Used to setup the page frame reg.
#endif
   // We cannot change the frame register until we have mapped the board...
 
   for (i = 0; i < BT_NUM_PAGES; i++)
      bt_PageAddr[cardnum][i] = NULL;  // Initialize the Host Address Pointers.

   if ( phys_addr < MAX_BTA )
      api_device[cardnum] = phys_addr; 
   else
      api_device[cardnum] = cardnum;

   hw_int_enable[cardnum] = HWflag;

   // Clear out the Register Function pointer array.
   RegisterFunctionOpen(cardnum);

   // Setup the API Bus Monitor buffer for this card.
   if(CurrentCardType[cardnum] == R15USB)
      NAPI_BM_V6BUFFERS[cardnum] = NAPI_BM_BUFFERS * 4;
   else
      NAPI_BM_V6BUFFERS[cardnum] = NAPI_BM_BUFFERS; 

   lpAPI_BM_Buffers[cardnum] = (char *)(CEI_MALLOC(NAPI_BM_V6BUFFERS[cardnum] * sizeof(API_BM_MBUF)));

   nAPI_BM_Head[cardnum] = nAPI_BM_Tail[cardnum] = 0;    // FIFO is empty.
   if ( lpAPI_BM_Buffers[cardnum] == NULL )
      return BTD_ERR_NOMEMORY;

#ifdef _USER_INIT_
   /*******************************************************************
   *  software simulation 
   *******************************************************************/
   if(CurrentPlatform[cardnum] == PLATFORM_USER)
   {
      // sanity-check to make sure we can treat CurrentMemMap[cardnum] as a pointer
      if ((sizeof(&bt_UserDLLName[cardnum][0])) != (sizeof(CurrentMemMap[cardnum])))
         return BTD_ERR_PARAM;
      strcpy(bt_UserDLLName[cardnum],(char *)((CEI_UINT64)CurrentMemMap[cardnum]));/* extraneous CEI_UINT64 cast for Wp64 warning */ 
   }
#endif //_USER_INIT_

   if ( HWflag == 0 )
   {
#ifdef DEMO_CODE
      // Simulation, allocate 256 Kbytes of RAM
      bt_inuse[cardnum] = -1;      // Set software version of driver.V4.30.ajh

      if ( (status = vbtDemoSetup(cardnum)) != BTD_OK )
      {
         bt_inuse[cardnum] = 0;    // Driver not initialized.V4.30.ajh
         return status;
      }
#else
      return API_NO_BUILD_SUPPORT;
#endif //DEMO_CODE      return API_NO_BUILD_SUPPORT;
   }
   else
   {
      // See if the device and slot we are trying to open is already open.
      if ( SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] )
      {
            return API_CHANNEL_OPEN_OTHER;
      }

      // Initialize a Critical Section to protect the HW frame register.
      // We do this even for boards that do not have a frame register,
      //  except for demo mode.
      CEI_MUTEX_CREATE(&hFrameCritSect[cardnum]);

      //  Hardware Version, setup paged access to BusTools Adapter.
      bt_inuse[cardnum] = 1;         // Set hardware version of driver.
      if ( (status = vbtBoardAccessSetup(cardnum,phys_addr)) != BTD_OK )
      {
         vbtShutdown(cardnum);       // Release mapped memory, etc.
         return status;
      }
   }

   /*******************************************************************
   *  Setup the timer callback polling routine, error reporting thread
   *   and hardware interrupt functions.
   *******************************************************************/
   SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] =
              cardnum+1;

   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
   {
      API_InterruptInit(cardnum,1);
      status = vbtInterruptSetup(cardnum, hw_int_enable[cardnum],
                                 api_device[cardnum]);
      if ( status )
         vbtShutdown(cardnum);
   }

   return status;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:           v b t S h u t d o w n
 *===========================================================================*
 *
 * FUNCTION:    Disables access to specified adapter.
 *
 * DESCRIPTION: The specified adapter is marked shutdown, all allocated memory
 *              is freed, and the memory-mapping selectors are released.  The
 *              timer callback is distroyed if this is the last operational
 *              adapter.
 *
 *      It will return:
 *              Nothing.
 *===========================================================================*/

void vbtShutdown( BT_UINT   cardnum)       // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int     bt_inuse_local;

   /*******************************************************************
   *  Basic parameter and state verification
   *******************************************************************/
   AddTrace(cardnum, NVBTSHUTDOWN, bc_running[cardnum],
                     bm_running[cardnum], rt_running[cardnum], 0, 0);

#ifdef INCLUDE_USB_SUPPORT
   if(CurrentCardType[cardnum] == R15USB)
      MSDELAY(1000);
#endif // INCLUDE_USB_SUPPORT

   bt_inuse_local      = bt_inuse[cardnum];
   bm_running[cardnum] = 0;         // BM is shutdown.
   bt_inuse[cardnum]   = 0;         // Card is shutdown.

   if ( bt_inuse_local == 0 )
      return;                       // Card is already shutdown.
   /*******************************************************************
   *  If the software version of the driver is running, we must release
   *   the memory we allocated to simulate the board.  If the hardware
   *   version is running, the protected mode selectors must be released.
   *  Only selectors which map memory above 1 meg must be released.
   *******************************************************************/
   if ( bt_inuse_local < 0 )  // -1=SW version.
   {
      /*******************************************************************
      *  Release the Bus Monitor buffer we allocated.
      *******************************************************************/
      if ( lpAPI_BM_Buffers[cardnum] != NULL )
      {
         CEI_FREE(lpAPI_BM_Buffers[cardnum]);
         lpAPI_BM_Buffers[cardnum] = NULL;
      }
      return;
   }

   RegisterFunctionClose(cardnum);  // Close down user threads.

   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
   {
      vbtInterruptClose(cardnum);      // Close down interrupt and thread processing.
   }
   /*******************************************************************
   *  Clear all of the DLL function addresses.
   *******************************************************************/
#if defined(_USER_DLL_)
   pUsrAPI_Close[cardnum]        = NULL;
   pUsrBC_MessageAlloc[cardnum]  = NULL;
   pUsrBC_MessageRead[cardnum]   = NULL;
   pUsrBC_MessageUpdate[cardnum] = NULL;
   pUsrBC_MessageWrite[cardnum]  = NULL;
   pUsrBC_StartStop[cardnum]     = NULL;
   pUsrBM_MessageAlloc[cardnum]  = NULL;
   pUsrBM_MessageRead[cardnum]   = NULL;
   pUsrBM_StartStop[cardnum]     = NULL;
   pUsrRT_CbufWrite[cardnum]     = NULL;
   pUsrRT_MessageRead[cardnum]   = NULL;
   pUsrRT_StartStop[cardnum]     = NULL;
#endif

   /*******************************************************************
   *  Release the Bus Monitor buffer we allocated.
   *******************************************************************/
   if ( lpAPI_BM_Buffers[cardnum] != NULL )
   {
      CEI_FREE(lpAPI_BM_Buffers[cardnum]);
      lpAPI_BM_Buffers[cardnum] = NULL;
   }

   CEI_MUTEX_DESTROY(&hFrameCritSect[cardnum]);

   SlotOpened[api_device[cardnum]][CurrentCardSlot[cardnum]] = 0;
   // In 95/98/NT/2000 free the critical section protecting the frame register.
   // We do this even for boards which do not need a critical section...
  
#ifdef _USER_INIT_
   if ( CurrentPlatform[cardnum] == PLATFORM_USER )
   {
      // Call the user supplied function in the user specified DLL,
      //  and close it down.  Ignor errors since we are shutting down.
      vbtMapUserBoardAddress(cardnum, 0, 0, 0, bt_UserDLLName[cardnum]);
   }
#endif //_USER_INIT_

   if ( CurrentPlatform[cardnum] == PLATFORM_PC && CurrentCardType[cardnum] != R15USB)
      vbtFreeBoardAddresses(&bt_devmap[api_device[cardnum]]);

#ifdef INCLUDE_USB_SUPPORT
   if(CurrentCardType[cardnum] == R15USB)
      usb_close(cardnum);
#endif //#ifdef INCLUDE_USB_SUPPORT
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 5 R e a d T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Read the value of the 45-bit time tag counter register.
 *
 * DESCRIPTION: The value of the time tag register is read and returned.
 *              Care is taken to insure that the components do not wrap
 *              while reading the 3 16-bit words.  Byte offsets 0x2A-0x2F.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

void v5ReadTimeTag(
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_U32BIT * timetag)        // (o) resulting 64-bit time value from HW
{
  BT_U16BIT volatile *hw_ttc;  // pointer to the time tag registers(3)
  BT_U16BIT volatile *hw_ttrd; // new time read register address

  // pointer to the "Time Tag Read-back Register Load"
  hw_ttc = ((BT_U16BIT*)(bt_PageAddr[cardnum][1])+HWREG_READ_T_TAG);
  // pointer to the "Time Tag Read-back Register"
  hw_ttrd  = ((BT_U16BIT*)(bt_PageAddr[cardnum][1])+HWREG_READ_T_TAG386);

  // Now read the time tag counter
  *hw_ttc = 0;  // initialize the time tag counter read register

#if defined (PPC_SYNC)
  IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
  eieio
  sync
#pragma endasm
#endif
     
  // read the three words of the TT counter
  ((BT_U16BIT*)timetag)[0] = *hw_ttrd;
  hw_ttrd++;
  ((BT_U16BIT*)timetag)[1] = *hw_ttrd;
  hw_ttrd++;
  ((BT_U16BIT*)timetag)[2] = *hw_ttrd;
  ((BT_U16BIT*)timetag)[3] = 0;

  fliplong(&timetag[0]);
  fliplong(&timetag[1]);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 R e a d T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Read the value of the 45-bit time tag counter register.
 *
 * DESCRIPTION: The value of the time tag register is read and returned.
 *              Care is taken to insure that the components do not wrap
 *              while reading the 3 16-bit words.  Byte offsets 0x2A-0x2F.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

void v6ReadTimeTag(
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_U32BIT * timetag)        // (o) resulting 64-bit time value from HW
{
   BT_U32BIT volatile *tt_latch; // Pointer to the time tag registers(3)
   BT_U32BIT volatile *tt_rdlow; // new time read register address
   BT_U32BIT volatile *tt_rdhigh;// new time read register address
   BT_U32BIT jdata = 0x0;
   
   tt_latch   = (BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + TTREG_LATCH;
   tt_rdlow   = (BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + TTREG_READ_LOW;
   tt_rdhigh  = (BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + TTREG_READ_HIGH;

   // Now latch the time tag counter
   *tt_latch = jdata; 

#if defined (PPC_SYNC)
   IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
   eieio
   sync
#pragma endasm
#endif
     
   timetag[0] = *tt_rdlow;      // Read the three words of the TT counter     
   timetag[1] = *tt_rdhigh;     // The write latches the current value?      

   fliplong(&timetag[0]);
   fliplong(&timetag[1]);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 5 W r i t e T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v5WriteTimeTag(
   BT_UINT   cardnum,             // (i) card number
   BT1553_TIME * timetag)     // (i) 48-bit time value to load into register
{
   BT_U16BIT *hw_ttlc;        // Pointer to the Time Tag load registers (3)

   // Write the time tag load register depending on the board type.
   // Caller verified that this board has a writable time tag load register.

   // Get a pointer to the ISA/PCI/PMC-1553 time tag load register
   hw_ttlc = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+0x20/2);

#if defined (PPC_SYNC)
      IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
 eieio
 sync
#pragma endasm
#endif

   // Load the time tag load register with the specified value

   hw_ttlc[0] = flipws((BT_U16BIT)(timetag->microseconds));       // for PMC on PowerPC   //Endian code
   hw_ttlc[1] = flipws((BT_U16BIT)(timetag->microseconds >> 16)); // for PMC on PowerPC   //Endian code
   hw_ttlc[2] = flipws((BT_U16BIT)timetag->topuseconds);          // for PMC on PowerPC   //Endian code
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e T i m e T a g
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6WriteTimeTag(
   BT_UINT   cardnum,             // (i) card number
   BT1553_TIME * timetag)     // (i) 48-bit time value to load into register
{
   BT_U32BIT *hw_ttlc;        // Pointer to the Time Tag load registers (2)

   // Write the time tag load register depending on the board type.
   // Caller verified that this board has a writable time tag load register.

   // Get a pointer to the ISA/PCI/PMC-1553 time tag load register
   hw_ttlc = ((BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + TTREG_LOAD_LOW);

   // Load the time tag load register with the specified value
   hw_ttlc[0] = fliplongs(timetag->microseconds);       // for PMC on PowerPC   //Endian code
   hw_ttlc[1] = fliplongs(timetag->topuseconds);        // for PMC on PowerPC   //Endian code

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 5 W r i t e T i m e T a g I n c r
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v5WriteTimeTagIncr(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT incr)                // (i) increment value
{
   BT_U16BIT *hw_ttlc;        // Pointer to the Time Tag icrement load registers

   // Get a pointer to the time tag load increment register
   hw_ttlc = ((BT_U16BIT *)(bt_PageAddr[cardnum][1])+0x26/2);

#if defined (PPC_SYNC)
      IO_SYNC;
#elif defined (INTEGRITY_PPC_SYNC)
#pragma asm
 eieio
 sync
#pragma endasm
#endif

   // Load the time tag load register with the specified value
   hw_ttlc[0] = flipws((BT_U16BIT)(incr));       //
   hw_ttlc[1] = flipws((BT_U16BIT)(incr >> 16)); //
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e T i m e T a g I n c r
 *===========================================================================*
 *
 * FUNCTION:    Write the value of the 45-bit time tag load register.
 *
 * DESCRIPTION: The value of the time tag load register is set to the specified
 *              value.
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6WriteTimeTagIncr(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT incr)                // (i) increment value
{
   BT_U32BIT *hw_ttlc;        // Pointer to the Time Tag icrement load registers

   // Get a pointer to the time tag load increment register
   hw_ttlc = (BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + TTREG_INCREMENT;

   // Load the time tag load register with the specified value
   hw_ttlc[0] = fliplongs(incr);       //
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d H I F
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of HIF memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadHIF(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of words to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter


   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]+byteOffset))[i];
      fliplong(&lpbuffer[i]);     // for PowerPC
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e H I F 
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteHIF(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,    // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   for ( i = 0; i < wordsToWrite; i++ )
   {
      ((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]+byteOffset))[i] =  fliplongs(lpbuffer[i]);
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d F L A S H
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of HIF memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadFlash(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   wordToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter


   for ( i = 0; i < wordToRead; i++ )
   {
      ((BT_U32BIT *)lpbuffer)[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]+byteOffset))[i];
      fliplong(&(((BT_U32BIT *)lpbuffer)[i])); // for PowerPC
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e F L A S H
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteFLASH(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,    // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   for ( i = 0; i < bytesToWrite/4; i++ )
   {
      ((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]+byteOffset))[i] =  fliplongs(((BT_U32BIT *)lpbuffer)[i]);
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R A M 8
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadRAM8(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter

   for ( i = 0; i < bytesToRead; i++ )
   {
      lpbuffer[i] = ((char *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i];
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d R A M
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadRAM(
   BT_UINT   cardnum,        // (i) card number
   BT_U16BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead)     // (i) number of word to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter

   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i];
      flipw(&lpbuffer[i]); // for PowerPC
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R A M
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v5ReadRAM(
   BT_UINT   cardnum,        // (i) card number
   BT_U16BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)     // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   if (CurrentCardType[cardnum] == PCCD1553)
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,(LPSTR)lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else 
   {
      for ( i = 0; i <bytesToRead/2; i++ )
     {
        lpbuffer[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
        flipw(&lpbuffer[i]); // for PowerPC
     }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R e l R A M
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadRelRAM(
   BT_UINT   cardnum,        // (i) card number
   BT_U16BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead)     // (i) number of word to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter

   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][BASE_PAGE]+byteOffset))[i];
      flipw(&lpbuffer[i]); // for PowerPC
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.  This function takes the byte destination address
 *              and reads n long words starting at that address, filing the host
 *              buffer
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) BYTE offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of WORDS to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i];
      fliplong(&lpbuffer[i]);
   }
}

void v6ReadRAM32S(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) BYTE offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of WORDS to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i];
      flip(&lpbuffer[i]);
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d S h a r e d M e m o r y
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of shared memory into a caller
 *              supplied buffer.  This function takes the byte destination address
 *              and reads n long words starting at that address, filing the host
 *              buffer
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadSharedMemory(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) BYTE offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of WORDS to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][SHR_MEM_PAGE]+byteOffset))[i];
      fliplong(&lpbuffer[i]);
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 5 R e a d R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.  This function takes the byte destination address
 *              and reads n long words starting at that address, filing the host
 *              buffer
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v5ReadRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) BYTE offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of WORDS to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
#ifdef WORD_SWAP
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
   }
   for( i = 0; i < wordsToRead; i++ )
   {
      flipw(&lpbuffer[i]);
   }
#else //WORD_SWAP
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = flips(((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i]);
   }
#endif //WORD_SWAP

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d R e l R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.  This function takes the byte destination address
 *              and reads n long words starting at that address, filing the host
 *              buffer
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadRelRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) BYTE offset within adapter memory (source)
   BT_UINT   wordsToRead)   // (i) number of WORDS to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   for ( i = 0; i < wordsToRead; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][BASE_PAGE]+byteOffset))[i];
      fliplong(&lpbuffer[i]);
   }

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v 6 R e a d I n t Q u e u e
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of interrupt queue into a caller
 *              supplied buffer.  This function takes the byte destination address
 *              and reads a int queue record filing the host
 *              buffer
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void v6ReadIntQueue(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset)    // (i) BYTE offset within adapter memory (source)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;

   for ( i = 0; i < 2; i++ )
   {
      lpbuffer[i] = ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE] + byteOffset))[i];
      fliplong(&lpbuffer[i]);
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e R A M
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteRAM(
   BT_UINT   cardnum,      // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U16BIT temp;
      
   for ( i = 0; i < wordToWrite; i++ )
   {
      temp = flipws(lpbuffer[i]);
      ((BT_U16BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i] = temp;
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 5 W r i t e R A M
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v5WriteRAM(
   BT_UINT   cardnum,      // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variablesr
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i,bytesToWrite;
      
   if ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2)) 
   {
      bytesToWrite = wordToWrite*4;
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,(LPSTR)lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else
   {
      for ( i = 0; i < wordToWrite; i++ )
      {
         ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flipws(lpbuffer[i]);
      }
   }
}

 /*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e S h a r e d M e m o r y
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteSharedMemory(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U32BIT temp;

   for ( i = 0; i < wordsToWrite; i++ )
   {
      temp = fliplongs(lpbuffer[i]);      
      ((BT_U32BIT *)(bt_PageAddr[cardnum][SHR_MEM_PAGE]+byteOffset))[i] = temp;
   }
}


 /*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteRAM32(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,    // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U32BIT temp;

   for ( i = 0; i < wordsToWrite; i++ )
   {
      temp =  fliplongs(lpbuffer[i]);
      ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i] = temp;
   }
}

/*===========================================================================*
* EXTERNAL ENTRY POINT:      v 6 W r i t e R A M 3 2
*===========================================================================*
*
* FUNCTION:    Write data from caller supplied buffer into adapter memory.
*
* DESCRIPTION:
*
*      It will return:
*              nothing
*===========================================================================*/

void v6WriteRAM32S(
  BT_UINT   cardnum,      // (i) card number
  BT_U32BIT *lpbuffer,    // (i) host buffer (source)
  BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
  BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
  /*******************************************************************
  *  Local variables
  *******************************************************************/
  BT_UINT    i;
  BT_U32BIT temp;

  for ( i = 0; i < wordsToWrite; i++ )
  {
     temp =  flips(lpbuffer[i]);
     ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+byteOffset))[i] = temp;
  }
}

 /*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 5 W r i t e R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v5WriteRAM32(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   for ( i = 0; i < wordsToWrite; i++ )
   {
      ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = fliplongs(lpbuffer[i]);
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e R e l R A M
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteRelRAM(
   BT_UINT   cardnum,      // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U16BIT temp;
      
   for ( i = 0; i < wordToWrite; i++ )
   {
      temp = flipws(lpbuffer[i]);
      ((BT_U16BIT *)(bt_PageAddr[cardnum][BASE_PAGE]+byteOffset))[i] = temp;
   }
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 W r i t e R e l R A M 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void v6WriteRelRAM32(
   BT_UINT   cardnum,      // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;
   BT_U32BIT temp;

   for ( i = 0; i < wordsToWrite; i++ )
   {
      temp = fliplongs(lpbuffer[i]);
      ((BT_U32BIT *)(bt_PageAddr[cardnum][BASE_PAGE]+byteOffset))[i] = temp; 
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t I q H e a d P t r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a IQ Head pointer.
 *
 *      It will return:
 *              Value of the IO Head pointer.
 *===========================================================================*/

BT_U32BIT v6GetIqHeadPtr(
   BT_UINT cardnum)       // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval  = *((BT_U32BIT *)(bt_PageAddr[cardnum][HWREG_PAGE]) + HWREG_IQ_HEAD_PTR);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t B M H e a d P t r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a IQ Head pointer.
 *
 *      It will return:
 *              Value of the IO Head pointer.
 *===========================================================================*/

BT_U32BIT v6GetBMHeadPtr(
   BT_UINT cardnum)       // (i) card number (0 based)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval  = *((BT_U32BIT *)(bt_PageAddr[cardnum][HWREG_PAGE]) + HWREG_BM_HEAD_PTR);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Read HW registers
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v6GetRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval = *((BT_U32BIT *)(bt_PageAddr[cardnum][HWREG_PAGE])+regnum);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t R e g i s t e r 1 6
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register. V5 Only
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetRegister16(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Dispatch on card and access type.
   if ((CurrentCardType[cardnum] == PCCD1553) && (regnum >= 0x80))
   {
      // Paged access board, setup and read two bytes from page.
      vbtPageDataCopy(cardnum,(LPSTR)&regval,regnum*2,2,0);
      return regval;
   }
   else 
   {

      // Non-paged access.  Map to proper address offset and read register.
      if ( regnum < 0x20 )
      {  // Hardware register is a WORD address.
         regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      else if ( regnum < 0x80 )  // WORD offset indicates RAM Reg?
      {  // RAM/File Register is a WORD address.
         regval =*((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      else
      {  // RAM offset is a WORD address.
         regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][0])+regnum);
         flipw(&regval); // for PMC on PowerPC
      }
      return regval;
   }
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v5SetRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   // Dispatch on card and access type.
   if ((CurrentCardType[cardnum] == PCCD1553) && (regnum >= 0x80))
   {
      // Paged access board, setup and write two bytes from page.
      vbtPageDataCopy(cardnum,(LPSTR)&regval,regnum*2,2,1);
   }
   else
   {
      // Non-paged access.  Map to proper address offset and write register.
      if ( regnum < HWREG_COUNT )
      {  // Hardware register is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum) = flipws((BT_U16BIT)regval);
         hwreg_value[cardnum][regnum] = (BT_U16BIT)regval;
      }
      else  if ( regnum < 0x80 )  // WORD offset indicates RAM Reg?
      {  // RAM(File) Register is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum) = flipws((BT_U16BIT)regval);
      }
      else
      {  // RAM offset is a WORD address.
         *((BT_U16BIT *)(bt_PageAddr[cardnum][0])+regnum) = flipws((BT_U16BIT)regval);
      }
   }
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v 6 S e t R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Update HW registers Registers.
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v6SetRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U32BIT *)(bt_PageAddr[cardnum][HWREG_PAGE])+regnum) = fliplongs(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t C S C R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Read HW registers
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v6GetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval  = *((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE])+regnum);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}


//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t C S C R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Update HW registers Registers.
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v6SetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE])+regnum) = fliplongs(regval);
}



/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t T r i g R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Read trigger register.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v6GetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval  =*((BT_U32BIT *)(bt_PageAddr[cardnum][TRIG_PAGE])+regnum);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}


//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v 6 S e t T r i g R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Write trigger registers
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v6SetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U32BIT *)(bt_PageAddr[cardnum][TRIG_PAGE]) + regnum) = fliplongs(regval);
}



/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t T T R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v6GetTTRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval  =*((BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE])+regnum);
   fliplong(&regval); // for PMC on PowerPC

   return regval;
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v 6 S e t T T R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v6SetTTRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U32BIT *)(bt_PageAddr[cardnum][TTREG_PAGE]) + regnum) = fliplongs(regval);
}


/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v 6 S e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Sets the discrete register.
 *
 * DESCRIPTION: Set the specified discrete register to the specify value.
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v6SetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE]) + regnum) = fliplongs(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v 5 S e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Sets the discrete register.
 *
 * DESCRIPTION: Set the specified discrete register to the specify value.
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void v5SetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval)        // (i) new value
{
   *((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum) = flipws((BT_U16BIT)regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v 6 G e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Read discrete reigsters.
 *
 * DESCRIPTION: The routine read the specified discrete registers.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v6GetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT regval;

   regval = *((BT_U32BIT *)(bt_PageAddr[cardnum][CSC_REG_PAGE])+regnum);
   fliplong(&regval); // for PMC on PowerPC
   return regval;
}

/*****************************************************************************
 *  Comnpatibility functions for earlier F/W
 *****************************************************************************/

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t F i l e R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetFileRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t F i l e R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetFileRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   // Non-paged access.  Map to proper address offset and write register.
   *((BT_U16BIT *)(bt_PageAddr[cardnum][2])+regnum) = flipws(regval);
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t H W R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then read and the frame reg
 *              is restored to the saved value.  The value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U16BIT vbtGetHWRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   // Hardware register is a WORD address.
   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t S e t H W R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Sets the value of a specified HW or RAM register.
 *
 * DESCRIPTION: The current frame register is first saved, then set to page
 *              zero.  The specified register is then written and the frame reg
 *              is restored to the saved value.  The old value of the specified
 *              register is returned to the caller.  The register offset is
 *              assumed to be a WORD offset, NOT a BYTE offset...
 *
 *      It will return:
 *              Previous value of the specified register.
 *===========================================================================*/

void vbtSetHWRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U16BIT regval)        // (i) new value
{
   // Hardware register is a WORD address.
   *((BT_U16BIT *)(bt_PageAddr[cardnum][1])+regnum) = flipws(regval);
   hwreg_value[cardnum][regnum] = regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d 3 2
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT i;
  
   if((bytesToRead % 4)==0)
   {
      for ( i = 0; i < bytesToRead/4; i++ )
      {
         ((BT_U32BIT *)lpbuffer)[i] = flips(((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i]);
      }
   }
   else
   {
      channel_status[cardnum].byte_cnt_err=1;
   }   
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWrite(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i;

   // Dispatch on card and access type.
   if ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2))
   {
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else 
   {
      // Non-paged access board, write to the specified location...
      if ( byteOffset < 0x10*2 )        // Byte offset indicates HW Regs?
      {
         // Hardware register is being accessed (offset 0x400000).
         for ( i = 0; i < bytesToWrite/2; i++ )
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][1]+byteOffset))[i] =  flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }
      else if ( byteOffset < 0x80*2 )  // Byte offset indicates RAM Regs?
      {
         // RAM Register (File Register) is being accessed (offset 0x500000).
         for ( i = 0; i < bytesToWrite/2; i++ )
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][2]+byteOffset))[i] = flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }
      else if ( byteOffset <= 0x7FFFF*2 ) // Byte offset indicates Memory area.
      {
         // Non-paged access.  Map to proper address offset and write memory.
         // Code this routine using in-line assembly, to obtain max speed
         //  AND to insure that only WORD (or DWORD) writes are made from the board.
         for ( i = 0; i < bytesToWrite/2; i++ ) //Was bytesToWrite/4
         {
            ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flipws(((BT_U16BIT *)lpbuffer)[i]);
         }
      }
      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtWrite", MB_OK | MB_SYSTEMMODAL);	 
         vbtShutdown(cardnum);
         exit(0);
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter
   // Dispatch on card and access type.
   if ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2))
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else
   {
      if ( byteOffset < HWREG_COUNT*2 )
      {  // Hardware Register area.  
         for ( i = 0; i < bytesToRead/2; i++ )
         {
           ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][1]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
         }
      }
      else if ( byteOffset < 0x80*2 )
      {  // RAM(File) Register area.  Can only be read as WORDS.
         for ( i = 0; i < bytesToRead/2; i++ )
         {
            ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][2]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
         }
      }
      else if ( byteOffset < 0x80000*2 ) // Byte offset indicates Memory area.
      {  // Dual-Port Memory area.  Can only be read as WORDS.
         for ( i = 0; i < bytesToRead/2; i++ )
         {
            ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
            flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
         }
      }
      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtRead", MB_OK | MB_SYSTEMMODAL);		 
         vbtShutdown(cardnum);
         exit(0);
      }
   }
}

 /*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e 3 2
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWrite32(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   if((bytesToWrite % 4)==0)
   {
      for ( i = 0; i < bytesToWrite/4; i++ )
      {
         ((BT_U32BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = flips(((BT_U32BIT *)lpbuffer)[i]);
      }
   }
   else
   {
      channel_status[cardnum].byte_cnt_err=1;
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t D i s c r e t e
 *===========================================================================*
 *
 * FUNCTION:    Read discrete reigsters.
 *
 * DESCRIPTION: The routine read the specified discrete registers.
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v5GetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   regval  =*((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return (BT_U32BIT)regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d H I F
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of HIF memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtReadHIF(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    bytesread;          // bytes read
   BT_UINT    i;                  // Word counter

   // Dispatch on card and access type.
   if (CurrentCardType[cardnum] == PCCD1553) 
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else 
   {
      for ( i = 0; i < bytesToRead/2; i++ )
      {
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][3]+byteOffset))[i];
         flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PowerPC
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e H I F
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWriteHIF(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    byteswritten;       // bytes written
   BT_UINT    i;

   if ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2))
   {
      // Paged access board, setup and write to page(s).
      while(bytesToWrite)
      {
         byteswritten = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToWrite,1);

         lpbuffer     += byteswritten;  // Increment user buffer pointer.
         byteOffset   += byteswritten;  // Increment board address offset.
         bytesToWrite -= byteswritten;  // Decrement bytes to be written.
      }
   }
   else 
   {
      for ( i = 0; i < bytesToWrite/2; i++ )
      {
         ((BT_U16BIT *)(bt_PageAddr[cardnum][3]+byteOffset))[i] =  flipws(((BT_U16BIT *)lpbuffer)[i]);
      }
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t G e t C S C R e g i s t e r
 *===========================================================================*
 *
 * FUNCTION:    Read the value of a specified HW or RAM register.
 *
 * DESCRIPTION: Read the CSC and ACR registers
 *
 *      It will return:
 *              Value of the specified register.
 *===========================================================================*/

BT_U32BIT v5GetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum)        // (i) register number, WORD offset
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT regval;

   // Non-paged access.  Map to proper address offset and read register.
   regval  = *((BT_U16BIT *)(bt_PageAddr[cardnum][3])+regnum);
   flipw(&regval); // for PMC on PowerPC
   return (BT_U32BIT)regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d R A M 8
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of RAM memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtReadRAM8(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter

   for ( i = 0; i < bytesToRead; i++ )
   {
      lpbuffer[i] = ((char *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
   }
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t W r i t e R A M 8
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void vbtWriteRAM8(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite) // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;

   for ( i = 0; i < bytesToWrite; i++ ) //Was bytesToWrite/4
   {
      ((char *)(bt_PageAddr[cardnum][0]+byteOffset))[i] = lpbuffer[i];
   }
}

//
/*===========================================================================*
 * EXTERNAL ENTRY POINT:      v b t R e a d M o d i f y W r i t e
 *===========================================================================*
 *
 * FUNCTION:    Read and update a single word in adapter memory.
 *              This routine reads and restores the contents of the
 *              frame mapping register, so it is interrupt safe...
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              BTD_OK            if successful.
 *              BTD_ERR_PARAM     if board number too large.
 *              BTD_ERR_NOTSETUP  if board has not been setup.
 *===========================================================================*/

BT_U16BIT vbtReadModifyWrite(
   BT_UINT   cardnum,       // (i) card number
   BT_UINT   region,        // (i) HWREG,FILEREG, or RAM
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_U16BIT wNewWord,      // (i) new value to be written under mask
   BT_U16BIT wNewWordMask)  // (i) mask for new value
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT  regval;
   LPSTR      lppage = NULL;
   BT_U32BIT  pagebytes;      // Can be up to the full 256 Kb
   BT_U16BIT  wCurrentFrame;
   BT_U16BIT  framenum;

   // Dispatch on card and access type.
   if ((CurrentCardType[cardnum] == PCCD1553) && (byteOffset >= 0x80*2))
   {
      /****************************************************************
      * Paged access board, setup and read/modify/write to page.
      * Given a byte offset within adapter memory, setup the page and
      *  the pointer within the page, and the number of bytes
      *  which remain within the page, following the returned pointer
      *  (returned in "pagebytes")
      ****************************************************************/
      lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

      // Save current frame, set frame to value returned by GetPagePtr.
      wCurrentFrame = vbtAcquireFrame(cardnum, framenum);

      regval  = *(BT_U16BIT *)(lppage);

      *(BT_U16BIT *)(lppage) = (BT_U16BIT)((regval & ~wNewWordMask) |
                                           (wNewWord & wNewWordMask));

      // Restore the frame register to the value we saved.
      vbtReleaseFrame(cardnum, wCurrentFrame);
   }
   else 
   {
      // Non-paged access board, read/modify/write specified location...
      if ( region == HWREG)        // Byte offset indicates HW Reg?
      {
         // Hardware register is being accessed (offset 0x400000).
         lppage = bt_PageAddr[cardnum][1] + byteOffset;
      }
      else if ( region == FILEREG )  // Byte offset indicates RAM Reg?
      {
         // RAM Register (File Register) is being accessed (offset 0x500000).
         lppage = bt_PageAddr[cardnum][2] + byteOffset;
      }
      else if ( region == RAM ) // Byte offset indicates Memory area.
      {
         // Memory is being accessed (offset 0x200000).
         lppage = bt_PageAddr[cardnum][0] + byteOffset;
      }
      else
      {		  
         debugMessageBox(NULL, "Address out of bounds", "vbtReadModifyWrite", MB_OK | MB_SYSTEMMODAL);		 
         vbtShutdown(cardnum);
         exit(0);
      }
      // Now read/modify/write the proper location.
      regval  = *(BT_U16BIT *)(lppage);
      flipw(&regval); //for PMC on the PowerPC
      regval = (BT_U16BIT)((regval & ~wNewWordMask) | (wNewWord & wNewWordMask));

      flipw(&regval); //for PMC on the PowerPC
      *(BT_U16BIT *)(lppage) = regval;
      flipw(&regval);
   }
   return regval;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:         v b t R e a d_iq
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of adapter memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void vbtRead_iq(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead)   // (i) number of bytes to copy
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    i;                  // Word counter
   BT_UINT    bytesread;          // bytes read

  // Dispatch on card and access type.
   if (CurrentCardType[cardnum] == PCCD1553)
   {
      // Paged access board, setup and read from page(s).
      do
      {
         bytesread = vbtPageDataCopy(cardnum,lpbuffer,byteOffset,bytesToRead,0);

         lpbuffer     += bytesread;     // Increment user buffer pointer.
         byteOffset   += bytesread;     // Increment board address offset.
         bytesToRead  -= bytesread;     // Decrement bytes to be read.
      }
      while( bytesToRead );
   }
   else
   {  
      for ( i = 0; i < bytesToRead/2; i++ )
      {
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)(bt_PageAddr[cardnum][0]+byteOffset))[i];
         flipw(&(((BT_U16BIT *)lpbuffer)[i])); // for PMC on PowerPC
      }
   }
}

/*===========================================================================*
 * EXTERNAL API ENTRY POINT:       v b t G e t P a g e P t r
 *===========================================================================*
 *
 * FUNCTION:    Given adapter offset, switches to correct DP memory page and
 *              returns a pointer to the specified offset within the page.
 *
 * DESCRIPTION: This function is intended to be used by the
 *              BM_MessageConvert() helper function ONLY.
 *              If the requested number of bytes is not present in the
 *              specified page, then the requested number of bytes are
 *              copied from the board (in two steps, of course) into a
 *              caller's buffer, and the address of this buffer is returned.
 *
 *      It will return:
 *              Page pointer.
 *===========================================================================*/
LPSTR vbtGetPagePtr(
   BT_UINT   cardnum,           // (i) card number.
   BT_U32BIT byteOffset,        // (i) offset within adapter memory.
   BT_UINT   bytesneeded,       // (i) number of bytes needed in page.
   LPSTR     local_board_buffer)   // (io) scratch buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT     pagebytes;     // Can be up to the full 1 MByte
   LPSTR         lppage;
   BT_U16BIT     framenum;

   // Dispatch on card type.  Access is always to dual port RAM!
   if (CurrentCardType[cardnum] == PCCD1553) 
   {
      /****************************************************************
      * Adaptor requires paged access.
      * Given a byte offset within adapter memory, setup the page and
      * the pointer within the page, and the number of bytes
      * which remain within the page, following the returned pointer
      * (returned in "pagebytes")
      ****************************************************************/
      lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

      /****************************************************************
      * If the current page does not contain all of the needed data, read
      *  the data from the board into our local buffer, and return the
      *  pointer to the local buffer.  We never check for a request for
      *  more data than will fit into the caller's buffer...
      * Certain PLATFORM_USER's cannot properly read 32-bit quantities.
      *  We just do the 16-bit copy for all such platforms, rather than
      *  try to keep track of which work and which don't work.
      ****************************************************************/

      if ( (bytesneeded > pagebytes) || (CurrentPlatform[cardnum] == PLATFORM_USER) )
      {
         vbtRead(cardnum, local_board_buffer, byteOffset, bytesneeded);
         return local_board_buffer;
      }

      /****************************************************************
      * Requested number of bytes does not cross a page boundry.  Setup
      *  the frame register, and return a pointer to the board.  Assume
      *  that the frame register has been locked by the caller!
      ****************************************************************/
      vbtSetFrame(cardnum,framenum);
      return lppage;
   }
   else
   {

#if !defined(NON_INTEL_WORD_ORDER) && !defined(INCLUDE_VME_VXI_1553)
      return (bt_PageAddr[cardnum][0]+byteOffset);
#else
      if(board_access_32[cardnum])
         vbtRead32(cardnum, local_board_buffer, byteOffset, bytesneeded);
      else
         vbtRead(cardnum, local_board_buffer, byteOffset, bytesneeded);
      return local_board_buffer;
#endif
   }
}

//
/*===========================================================================*
 * LOCAL ENTRY POINT:      v b t P a g e D a t a C o p y
 *===========================================================================*
 *
 * FUNCTION:    Copies data to or from a single page, up to number of bytes
 *              requested or to the end of the page.
 *
 *      It will return:
 *              Number of bytes read.
 *===========================================================================*/

BT_UINT vbtPageDataCopy(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer to copy data to
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory
   BT_UINT   bytesToRead,   // (i) number of bytes to copy
   int       direction)     // (i) 0=read from board, 1=write to board
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   LPSTR      lppage;
   BT_U32BIT  pagebytes;  // Can be up to the full 256 Kb
   BT_UINT    bytestocopy;
   BT_U16BIT  framenum;
   BT_U16BIT  wCurrentFrame;
   BT_UINT i;

   /*******************************************************************
   * Given a byte offset within adapter memory, setup the page and
   *  the pointer within the page, and the number of bytes
   *  which remain within the page, following the returned pointer
   *  (returned in "pagebytes")
   *******************************************************************/
   lppage = vbtMapPage(cardnum,byteOffset,&pagebytes,&framenum);

   if ( bytesToRead > pagebytes )
      bytestocopy = (BT_UINT)pagebytes; 
   else
      bytestocopy = bytesToRead;

   // Save current frame, set frame to value returned by GetPagePtr.
   wCurrentFrame = vbtAcquireFrame(cardnum, framenum);

   // Code this routine using in-line assembly, to obtain max speed
   //  AND to insure that only WORD (or DWORD) reads are made from the board.

   if ( direction )
   {
      for ( i = 0; i < bytestocopy/2; i++ )
         ((BT_U16BIT *)lppage)[i] = ((BT_U16BIT *)lpbuffer)[i];
   }
   else
   {
      for ( i = 0; i < bytestocopy/2; i++ )
         ((BT_U16BIT *)lpbuffer)[i] = ((BT_U16BIT *)lppage)[i];
   }
   // Release and restore the frame register to the value we saved.
   vbtReleaseFrame(cardnum, wCurrentFrame);
   return(bytestocopy);
}

/*===========================================================================*
 * LOCAL ENTRY POINT:            v b t M a p P a g e
 *===========================================================================*
 *
 * FUNCTION:    This function is used to setup for access to dual-port memory.
 *
 * DESCRIPTION: Given an adapter offset, calculates correct page and returns
 *              a pointer to specified offset in the page.  Pages can be up
 *              to 1024 Kb long, thus requiring 32 bit arithmetic.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              byte pointer to specified element in page as mapped by
 *              "framereg".  This pointer is valid for "pagebytes".
 *===========================================================================*/
LPSTR vbtMapPage(
   BT_UINT    cardnum,       // (i) card number
   BT_U32BIT  offset,        // (i) byte address within adapter memory
   BT_U32BIT *pagebytes,     // (o) number of bytes remaining in page
   BT_U16BIT *framereg)      // (o) value of frame register which maps page
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   LPSTR      lppage;        // Pointer to the requested byte address.
   BT_U32BIT  rawframe;      // Raw frame number (high bits of the offset).
   int        ptrnum;        // Index of the pointer which maps this page.
   BT_U32BIT  offsetinpage;  // Offset of requested byte address in page.

   /*************************************************************************
   *  Given a byte offset within adapter memory, setup the page, the
   *   pointer within the page, and the number of bytes which remain
   *   within the page, following the returned pointer. Note that the
   *   number of bytes remaining in the page could be as much as 1024K!
   * We convert a user-specified byte offset "offset" from the beginning of
   *  the board into the following parameters:
   *
   *  bt_FrameReg[] - The actual value needed to program the frame register.
   *  ptrnum        - Index into bt_pageaddr[cardnum][ptrnum].
   *  offsetinpage  - Value to add to bt_pageaddr[cardnum][ptrnum] to get the
   *                  actual host pointer.
   *  pagebytes     - Number of bytes in page that can be accessed via ptr.
   *  lppage        - Host pointer to specified offset within BusTools board.
   *
   * This conversion is performed as follows:
   *************************************************************************/
   // The lower bits of the offset map the location in the page.
   offsetinpage = offset & bt_OffsetMask[cardnum];
   // Compute the number of bytes remaining in the page.
   *pagebytes   = bt_OffsetMask[cardnum] - offsetinpage + 1;
   // Extract the upper bits of the offset; they are the page/frame number.
   rawframe     = offset & ~bt_OffsetMask[cardnum];
   // Shift the page/frame number to match the frame register.
   *framereg    = (BT_U16BIT)(rawframe >> bt_FrameShift[cardnum]);
   // Take the page/frame number and compute the bt_PageAddr[] index.
   ptrnum       = (int)(rawframe >> bt_PtrShift[cardnum]);
   // The host pointer is the base address of the frame plus the offset
   //   into the frame.
   lppage       = bt_PageAddr[cardnum][ptrnum] + (BT_UINT)offsetinpage;

   /*************************************************************************
   * Where: 
   *
   *  bt_OffsetMask[cardnum] - Mask that extracts page offset from board offset
   *  bt_FrameShift[cardnum] - Value to shift raw frame to get actual frame reg
   *  bt_PtrShift[cardnum]   - Value to shift raw frame to get pointer index.
   *************************************************************************/
  return lppage;
}

/*===========================================================================*
 * LOCAL ENTRY POINT:            v b t S e t F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function sets the BusTools board frame register to the
 *              specified value.  This function should only be called directly
 *              during initialization, since it does not acquire or release
 *              ownership of the Frame Register.
 *
 * DESCRIPTION: The frame register of the PCCD-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
void vbtSetFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)
{
   if ( frame == bt_FrameReg[cardnum] )
      return;                            // Frame register already set properly.

   bt_FrameReg[cardnum] = frame;         // Remember the setting.

   if ( bt_inuse[cardnum] < 0 )
       return;      // Software versions of the driver have no frame register.

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   // Define the code needed;  The code varies depending on what      +
   //  type of board/carrier/environment we are using.                +
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   if (CurrentCardType[cardnum] == PCCD1553) 
   {   // Q104-1553 ISA.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_2 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_3 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2) = frame;
       else if( CurrentCardSlot[cardnum] == CHANNEL_4 )
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2) = frame;
   }
   return;
}


/*===========================================================================*
 * LOCAL ENTRY POINT:        v b t A c q u i r e F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function first acquires control of the frame, then it
 *              reads the frame register and saves it, then sets it to the
 *              value supplied by the caller.  The value read is returned
 *              to the caller.
 *              This function should be called in pairs with vbtReleaseFrame()
 *              to first acquire then release the ownership of the frame
 *              register.
 *
 * DESCRIPTION: The frame register of the PC-/ISA/PCC/PCCD-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
BT_U16BIT vbtAcquireFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)         // (i) actual value to load into frame register
{
   BT_U16BIT  ret_frame=0;    // Value of the frame register, returned to caller.

   /*******************************************************************
   *  Define the code needed:  the code varies depending on what
   *   type of board/carrier and environment we are using.
   *******************************************************************/

   if ( bt_inuse[cardnum] < 0 )
      return 0;      // Software versions of the driver have no frame register.

   CEI_MUTEX_LOCK(&hFrameCritSect[cardnum]);

   if ( frame == bt_FrameReg[cardnum] )
      return frame;                      // Frame register already set properly.

   bt_FrameReg[cardnum] = frame;         // Remember the new setting.

   // Dispatch to the proper code based on the board type.
   if (CurrentCardType[cardnum] == PCCD1553)  // Single- or multi-function
   {   // Q104-1553 ISA.  The fourth page pointer maps the Hardware Interface Regs.
       // The frame register is one of these registers.
       if ( CurrentCardSlot[cardnum] == CHANNEL_1 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_1*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_2 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_2*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_3 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_3*2) = frame;
       }
       else if ( CurrentCardSlot[cardnum] == CHANNEL_4 )
       {
          ret_frame = *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2);
          *(BT_U16BIT *)(bt_PageAddr[cardnum][3]+HIR_QPAGE_4*2) = frame;
       }
   }

   return (BT_U16BIT)(ret_frame & bt_FrameMask[cardnum]);
}

/*===========================================================================*
 * LOCAL ENTRY POINT:       v b t R e l e a s e F r a m e
 *===========================================================================*
 *
 * FUNCTION:    This function reloads the BusTools board frame register with
 *              the specified value, then releases it.  This function should
 *              be called in pairs with vbtAcquireFrame() to first acquire then
 *              release the ownership of the frame register...
 *
 * DESCRIPTION: The frame register of the PCCD-1553 is used to
 *              select the page within dual-port memory to be read or written
 *              by the host.
 *              Since there is only one frame register, access to it must be
 *              interlocked so that a thread or interrupt cannot modify the
 *              register while another thread or interrupt function is still
 *              accessing the board.
 *
 *              Note that this function is very time critical.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *              BTD_ERR_BADWCS if error reading back the WCS
 *===========================================================================*/
void vbtReleaseFrame(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT frame)
{
   if ( bt_inuse[cardnum] < 0 )
      return;      // Software versions of the driver have no frame register.

   CEI_MUTEX_UNLOCK(&hFrameCritSect[cardnum]);   // Leave Critical Section.

   return;
}

#ifdef INCLUDE_USB_SUPPORT
BT_INT  USB_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT      cnt=0;                  // Loop index

   ptrMbuf[cardnum] = (BT_U32BIT *)CEI_MALLOC(nBM_MBUF_len[cardnum] * mbuf_count);
   if(ptrMbuf[cardnum] == NULL)
      return -1;

   memset(ptrMbuf[cardnum],0,nBM_MBUF_len[cardnum] * mbuf_count);
  
   ptrMbuf[cardnum][0] = 0xbeef0000; //inialize to BM header
   bt_PageAddr[cardnum][BM_SIM_PAGE] = (LPSTR)ptrMbuf[cardnum];
  
   return API_SUCCESS;
}

#endif //INCLUDE_USB_SUPPORT 
