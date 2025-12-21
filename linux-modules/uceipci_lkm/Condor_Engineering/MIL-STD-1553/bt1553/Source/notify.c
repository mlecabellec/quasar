/*============================================================================*
 * FILE:                     N O T I F Y . C
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
 * FUNCTION:   BusTools/1553-API Library:
 *             This module contains the interrupt processing functions for the
 *             BusTools Application Programmer's Interface (API).
 *
 * USER ENTRY POINTS: 
 *     BusTools_RegisterFunction - Registers a thread to be notified of user-
 *                                 specified events.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *     API_InterruptInit    - Performs initialization of interrupt structures
 *     vbtNotify            - Interrupt function which reads interrupt queue
 *                            and dispatches notification to threads
 *                            which are registered for events.
 *     DumpRegisterFIFO     - Dumps the FIFO's to a specified debug file.
 *     RegisterFunctionInit - Clear out the array of registered functions
 *     SignalUserThread     - Scan user threads for thread registered for event
 *
 * INTERNAL ROUTINES:
 *     LocalInterruptThreadFunction - Control function which calls user interrupts
 *     AddComplexSignaledEvent      - Decode and add event to user interrupt FIFO
 *
 *===========================================================================*/

/* $Revision:  8.22 $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/31/1999   Completed hardware interrupt support.V3.03.ajh
  11/10/1999   Added DumpRegisterFIFO() debug function, changed HW interrupt
               code to correctly support the PCI-1553.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  03/08/2000   Cleanup the LocalInterruptThreadFunction to clear out the FIFO
               entries if the user interrupt function returns an exit code.V4.01.ajh
  03/21/2000   Modified SignalUserThread() to correctly obtain the RT#, SA#,
               TR, WC and BUF# from the RT MBUF.V4.01.ajh
  06/01/2000   Timeout BusTools_RegisterFunction waiting for the user function
               to exit.V4.04.ajh
  12/07/2000   Changed vbtNotify to support Dynamic Bus Control mode codes.V4.27.ajh
  01/03/2001   Changed vbtNotify to avoid reading 1553 control register on a V5
               IP-1553 PROM on a PCI carrier, since it can reset the time tag.V4.30.ajh
  03/28/2001   Called the function that performs bus loading calculations.V4.32.ajh
  04/02/2001   Modified _RegisterFunction to not permit two entries to use the
               same user-specified FIFO structure.V4.35.ajh
  05/10/2001   Add interrupt support for Linux. v4.40.rhc
  01/15/2002   Added support for QPMC-1553 and IP-D1553. v4.46
  02/15/2002   Added support for modular API. v4.48
  02/10/2003   Added support for the QPCI-1553
  07/09/2003   Remove support for 16-Bit Windows interrupts and post message
  10/22/2003   Added selection for TX or RX message interrupts on RT-RT msgs.
  08/18/2005   Put error information into channel status
  12/29/2005   Modify interrupts and threads for commom CEI functions
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API
  10/22/2015   Added a mutex for process/thread synchronization. Modified
               DumpRegisterFIFO to output 64-bit addresses, and changed output
               of "Priority" and "Overflow" from hex to decimal.bch
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"
#include "lowlevel.h"

/****************************************************************************
*  Next interrupt addr.
****************************************************************************/
static BT_U32BIT addr_nextint[MAX_BTA]= {BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,    // Address of next HW interrupt block
                                         BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,
                                         BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,
                                         BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6,BTMEM_IQ_V6};

static BT_U32BIT addr_nextV5int[MAX_BTA]= {BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,    // Address of next HW interrupt block
                                         BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,
                                         BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,
                                         BTMEM_IQ,BTMEM_IQ,BTMEM_IQ,BTMEM_IQ};
/****************************************************************************
*     Local Variables
****************************************************************************/
static char bm_rec_trig[MAX_BTA];          // Trigger mode:
                                           //   0 -> not yet received
                                           //   1 -> trigger start encountered
                                           //   2 -> trigger stop encountered

// List of pointers to user-supplied API_INT_FIFO structures:
static API_INT_FIFO * pFIFO[MAX_BTA][MAX_REGISTER_FUNCTION];
static int nFIFO[MAX_BTA];    // Number of user registered functions.
static int error_reported = 0;
static BT_UINT ovfl_count[MAX_BTA];

#if defined(_LINUXBOARDSETUP) || defined(_LINUXVMEBOARDSETUP)
// mutex to synchronize spawning "LocalInterruptThreadFunction" thread
static CEI_MUTEX local_int_thrd_mtx[MAX_BTA][MAX_REGISTER_FUNCTION];
#endif

#ifdef INTEGRITY_THREAD_CREATE
void InterityInterruptThreadFunction(void);
#endif


/****************************************************************************
*
* PROCEDURE NAME:  API_InterruptInit
*
* FUNCTION
*     Initialize interrupt structures in 32-bit land.
*
*   Parameters
*     BT_UINT cardnum   = card number (0 - MAX_BTA-1)
*
****************************************************************************/
void API_InterruptInit(BT_UINT cardnum, BT_UINT flag)
{
   if(flag)
      nFIFO[cardnum] = 0;                    // Number of registered threads

   if(board_is_v5_uca[cardnum]) 
      addr_nextV5int[cardnum] = BTMEM_IQ;    // Beginning of interrupt queue
   else
      addr_nextint[cardnum] = BTMEM_IQ_V6;   // Beginning of interrupt queue   
}

/****************************************************************************
*
* PROCEDURE NAME:  BusTools_RegisterFunction
*
* FUNCTION:
*     An application thread calls this function to indicate its desire
*     to receive notification of any one of a list of specified events.
*     This notification is via an entry in a user-supplied FIFO structure,
*     and the signaling of an event object created by this function call.
*
*   Parameters

*     BT_UINT cardnum        = card number (0 - 3)
*     API_INT_FIFO *sIntFIFO = Pointer to Thread Interrupt Filter,
*                              Control and FIFO structure.
*     BT_UINT wFlag          = 0 -> "unregister" this function
*                              1 -> register this function, structure and FIFO
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM
*     API_BUSTOOLS_NOTINITED
*     API_BUSTOOLS_FIFO_BAD
*     API_BUSTOOLS_TOO_MANY
*     API_BUSTOOLS_NO_OBJECT
****************************************************************************/
 
NOMANGLE BT_INT CCONV BusTools_RegisterFunction(BT_UINT cardnum,
                                                API_INT_FIFO *sIntFIFO,
                                                BT_UINT wFlag)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   BT_INT      iFIFO=0;     // Index to current entry in the pFIFO array
   BT_INT      MaxWait=0;   // Time out waiting for user function to exit
   BT_INT      timeOutDelay=0, status=0;

   /*******************************************************************
   *  Do error checking
   *******************************************************************/
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(channel_status[cardnum].int_mode == API_MANUAL_INT)
      return API_REGISTERFUNCTION_OFF; //Cannot run with manual interrupt mode.

   /*******************************************************************
   *  Enable/Disable thread processing based on "wFlag"
   *******************************************************************/
   if ( wFlag == 0 )
   {
      bm_hw_queue_ovfl[cardnum] = 0;    // BM hw queue overflow flag. 
      bm_rec_trig[cardnum]      = 0;    // Trigger has not been received. 
 
      /****************************************************************
      *  Disable and destroy event thread.
      ****************************************************************/
      // Verify that thread structure and pointers are valid.
      iFIFO = sIntFIFO->nPtrIndex;
      if ( iFIFO > nFIFO[cardnum] )
         return API_BUSTOOLS_FIFO_BAD;         // Table index invalid

      // Verify that the cardnum in the struct is this cardnum.
      if ( cardnum != sIntFIFO->cardnum )
         return API_BUSTOOLS_FIFO_BAD;

      // Verify that our table entry points to the user-supplied structure
      if ( sIntFIFO != pFIFO[cardnum][iFIFO] )
         return API_BUSTOOLS_FIFO_BAD;

      // House keeping is complete, now we need to shutdown the thread.
      // First mark the thread for termination.
      sIntFIFO->bForceShutdown = 1;
      // Now raise its priority so we don't have to wait for it.
      CEI_EVENT_SIGNAL(&sIntFIFO->hEvent,&sIntFIFO->mutex);

      // pFIFO may never get cleared if the FIFO entries are corrupted, or if
      //  the user's thread function is misbehaved does not return.
      // Timeout waiting for the thread to exit.V4.04
      if(CurrentCardType[cardnum]==R15USB) 
         timeOutDelay=1000;
      else
         timeOutDelay=100;

      for ( MaxWait = 0; MaxWait < timeOutDelay; MaxWait++ )
      {
         MSDELAY(1);                           // Wait for the thread to exit
         if ( pFIFO[cardnum][iFIFO] == NULL )
         {
            channel_status[cardnum].int_fifo_count--;
            CEI_EVENT_DESTROY(&sIntFIFO->hEvent);
            CEI_MUTEX_DESTROY(&sIntFIFO->mutex);
            CEI_HANDLE_DESTROY(&sIntFIFO->hThread);
            return API_SUCCESS;              // Thread has exited successfully
         }
      }

      // The thread did not exit.  Kill the thread, close its handle and the event object handle.
      CEI_EVENT_DESTROY(&sIntFIFO->hEvent);
      CEI_MUTEX_DESTROY(&sIntFIFO->mutex);   
      CEI_THREAD_DESTROY(&sIntFIFO->hThread);

      // Close the handles.
      pFIFO[cardnum][iFIFO] = NULL;          // Termination complete.
      channel_status[cardnum].int_fifo_count--;
      return API_SUCCESS;
   }

   /*******************************************************************
   *  Create and enable event processing thread.
   *******************************************************************/
   // Search allocated area of list for duplicate FIFO entries.  If the
   //  user-specified FIFO structure is already registered, return error.

   if(hw_int_enable[cardnum] == API_MANUAL_INT)
      return API_REGISTERFUNCTION_OFF;

   for ( iFIFO = 0; iFIFO < nFIFO[cardnum]; iFIFO++ )
      if ( pFIFO[cardnum][iFIFO] == sIntFIFO )
         return API_BUSTOOLS_FIFO_DUP;       // New test in V4.35.ajh

   // Search allocated area of list for empty entries we can use.
   for ( iFIFO = 0; iFIFO < nFIFO[cardnum]; iFIFO++ )
      if ( pFIFO[cardnum][iFIFO] == NULL )
         break;

   // If no empty entries in table, look at using the next entry at the end.
   if ( iFIFO == nFIFO[cardnum] )
   {
      if ( iFIFO >= MAX_REGISTER_FUNCTION )
         return API_BUSTOOLS_TOO_MANY;
      // Clear the structure address in the table.
      pFIFO[cardnum][iFIFO] = NULL;
      nFIFO[cardnum] = iFIFO+1;
   }
   // Initialize the caller's structure data.

   sIntFIFO->nPtrIndex      = iFIFO;     // Cross check for corrupted structure.
   sIntFIFO->cardnum        = cardnum;   // For the user to use.
   sIntFIFO->numEvents      = 0;         // No events posted to this thread yet.
   sIntFIFO->queue_oflow    = 0;         // FIFO has not overflowed.
   sIntFIFO->head_index     = 0;         // FIFO is empty.
   sIntFIFO->tail_index     = 0;         // FIFO is empty.
   sIntFIFO->mask_index     = MAX_FIFO_LEN - 1;  // FIFO index wrap around mask.

   if(sIntFIFO->FilterType == EVENT_IMMEDIATE)
     bt_interrupt_enable[cardnum] = (unsigned char)EVENT_IMMEDIATE;  

   if(CEI_MUTEX_CREATE(&sIntFIFO->mutex) != BTD_OK)
     return API_BUSTOOLS_NO_OBJECT;
   if(CEI_EVENT_CREATE(&sIntFIFO->hEvent) != BTD_OK) {
     CEI_MUTEX_DESTROY(&sIntFIFO->mutex);
     return API_BUSTOOLS_NO_OBJECT;
   }
   
  #if defined(_LINUXBOARDSETUP) || defined(_LINUXVMEBOARDSETUP)
   if(CEI_MUTEX_CREATE(&local_int_thrd_mtx[cardnum][iFIFO]) != BTD_OK) {
     CEI_MUTEX_DESTROY(&sIntFIFO->mutex);
     CEI_EVENT_DESTROY(&sIntFIFO->hEvent);
     return API_BUSTOOLS_NO_OBJECT;
   }
   /* To synchronize with the LocalInterruptThreadFunction thread, the process
      sets the lock, creates the thread, sets the lock and then unlocks.  While
      the thread only unlocks.
   */
   CEI_MUTEX_LOCK(&local_int_thrd_mtx[cardnum][iFIFO]);
  #endif
   status = CEI_THREAD_CREATE(&sIntFIFO->hThread, sIntFIFO->iPriority,(void *)LocalInterruptThreadFunction,(void *)sIntFIFO);
  #if defined(_LINUXBOARDSETUP) || defined(_LINUXVMEBOARDSETUP)
   if(status == BTD_OK) {
     CEI_MUTEX_LOCK(&local_int_thrd_mtx[cardnum][iFIFO]);
     CEI_MUTEX_UNLOCK(&local_int_thrd_mtx[cardnum][iFIFO]);
   }
   CEI_MUTEX_DESTROY(&local_int_thrd_mtx[cardnum][iFIFO]);
  #endif
   if(status != BTD_OK) {
     CEI_EVENT_DESTROY(&sIntFIFO->hEvent);
     CEI_MUTEX_DESTROY(&sIntFIFO->mutex);
     return API_BUSTOOLS_NO_OBJECT;
   }  
   
   // Lastly, now that we are done, put the structure address in the table.
   // This enables the interrupt function to access this instance.
   pFIFO[cardnum][iFIFO] = sIntFIFO;
   
   channel_status[cardnum].int_fifo_count++;

   return API_SUCCESS;
}

/****************************************************************************
*
* PROCEDURE NAME:  RegisterFunctionClose
*
* FUNCTION:
*     During shutdown this function is called to "un-register" any user
*     threads which might still be active from a user call to the
*     BusTools_RegisterFunction() interface above.
*
*   Parameters
*     BT_UINT cardnum        = card number
*
*   Returns
*     Nothing
****************************************************************************/
void RegisterFunctionClose(BT_UINT cardnum)         // (i) card number
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int      iFIFO;     // Index to current entry in the pFIFO array

   for ( iFIFO = 0; iFIFO < MAX_REGISTER_FUNCTION; iFIFO++ )
   {
      /*************************************************************
      *  Find and close any open processing threads.
      *************************************************************/
      if ( pFIFO[cardnum][iFIFO] != NULL )
         BusTools_RegisterFunction(cardnum, pFIFO[cardnum][iFIFO], UNREGISTER_FUNCTION);
   }
}

/****************************************************************************
*
* PROCEDURE NAME:  RegisterFunctionOpen
*
* FUNCTION:
*     During initialization this function is called to clear out the array
*     of registered functions.
*
*   Returns
*     Nothing
****************************************************************************/
void RegisterFunctionOpen(BT_UINT cardnum)         // (i) card number
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int      iFIFO;     // Index to current entry in the pFIFO array

   api_polling_interval = TIMER_DEFAULT; //Default Polling Interval

   for ( iFIFO = 0; iFIFO < MAX_REGISTER_FUNCTION; iFIFO++ )
      pFIFO[cardnum][iFIFO] = NULL;

   ovfl_count[cardnum]=0;
   bm_rec_trig[cardnum]      = 0;    // Trigger has not been received.
}

/****************************************************************************
*
* PROCEDURE NAME:  DumpRegisterFIFO
*
* FUNCTION:
*       This procedure outputs a dump of the BusTools_RegisterFunction()
*       API_INT_FIFO FIFO structures.  It is a local helper
*       function for the BusTools_DumpMemory user-callable function.
*
*   Returns
*     nothing.
*
****************************************************************************/
void DumpRegisterFIFO(
   BT_UINT cardnum,         // (i) card number
   FILE   *hfMemFile)       // (i) handle of output file
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/
   int            iFIFO;    // Index to current entry in the pFIFO array
   int            i;        // Loop counter
   char iFIFO_ptr_addresses[128];

   typedef union {
       BT1553_COMMAND cmd;
       BT_U16BIT      data;
   } CW;
   
   CW cw;

   /*******************************************************************
   *  Log the contents of each active FIFO to the Dump File.
   *******************************************************************/
   // Search allocated area of list for empty entries we can use.
   fprintf(hfMemFile, "\nAPI_INT_FIFO entries(%d):\n", nFIFO[cardnum]);
   for ( iFIFO = 0; iFIFO < nFIFO[cardnum]; iFIFO++ )
   {
      if ( pFIFO[cardnum][iFIFO] == NULL )
         break;

      sprintf(iFIFO_ptr_addresses, "Func=@%p, FIFO=@%p",
     	pFIFO[cardnum][iFIFO]->function,      // Address of registered function
        pFIFO[cardnum][iFIFO]);               // Address of the FIFO structure.V4.35

      // Print out the elements of this FIFO entry.
      fprintf(hfMemFile,
         "Index=%2.2d, %s, Priority=%d, ms=%d, Notify=%d, Filter=%X, Start=%d, Shut=%d\n"
         "  card=%d, PtrIndex=%d, #Events=%d, Overflow=%d, Head=%d, Tail=%d, Mask=%X\n",
         iFIFO,                                // FIFO structure index
         iFIFO_ptr_addresses,
         pFIFO[cardnum][iFIFO]->iPriority,     // Thread Priority
         pFIFO[cardnum][iFIFO]->dwMilliseconds,// Interval at which function is polled
         pFIFO[cardnum][iFIFO]->iNotification, // CALL_STARTUP | CALL_SHUTDOWN
         pFIFO[cardnum][iFIFO]->FilterType,    // One or more EVENT_ definitions
         pFIFO[cardnum][iFIFO]->bForceStartup, // 1 - Thread is being started, 0 complete
         pFIFO[cardnum][iFIFO]->bForceShutdown,// 1 - Thread is being shutdown, -1 complete
         pFIFO[cardnum][iFIFO]->cardnum,       // card number
         pFIFO[cardnum][iFIFO]->nPtrIndex,     // Index into API pointer table
         pFIFO[cardnum][iFIFO]->numEvents,     // Total number of events, including overflows
         pFIFO[cardnum][iFIFO]->queue_oflow,   // Count incremented by API when FIFO overflows
         pFIFO[cardnum][iFIFO]->head_index,    // Index of element being added to queue
         pFIFO[cardnum][iFIFO]->tail_index,    // Index of element to be removed from queue
         pFIFO[cardnum][iFIFO]->mask_index);   // Mask for wrapping head and tail pointers
      for ( i = 0; i < MAX_FIFO_LEN; i++ )
      {
         cw.cmd.rtaddr     = pFIFO[cardnum][iFIFO]->fifo[i].rtaddress;    
         cw.cmd.subaddr    = pFIFO[cardnum][iFIFO]->fifo[i].subaddress;   
         cw.cmd.wcount     = pFIFO[cardnum][iFIFO]->fifo[i].wordcount;    
         cw.cmd.tran_rec   = pFIFO[cardnum][iFIFO]->fifo[i].transrec;  

         fprintf(hfMemFile,
         " Entry=%2.2d, Event=%4X, bOffset=%8.8X, CW %04X. RT=%2d, SA=%2d, WC=%2d, T/R=%d, bufID=%08X, res=%8.8X\n",
                 i,
                 pFIFO[cardnum][iFIFO]->fifo[i].event_type,
                 RAM_ADDR(cardnum, pFIFO[cardnum][iFIFO]->fifo[i].buffer_off),
                 cw.data,
                 pFIFO[cardnum][iFIFO]->fifo[i].rtaddress,
                 pFIFO[cardnum][iFIFO]->fifo[i].subaddress,
                 pFIFO[cardnum][iFIFO]->fifo[i].wordcount,
                 pFIFO[cardnum][iFIFO]->fifo[i].transrec,
                 pFIFO[cardnum][iFIFO]->fifo[i].bufferID,
                 pFIFO[cardnum][iFIFO]->fifo[i].int_status);
      } 
   }
}

/****************************************************************************
*
* PROCEDURE NAME:  LocalInterruptThreadFunction
*
* FUNCTION:
*     This is the control function for an application thread which gets
*     created by a call to BusTools_RegisterFunction().  This thread waits
*     on an Event Object until signaled by the interrupt handler, indicating
*     that an event of interest to the thread has occured.  This function
*     then calls the user function, after performing some house keeping.
*     When the user function returns to this function we go back and wait
*     for the event again.
*
*   Parameters
*
*     API_INT_FIFO *sIntFIFO = Pointer to Thread Interrupt Filter,
*                              Control and FIFO structure.
*   Returns
*     nothing, thread distroys itself when complete.
*
****************************************************************************/
void * LocalInterruptThreadFunction(void *pFIFOLocal)
{
   BT_UINT       cardnum;     // (i) Card number this instance processes
   API_INT_FIFO *sIntFIFO;    // (i) Pointer to the FIFO structure for instance
   BT_INT        iFIFO;       // (i) Index to current entry in the pFIFO array
   BT_INT        status; 

   // Setup local pointer to Thread Interrupt Filter, Control and FIFO structure
   //  and card number for this instance of the user function.
   sIntFIFO = (API_INT_FIFO *)pFIFOLocal;
   cardnum  = sIntFIFO->cardnum;
   iFIFO    = sIntFIFO->nPtrIndex;

  #if defined(_LINUXBOARDSETUP) || defined(_LINUXVMEBOARDSETUP)
   CEI_MUTEX_UNLOCK(&local_int_thrd_mtx[cardnum][iFIFO]);
  #endif
   
   // This function runs until requested to terminate
   while ( !(sIntFIFO->bForceShutdown) )
   {
      // Wait until we get a hardware interrupt, then call the interrupt
      //  processing function.  Then wait for the next interrupt.
      if ( sIntFIFO->bForceStartup )
      {
         if ( (sIntFIFO->iNotification & CALL_STARTUP) )
         {
            sIntFIFO->bForceStartup = 0;             // Startup complete.
            if ( sIntFIFO->function(cardnum, sIntFIFO) != API_SUCCESS )
               break;
         }
      }

      status = CEI_WAIT_FOR_EVENT(&sIntFIFO->hEvent, sIntFIFO->dwMilliseconds, &sIntFIFO->mutex);
      if(status < 0) // Wait error condition skip data processing
      {
         channel_status[cardnum].err_info=0x4;
         continue;
      }
      channel_status[cardnum].err_info=0x0;
      sIntFIFO->timeout = status;

      // We have been signaled.
      // If this is a terminate wakeup, and the user doesn't want notification
      //  of termination, then just exit.
      if ( sIntFIFO->bForceShutdown )
      {
         if ( !(sIntFIFO->iNotification & CALL_SHUTDOWN) )
            break;                                // Just shutdown.
      }

      // Call the user thread.
      if ( sIntFIFO->function(cardnum, sIntFIFO) != API_SUCCESS )
         break;
   }

   /*******************************************************************
   *  Disable and destroy event thread, clearing out the FIFO entry.
   *******************************************************************/
   // Verify that our table entry points to the user-supplied structure
   if ( sIntFIFO == pFIFO[cardnum][iFIFO] )
   {
      // We need to remove this entry from the pFIFO
      //  list, while the interrupt function might be using the entry.
      // Let's leave NULL entries in the list, rather than try to re-arrange
      //  the list with the interrupt function running.
      pFIFO[cardnum][iFIFO] = NULL;   // No more interrupt function access,
                                      //  after its current pass.
      // If this is the last entry in the list, we can safely remove it,
      //  otherwise leave the NULL entry in the middle of the list.
      if ( iFIFO == nFIFO[cardnum]-1 )
         nFIFO[cardnum]--;
   }
   
   sIntFIFO->bForceShutdown = -1;     // Termination (nearly) complete.

   CEI_THREAD_EXIT(&status, &sIntFIFO->hThread);

   return 0;
}

/****************************************************************************
*
* PROCEDURE NAME:  AddComplexSignaledEvent
*
* FUNCTION
*     This routine gets called to post a specified complex event to all of the
*     threads which are waiting for that event.
*     It enters the interrupt type, RT address, etc., into the thread FIFO.
*
*   Parameters
*     BT_UINT   cardnum    - card being processed.
*     int       nType      - interrupt type to process
*     BT_UINT   rtaddress  - RT address
*     BT_UINT   Tr         - Transmit/Receive bit
*     BT_UINT   subaddress - Subaddress
*     BT_UINT   wordCount  - Message Word Count
*     BT_UINT   nBufferNum - Buffer number or Message number
*     BT_UINT   brdAddr    - Byte address of message buffer
*     BT_UINT   IQAddr     - Word address of interrupt queue entry
*
*   Returns
*     nothing
****************************************************************************/
static void AddComplexSignaledEvent(BT_UINT cardnum, 
                                    BT_UINT nType,
                                    BT_UINT rtaddress, 
                                    BT_UINT Tr,
                                    BT_UINT subaddress, 
                                    BT_UINT wordCount,
                                    BT_U32BIT int_status, 
                                    BT_UINT nBufferNum, 
                                    BT_UINT brdAddr, 
                                    BT_UINT IQAddr)
{
   int           i;              // Loop index
   API_INT_FIFO *sIntFIFO;       // Pointer to the FIFO structure for instance
   BT_INT        head;           // New FIFO Head index
   BT_INT        head_index;     // FIFO Head Index (where new data is added)
 
   for ( i = 0; i < nFIFO[cardnum]; i++ )
   {

      // If current entry not in use, skip it.
      if ( ( sIntFIFO = pFIFO[cardnum][i]) == NULL )
      {
         continue;
      }
      // If current entry not registered for this event, skip it.
      if ( (sIntFIFO->FilterType & nType) == 0 )
      {
         continue;
      }

      //If this is immediate then signal and exit  
      if(nType == EVENT_IMMEDIATE)
      {
         CEI_EVENT_SIGNAL(&sIntFIFO->hEvent,&sIntFIFO->mutex);
         return;
      }

      // If current entry is not the proper RT address, subaddress, TR and
      // word count, continue. 
      if ( nType & (EVENT_BC_MESSAGE | EVENT_BM_MESSAGE | EVENT_BM_TRIG | EVENT_RT_MESSAGE) )
      {
         if ( ( sIntFIFO->FilterMask[rtaddress][Tr][subaddress] &
                                              (1 << (wordCount&0x1F)) ) == 0 )
            continue;

         if(sIntFIFO->EventInit == NO_ERRORS)
         {
            if ((int_status & BT1553_INT_ERROR_BITS) != 0)
               continue;
         }

         if(sIntFIFO->EventInit == USE_INTERRUPT_MASK)
	     {  
            if ((sIntFIFO->EventMask[rtaddress][Tr][subaddress] & int_status) == 0)
               continue;
	     }
      }

      // Add this event to the FIFO.
      sIntFIFO->numEvents++;        // Another event posted/attempted to thread
      head_index = sIntFIFO->head_index;  // Where the new data goes.
      head = head_index + 1;              // New head pointer.
      head &= sIntFIFO->mask_index;       // Handle wrap-around of pointer.
      // If FIFO overflow count overflows, else add entry to FIFO

      if ( head == sIntFIFO->tail_index )
         sIntFIFO->queue_oflow++;
      else
      {
         sIntFIFO->fifo[head_index].int_status = int_status; // 32-bit interrupt status
         sIntFIFO->fifo[head_index].event_type = nType;      // Interrupt type
         sIntFIFO->fifo[head_index].buffer_off = brdAddr;    // Byte address of buffer
         sIntFIFO->fifo[head_index].rtaddress  = rtaddress;
         sIntFIFO->fifo[head_index].transrec   = Tr;
         sIntFIFO->fifo[head_index].subaddress = subaddress;
         sIntFIFO->fifo[head_index].wordcount  = wordCount;
         sIntFIFO->fifo[head_index].bufferID   = nBufferNum; // Buffer or message number
         sIntFIFO->head_index = head;                        // Update head pointer
         // FIFO has been updated, signal the thread.
         CEI_EVENT_SIGNAL(&sIntFIFO->hEvent,&sIntFIFO->mutex);
      }
   }
}

/****************************************************************************
*
* PROCEDURE NAME:  SignalV5UserThread
*
* FUNCTION
*     This routine gets called whenever there is an event detected in the
*     interrupt queue.
*     It scans the threads which have been registered and notifies all threads
*     which have registered for this interrupt type.
*
*   Parameters
*     BT_UINT   cardnum - card being processed.
*     int       nType   - interrupt type to process
*     BT_U16BIT brdAddr - byte address of message of board
*     BT_UINT   IQAddr     - Word address of interrupt queue entry
*
****************************************************************************/
void SignalV5UserThread(
   BT_UINT   cardnum,     // (i) Card number to single for this event
   BT_UINT   nType,       // (i) Interrupt event type to queue
   BT_U32BIT brdAddr,     // (i) Board address of messages causing the interrupt
   BT_UINT   IQAddr)      // (i) Interrupt queue address of this event
{
   BT_UINT        rtaddress=0;   // RT address of message being posted
   BT_UINT        Tr=0;          // RT Transmit/Receive of message being posted
   BT_UINT        subaddress=0;  // RT subaddress of message being posted
   BT_UINT        wordCount=0;   // RT word count/mode code number of message
   BT_U32BIT      int_status=0;  // interrupt status of message
   BT_UINT        nBufferNum;  // Buffer number of message being posted
   BT1553_COMMAND msg;         // Temp which holds 1553 cmd word from the board
   RT_MBUF_API    two;         // Buffer which reads the RT API MBUF data
#ifdef BM_EXPRESS
   BT_U16BIT      buf_addr;
   BT_U32BIT      bufaddr32;
#endif // BM_EXPRESS
   /* Only if timing trace is enabled */
   AddTrace(cardnum, NSIGNALUUSERTHREAD, nType,
            brdAddr >> hw_addr_shift[cardnum], 0, IQAddr, 0);

   /*******************************************************************
   *  If there are no registered user interrupt threads give it up.
   *******************************************************************/
   if ( (nFIFO[cardnum] == 0) && (DBC_Enable[cardnum] == 0) )
      return;             // Need to fall through if DBA enabled...V4.27

   /*******************************************************************
   *  Look up the additional parameters needed to process this call.
   *  These parameters depend on the interrupt type.
   *******************************************************************/
   switch ( nType )
   {
      case EVENT_IMMEDIATE: /* Immediate H/W only interrupt         */
         AddComplexSignaledEvent(cardnum, nType, 0, 0, 0, 0, 0, 0, 0, 0);
         return;
      default:
#ifdef BM_EXPRESS
      case EVENT_BM_SWAP:
         buf_addr = vbtGetFileRegister(cardnum,BM_BUFFER_START1);
         bufaddr32 = (BT_U32BIT)(buf_addr << hw_addr_shift[cardnum]);

         if(bufaddr32 == brdAddr)
         {
            //current active buffer is buffer 1 set buffer 2 for DMA
            buf_addr = vbtGetFileRegister(cardnum,BM_BUFFER_START2);
            brdAddr = (BT_U32BIT)(buf_addr << hw_addr_shift[cardnum]);
         }
         else
         {
            //current active buffer is buffer 2 set buffer 1 for DMA
            brdAddr = bufaddr32;
         }
         AddComplexSignaledEvent(cardnum, nType, 0, 0, 0, 0, 0, 0, brdAddr, IQAddr);       
         return;                 // No further information needed
#endif          
      case EVENT_TIMER_WRAP:     /* Timer has wrapped around (overflowed) */
      case EVENT_RECORDER:       /* BM recorder buffer has 64K or timeout */
      case EVENT_MF_OVERFLO:     /* Minor frame timing overflow           */
      case EVENT_API_OVERFLO:    /* API BM Recorder buffer overflowed     */
      case EVENT_EXT_TRIG:
         AddComplexSignaledEvent(cardnum, nType, 0, 0, 0, 0, 0, 0, brdAddr, IQAddr);
         return;                 // No further information needed
      case EVENT_BC_MESSAGE:     /* BC message transacted                 */
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         //  First the message ID:
         BusTools_BC_MessageGetid(cardnum, brdAddr>>(hw_addr_shift[cardnum]-1),
                                  &nBufferNum);

         // Now read the command word: 
         vbtRead(cardnum, (LPSTR)&msg, brdAddr+2, sizeof(msg));
         rtaddress  = msg.rtaddr;
         Tr         = msg.tran_rec;
         subaddress = msg.subaddr;
         wordCount  = msg.wcount;
         vbtRead(cardnum, (LPSTR)&int_status, brdAddr+9*2, sizeof(int_status));
         break;                  // RT address, subaddress, T/R, WC & buf #
      case EVENT_BC_CONTROL:     /* BC message transacted                 */
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         //  First the message ID:7
         BusTools_BC_MessageGetid(cardnum, brdAddr>>(hw_addr_shift[cardnum]-1),
                                  &nBufferNum);
         break;                  // RT address, subaddress, T/R, WC & buf #
      case EVENT_BM_MESSAGE:     /* BM message transacted                 */
      case EVENT_BM_TRIG:        /* BM trigger event (start/stop)         */
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         //  First the message ID:
         BusTools_BM_MessageGetid(cardnum, brdAddr>>(hw_addr_shift[cardnum]-1),
                                  &nBufferNum);
         // Now read the command word:
         vbtRead(cardnum, (LPSTR)&msg, brdAddr+8*2, sizeof(msg));
         rtaddress  = msg.rtaddr;
         Tr         = msg.tran_rec;
         subaddress = msg.subaddr;
         wordCount  = msg.wcount;
         vbtRead(cardnum, (LPSTR)&int_status, brdAddr+3*2, sizeof(int_status));

         if((int_status & BT1553_INT_RT_RT_FORMAT) && BM_INT_ON_RTRT_TX[cardnum])
         {
            vbtRead(cardnum, (LPSTR)&msg, brdAddr+10*2, sizeof(msg));
            rtaddress  = msg.rtaddr;
            Tr        = msg.tran_rec;
            subaddress = msg.subaddr;
            wordCount  = msg.wcount;
         }
         break;                  // RT address, subaddress, T/R, WC & buf #
      case EVENT_RT_MESSAGE:     /* RT message transacted                 */
         //*****************************************************************
         // This code is duplicated in BusTools_RT_MessageGetid().  We don't
         //  call the API function because of speed and because we need the
         //  word count, which it does not return.V4.01.ajh
         // Fetch message ID, RT address, subaddress, T/R and word count.
         //*****************************************************************
         // Get the RT command word to determine the RT, SA and TR.

         vbtRead(cardnum, (LPSTR)&msg, brdAddr+9*2, sizeof(msg));

         // Use the RT command word to determine the RT#, SA#, T/R and word count.
         // It must be valid since we are processing an interrupt from it!
         rtaddress  = msg.rtaddr;
         Tr         = msg.tran_rec;
         subaddress = msg.subaddr;
         wordCount  = msg.wcount;

	     vbtRead(cardnum, (LPSTR)&int_status, brdAddr+4*2, sizeof(int_status));

         // If this is a DBC (Dynamic Bus Control mode code 0) we need to see if
         //  DBA is enabled.  If so we need to turn on the BC here, and handle
         //  the RT...
         if ( wordCount == 0 )   // Possible Dynamic Bus Control Mode Code?
         {
            if ( (subaddress == 0) ||
                 (rt_sa31_mode_code[cardnum] && (subaddress == 31))  )
            {
               // Have a Dynamic Bus Control Mode Code.  Is DBA enabled?
               API_RT_ABUF   abuf;

               BusTools_RT_AbufRead(cardnum, rtaddress, &abuf);
               if ( abuf.inhibit_term_flag & RT_ABUF_DBC_ENA )
               {
                  // DBA is enabled.  Shut off the RT (or the entire RT sim)
                  //  and startup the BC.
                  if ( _HW_1Function[cardnum] )
                     BusTools_RT_StartStop(cardnum, 0);  // SF board; turn off RT sim
                  else
                  {
                     // Multi-function board.  Only turn off this one RT sim,
                     //  that is of course if the user wants it turned off!
                     if ( abuf.inhibit_term_flag & RT_ABUF_DBC_RT_OFF )
                     {
                        // Yes, the user wants us to turn this RT sim off:
                        abuf.enable_a = 0;
                        abuf.enable_b = 0;
                        BusTools_RT_AbufWrite(cardnum, rtaddress, &abuf);
                     }
                  }
                  BusTools_BC_StartStop(cardnum, 1);     // Start the BC
               }
            }
         }

         // Read the API-specific data from the end of the RT Message buffer
         //  to determine the RT MBUF number.
         vbtRead(cardnum, (LPSTR)&two, brdAddr+sizeof(RT_MBUF_HW), sizeof(two));
         nBufferNum = two.mess_bufnum;
         if ( two.mess_verify != RT_VERIFICATION )
         {
            rtaddress = API_RT_MBUF_NOMATCH;
            return;
         }
         break;                  // RT address, subaddress, T/R, WC & buf #
   }
   AddComplexSignaledEvent(cardnum, nType, rtaddress, Tr, subaddress,
                           wordCount, int_status, nBufferNum, brdAddr, IQAddr);
}

/****************************************************************************
*
* PROCEDURE NAME:  SignalUserThread
*
* FUNCTION
*     This routine gets called whenever there is an event detected in the
*     interrupt queue.
*     It scans the threads which have been registered and notifies all threads
*     which have registered for this interrupt type.
*
*   Parameters
*     BT_UINT   cardnum - card being processed.
*     int       nType   - interrupt type to process
*     BT_U16BIT brdAddr - byte address of message of board
*     BT_UINT   IQAddr     - Word address of interrupt queue entry
*
****************************************************************************/
void SignalUserThread(
   BT_UINT   cardnum,     // (i) Card number to single for this event
   BT_UINT   nType,       // (i) Interrupt event type to queue
   BT_U32BIT brdAddr,     // (i) Board address of messages causing the interrupt
   BT_UINT   IQAddr)      // (i) Interrupt queue address of this event
{
   BT_UINT        rtaddress=0;   // RT address of message being posted
   BT_UINT        Tr=0;          // RT Transmit/Receive of message being posted
   BT_UINT        subaddress=0;  // RT subaddress of message being posted
   BT_UINT        wordCount=0;   // RT word count/mode code number of message
   BT_U32BIT      int_status=0;  // interrupt status of message
   BT_UINT        nBufferNum;    // Buffer number of message being posted
   BT1553_COMMAND msg;           // Temp which holds 1553 cmd word from the board
   BM_CMD_WRD     bmmsg;
   RT_MBUF_API    two;           // Buffer which reads the RT API MBUF data
   BT_U32BIT      msgAddr;       
   BT_UINT        count=0;

   /*******************************************************************
   *  If there are no registered user interrupt threads give it up.
   *******************************************************************/
   if ( (nFIFO[cardnum] == 0) && (DBC_Enable[cardnum] == 0) )
      return;             // Need to fall through if DBA enabled...V4.27

   /*******************************************************************
   *  Look up the additional parameters needed to process this call.
   *  These parameters depend on the interrupt type.
   *******************************************************************/
   switch ( nType )
   {
      case EVENT_IMMEDIATE: /* Immediate H/W only interrupt         */
         AddComplexSignaledEvent(cardnum, nType, 0, 0, 0, 0, 0, 0, 0, 0);
         return;

      case EVENT_TIMER_WRAP:     /* Timer has wrapped around (overflowed) */
      case EVENT_RECORDER:       /* BM recorder buffer has 64K or timeout */
      case EVENT_MF_OVERFLO:     /* Minor frame timing overflow           */
      case EVENT_API_OVERFLO:    /* API BM Recorder buffer overflowed     */
      case EVENT_EXT_TRIG:
      case EVENT_BC_BSY_OVFL:
      case EVENT_LP_MF_OVFL:
      case EVENT_HP_MF_OVFL:
      case EVENT_BM_OVRFLW:
         AddComplexSignaledEvent(cardnum, nType, 0, 0, 0, 0, 0, 0, brdAddr, IQAddr);
         return;                 // No further information needed
      case EVENT_BC_MESSAGE:     /* BC message transacted                 */
         //Read the message buffer address
         vbtReadRAM32[cardnum](cardnum, &msgAddr, brdAddr + BC_DATA_MSG_OFFSET, 1);
         //Read the interrupt-status word
         vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&int_status, brdAddr + BC_DATA_STATUS_OFFSET, 1);
         //Read the buffer number (messno)
         vbtReadRelRAM32[cardnum](cardnum, &nBufferNum,msgAddr,1);
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         vbtReadRelRAM[cardnum](cardnum, (BT_U16BIT *)&msg, msgAddr + BC_MSG_CMD1_OFFSET, 1);
         rtaddress  = msg.rtaddr;
         Tr         = msg.tran_rec;
         subaddress = msg.subaddr;
         wordCount  = msg.wcount;
         
         break;                  // RT address, subaddress, T/R, WC & buf #
      case EVENT_BC_CONTROL:     /* BC message transacted                 */
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         //  First the message ID:7
         BusTools_BC_MessageGetid(cardnum, brdAddr,&nBufferNum);
         break;                  // RT address, subaddress, T/R, WC & buf #
      case EVENT_BM_MESSAGE:     /* BM message transacted                 */
      case EVENT_BM_TRIG:        /* BM trigger event (start/stop)         */
         // Fetch the message ID, RT address, subaddress, T/R and Word Count
         // First the message ID:
         nBufferNum=brdAddr;

         // Now read the command word:
         if(brdAddr + BM_CMD1_OFFSET >= btmem_bm_mbuf_next[cardnum])
         {  //BM_MBUF data is wrapped. 
            count = (brdAddr + BM_CMD1_OFFSET) - btmem_bm_mbuf_next[cardnum];
            vbtReadBMRAM32[cardnum](cardnum,  (BT_U32BIT *)&bmmsg, btmem_bm_mbuf[cardnum] + count, 1);
         }
         else
            vbtReadBMRAM32[cardnum](cardnum, (BT_U32BIT *)&bmmsg, brdAddr + BM_CMD1_OFFSET, 1);
       
         rtaddress  = bmmsg.command.rtaddr;
         Tr         = bmmsg.command.tran_rec;
         subaddress = bmmsg.command.subaddr;
         wordCount  = bmmsg.command.wcount;

         if(brdAddr + BM_INT_STATUS_OFFSET >= btmem_bm_mbuf_next[cardnum])
         {  //BM_MBUF data is wrapped.
            count = (brdAddr + BM_INT_STATUS_OFFSET) - btmem_bm_mbuf_next[cardnum];
            vbtReadBMRAM32[cardnum](cardnum,  (BT_U32BIT *)&int_status, btmem_bm_mbuf[cardnum] + count, 1);
         }
         else
            vbtReadBMRAM32[cardnum](cardnum, (BT_U32BIT *)&int_status, brdAddr + BM_INT_STATUS_OFFSET, 1);

         if((int_status & BT1553_INT_RT_RT_FORMAT) && BM_INT_ON_RTRT_TX[cardnum])
         {
            if(brdAddr + BM_CMD2_OFFSET >= btmem_bm_mbuf_next[cardnum])
            {  //BM_MBUF data is wrapped.
               count = (brdAddr + BM_CMD2_OFFSET) - btmem_bm_mbuf_next[cardnum];
               vbtReadBMRAM32[cardnum](cardnum,  (BT_U32BIT *)&bmmsg, btmem_bm_mbuf[cardnum] + count, 1);
            }
            else
               vbtReadBMRAM32[cardnum](cardnum, (BT_U32BIT *)&bmmsg, brdAddr + BM_CMD2_OFFSET, 1);

            rtaddress  = bmmsg.command.rtaddr;
            Tr         = bmmsg.command.tran_rec;
            subaddress = bmmsg.command.subaddr;
            wordCount  = bmmsg.command.wcount;
         }

         break;                  
      case EVENT_RT_MESSAGE:     /* RT message transacted                 */
         //*****************************************************************
         // This code is duplicated in BusTools_RT_MessageGetid().  We don't
         //  call the API function because of speed and because we need the
         //  word count, which it does not return.V4.01.ajh
         // Fetch message ID, RT address, subaddress, T/R and word count.
         //*****************************************************************
         // Get the RT command word to determine the RT, SA and TR.

         vbtReadRAM[cardnum](cardnum, (BT_U16BIT *)&msg, brdAddr + RT_CMD_OFFSET, 1);

         // Use the RT command word to determine the RT#, SA#, T/R and word count.
         // It must be valid since we are processing an interrupt from it!
         rtaddress  = msg.rtaddr;
         Tr         = msg.tran_rec;
         subaddress = msg.subaddr;
         wordCount  = msg.wcount;

         vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&int_status, brdAddr+RT_INT_STATUS_OFFSET, 1);

         // If this is a DBC (Dynamic Bus Control mode code 0) we need to see if
         //  DBA is enabled.  If so we need to turn on the BC here, and handle
         //  the RT...
         if ( wordCount == 0 )   // Possible Dynamic Bus Control Mode Code?
         {
            if ( (subaddress == 0) ||
                 (rt_sa31_mode_code[cardnum] && (subaddress == 31))  )
            {
               // Have a Dynamic Bus Control Mode Code.  Is DBA enabled?
               API_RT_ABUF   abuf;

               BusTools_RT_AbufRead(cardnum, rtaddress, &abuf);
               if ( abuf.inhibit_term_flag & RT_ABUF_DBC_ENA )
               {
                  // DBA is enabled.  Shut off the RT (or the entire RT sim)
                  //  and startup the BC.
                  if ( _HW_1Function[cardnum] )
                     BusTools_RT_StartStop(cardnum, RT_STOP);  // SF board; turn off RT sim
                  else
                  {
                     // Multi-function board.  Only turn off this one RT sim,
                     //  that is of course if the user wants it turned off!
                     if ( abuf.inhibit_term_flag & RT_ABUF_DBC_RT_OFF )
                     {
                        // Yes, the user wants us to turn this RT sim off:
                        abuf.enable_a = 0;
                        abuf.enable_b = 0;
                        BusTools_RT_AbufWrite(cardnum, rtaddress, &abuf);
                     }
                  }
                  BusTools_BC_StartStop(cardnum, BC_START);     // Start the BC
               }
            }
         }

         // Read the API-specific data from the end of the RT Message buffer
         //  to determine the RT MBUF number.
         vbtReadRAM32[cardnum](cardnum, (BT_U32BIT *)&two, brdAddr+sizeof(RT_MBUF_V6HW), RT_MBUF_API_DWSIZE);
         flip((BT_U32BIT *)&two.mess_id);
         flip((BT_U32BIT *)&two.mess_statuswd);

         nBufferNum = two.mess_bufnum;     
         
         if ( two.mess_verify != RT_VERIFICATION )
         {
            rtaddress = API_RT_MBUF_NOMATCH;
            return;
         }
         break;                  // RT address, subaddress, T/R, WC & buf #
         default:
            return;  //Unrecognized interrupt
   }
   AddComplexSignaledEvent(cardnum, nType, rtaddress, Tr, subaddress,
                           wordCount, int_status, nBufferNum, brdAddr, IQAddr);
}

/****************************************************************************
*
* PROCEDURE NAME:  v6Notify (vbtNotify)
*
* FUNCTION
*     This routine gets called every 10 milliseconds or when a HW interrupt
*     occurs.  This routine checks the interrupt queue on the board.  If
*     there are new entries and the item (BC, BM, specific RT) has been
*     registered, a message will be posted to the associated window.
*     The address of the item in the hardware memory is sent with the message.
*
*     NOTE:  This routine is called by interrupt service.  Therefore, there
*     are only a few things it can do (and a very few WINDOWS or system
*     functions it can call).
*
*     THIS ROUTINE IS VERY TIME CRITICAL ! ! !
*
****************************************************************************/

void v6Notify(
   BT_UINT cardnum,      // (i) card number of board/channel to process
   BT_U32BIT   *bmLocalRecordTime) // (i) Time to call BM recorder if > BM_RECORD_TIME
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   static unsigned bmRecordTimePrev[MAX_BTA]= {0,0,0,0,
                                               0,0,0,0,
                                               0,0,0,0,
                                               0,0,0,0}; // Previous value of bmRecordTime[0]


   unsigned    i;                     // Loop index.
   BT_U32BIT   msg_addr;              // Address of buffer giving interrupt.
   BT_U32BIT   bmaddr=0;              // This for the BM tail pointer update on H/W interrupts
   BT_U32BIT   iqptr_hw;              // Pointer to HW interrupt queue.
   BT_U32BIT   iqptr_sw;              // Previous HW interrupt queue ptr.
   BT_U32BIT   rt_address_buffer;     // Address of current RT addr buffer.
   RT_MBUF_API rt_api;                // Buffer for reading TA from RT buffer.
   IQ_MBLOCK_V6   intqueue;           // Buffer for reading single HW IQ entry.
   BT_U32BIT   iqptr_current=0;       // Current HW interrupt queue WORD offset.

#ifdef FILE_SYSTEM   
   char        szMsg[200];            // Local error message buffer
#endif //FILE_SYSTEM
     
#ifdef INCLUDE_USB_SUPPORT
   if(CurrentCardType[cardnum] == R15USB)
   {
      BT_INT retval;
      retval = usb_get_int_data(cardnum);
      if(retval)
         return;
      if ( bm_running[cardnum] )
      {
         retval = usb_get_bm_data(cardnum);  // the new messages since last update
         if(retval)
            return;
      }
   }
#endif

   /***********************************************************************
   *  Process the card, if it is enabled and properly setup.
   ***********************************************************************/

   if(bt_interrupt_enable[cardnum] == EVENT_IMMEDIATE)
   {
      API_INT_FIFO *sIntFIFO;       // Pointer to the FIFO structure for instance
      BT_INT        head;           // New FIFO Head index
      BT_INT        head_index;     // FIFO Head Index (where new data is added)

      if(( sIntFIFO = pFIFO[cardnum][0]) == NULL )
      {
         return;
      }
      head_index = sIntFIFO->head_index;  // Where the new data goes.
      head = head_index + 1;               // New head pointer.
      head &= sIntFIFO->mask_index;       // Handle wrap-around of pointer.
     
      sIntFIFO->fifo[head_index].event_type = EVENT_IMMEDIATE;      // Interrupt type
      sIntFIFO->head_index = head;                                  // Update head pointer

      sIntFIFO->function(cardnum, sIntFIFO);

      return;
   }

   /***********************************************************************
   *  Get current HW interrupt queue address from hardware register.
   *  Verify that the HW interrupt queue address is within the queue.
   *  This is a test for bad hardware, firmware or microcode.
   ***********************************************************************/
   iqptr_hw = vbtGetIqHeadPtr[cardnum](cardnum);
  
   iqptr_hw = REL_ADDR(cardnum,iqptr_hw);

   if ( (iqptr_hw < BTMEM_IQ_V6) || (iqptr_hw >= BTMEM_IQ_V6_NEXT) ||
       ((iqptr_hw - BTMEM_IQ_V6) % sizeof(IQ_MBLOCK_V6) != 0 )       )
   {
      if ( !error_reported )   // Only dump/report one time V4.25.ajh
      {
         error_reported = 1;

#ifdef FILE_SYSTEM
         sprintf(szMsg,"Interrupt register(x53) not within/on Interrupt Queue\n"
                    "Cardnum=%i, iqptr_hw=%X, start=%X, end=%X (Word Offsets)\n"
                    "Dumping memory to C:\\BUSAPIIQ.DMP",
              cardnum, iqptr_hw>>1, BTMEM_IQ_V6>>1, BTMEM_IQ_V6_NEXT>>1);
         BusTools_DumpMemory(cardnum, 0xFFFFFFFF, "C:\\BUSAPIIQ.DMP", szMsg);
#endif //FILE_SYSTEM

         channel_status[cardnum].err_info |= 0x1;
      }
      return;
   }

   /***********************************************************************
   *  Process all HW Interrupt Queue entries that have been written by the
   *    board firmware, starting with the SW pointer from the last time.
   **********************************************************************/
   iqptr_sw = addr_nextint[cardnum];
   
   while ( iqptr_sw != iqptr_hw )
   {
      /********************************************************************
      *  Get the 2 word interrupt block pointed to by iqptr_sw.
      ********************************************************************/
      vbtReadIntQueue[cardnum](cardnum,(BT_U32BIT *)&intqueue,iqptr_sw);

      iqptr_current = iqptr_sw;                      // Save for logging

      iqptr_sw+=sizeof(IQ_MBLOCK_V6);                                   // Chain to next entry
      if(iqptr_sw == BTMEM_IQ_V6_NEXT)                  // If at the end of queue
         iqptr_sw = BTMEM_IQ_V6;                        // point to the start

      msg_addr = intqueue.msg_ptr;      
 
      switch(intqueue.mode)
      {
      case TTIMER_LOAD_INTERRUPT:
         sched_tt_clear[cardnum] = TimeTagInterrupt(cardnum);
         SignalUserThread(cardnum, EVENT_TIMER_WRAP, 0, iqptr_current);
         break;
      case BM_TRIGGER_INTERRUPT:
         bmaddr=msg_addr;
         SignalUserThread(cardnum, EVENT_BM_TRIG, REL_ADDR(cardnum,msg_addr), iqptr_current);
         // Trigger received, see if we are supposed to restart the
         //  trigger mechanism to generate multiple output triggers.
         if ( MultipleBMTrigEnable[cardnum] )
         {
            // Re-initialize the trigger buffer to enable another trigger.
            vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&BMV6TriggerSave[cardnum], BTMEM_BM_V6TBUF, BM_TBUF_ENH_DWSIZE);
            if ( bm_rec_trig[cardnum] )  // If we have already reported BM
               continue;                 //  trigger start, done.
         }

         // Tag this message with the Trigger Begin/End bit.
         if ( bm_rec_trig[cardnum] == 0 )
         {
            BT_U32BIT tstat;
            vbtReadBMRAM32[cardnum](cardnum, &tstat,REL_ADDR(cardnum,msg_addr) + BM_INT_STATUS_OFFSET,1);
            tstat |= BT1553_INT_TRIG_BEGIN;
            vbtWriteBMRAM32[cardnum](cardnum, &tstat,REL_ADDR(cardnum,msg_addr) + BM_INT_STATUS_OFFSET,1);
         }           
         else
         {
            BT_U32BIT tstat;
            vbtReadBMRAM32[cardnum](cardnum, &tstat,REL_ADDR(cardnum,msg_addr) + BM_INT_STATUS_OFFSET,1);
            tstat |= BT1553_INT_TRIG_END;
            vbtWriteBMRAM32[cardnum](cardnum, &tstat,REL_ADDR(cardnum,msg_addr) + BM_INT_STATUS_OFFSET,1);
         }

         bm_rec_trig[cardnum]++;    // Increment count of triggers.
         break;

      /********************************************************************
      *  Dispatch on BC/BM/RT interrupt.
      ********************************************************************/

      case BC_MESSAGE_INTERRUPT:

         SignalUserThread(cardnum, EVENT_BC_MESSAGE, REL_ADDR(cardnum,msg_addr), iqptr_current);
         if ( ext_trig_bc[cardnum] == BC_TRIGGER_IMMEDIATE)
            break;
         /*******************************************************************
         *  Check if we should reset the board and wait for another BC start
         *******************************************************************/
         if ( (vbtGetRegister[cardnum](cardnum, HWREG_CONTROL) & CR1_BCBUSY) == 0 )//
         {
            if ( ext_trig_bc[cardnum] == BC_TRIGGER_REPETITIVE )
            {
               api_clearhwcbits(cardnum, CR1_BCRUN);
            }
            /********************************************************************
            *  If this is a one-shot BC mode with external trigger, turn off
            *   the re-trigger bit.
            ********************************************************************/
            else if ( ext_trig_bc[cardnum] == BC_TRIGGER_ONESHOT )
            {
               api_clearhwcbits(cardnum, CR1_BC_EXT_SYNC);   /* Disable HW BC trig */
            }
         }
         break;
      case BC_CNTRLWD_INTERRUPT:
         SignalUserThread(cardnum, EVENT_BC_CONTROL, REL_ADDR(cardnum,msg_addr), iqptr_current);
         break;
      case BM_MESSAGE_INTERRUPT:
            SignalUserThread(cardnum, EVENT_BM_MESSAGE, REL_ADDR(cardnum,msg_addr), iqptr_current);
            bmaddr=msg_addr;
         break;
      case BM_HW_OVRFLW_INTERRUPT:
            SignalUserThread(cardnum, EVENT_BM_OVRFLW, REL_ADDR(cardnum,msg_addr), iqptr_current);
         break;      
      case RT_MESSAGE_INTERRUPT:         // Remote Terminal interrupt..
         // Offset to the beginning address of RT memory.
         SignalUserThread(cardnum, EVENT_RT_MESSAGE, REL_ADDR(cardnum,msg_addr), iqptr_current);
         // Read only the API-specific portion of the RT message buffer
         //  to determine the RT number, and if the status word needs to
         //  be changed.
         vbtReadRelRAM32[cardnum](cardnum, (BT_U32BIT *)&rt_api, msg_addr+RT_MBUF_HW_SIZE,RT_MBUF_API_DWSIZE);

         // See if this message updates the RT status word.
         if ( rt_api.mess_statuswd & RT_SET )
         {
            BT_U32BIT new_status;
            // Compute the pointer to the RT status word and change it.
            rt_address_buffer = BTMEM_RT_V6ABUF + (rt_api.mess_id.rtaddr * 2);
            new_status = rt_status_options[cardnum][rt_api.mess_id.rtaddr];
            new_status &= 0x0000ffff;
            new_status |= rt_api.mess_statuswd & RT_STATUSMASK_RES;
            vbtSetRegister[cardnum](cardnum, rt_address_buffer, new_status);
         }
         break;
      case TRIGGER_IN_INTERRUPT:
         SignalUserThread(cardnum, EVENT_EXT_TRIG, 0, iqptr_current);
         break;
      case MF_OVFL_INTERRUPT:
         channel_status[cardnum].mf_ovfl=1;   // report active
         SignalUserThread(cardnum, EVENT_MF_OVERFLO, 0, iqptr_current);
         ovfl_count[cardnum]++;
         break;
      case BC_BSY_MFOVFL_INTERRUPT: 
         SignalUserThread(cardnum, EVENT_BC_BSY_OVFL, 0, iqptr_current);
         break;
      case LP_MF_OVFL_INTERRUPT:
         SignalUserThread(cardnum, EVENT_LP_MF_OVFL, 0, iqptr_current);
         break;
      case HP_MF_OVFL_INTERRUPT:
         SignalUserThread(cardnum, EVENT_HP_MF_OVFL, 0, iqptr_current);
         break;
      default:
         continue;  // Ignore unknown interrupt type in queue.
      } //switch

   } // end while (iqptr_sw != iqptr_hw)

   addr_nextint[cardnum] = iqptr_sw; // Save new SW IQ pointer.

   /***********************************************************************
   *  The rest of this function should only be executed on a polling call,
   *   not a hardware interrupt!  Polling calls are identified by the fact
   *   that the bmRecordTime has been incremented by the caller.
   ***********************************************************************/
   if ((bmRecordTimePrev[cardnum] == *bmLocalRecordTime) && procBmOnInt[cardnum] == 0)
   {
      if(bmaddr>0)
         vbtSetRegister[cardnum](cardnum,HWREG_BM_TAIL_PTR,bmaddr);
      return;
   }

   /***********************************************************************
   *  Check to see if we should clear timetag wrap-around flag (if we
   *   are past the time that messages received before the wrap around
   *    would still be in the BM buffers).  Initial zero does not call.
   ***********************************************************************/
   bmRecordTimePrev[cardnum] = *bmLocalRecordTime;   // This is a polling call

   /***********************************************************************
   *  Copy the Bus Monitor message queue into the API buffer.
   ***********************************************************************/
   if ( bm_running[cardnum] )         // If the BM is running.
   {    
      BM_V6MsgReadBlock(cardnum);     // Copy BM HW queue into the API buffer.
      BM_BusLoadingFilter(cardnum);   // Update the bus loadings if the time is right

      /********************************************************************
      *  If it is time to call the recorder (or we need to call the recorder
      *  because there is 64 Kb of messages available), and this card has
      *  BM Recording enabled, signal to the specified process.
      ********************************************************************/
      i = (nAPI_BM_Head[cardnum] - nAPI_BM_Tail[cardnum] + NAPI_BM_V6BUFFERS[cardnum]) &
         (NAPI_BM_V6BUFFERS[cardnum]-1);

      if ( (*bmLocalRecordTime > BM_RECORD_TIME) || (i > BM_MAX_MSGS_PER_READ_BLOCK) )
      {
         *bmLocalRecordTime = 0;                 // Restart recorder timer.
         bmRecordTimePrev[cardnum] = 0;          // Remember that we restarted.V4.37.ajh
         SignalUserThread(cardnum, EVENT_RECORDER, i, iqptr_current);
      }
   }  // end if (bm_running ...
   return;
}

/****************************************************************************
*
* PROCEDURE NAME:  v5Notify (vbtNotify)
*
* FUNCTION
*     This routine gets called every 10 milliseconds or when a HW interrupt
*     occurs.  This routine checks the interrupt queue on the board.  If
*     there are new entries and the item (BC, BM, specific RT) has been
*     registered, a message will be posted to the associated window.
*     The address of the item in the hardware memory is sent with the message.
*
*     NOTE:  This routine is called by interrupt service.  Therefore, there
*     are only a few things it can do (and a very few WINDOWS or system
*     functions it can call).
*
*     THIS ROUTINE IS VERY TIME CRITICAL ! ! !
*
****************************************************************************/

void v5Notify(
   BT_UINT cardnum,      // (i) card number of board/channel to process
   BT_U32BIT   *bmLocalRecordTime) // (i) Time to call BM recorder if > BM_RECORD_TIME
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   static unsigned bmRecordTimePrev[MAX_BTA]= {0,0,0,0,
                                               0,0,0,0,
                                               0,0,0,0,
                                               0,0,0,0}; // Previous value of bmRecordTime[0]

   int         temp;                  // Temp for external trigger of BC.
   unsigned    i;                     // Loop index.
   BT_U32BIT   msg_addr;              // Address of buffer giving interrupt.
   BT_U32BIT   iqptr_hw;              // Pointer to HW interrupt queue.
   BT_U32BIT   iqptr_sw;              // Previous HW interrupt queue ptr.
   BT_U32BIT   rt_address_buffer;     // Address of current RT addr buffer.
   RT_MBUF_API rt_api;                // Buffer for reading TA from RT buffer.
   IQ_MBLOCK   intqueue;              // Buffer for reading single HW IQ entry.
   BT_U32BIT   iqptr_current=0;       // Current HW interrupt queue WORD offset.
   BT_U16BIT   rvalue;

#ifdef FILE_SYSTEM   
   char        szMsg[200];            // Local error message buffer
#endif //FILE_SYSTEM

   /***********************************************************************
   *  Process the card, if it is enabled and properly setup.
   ***********************************************************************/
   if(board_is_paged[cardnum])
      vbtAcquireFrameRegister(cardnum, 1); 

   if(bt_interrupt_enable[cardnum] == EVENT_IMMEDIATE)
   {
      API_INT_FIFO *sIntFIFO;       // Pointer to the FIFO structure for instance
      BT_INT        head;           // New FIFO Head index
      BT_INT        head_index;     // FIFO Head Index (where new data is added)

      if(( sIntFIFO = pFIFO[cardnum][0]) == NULL )
      {
         return;
      }
      head_index = sIntFIFO->head_index;  // Where the new data goes.
      head = head_index +1;               // New head pointer.
      head &= sIntFIFO->mask_index;       // Handle wrap-around of pointer.
     
      sIntFIFO->fifo[head_index].event_type = EVENT_IMMEDIATE;      // Interrupt type
      sIntFIFO->head_index = head;                        // Update head pointer
            // FIFO has been updated, signal the thread.

      sIntFIFO->function(cardnum, sIntFIFO);

      return;
   }

   /***********************************************************************
   *  Get current HW interrupt queue address from hardware register.
   *  Verify that the HW interrupt queue address is within the queue.
   *  This is a test for bad hardware, firmware or microcode.
   ***********************************************************************/

   iqptr_hw  = ((BT_U16BIT *)(bt_PageAddr[cardnum][2]))[RAMREG_INT_QUE_PTR];
   flipw(&iqptr_hw); // for PMC on PowerPC
   iqptr_hw = iqptr_hw<<1;

   if ( (iqptr_hw < BTMEM_IQ) || (iqptr_hw >= BTMEM_IQ_NEXT) ||
       ((iqptr_hw - BTMEM_IQ) % sizeof(IQ_MBLOCK) != 0 )       )
   {
      if(board_is_paged[cardnum])
         vbtAcquireFrameRegister(cardnum, 0);   // Release frame register if there is one
      if ( !error_reported )   // Only dump/report one time V4.25.ajh
      {
         error_reported = 1;

#ifdef FILE_SYSTEM
         sprintf(szMsg,"Interrupt register(x53) not within/on Interrupt Queue\n"
                    "Cardnum=%i, iqptr_hw=%X, start=%X, end=%X (Word Offsets)\n"
                    "Dumping memory to C:\\BUSAPIIQ.DMP",
              cardnum, iqptr_hw>>1, BTMEM_IQ>>1, BTMEM_IQ_NEXT>>1);
         BusTools_DumpMemory(cardnum, 0xFFFFFFFF, "C:\\BUSAPIIQ.DMP", szMsg);
#endif //FILE_SYSTEM

         channel_status[cardnum].err_info |= 0x1;
      }
      return;
   }

   /***********************************************************************
   *  Process all HW Interrupt Queue entries that have been written by the
   *    board firmware, starting with the SW pointer from the last time.
   **********************************************************************/
   iqptr_sw = addr_nextV5int[cardnum];

   /* Only if timing trace is enabled */
   AddTrace(cardnum, NVBTNOTIFY, bt_interrupt_enable[cardnum],
            0, *bmLocalRecordTime, iqptr_sw >> 1, iqptr_hw >> 1);
   
   while ( iqptr_sw != iqptr_hw )
   {
      /********************************************************************
      *  Get the 3 word interrupt block pointed to by iqptr_sw.
      ********************************************************************/
      vbtRead_iq(cardnum,(LPSTR)&intqueue,iqptr_sw,sizeof(intqueue));
      iqptr_current = iqptr_sw >> 1;                 // Save for logging
      iqptr_sw = ((BT_U32BIT)intqueue.nxt_int) << 1; // Chain to next entry

      /********************************************************************
      *  Handle time tag update by discrete input or
      *   sync mode code.  Schedule the call to TimeTagClearFlag
      *   based on the current polling interval.
      ********************************************************************/
      if ( intqueue.t.mode.timer )
      {
         sched_tt_clear[cardnum] = TimeTagInterrupt(cardnum);
         SignalV5UserThread(cardnum, EVENT_TIMER_WRAP, 0, iqptr_current);
         continue;   // Done with timetag overflow interrupt queue entry.
      }

      msg_addr = ((BT_U32BIT)intqueue.msg_ptr) << hw_addr_shift[cardnum]; // 8-Word to byte addr

      if ( intqueue.t.mode.bmtrig )
      {
         SignalV5UserThread(cardnum, EVENT_BM_TRIG, msg_addr, iqptr_current);
         // Trigger received, see if we are supposed to restart the
         //  trigger mechanism to generate multiple output triggers.
         if ( MultipleBMTrigEnable[cardnum] )
         {
            // Re-initialize the trigger buffer to enable another trigger.
            vbtWrite(cardnum, (LPSTR)&BMTriggerSave[cardnum], BTMEM_BM_TBUF, sizeof(BMTriggerSave[cardnum]));
            if ( bm_rec_trig[cardnum] )  // If we have already reported BM
               continue;                 //  trigger start, done.
         }
         // Tag this message with the Trigger Begin/End bit.
         if ( bm_rec_trig[cardnum] == 0 )
            vbtReadModifyWrite(cardnum, RAM, msg_addr+4*2,
                               (short)(BT1553_INT_TRIG_BEGIN>>16),
                               (short)(BT1553_INT_TRIG_BEGIN>>16));
         else
            vbtReadModifyWrite(cardnum, RAM, msg_addr+4*2,
                               (short)(BT1553_INT_TRIG_END>>16),
                               (short)(BT1553_INT_TRIG_END>>16));
         AddTrace(cardnum, NBM_TRIGGER_OCCUR, bm_rec_trig[cardnum], msg_addr, 0, 0, 0);
         bm_rec_trig[cardnum]++;    // Increment count of triggers.

         // Process the BM Message Interrupt bit in this queue entry.
      }

      /********************************************************************
      *  Dispatch on BC/BM/RT interrupt.
      ********************************************************************/
      if ( intqueue.t.mode.bc )
      {
         SignalV5UserThread(cardnum, EVENT_BC_MESSAGE, msg_addr, iqptr_current);
         /*******************************************************************
         *  Check if we should reset the board and wait for another BC start
         *******************************************************************/
         if ( (vbtGetHWRegister(cardnum, HWREG_CONTROL1) & CR1_BCBUSY) == 0 )//
         {
            if ( ext_trig_bc[cardnum] == BC_TRIGGER_REPETITIVE )
            {
               temp = vbtGetHWRegister(cardnum, HWREG_CONTROL1);//
               // Turn off the BC run bit.
               vbtSetHWRegister(cardnum, HWREG_CONTROL1, (BT_U16BIT)(temp&0xFFFC));
            }
            /********************************************************************
            *  If this is a one-shot BC mode with external trigger, turn off
            *   the re-trigger bit.
            ********************************************************************/
            else if ( ext_trig_bc[cardnum] == BC_TRIGGER_ONESHOT )
            {
               api_writehwreg(cardnum,HWREG_BC_EXT_SYNC,0x0000);  /* Disable HW BC trigger */ 
            }
         }
      }
      if ( intqueue.t.mode.bc_ctl )
      {
         SignalV5UserThread(cardnum, EVENT_BC_CONTROL, msg_addr, iqptr_current);
      }
      else if ( intqueue.t.mode.bm )
      {
         SignalV5UserThread(cardnum, EVENT_BM_MESSAGE, msg_addr, iqptr_current);
      }
      else if ( intqueue.t.mode.rt )
      {                                 // Remote Terminal interrupt.
         // Offset to the beginning address of RT memory.
         SignalV5UserThread(cardnum, EVENT_RT_MESSAGE, msg_addr, iqptr_current);
         // Read only the API-specific portion of the RT message buffer
         //  to determine the RT number, and if the status word needs to
         //  be changed.
         vbtRead(cardnum, (LPSTR)&rt_api, msg_addr+sizeof(RT_MBUF_HW),
                 sizeof(RT_MBUF_API));

         // See if this message updates the RT status word.
         if ( rt_api.mess_statuswd & RT_SET )
         {
            // Compute the pointer to the RT status word and change it.
            rt_address_buffer = ((rt_api.mess_id.rtaddr) << 3) + 2 + BTMEM_RT_ABUF;
            vbtReadModifyWrite(cardnum, RAM, rt_address_buffer,
                               rt_api.mess_statuswd, RT_STATUSMASK);
         }
      }
      else if ( intqueue.t.mode.ext_trig)
      {
         SignalV5UserThread(cardnum, EVENT_EXT_TRIG, msg_addr, iqptr_current);
      }
#ifdef BM_EXPRESS
      else if ( intqueue.t.mode.bm_swap)
      {
         SignalV5UserThread(cardnum, EVENT_BM_SWAP, msg_addr, iqptr_current);
      }
#endif
      else
      {
         continue;        // Ignor unknown interrupt type in queue.
      }  // end if (intqueue.t.mode.bc)
   } // end while (iqptr_sw != iqptr_hw)

   addr_nextV5int[cardnum] = iqptr_sw; // Save new SW IQ pointer.


   /***********************************************************************
   *  The rest of this function should only be executed on a polling call,
   *   not a hardware interrupt!  Polling calls are identified by the fact
   *   that the bmRecordTime has been incremented by the caller.
   ***********************************************************************/
   if ((bmRecordTimePrev[cardnum] == *bmLocalRecordTime) && procBmOnInt[cardnum] == 0)
   {
      if(board_is_paged[cardnum])
         vbtAcquireFrameRegister(cardnum, 0); // Release frame register if there is one
      return;
   }

   /***********************************************************************
   *  Check to see if we should clear timetag wrap-around flag (if we
   *   are past the time that messages received before the wrap around
   *    would still be in the BM buffers).  Initial zero does not call.
   ***********************************************************************/
   bmRecordTimePrev[cardnum] = *bmLocalRecordTime;   // This is a polling call

   /***********************************************************************
   *  Check orphaned HW register to see if the Minor Frame
   *   has overflowed.
   ***********************************************************************/
   if(bc_running[cardnum])
   {
      if ((rvalue = vbtGetFileRegister(cardnum, RAMREG_ORPHAN)) & RAMREG_ORPHAN_MINORFRAME_OFLOW)
      {
         channel_status[cardnum].mf_ovfl=1;   // report active
         SignalV5UserThread(cardnum, EVENT_MF_OVERFLO, 0, iqptr_current);
         channel_status[cardnum].err_info |= CHAN_STAT_MF_OVFL;
         vbtSetFileRegister(cardnum,RAMREG_ORPHAN,(BT_U16BIT)(rvalue & 0xdbff)); //clear MFOVFL
         ovfl_count[cardnum]++;
      }
      else
      {
         channel_status[cardnum].mf_ovfl=0;
      }
   }

   /***********************************************************************
   *  Copy the Bus Monitor message queue into the API buffer.
   ***********************************************************************/
   if ( bm_running[cardnum] )         // If the BM is running.
   {
      BM_V5MsgReadBlock(cardnum);     // Copy BM HW queue into the API buffer.
      BM_BusLoadingFilter(cardnum);   // Update the bus loadings if the time is right

      /********************************************************************
      *  If it is time to call the recorder (or we need to call the recorder
      *  because there is 64 Kb of messages available), and this card has
      *  BM Recording enabled, signal to the specified process.
      ********************************************************************/
      i = (nAPI_BM_Head[cardnum] - nAPI_BM_Tail[cardnum] + NAPI_BM_BUFFERS) &
         (NAPI_BM_BUFFERS-1);

      if ( (*bmLocalRecordTime > BM_RECORD_TIME) || (i > BM_MAX_MSGS_PER_READ_BLOCK) )
      {
         *bmLocalRecordTime = 0;                    // Restart recorder timer.
         bmRecordTimePrev[cardnum] = 0;          // Remember that we restarted.V4.37.ajh
         SignalV5UserThread(cardnum, EVENT_RECORDER, i, iqptr_current);
      }
   }  // end if (bm_running ...

   if(board_is_paged[cardnum])
      vbtAcquireFrameRegister(cardnum, 0);   // Release frame register if any
   return;
}
