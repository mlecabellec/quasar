/*============================================================================*
 * FILE:                          B C . C
 *============================================================================*
 *
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
 *             This file contains the API routines for BC operation of
 *             the BusTools board, and routines which are specific to the
 *             One Shot Bus Controller Mode.
 *             These functions assume that the BusTools_API_Init*() function
 *             has been successfully called.
 *
 * USER ENTRY POINTS: 
 *    BusTools_BC_AperiodicRun          - Executes a list of aperiodic messages
 *    BusTools_BC_AperiodicTest         - Tests if aperiodic message list is complete
 *    BusTools_BC_AutoIncrMessageData   - Increments message data
 *    BusTools_BC_ControlWordRead       - Reads the control word from the BC message Buffer 
 *    BusTools_BC_ControlWordUpdate     - Updates certain control word parameters
 *    BusTools_BC_Checksum1760          - Calculates 1760 checksum on a BC message buffer
 *    BusTools_BC_DataBufferUpdate      - Updates the specified data using data buffer address **
 *    BusTools_BC_DataBufferWrite       - Updates specified BC Message Block data area buffer
 *    BusTools_BC_GetBufferCount        - Returns the number of buffer allocated for a BC Message**
 *    BusTools_BC_Init                  - Sets the BC init data in the RAM registers
 *    BusTools_BC_ReadLastMessage       - Reads the last message in interrupt queue
 *    BusTools_BC_ReadLastMessageBlock  - Reads the last group of messages in the interrupt queue
 *    BusTools_BC_ReadNextMessage       - Reads the next message in the interrupt queue.
 *    BusTools_BC_RetryInit             - Sets up multiple hardware retries
 *    BusTools_BC_IsRunning             - Checks if BC is running.
 *    BusTools_BC_IsRunning2            - Checks if BC is running.  Can be embedded into a conditional
 *    BusTools_BC_MessageAlloc          - Allocates memory on board for BC msg buffers.
 *    BusTools_BC_MessageBlockAlloc     - Allocates memory for a BC Message Buffer**
 *    BusTools_BC_MessageGetaddr        - Converts BC message id to an address
 *    BusTools_BC_MessageGetid          - Converts BT1553 address to a BC message id
 *    BusTools_BC_MessageNoop           - Toggles a BC message between message & NOOP
 *    BusTools_BC_MessageRead           - Reads the specified BC Message Block
 *    BusTools_BC_MessageBufferRead     - Reads a BC message buffer using data buffer addr**
 *    BusTools_BC_MessageReadData       - Reads the data only from a specified BC message
 *    BusTools_BC_MessageReadDataBuffer - Reads the data only from a specified BC message buffer
 *    BusTools_BC_MessageWrite          - Write the specified BC Message Block
 *    BusTools_BC_MessageUpdate         - Update specified BC Message Block data area
 *    BusTools_BC_ReadDataBuffer        - Reads data from a specified buffer address**
 *    BusTools_BC_MessageUpdateBuffer   - Updates specified BC Message Buffer data**
 *    BusTools_BC_SelectBufferRead      - Reads data from a selected message data buffer**
 *    BusTools_BC_SelectBufferUpdate    - Updates the Data in the selected data buffer for a message** 
 *    BusTools_BC_SetFrameRate          - Dynamically updates the frame rate.
 *    BusTools_BC_Start                 - Start BC at specified message number.
 *    BusTools_BC_StartStop             - Turns the BC on or off.
 *    BusTools_BC_Trigger               - Sets up the BC external trigger mode.
 *                                        ** functions only available with v6 firmware
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    V6DumpBC                     - Helper function for BusTools_DumpMemory()
 * 
 * INTERNAL ROUTINES:
 *    BC_MBLOCK_ADDR()           - Compute BC message address.
 *
 *===========================================================================*/

/* $Revision:  8.28 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  04/22/1999   Changed StartStop() to stop if BC_RUN bit is clear, or if BC_BUSY
               bit is clear.V3.05.ajh
  04/26/1999   Fixed error in BC_MessageWrite() and friends which would cause
               BC message blocks to become corrupted when switched between msg
               and branchs.V3.05.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  01/27/2000   Added offset IM_GAP_ADJ to the PCI-/ISA-/IP-1553 gap time.V4.01.1jh
  04/13/2000   Changed MessageWrite to correctly limit the gap time.V4.02.ajh
  06/19/2000   Changed BC_Start to return API_SINGLE_FUNCTION_ERR if either the
               RT or the BM is running, and _HW_1Function[cardnum] is set.
               Swapped order of RT-RT broadcast words from MessageRead.V4.05.ajh
  06/22/2000   Added support for the User Interface DLL.V4.06.ajh
  10/04/2000   Changed BC_MessageWrite to support short gap times.V4.16.ajh
  10/23/2000   Removed change BC_MessageWrite to support short gap times.V4.18.ajh
  11/08/2000   Changed BusTools_BC_Init() to support longer time-out and late
               responses.V4.21.ajh
  12/04/2000   Changed BC_IsRunning() to set/clear bc_running[cardnum] based on
               the current value of the hardware BC_RUN bit.  Changed BC_Start
               to report single function warning after starting BC.V4.26.ajh
  04/25/2001   Modified message read and write to support counted conditional
               messages, also the LabView wrappers.V4.39.ajh
  05/18/2001   Added code to allow multiple hardware retries 4.40 rhc
  02/15/2002   Add interrupt queue processing functions. v4.48
  01/27/2003   Fix LabView Word swap problem in BusTools_BC_OneShotExecuteLV.
  01/27/2003   Fix Signed/Unsigned mismatch
  09/18/2003   Move Labview functions to LabView.c
  10/01/2003   32-Bit API
  08/02/2004   Add 3 new functions, BusTools_BC_DataBufferWrite, 
               BusTools_BC_MessageReadDataBuffer, and BusTools_BC_ControlWordUpdate
  12/07/2006   Improve timing in BusTools_BC_OneShotExecute.
  12/12/2006   Improve BusTools_BC_MessageRead.
  11/19/2007   Change BusTools_BC_Init to add the fixed time flag in place of bPrimeA.
  03/05/2009   Add message scheduling and timed noop features.
  06/10/2009   Add BC_CONTROL_HALT to BusTools_BC_MessageWrite
  10/22/2009   Extended timing, Frame Start timing channel sharing
  12/03/2010   Fix BusTools_BC_MessageRead to return all buffer data  
  12/07/2010   Fix Timed noop check.
  04/13/2011   Allow memory dumps to screen for processors with file systems
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API  
  02/22/2013   Fixed problems with BusTools_BC_ReadlastMessageBlock for v6  
  11/05/2014   Fix Rep_rate and status flips
  11/12/2015   Modified v6_BC_Start to clear aperiodic registers. Modified
               v5_BC_ReadLastMessage and v6_BC_ReadLastMessage .bch
  03/12/2017   Fixed AddTrace() input error
   
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

BT_INT _stdcall bc_auto_incr(BT_UINT,struct api_int_fifo *);
BT_INT _stdcall bc_auto_multi_incr(BT_UINT,struct api_int_fifo *);
static API_INT_FIFO *pIntFIFO[MAX_BTA][512];     // Thread FIFO structure pointer
static BT_INT bit_running[MAX_BTA];

/****************************************************************************
*  BC control information, local to this module.
****************************************************************************/
static BT_UINT bc_num_dbufs[MAX_BTA];    /* BC data buffers per msg      */
static BT_UINT bc_size_dbuf[MAX_BTA];    /* Bytes in 1 BC Data Buffer    */
static CEI_ULONG iqptr_bc_last[MAX_BTA];

/*==========================================================================*
 * LOCAL FUNCTION:          B C _ M B L O C K _ A D D R
 *==========================================================================*
 *
 * FUNCTION:    Calculates and returns the address of a BC message block,
 *              given the zero-based index of the message block.
 *
 * DESCRIPTION: This macro uses the base address of the BC message blocks,
 *              the size of a message block, the size and number of data
 *              blocks, the size of a message block and the message index
 *              to calculate the address requested.
 *
 *      It will return:
 *              Byte address of the specified message block.
 *=========================================================================*/

static BT_U32BIT BC_MBLOCK_ADDR(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT mblock_id)
{
   BT_U32BIT  addr;          // Base address of requested BC message
   BT_U32BIT  msg_blk_size;  // Size of a BC message block, including data bufs

   msg_blk_size = sizeof(BC_MESSAGE) +
                               (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);
   addr = btmem_bc[cardnum] + ( mblock_id * msg_blk_size );
   return addr;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_AperiodicRun
*
* FUNCTION
*     If an aperiodic message list is currently being processed return
*     an error, then
*     Loads the address of the specified message(s) into either the high-
*     or the low-priority aperiodic message register, then optionally waits
*     for the message(s) to be executed.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTRUNNING       -> BC not running and wait specified
*     API_BC_NOTINITED        -> BC has not been initialized
*     API_HARDWARE_NOSUPPORT  -> Non-IP does not support priority messages
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_APERIODIC_RUNNING-> BC Aperiodics still running, cannot start new msg list
*     API_BC_APERIODIC_TIMEOUT-> Aperiodic messages did not complete in time
*
*     Common Version
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicRun(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) Number of BC message which begins list
   BT_UINT   Hipriority,    // (i)  1 -> Hi Priority msgs, 0 -> Low Priority msgs
   BT_UINT   WaitFlag,      // (i)  1 -> Wait for BC to complete executing msgs
   BT_UINT   WaitTime)      // (i)  Timeout in seconds(16-bit) or milliseconds(32-bit)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT addr;         /* Address of the first BC msg */
   BT_U16BIT addr16;       /* Address of the first BC msg (16BIT) */
   BT_UINT   high_low_reg; /* Aperiodic register address */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bc_inited[cardnum] == BC_NO_INIT)
      return API_BC_NOTINITED;

   if (!bc_running[cardnum])
      return API_BC_NOTRUNNING;

   /*******************************************************************
   *  Check parameter ranges
   *******************************************************************/
   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(board_is_v5_uca[cardnum]) 
   {
      /* Setup which register is to be used. */
      if ( Hipriority )
         high_low_reg = RAMREG_HIGHAPTR;
      else
         high_low_reg = RAMREG_LOWAPTR;

      /* If currently active, return error. */
      if ( vbtGetFileRegister(cardnum, high_low_reg) )
         return API_BC_APERIODIC_RUNNING;

      /* Address of BC message is base plus length of preceeding messages. */
      /* Convert it from bytes to words. */
      addr16 = (BT_U16BIT)(BC_MBLOCK_ADDR(cardnum, messageid)>>hw_addr_shift[cardnum]);

      /* Start the aperiodic message list processing. */
      vbtSetFileRegister(cardnum, high_low_reg, addr16);

      if ( WaitFlag == BC_NO_WAIT )
         return API_SUCCESS;

      while ( vbtGetFileRegister(cardnum, high_low_reg) )
      {
         MSDELAY(1);
         if ( WaitTime )
            WaitTime--;
         else
            return API_BC_APERIODIC_TIMEOUT;
      }
   }
   else
   {
      /* Setup which register is to be used. */
      if ( Hipriority )
         high_low_reg = HWREG_BC_HIGH_PRI_MSG;
      else
         high_low_reg = HWREG_BC_LOW_PRI_MSG;

      /* If currently active, return error. */
      if ( vbtGetRegister[cardnum](cardnum, high_low_reg) )
         return API_BC_APERIODIC_RUNNING;

      /* Address of BC message is base plus length of preceeding messages. */
      /* Convert it from bytes to words. */

      addr = RAM_ADDR(cardnum,mblock_addr[cardnum][messageid]);
      if(addr == 0)
         return API_BC_ILLEGAL_MBLOCK;

      /* Start the aperiodic message list processing. */
      vbtSetRegister[cardnum](cardnum, high_low_reg, addr);

      if ( WaitFlag == BC_NO_WAIT )
         return API_SUCCESS;

      while (vbtGetRegister[cardnum](cardnum, high_low_reg) )
      {
         MSDELAY(1);
         if ( WaitTime )
         WaitTime--;
         else
            return API_BC_APERIODIC_TIMEOUT;
      }
   }

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_AperiodicTest
*
* FUNCTION
*     Tests to see if an aperiodic message list is currently being
*     processed.
*
*   Returns
*     API_SUCCESS             -> success, aperiodics not running
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC has not been initialized
*     API_HARDWARE_NOSUPPORT  -> Non-IP does not support priority messages
*     API_BC_APERIODIC_RUNNING-> BC Aperiodics still running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_AperiodicTest(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   Hipriority)    // (i) 1 -> Hi Priority msgs, 0 -> Low Priority msgs
{

   BT_U32BIT active32;

   /***********************************************************************
   *  Check initial conditions
   ***********************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  See if message list still running.
   *******************************************************************/

   if(board_is_v5_uca[cardnum])
   {
      if ( Hipriority )
      { 
          if (vbtGetFileRegister(cardnum, RAMREG_HIGHAPTR) == 0)
          {
             if( vbtGetFileRegister(cardnum,RAMREG_HIGHAFLG) == 0)
                return API_SUCCESS;
          }
          return API_BC_APERIODIC_RUNNING;
      }
      else
      {
         if ( vbtGetFileRegister(cardnum, RAMREG_LOWAPTR) == 0 )
         {
            if( vbtGetFileRegister(cardnum,RAMREG_LOW_AFLG) == 0)
                return API_SUCCESS;
         }
         return API_BC_APERIODIC_RUNNING;
      }
   }
   else
   {
      if ( Hipriority )
      {
         if ( vbtGetRegister[cardnum](cardnum, HWREG_BC_HIGH_PRI_MSG) == 0)
         {
            active32 = vbtGetRegister[cardnum](cardnum, HWREG_BC_APER_FLAG);
            if((active32 & 0x1) == 0)
               return API_SUCCESS;   
         }
         return API_BC_APERIODIC_RUNNING;
      }
      else
      {
         if ( vbtGetRegister[cardnum](cardnum, HWREG_BC_LOW_PRI_MSG) == 0)
         {
            active32 = vbtGetRegister[cardnum](cardnum, HWREG_BC_APER_FLAG);
            if((active32 & 0x2) == 0)
               return API_SUCCESS;   
         }
         return API_BC_APERIODIC_RUNNING;
      }
   }
}

/****************************************************************************
*
* PROCEDURE NAME - v6_BC_Init
*
* FUNCTION
*     Set the BC initialization data in the RAM registers (V6 and later F/W)
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_RUNNING          -> BC currently running
*     API_BC_INITED           -> Error condition not used
*     API_BC_BADTIMEOUT1      -> No Response Time < 4 or >= 32/62
*     API_BC_BADTIMEOUT2      -> Late Response Time < 4 or >= 32/62
*     API_BC_BADFREQUENCY     -> frame > 1638375 us or frame < 1000 us
*
****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers)   // (i) number of BC message buffers ( 1 or 2 for legacy)
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U32BIT wValue;
   BT_INT status = API_SUCCESS;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((bc_options & FIXED_GAP) && (bc_options & FRAME_START_TIMING))
      return API_PARAM_CONFLICT;

   if(num_buffers > 2)
      return API_BAD_PARAM;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
   {
      vbtReadRAM32[cardnum](cardnum,&bc_inited[cardnum],pShareTable[cardnum] + SHARE_BC_INITED,1);
      if(bc_inited[cardnum])
         return API_BC_INITED;
   } 
#endif

   bc_mblock_count[cardnum] = 0;   // No BC message buffers allocated
   /*******************************************************************
   *  Check parameter ranges
   *******************************************************************/
   if ((wTimeout1 < 4) || (wTimeout1 > 31))
      return API_BC_BADTIMEOUT1;
   if ((wTimeout2 < 4) || (wTimeout2 > 30))
      return API_BC_BADTIMEOUT2;

   /*******************************************************************
   *  BC is either single or double buffered.  Record
   *  the size of a buffer, and the number of buffers.
   *******************************************************************/
   if ( bc_running[cardnum] == 0 )
   {
      bc_size_dbuf[cardnum] = 2 * BT1553_BUFCOUNT;  /* 34 words per buffer */
      if(bc_options & MULTIPLE_BC_BUFFERS)
      {
         channel_using_multiple_bc_buffers[cardnum] = 1;
         bc_num_dbufs[cardnum] = 0;               // Set this to zero since it is not used
      }
      else
      {
         channel_using_multiple_bc_buffers[cardnum] = 0;
         bc_num_dbufs[cardnum] = num_buffers;  /* Single buffered. */
      }
   }

   /*******************************************************************
   *  BC interrupt enables
   *******************************************************************/
   vbtSetRegister[cardnum](cardnum,HWREG_BC_INT_ENABLE,Enable);

   /* Set or Reset the interrupt enable on minor-framne overflow */
   wValue = vbtGetRegister[cardnum](cardnum, HWREG_FW_CNTRL);
   if(bc_options & MFOVFL_INT_ENA)
   {
      wValue |= FW_MINORFRAME_OFLOW_ENA;
   }
   else
   {
      wValue &= ~FW_MINORFRAME_OFLOW_ENA;
   }
   vbtSetRegister[cardnum](cardnum,HWREG_FW_CNTRL,wValue);

   // Clear the aperiodic registers
   vbtSetRegister[cardnum](cardnum, HWREG_BCMSG_PTR, 0x0);
   vbtSetRegister[cardnum](cardnum, HWREG_BC_LOW_PRI_MSG, 0x0);  
   vbtSetRegister[cardnum](cardnum, HWREG_BC_HIGH_PRI_MSG, 0x0);
   vbtSetRegister[cardnum](cardnum, HWREG_BC_RETRY, 0);
   vbtSetRegister[cardnum](cardnum, HWREG_DQ_RETRY, 0); 
   // check for the retry method to use based on card type

   /******************************************************************
	* use the retry buffer to allow up to 16 retries
	******************************************************************/
   wValue = 0x0;
   vbtSetRegister[cardnum](cardnum,HWREG_BC_RETRY_CTL,wValue);   //clear retries
   if(wRetry) // If there are any retry options selected set up a single retry
   {
      if ( wRetry & BC_RETRY_ALTB )
      {
         wValue = RETRY_ALTERNATE_BUS;
         vbtSetRegister[cardnum](cardnum,HWREG_BC_RETRY_CTL,wValue);  //Retry Alernate Bus
      }
      else
      {
         wValue = RETRY_SAME_BUS;
         vbtSetRegister[cardnum](cardnum,HWREG_BC_RETRY_CTL,wValue);      // Retry Same Bus
      }
   }

   wValue = 0x0; /* clear the 1553 status word retry bits */
   if ( wRetry & BC_RETRY_ME   )
      wValue |= BC_RETRY_ME_BIT;
   if ( wRetry & BC_RETRY_BUSY )
      wValue |= BC_RETRY_BUSY_BIT;
   if ( wRetry & BC_RETRY_TF   )
      wValue |= BC_RETRY_TF_BIT;
   if ( wRetry & BC_RETRY_SSF  )  
      wValue |= BC_RETRY_SSF_BIT;
   if ( wRetry & BC_RETRY_INSTR) 
      wValue |= BC_RETRY_INSTR_BIT;
   if ( wRetry & BC_RETRY_SRQ  )
      wValue |= BC_RETRY_SRQ_BIT;

   /* set the 1553 status retry bits */
   vbtSetRegister[cardnum](cardnum,HWREG_BC_RETRY,wValue);

   /*******************************************************************
   *  There are two registers the hold retry options
   *     HWREG_BC_RETRY contains the 1553 status bits
   *     HWREG_DQ_RETRY contains the data quality bits (including no-resp)
   *******************************************************************/
   wValue = 0x0;                         /* start with all retries off      */
   if ( wRetry & BC_RETRY_NRSP)          /* Add retry on no response        */
      wValue |= BC_RETRY_NRSP_BIT;
   if ( wRetry & BC_RETRY_INV_WRD) 
      wValue |= BC_RETRY_INV_WRD_BIT;
   if ( wRetry & BC_RETRY_INV_SYNC)
      wValue |= BC_RETRY_INV_SYNC_BIT;
   if ( wRetry & BC_RETRY_MID_BIT)  
      wValue |= BC_RETRY_MID_BIT_BIT;
   if ( wRetry & BC_RETRY_TWO_BUS) 
      wValue |= BC_RETRY_TWO_BUS_BIT;
   if ( wRetry & BC_RETRY_PARITY)
      wValue |= BC_RETRY_PARITY_BIT;
   if ( wRetry & BC_RETRY_CONT_DATA)
      wValue |= BC_RETRY_CONT_DATA_BIT;
   if ( wRetry & BC_RETRY_EARLY_RSP)
      wValue |= BC_RETRY_EARLY_RSP_BIT;
   if ( wRetry & BC_RETRY_LATE_RSP)
      wValue |= BC_RETRY_LATE_RSP_BIT;
   if ( wRetry & BC_RETRY_BAD_ADDR)
      wValue |= BC_RETRY_BAD_ADDR_BIT;    
   if ( wRetry & BC_RETRY_WRONG_BUS)
      wValue |= BC_RETRY_WRONG_BUS_BIT;
   if ( wRetry & BC_RETRY_NO_GAP)
      wValue |= BC_RETRY_NO_GAP_BIT;
   /* set the data quallity retry bits */
   vbtSetRegister[cardnum](cardnum,HWREG_DQ_RETRY,wValue);

   /*******************************************************************
   *  Store response register.  Round it up by 1/2 microsecond.V4.01.ajh
   *******************************************************************/
   wTimeout1 = (wTimeout1 << 1) | 1;
   wTimeout2 = (wTimeout2 << 1) | 1;
   wValue = (BT_U32BIT)(((wTimeout1 & 0x3f) << 8) + (wTimeout2 & 0x3f));

   wResponseReg4[cardnum] = wValue;  /* Save value for later.*/
   vbtSetRegister[cardnum](cardnum,HWREG_V6RESPONSE,wValue);

   /*******************************************************************
   *  Store minor frame register -- frame time is in
   *   units of 1 or 25 micro-seconds
   *******************************************************************/

   if (frame < FRAME_TIME_LOWER_LIMIT)
   {
      frame = FRAME_TIME_LOWER_LIMIT;
      status = API_BC_BADFREQUENCY;
   }
   vbtSetRegister[cardnum](cardnum, HWREG_MINOR_FRAMEV6, frame);
   frametime32[cardnum] = frame;

   iqptr_bc_last[cardnum] = REL_ADDR(cardnum,vbtGetRegister[cardnum](cardnum,HWREG_IQ_BUF_START));

   /*******************************************************************
   *  gap timing options fix or relative
   *******************************************************************/
   if(bc_options < 0xf)
      bc_options = 0;

   wValue = CR1_FIXED_GAP;
   if(bc_options & FIXED_GAP)// use fixed timing from start of message
   {
      api_sethwcbits(cardnum,wValue);
   }
   else // use relative timing from end of message.
   {
      api_clearhwcbits(cardnum,wValue);
   }

   wValue = CR1_MSG_SCHD;
   if(bc_options & MSG_SCHD)   
   {
      if(num_buffers == 2)
         return API_PARAM_CONFLICT;

      api_sethwcbits(cardnum,wValue);
      board_using_msg_schd[cardnum]=1;
   }
   else // no message scheduling.
   {
      api_clearhwcbits(cardnum,wValue);
      board_using_msg_schd[cardnum]=0;
   }

   wValue = CR1_FR_STRT_TIME;
   if(bc_options & FRAME_START_TIMING)   
   {
      api_sethwcbits(cardnum,wValue);
   }
   else // use relative timing from end of message.
   {
      api_clearhwcbits(cardnum,wValue);
   }
        
   /*******************************************************************
   *  Show BC being initialized.
   *******************************************************************/
   bc_inited[cardnum] = 1;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtWriteRAM32[cardnum](cardnum,&bc_inited[cardnum],pShareTable[cardnum] + SHARE_BC_INITED,1); 
#endif //#ifdef SHARE_CHANNEL

   return status;
}

/****************************************************************************
*
* PROCEDURE NAME - v5_BC_Init
*
* FUNCTION
*     Set the BC initialization data in the RAM registers (V5 and earlier F/W)
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_RUNNING          -> BC currently running
*     API_BC_INITED           -> Error condition not used
*     API_BC_BADTIMEOUT1      -> No Response Time < 4 or >= 32/62
*     API_BC_BADTIMEOUT2      -> Late Response Time < 4 or >= 32/62
*     API_BC_BADFREQUENCY     -> frame > 1638375 us or frame < 1000 us
*
****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers)   // (i) number of BC message buffers ( 1 or 2 )
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U16BIT data;
   BT_U16BIT wValue;
   int i;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
   {
      vbtReadRAM32[cardnum](cardnum,&bc_inited[cardnum],pShareTable[cardnum] + SHARE_BC_INITED,1);
      if(bc_inited[cardnum])
         return API_BC_INITED;
   } 
#endif //#ifdef SHARE_CHANNEL

   /*******************************************************************
   *  Check parameter ranges
   *******************************************************************/
   if ((wTimeout1 < 4) || (wTimeout1 > 31))
      return API_BC_BADTIMEOUT1;
   if ((wTimeout2 < 4) || (wTimeout2 > 30))
      return API_BC_BADTIMEOUT2;

   if((bc_options & FIXED_GAP) && (bc_options & FRAME_START_TIMING))
      return API_PARAM_CONFLICT;

   /*******************************************************************
   *  BC is either single or double buffered.  Record
   *  the size of a buffer, and the number of buffers.
   *******************************************************************/
   if ( bc_running[cardnum] == 0 )
   {
      bc_size_dbuf[cardnum] = 2 * PCI1553_BUFCOUNT;  /* 40 words per buffer */

      if ( num_buffers != 1 ) /* Number of BC message buffers ( 1 or 2 ) */
         bc_num_dbufs[cardnum] = 2;  /* Double buffered. */
      else
         bc_num_dbufs[cardnum] = 1;  /* Single buffered. */
   }

   /*******************************************************************
   *  BC interrupt enables
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB1,(BT_U16BIT)(Enable      ));
   vbtSetFileRegister(cardnum,RAMREG_BC_INT_ENB2,(BT_U16BIT)(Enable >> 16));

   vbtSetFileRegister(cardnum,RAMREG_BC_RETRY, 0);
   vbtSetFileRegister(cardnum,RAMREG_DQ_RETRY, 0);
   // check for the retry methosd to use based on card type

   /******************************************************************
	* use the retry buffer to allow up to 8 retries
	******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_RETRYPTR,(BT_U16BIT)(BTMEM_BC_RETRY_BUF>>hw_addr_shift[cardnum]));
   data = 0; 
   for(i=0;i<9;i++) // clear the retry buffer
   {
      vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF+(i*2),2);
   }
   if(wRetry) // If there are any retry options selected set up a single retry
   {
      if ( wRetry & BC_RETRY_ALTB )
      {
         data = RETRY_ALTERNATE_BUS;
         vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF,2); //Retry Alernate Bus
      }
      else
      {
         data = RETRY_SAME_BUS;
         vbtWrite(cardnum,(LPSTR)(&data),BTMEM_BC_RETRY_BUF,2);      // Retry Same Bus
      }
      data = RETRY_END;
      vbtWrite(cardnum,(LPSTR)(&data),(BTMEM_BC_RETRY_BUF+2),2);            // Single Retry
   }
   
   if(bc_options & MULTIPLE_BC_BUFFERS)
      return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Store retry bits into RAM registers 45 & 78,
   *  using the write under mask low-level function.
   *******************************************************************/
   if ( wRetry & BC_RETRY_NRSP )  /* Register 78 Bit 10,No Response    */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_FLAGS*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_FLAGS*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_ME   )  /* Register 45 Bit 10,Message Error  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_BUSY )  /* Register 45 Bit  3,Busy Bit Set   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT03, BIT03);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT03);

   if ( wRetry & BC_RETRY_TF   )  /* Register 45 Bit  0,Terminal Flag  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT00, BIT00);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT00);

   if ( wRetry & BC_RETRY_SSF  )  /* Register 45 Bit  2,SubSystem Flag */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT02, BIT02);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT02);

   if ( wRetry & BC_RETRY_INSTR)  /* Register 45 Bit  9,Instrumentation*/
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT09, BIT09);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT09);

   if ( wRetry & BC_RETRY_SRQ  )  /* Register 45 Bit  8,Service Request*/
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, BIT08, BIT08);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_BC_RETRY*2, 0,     BIT08);

   if ( wRetry & BC_RETRY_INV_WRD   )  /* Register 7 Bit 2,Invalid Word  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT01, BIT01);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT01);

   if ( wRetry & BC_RETRY_INV_SYNC )  /* Register 7 Bit  3,Inverted Sync   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT03, BIT03);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT03);

   if ( wRetry & BC_RETRY_MID_BIT   )  /* Register 7 Bit  4,Mid Bit  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT04, BIT04);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT04);

   if ( wRetry & BC_RETRY_TWO_BUS  )  /* Register 7 Bit  5,Two Bus */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT05, BIT05);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT05);

   if ( wRetry & BC_RETRY_PARITY)  /* Register 7 Bit  6, Parity */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT06, BIT06);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT06);

   if ( wRetry & BC_RETRY_CONT_DATA  )  /* Register 7 Bit  7,  Non Contiguous Data */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT07, BIT07);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT07);

   if ( wRetry & BC_RETRY_EARLY_RSP   )  /* Register 7 Bit 8, Early Response  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT08, BIT08);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT08);

   if ( wRetry & BC_RETRY_LATE_RSP )  /* Register 7 Bit  9, Late Response   */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT09, BIT09);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT09);

   if ( wRetry & BC_RETRY_BAD_ADDR   )  /* Register 7 Bit  10 , Bad RT Address  */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT10, BIT10);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT10);

   if ( wRetry & BC_RETRY_WRONG_BUS  )  /* Register 7 Bit  13, Wrong Bus */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT13, BIT13);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT13);

   if ( wRetry & BC_RETRY_BIT_COUNT)  /* Register 7 Bit  14, Bit Count */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT14, BIT14);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT14);

   if ( wRetry & BC_RETRY_NO_GAP  )  /* Register 7 Bit  15,  No Inter-message gap */
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, BIT15, BIT15);
   else
      vbtReadModifyWrite(cardnum, RAMREG, RAMREG_DQ_RETRY*2, 0,     BIT15);

   /*******************************************************************
   *  Store response register.  Round it up by 1/2 microsecond.V4.01.ajh
   *******************************************************************/
   wTimeout1 = (wTimeout1 << 1) | 1;
   wTimeout2 = (wTimeout2 << 1) | 1;
   wValue = (BT_U16BIT)(((wTimeout1 & 0x3f) << 8) + (wTimeout2 & 0x3f));

   /* The HWREG_RESPONSE register must be programmed after one of the three  */
   /* run bits has been set.                                                 */

   wResponseReg4[cardnum] = wValue;  /* Save value for later.*/
   api_writehwreg(cardnum,HWREG_RESPONSE,wValue);

   if((_HW_FPGARev[cardnum] & 0xfff) >= 0x499)
   {
      board_using_extended_timing[cardnum]=1;
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue |= CR2_EXTD_TIME;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }
   else
      board_using_extended_timing[cardnum]=0;

   /*******************************************************************
   *  Store minor frame register -- frame time is in
   *   units of 1 or 25 micro-seconds
   *******************************************************************/
   if(board_using_extended_timing[cardnum])// 1 microsecond timing
   {
      BT_U16BIT frame_lsb,frame_msb;

      if (frame < 250)
         return API_BC_BADFREQUENCY;
      frametime32[cardnum] = (BT_U32BIT)frame;
      frame_lsb = (BT_U16BIT)frametime32[cardnum] & 0xffff;
      frame_msb = (BT_U16BIT)((frametime32[cardnum] & 0xffff0000)>>16);
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_LSB, frame_lsb );
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_MSB, frame_msb );
   }
   else //25 microsecond timing
   {
      if ((frame > 1638375L) || (frame < 250))
         return API_BC_BADFREQUENCY;
      frametime[cardnum] = (BT_U16BIT)(frame/25);   /* Must be less than 65535. */
      vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);
   }

   //iqptr_bc_last[cardnum] = BTMEM_IQ;
   iqptr_bc_last[cardnum] = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;

   /*******************************************************************
   *  gap timing options fix or relative
   *******************************************************************/
   if(bc_options < 0xf)
      bc_options = 0;

   if((_HW_FPGARev[cardnum] & 0xfff) >= 0x415)
   {
      if(bc_options & FIXED_GAP)// use fixed timing from start of message
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_FIXED_GAP;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else // use relative timing from end of message.
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue &= ~CR2_FIXED_GAP;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
   }

   if(bc_options & MSG_SCHD)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x439)
      {
         if(num_buffers == 2)
            return API_PARAM_CONFLICT;

         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_MSG_SCHD;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
         board_using_msg_schd[cardnum]=1;
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // no message scheduling.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_MSG_SCHD;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      board_using_msg_schd[cardnum]=0;
   }

   if(bc_options & FRAME_START_TIMING)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x499)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_FR_STRT_TIME;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
         board_using_frame_start_timing[cardnum]=1;
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // No frame start timing.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_FR_STRT_TIME;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      board_using_frame_start_timing[cardnum]=0;
   }

   if(bc_options & NO_PRED_LOGIC)   
   {
      if((_HW_FPGARev[cardnum] & 0xfff) >= 0x440)
      {
         vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
         wValue |= CR2_NO_PREDICT;
         vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
      }
      else
         return API_HARDWARE_NOSUPPORT;
   }
   else // no message scheduling.
   {
      vbtRead(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2); 
      wValue &= ~CR2_NO_PREDICT;
      vbtWrite(cardnum,(char *)&wValue,HWREG_CONTROL2*2,2);
   }
        
   /*******************************************************************
   *  Show BC being initialized.
   *******************************************************************/
   bc_inited[cardnum] = 1;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtWriteRAM32[cardnum](cardnum,&bc_inited[cardnum],pShareTable[cardnum] + SHARE_BC_INITED,1); 
#endif //#ifdef SHARE_CHANNEL

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_Init
*
* FUNCTION
*     Set the BC initialization data in the RAM registers
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_RUNNING          -> BC currently running
*     API_BC_INITED           -> Error condition not used
*     API_BC_BADTIMEOUT1      -> No Response Time < 4 or >= 32/62
*     API_BC_BADTIMEOUT2      -> Late Response Time < 4 or >= 32/62
*     API_BC_BADFREQUENCY     -> frame > 1638375 us or frame < 1000 us
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers)   // (i) number of BC message buffers ( 1 or 2 for legacy)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_Init[cardnum](cardnum,bc_options, Enable, wRetry,wTimeout1,wTimeout2,frame,num_buffers);
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_RetryInit
*   
*  FUNCTION
*     This routine setup retry options. These include the number
*     of retries and the bus on which the retry occurs. 
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_ERR_PARAM           -> Bad retry parameter.
*
*     v6 Version
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_RetryInit(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U16BIT *bc_retry)     //
{
   BT_INT i;
   BT_U32BIT retry_data=0;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   
   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
   {
      for(i=0;i<9;i++)
      {
         if(bc_retry[i] < 3)
            vbtWrite(cardnum,(char *)&bc_retry[i],BTMEM_BC_RETRY_BUF+(i*2),2);
         else
            return API_BAD_PARAM;

         if(bc_retry[i] == 0)  // 0 = end of retry buffer
            break;
      }
   }
   else
   {
      for(i=0;i<16;i++)
      {
         if(bc_retry[i] < 3)
            retry_data |= bc_retry[i]<<(2*i);   
         else
            return API_BAD_PARAM;

         if(bc_retry[i] == 0)  // 0 = end of retry buffer
            break;
      }
      vbtSetRegister[cardnum](cardnum,HWREG_BC_RETRY_CTL,retry_data);
   }

   return API_SUCCESS;
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_IsRunning
*   
*  FUNCTION
*     This routine returns the running/not-running state
*     of the BC.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_NOTRUNNING       -> BC not currently running
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT * flag)      // (o) Returns 1 if running, 0 if not
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT value;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /************************************************
   *  Read the hardware register and check BC bit
   ************************************************/
   if(board_is_v5_uca[cardnum])
      value = api_readhwreg(cardnum,HWREG_CONTROL1);
   else
      value = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);

   if ( value & CR1_BCRUN )
   {
      // Let the API know the status.V4.26.ajh
      bc_running[cardnum] = 1;
      *flag = 1;
   }
   else
   {
      // Let the API know the status.V4.26.ajh
      bc_running[cardnum] = 0;
      *flag = 0;
   }
   return API_SUCCESS;
}

/****************************************************************
*                               
*  PROCEDURE NAME - BusTools_BC_IsRunning2
*   
*  FUNCTION
*     This routine returns the running/not-running state
*     of the BC.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_NOTRUNNING       -> BC not currently running
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_IsRunning2(
   BT_UINT   cardnum)       // (i) card number (0 based)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_UINT value;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /************************************************
   *  Read the hardware register and check BC bit
   ************************************************/
   if(board_is_v5_uca[cardnum])
      value = api_readhwreg(cardnum,HWREG_CONTROL1);
   else
      value = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);

   if ( value & CR1_BCRUN )
   {
      bc_running[cardnum] = 1;
      return API_BC_IS_RUNNING;
   }
   else
   {
      bc_running[cardnum] = 0;
      return API_BC_IS_STOPPED;
   }
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageBlockAlloc (v6 Only)
*
*  FUNCTION
*     This routine allocates memory on the BT1553 board for a single message
*     block with a specified number of BC data buffers.  The memory are 
*     cleared, with the message and data buffer link addresses inserted.  This
*     function is only compatible with Multi-BC buffer mode. Call BusTools_BC_init
*     with MULTIPLE_BC_BUFFERS OR'd into the bc_options parameter.
*
*     When using this function you must allocate all the message block and
*     associated data buffer at one time.  Each message block has a variable 
*     number of linked data buffers specified by the count parameter.  The
*     bufferID parameter is used to indicate the end of the message block list.
*     Use BC_BUFFER_NEXT when creating all but the last message block.  Use 
*     BC_BUFFER_LAST when creating the last message buffer in the list,   
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_MULTI_BUFFER_ERR -> Not configure for Multi-buffer operation
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_MEMORY_OFLOW     -> Not enough memory to create messages
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageBlockAlloc(
   BT_UINT cardnum,         // (i) card number (0 based)
   BT_UINT bufID,           // (i) BC_BLOCK_NEXT or BC_BLOCK_LAST
   BT_UINT count)           // (i) Number of BC data buffer linked to the
                            // message buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BC_MSGBUF   bc_buffer;          /* Hardware BM Message buffer image */
   BC_MSGDATA  bc_data;            /* Hardware data buffer             */
   BT_UINT     i;               /* Loop counters */

   BT_U32BIT   bytes_required;     /* Number of bytes required for msgs +data */
   BT_U32BIT   addr;               /* Byte offset of current BC msg buffer    */
   BT_U32BIT   nextbc;             // Byte offset to next BC message buffer   */
   BT_U32BIT   addr_data1;         /* Byte offset to BC data buffer 1         */
   BT_U32BIT   btmem_bc_save;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (bc_running[cardnum])
      return API_BC_RUNNING;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(!channel_using_multiple_bc_buffers[cardnum])
      return API_BC_MULTI_BUFFER_ERR;
   if(count == 0)
      return API_BAD_PARAM;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtReadRAM32[cardnum](cardnum,&btmem_pci1553_next[cardnum],pShareTable[cardnum] + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   /* Compute the number of bytes required for message and data buffers */
   bytes_required =  BC_MSGBUF_SIZE + (count * BC_MSGDATA_SIZE);

   if(mblock_addr[cardnum] == NULL)
   {
      bc_mblock_count[cardnum]++;  //increment the mblock count for this buffer
      mblock_addr[cardnum] = (BT_U32BIT *)CEI_MALLOC(sizeof(BT_U32BIT)); //allocate table of buffer address
   }
   else
   {
      bc_mblock_count[cardnum]++;  //increment the mblock count for this buffer
      mblock_addr[cardnum] = (BT_U32BIT *)CEI_REALLOC((void *)mblock_addr[cardnum], (sizeof(BT_U32BIT) * bc_mblock_count[cardnum]));
   }
   /*******************************************************************
   *  Check for memory overflow
   *******************************************************************/
   nextbc = bytes_required + btmem_pci1553_next[cardnum];
   if ( nextbc > btmem_pci1553_rt_mbuf[cardnum] )
      return API_BC_MEMORY_OFLOW;


   if(bc_mblock_count[cardnum]==1)   //This is the first buffer allocated save the address.
      btmem_bc[cardnum] = btmem_pci1553_next[cardnum];

   btmem_bc_save = btmem_pci1553_next[cardnum];
   btmem_bc_next[cardnum] = nextbc;
   btmem_pci1553_next[cardnum] = nextbc;

   /*******************************************************************
   *  Initialize the count of messages available.
   *******************************************************************/
   nextbc = btmem_bc_save;

   /******************************************************************
   *  Init the hardware pointer to the beginning of the new bus list
   *  Save BC buffer start address in byte address format
   *******************************************************************/
   if(bc_mblock_count[cardnum]==1)
      vbtSetRegister[cardnum](cardnum,HWREG_BCMSG_PTR,RAM_ADDR(cardnum,nextbc));

   /*******************************************************************
   *  Build and output a no-op BC message block, linked to the next 
   *  containing pointers to the BC data blocks.
   *******************************************************************/
   memset(&bc_buffer, 0, BC_MSGBUF_SIZE);

   /* Set error injection address to default location (EI buffer 0) */
   bc_buffer.addr_error_inj = RAM_ADDR(cardnum, BTMEM_EI_V6);
   bc_buffer.gap_time = 15;  // Intermessage gap time in microseconds.
   bc_buffer.messno = bc_mblock_count[cardnum] - 1;

   /* Initialize the BC message blocks. */
   addr = btmem_bc_save;

   /****************************************************************
   *  Fill in default values for all pointers in block.
   ****************************************************************/
   /* Set data buffer addresses */
   addr_data1 = nextbc + BC_MSGBUF_SIZE;
   bc_buffer.data_addr = RAM_ADDR(cardnum,addr_data1);
   bc_buffer.num_data_buffers = count;      

   /* Compute the next message address.    */
   addr += BC_MSGBUF_SIZE + (count * BC_MSGDATA_SIZE);

   if (bufID == BC_BLOCK_NEXT)
      bc_buffer.addr_next = RAM_ADDR(cardnum,addr);
   else
      bc_buffer.addr_next = RAM_ADDR(cardnum,btmem_bc_save);

   /* Write the message buffer to the board */
   vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_buffer, nextbc, BC_MSGBUF_DWSIZE);

   //add the data buffers
   for(i=0; i<count; i++)
   {   
      memset(&bc_data, 0, BC_MSGDATA_SIZE);      
      bc_data.msg_addr = RAM_ADDR(cardnum,nextbc);
      if(i<count-1)
         bc_data.next_data = RAM_ADDR(cardnum, addr_data1 + (BC_MSGDATA_SIZE * (i+1))); //point to next buffer
      else
         bc_data.next_data = RAM_ADDR(cardnum,addr_data1);

      vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_data,addr_data1 + (BC_MSGDATA_SIZE * i), BC_MSGDATA_DWSIZE);
   }
   mblock_addr[cardnum][bc_mblock_count[cardnum]-1] = nextbc;  
   nextbc = addr;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&btmem_pci1553_next[cardnum],pShareTable[cardnum] + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - v5_BC_MessageAlloc
*
*  FUNCTION
*     This routine allocates memory on the BT1553 board for the specified
*     number of BC message buffers.  The buffers are cleared, with the
*     message and data buffer link addresses inserted.
*
*     The message buffers are allocated from btmem_bc[] up, with message zero
*     located at btmem_BC[].  The data buffers are allocated from btmem_bc_next[]
*     down, with data buffer zero being allocated at btmem_bc_next[].
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_MBUF_ALLOCD      -> More messages requested than first call
*     API_BC_MEMORY_OFLOW     -> Not enough memory to create messages
*
****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT count)           // (i) Number of BC messages to allocate
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BC_MESSAGE  bc_buffer;          /* Hardware BM Message buffer image */
   BT_UINT     i;                  /* Loop counter */

   BT_U32BIT   bytes_required;     /* Number of bytes required for msgs +data */
   BT_U32BIT   addr;               /* Byte offset of current BC msg buffer    */
   BT_U32BIT   nextbc;             // Byte offset to next BC message buffer   */
   BT_U32BIT   data_addr1;         /* Byte offset to BC data buffer 1         */
   BT_U32BIT   data_addr2;         /* Byte offset to BC data buffer 2         */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (bc_running[cardnum])
      return API_BC_RUNNING;

   if (bc_mblock_count[cardnum] != 0)
   {
      if (bc_mblock_count[cardnum] < count)
         return API_BC_MBUF_ALLOCD;
   }

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageAlloc[cardnum] )
   {
      i = (*pUsrBC_MessageAlloc[cardnum])(cardnum, &count);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageAlloc[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   /* Compute the number of bytes required for message and data buffers */
   bytes_required = (BT_U32BIT)count * ((sizeof(BC_MESSAGE)) +
                               (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]));

   /*******************************************************************
   *  Check for memory overflow
   *******************************************************************/

   /*  Round the starting address up to a multiple of 8 words/16 bytes */
   btmem_pci1553_next[cardnum] = (btmem_pci1553_next[cardnum]+15) & 0xFFFFFFF0L;
   nextbc = bytes_required + btmem_pci1553_next[cardnum];
   if ( nextbc > btmem_pci1553_rt_mbuf[cardnum] )
      return API_BC_MEMORY_OFLOW;

   if ( btmem_bc[cardnum] == 0 )
   {
      btmem_bc[cardnum]           = btmem_pci1553_next[cardnum];
      btmem_bc_next[cardnum]      = nextbc;
      btmem_pci1553_next[cardnum] = nextbc;
   }

   /*******************************************************************
   *  Initialize the count of messages available.
   *******************************************************************/
   if ( bc_mblock_count[cardnum] == 0 )
      bc_mblock_count[cardnum] = (BT_U16BIT)count;

   nextbc = btmem_bc[cardnum];

   /*******************************************************************
   *  Init the hardware pointer to the beginning of the new bus list
   *******************************************************************/
   vbtSetFileRegister(cardnum,RAMREG_BC_MSG_PTR  ,0x0000);
   vbtSetFileRegister(cardnum,RAMREG_BC_MSG_PTRSV,(BT_U16BIT)(nextbc >> hw_addr_shift[cardnum]));

   /*******************************************************************
   *  Build and output the no-op BC message blocks, linked together
   *   and containing pointers to the BC data blocks.
   *******************************************************************/
   memset(&bc_buffer, 0, sizeof(BC_MESSAGE));

   /* Set error injection address to default location (EI buffer 0) */
   bc_buffer.addr_error_inj = (BT_U16BIT)(BTMEM_EI >> 1);
   bc_buffer.gap_time = 15;  // Intermessage gap time in microseconds.

   /* Initialize the BC message blocks. */
   for ( i = 0; i < count; i++ )
   {
      /****************************************************************
      *  Fill in default values for all pointers in block.
      ****************************************************************/
      /* Set data buffer addresses */
      data_addr2 = data_addr1 = nextbc + sizeof(BC_MESSAGE);

      if ( bc_num_dbufs[cardnum] != 1 ) /* Not single buffered. */
         data_addr2 += bc_size_dbuf[cardnum];
      bc_buffer.addr_data1 = (BT_U16BIT)(data_addr1 >> hw_addr_shift[cardnum]);
      bc_buffer.addr_data2 = (BT_U16BIT)(data_addr2 >> hw_addr_shift[cardnum]);

      /* Compute the next message address.    */
      addr = BC_MBLOCK_ADDR(cardnum, i+1);
      if ( i < (count-1) )
         bc_buffer.addr_next = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);
      else
         bc_buffer.addr_next = (BT_U16BIT)(btmem_bc[cardnum] >> hw_addr_shift[cardnum]);

      /* Write the buffers to the board so BusTools_BC_MessageWrite() */
      /*  et.al. can get the pointers to the data buffers.            */
      vbtWrite(cardnum, (LPSTR)&bc_buffer, nextbc, sizeof(BC_MESSAGE));
      nextbc = addr;
   }

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtWriteRAM32[cardnum](cardnum,&btmem_pci1553_next[cardnum],BTMEM_CH_SHARE + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - v6_BC_MessageAlloc
*
*  FUNCTION
*     This routine allocates memory on the BT1553 board for the specified
*     number of BC message buffers.  The buffers are cleared, with the
*     message and data buffer link addresses inserted.
*
*     The message buffers are allocated from btmem_bc[] up, with message zero
*     located at btmem_BC[].  The data buffers are allocated from btmem_bc_next[]
*     down, with data buffer zero being allocated at btmem_bc_next[].
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_MBUF_ALLOCD      -> More messages requested than first call
*     API_BC_MEMORY_OFLOW     -> Not enough memory to create messages
*
****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 based)
   BT_UINT count)           // (i) Number of BC messages to allocate
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BC_MSGBUF   bc_buffer;          /* Hardware BM Message buffer image */
   BC_MSGDATA  bc_data;            /* Hardware data buffer             */
   BT_UINT     i;               /* Loop counters */

   BT_U32BIT   bytes_required;     /* Number of bytes required for msgs +data */
   BT_U32BIT   addr;               /* Byte offset of current BC msg buffer    */
   BT_U32BIT   nextbc;             // Byte offset to next BC message buffer   */
   BT_U32BIT   addr_data1;         /* Byte offset to BC data buffer 1         */
   BT_U32BIT   addr_data2;         /* Byte offset to BC data buffer 2         */

   /******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (bc_running[cardnum])
      return API_BC_RUNNING;

   if (bc_mblock_count[cardnum] != 0)
   {
      return API_BC_MBUF_ALLOCD;
   } 

   if(channel_using_multiple_bc_buffers[cardnum])
      return API_BC_MULTI_BUFFER_ERR;

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtReadRAM32[cardnum](cardnum,&btmem_pci1553_next[cardnum],BTMEM_CH_V6SHARE + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageAlloc[cardnum] )
   {
      i = (*pUsrBC_MessageAlloc[cardnum])(cardnum, &count);
      if ( i == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( i == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageAlloc[cardnum] = NULL;
      else if ( i != API_CONTINUE )
         return i;
   }
#endif

   /* Compute the number of bytes required for message and data buffers */
   bytes_required = count * (BC_MSGBUF_SIZE + (bc_num_dbufs[cardnum] * BC_MSGDATA_SIZE));

   /*******************************************************************
   *  Check for memory overflow
   *******************************************************************/
   nextbc = bytes_required + btmem_pci1553_next[cardnum];
   if ( nextbc > btmem_pci1553_rt_mbuf[cardnum] )
      return API_BC_MEMORY_OFLOW;

   /* allocate buffer */
   mblock_addr[cardnum] = (BT_U32BIT *)CEI_MALLOC(count * sizeof(BT_U32BIT));

   btmem_bc[cardnum]           = btmem_pci1553_next[cardnum];
   btmem_bc_next[cardnum]      = nextbc;
   btmem_pci1553_next[cardnum] = nextbc;

   /*******************************************************************
   *  Initialize the count of messages available.
   *******************************************************************/
   bc_mblock_count[cardnum] = (BT_U16BIT)count;
   nextbc = btmem_bc[cardnum];

   /*******************************************************************
   *  Init the hardware pointer to the beginning of the new bus list
   *******************************************************************/
   vbtSetRegister[cardnum](cardnum,HWREG_BCMSG_PTR,RAM_ADDR(cardnum,nextbc));  //Save BC buffer start address in byte address format

   /*******************************************************************
   *  Build and output the no-op BC message blocks, linked together
   *   and containing pointers to the BC data blocks.
   *******************************************************************/
   memset(&bc_buffer, 0, BC_MSGBUF_SIZE);

   /* Set error injection address to default location (EI buffer 0) */
   bc_buffer.addr_error_inj = RAM_ADDR(cardnum, BTMEM_EI_V6);
   bc_buffer.gap_time = 15;  // Intermessage gap time in microseconds.

   /* Initialize the BC message blocks. */
   addr = btmem_bc[cardnum];
   for ( i = 0; i < count; i++ )
   {
      /****************************************************************
      *  Fill in default values for all pointers in block.
      ****************************************************************/
      /* Set data buffer addresses */
      addr_data1 = addr_data2 = nextbc + BC_MSGBUF_SIZE;
      bc_buffer.data_addr = RAM_ADDR(cardnum,addr_data1); 
      bc_buffer.messno = i;
      bc_buffer.num_data_buffers = bc_num_dbufs[cardnum];    

      /* Compute the next message address.    */
      addr += BC_MSGBUF_SIZE + (bc_num_dbufs[cardnum] * BC_MSGDATA_SIZE);

      if ( i < (count-1) )
         bc_buffer.addr_next = RAM_ADDR(cardnum,addr);
      else
         bc_buffer.addr_next = RAM_ADDR(cardnum,btmem_bc[cardnum]);

      /* Write the message buffer to the board */
      vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_buffer, nextbc, BC_MSGBUF_DWSIZE);

      //add the data buffer(s)   
      memset(&bc_data, 0, BC_MSGDATA_SIZE);      
      bc_data.msg_addr = RAM_ADDR(cardnum,nextbc);
      bc_data.next_data = RAM_ADDR(cardnum,addr_data1);                 //point to this buffer 
      vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_data,addr_data1 , BC_MSGDATA_DWSIZE);
      if ( bc_num_dbufs[cardnum] == 2 )           /* Not single buffered. */
      {
         addr_data2 += BC_MSGDATA_SIZE;
         bc_data.next_data = RAM_ADDR(cardnum,addr_data2);                 //point to this buffer
         vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_data,addr_data2 , BC_MSGDATA_DWSIZE);
      }
      mblock_addr[cardnum][i] = nextbc;  
      nextbc = addr;
   }

#ifdef SHARE_CHANNEL
   if(channel_is_shared[cardnum])
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&btmem_pci1553_next[cardnum],BTMEM_CH_V6SHARE + SHARE_BTMEM_PCI1553_NEXT,1);
#endif //#ifdef SHARE_CHANNEL

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageAlloc
*
*  FUNCTION
*     This routine allocates memory on the BT1553 board for the specified
*     number of BC message buffers.  The buffers are cleared, with the
*     message and data buffer link addresses inserted.
*
*     The message buffers are allocated from btmem_bc[] up, with message zero
*     located at btmem_BC[].  The data buffers are allocated from btmem_bc_next[]
*     down, with data buffer zero being allocated at btmem_bc_next[].
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_RUNNING          -> BC currently running
*     API_BC_MBUF_ALLOCD      -> More messages requested than first call
*     API_BC_MEMORY_OFLOW     -> Not enough memory to create messages
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT count)           // (i) Number of BC messages to allocate
{
   BT_INT status;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;
   
   status = BT_BC_MessageAlloc[cardnum](cardnum,count);
   if(status)
      return status;

   playback_is_running[cardnum] = 0;
   return API_SUCCESS;
   
}

/****************************************************************
*
* PROCEDURE NAME - BusTools_BC_MessageGetaddr()
*
* FUNCTION
*     This routine converts a BC message id to a BT1553 address.
*     Note that the address is in bytes.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetaddr(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to compute address
   BT_U32BIT * addr)        // (o) Returned byte address of BC message
{
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(board_is_v5_uca[cardnum])
   { 
      /************************************************
      *  Return computed address
      ************************************************/
      /* Address is base plus length of preceeding messages. */
      *addr = BC_MBLOCK_ADDR(cardnum, messageid);
   }
   else
   {
      /************************************************
      *  Return computed address
      ************************************************/
      /* Address is base plus length of preceeding messages. */
      *addr = mblock_addr[cardnum][messageid];
   }
   return API_SUCCESS;
}

/****************************************************************************
*                               
* PROCEDURE NAME - BusTools_BC_MessageGetid()
*   
* FUNCTION
*     This routine converts a BT1553 address to a BC message
*     id.  Note that the address is in bytes, as from the interrupt queue.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_MBLOCK_NOMATCH   -> Address is not a BC message block
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageGetid(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT addr,          // (i) Byte address of BC message data from int queue
   BT_UINT * messageid)     // (o) Number of the BC message
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  maddr;
   BT_U32BIT  msg_blk_size;   // Size of a BC message plus data buffers.

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
   {
      /*******************************************************************
      *  Calculate index based on starting address & size.
      *******************************************************************/
      msg_blk_size = sizeof(BC_MESSAGE) +
                                  (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

      addr <<= (hw_addr_shift[cardnum]-1);
 
      *messageid = (BT_UINT)( (addr - btmem_bc[cardnum]) / msg_blk_size);
   }
   else
   {
      /*******************************************************************
      *  Calculate index based on starting address & size.
      *******************************************************************/
      vbtReadRAM32[cardnum](cardnum,&maddr,addr+BC_DATA_MSG_OFFSET,1);  
      vbtReadRelRAM32[cardnum](cardnum,messageid,maddr + BC_MESSNO_OFFSET,1);
   }
   
   if ( *messageid < bc_mblock_count[cardnum] )
      return API_SUCCESS;
   else
      return API_BC_MBLOCK_NOMATCH;
}

/****************************************************************************
*
* PROCEDURE NAME - v5_BC_MessageNoop()
*
* FUNCTION
*     This routine takes a BC message id and toggles the message
*     between a NOOP and an 1553 message.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_NOTMESSAGE       -> This is not a proper 1553-type message
*     API_BC_NOTNOOP          -> This is not a proper noop-type message
*
****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag)        // (i) Flag, Not used
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT    msg_addr;     /* Address of BC message. */
   BT_U16BIT    temp;         /* Temp for swap. */

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Compute address of BC message
   ************************************************/
   /* Address is base plus length of preceeding messages. */
   msg_addr = BC_MBLOCK_ADDR(cardnum, messageid);

   /*******************************************************************
   *  Figure out if this is a legal swap.
   *******************************************************************/
   /* Fetch the message control word from the board. */
   vbtRead(cardnum, (LPSTR)&temp, msg_addr, 2);

   if ( (temp & 0x0006) == 0x0000 )
      return API_BC_CANT_NOOP;

   if (NoopFlag)         /* Set 1553 message to noop */
   {
      temp &= BC_HWCONTROL_SET_NOP; /* Noop bit is active when zero. */
      if(NoopFlag == TIMED_NOOP)
      {
         if((_HW_FPGARev[cardnum] & 0xfff) >= 0x439)
            temp |= BC_HWCONTROL_SET_TNOP;
         else
            return API_HARDWARE_NOSUPPORT;
      }
   }
   else
   {
      temp |= BC_HWCONTROL_OP;      /* Noop is inactive when one. */
      temp &= BC_HWCONTROL_CLEAR_TNOP; /* Timed noop disable when cleared */
   }   
   
   /* Update the message control word on the board. */
   vbtWrite(cardnum, (LPSTR)&temp, msg_addr, 2);

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - v6_BC_MessageNoop()
*
* FUNCTION
*     This routine takes a BC message id and toggles the message
*     between a NOOP and an 1553 message.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_NOTMESSAGE       -> This is not a proper 1553-type message
*     API_BC_NOTNOOP          -> This is not a proper noop-type message
*
****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag)        // (i) Flag, Not used
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT    msg_addr;     /* Address of BC message. */
   BT_U16BIT    temp;         /* Temp for swap. */

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Compute address of BC message
   ************************************************/
   /* Address is base plus length of preceeding messages. */
   msg_addr = mblock_addr[cardnum][messageid];

   /*******************************************************************
   *  Figure out if this is a legal swap.
   *******************************************************************/
   /* Fetch the message control word from the board. */
   vbtReadRAM[cardnum](cardnum, (BT_U16BIT *)&temp, msg_addr + BC_CTLWD_OFFSET, 1);

   if ( (temp & 0x0006) == 0x0000 )
      return API_BC_CANT_NOOP;

   if (NoopFlag)         /* Set 1553 message to noop */
   {
      temp &= BC_HWCONTROL_SET_NOP; /* Noop bit is active when zero. */
      if(NoopFlag == TIMED_NOOP)
      {
         temp |= BC_HWCONTROL_SET_TNOP;
      }
   }
   else
   {
      temp |= BC_HWCONTROL_OP;      /* Noop is inactive when one. */
      temp &= BC_HWCONTROL_CLEAR_TNOP; /* Timed noop disable when cleared */
   }   
   
   /* Update the message control word on the board. */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)&temp, msg_addr + BC_CTLWD_OFFSET, 1);

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - v5_BC_MessageNoop()
*
* FUNCTION
*     This routine takes a BC message id and toggles the message
*     between a NOOP and an 1553 message.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_NOTMESSAGE       -> This is not a proper 1553-type message
*     API_BC_NOTNOOP          -> This is not a proper noop-type message
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag)        // (i) Flag, Not Used
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_MessageNoop[cardnum](cardnum,messageid,NoopFlag,WaitFlag);
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_BC_ControlWordRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_ControlWordRead(
   BT_UINT   cardnum,            // (i) card number (0 based)
   BT_UINT messageid,            // (i) Number of BC message
   BT_U16BIT * api_control_word) // (i) Pointer variable to hold control word
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT       addr;        // Byte address of BC Message
   BT_U16BIT       control_word=0;
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(board_is_v5_uca[cardnum])
   {
      /*******************************************************************
      *  Get BC Message Block from hardware
      *******************************************************************/
      addr = BC_MBLOCK_ADDR(cardnum, messageid);
      vbtReadRAM[cardnum](cardnum,&control_word,addr,1*2);
   }
   else
   {
      /*******************************************************************
      *  Get BC Message Block from hardware
      *******************************************************************/
      addr = mblock_addr[cardnum][messageid];
      vbtReadRAM[cardnum](cardnum,(BT_U16BIT *)&control_word,addr + BC_CTLWD_OFFSET ,1);
   }

   if ( (control_word & BC_CONTROL_TYPEMASK) == 0x0002 || (control_word & 0x0007) == 0x0003 )  /* Normal 1553 message */
   {
      if ( (control_word & BC_CONTROL_TYPEMASK) == 0x0003)
         *api_control_word                = BC_CONTROL_MESSAGE;
      else
         *api_control_word                = BC_CONTROL_MSG_NOP;

      if (control_word & BC_HWCONTROL_MFRAMEBEG)
         *api_control_word             |= BC_CONTROL_MFRAME_BEG;
      if (control_word & BC_HWCONTROL_MFRAMEEND)
         *api_control_word             |= BC_CONTROL_MFRAME_END;
      if (control_word & BC_HWCONTROL_RTRTFORMAT)
         *api_control_word             |= BC_CONTROL_RTRTFORMAT;
      if (control_word & BC_HWCONTROL_RETRY)
         *api_control_word             |= BC_CONTROL_RETRY;
      if (control_word & BC_HWCONTROL_INTERRUPT)
         *api_control_word             |= BC_CONTROL_INTERRUPT;

      if(board_is_v5_uca[cardnum]) // Only for V5/4 boards
      {
         if (control_word & BC_HWCONTROL_BUFFERA)
            *api_control_word          |= BC_CONTROL_BUFFERA;
         else
            *api_control_word          |= BC_CONTROL_BUFFERB;
      }

      if (control_word & BC_HWCONTROL_CHANNELB)
         *api_control_word          |= BC_CONTROL_CHANNELB;
      else
         *api_control_word          |= BC_CONTROL_CHANNELA;

      if(control_word & BC_CONTROL_TIMED_NOP)
         *api_control_word          |= BC_CONTROL_TIMED_NOP;
   }
   else
      return API_BC_NOTMESSAGE;

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BC_ControlWordUpdate()
*
* FUNCTION
*     This routine allows users to change some of the BC Message
*     control parameters.  The following is the list of parameters:
*      Bus       - Select Bus A or B          -- BC_CONTROL_BUFFERA,  BC_CONTROL_BUFFERB
*      Buffer    - Select Buffer A or B       -- BC_CONTROL_CHANNELA, BC_CONTROL_CHANNELB
*      Interrupt - switch interrupt on or off -- BC_CONTROL_INTERRUPT
*      Retry     - switch retries on or off   -- BC_CONTROL_RETRY 
*      Int Queue - BC_CONTROL_INTQ_ONLY
*      NOP/Message - BC_CONTROL_NOP -- BC_CONTROL_MESSAGE
*     Each parameter in the list is processes so, if you do not select BC_CONTROL_RETRY
*     The retry option is cleared on this message
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> Message number larger than num alloc'ed
*     API_BC_UPDATEMESSTYPE   -> This is not a proper 1553-type message
*     API_BC_BOTHBUFFERS      -> Can't use both buffers
*     API_BC_BOTHBUSES        -> Can't set both buses
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ControlWordUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_U16BIT controlWord,   // (i) New Control Word settings
   BT_UINT WaitFlag)        // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_U32BIT    msg_addr;     // Address of BC message. 
   BT_U32BIT    bc_cntl_wrd;  // BC control Word.

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /************************************************
   *  Compute address of BC message
   ************************************************/
   if(board_is_v5_uca[cardnum]) 
   {
      msg_addr = BC_MBLOCK_ADDR(cardnum, messageid); 
      vbtReadRAM[cardnum](cardnum, (BT_U16BIT *)&bc_cntl_wrd, msg_addr,1*2);
   }
   else
   { 
      msg_addr = mblock_addr[cardnum][messageid];
      vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&bc_cntl_wrd, msg_addr + BC_CTLWD_OFFSET, 1);
   }

   if ( (bc_cntl_wrd & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   //Set or clear retries 
   if(controlWord & BC_CONTROL_RETRY)
      bc_cntl_wrd |= BC_HWCONTROL_RETRY;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_RETRY;

   //Set or clear interrupts
   if(controlWord & BC_CONTROL_INTERRUPT)
      bc_cntl_wrd |= BC_HWCONTROL_INTERRUPT;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_INTERRUPT;

   if(board_is_v5_uca[cardnum]) 
   {
      //Set Buffer A or Buffer B
      if((controlWord & BC_CONTROL_BUFFERA) && 
         (controlWord & BC_CONTROL_BUFFERB)   )
         return API_BC_BOTHBUFFERS;

      if(controlWord & BC_CONTROL_BUFFERA)
         bc_cntl_wrd |= BC_HWCONTROL_BUFFERA;
      if(controlWord & BC_CONTROL_BUFFERB)
         if (bc_num_dbufs[cardnum] == 2)
            bc_cntl_wrd &= ~BC_HWCONTROL_BUFFERA;
   }

   //Set Bus A or Bus B
   if((controlWord & BC_CONTROL_CHANNELA) && 
      (controlWord & BC_CONTROL_CHANNELB)   )
      return API_BC_BOTHBUSES;

   if(controlWord & BC_CONTROL_CHANNELB)
      bc_cntl_wrd |= BC_HWCONTROL_CHANNELB;
   if(controlWord & BC_CONTROL_CHANNELA)
      bc_cntl_wrd &= ~BC_HWCONTROL_CHANNELB;

   if(controlWord & BC_CONTROL_INTQ_ONLY)
      bc_cntl_wrd |= BC_HWCONTROL_INTQ_ONLY;
   else
      bc_cntl_wrd &= ~BC_HWCONTROL_INTQ_ONLY;

   if(controlWord & BC_CONTROL_MESSAGE)
      bc_cntl_wrd |= 0x1;     //Message
   else
      bc_cntl_wrd &= 0xfffe;  //NOP

   // Update the message control word on the board.
   if(board_is_v5_uca[cardnum])
      vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)&bc_cntl_wrd, msg_addr, 1);
   else 
      vbtWriteRAM32[cardnum](cardnum, &bc_cntl_wrd, msg_addr + BC_CTLWD_OFFSET, 1);
   
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - v5_BC_MessageRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV v5_BC_MessageRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT mblock_id,       // (i) Number of BC message
   API_BC_MBUF * api_message) // (i) Pointer to buffer to receive msg
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT         id;          // Message number or error injection number
   BT_UINT         status;      // Function return status
   BT_UINT         wordno;      // Word number for conditional messages
   union
   {
      BT1553_STATUS status;           // enabled word counts (bit field)
      BT_U16BIT data;         // enabled mode codes
   }NullStatus;  // Null status word for Broadcast RT-RT messages

   BT_U32BIT       addr;        // Byte address of BC Message
   BT_U32BIT       addr_prev;   // Byte address of previous BC Message
   BC_MESSAGE      mblock;      // Local copy of hardware BC Message
   union{
     BC_CBUF         *cpbuf;       // Pointer used to ref msg as an IP conditional
     BC_MESSAGE      *mpbuf;
   }bcptr;
   BC_CBUF         *cbuf;
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageRead[cardnum] )
   {
      status = (*pUsrBC_MessageRead[cardnum])(cardnum, &mblock_id, api_message);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageRead[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get BC Message Block from hardware
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   if(board_access_32[cardnum])
      vbtRead32(cardnum, (LPSTR)&mblock, addr, sizeof(mblock));
   else
      vbtRead(cardnum, (LPSTR)&mblock, addr, sizeof(mblock));

   /*******************************************************************
   *  Clear out caller's structure and fill in simple stuff
   *******************************************************************/
   /*memset(api_message+6,0,sizeof(API_BC_MBUF)-14-6);  // Not really needed */
   api_message->messno  = (BT_U16BIT)mblock_id;

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_BC_MESSAGEREAD, mblock_id, addr >> 1, 0, 0, 0);

   if ( (mblock.control_word & 0x0007) == 0x0002 || (mblock.control_word & 0x0007) == 0x0003 )  /* Normal 1553 message */
   {
      if ( (mblock.control_word & 0x0007) == 0x0003)
         api_message->control                = BC_CONTROL_MESSAGE;
      else
         api_message->control                = BC_CONTROL_MSG_NOP;

      if (mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
         api_message->control             |= BC_CONTROL_MFRAME_BEG;
      if (mblock.control_word & BC_HWCONTROL_MFRAMEEND)
         api_message->control             |= BC_CONTROL_MFRAME_END;
      if (mblock.control_word & 0x0020)
         api_message->control             |= BC_CONTROL_RTRTFORMAT;
      if (mblock.control_word & 0x0400)
         api_message->control             |= BC_CONTROL_RETRY;
      if (mblock.control_word & BC_HWCONTROL_INTERRUPT)
         api_message->control             |= BC_CONTROL_INTERRUPT;

      if (mblock.control_word & BC_HWCONTROL_BUFFERA)
         api_message->control          |= BC_CONTROL_BUFFERA;
      else
         api_message->control          |= BC_CONTROL_BUFFERB;
      if (mblock.control_word & BC_HWCONTROL_CHANNELB)
         api_message->control          |= BC_CONTROL_CHANNELB;
      else
         api_message->control          |= BC_CONTROL_CHANNELA;

      if(mblock.control_word & BC_CONTROL_TIMED_NOP)
         api_message->control          |= BC_CONTROL_TIMED_NOP;

      addr = ((BT_U32BIT)mblock.addr_next) << 1;                   // *8 applied below
      if ( addr == 0 )
         api_message->messno_next = 0xFFFF;
      else
      {
         status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
         if (status)
             return status;
         api_message->messno_next   = (BT_U16BIT)id;
      }

      if(board_using_extended_timing[cardnum])
      {
         api_message->long_gap = (BT_U32BIT)((mblock.gap_time2 << 16) | mblock.gap_time);
         if(api_message->long_gap < 0x10000)
            api_message->gap_time = (BT_U16BIT)(api_message->long_gap &0x0000ffff); //Set gap_time as well
      }
      else
         api_message->gap_time = mblock.gap_time;

      // Modify to correctly handle Broadcast RT-RT format.V4.01.ajh
      // Modified again for RT-RT and Broadcast RT-RT.V4.05.ajh
      api_message->mess_command1 = mblock.mess_command1;

      NullStatus.data = 0;
      if ( api_message->control & 0x0020 )       // RT-RT format
      {
         api_message->mess_command2 = mblock.mess_command2;
         if ( rt_bcst_enabled[cardnum] && (mblock.mess_command1.rtaddr == 31) )
         {
            api_message->mess_status1  = mblock.mess_status1;            // Swapped order
            api_message->mess_status2  = NullStatus.status;  //  in V4.05.
         }
         else
         {
            api_message->mess_status1  = mblock.mess_status1;
            api_message->mess_status2  = mblock.mess_status2;
         }
      }
      else
      {
         api_message->mess_status1  = mblock.mess_status1;
         api_message->mess_status2  = NullStatus.status; // Clear unused word.V4.04.ajh
      }
      api_message->status        = (((BT_U32BIT)mblock.mstatus[1])<<16) | mblock.mstatus[0];

      if(board_has_bc_timetag[cardnum])
         //api_message->time_tag = mblock.timetag;
         TimeTagConvert(cardnum, &(mblock.timetag), &(api_message->time_tag));

      if(board_using_msg_schd[cardnum])
      {
         api_message->rep_rate = mblock.addr_data2;
         api_message->start_frame = mblock.start_frame;
      }

      addr = ((BT_U32BIT)mblock.addr_error_inj) << 1;
      id = (BT_U16BIT)((addr - BTMEM_EI) / sizeof(EI_MESSAGE));
      api_message->errorid = id;

      addr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      
      if(board_access_32[cardnum])
         vbtRead32(cardnum,(LPSTR)(api_message->data[0]),addr,(2*BT1553_BUFCOUNT));
      else
         vbtRead(cardnum,(LPSTR)(api_message->data[0]),addr,2*BT1553_BUFCOUNT);

      /* If we are single-buffered, no need to read the same buffer twice. */
      if ( mblock.addr_data1 != mblock.addr_data2 )
      {
         addr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
         if(board_access_32[cardnum])
            vbtRead32(cardnum,(LPSTR)(api_message->data[1]),addr,(2*BT1553_BUFCOUNT));
         else
            vbtRead(cardnum,(LPSTR)(api_message->data[1]),addr,2*BT1553_BUFCOUNT);
      }

      return API_SUCCESS;
   }

   /*******************************************************************
   *  Buffer is NOT a 1553 message -- figure it out here
   *******************************************************************/
   bcptr.mpbuf = &mblock;
   cbuf = bcptr.cpbuf;

   /************************************************
   *  Process Read BC Message
   *  Get branch and next jump to addresses
   ************************************************/
   addr = ((BT_U32BIT)cbuf->branch_msg_ptr) << 1;
   if(addr>0)
   {
      status = BusTools_BC_MessageGetid(cardnum,addr,&id); 
      if (status)
         return status;
      api_message->messno_branch = (BT_U16BIT)id;
   }
   else
      api_message->messno_branch = (BT_U16BIT)0;

   addr = ((BT_U32BIT)cbuf->addr_next) << 1;    // Apply *8 below if needed.
   if ( addr == 0 )
      api_message->messno_next   = 0xFFFF;
   else
   {
      status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
      if (status)
         return status;
      api_message->messno_next   = (BT_U16BIT)id;
   }

   /*******************************************************************
   *  Now figure out what type of message this really is
   *******************************************************************/
   switch ( cbuf->control_word & 0x0006 )  
   {
      case 0x0000:       /* no-op or simple branch message (same format) */
         api_message->time_tag.microseconds = cbuf->timetag.microseconds;
         api_message->time_tag.topuseconds = (BT_U32BIT)cbuf->timetag.topuseconds;  
         if (api_message->messno_next != (api_message->messno + 1))
            api_message->control    = BC_CONTROL_BRANCH;
         else
         {
            api_message->control    = BC_CONTROL_NOP;
            if (mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
               api_message->control             |= BC_CONTROL_MFRAME_BEG;
            if (mblock.control_word & BC_HWCONTROL_MFRAMEEND)
               api_message->control             |= BC_CONTROL_MFRAME_END;
            if (mblock.control_word & BC_HWCONTROL_SET_TNOP)
               api_message->control             |= BC_CONTROL_TIMED_NOP;
            if (mblock.control_word & BC_HWCONTROL_INTERRUPT)
               api_message->control             |= BC_CONTROL_INTERRUPT;
         }
         if(mblock.control_word & BC_HWCONTROL_SET_TNOP)
         {
            if(board_using_extended_timing[cardnum])
               api_message->long_gap = (BT_U32BIT)((mblock.gap_time2 << 16) | mblock.gap_time);
            else
               api_message->gap_time = mblock.gap_time;
         }
         return API_SUCCESS;

      /************************************************
      *  Conditional branch
      ************************************************/
      case 0x0006:       /* Conditional Branch */
         if (mblock_id == 0)
            return API_BC_MESS1_COND;

         api_message->time_tag.microseconds = cbuf->timetag.microseconds;
         api_message->time_tag.topuseconds = (BT_U32BIT)cbuf->timetag.topuseconds;
         addr = ((BT_U32BIT)cbuf->branch_msg_ptr) << 1;
         status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
         if (status)
            return status;
         api_message->messno_branch = (BT_U16BIT)id;

         addr = ((BT_U32BIT)cbuf->addr_next) << 1;    // Apply *8 below if needed.
         if ( addr == 0 )
            api_message->messno_next   = 0xFFFF;
         else
         {
            status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
            if (status)
               return status;
            api_message->messno_next   = (BT_U16BIT)id;
         }

         api_message->data_value    = cbuf->data_pattern;
         api_message->data_mask     = cbuf->bit_mask;

         /* Read back the conditional counter values.V4.39.ajh */
         api_message->cond_count_val = cbuf->cond_count_val;
         api_message->cond_counter   = cbuf->cond_counter;

         addr = ((BT_U32BIT)cbuf->tst_wrd_addr1) << 1;      
         addr_prev = BC_MBLOCK_ADDR(cardnum, mblock_id-1);

         if (addr < addr_prev)
         {
            api_message->control    = BC_CONTROL_CONDITION2;
            api_message->address    = (BT_U16BIT)(addr >> hw_addr_shift[cardnum]);

            api_message->test_address = cbuf->tst_wrd_addr1 << 4 | cbuf->tst_wrd_addr2 << 1;

         }
         else
         {
            api_message->control    = BC_CONTROL_CONDITION;

            if (addr == addr_prev +
                            ((char*)&(mblock.mess_command1) - (char*)&mblock))
               wordno = 0;
            else if (addr == addr_prev +
                            ((char*)&(mblock.mess_command2) - (char*)&mblock))
               wordno = 1;
            else if (addr == addr_prev +
                             ((char*)&(mblock.mess_status1) - (char*)&mblock))
               wordno = 2;
            else if (addr == addr_prev +
                             ((char*)&(mblock.mess_status2) - (char*)&mblock))
               wordno = 3;
            else
               wordno = 4 +
                        (BT_U16BIT)((addr - (addr_prev + sizeof(BC_MESSAGE))) / 2);

            if (wordno > 35)
               return API_BC_BAD_COND_ADDR;

            api_message->address = (BT_U16BIT)wordno;
         }
         return API_SUCCESS;

      /****************************************************************
      *   Last message in list(Stop BC message)
      ****************************************************************/

      case 0x0004:       /* Stop BC message */
         api_message->control = BC_CONTROL_LAST;
         api_message->time_tag.microseconds = cbuf->timetag.microseconds;
         api_message->time_tag.topuseconds = (BT_U32BIT)cbuf->timetag.topuseconds;
         return API_SUCCESS;
   }
   return API_BC_ILLEGALMESSAGE;
}

/****************************************************************
*
*  PROCEDURE NAME - v6_BC_MessageRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV v6_BC_MessageRead(
   BT_UINT cardnum,            // (i) card number (0 based)
   BT_UINT messageid,          // (i) Number of BC message
   API_BC_MBUF * api_message)  // (i) Pointer to buffer to receive msg
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT         id;          // Message number or error injection number
   BT_UINT         wordno;      // Word number for conditional messages
   union
   {
      BT1553_STATUS status;           // enabled word counts (bit field)
      BT_U16BIT data;         // enabled mode codes
   }NullStatus;  // Null status word for Broadcast RT-RT messages

   BT_U32BIT       cwdata;      // Control Word 
   BT_U32BIT       addr;        // Byte address of BC Message
   BT_U32BIT       addr_prev;   // Byte address of previous BC Message
   BC_MSGBUF       mblock;      // Local copy of hardware BC Message
   BC_MSGDATA      dblock;
   BC_CTRLBUF      cblock;
   BC_CTRLDATA     cdblock;
   BT_INT i, count;
   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageRead[cardnum] )
   {
      BT_UINT         status;      // Function return status

      status = (*pUsrBC_MessageRead[cardnum])(cardnum, &messageid, api_message);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageRead[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get BC Message Block from hardware
   *******************************************************************/
   addr = mblock_addr[cardnum][messageid];

   /*******************************************************************
   * fill in simple stuff
   *******************************************************************/
   api_message->messno  = (BT_U16BIT)messageid;

   //Read Control Word
   vbtReadRAM32[cardnum](cardnum, &cwdata, addr + BC_CTLWD_OFFSET, 1);

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_BC_MESSAGEREAD, messageid, addr >> 1, 0, 0, 0);

   if ( (cwdata & 0x0007) == 0x0002 || (cwdata & 0x0007) == 0x0003 )  /* Normal 1553 message */
   {
      vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&mblock, addr, BC_MSGBUF_DWSIZE);
      vbtReadRelRAM32[cardnum](cardnum, (BT_U32BIT *)&dblock, mblock.data_addr,6);
      vbtReadRelRAM[cardnum](cardnum, (BT_U16BIT *)&dblock.data_word, mblock.data_addr+BC_DATA_DATA_OFFSET,34);
      
      if ( (mblock.control_word & 0x0007) == 0x0003)
         api_message->control                = BC_CONTROL_MESSAGE;
      else
         api_message->control                = BC_CONTROL_MSG_NOP;

      if (mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
         api_message->control             |= BC_CONTROL_MFRAME_BEG;
      if (mblock.control_word & BC_HWCONTROL_MFRAMEEND)
         api_message->control             |= BC_CONTROL_MFRAME_END;
      if (mblock.control_word & 0x0020)
         api_message->control             |= BC_CONTROL_RTRTFORMAT;
      if (mblock.control_word & 0x0400)
         api_message->control             |= BC_CONTROL_RETRY;
      if (mblock.control_word & BC_HWCONTROL_INTERRUPT)
         api_message->control             |= BC_CONTROL_INTERRUPT;

      if(mblock.data_addr == RAM_ADDR(cardnum,addr) + BC_MSGBUF_SIZE)
         api_message->control          |= BC_CONTROL_BUFFERA;
      else if(mblock.data_addr == RAM_ADDR(cardnum,addr) + BC_MSGBUF_SIZE + BC_MSGDATA_SIZE)
         api_message->control          |= BC_CONTROL_BUFFERB;         

      if (mblock.control_word & BC_HWCONTROL_CHANNELB)
         api_message->control          |= BC_CONTROL_CHANNELB;
      else
         api_message->control          |= BC_CONTROL_CHANNELA;

      if(mblock.control_word & BC_CONTROL_TIMED_NOP)
         api_message->control          |= BC_CONTROL_TIMED_NOP;
 
      if ( mblock.addr_next == 0 )
         api_message->messno_next = 0xFFFF;
      else
      {
         vbtReadRelRAM32[cardnum](cardnum,&id,mblock.addr_next + BC_MESSNO_OFFSET,1);
         api_message->messno_next   = (BT_U16BIT)id;
      }

      api_message->long_gap = mblock.gap_time;
      if(api_message->long_gap < 0x10000)
         api_message->gap_time = (BT_U16BIT)(api_message->long_gap &0x0000ffff); //Set gap_time as well

      flip((BT_U32BIT *)&mblock.mess_command1);
      flip((BT_U32BIT *)&dblock.mess_status1);

      api_message->mess_command1 = mblock.mess_command1;

      NullStatus.data = 0;
      if ( api_message->control & 0x0020 )       // RT-RT format
      {
         api_message->mess_command2 = mblock.mess_command2;
         if ( rt_bcst_enabled[cardnum] && (mblock.mess_command1.rtaddr == 31) )
         {
            api_message->mess_status1  = dblock.mess_status1;            // Swapped order
            api_message->mess_status2  = NullStatus.status;  //  in V4.05.
         }
         else
         {
            api_message->mess_status1  = dblock.mess_status1;
            api_message->mess_status2  = dblock.mess_status2;
         }
      }
      else
      {
         api_message->mess_status1  = dblock.mess_status1;
         api_message->mess_status2  = NullStatus.status; // Clear unused word.V4.04.ajh
      }
      api_message->status = dblock.mstatus;

      api_message->time_tag = dblock.timetag;

      if(board_using_msg_schd[cardnum])
      {
         flip((BT_U32BIT *)&mblock.rep_rate);
         api_message->rep_rate = mblock.rep_rate;
         api_message->start_frame = mblock.start_frame;
      }

      addr = REL_ADDR(cardnum,mblock.addr_error_inj);
      id = (BT_U16BIT)((addr - BTMEM_EI_V6) / sizeof(EI_MESSAGE));
      api_message->errorid = id;

      count = mblock.mess_command1.wcount;
      if(count == 0)
         count =32;

      if (api_message->control & BC_CONTROL_BUFFERA)
         for(i=0; i<count; i++)
           api_message->data[0][i] = dblock.data_word[i];
      else
         for(i=0; i<count ;i++)
            api_message->data[1][i] = dblock.data_word[i];

      return API_SUCCESS;
   }

   /*******************************************************************
   *  Buffer is NOT a 1553 message -- figure it out here
   *******************************************************************/
   vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&cblock, addr, BC_CTRLBUF_DWSIZE);
   vbtReadRelRAM32[cardnum](cardnum, (BT_U32BIT *)&cdblock, cblock.data_addr,BC_CTRLDATA_DWSIZE);

   /************************************************
   *  Process Read BC Message
   *  Get branch and next jump to addresses
   ************************************************/

   if(cblock.branch_msg_ptr>0)
   {
      vbtReadRelRAM32[cardnum](cardnum,&id,cblock.branch_msg_ptr + BC_MESSNO_OFFSET,1);
      api_message->messno_branch = (BT_U16BIT)id;
   }
   else
      api_message->messno_branch = (BT_U16BIT)0;

    
   if ( cblock.addr_next == 0 )
      api_message->messno_next   = 0xFFFF;
   else
   {
      vbtReadRelRAM32[cardnum](cardnum,&id,cblock.addr_next + BC_MESSNO_OFFSET,1);
      api_message->messno_next   = (BT_U16BIT)id;
   }

   /*******************************************************************
   *  Now figure out what type of message this really is
   *******************************************************************/
   switch ( cblock.control_word & 0x0006 )
   {
      case 0x0000:       /* no-op or simple branch message (same format) */
         api_message->time_tag = cdblock.timetag;
         if (api_message->messno_next != (api_message->messno + 1))
            api_message->control    = BC_CONTROL_BRANCH;
         else
         {
            api_message->control    = BC_CONTROL_NOP;
            if (cblock.control_word & BC_HWCONTROL_MFRAMEBEG)
               api_message->control             |= BC_CONTROL_MFRAME_BEG;
            if (cblock.control_word & BC_HWCONTROL_MFRAMEEND)
               api_message->control             |= BC_CONTROL_MFRAME_END;
            if (cblock.control_word & BC_HWCONTROL_SET_TNOP)
               api_message->control             |= BC_CONTROL_TIMED_NOP;
            if (cblock.control_word & BC_HWCONTROL_INTERRUPT)
               api_message->control             |= BC_CONTROL_INTERRUPT;
         }
         if(cblock.control_word & BC_HWCONTROL_SET_TNOP)
         {
            api_message->long_gap = cblock.gap_time;
            if(api_message->long_gap < 0x10000)
               api_message->gap_time = (BT_U16BIT)(api_message->long_gap &0x0000ffff); //Set gap_time as well
         }
         return API_SUCCESS;

      /************************************************
      *  Conditional branch
      ************************************************/
      case 0x0006:       /* Conditional Branch */
         if (messageid == 0)
            return API_BC_MESS1_COND;

         api_message->time_tag = cdblock.timetag;

         api_message->next_branch_addr = cdblock.next_msg_addr;  // Only applicable to f/w 6.03 or greater

         vbtReadRelRAM32[cardnum](cardnum,&id,cblock.branch_msg_ptr,1);

         api_message->messno_branch = (BT_U16BIT)id;

         if ( cblock.addr_next == 0 )
            api_message->messno_next   = 0xFFFF;
         else
         {
            vbtReadRelRAM32[cardnum](cardnum,&id,cblock.addr_next,1);
            //status = BusTools_BC_MessageGetid(cardnum,addr,&id);  // Apply *8 if needed.
            //if (status)
               //return status;
            api_message->messno_next   = (BT_U16BIT)id;
         }

         api_message->data_value    = cdblock.data_pattern;
         api_message->data_mask     = cdblock.bit_mask;

         api_message->cond_count_val = cdblock.cond_count_val;
         api_message->cond_counter   = cdblock.cond_counter;

         addr = cdblock.tst_wrd_addr;      
         addr_prev = RAM_ADDR(cardnum,mblock_addr[cardnum][messageid-1]);

         if(addr < addr_prev) 
         {
            api_message->control    = BC_CONTROL_CONDITION2;
            api_message->address    = (BT_U16BIT)(addr);

            api_message->test_address = cdblock.tst_wrd_addr;

         }
         else  //
         {
            api_message->control    = BC_CONTROL_CONDITION;

            if (addr == addr_prev + BC_MSG_CMD1_OFFSET) 
            {
               if(cdblock.bit_mask & 0xffff)
                  wordno = 0;
               else 
                  wordno = 1;
            }
            else if (addr == addr_prev + BC_DATA_MSTAT1_OFFSET)
            {
               if(cdblock.bit_mask & 0xffff)
                  wordno = 2;
               else 
                  wordno = 3;
            }
            else if (addr == addr_prev + BC_DATA_STATUS_OFFSET)
               wordno = 50;
            else
               wordno = 4 + (addr - (addr_prev + BC_DATA_DATA_OFFSET)); //change for v6                       
               if (wordno >= BC_CTRLDATA_RESERVED)
                  return API_BC_BAD_COND_ADDR;

            api_message->address = (BT_U16BIT)wordno;
         }
         return API_SUCCESS;

      /****************************************************************
      *   Last message in list(Stop BC message)
      ****************************************************************/

      case 0x0004:       /* Stop BC message */
         api_message->control = BC_CONTROL_LAST;
         api_message->time_tag = cdblock.timetag;
         return API_SUCCESS;
   }
   return API_BC_ILLEGALMESSAGE;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageRead(
   BT_UINT cardnum,            // (i) card number (0 based)
   BT_UINT messageid,          // (i) Number of BC message
   API_BC_MBUF * api_message)  // (i) Pointer to buffer to receive msg
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_MessageRead[cardnum](cardnum,messageid,api_message);
}


/****************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageBufferRead()
*
*  FUNCTION
*     This routine is used to read the specified BC Message
*     Block from the board.  The data contained therein is
*     returned in the caller supplied structure. V6 Only
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageBufferRead(
   BT_UINT cardnum,            // (i) card number (0 based)
   BT_U32BIT addr,             // (i) address of BC Data buffer
   API_BC_MBUF * api_message)  // (i) Pointer to buffer to receive msg
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT         id;          // Message number or error injection number
   BT_UINT         wordno;      // Word number for conditional messages
   union
   {
      BT1553_STATUS status;           // enabled word counts (bit field)
      BT_U16BIT data;         // enabled mode codes
   }NullStatus;  // Null status word for Broadcast RT-RT messages

   BT_U32BIT       cwdata;      // Control Word 
   BT_U32BIT       addr_prev;   // Byte address of previous BC Message

   union
   {
      BC_MSGBUF       mblock;      // Local copy of hardware BC Message 
      BC_CTRLBUF      cblock;         
      BT_U32BIT       cdata[BC_MSGBUF_DWSIZE];
   }CTRL;

   union
   {
      BC_MSGDATA      dblock;
      BC_CTRLDATA     cdblock;
      BT_U32BIT       cdata[BC_MSGDATA_DWSIZE];    
   }MDATA;

   BT_INT i, count;

   /************************************************
   *  Check initial conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /******************************************************************
   * Read the data buffer 
   *******************************************************************/
   //vbtReadRAM32[cardnum](cardnum,MDATA.cdata,addr,BC_MSGDATA_DWSIZE);

   vbtReadRAM32[cardnum](cardnum, MDATA.cdata, addr,6);
   vbtReadRAM[cardnum](cardnum, (BT_U16BIT *)&MDATA.dblock.data_word, addr+BC_DATA_DATA_OFFSET,34);

   /*******************************************************************
   * Read the Message buffer
   ********************************************************************/
   vbtReadRelRAM32[cardnum](cardnum,CTRL.cdata,MDATA.dblock.msg_addr,BC_MSGBUF_DWSIZE);
   cwdata = CTRL.mblock.control_word;

   api_message->messno  = (BT_U16BIT)CTRL.mblock.messno;

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NBUSTOOLS_BC_MESSAGEREAD, api_message->messno, addr >> 1, 0, 0, 0);

   if ( (cwdata & 0x0007) == 0x0002 || (cwdata & 0x0007) == 0x0003 )  /* Normal 1553 message */
   {
      if ( (CTRL.mblock.control_word & 0x0007) == 0x0003)
         api_message->control                = BC_CONTROL_MESSAGE;
      else
         api_message->control                = BC_CONTROL_MSG_NOP;

      if (CTRL.mblock.control_word & BC_HWCONTROL_MFRAMEBEG)
         api_message->control             |= BC_CONTROL_MFRAME_BEG;
      if (CTRL.mblock.control_word & BC_HWCONTROL_MFRAMEEND)
         api_message->control             |= BC_CONTROL_MFRAME_END;
      if (CTRL.mblock.control_word & 0x0020)
         api_message->control             |= BC_CONTROL_RTRTFORMAT;
      if (CTRL.mblock.control_word & 0x0400)
         api_message->control             |= BC_CONTROL_RETRY;
      if (CTRL.mblock.control_word & BC_HWCONTROL_INTERRUPT)
         api_message->control             |= BC_CONTROL_INTERRUPT;

      if(CTRL.mblock.data_addr == MDATA.dblock.msg_addr + BC_MSGBUF_SIZE)
         api_message->control          |= BC_CONTROL_BUFFERA;
      else if(CTRL.mblock.data_addr == MDATA.dblock.msg_addr + BC_MSGBUF_SIZE + BC_MSGDATA_SIZE)
         api_message->control          |= BC_CONTROL_BUFFERB;  

      if (CTRL.mblock.control_word & BC_HWCONTROL_CHANNELB)
         api_message->control          |= BC_CONTROL_CHANNELB;
      else
         api_message->control          |= BC_CONTROL_CHANNELA;

      if(CTRL.mblock.control_word & BC_CONTROL_TIMED_NOP)
         api_message->control          |= BC_CONTROL_TIMED_NOP;
 
      if ( CTRL.mblock.addr_next == 0 )
         api_message->messno_next = 0xFFFF;
      else
      {
         vbtReadRelRAM32[cardnum](cardnum,&id,CTRL.mblock.addr_next,1);
         api_message->messno_next   = (BT_U16BIT)id;
      }

      flip((BT_U32BIT *)&CTRL.mblock.mess_command1);
      flip((BT_U32BIT *)&MDATA.dblock.mess_status1);

      api_message->long_gap = CTRL.mblock.gap_time;
      if(api_message->long_gap < 0x10000)
         api_message->gap_time = (BT_U16BIT)(api_message->long_gap &0x0000ffff); //Set gap_time as well

      // Modify to correctly handle Broadcast RT-RT format.V4.01.ajh
      // Modified again for RT-RT and Broadcast RT-RT.V4.05.ajh
      api_message->mess_command1 = CTRL.mblock.mess_command1;

      NullStatus.data = 0;
      if ( api_message->control & 0x0020 )       // RT-RT format
      {
         api_message->mess_command2 = CTRL.mblock.mess_command2;
         if ( rt_bcst_enabled[cardnum] && (CTRL.mblock.mess_command1.rtaddr == 31) )
         {
            api_message->mess_status1  = MDATA.dblock.mess_status1;            // Swapped order
            api_message->mess_status2  = NullStatus.status;  //  in V4.05.
         }
         else
         {
            api_message->mess_status1  = MDATA.dblock.mess_status1;
            api_message->mess_status2  = MDATA.dblock.mess_status2;
         }
      }
      else
      {
         api_message->mess_status1  = MDATA.dblock.mess_status1;
         api_message->mess_status2  = NullStatus.status; // Clear unused word.V4.04.ajh
      }
      api_message->status   = MDATA.dblock.mstatus;
      api_message->time_tag = MDATA.dblock.timetag;

      if(board_using_msg_schd[cardnum])
      {
         api_message->rep_rate = CTRL.mblock.rep_rate;
         api_message->start_frame = CTRL.mblock.start_frame;
      }

      addr = REL_ADDR(cardnum,CTRL.mblock.addr_error_inj);
      id = (BT_U16BIT)((addr - BTMEM_EI_V6) / sizeof(EI_MESSAGE));
      api_message->errorid = id;

      count = CTRL.mblock.mess_command1.wcount;
      if(count == 0)
         count = 32;

      //Always write data to buffer A
      for(i=0; i<count; i++)
         api_message->data[0][i] = MDATA.dblock.data_word[i];

      return API_SUCCESS;
   }

   /************************************************
   *  Process Read BC Message
   *  Get branch and next jump to addresses
   ************************************************/

   if(CTRL.cblock.branch_msg_ptr>0)
   {
      vbtReadRelRAM32[cardnum](cardnum,&id,CTRL.cblock.branch_msg_ptr,1);
      api_message->messno_branch = (BT_U16BIT)id;
   }
   else
      api_message->messno_branch = (BT_U16BIT)0;

   if ( CTRL.cblock.addr_next == 0 )
      api_message->messno_next   = 0xFFFF;
   else
   {
      vbtReadRelRAM32[cardnum](cardnum,&id,CTRL.cblock.addr_next,1);
      api_message->messno_next   = (BT_U16BIT)id;
   }

   /*******************************************************************
   *  Now figure out what type of message this really is
   *******************************************************************/
   switch ( CTRL.cblock.control_word & 0x0006 )
   {
      case 0x0000:       /* no-op or simple branch message (same format) */
         api_message->time_tag = MDATA.cdblock.timetag;
         if (api_message->messno_next != (api_message->messno + 1))
            api_message->control    = BC_CONTROL_BRANCH;
         else
         {
            api_message->control    = BC_CONTROL_NOP;
            if (CTRL.cblock.control_word & BC_HWCONTROL_MFRAMEBEG)
               api_message->control             |= BC_CONTROL_MFRAME_BEG;
            if (CTRL.cblock.control_word & BC_HWCONTROL_MFRAMEEND)
               api_message->control             |= BC_CONTROL_MFRAME_END;
            if (CTRL.cblock.control_word & BC_HWCONTROL_SET_TNOP)
               api_message->control             |= BC_CONTROL_TIMED_NOP;
         }
         if(CTRL.cblock.control_word == BC_HWCONTROL_SET_TNOP)
         {
            api_message->long_gap = CTRL.cblock.gap_time;
            if(api_message->long_gap < 0x10000)
               api_message->gap_time = (BT_U16BIT)(api_message->long_gap &0x0000ffff); //Set gap_time as well
         }
         return API_SUCCESS;

      /************************************************
      *  Conditional branch
      ************************************************/
      case 0x0006:       /* Conditional Branch */
         if (api_message->messno == 0)
            return API_BC_MESS1_COND;

         api_message->time_tag = MDATA.cdblock.timetag;

         vbtReadRelRAM32[cardnum](cardnum,&id,CTRL.cblock.branch_msg_ptr,1);
         api_message->messno_branch = (BT_U16BIT)id;

         if ( CTRL.cblock.addr_next == 0 )
            api_message->messno_next   = 0xFFFF;
         else
         {
            vbtReadRelRAM32[cardnum](cardnum,&id,CTRL.cblock.addr_next,1);
            api_message->messno_next   = (BT_U16BIT)id;
         }

         api_message->data_value    = MDATA.cdblock.data_pattern;
         api_message->data_mask     = MDATA.cdblock.bit_mask;

         /* Read back the conditional counter values.V4.39.ajh */
         api_message->cond_count_val = MDATA.cdblock.cond_count_val;
         api_message->cond_counter   = MDATA.cdblock.cond_counter;

         addr = MDATA.cdblock.tst_wrd_addr;      
         addr_prev = mblock_addr[cardnum][api_message->messno-1];

         if (addr < addr_prev)
         {
            api_message->control    = BC_CONTROL_CONDITION2;
            api_message->address    = (BT_U16BIT)(addr);

            api_message->test_address = MDATA.cdblock.tst_wrd_addr;

         }
         else  //
         {
            api_message->control    = BC_CONTROL_CONDITION;
            if (addr == addr_prev + BC_MSG_CMD1_OFFSET)
               wordno = 0;
            else if (addr == addr_prev + BC_MSG_CMD2_OFFSET)
               wordno = 1;
            else if (addr == addr_prev + BC_DATA_MSTAT1_OFFSET)
               wordno = 2;
            else if (addr == addr_prev + BC_DATA_MSTAT2_OFFSET)
               wordno = 3;
            else
               wordno = 4 + (addr - (addr_prev + BC_DATA_DATA_OFFSET));   //changed for v6

            if (wordno >= BC_CTRLDATA_RESERVED)
               return API_BC_BAD_COND_ADDR;

            api_message->address = (BT_U16BIT)wordno;
         }
         return API_SUCCESS;

      /****************************************************************
      *   Last message in list(Stop BC message)
      ****************************************************************/

      case 0x0004:       /* Stop BC message */
         api_message->control = BC_CONTROL_LAST;
         api_message->time_tag = MDATA.cdblock.timetag;
         return API_SUCCESS;
   }
   return API_BC_ILLEGALMESSAGE;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageReadData()
*
*  FUNCTION
*     This routine reads the current data area of the specified message
*     block.  The data area to be read is determined by the current the last
*     data buffer that was transacted. 
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageReadData(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) number of the BC message
   BT_U16BIT * buffer)      // (o) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MSGBUF  mblock;    // Used to read v6 the BC message to get data buf ptrs
   BC_MESSAGE cblock;    // Used to read v5 the BC message to get data buf ptrs
   int        wCount;    // Number of data words in message

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Get message block from hardware
   *******************************************************************/
   if(board_is_v5_uca[cardnum])
   {
      addr = BC_MBLOCK_ADDR(cardnum, messageid);
      vbtReadRAM[cardnum](cardnum,(BT_U16BIT *)&cblock,addr,6*2);

      if ( (cblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
         return API_BC_UPDATEMESSTYPE;

      /* Extract the number of words to read */
      wCount = cblock.mess_command1.wcount;
      if ( wCount == 0 )
         wCount = 32;
   }
   else
   {
      addr = mblock_addr[cardnum][messageid];
      vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,addr,BC_MSGBUF_DWSIZE);

      if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

      /* Extract the number of words to read */
      wCount = mblock.mess_command1.wcount;
      if ( wCount == 0 )
         wCount = 32;
   }

   /*******************************************************************
   *  Read data from the data buffer which is in use.
   *******************************************************************/
   if(board_is_v5_uca[cardnum])
   {
      if (cblock.control_word & BC_HWCONTROL_BUFFERA)
         daddr = ((BT_U32BIT)cblock.addr_data1) << hw_addr_shift[cardnum];
      else
         daddr = ((BT_U32BIT)cblock.addr_data2) << hw_addr_shift[cardnum];
      /* Read the specified data buffer, using the specified word count */
      vbtReadRAM[cardnum](cardnum, buffer, daddr, wCount*2);
   }
   else
   {
      daddr = mblock.data_addr;
 
      /* Read the specified data buffer, using the specified word count */
     vbtReadRelRAM[cardnum](cardnum, (BT_U16BIT *)buffer, daddr + BC_DATA_DATA_OFFSET, wCount);
   }

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageReadDataBuffer()
*
*  FUNCTION
*     This routine reads the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the buffer parameter passed.
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageReadDataBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) number of the BC message
   BT_UINT   buffer_id,     // (i) Buffer ID 0 - n 
   BT_U16BIT * buffer)      // (o) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MSGBUF  mblock;    // Used to read the v6 BC message; to get data buf ptrs
   BC_MESSAGE cblock;    // Used to read the v5 BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(buffer_id > 1)
      return API_BAD_PARAM;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   if(board_is_v5_uca[cardnum])
   {
      addr = BC_MBLOCK_ADDR(cardnum, messageid);
      vbtReadRAM[cardnum](cardnum,(BT_U16BIT *)&cblock,addr,6*2);

      if ( (cblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
        return API_BC_UPDATEMESSTYPE;

      /* Extract the number of words to read */
      wCount = cblock.mess_command1.wcount;
   }
   else
   {
      addr = mblock_addr[cardnum][messageid];
      vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,addr,BC_MSGBUF_DWSIZE);
  
      if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
        return API_BC_UPDATEMESSTYPE;

     if( buffer_id > mblock.num_data_buffers)
        return API_BC_MBLOCK_NOMATCH;

     /* Extract the number of words to read */
     wCount = mblock.mess_command1.wcount;
   }

   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  Read data from the data buffer which is in use.
   *  If there is only one data buffer, both pointers point to it.
   *******************************************************************/
   if(board_is_v5_uca[cardnum])
   {
      if (buffer_id==0)
         daddr = ((BT_U32BIT)cblock.addr_data1) << hw_addr_shift[cardnum];
      else
         daddr = ((BT_U32BIT)cblock.addr_data2) << hw_addr_shift[cardnum];
      /* Read the specified data buffer, using the specified word count */
      vbtReadRAM[cardnum](cardnum, buffer, daddr, wCount*2);
   }
   else
   {
      daddr = addr + BC_MSGBUF_SIZE + (buffer_id * BC_MSGDATA_SIZE);
      /* Read the specified data buffer, using the specified word count */
      vbtReadRAM[cardnum](cardnum, buffer, daddr + BC_DATA_DATA_OFFSET, wCount);
   }

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_ReadDataBuffer() V6 Only
*
*  FUNCTION
*     This routine reads the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the buffer parameter passed.
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_ReadDataBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bufaddr,       // (i) Buffer address 
   BT_U16BIT * buffer)      // (o) pointer to user's data buffer
{
   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   vbtReadRAM[cardnum](cardnum,buffer,bufaddr+BC_DATA_DATA_OFFSET,32);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_SelectBufferRead() V6 Only
*
*  FUNCTION
*     This routine reads the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the buffer parameter passed.
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_SelectBufferRead(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_UINT buf_num,       // (i) Data buffer number
   BT_U16BIT * buffer)      // (i) Pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    status;    // Status return from called functions
   CEI_NATIVE_ULONG buf_addr;
   CEI_NATIVE_ULONG msg_addr;
   BT_U32BIT control, num_buf;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (messno >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   msg_addr = mblock_addr[cardnum][messno];
   vbtReadRAM32[cardnum](cardnum,&control,msg_addr+BC_CTLWD_OFFSET,1);
   vbtReadRAM32[cardnum](cardnum,&num_buf,msg_addr+BC_NUM_BUFFERS_OFFSET,1);

   if(buf_num >= num_buf)
      return API_BC_BAD_DATA_BUFFER;
   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (control & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   buf_addr = msg_addr + BC_MSGBUF_SIZE + (buf_num * BC_MSGDATA_SIZE); 

   /* Update the specified data buffer, using the specified word count */
   vbtReadRAM[cardnum](cardnum, (BT_U16BIT *)buffer, buf_addr+BC_DATA_DATA_OFFSET, 32);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_GetBufferCount() V6 Only
*
*  FUNCTION
*     This routine reads the data area of the specified message
*     block.  The data area to be read (there are two for each message
*     block) is determined by the buffer parameter passed.
*
*     If both message data buffer pointers are the same, just read
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_GetBufferCount(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_U32BIT * count)         // (i) Pointer to count
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    status;    // Status return from called functions
   CEI_NATIVE_ULONG msg_addr;
   BT_U32BIT control;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (messno >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   msg_addr = mblock_addr[cardnum][messno];
   vbtReadRAM32[cardnum](cardnum,&control,msg_addr+BC_CTLWD_OFFSET,1);
   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (control & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   vbtReadRAM32[cardnum](cardnum,count,msg_addr+BC_NUM_BUFFERS_OFFSET,1);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - v5_BC_MessageWrite()
*
*  FUNCTION
*     This routine is used to write the specified BC Message Block to the
*     board.  The data is retrieved from the caller supplied structure.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*     API_HARDWARE_NOSUPPORT  -> IP does not support conditional messages
*
****************************************************************************/
NOMANGLE BT_INT CCONV v5_BC_MessageWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   API_BC_MBUF * api_message)  // (i) pointer to user's buffer containing msg
{
   /*******************************************************************
   *  Local variables
   ********************************************************************/
   BT_UINT    next_messno;  /* Next message's message number          */
   BT_UINT    id;           /* Compare or branch message number       */
   BT_UINT    messtype;     /* Message type to be generated           */
   BT_UINT    wordno;
   BT_U32BIT  addr;         /* General purpose byte address           */
   BT_U32BIT  bc_msg_addr;  /* Byte address of the current BC message block */
   BT_U32BIT  data_addr1;   /* First BC message data buffer byte address    */
   BT_U32BIT  data_addr2;   /* Second BC message data buffer byte address   */

   BC_MESSAGE mblock;       /* BC Message block structure              */
   union{
     BC_CBUF         *cpbuf;       // Pointer used to ref msg as an IP conditional
     BC_MESSAGE      *mpbuf;
   }bcptr;
   BC_CBUF    *cbuf;        /* Control message block structure pointer */
   BT_U32BIT  gap_time;     /* Local corrected gap time                */

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   {
      BT_UINT    status;       /* Status return from called functions   */
      if ( pUsrBC_MessageWrite[cardnum] )
      {
         status = (*pUsrBC_MessageWrite[cardnum])(cardnum, &mblock_id, api_message);
         if ( status == API_RETURN_SUCCESS )
            return API_SUCCESS;
         else if ( status == API_NEVER_CALL_AGAIN )
            pUsrBC_MessageWrite[cardnum] = NULL;
         else if ( status != API_CONTINUE )
            return status;
      }
   }
#endif

   /*******************************************************************
   *  We check the intermessage gap time or absolute time
   *******************************************************************/

   gap_time = api_message->gap_time;

   if(board_using_extended_timing[cardnum] == 0)
   {
      if(gap_time == 0) //if the gap is not set, set to default of 15 usec
         gap_time = 15;
   }

   /* Caluclate the Data Buffer addresses                             */
   bc_msg_addr = BC_MBLOCK_ADDR(cardnum, mblock_id);

   memset((char *)&mblock, 0, sizeof(mblock));

   data_addr2 = data_addr1 = bc_msg_addr + sizeof(BC_MESSAGE);
   if ( bc_num_dbufs[cardnum] != 1 ) /* Not single buffered. */
      data_addr2 += bc_size_dbuf[cardnum];
   mblock.addr_data1 = (BT_U16BIT)(data_addr1 >> hw_addr_shift[cardnum]);
   mblock.addr_data2 = (BT_U16BIT)(data_addr2 >> hw_addr_shift[cardnum]);

   /*******************************************************************
   *  Figure out what kind of message this is, based on the 3 bit field.
   *******************************************************************/
   messtype = api_message->control & BC_CONTROL_TYPEMASK;

   if (messtype == BC_CONTROL_MESSAGE || messtype == BC_CONTROL_MSG_NOP)
   {
      /* Check for illegal bits */
      if ((api_message->control & BC_CONTROL_BUFFERA) &&
          (api_message->control & BC_CONTROL_BUFFERB))
         return API_BC_BOTHBUFFERS;
      if ((api_message->control & BC_CONTROL_CHANNELA) &&
          (api_message->control & BC_CONTROL_CHANNELB))
         return API_BC_BOTHBUSES;

      if(messtype == BC_CONTROL_MSG_NOP)
      {
         if(api_message->control & BC_CONTROL_TIMED_NOP) //make this a timed noop
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP | BC_HWCONTROL_SET_TNOP;    // 1553 message Noop
         else
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP;
      }
      else  
         mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_OP;    // 1553 message

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEBEG;
      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEEND;
      if (api_message->control & BC_CONTROL_RTRTFORMAT)
         mblock.control_word        |= 0x0020;
      if (api_message->control & BC_CONTROL_RETRY)
         mblock.control_word        |= 0x0400;
      if (api_message->control & BC_CONTROL_INTERRUPT)
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
      if (api_message->control & BC_CONTROL_INTQ_ONLY)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }
      if (api_message->control & BC_CONTROL_BUFFERA)
         mblock.control_word        |= BC_HWCONTROL_BUFFERA;
      if (api_message->control & BC_CONTROL_BUFFERB)
         mblock.control_word        |= BC_HWCONTROL_BUFFERB;

      if (api_message->control & BC_CONTROL_CHANNELA)
         mblock.control_word        |= BC_HWCONTROL_CHANNELA;
      if (api_message->control & BC_CONTROL_CHANNELB)
         mblock.control_word        |= BC_HWCONTROL_CHANNELB;

      mblock.mess_command1 = api_message->mess_command1; // All messages
      mblock.mess_command2 = api_message->mess_command2; // Only for RT-RT msgs
      
      if(board_using_extended_timing[cardnum])
      {
         if(api_message->long_gap == 0)
            api_message->long_gap = (BT_U32BIT)gap_time;
         mblock.gap_time = (BT_U16BIT)(api_message->long_gap & 0xffff);
         mblock.gap_time2 = (BT_U16BIT)((api_message->long_gap & 0x00ff0000)>> 16);        
      }
      else
         mblock.gap_time = api_message->gap_time;

      mblock.mess_status1  = api_message->mess_status1;
      mblock.mess_status2  = api_message->mess_status2;
      //mblock.status        = api_message->status;
      api_message->status  = (((BT_U32BIT)mblock.mstatus[1])<<16) | mblock.mstatus[0];

      addr = BTMEM_EI + api_message->errorid * sizeof(EI_MESSAGE);
      mblock.addr_error_inj = (BT_U16BIT)(addr >> 1); // EI pointers are word addrs

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      mblock.timetag.microseconds=0x0;
      mblock.timetag.topuseconds=0x0;

      if(board_using_msg_schd[cardnum])
      {
         mblock.addr_data2 = api_message->rep_rate;
         mblock.start_frame = api_message->start_frame;
      }

      /* Data buffer pointers already setup by BusTools_MessageAlloc() */
      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      /* Write the first data buffer to the board  */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)(api_message->data[0]),data_addr1,2*BT1553_BUFCOUNT);
      else
         vbtWrite(cardnum,(LPSTR)(api_message->data[0]),data_addr1,2*BT1553_BUFCOUNT);

      /* If buffers are different, write the second data buffer to the board */
      //if ( data_addr1 != data_addr2 ) /* Not single buffered. */
      if(bc_num_dbufs[cardnum] != 1)
      {
         if(board_access_32[cardnum])
            vbtWrite32(cardnum,(LPSTR)(api_message->data[1]),data_addr2,(2*BT1553_BUFCOUNT));//round up to 
         else
            vbtWrite(cardnum,(LPSTR)(api_message->data[1]),data_addr2,2*BT1553_BUFCOUNT);
      }

      return API_SUCCESS;
   }

   /*******************************************************************
   *   Handle last message entry which generates a BC Stop message.
   *******************************************************************/
   if ( messtype == BC_CONTROL_LAST )
   {
      mblock.control_word = BC_HWCONTROL_LASTMESS | BC_HWCONTROL_OP;  /* Stop BC message (last message block) */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word |= BC_HWCONTROL_INTQ_ONLY;
      }

	  if(api_message->control & BC_CONTROL_EXT_SYNC)  /* This is from BC_CONTROL_HALT */
	     mblock.control_word |= BC_CLR_EXT_SYNC;

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if (next_messno >= bc_mblock_count[cardnum])
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle timed no-op message 
   *******************************************************************/
   if (api_message->control & BC_CONTROL_TIMED_NOP)
   {
      mblock.control_word = BC_HWCONTROL_SET_TNOP;         /* Timed Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word |= BC_HWCONTROL_MFRAMEBEG;  /* V4.01 */

      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word |= BC_HWCONTROL_MFRAMEEND;  /* V2.41 */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }
      mblock.gap_time      = (BT_U16BIT)gap_time; // Set the gap time for this noop
      next_messno = api_message->messno_next;

      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      if(board_using_msg_schd[cardnum])
      {
         mblock.addr_data2 = api_message->rep_rate;
         mblock.start_frame = api_message->start_frame;
      }

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle no-op message and unconditional branch to specified msg.
   *  Both generate a Noop message, which can have the End Minor Frame 
   *  bit set.
   *******************************************************************/
   if ((messtype == BC_CONTROL_NOP) || (messtype == BC_CONTROL_BRANCH))
   {
      mblock.control_word = BC_HWCONTROL_NOP;         /* Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word |= BC_HWCONTROL_MFRAMEBEG;  /* V4.01 */

      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word |= BC_HWCONTROL_MFRAMEEND;  /* V2.41 */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
      }

      /* Write the completed message to the board.                     */
      if(board_access_32[cardnum])
         vbtWrite32(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));
      else
         vbtWrite(cardnum,(LPSTR)&mblock,bc_msg_addr,sizeof(mblock));

      return API_SUCCESS;
   }

   /*******************************************************************
   *   BC Write Message:
   *   Handle conditional messages (three kinds).
   *   All generate the same Conditional Message.
   *******************************************************************/
   if ( mblock_id == 0 )
      return API_BC_MESS1_COND;     /* First message cannot be a conditional. */

   bcptr.mpbuf = &mblock;
   cbuf = bcptr.cpbuf;
   
   cbuf->control_word = 0x0006 | 1;    /* Conditional message */
   if (api_message->control & BC_CONTROL_INTERRUPT) /* add interrupt */
         cbuf->control_word |= BC_HWCONTROL_INTERRUPT;

   /* Move the fixed data into the hardware-image scratch buffer. */
   cbuf->data_pattern = (BT_U16BIT)api_message->data_value;
   cbuf->bit_mask     = (BT_U16BIT)api_message->data_mask;
   /* Move the conditional count values.V4.39.ajh */
   cbuf->cond_count_val = (BT_U16BIT)api_message->cond_count_val;
   cbuf->cond_counter   = (BT_U16BIT)api_message->cond_counter;

   id = api_message->messno_branch;
   if (id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_BRANCH; 
   cbuf->branch_msg_ptr = (BT_U16BIT)
                        (BC_MBLOCK_ADDR(cardnum, id) >> hw_addr_shift[cardnum]);

   next_messno = api_message->messno_next;
   if ( next_messno == 0xFFFF ) /* Check for special end-of-aperiodic msg list */
      cbuf->addr_next = 0;
   else
   {
      if ( next_messno >= bc_mblock_count[cardnum] )
         return API_BC_ILLEGAL_NEXT;
      cbuf->addr_next = (BT_U16BIT)
               (BC_MBLOCK_ADDR(cardnum, next_messno) >> hw_addr_shift[cardnum]);
   }

   if ( (messtype == BC_CONTROL_CONDITION) ||
        (messtype == BC_CONTROL_CONDITION3) ) /* Bug Report A-000040.ajh */
   {
      /********************************************************************
      *  Type 1 or Type 3 Conditional Branch.  Set up address of compare,
      *   using the specified word number, relative to the previous or
      *    the specified message.
      ********************************************************************/
      wordno = api_message->address;
      if ( wordno > 35 )
         return API_BC_BAD_COND_ADDR;

      if (messtype == BC_CONTROL_CONDITION3)  /* Bug Report A-000040.ajh */
      {
         /* Fetch the address of the specified message. */
         id = api_message->messno_compare;
         if (id >= bc_mblock_count[cardnum])
            return API_BC_ILLEGALTARGET;
         addr = BC_MBLOCK_ADDR(cardnum, id);
      }
      else
      {
         /* Fetch the address of the previous message. */
         addr = BC_MBLOCK_ADDR(cardnum, mblock_id-1);
      }
      /* Adjust the address of previous msg, to specified word. */
      switch ( wordno )
      {
         case 0:    /* Command word of previous or specified message */
            addr += (BT_U32BIT)((char*)&(mblock.mess_command1) - (char*)&mblock);
            break;
         case 1:    /* Command word #2 (RT-RT msgs only) of previous or specified message */
            addr += (BT_U32BIT)((char*)&(mblock.mess_command2) - (char*)&mblock);
            break;
         case 2:    /* Status word of previous or specified message */
            addr += (BT_U32BIT)((char*)&(mblock.mess_status1) - (char*)&mblock);
            break;
         case 3:    /* Status word #2 (RT-RT msgs only) of previous or specified message */
            addr += (BT_U32BIT)((char*)&(mblock.mess_status2) - (char*)&mblock);
            break;
         default:   /* Data words #1 through #32 of first data buffer */
            addr += sizeof(BC_MESSAGE) + 2 * (wordno-4);
            break;
      }

      // Convert byte address to word address and split into components.
      cbuf->tst_wrd_addr1 = (BT_U16BIT)((addr >> 4)       );  // V3.30.ajh
      cbuf->tst_wrd_addr2 = (BT_U16BIT)((addr >> 1) & 0x07);
   }
   else if ( messtype == BC_CONTROL_CONDITION2 )
   {
      /* Compare address is directly specified by caller. */
      
      // Convert byte address to word address and split into components.
      cbuf->tst_wrd_addr1 = (BT_U16BIT)((api_message->test_address >> 4)       );// V3.30.ajh
      cbuf->tst_wrd_addr2 = (BT_U16BIT)((api_message->test_address >> 1) & 0x07);
   }

   /* Write the completed message to the board.                     */
   if(board_access_32[cardnum])
      vbtWrite32(cardnum,(LPSTR)cbuf,bc_msg_addr,sizeof(BC_CBUF));
   else
      vbtWrite(cardnum,(LPSTR)cbuf,bc_msg_addr,sizeof(BC_CBUF));

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - v6_BC_MessageWrite()
*
*  FUNCTION
*     This routine is used to write the specified BC Message Block to the
*     board.  The data is retrieved from the caller supplied structure.  If the
*    application has specified multiple BC buffers, this function write the 
*    command information and data for buffer 0.  If additional data buffers are
*    specified use BusTools_BC_DataBufferWrite to fill in the remaining data
*    buffers.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*     API_HARDWARE_NOSUPPORT  -> IP does not support conditional messages
*
****************************************************************************/
NOMANGLE BT_INT CCONV v6_BC_MessageWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   API_BC_MBUF * api_message)  // (i) pointer to user's buffer containing msg
{
   /*******************************************************************
   *  Local variables
   ********************************************************************/
   BT_UINT    next_messno;   /* Next message's message number          */
   BT_UINT    id;            /* Compare or branch message number       */
   BT_UINT    messtype;      /* Message type to be generated           */
   BT_UINT    wordno;
   BT_INT     i, count;
   BT_U32BIT  addr,daddr;    /* General purpose byte address           */
   BT_U32BIT  bc_msg_addr;   /* Byte address of the current BC message block */
   BT_U32BIT  daddrA;        /* First BC message data buffer byte address    */
   BT_U32BIT  daddrB;        /* Second BC message data buffer byte address   */
   BT_U32BIT  data_addr;     /*  Saves the data address                       */ 

   BC_MSGBUF   mblock;       /* BC Message block structure                  */
   BC_MSGDATA  dblock;       /* BC Message data Blcok structure             */
   BC_CTRLBUF  cblock;       /* BC Control Block structure                  */
   BC_CTRLDATA cdblock;      /* BC Control Data structure                   */ 
 
   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /* Get the Data Buffer addresses  */
   bc_msg_addr = mblock_addr[cardnum][messageid];

   vbtReadRAM32[cardnum](cardnum,&data_addr,bc_msg_addr + BC_MSG_DATA_OFFSET,1);

   memset((char *)&mblock,  0, BC_MSGBUF_SIZE);
   memset((char *)&dblock,  0, BC_MSGDATA_SIZE);
   memset((char *)&cblock,  0, BC_CTRLBUF_SIZE);
   memset((char *)&cdblock, 0, BC_CTRLDATA_SIZE);
   
   /*******************************************************************
   *  Figure out what kind of message this is, based on the 3 bit field.
   *******************************************************************/
   messtype = api_message->control & BC_CONTROL_TYPEMASK;

   if (messtype == BC_CONTROL_MESSAGE || messtype == BC_CONTROL_MSG_NOP)
   {
      // Set the data address base on selected bufffer
      daddrA = RAM_ADDR(cardnum,bc_msg_addr + BC_MSGBUF_SIZE);                    //This points to the first buffer (BUFFER A)  
      daddrB = RAM_ADDR(cardnum,bc_msg_addr + BC_MSGBUF_SIZE + BC_MSGDATA_SIZE);  //This points to the second buffer (BUFFER B)
      if (api_message->control & BC_CONTROL_BUFFERB)
      {
         if(bc_num_dbufs[cardnum]==1 || channel_using_multiple_bc_buffers[cardnum])
            return API_BC_BAD_DATA_BUFFER;
         mblock.data_addr = daddrB;  
      } 
      if (api_message->control & BC_CONTROL_BUFFERA)
         mblock.data_addr = daddrA;                    
      
      data_addr = mblock.data_addr;
      mblock.messno = messageid;

      if(channel_using_multiple_bc_buffers[cardnum])
         vbtReadRAM32[cardnum](cardnum,&mblock.num_data_buffers,bc_msg_addr + BC_NUM_BUFFERS_OFFSET,1);
      else
         mblock.num_data_buffers = bc_num_dbufs[cardnum];

      /* Check for illegal bits */
      if ((api_message->control & BC_CONTROL_BUFFERA) &&
          (api_message->control & BC_CONTROL_BUFFERB))
         return API_BC_BOTHBUFFERS;
      if ((api_message->control & BC_CONTROL_BUSA) &&
          (api_message->control & BC_CONTROL_BUSB))
         return API_BC_BOTHBUSES;

      if(messtype == BC_CONTROL_MSG_NOP)
      {
         if(api_message->control & BC_CONTROL_TIMED_NOP) //make this a timed noop
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP | BC_HWCONTROL_SET_TNOP;    // 1553 message Noop
         else
            mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_NOP;
      }
      else  
         mblock.control_word = BC_HWCONTROL_MESSAGE | BC_HWCONTROL_OP;    // 1553 message

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEBEG;
      if (api_message->control & BC_CONTROL_MFRAME_END)
         mblock.control_word        |= BC_HWCONTROL_MFRAMEEND;
      if (api_message->control & BC_CONTROL_RTRTFORMAT)
         mblock.control_word        |= 0x0020;
      if (api_message->control & BC_CONTROL_RETRY)
         mblock.control_word        |= 0x0400;
      if (api_message->control & BC_CONTROL_INTERRUPT)
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
      if (api_message->control & BC_CONTROL_INTQ_ONLY)
      {
         mblock.control_word        |= BC_HWCONTROL_INTERRUPT;
         mblock.control_word        |= BC_HWCONTROL_INTQ_ONLY;
      }

      if (api_message->control & BC_CONTROL_CHANNELA)
         mblock.control_word        |= BC_HWCONTROL_CHANNELA;
      if (api_message->control & BC_CONTROL_CHANNELB)
         mblock.control_word        |= BC_HWCONTROL_CHANNELB;

      mblock.mess_command1 = api_message->mess_command1; // All messages
      mblock.mess_command2 = api_message->mess_command2; // Only for RT-RT msgs
      flip((BT_U32BIT *)&mblock.mess_command1);

      if(api_message->long_gap == 0)
         api_message->long_gap = api_message->gap_time;

      mblock.gap_time = api_message->long_gap;   

      dblock.mess_status1  = api_message->mess_status1;
      dblock.mess_status2  = api_message->mess_status2;
      flip((BT_U32BIT *)&dblock.mess_status1);

      mblock.addr_error_inj = RAM_ADDR(cardnum,(BTMEM_EI_V6 + api_message->errorid * sizeof(EI_MESSAGE)));

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         mblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         mblock.addr_next = RAM_ADDR(cardnum,mblock_addr[cardnum][next_messno]);
      }

      dblock.timetag.microseconds=0x0;
      dblock.timetag.topuseconds=0x0;

      mblock.rep_rate = api_message->rep_rate;
      mblock.start_frame = api_message->start_frame;
      flip((BT_U32BIT *)&mblock.rep_rate);

      /* Write the completed message to the board.                     */
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,bc_msg_addr,wsizeof(mblock));

      /* Write the data buffer to the board  */
      dblock.msg_addr = RAM_ADDR(cardnum,bc_msg_addr);
      if(channel_using_multiple_bc_buffers[cardnum])
      {
         vbtReadRelRAM32[cardnum](cardnum,&dblock.next_data,data_addr + BC_DATA_NEXT_OFFSET,1);
      }
      else
         dblock.next_data = mblock.data_addr;

      count = mblock.mess_command1.wcount;
      if(count == 0)
         count = 32;     

      for(i=0; i < count; i++)
      {
         dblock.data_word[i] = ((BT_U16BIT *)(api_message->data[0]))[i];
      }  

      vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&dblock,daddrA,6);//BC_MSGDATA_DWSIZE);
      vbtWriteRelRAM[cardnum](cardnum,(BT_U16BIT *)&dblock.data_word,daddrA+BC_DATA_DATA_OFFSET,34);
      if(bc_num_dbufs[cardnum]==2)
      {
         for(i=0; i < count; i++)
         {
            dblock.data_word[i] = ((BT_U16BIT *)(api_message->data[1]))[i];
         }
         vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&dblock,daddrB,6);//BC_MSGDATA_DWSIZE);
         vbtWriteRelRAM[cardnum](cardnum,(BT_U16BIT *)&dblock.data_word,daddrB+BC_DATA_DATA_OFFSET,34);
      }
      return API_SUCCESS;
   }
   
   cblock.data_addr = data_addr;   //This write data to the start data address only
   cblock.messno = messageid;
   if(channel_using_multiple_bc_buffers[cardnum])
      vbtReadRAM32[cardnum](cardnum,&cblock.num_data_buffers,bc_msg_addr + BC_NUM_BUFFERS_OFFSET,1);
   else
      cblock.num_data_buffers = bc_num_dbufs[cardnum];

   /*******************************************************************
   *   Handle last message entry which generates a BC Stop message.
   *******************************************************************/
   if ( messtype == BC_CONTROL_LAST )
   {
      cblock.control_word = BC_HWCONTROL_LASTMESS | BC_HWCONTROL_OP;  /* Stop BC message (last message block) */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         cblock.control_word |= BC_HWCONTROL_INTERRUPT;
      }

	  if(api_message->control & BC_CONTROL_EXT_SYNC)  /* This is from BC_CONTROL_HALT */
	     cblock.control_word |= BC_CLR_EXT_SYNC;

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         cblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         cblock.addr_next = RAM_ADDR(cardnum,mblock_addr[cardnum][next_messno]);
      }

      /* Write the completed message to the board.                     */
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&cblock,bc_msg_addr,BC_CTRLBUF_DWSIZE);

      cdblock.next_data = data_addr;
      cdblock.msg_addr = RAM_ADDR(cardnum,bc_msg_addr);
      vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&cdblock,data_addr,BC_CTRLDATA_DWSIZE);

      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle timed no-op message 
   *******************************************************************/
   if (api_message->control & BC_CONTROL_TIMED_NOP)
   {
      cblock.control_word = BC_HWCONTROL_SET_TNOP;         /* Timed Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         cblock.control_word |= BC_HWCONTROL_MFRAMEBEG;  /* V4.01 */

      if (api_message->control & BC_CONTROL_MFRAME_END)
         cblock.control_word |= BC_HWCONTROL_MFRAMEEND;  /* V2.41 */

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         cblock.control_word        |= BC_HWCONTROL_INTERRUPT;
      }

      if(api_message->long_gap == 0)
         api_message->long_gap = api_message->gap_time;

      cblock.gap_time = api_message->long_gap;; // Set the gap time for this noop
      next_messno = api_message->messno_next;

      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         cblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         cblock.addr_next = RAM_ADDR(cardnum,mblock_addr[cardnum][next_messno]);
      }

      /* Write the completed message to the board.                     */
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&cblock,bc_msg_addr,BC_MSGBUF_DWSIZE);

      cdblock.next_data = data_addr; 
      cdblock.msg_addr = RAM_ADDR(cardnum,bc_msg_addr);
      vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&cdblock,data_addr,BC_CTRLDATA_DWSIZE);

      return API_SUCCESS;
   }

   /*******************************************************************
   *  BC Write Message:
   *  Handle no-op message and unconditional branch to specified msg.
   *  Both generate a Noop message, which can have the End Minor Frame 
   *  bit set.
   *******************************************************************/
   if ((messtype == BC_CONTROL_NOP) || (messtype == BC_CONTROL_BRANCH))
   {
      cblock.control_word = BC_HWCONTROL_NOP;         /* Noop message */

      if (api_message->control & BC_CONTROL_MFRAME_BEG)
         cblock.control_word |= BC_HWCONTROL_MFRAMEBEG;

      if (api_message->control & BC_CONTROL_MFRAME_END)
         cblock.control_word |= BC_HWCONTROL_MFRAMEEND; 

      if (api_message->control & BC_CONTROL_INTERRUPT)
      {
         cblock.control_word |= BC_HWCONTROL_INTERRUPT;
      }

      next_messno = api_message->messno_next;
      if ( next_messno == 0xFFFF )  /* Check for special end-of-aperiodic msg list */
         cblock.addr_next = 0;
      else
      {
         if ( next_messno >= bc_mblock_count[cardnum] )
            return API_BC_ILLEGAL_NEXT;
         cblock.addr_next = RAM_ADDR(cardnum,mblock_addr[cardnum][next_messno]);
      }

      /* Write the completed message to the board.                     */
      vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&cblock,bc_msg_addr,BC_MSGBUF_DWSIZE);

      cdblock.next_data = data_addr; 
      cdblock.msg_addr  = RAM_ADDR(cardnum,bc_msg_addr);
      vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&cdblock,data_addr,BC_CTRLDATA_DWSIZE);

      return API_SUCCESS;
   }

   /*******************************************************************
   *   BC Write Message:
   *   Handle conditional messages (three kinds).
   *   All generate the same Conditional Message.
   *******************************************************************/
   if ( messageid == 0 )
      return API_BC_MESS1_COND;     /* First message cannot be a conditional. */
   
   cblock.control_word = 0x0006 | 1;    /* Conditional message */
   if (api_message->control & BC_CONTROL_INTERRUPT) /* add interrupt */
         cblock.control_word |= BC_HWCONTROL_INTERRUPT;

   /* Move the fixed data into the hardware-image scratch buffer. */
   cdblock.data_pattern = api_message->data_value;
   cdblock.bit_mask     = api_message->data_mask;
   /* Move the conditional count values.V4.39.ajh */
   cdblock.cond_count_val = api_message->cond_count_val;
   cdblock.cond_counter   = api_message->cond_counter;
   cdblock.next_data = data_addr; 
   cdblock.msg_addr  = RAM_ADDR(cardnum,bc_msg_addr);

   id = api_message->messno_branch;
   if (id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_BRANCH; 
   cblock.branch_msg_ptr = RAM_ADDR(cardnum,mblock_addr[cardnum][id]);

   next_messno = api_message->messno_next;
   if ( next_messno == 0xFFFF ) /* Check for special end-of-aperiodic msg list */
     cblock.addr_next = 0;
   else
   {
      if ( next_messno >= bc_mblock_count[cardnum] )
         return API_BC_ILLEGAL_NEXT;
      cblock.addr_next = RAM_ADDR(cardnum,mblock_addr[cardnum][next_messno]);
   }

   if ( (messtype == BC_CONTROL_CONDITION) ||
        (messtype == BC_CONTROL_CONDITION3) ) 
   {
      /********************************************************************
      *  Type 1 or Type 3 Conditional Branch.  Set up address of compare,
      *   using the specified word number, relative to the previous or
      *    the specified message.
      ********************************************************************/
      wordno = api_message->address;
      if ( wordno > 35 && wordno != 50 )
         return API_BC_BAD_COND_ADDR;

      if (messtype == BC_CONTROL_CONDITION3)  
      {
         /* Fetch the address of the specified message. */
         id = api_message->messno_compare;
         if (id >= bc_mblock_count[cardnum])
            return API_BC_ILLEGALTARGET;
         addr = RAM_ADDR(cardnum,mblock_addr[cardnum][id]);

      }
      else
      {
         /* Fetch the address of the previous message. */
         addr = RAM_ADDR(cardnum,mblock_addr[cardnum][messageid-1]);
      }
      vbtReadRelRAM32[cardnum](cardnum,&daddr,addr + BC_MSG_DATA_OFFSET,1);

      /* Adjust the address of previous msg, to specified word. */
      switch ( wordno )
      {
         case API_BC_BRANCH_CMD1:    /* Command word of previous or specified message */
            addr += BC_MSG_CMD1_OFFSET;
            cdblock.tst_wrd_addr = addr;
            break;
         case API_BC_BRANCH_CMD2:    /* Command word #2 (RT-RT msgs only) of previous or specified message */
            addr += BC_MSG_CMD1_OFFSET;  // Need to use CMD1 for even word alignment
            cdblock.tst_wrd_addr = addr;
            cdblock.bit_mask = cdblock.bit_mask<<16;
            cdblock.data_pattern = cdblock.data_pattern<<16;
            break;
         case API_BRANCH_MSTAT1:    /* Status word of previous or specified message */
            daddr += BC_DATA_MSTAT1_OFFSET;
            cdblock.tst_wrd_addr = daddr;
            break;
         case API_BC_BRANCH_MSTAT2:    /* Status word #2 (RT-RT msgs only) of previous or specified message */
            daddr += BC_DATA_MSTAT1_OFFSET;
            cdblock.tst_wrd_addr = daddr;
            cdblock.bit_mask = cdblock.bit_mask<<16;
            cdblock.data_pattern = cdblock.data_pattern<<16;
            break;
         case API_BC_BRANCH_INT_STATUS: 
             daddr += BC_DATA_STATUS_OFFSET;
             cdblock.tst_wrd_addr = daddr;
             break;
         default:   /* Data words #1 through #32 of first data buffer */
            daddr += BC_DATA_DATA_OFFSET + (2 * (wordno - API_BC_BRANCH_DATA));
            if((wordno - API_BC_BRANCH_DATA) % 2)
            {
               cdblock.bit_mask = cdblock.bit_mask<<16;
               cdblock.data_pattern = cdblock.data_pattern<<16;
            }
            cdblock.tst_wrd_addr = daddr;
            break;
      }
   }
   else if ( messtype == BC_CONTROL_CONDITION2 )
   {
      /* Compare address is directly specified by caller. */
      cdblock.tst_wrd_addr = RAM_ADDR(cardnum,api_message->test_address);
   }

   /* Write the completed message to the board.                     */
   vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&cblock,bc_msg_addr,BC_CTRLBUF_DWSIZE);
   vbtWriteRelRAM32[cardnum](cardnum,(BT_U32BIT *)&cdblock,data_addr, BC_CTRLDATA_DWSIZE);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageWrite()
*
*  FUNCTION
*     This routine is used to write the specified BC Message Block to the
*     board.  The data is retrieved from the caller supplied structure.  If the
*    application has specified multiple BC buffers, this function write the 
*    command information and data for buffer 0.  If additional data buffers are
*    specified use BusTools_BC_DataBufferWrite to fill in the remaining data
*    buffers.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*     API_HARDWARE_NOSUPPORT  -> IP does not support conditional messages
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   API_BC_MBUF * api_message)  // (i) pointer to user's buffer containing msg
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_MessageWrite[cardnum](cardnum,messageid,api_message);
}

/****************************************************************************
*
*  PROCEDURE NAME - v6_BC_MessageUpdate()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block.  The data area to be updated
*     (there are two for each message block) is determined
*     by the current state of the Ping-Pong value in the
*     BC Message Block Control Word.  If data buffer A is
*     currently being used, then buffer B is updated.  Likewise,
*     if data buffer B is currently being used, then buffer
*     A is updated.  After the data area is updated, the buffer
*     bit in the Control Word is updated.  The next time this
*     message is sent by the BC, it will use the new data.
*
*     If both message data buffer pointers are the same, just update
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
* 01/19/1998 Improve speed by only reading message buffer up to the
*            data buffer pointers.
* 07/07/1998 Improve speed by only writing number of words specified
*            by the 1553 command word.
* 11/20/1998 If single buffered, just output the data words.
*****************************************************************************/
NOMANGLE BT_INT CCONV v6_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MSGBUF  mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(channel_using_multiple_bc_buffers[cardnum])
      return API_BC_MULTI_BUFFER_ERR;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &messageid, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = mblock_addr[cardnum][messageid];
   vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,addr,BC_MSGBUF_DWSIZE); //only read the first two words for speed.

   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
  
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  If there is only one data buffer, just update it.
   *******************************************************************/
   if ( bc_num_dbufs[cardnum] == 1)
   {
      /* Update the single data buffer, using the specified word count */
      vbtWriteRelRAM[cardnum](cardnum, (BT_U16BIT *)buffer, mblock.data_addr+BC_DATA_DATA_OFFSET, wCount);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Two data buffers are defined.
   *  Output new data buffer to buffer not in use, then swap buffers.
   *******************************************************************/
   if (mblock.data_addr == RAM_ADDR(cardnum,(addr + BC_MSGBUF_SIZE)))//mblock.control_word & BC_HWCONTROL_BUFFERA
   {
      daddr = mblock.data_addr + BC_MSGDATA_SIZE;
   }
   else
   {
      daddr = mblock.data_addr - BC_MSGDATA_SIZE;
   }
   /* Update the specified data buffer, using the specified word count */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)buffer, daddr+BC_DATA_DATA_OFFSET, wCount);

   /*******************************************************************
   *  Update message block command word (2 bytes)
   *******************************************************************/
   mblock.data_addr = daddr;
   vbtWriteRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,addr,wsizeof(mblock));
   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - v5_BC_MessageUpdate()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block.  The data area to be updated
*     (there are two for each message block) is determined
*     by the current state of the Ping-Pong value in the
*     BC Message Block Control Word.  If data buffer A is
*     currently being used, then buffer B is updated.  Likewise,
*     if data buffer B is currently being used, then buffer
*     A is updated.  After the data area is updated, the buffer
*     bit in the Control Word is updated.  The next time this
*     message is sent by the BC, it will use the new data.
*
*     If both message data buffer pointers are the same, just update
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
* 01/19/1998 Improve speed by only reading message buffer up to the
*            data buffer pointers.
* 07/07/1998 Improve speed by only writing number of words specified
*            by the 1553 command word.
* 11/20/1998 If single buffered, just output the data words.
*****************************************************************************/
NOMANGLE BT_INT CCONV v5_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer)  // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &mblock_id, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);   /*sizeof(mblock)); V2.71.ajh */

   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
   
   /* Extract the number of words to write */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  If there is only one data buffer, just update it.
   *******************************************************************/
   if ( bc_num_dbufs[cardnum] == 1)
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      /* Update the single data buffer, using the specified word count */
      vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Two data buffers are defined.
   *  Output new data buffer to buffer not in use, then swap buffers.
   *******************************************************************/
   if (mblock.control_word & BC_HWCONTROL_BUFFERA)
   {
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
      mblock.control_word &= ~BC_HWCONTROL_BUFFERA;
   }
   else
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      mblock.control_word |= BC_HWCONTROL_BUFFERA;
   }
   /* Update the specified data buffer, using the specified word count */
   vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);

   /*******************************************************************
   *  Update message block command word (2 bytes)
   *******************************************************************/
   vbtWrite(cardnum,(LPSTR)&mblock,addr,1*2);
   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageUpdate()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block.  The data area to be updated
*     (there are two for each message block) is determined
*     by the current state of the Ping-Pong value in the
*     BC Message Block Control Word.  If data buffer A is
*     currently being used, then buffer B is updated.  Likewise,
*     if data buffer B is currently being used, then buffer
*     A is updated.  After the data area is updated, the buffer
*     bit in the Control Word is updated.  The next time this
*     message is sent by the BC, it will use the new data.
*
*     If both message data buffer pointers are the same, just update
*     the data buffer and exit.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
* 01/19/1998 Improve speed by only reading message buffer up to the
*            data buffer pointers.
* 07/07/1998 Improve speed by only writing number of words specified
*            by the 1553 command word.
* 11/20/1998 If single buffered, just output the data words.
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_MessageUpdate[cardnum](cardnum,mblock_id,buffer);
}

/****************************************************************************
*
*  PROCEDURE NAME - v5_BC_MessageUpdateBuffer()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block and Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV v5_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MESSAGE mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if (bc_num_dbufs[cardnum] == 1)// Only use 1 buffer
      if(buffer_id == 1)
         buffer_id =0;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &mblock_id, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = BC_MBLOCK_ADDR(cardnum, mblock_id);
   vbtRead(cardnum,(LPSTR)&mblock,addr,6*2);   /*sizeof(mblock)); V2.71.ajh */

   /*******************************************************************
   *  If this is not a 1553 message, return error.
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
   
   /* Extract the number of words to write */
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  If there is only one data buffer, just update it.
   *******************************************************************/
   if ( mblock.addr_data1 == mblock.addr_data2 )
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
      /* Update the single data buffer, using the specified word count */
      vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Two data buffers are defined.
   *  Output new data buffer to selected buffer.
   *******************************************************************/
   if (buffer_id == 1)
   {
      daddr = ((BT_U32BIT)mblock.addr_data2) << hw_addr_shift[cardnum];
   }
   else
   {
      daddr = ((BT_U32BIT)mblock.addr_data1) << hw_addr_shift[cardnum];
   }

   /* Update the specified data buffer, using the specified word count */
   vbtWrite(cardnum, (LPSTR)buffer, daddr, 2*wCount);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - v6_BC_MessageUpdateBuffer()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block and Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
*****************************************************************************/
NOMANGLE BT_INT CCONV v6_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BT_U32BIT  control;   // Control Word

   BT1553_COMMAND mess_cmd;
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if(channel_using_multiple_bc_buffers[cardnum])
      return API_BC_MULTI_BUFFER_ERR;

   if ( bc_num_dbufs[cardnum] == 1 && buffer_id==1)
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   if ( pUsrBC_MessageUpdate[cardnum] )
   {
      status = (*pUsrBC_MessageUpdate[cardnum])(cardnum, &messageid, buffer);
      if ( status == API_RETURN_SUCCESS )
         return API_SUCCESS;
      else if ( status == API_NEVER_CALL_AGAIN )
         pUsrBC_MessageUpdate[cardnum] = NULL;
      else if ( status != API_CONTINUE )
         return status;
   }
#endif

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = mblock_addr[cardnum][messageid];
   vbtReadRAM32[cardnum](cardnum,&control,addr+BC_CTLWD_OFFSET,1);

   vbtReadRAM[cardnum](cardnum,(BT_U16BIT *)&mess_cmd,addr+BC_MSG_CMD1_OFFSET,1);

   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (control & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
  
   wCount = mess_cmd.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  Update the data Buffer
   *******************************************************************/

   daddr = addr + BC_MSGBUF_SIZE;
   if(buffer_id == 1)
      daddr += BC_MSGDATA_SIZE;

   /* Update the specified data buffer, using the specified word count */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)buffer, daddr+BC_DATA_DATA_OFFSET, wCount);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_MessageUpdateBuffer()
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified message block and Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_MessageUpdateBuffer[cardnum](cardnum,mblock_id,buffer_id,buffer);
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_DataBufferWrite() V6 Only
*
*  FUNCTION
*     This routine is used to write the data area of the
*     specified message block and Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_DataBufferWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messageid,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0 - number of buffers
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  addr;      // General purpose byte address
   BT_U32BIT  daddr;     // Byte address of data buffer
   BC_MSGBUF  mblock;    // Used to read the BC message; to get data buf ptrs
   int        wCount;    // Number of data words in message
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   if (bc_num_dbufs[cardnum] == 1) // Only use 1 buffer
      if(buffer_id == 1)
         buffer_id = 0;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   addr = mblock_addr[cardnum][messageid];
   vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&mblock,addr,BC_MSGBUF_DWSIZE);

   /*******************************************************************
   *  If this is not a 1553 message, return error.
   *******************************************************************/
   if ( (mblock.control_word & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;
   
   /* Extract the number of words to write */
   flip((BT_U32BIT*)&mblock.mess_command1);
   wCount = mblock.mess_command1.wcount;
   if ( wCount == 0 )
      wCount = 32;

   /*******************************************************************
   *  Calculate the byte address of the specified data buffer.
   *******************************************************************/
   daddr = addr + BC_MSGBUF_SIZE + (buffer_id * BC_MSGDATA_SIZE) + BC_DATA_DATA_OFFSET;

   /* Update the specified data buffer, using the specified word count */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)buffer, daddr, wCount);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_DataBufferUpdate()  V6 Only
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified data Buffer address.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_DataBufferUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT buf_addr,      // (i) Address of the data buffer
   BT_UINT   dcount,        // (i) data count
   BT_U16BIT * buffer)      // (i) pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    status;    // Status return from called functions

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   /* Update the specified data buffer, using the specified word count */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)buffer, buf_addr+BC_DATA_DATA_OFFSET, dcount);

   status = API_SUCCESS;
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_SelectBufferUpdate()  V6 Only
*
*  FUNCTION
*     This routine is used to update the data area of the
*     specified data Buffer.  The data area to be updated
*     is determined by the buffer parameter passed to this function.  
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_SelectBufferUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   messno,        // (i) Message number
   BT_UINT   buf_num,       // (i) Data buffer number
   BT_UINT   dcount,        // (i) Data count
   BT_U16BIT * buffer)      // (i) Pointer to user's data buffer
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_UINT    status;    // Status return from called functions
   CEI_NATIVE_ULONG buf_addr;
   CEI_NATIVE_ULONG msg_addr;
   BT_U32BIT control;
   BT_U32BIT num_buf;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (messno >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Get message block from hardware, up to the second buffer pointer.
   *******************************************************************/
   msg_addr = mblock_addr[cardnum][messno];
   vbtReadRAM32[cardnum](cardnum,&control,msg_addr+BC_CTLWD_OFFSET,1);
   vbtReadRAM32[cardnum](cardnum,&num_buf,msg_addr+BC_NUM_BUFFERS_OFFSET,1);

   if(buf_num >= num_buf)
      return API_BC_BAD_DATA_BUFFER;

   /*******************************************************************
   *  If this is not a 1553 message.return error
   *******************************************************************/
   if ( (control & 0x0006) != 0x0002 )  /* Must be 1553 message */
      return API_BC_UPDATEMESSTYPE;

   buf_addr = msg_addr + BC_MSGBUF_SIZE + (buf_num * BC_MSGDATA_SIZE); 

   /* Update the specified data buffer, using the specified word count */
   vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)buffer, buf_addr+BC_DATA_DATA_OFFSET, dcount);

   status = API_SUCCESS;
   return status;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_SetFrameRate
*
*  FUNCTION
*     This routine dynamically update the BC frame time.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_BADFREQUENCY     -> Invalid Frame time
*     API_HARDWARE_NOSUPPORT  -> Feature not support by H/W
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_SetFrameRate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT frame_time)    // (i) New Frame Time in uSecs
{
   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (frame_time < FRAME_TIME_LOWER_LIMIT)
      return API_BC_BADFREQUENCY;
      
   /*******************************************************************
   *  Store minor frame register 1 microseconds resolution
   *******************************************************************/
   if(board_is_v5_uca[cardnum])
   {
      if(board_using_extended_timing[cardnum])// 1 microsecond timing
      {
         BT_U16BIT frame_lsb,frame_msb;

         if (frame_time < 250)
            return API_BC_BADFREQUENCY;
         frametime32[cardnum] = (BT_U32BIT)frame_time;
         frame_lsb = (BT_U16BIT)frametime32[cardnum] & 0xffff;
         frame_msb = (BT_U16BIT)((frametime32[cardnum] & 0xffff0000)>>16);
         vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_LSB, frame_lsb );
         vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME_MSB, frame_msb );
      }
      else 
      {
         frametime[cardnum] = (BT_U16BIT)(frame_time/25);   /* Must be less than 65535. */
         vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);
      }
   }
   else
   {
      BT_U32BIT csc_data;
    
      if(CurrentCardType[cardnum]==R15USB)
      {
         csc_data = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);
         if(csc_data & CR1_BCRUN)
            return API_HARDWARE_NOSUPPORT; 
         else
            vbtSetRegister[cardnum](cardnum, HWREG_MINOR_FRAMEV6, frame_time);
         return API_SUCCESS; 
      }
 
      if(bc_running[cardnum])
      {
         do{
            csc_data = vbtGetRegister[cardnum](cardnum, HWREG_CONTROL);
         } while ((csc_data & CR1_BCBUSY) == 0);
      }

      vbtSetRegister[cardnum](cardnum, HWREG_MINOR_FRAMEV6, frame_time);
      frametime32[cardnum] = frame_time;
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - v5_BC_Start
*
*  FUNCTION
*     This routine handles starting the BC at a specified message number.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BUSTOOLS_BADCARDNUM -> Invalid card number
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT mblock_id)       // (i) BC Message number to start processing at
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U16BIT  value;       /* General Purpose variable */
   BT_INT mode_warning=0;
   BT_UINT hwrData;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BC_START, mblock_id, bc_running[cardnum], 0, 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (mblock_id >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Handle "BC-Start" operation:
   *     re-load BC message start pointer
   *     start BC
   *******************************************************************/
   // Detect single function or sRT board attempting to start second function.V4.26
   if(!bit_running[cardnum])
   {
      if(board_is_dual_function[cardnum] == 1)
      {
         mode_warning = API_DUAL_FUNCTION_ERR;
         if ( _HW_1Function[cardnum] && (rt_running[cardnum]) )
           return API_DUAL_FUNCTION_ERR;
      }
      else if (board_is_sRT[cardnum])
         return API_SINGLE_RT_MODE_ERR; 
      else 
      {
         mode_warning = API_SINGLE_FUNCTION_ERR;
         if ( _HW_1Function[cardnum] && (bm_running[cardnum] | rt_running[cardnum]) )
            return API_SINGLE_FUNCTION_ERR;
      }
   }

   SignalV5UserThread(cardnum, EVENT_BC_START, 0, 0);

   value = (BT_U16BIT)(BC_MBLOCK_ADDR(cardnum, mblock_id) >> hw_addr_shift[cardnum]);
   vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTR, 0x0000);
   vbtSetFileRegister(cardnum, RAMREG_BC_MSG_PTRSV, value);
   vbtSetFileRegister(cardnum, RAMREG_ENDFLAGS, 0);

   /* clear the high/low priority aperiodic message pointers */
   vbtSetFileRegister(cardnum, RAMREG_HIGHAPTR, 0);
   vbtSetFileRegister(cardnum, RAMREG_LOWAPTR, 0);
   /* clear the BC aperiodic flags */
   vbtSetFileRegister(cardnum, RAMREG_HIGHAFLG, 0);
   vbtSetFileRegister(cardnum, RAMREG_LOW_AFLG, 0);
   /* aperiodic End-of-List */
   vbtSetFileRegister(cardnum, 0x23, 0);

   // Clear the minor frame overflow bit.
   vbtSetFileRegister(cardnum, RAMREG_ORPHAN, (BT_U16BIT)(vbtGetFileRegister(cardnum, RAMREG_ORPHAN) & ~RAMREG_ORPHAN_MINORFRAME_OFLOW));

   // Set the minor frame time into the hardware register 0x0E!
//   vbtSetHWRegister(cardnum, HWREG_MINOR_FRAME, frametime[cardnum]);

   /* If ext_trig_bc == 0, no external triggering of BC. */
   /*                == 1, BC started by external trigger. */
   /*                ==-1, each BC minor frame started by external trigger. */
   /* On a 486/DX4-100 it takes 17 us to get the bus started. */
   if ( ext_trig_bc[cardnum] == 0 )
   {
      api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  /* Disable HW BC trigger */
      api_writehwreg_or(cardnum,HWREG_CONTROL1,CR1_BCRUN); /* Just run the BC */
      //Test for single/dual mode warning
      hwrData = vbtGetHWRegister(cardnum,HWREG_CONTROL1);
      if(hwrData & CR1_SMODE)
         return mode_warning;
   }
   else
   {
      api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,CR1_BC_EXT_SYNC); /* Enable HW BC trig */
   }
   /* The HWREG_RESPONSE register must be programmed after one of the */
   /*  three run bits has been set.  Does the trigger bit count?? */
   api_writehwreg(cardnum,HWREG_RESPONSE,(BT_U16BIT)wResponseReg4[cardnum]);

   bmrec_timetag[cardnum].microseconds = 0;      /* Timetag for simulated messages only */
   bmrec_timetag[cardnum].topuseconds = 0;
   bc_running[cardnum]    = 1;      /* Try to remember that the BC is running */

   channel_status[cardnum].bc_run=1;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - v6_BC_Start
*
*  FUNCTION
*     This routine handles starting the BC at a specified message number.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BUSTOOLS_BADCARDNUM -> Invalid card number
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid)       // (i) BC Message number to start processing
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  value;       /* General Purpose variable */
   BT_UINT hwrData;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BC_START, messageid, bc_running[cardnum], 0, 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   if (messageid >= bc_mblock_count[cardnum])
      return API_BC_ILLEGAL_MBLOCK;

   /*******************************************************************
   *  Handle "BC-Start" operation:
   *     re-load BC message start pointer
   *     start BC
   *******************************************************************/
   // Detect single function or sRT board attempting to start second function.V4.26
   if(!bit_running[cardnum])
   {
      if (board_is_sRT[cardnum])
         return API_SINGLE_RT_MODE_ERR;
      else if ( _HW_1Function[cardnum] && (rt_running[cardnum]) )
         return API_DUAL_FUNCTION_ERR; 
   }

   SignalUserThread(cardnum, EVENT_BC_START, 0, 0);

   value = mblock_addr[cardnum][messageid];
   vbtSetRegister[cardnum](cardnum, HWREG_BCMSG_PTR, RAM_ADDR(cardnum,value));

   /* clear the high/low priority aperiodic message pointers */
   vbtSetRegister[cardnum](cardnum, HWREG_BC_HIGH_PRI_MSG, 0);
   vbtSetRegister[cardnum](cardnum, HWREG_BC_LOW_PRI_MSG, 0);
   /* clear the BC flags */
   vbtSetRegister[cardnum](cardnum, HWREG_BC_APER_FLAG, 0);

   if ( ext_trig_bc[cardnum] == 0 )
   {
      api_clearhwcbits(cardnum, CR1_BC_EXT_SYNC);   /* Disable HW BC trig */ 
      api_sethwcbits(cardnum,CR1_BCRUN);            /* Start the BC       */
      // Test for dual mode warning
      hwrData = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);
      if(hwrData & CR1_IMW)
         return API_DUAL_FUNCTION_ERR;
   }
   else
   {
      api_sethwcbits(cardnum, CR1_BC_EXT_SYNC);   /* Enable HW BC trig    */
   }
   /* The HWREG_RESPONSE register must be programmed after one of the     */
   /*  three run bits has been set.                                       */
   vbtSetRegister[cardnum](cardnum,HWREG_V6RESPONSE,wResponseReg4[cardnum]);

   bc_running[cardnum]    = 1;      /* Try to remember that the BC is running */

   channel_status[cardnum].bc_run=1;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_Start
*
*  FUNCTION
*     This routine handles starting the BC at a specified message number.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BUSTOOLS_BADCARDNUM -> Invalid card number
*     API_BC_ILLEGAL_MBLOCK   -> BC illegal message block number specified
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid)       // (i) BC Message number to start processing at
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_Start[cardnum](cardnum,messageid);
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_StartStop
*
*  FUNCTION
*     This routine handles turning the BC on or off.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BM_Init not yet called
*     API_BC_RUNNING          -> BM currently running
*     API_BC_NOTRUNNING       -> BM not currently running
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_StartStop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag)       // (i) flag=1 BC_START to start the BC, 
                            //     flag=0 BC_STOP to stop it 
                            //     flag=2 BC_START_BIT
                            //     flag=3 BC_HALT stop immediately
                             
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_U32BIT  value;       /* Determine if BC busy */
   time_t     lTime = 0;   /* Timeout value */
   BT_INT     status;

   /************************************************
   *  Check initial conditions
   ************************************************/
   AddTrace(cardnum, NBUSTOOLS_BC_STARTSTOP, startflag, bc_running[cardnum], bt_inited[cardnum], 0, 0);
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  Call the user interface dll entry point, if it exists
   *******************************************************************/
#if defined(_USER_DLL_)
   {
      int        i;           /* Loop counter */
      if ( pUsrBC_StartStop[cardnum] )
      {
         i = (*pUsrBC_StartStop[cardnum])(cardnum, &startflag);
         if ( i == API_RETURN_SUCCESS )
            return API_SUCCESS;
         else if ( i == API_NEVER_CALL_AGAIN )
            pUsrBC_StartStop[cardnum] = NULL;
         else if ( i != API_CONTINUE )
            return i;
      }
   }
#endif

   /*******************************************************************
   *  Handle "BC-Start" operation:
   *     re-load BC message start pointer
   *     start BC
   *******************************************************************/
   if ( startflag == BC_START || startflag == BC_START_BIT || startflag == BC_START_TT_RESET)
   {
      if(startflag==BC_START_TT_RESET)
      {
         /*  Initialize the H/W Bus Monitor time counter. */
         BusTools_TimeTagInit(cardnum);
      }
      if(startflag == BC_START_BIT)
         bit_running[cardnum]=1;
      /* Start the Bus Controller beginning with the first message in the list. */
      status = BusTools_BC_Start(cardnum, 0);
      if(startflag == BC_START_BIT)
         bit_running[cardnum]=0;
      return status;
   }

   /*******************************************************************
   *  Handle "stop" operation -- loop until the BC_BUSY bit is clear.
   *  If it takes too long, just turn off the BC anyway.
   *******************************************************************/

   /* Turn off the Bus Controller Enable external trigger bit first, */
   /*  if it is on. */

   if(board_is_v5_uca[cardnum])
   {
      SignalV5UserThread(cardnum, EVENT_BC_STOP, 0, 0);
      api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  /* Disable HW BC trigger */

      lTime = time(NULL) + 2;   /* Setup a time-out value of 1-2 seconds.V4.01 */
      /* This is to allow the frame to complete before stopping the BC */
      do{
         MSDELAY(1); //Wait a millisecond
         value = api_readhwreg(cardnum,HWREG_CONTROL1);
         /* Finish if either BC Run or BC Busy bits are clear */
         if ( (value & (CR1_BCBUSY|CR1_BCRUN)) != (CR1_BCBUSY|CR1_BCRUN) )
            break;
      }
      while ( time(NULL) < lTime );

      // Turn off the BC Run bit and the BC Busy.  
      api_writehwreg_and(cardnum,HWREG_CONTROL1,(BT_U16BIT)(~(CR1_BCRUN|CR1_BCBUSY)));

      bc_running[cardnum] = 0;

      /*******************************************************************
      *  If an error was detected -- return error code
      *******************************************************************/
      value = api_readhwreg(cardnum,HWREG_CONTROL1);
   }
   else
   {
      SignalUserThread(cardnum, EVENT_BC_STOP, 0, 0);
      if(startflag == BC_HALT)
      {
         api_clearhwcbits(cardnum,CR1_BCRUN|CR1_BCBUSY);
         MSDELAY(10);
         api_clearhwcbits(cardnum, CR1_BC_EXT_SYNC);   /* Disable HW BC trig */ 
         bc_running[cardnum] = 0;
         return API_SUCCESS;
      }
      api_clearhwcbits(cardnum, CR1_BC_EXT_SYNC);   /* Disable HW BC trig */   
      lTime = time(NULL) + 2;   /* Setup a time-out value of 1-2 seconds.V4.01 */
      /* This is to allow the frame to complete before stopping the BC */
      do{
         value = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);
         /* Finish if either BC Run or BC Busy bits are clear */
         if ( (value & (CR1_BCBUSY|CR1_BCRUN)) != (CR1_BCBUSY|CR1_BCRUN) )
            break;
         MSDELAY(1);
      }
      while ( time(NULL) < lTime );
      // Turn off the BC Run bit and the BC Busy.  
      api_clearhwcbits(cardnum,CR1_BCRUN|CR1_BCBUSY);
      bc_running[cardnum] = 0;

      /*******************************************************************
      *  If an error was detected -- return error code
      *******************************************************************/
      value = vbtGetRegister[cardnum](cardnum,HWREG_CONTROL);
   }
   if ( value & CR1_BCBUSY )
      return API_BC_HALTERROR;
   
   channel_status[cardnum].bc_run=0;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_BC_Trigger()
*
*  FUNCTION
*     This routine is used to setup the BC external trigger mode.
*     The BC supports three trigger modes:
*
*     BC_TRIGGER_IMMEDIATE  - BC starts running immediately
*     BC_TRIGGER_ONESHOT    - BC is triggered by external source,
*                             and free runs after the trigger.
*     BC_TRIGGER_REPETITIVE - Each minor frame is started by the
*                             external trigger.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BC_NOTINITED        -> BC_Init not yet called
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Trigger(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_INT    trigger_mode)  // (i0 BC_TRIGGER_IMMEDIATE - BC starts immediately.
                            //  -> default              - BC starts immediately.
                            //  -> BC_TRIGGER_ONESHOT   - BC trig ext TTL source.
                            //  -> BC_TRIGGER_REPETITIVE- Each minor frame is
                            //                            started by the TTL trig.
                            //  -> BC_TRIGGER_USER        Allows user to configure Frame triggering
{
   /***********************************************************************
   *  Check initial conditions
   **********************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /*******************************************************************
   *  Set the global BC Trigger Flag based on the trigger_mode parameter.
   *******************************************************************/
   /* If ext_trig_bc == 0, no external triggering of BC. */
   /*                == 1, BC started by external trigger. */
   /*                ==-1, each BC minor frame started by external trigger. */
   switch ( trigger_mode )
   {
      case BC_TRIGGER_IMMEDIATE:  /* BC starts running immediately. */
      default:
           ext_trig_bc[cardnum] = BC_TRIGGER_IMMEDIATE;
           break;
      case BC_TRIGGER_ONESHOT:    /* BC is triggered by external source, */
                                  /*   and free runs after trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_ONESHOT;
           break;
      case BC_TRIGGER_REPETITIVE: /* Each minor frame is started by the */
                                  /*   external trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_REPETITIVE;
           break;
      case BC_TRIGGER_USER:       /* User defined trigger */
                                  /*   external trigger. */
           ext_trig_bc[cardnum] = BC_TRIGGER_USER;
           break;
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -    DumpBCmsg()
*
*  FUNCTION
*       This procedure outputs a dump of BC Messages.  It is a local helper
*       function for the BusTools_DumpMemory user-callable function.
*
****************************************************************************/
void DumpBCmsg(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j, k;                     // Loop index
   int          msgnum;                   // Message number
   int          shift_count;              // Shift addresses
   int          num_ctrl_wrds;            // Number of BC Message control words
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[48];                 // Read 16+ words per line

   /* Dump the write-only BC setup parameters  */
   if(board_using_extended_timing[cardnum])
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime32[cardnum]);
   else
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime[cardnum]*25);

   /* This is where the specified buffers live */
   BusTools_GetAddr(cardnum, GETADDR_BCMESS, &first, &last);
   first /= 2;    // Convert to word offset.
   last  /= 2;    // Convert to word offset.
   msgnum = 0;
   shift_count = hw_addr_shift[cardnum] - 1;  // Just shift to word addresses
   num_ctrl_wrds = 24;

   for ( i = first; i < last; )
   {
      // Read the current line of 12 data words from the board.
      BusTools_MemoryRead(cardnum, i*2, 24*2, data);
      if ( (data[0] & 0x0006) == 0x0002 )
      {   // Normal 1553 message
         fprintf(hfMemFile, "Msg %3d @%04X:", msgnum, i);
         for ( j = 0; j < 2; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         if(board_has_bc_timetag[cardnum])
            fprintf(hfMemFile, " tt%04X%04X%04X", data[14], data[13], data[12]);
         fprintf(hfMemFile, " ei%3.3X", data[2]);
         fprintf(hfMemFile, " gap%d", data[3]);
         fprintf(hfMemFile, " F=%4.4X", data[4]<<shift_count);  // First Buffer address
         if ( (unsigned)(data[4]<<shift_count) == i+num_ctrl_wrds )
            fprintf(hfMemFile, "!");
         else
            fprintf(hfMemFile, "*");
         fprintf(hfMemFile, " S=%4.4X", data[5]<<shift_count);  // Second Buffer address
         if ( (unsigned)(data[5]<<shift_count) == i+num_ctrl_wrds + (bc_num_dbufs[cardnum]-1)*bc_size_dbuf[cardnum]/2 )
            fprintf(hfMemFile, "!");
         else
            fprintf(hfMemFile, "*");
         fprintf(hfMemFile, " 2c%4.4X", data[6]);                  // Second (RT-RT) Command word
         fprintf(hfMemFile, " tS%4.4X rS%4.4X", data[7], data[8]); // RT 1553 Status Words
         fprintf(hfMemFile, " %4.4X%4.4X", data[10], data[9]);     // Interrupt status
         if(board_using_msg_schd[cardnum])
           fprintf(hfMemFile, " SF=%d RR=%d ", data[15],data[5]);
         fprintf(hfMemFile, " nxt@%4.4X", data[11]<<shift_count);  // Next message addr
         if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
            fprintf(hfMemFile, "!\n");
         else
            fprintf(hfMemFile, "*\n");
      }
      else
      {   // NOOP, STOP or Conditional
         if ( (data[0] & 0x0006) == 0x0000 )
         {  // NOOP Message
            if((data[0] & BC_HWCONTROL_SET_TNOP) == BC_HWCONTROL_SET_TNOP)
               fprintf(hfMemFile, "TNop %3d @%04X gap%d:", msgnum, i,data[3]);
            else
               fprintf(hfMemFile, "Nop %3d @%04X:", msgnum, i);
            for ( j = 0; j < 11; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " nxt@%5.5X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
         else if ( (data[0] & 0x0006) == 0x0004 )
         {  // STOP Message
            fprintf(hfMemFile, "Stp %3d @%04X:", msgnum, i);
            for ( j = 0; j < 11; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " nxt@%5.5X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
         else if ( (data[0] & 0x0006) == 0x0006 )
         {  // CONDITIONAL Message
            fprintf(hfMemFile, "Con %3d @%04X:", msgnum, i);
            fprintf(hfMemFile, " %4.4X", data[0]);
            fprintf(hfMemFile, " tst=%5.5X/%4.4X", (data[1]<<shift_count)+data[2],data[2]);
            for ( j = 3; j < 10; j++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, " brn@%4.4X", data[10]<<shift_count);     // Branch msg addr
            if ( (unsigned)(data[10]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!");
            else
               fprintf(hfMemFile, "*");
            fprintf(hfMemFile, " nxt@%4.4X", data[11]<<shift_count);   // Next msg addr
            if ( (unsigned)(data[11]<<shift_count) == i+bc_num_dbufs[cardnum]*bc_size_dbuf[cardnum]/2+num_ctrl_wrds )
               fprintf(hfMemFile, "!\n");
            else
               fprintf(hfMemFile, "*\n");
         }
      }
      // Step over the message control words to the data buffers.
      i += num_ctrl_wrds;

      // For each buffer (either one or two)...
      for ( k = 0; k < bc_num_dbufs[cardnum]; k++ )
      {
         fprintf(hfMemFile, "Data %2d @%04X:", k, i);

         BusTools_MemoryRead(cardnum, i*2, 16*2, data);
         for ( j = 0; j < 16; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
         i += 16;
         // 33 or 40 words per buffer
         fprintf(hfMemFile, "Data %2d @%04X:", k, i);
         BusTools_MemoryRead(cardnum, i*2, (bc_size_dbuf[cardnum]-32), data);
         //for ( j = 0; j < (unsigned)(bc_size_dbuf[cardnum]/2-16); j++ )
         for ( j = 0; j < 16; j++ )
            fprintf(hfMemFile, " %4.4X", data[j]);
         fprintf(hfMemFile, "\n");
         i += bc_size_dbuf[cardnum]/2 - 16;
      }
      msgnum++;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME -    V6DumpBCmsg()
*
*  FUNCTION
*       This procedure outputs a dump of BC Messages.  It is a local helper
*       function for the BusTools_DumpMemory user-callable function.
*
****************************************************************************/
void V6DumpBCmsg(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    i;                  // Loop index
   BT_UINT      j, k,id, wcnt;      // Loop index
   BT_U32BIT    first;              // Offset to first list word
   BT_U32BIT    last;               // Offset to last list word
   BT_U32BIT    daddr;              // data address

   BT_U32BIT    firstEI;            // Offset to first list word
   BT_U32BIT    lastEI;             // Offset to last list word
   BT_U32BIT    testControl;        // flag for testing the control word
   BT_U32BIT    firstRAM;           // comparator for validating BC data
   BT_U32BIT    lastRAM;            // comparator for validating BC data

   union
   {
      BC_MSGBUF       mblock;      // Local copy of hardware BC Message 
      BC_CTRLBUF      cblock;         
      BT_U32BIT       cdata[BC_MSGBUF_DWSIZE];
   }CTRL;

   union
   {
      BC_MSGDATA      dblock;
      BC_CTRLDATA     cdblock;
      BT_U32BIT       cdata[BC_MSGDATA_DWSIZE];    
   }MDATA;

   /* This is where the specified buffers live */
   BusTools_GetAddr(cardnum, GETADDR_BCMESS, &first, &last);
   /* create variables to test for valid BC message data */
   firstRAM = RAM_ADDR(cardnum,first);
   lastRAM   = RAM_ADDR(cardnum,last);
   fprintf(hfMemFile, "\n%s.Start %8.8X End %8.8X (Byte Offset)\n",
      BusTools_GetAddrName(GETADDR_BCMESS), firstRAM, lastRAM);

   /* create variables to test for valid BC message block */
   BusTools_GetAddr(cardnum, GETADDR_EI, &firstEI, &lastEI);
   firstEI = RAM_ADDR(cardnum, firstEI);
   lastEI  = RAM_ADDR(cardnum, lastEI);

   /* Dump the write-only BC setup parameters  */
   if(board_using_extended_timing[cardnum])
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime32[cardnum]);
   else
      fprintf(hfMemFile, "HWREG_RESPONSE Reg = %X, frametime(us) = %u(dec)\n",
             wResponseReg4[cardnum], frametime[cardnum]*25);
   /* This is where the specified buffers live */
   BusTools_GetAddr(cardnum, GETADDR_BCMESS, &first, &last);

   for ( i = first; i < last; )
   {
      // Read the current message block from the board.
      vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&CTRL.cdata,i,BC_MSGBUF_DWSIZE);
      if(channel_using_multiple_bc_buffers[cardnum])
         daddr = CTRL.mblock.data_addr;
      else
         daddr = RAM_ADDR(cardnum,(i + BC_MSGBUF_SIZE));

      testControl = CTRL.mblock.control_word & 0x0006;

      // make some tests on the validity of the BC buffer information. If the next message pointer
      // is == 0 then this is the end of an aperiodic message list.
      if (CTRL.mblock.addr_next && ((CTRL.mblock.addr_next < firstRAM) || (CTRL.mblock.addr_next > lastRAM)))
      {
         // if the number of data buffers is the same as the pointer to the next address
         // that means that memory is hosed up somewhere. bail!
         flip((BT_U32BIT *)&CTRL.mblock.mess_command1);
         fprintf(hfMemFile, "INVALID Msg  %3d @%08X:", CTRL.mblock.messno, RAM_ADDR(cardnum,i));
         fprintf(hfMemFile, " nbuf=%d", CTRL.mblock.num_data_buffers);
         fprintf(hfMemFile, " ctl=%04X", CTRL.mblock.control_word);
         fprintf(hfMemFile, " cw1=%04X cw2=%04X", *(BT_U16BIT *)&CTRL.mblock.mess_command1,*(BT_U16BIT *)&CTRL.mblock.mess_command2);
         id = CTRL.mblock.addr_error_inj;        
         fprintf(hfMemFile, " ei=%8.8X",id);
         fprintf(hfMemFile, " data=%08X:", CTRL.mblock.data_addr);
         fprintf(hfMemFile, " gap=%d", CTRL.mblock.gap_time);
         flip((BT_U32BIT *)&CTRL.mblock.rep_rate);
         fprintf(hfMemFile, " sf=%d rr=%d", CTRL.mblock.start_frame,CTRL.mblock.rep_rate);         
         fprintf(hfMemFile, " nxt msg@%08X\n", CTRL.mblock.addr_next);  // Next message addr

         break; 
      }

      // check if the EI pointer is within acceptable range, and not a recognizable 
      // control word, this is probably a playback buffer and will be a problem.
      if ((testControl == 2) && ((CTRL.mblock.addr_error_inj < firstEI) || (CTRL.mblock.addr_error_inj > lastEI)))
      {
          fprintf (hfMemFile, "Invalid EI pointer 0x%08X. Probably Playback Data. Quit\n",
              CTRL.mblock.addr_error_inj);
          break;
      }
      if ( (CTRL.mblock.control_word & 0x0006) == 0x0002 )
      {   // Normal 1553 message

         flip((BT_U32BIT *)&CTRL.mblock.mess_command1);
         fprintf(hfMemFile, "Msg  %3d @%08X:", CTRL.mblock.messno, RAM_ADDR(cardnum,i));
         fprintf(hfMemFile, " nbuf=%d", CTRL.mblock.num_data_buffers);
         fprintf(hfMemFile, " ctl=%04X", CTRL.mblock.control_word);
         fprintf(hfMemFile, " cw1=%04X cw2=%04X", *(BT_U16BIT *)&CTRL.mblock.mess_command1,*(BT_U16BIT *)&CTRL.mblock.mess_command2);
         id = CTRL.mblock.addr_error_inj;        
         fprintf(hfMemFile, " ei=%8.8X",id);
         fprintf(hfMemFile, " data=%08X:", CTRL.mblock.data_addr);
         fprintf(hfMemFile, " gap=%d", CTRL.mblock.gap_time);
         flip((BT_U32BIT *)&CTRL.mblock.rep_rate);
         fprintf(hfMemFile, " sf=%d rr=%d", CTRL.mblock.start_frame,CTRL.mblock.rep_rate);         
         fprintf(hfMemFile, " nxt msg@%08X\n", CTRL.mblock.addr_next);  // Next message addr

         // For each buffer
         for ( k = 0; k<CTRL.mblock.num_data_buffers; k++ )
         {
            fprintf(hfMemFile, "Data %3d @%08X: ", k,daddr);

            // Try to make some assumption of the validity of the BC buffer information
            if (CTRL.mblock.num_data_buffers == CTRL.mblock.addr_next) 
            {
               // if the number of data buffers is the same as the pointer to the next address
               // that means that memory is hosed up somewhere. bail!
               fprintf(hfMemFile, "INVALID Msg  %3d @%08X:", CTRL.mblock.messno, RAM_ADDR(cardnum,i));
               break; 
            }

            //vbtReadRelRAM32[cardnum](cardnum,(BT_U32BIT *)&MDATA.cdata,daddr,BC_MSGDATA_DWSIZE);
            vbtReadRelRAM32[cardnum](cardnum,(BT_U32BIT *)&MDATA.dblock,daddr,6);
            vbtReadRelRAM[cardnum](cardnum, (BT_U16BIT *)&MDATA.dblock.data_word, daddr+BC_DATA_DATA_OFFSET,34);

            flip((BT_U32BIT *)&MDATA.dblock.mess_status1);
            fprintf(hfMemFile, "nxt data %08X", MDATA.dblock.next_data);
            fprintf(hfMemFile, " TTh %08X", MDATA.dblock.timetag.topuseconds);
            fprintf(hfMemFile, " TTl %08X", MDATA.dblock.timetag.microseconds);
            fprintf(hfMemFile, " stat %08X", MDATA.dblock.mstatus);
            fprintf(hfMemFile, " sw1=%04X", *(BT_U16BIT *)&MDATA.dblock.mess_status1);
            fprintf(hfMemFile, " sw2=%04X\n",  *(BT_U16BIT *)&MDATA.dblock.mess_status2);
            
            if(CTRL.mblock.mess_command1.wcount==0)
               wcnt = 32;
            else
               wcnt = CTRL.mblock.mess_command1.wcount;

            if((CTRL.mblock.mess_command1.subaddr == 0) || 
               (CTRL.mblock.mess_command1.subaddr == 31 && rt_sa31_mode_code[cardnum]==1))
            {
               if(wcnt > 15)
                  wcnt = 1;
               else
                  wcnt = 0;
            }
          
            for(j=0; j<32; j++)
            {
               if(j==0)
                  fprintf(hfMemFile, "Data %3d @%08X: ",k,daddr+BC_DATA_DATA_OFFSET );
               if(j==16)
                  fprintf(hfMemFile, "\nData %3d @%08X: ",k,daddr+BC_DATA_DATA_OFFSET+32 );                
             
               fprintf(hfMemFile, "%04X", MDATA.dblock.data_word[j]);
               if(j==wcnt-1)
                  fprintf(hfMemFile, "|");
               else
                  fprintf(hfMemFile, " ");
            }
            fprintf(hfMemFile, "\n");
            if(channel_using_multiple_bc_buffers[cardnum])
               daddr = MDATA.dblock.next_data;
            else
               daddr = daddr + BC_MSGDATA_SIZE;     
         }
      }
      else
      {   // NOOP, STOP or Conditional
         if ( (CTRL.cblock.control_word & 0x0006) == 0x0000 )
         {  // NOOP Message
            if((CTRL.cblock.control_word & BC_HWCONTROL_SET_TNOP) == BC_HWCONTROL_SET_TNOP)
               fprintf(hfMemFile, "TNOP %3d @%08X: ctl=%04X gap%d:",  CTRL.mblock.messno, RAM_ADDR(cardnum,i), CTRL.mblock.control_word, CTRL.cblock.gap_time);
            else
               fprintf(hfMemFile, "NOP  %3d @%08X: ctl=%04X", CTRL.cblock.messno, RAM_ADDR(cardnum,i), CTRL.mblock.control_word);
            fprintf(hfMemFile, " nxt msg@%8.8X\n", CTRL.mblock.addr_next);  // Next message addr
         }
         else if ( (CTRL.cblock.control_word & 0x0006) == 0x0004 )
         {  // STOP Message
            if((CTRL.cblock.control_word & BC_CLR_EXT_SYNC) == BC_CLR_EXT_SYNC)
               fprintf(hfMemFile, "HLT  %3d @%08X: ctl=%04X", CTRL.cblock.messno, RAM_ADDR(cardnum,i), CTRL.mblock.control_word);
            else
               fprintf(hfMemFile, "STOP  %3d @%08X: ctl=%04X", CTRL.cblock.messno, RAM_ADDR(cardnum,i), CTRL.mblock.control_word);
            fprintf(hfMemFile, " nxt msg@%8.8X\n", CTRL.mblock.addr_next);  // Next message addr
         }
         else if ( (CTRL.cblock.control_word & 0x0006) == 0x0006 )
         {  // CONDITIONAL Message
            fprintf(hfMemFile, "CON  %3d @%08X: ", CTRL.cblock.messno, RAM_ADDR(cardnum,i));
            fprintf(hfMemFile, " nbuf=%d", CTRL.cblock.num_data_buffers);
            fprintf(hfMemFile, " ctl=%04X", CTRL.cblock.control_word);
            fprintf(hfMemFile, " data=%08X:", CTRL.cblock.data_addr);
            fprintf(hfMemFile, " nxt msg@%8.8X", CTRL.cblock.addr_next);  // Next message addr
            fprintf(hfMemFile, " branch msg@%8.8X\n", CTRL.cblock.branch_msg_ptr);  // branch message addr
         }
         for ( k = 0; k<CTRL.cblock.num_data_buffers; k++ )
         {
            fprintf(hfMemFile, "Data %3d @%08X: ",k,daddr);
            vbtReadRelRAM32[cardnum](cardnum,(BT_U32BIT *)&MDATA.cdata,daddr,BC_MSGDATA_DWSIZE);
            fprintf(hfMemFile, "nxt data %08X", MDATA.cdblock.next_data);
            fprintf(hfMemFile, " TTh %08X", MDATA.cdblock.timetag.topuseconds);
            fprintf(hfMemFile, " TTl %08X", MDATA.cdblock.timetag.microseconds);
            if ( (CTRL.cblock.control_word & 0x0006) == 0x0006 )
            {
               fprintf(hfMemFile, " tst %08X ", MDATA.cdblock.tst_wrd_addr);
               fprintf(hfMemFile, "pat %08X ", MDATA.cdblock.data_pattern);
               fprintf(hfMemFile, "msk %08X ", MDATA.cdblock.bit_mask);
               fprintf(hfMemFile, "ctr %08X ", MDATA.cdblock.cond_counter);
               fprintf(hfMemFile, "val %08X ", MDATA.cdblock.cond_count_val);
               fprintf(hfMemFile, "nxt %08X ", MDATA.cdblock.next_msg_addr); 
            }
            fprintf(hfMemFile, "\n");
            
            // Try to make some determination of the validity of the BC buffer information
            if (CTRL.mblock.num_data_buffers == CTRL.mblock.addr_next) 
            {
               // if the number of data buffers is the same as the pointer to the next address
               // that means that memory is hosed up somewhere. bail!
               fprintf(hfMemFile, "INVALID Msg  %3d @%08X:", CTRL.mblock.messno, RAM_ADDR(cardnum,i));
               break; 
            } 

            daddr+=BC_CTRLDATA_SIZE;
         }
      }
      fprintf(hfMemFile, "\n");
      i+=(BC_MSGBUF_SIZE + (BC_MSGDATA_SIZE * CTRL.cblock.num_data_buffers));
   }
}

#ifndef _CVI_
/****************************************************************************
*
*   PROCEDURE NAME - v5_BC_ReadNextMessage()
*
*   FUNCTION
*       This function reads the next BC message that meets the criteria set
*       by the passed parameters.  The arguments include a timeout value in
*       in milliseconds. If there are no BC messages put onto the queue this
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_MBLOCK_NOMATCH;  -> Bad BC Messno
*       API_BC_READ_TIMEOUT     -> Timeout before data read
*       API_HW_IQPTR_ERROR      -> interrupt Queue pointer error
*
*****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_INT status;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.

   BT_U16BIT iq_addr;                  // Hardware Interrupt Queue Pointer
   BT1553_COMMAND cmd;
   BT_U32BIT beg;                      // Beginning of the interrupt queue
   BT_U32BIT end;                      // End of the interrupt queue

   BT_UINT   messno;
  
   BT_U32BIT mess_addr;
   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_U32BIT  msg_blk_size;  // Size of a BC message block, including data bufs
   BT_U32BIT start;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   //start = timeGetTime();
   start = CEI_GET_TIME();

   // Get range of byte addresses for interrupt queue 
   beg = BTMEM_IQ;
   end = BTMEM_IQ_NEXT - 1;

   iq_addr = vbtGetFileRegister(cardnum,INT_QUE_PTR_REG/2);

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_sw = ((BT_U32BIT)iq_addr) << 1;

   /************************************************
   *  Loop until timeout
   ************************************************/

   do
   {
      /* Read current location in interupt queue */
	  iq_addr = vbtGetFileRegister(cardnum,INT_QUE_PTR_REG/2);

      // Convert the hardware word address to a byte address.
      iqptr_hw = ((BT_U32BIT)iq_addr) << 1;
      // If the pointer is outside of the interrupt queue abort.V4.09.ajh
      if ( (iqptr_hw < beg) || (iqptr_hw > end) )
         return API_HW_IQPTR_ERROR;

      /**********************************************************************
      *  Process all HW Interrupt Queue entries that have been written
      *  Start with the SW interrupt pointer from the last time.
      **********************************************************************/

      while ( iqptr_sw != iqptr_hw )
      {
         /*******************************************************************
         *  Get the 3 word interrupt block pointed to by iqptr_sw.
         *******************************************************************/
         vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));

         iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

         // We only care about BC interrupt here.
         if ( intqueue.t.mode.bc )
         {
            mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
            /*******************************************************************
            *  Calculate index based on starting address & size.
            *******************************************************************/
            msg_blk_size = sizeof(BC_MESSAGE) +
                                        (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

            mess_addr <<= 3;     // Align to 8-word boundry.
            messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

            if ( messno >= bc_mblock_count[cardnum] )
               return API_BC_MBLOCK_NOMATCH;
            vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));
            if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
			{
			   if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			   {
                  if((tr == DONT_CARE) || (tr == cmd.tran_rec))
				  {
                     status = BusTools_BC_MessageRead(cardnum, messno, pBC_mbuf);
				     return status;
				  }
			   }
			}
         }
      }
   }while((CEI_GET_TIME() - start) < timeout);
   
   return API_BC_READ_TIMEOUT;
}

/****************************************************************************
*
*   PROCEDURE NAME - v6_BC_ReadNextMessage()
*
*   FUNCTION
*       This function reads the next BC message that meets the criteria set
*       by the passed parameters.  The arguments include a timeout value in
*       in milliseconds. If there are no BC messages put onto the queue this
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_MBLOCK_NOMATCH;  -> Bad BC Messno
*       API_BC_READ_TIMEOUT     -> Timeout before data read
*       API_HW_IQPTR_ERROR      -> interrupt Queue pointer error
*
*****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_INT status;
   IQ_MBLOCK_V6 intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT addr;

   BT_U32BIT iq_addr;                  // Hardware Interrupt Queue Pointer
   BT1553_COMMAND cmd;
   BT_U32BIT beg;                      // Beginning of the interrupt queue
   BT_U32BIT end;                      // End of the interrupt queue
  
   BT_U32BIT data_buffer_addr;
   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_U32BIT start;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   //start = timeGetTime();
   start = CEI_GET_TIME();

   // Get range of byte addresses for interrupt queue 
   beg = BTMEM_IQ_V6;
   end = BTMEM_IQ_V6_NEXT - 1;

   iq_addr = vbtGetRegister[cardnum](cardnum,INT_QUE_PTR_REG);
   iq_addr = REL_ADDR(cardnum,iq_addr);

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_sw = iq_addr;

   /************************************************
   *  Loop until timeout
   ************************************************/

   do
   {
      /* Read current location in interupt queue */
	  iq_addr = vbtGetRegister[cardnum](cardnum,INT_QUE_PTR_REG);

      // Convert the hardware word address to a byte address.
      iqptr_hw = iq_addr;
      iqptr_hw = REL_ADDR(cardnum,iqptr_hw);
      // If the pointer is outside of the interrupt queue abort.V4.09.ajh
      if ( (iqptr_hw < beg) || (iqptr_hw > end) )
         return API_HW_IQPTR_ERROR;

      /**********************************************************************
      *  Process all HW Interrupt Queue entries that have been written
      *  Start with the SW interrupt pointer from the last time.
      **********************************************************************/

      while ( iqptr_sw != iqptr_hw )
      {
         /*******************************************************************
         *  Get the 2 word interrupt block pointed to by iqptr_sw.
         *******************************************************************/
         vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&intqueue, iqptr_sw, wsizeof(intqueue));

         // We only care about BC interrupt here.
         if ( intqueue.mode == BC_MESSAGE_INTERRUPT)
         {
            data_buffer_addr = intqueue.msg_ptr;
            /*******************************************************************
            *  Read the data buffer to get message buffer pointer
            *  Then read the command word from the message buffer
            *******************************************************************/
            vbtReadRelRAM32[cardnum](cardnum,&addr,data_buffer_addr,1);
            vbtReadRelRAM[cardnum](cardnum,(BT_U16BIT *)&cmd, addr + BC_MSG_CMD1_OFFSET,1); 

            if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
			{
			   if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			   {
                  if((tr == DONT_CARE) || (tr == cmd.tran_rec))
				  {
                     status = BusTools_BC_MessageBufferRead(cardnum, REL_ADDR(cardnum,data_buffer_addr), pBC_mbuf);  //Reads message buffer base data buffer address
				     return status;
				  }
			   }
			}
         }

         iqptr_sw+=sizeof(IQ_MBLOCK_V6); // Chain to next entry
         if(iqptr_sw == BTMEM_IQ_V6_NEXT)
            iqptr_sw = BTMEM_IQ_V6;

      }
   }while((CEI_GET_TIME() - start) < timeout);
   
   return API_BC_READ_TIMEOUT;
}

/****************************************************************************
*
*   PROCEDURE NAME - v5_BC_ReadNextMessage()
*
*   FUNCTION
*       This function reads the next BC message that meets the criteria set
*       by the passed parameters.  The arguments include a timeout value in
*       in milliseconds. If there are no BC messages put onto the queue this
*
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_MBLOCK_NOMATCH;  -> Bad BC Messno
*       API_BC_READ_TIMEOUT     -> Timeout before data read
*       API_HW_IQPTR_ERROR      -> interrupt Queue pointer error
*
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_ReadNextMessage(int cardnum,BT_UINT timeout,BT_INT rt_addr,
									   BT_INT subaddress,BT_INT tr, API_BC_MBUF *pBC_mbuf)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_ReadNextMessage[cardnum](cardnum,timeout,rt_addr,subaddress,tr,pBC_mbuf);
}
#endif

/****************************************************************************
*
*   PROCEDURE NAME - v5_BC_ReadLastMessage()
*
*   FUNCTION
*       This function reads the last BC message that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/
NOMANGLE BT_INT CCONV v5_BC_ReadLastMessage(
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   BT_INT status;
   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.
   BT1553_COMMAND cmd;
   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT iqptr_cur;
   BT_U32BIT mess_addr;

   BT_U32BIT msg_blk_size;
   BT_UINT   messno;

   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_UINT queue_entry=sizeof(IQ_MBLOCK);//6;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % queue_entry != 0 ) )
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   iqptr_cur = iqptr_hw;
   
   /************************************************
   *  Loop until all the message are checked
   ************************************************/

   while ( iqptr_sw != iqptr_cur )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
	  if(iqptr_cur == BTMEM_IQ)
         iqptr_cur = BTMEM_IQ_NEXT - queue_entry;
      else
	     iqptr_cur = iqptr_cur - queue_entry;      
	   
	   vbtRead(cardnum,(LPSTR)&intqueue, iqptr_cur, sizeof(intqueue));

      // We only care about RT interrupt here.
      if ( intqueue.t.mode.bc )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;
         /*******************************************************************
         *  Calculate index based on starting address & size.
         *******************************************************************/
         msg_blk_size = sizeof(BC_MESSAGE) + (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

         mess_addr <<= 3;     // Align to 8-word boundry.
         messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

         if ( messno >= bc_mblock_count[cardnum] )
            return API_BC_MBLOCK_NOMATCH;
         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));

         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {
                  status = BusTools_BC_MessageRead(cardnum, messno, pBC_mbuf);
				  if(status)
                     return status;
				  else
					 break;
			   }
			}
		 }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_hw;
   if(iqptr_sw == iqptr_hw)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - v6_BC_ReadLastMessage()
*
*   FUNCTION
*       This function reads the last BC message that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/
NOMANGLE BT_INT CCONV v6_BC_ReadLastMessage(
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   BT_INT status;
   IQ_MBLOCK_V6 intqueue;                 // Buffer for reading single HW IQ entry.
   BT1553_COMMAND cmd;
   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_U32BIT iqptr_cur;
   BT_U32BIT data_buffer_addr, addr;

   BT_INT DONT_CARE = -1;
   BT_UINT bit = 1;
   BT_UINT queue_entry=sizeof(IQ_MBLOCK_V6);//6;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   iqptr_hw = vbtGetRegister[cardnum](cardnum,HWREG_IQ_HEAD_PTR);
   iqptr_hw = REL_ADDR(cardnum,iqptr_hw);
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   if ( (iqptr_hw < BTMEM_IQ_V6) || (iqptr_hw >= BTMEM_IQ_V6_NEXT) ||
       ((iqptr_hw - BTMEM_IQ_V6) % queue_entry != 0 ) )
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   iqptr_cur = iqptr_hw;
   
   /************************************************
   *  Loop until all the message are checked
   ************************************************/

   while ( iqptr_sw != iqptr_cur )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
	  if(iqptr_cur == BTMEM_IQ_V6)
         iqptr_cur = BTMEM_IQ_V6_NEXT - queue_entry;
      else
	     iqptr_cur = iqptr_cur - queue_entry;      
	   
	   vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&intqueue, iqptr_cur, wsizeof(intqueue));

      // We only care about BC interrupt here.
      if ( intqueue.mode == BC_MESSAGE_INTERRUPT )
      {
         data_buffer_addr = intqueue.msg_ptr;
         /*******************************************************************
         *  Read the data buffer to get message buffer pointer
         *  Then read the command word from the message buffer
         *******************************************************************/
         vbtReadRelRAM32[cardnum](cardnum,&addr,data_buffer_addr,1);
         vbtReadRelRAM[cardnum](cardnum,(BT_U16BIT *)&cmd, addr + BC_MSG_CMD1_OFFSET,1);

         if((rt_addr == DONT_CARE) || (rt_addr & (bit<<cmd.rtaddr)))
		 {
			if((subaddress == DONT_CARE) || (subaddress & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {
                  status = BusTools_BC_MessageBufferRead(cardnum, REL_ADDR(cardnum,data_buffer_addr), pBC_mbuf);
				  if(status)
                     return status;
				  else
					 break;
			   }
			}
		 }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_hw;
   if(iqptr_sw == iqptr_hw)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_ReadLastMessage()
*
*   FUNCTION
*       This function reads the last BC message that meets the criteria set
*       by the passed parameters.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessage(
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_ReadlastMessage[cardnum](cardnum,rt_addr,subaddress,tr,pBC_mbuf);
}

/****************************************************************************
*
*   PROCEDURE NAME - v5_BC_ReadLastMessageBlock()
*
*   FUNCTION
*       This function reads all the  BC messages that meets the criteria set
*       by the passed parameters that are stored in the interrupt queue between
*       the queue head pointer and the tail pointer.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV v5_BC_ReadLastMessageBlock(
   BT_INT cardnum,
   BT_INT rt_addr_mask, 
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/

   IQ_MBLOCK intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_UINT   messno;
   BT_UINT   msg_cnt;
   BT_U32BIT mess_addr;
   BT1553_COMMAND cmd;
   BT_U32BIT msg_blk_size;

   BT_U32BIT bit = 1;
   BT_INT DONT_CARE = -1;
 
   *mcount = msg_cnt = 0; // Clear out the count before returning any error

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   iqptr_hw = ((BT_U32BIT)vbtGetFileRegister(cardnum,RAMREG_INT_QUE_PTR)) << 1;
   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 ))   
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   /************************************************
   *  Loop until
   ************************************************/
   
   while ( iqptr_sw != iqptr_hw )
   {
      /*******************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
      vbtRead_iq(cardnum,(LPSTR)&intqueue, iqptr_sw, sizeof(intqueue));
      iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

      // We only care about BC interrupts here.
      if ( intqueue.t.mode.bc )
      {
         mess_addr = ((BT_U32BIT)intqueue.msg_ptr) << 1;

         /*******************************************************************
         *  Calculate index based on starting address & size.
         *******************************************************************/
         msg_blk_size = sizeof(BC_MESSAGE) + (bc_num_dbufs[cardnum] * bc_size_dbuf[cardnum]);

         mess_addr <<= 3;     // Align to 8-word boundry.
         messno = (BT_UINT)( (mess_addr - btmem_bc[cardnum]) / msg_blk_size);

         if ( messno >= bc_mblock_count[cardnum] )
            return API_BC_MBLOCK_NOMATCH;

         vbtRead(cardnum, (LPSTR)&cmd, mess_addr+2, sizeof(BT1553_COMMAND));

         if((rt_addr_mask == DONT_CARE) || (rt_addr_mask & (bit<<cmd.rtaddr)))
         {
            if((subaddr_mask == DONT_CARE) || (subaddr_mask & (bit<<cmd.subaddr)))
            {
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
               {
                  BusTools_BC_MessageRead(cardnum, messno, &pBC_mbuf[msg_cnt]);
                  msg_cnt++;
               }  
            }
         }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_sw;
   *mcount = msg_cnt;
   if(msg_cnt == 0)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - v6_BC_ReadLastMessageBlock()
*
*   FUNCTION
*       This function reads all the BC messages that meets the criteria set
*       by the passed parameters that are stored in the interrupt queue between
*       the queue head pointer and the tail pointer.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV v6_BC_ReadLastMessageBlock(
   BT_INT cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf)
{
   /************************************************
   *  Local variables
   ************************************************/
   BT_INT status;
   IQ_MBLOCK_V6 intqueue;                 // Buffer for reading single HW IQ entry.

   BT_U32BIT iqptr_hw;                 // Pointer to HW interrupt queue.
   BT_U32BIT iqptr_sw;                 // Previous HW interrupt queue ptr.
   BT_UINT   msg_cnt;
   BT_U32BIT data_buffer_addr;
   BT1553_COMMAND cmd;
   BT_U32BIT addr;

   BT_U32BIT bit = 1;
   BT_INT DONT_CARE = -1;
 
   *mcount = msg_cnt = 0; // Clear out the count before returning any error

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   // Convert the hardware word address to a byte address.
   //  and start at the current location in the queue.
   // If the pointer is outside of the interrupt queue abort.V4.09.ajh
   iqptr_hw = vbtGetRegister[cardnum](cardnum,HWREG_IQ_HEAD_PTR);
   iqptr_hw = REL_ADDR(cardnum,iqptr_hw);
   if ( (iqptr_hw < BTMEM_IQ_V6) || (iqptr_hw >= BTMEM_IQ_V6_NEXT) ||
       ((iqptr_hw - BTMEM_IQ_V6) % sizeof(IQ_MBLOCK_V6) != 0 ))   
      return API_HW_IQPTR_ERROR;

   iqptr_sw = iqptr_bc_last[cardnum];
   /************************************************
   *  Loop until
   ************************************************/
   
   while ( iqptr_sw != iqptr_hw )
   {
      /*******************************************************************
      *  Get the 2 word interrupt block pointed to by iqptr_sw.
      *******************************************************************/
      vbtReadRAM32[cardnum](cardnum,(BT_U32BIT *)&intqueue, iqptr_sw, 2);

      iqptr_sw += sizeof(IQ_MBLOCK_V6); // Chain to next entry
      if(iqptr_sw == BTMEM_IQ_V6_NEXT)
         iqptr_sw = BTMEM_IQ_V6;

      // We only care about BC interrupts here.
      if ( intqueue.mode == BC_MESSAGE_INTERRUPT )
      {
         data_buffer_addr = intqueue.msg_ptr;
         /*******************************************************************
         *  Read the data buffer to get message buffer pointer
         *  Then read the command word from the message buffer
         *******************************************************************/
         vbtReadRelRAM32[cardnum](cardnum,&addr,data_buffer_addr,1);
         vbtReadRelRAM[cardnum](cardnum,(BT_U16BIT *)&cmd,addr + BC_MSG_CMD1_OFFSET,1);

         if((rt_addr_mask == DONT_CARE) || (rt_addr_mask & (bit<<cmd.rtaddr)))
		 {
			if((subaddr_mask == DONT_CARE) || (subaddr_mask & (bit<<cmd.subaddr)))
			{
               if((tr == DONT_CARE) || (tr == cmd.tran_rec))
			   {
                  status = BusTools_BC_MessageBufferRead(cardnum, REL_ADDR(cardnum,data_buffer_addr), &pBC_mbuf[msg_cnt]);
				  if(status)
                     return status;
				  msg_cnt++;
			   }
			}
		 }
      }
   }
   iqptr_bc_last[cardnum] = iqptr_sw;
   *mcount = msg_cnt;
   if(msg_cnt == 0)
	   return API_BC_READ_NODATA;

   return API_SUCCESS;
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_ReadLastMessageBlock()
*
*   FUNCTION
*       This function reads all the BC messages that meets the criteria set
*       by the passed parameters that are stored in the interrupt queue between
*       the queue head pointer and the tail pointer.  
*
*   RETURNS
*       API_SUCCESS             -> success
*       API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*       API_BC_NOTINITED        -> RT_Init not yet called
*       API_BUSTOOLS_BADCARDNUM -> Bad card number
*       API_BC_READ_NODATA      -> No data matcning parameters
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_ReadLastMessageBlock(
   BT_INT cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf)
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_BC_ReadLastMessageBlock[cardnum](cardnum,rt_addr_mask,subaddr_mask,tr,mcount,pBC_mbuf);
}

/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_Checksum1760()
*
*   FUNCTION
*       This function Calculates a 1760 checksum for the data in and stores  
*       the result in the next location in the API_BC_MBUF.  
*
*   RETURNS
*       API_SUCCESS             -> success
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BC_Checksum1760(API_BC_MBUF *mbuf, BT_U16BIT *cksum)
{
	BT_U16BIT i, checksum=0, wd=0, temp=0, shiftbit=0, wdcnt=0;

    if((wdcnt = mbuf->mess_command1.wcount) == 0)
      wdcnt = 32;

	// Process each word in the data buffer.
	for (wd = 0; wd < (wdcnt-1); wd++) {
		temp = *mbuf->data[wd];

		// Cyclically right-shift the word by the word index.
		for (i=0; i<wd; i++) {
			shiftbit = temp & 0x0001;
			temp = temp >> 1;
			if (shiftbit) temp |= 0x8000;
		}

		// XOR the shifted word into the checksum.
		checksum ^= temp;
	}

	// Cyclically left-shift the checksum by the word index.
	for (i=0; i<wd; i++) {
		shiftbit = checksum & 0x8000;
		checksum = checksum << 1;
		if (shiftbit) checksum |= 0x0001;
	}

    *cksum = checksum;
    *mbuf->data[wdcnt-1] = checksum;

	return API_SUCCESS;
}

#if !defined( _LVRT_)
/****************************************************************************
*
*   PROCEDURE NAME - BusTools_BC_AutoIncrMessageData()
*
*   FUNCTION
*       This function sets up an interrupt to automatically increment a data
*       value in a specified BC message.  You supply the message number, start
*       value, increment value, increment rate, and max value.  You need to
*       both to start and stop the auto incrementing.  
*
*   RETURNS
*       API_SUCCESS         
*       API_BUSTOOLS_BADCARDNUM
*       API_BUSTOOLS_NOTINITED
*       API_BC_NOTMESSAGE
*       
*****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_BC_AutoIncrMessageData(BT_INT cardnum,BT_INT messno,BT_INT data_wrd,
                                                      BT_U16BIT start, BT_U16BIT incr, 
                                                      BT_INT rate, BT_U16BIT max, BT_INT sflag)
{
   int    status;

   API_BC_MBUF api_message;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bc_inited[cardnum] == 0)
      return API_BC_NOTINITED;

   /* Make sure we get an interrupt on this message */

   /***********************************************************************
   *  If the BC is starting, register a thread for the board.
   *  If the BC is shutting down, unregister the thread.
   **********************************************************************/
   if (sflag)
   {
      if(max == 0)
         max = 0xffff;

      if(pIntFIFO[cardnum][messno] != NULL)
      {
         return API_BC_AUTOINC_INUSE;
      }
           
      pIntFIFO[cardnum][messno] = (API_INT_FIFO *)CEI_MALLOC(sizeof(API_INT_FIFO));
      if(pIntFIFO[cardnum][messno] == NULL)
         return API_MEM_ALLOC_ERR;

      status = BusTools_BC_MessageRead(cardnum,messno,&api_message);
      if(status)
         return status;
      
      if ( (api_message.control & BC_CONTROL_TYPEMASK) != BC_CONTROL_MESSAGE )
         return API_BC_NOTMESSAGE;

      api_message.control &= ~BC_CONTROL_INTERRUPT;    // Turn off the interrupt
      api_message.control |= BC_CONTROL_INTERRUPT;     // Turn on the interrupt

      if( api_message.control & BC_CONTROL_BUFFERA)
         api_message.data[0][data_wrd] = start;
      if( api_message.control & BC_CONTROL_BUFFERB)
         api_message.data[1][data_wrd] = start;

      status = BusTools_BC_MessageWrite(cardnum,messno,&api_message);
      if(status)
         return status; 

      // Setup the FIFO structure for this board.
      memset(pIntFIFO[cardnum][messno], 0, sizeof(API_INT_FIFO));
      if(board_is_v5_uca[cardnum])
         pIntFIFO[cardnum][messno]->function     = bc_auto_incr;
      else
         pIntFIFO[cardnum][messno]->function     = bc_auto_multi_incr;
      pIntFIFO[cardnum][messno]->iPriority    = THREAD_PRIORITY_ABOVE_NORMAL;
      pIntFIFO[cardnum][messno]->dwMilliseconds = INFINITE;
      pIntFIFO[cardnum][messno]->iNotification  = 0;       // Dont care about startup or shutdown
      pIntFIFO[cardnum][messno]->FilterType     = EVENT_BC_MESSAGE;
      pIntFIFO[cardnum][messno]->nUser[0] = messno;  // Set the first user parameter to message number.
      pIntFIFO[cardnum][messno]->nUser[1] = data_wrd;
      pIntFIFO[cardnum][messno]->nUser[2] = incr;
      pIntFIFO[cardnum][messno]->nUser[3] = rate;
      pIntFIFO[cardnum][messno]->nUser[4] = 1; // use for counter
      pIntFIFO[cardnum][messno]->nUser[5] = max;
      pIntFIFO[cardnum][messno]->nUser[6] = start;

      pIntFIFO[cardnum][messno]->FilterMask[api_message.mess_command1.rtaddr][api_message.mess_command1.tran_rec]
                          [api_message.mess_command1.subaddr] = 0xffffffff;

      // Call the register function to register and start the BC thread.
      status = BusTools_RegisterFunction(cardnum, pIntFIFO[cardnum][messno], REGISTER_FUNCTION);
      if ( status )
         return status;
   }
   else
   {
      // Call the register function to unregister and stop the BC thread.
      if(pIntFIFO[cardnum][messno] == NULL)
         return API_NULL_PTR;
      status = BusTools_RegisterFunction(cardnum, pIntFIFO[cardnum][messno], UNREGISTER_FUNCTION);
      CEI_FREE(pIntFIFO[cardnum][messno]);
      pIntFIFO[cardnum][messno]=NULL;
   }
   return status;      // Have the API function continue normally
}

/*===========================================================================*
 * User ENTRY POINT:        B C _ A U T O _ I N C R
 *===========================================================================*
 *
 * FUNCTION:    bc_auto_incr()
 *
 * DESCRIPTION: This function increments the pre-defined data word.
 *
 * It will return:
 *   API_SUCCESS - Thread continues execution
 *===========================================================================*/

BT_INT _stdcall bc_auto_incr(
   BT_UINT cardnum,
   struct api_int_fifo *sIntFIFO)
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U16BIT   data[33];       // Data buffer we update
   BT_INT      tail;           // FIFO Tail index
   BT_UINT     messno;         // Message number to be updated
   BT_INT      status;

   /***********************************************************************
   *  Loop on all entries in the FIFO.  Get the tail pointer and extract
   *   the FIFO entry it points to.   When head == tail FIFO is empty
   ***********************************************************************/
   tail = sIntFIFO->tail_index;
   while ( tail != sIntFIFO->head_index )
   {
      // Extract the buffer ID from the FIFO
      messno = sIntFIFO->fifo[tail].bufferID;
      if(messno == (BT_UINT)sIntFIFO->nUser[0])
      {   
         //  and read the message data from the board.
         status = BusTools_BC_MessageReadData(cardnum, messno, data);
         if ( status )
            return status;

         // Update the data buffer
         if(sIntFIFO->nUser[4] == sIntFIFO->nUser[3])
         {
            data[sIntFIFO->nUser[1]] += sIntFIFO->nUser[2];
            data[sIntFIFO->nUser[1]] = (data[sIntFIFO->nUser[1]] % sIntFIFO->nUser[5]);
            if(data[sIntFIFO->nUser[1]] == 0)
               data[sIntFIFO->nUser[1]] = sIntFIFO->nUser[6];
            sIntFIFO->nUser[4] = 1;
         }
         else
            sIntFIFO->nUser[4]++;

         // Now write the data back to the message buffer:
         status = BusTools_BC_MessageUpdate(cardnum, messno, data);
         if ( status )
            return status;
      }
      // Now update and store the tail pointer.
      tail++;                         // Next entry
      tail &= sIntFIFO->mask_index;   // Wrap the index
      sIntFIFO->tail_index = tail;    // Save the index
   }
   return API_SUCCESS;
}

/*===========================================================================*
 * User ENTRY POINT:        B C _ A U T O _ M U L T I _ I N C R
 *===========================================================================*
 *
 * FUNCTION:    bc_auto_multi_incr()
 *
 * DESCRIPTION: This function increments the pre-defined data word.
 *
 * It will return:
 *   API_SUCCESS - Thread continues execution
 *===========================================================================*/

BT_INT _stdcall bc_auto_multi_incr(
   BT_UINT cardnum,
   struct api_int_fifo *sIntFIFO)
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U16BIT   data[32],save_data; // Data buffer we update and data word
   BT_INT      tail;               // FIFO Tail index
   BT_UINT     messno;             // Message number to be updated
   BT_INT      status;
   BT_UINT     bufaddr;
   BT_INT      wcount;

   /***********************************************************************
   *  Loop on all entries in the FIFO.  Get the tail pointer and extract
   *   the FIFO entry it points to.   When head == tail FIFO is empty
   ***********************************************************************/
   tail = sIntFIFO->tail_index;
   while ( tail != sIntFIFO->head_index )
   {
      // Extract the buffer ID from the FIFO
      messno = sIntFIFO->fifo[tail].bufferID;
      wcount = sIntFIFO->fifo[tail].wordcount;
      if(wcount==0)
         wcount = 32;
 
      if(messno == (BT_UINT)sIntFIFO->nUser[0])
      {
         bufaddr = sIntFIFO->fifo[tail].buffer_off; 
         status = BusTools_BC_ReadDataBuffer(cardnum,bufaddr,data);  
         if ( status )
            return status;

         // Update the data buffer
         if(sIntFIFO->nUser[4] == sIntFIFO->nUser[3])
         {
            data[sIntFIFO->nUser[1]] += sIntFIFO->nUser[2];
            data[sIntFIFO->nUser[1]] = (data[sIntFIFO->nUser[1]] % sIntFIFO->nUser[5]);
            if(data[sIntFIFO->nUser[1]] == 0)
               data[sIntFIFO->nUser[1]] = sIntFIFO->nUser[6];
            sIntFIFO->nUser[4] = 1;
         }
         else
            sIntFIFO->nUser[4]++;

         save_data = data[sIntFIFO->nUser[1]];

         vbtReadRAM32[cardnum](cardnum,&bufaddr,bufaddr+BC_DATA_NEXT_OFFSET,1);
         
         vbtReadRAM[cardnum](cardnum,data,REL_ADDR(cardnum,bufaddr)+BC_DATA_DATA_OFFSET,32);
         data[sIntFIFO->nUser[1]] = save_data;

         // Now write the data back to the message buffer:
         status = BusTools_BC_DataBufferUpdate(cardnum,REL_ADDR(cardnum,bufaddr),wcount,data);
         if ( status )
            return status;
      }
      // Now update and store the tail pointer.
      tail++;                         // Next entry
      tail &= sIntFIFO->mask_index;   // Wrap the index
      sIntFIFO->tail_index = tail;    // Save the index
   }
   return API_SUCCESS;
}


#endif //_LVRT_

