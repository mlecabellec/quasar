/*============================================================================*
 * FILE:                     P L A Y B A C K . C
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
 *             Bus Playback function.
 *
 *             This module read a specified .bmd file and controls the playback
 *             of the data in the file over the 1553 bus.
 *
 *             Playback work by reading data from a BusMonitor file (.bmd) and
 *             converting the data into a format used by the playback firmware 
 *             to recreate the signal on the 1553 bus.  
 *
 *             To do this three buffer are used to store the data.  The first 
 *             buffer, the disk buffer, stores the raw bus monitor data.  This 
 *             allows playback read a block of data from the disk.  Playback
 *             then converts this data into the required format and stores that 
 *             information into a lookahead buffer.  The data in the lookahead  
 *             buffer is then written to the buffer on the board.
 *             For large .bmd files a pipeline is created from the disk file  
 *             playback buffer on the PCI board.  The speed at which the PCI card 
 *             through the disk buffer and lookahead buffer to the processes 
 *             playback message determines the rate at which the playback buffer 
 *             is replenished.  The playback software setups up a timed interrupt
 *             that invokes the periodicUpdate routine. This routine checks for
 *             the end-of-playback and fills the playback buffer, lookahead   
 *             buffer and disk buffer as needed. 
 *
 * EXTERNAL ENTRY POINTS:
 *
 *    BusTools_Playback       Starts the playback process.
 *    BusTools_Playback_Check Checks the BMD file for time gap or time sequence problems.
 *    BusTools_Playback_Stop  Stops the playback process.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *  initPlayback    - Initializes Playback data and hardware
 *  setupDiskBuffer - Sets up the disk buffer used to store the raw playback data
 *  setupLookAheadBuffer  - Sets up the LookAhead Buffer used to store processed data
 *  setupPlaybackBuffer   - Sets up the playback buffer on teh PCI 1553 card
 *  setupThreadsAndEvents - Sets up the runPlayback Thread and events
 *  setupAPI_INT_FIFO     - Sets up the API_INT_FIFO used to setup the time interrupt
 *  setupTheLookAheadBuffer - Set up the look ahead buffer that stores formatted 
 *                            records
 *  setupThePlaybackBuffer  - Sets up the RAM playback buffer on the PCI 1553
 *  setUpThreadsAndEvents   - Sets upo the Threads and events used to control playback
 *  readLogFileBlock        - Reads a block of data form the disk file
 *  processTheRecord        - Processes on file record into playback format
 *  processBC_RT            - Formats a BC to RT (receive) command
 *  processRT_BC            - Formats a RT to BC (transmit) command
 *  processRT_RT            - Formats a RT to RT command
 *  buildXmitMessage        - Builds the playback command message
 *  processCommand          - Formats the 1553 command record
 *  processResponseGap      - Calculate the response gap
 *  processData             - Format the message data
 *  processStatus           - Format the status record
 *  processTagTime          - Format tag time.
 *  fillLookAheadBuffer     - Fills the lookahead buffer with formatted records
 *  writeReformattedDataToBuffer - Write the formatted playback record to the
 *                                 playback buffer
 *  advancePlaybackHeadPTR - Keeps track of playback buffer head pointer
 *  playbackBufferFreeSpace      - Calculates the free space in the playback buffer
 *  lookAheadBufferDataAvailable - Calcultes the data available in the lookahead buffer
 *  filterRecord    - Filters the .bmd records based on message number
 *  periodicUpdate  - Update routine called periodically to process records.
 *  runPlayback     - Starts playback after initialization and setup complete
 *===========================================================================*/

/* $Revision:  8.22 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/05/1999   Initial version. V3.23 rhc
  01/18/2000   Added test for valid boards which support playback.V4.00.ajh
  03/07/2001   Added playback to Linux API V4.30-RHC rhc
  2/15/2002    Added support for modular API. v4.48
  07/11/2005   Added Fix for Playback stop.
  08/12/2005   Added New Function BusTools_Playback_Check to test for time gaps.
  02/27/2012   Add updates to support V6 F/W
  11/05/2014   Change signalUserThread to use 32-bit read if RT-RT
  11/05/2014   Add Flips for the RT API mess_id and status word
  14/10/2015   initialize all bits of bt and tt of bigtime to 0 and handle the end of file for BusTools_Playback_Check()-skb
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
*/

/*---------------------------------------------------------------------------*
 *                    INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

#ifdef _PLAYBACK_
#include "playback.h"

/*---------------------------------------------------------------------------*
 *                 Local Function prototypes
 *---------------------------------------------------------------------------*/

static int        processTheRecord(BT_UINT);
static BT_INT     initPlayback(BT_UINT, API_PLAYBACK);
static void       getPlaybackData(API_PLAYBACK, BT_UINT);
static int        processBC_RT(BT_UINT cardnum);
static int        processRT_BC(BT_UINT cardnum);
static int        processRT_RT(BT_UINT cardnum);
static BT_U16BIT* processCommand(BT_U16BIT*, int,BT_UINT );
static BT_U16BIT* processResponseGap(BT_U16BIT,BT_U32BIT ,BT_U16BIT*,BT_UINT);
static BT_U16BIT* processData(BT_U16BIT*,BT_UINT);
static BT_U16BIT* processStatus(BT_U16BIT*, int,BT_UINT );
static BT_U16BIT* processTagTime(BT_U16BIT*,BT_UINT);
static XMIT_MESSAGE buildXmitMessage(BT_U16BIT, int, int,BT_UINT);
#ifdef _UNIX_
static void * runPlayback(void *);
#else
static BT_U32BIT  _stdcall runPlayback(LPVOID);
#endif
static BT_INT     _stdcall periodicUpdate(BT_UINT, API_INT_FIFO*);
static void       writeReformattedDataToBuffer(BT_UINT);
static int        filterRecord(BT_UINT);
static void       fillLookAheadBuffer(BT_UINT);
static BT_U32BIT  playbackBufferFreeSpace(BT_UINT);
static BT_U32BIT  lookAheadBufferDataAvailable(BT_INT,BT_UINT);
#ifdef _UNIX_
static void * readLogFileBlock(void *);
#else
static BT_U32BIT  _stdcall readLogFileBlock(LPVOID);
#endif
static void       setupAPI_INT_FIFO(BT_UINT);
static BT_INT     setupTheDiskBuffer(BT_UINT);
static BT_INT     setupTheLookAheadBuffer(BT_UINT);
static BT_INT     setupThePlaybackBuffer(BT_UINT);
static BT_INT     setUpThreadsAndEvents(BT_UINT);
static void       advancePlaybackHeadPTR(BT_U32BIT,BT_UINT);

/*---------------------------------------------------------------------------*
 *                 Local Function prototypes V6
 *---------------------------------------------------------------------------*/

static int        processTheRecordv6(BT_UINT);
static BT_INT     initPlaybackv6(BT_UINT, API_PLAYBACK);
static void       getPlaybackDatav6(API_PLAYBACK, BT_UINT);
static int        processBC_RTv6(BT_UINT cardnum);
static int        processRT_BCv6(BT_UINT cardnum);
static int        processRT_RTv6(BT_UINT cardnum);
static BT_U16BIT* processCommandv6(BT_U16BIT*, int,BT_UINT );
static BT_U16BIT* processResponseGapv6(BT_U16BIT,BT_U32BIT ,BT_U16BIT*,BT_UINT);
static BT_U16BIT* processDatav6(BT_U16BIT*,BT_UINT);
static BT_U16BIT* processStatusv6(BT_U16BIT*, int,BT_UINT );
static BT_U16BIT* processTagTimev6(BT_U16BIT*,BT_UINT);
static XMIT_MESSAGE buildXmitMessagev6(BT_U16BIT, int, int,BT_UINT);
#ifdef _UNIX_
static void * runPlaybackv6(void *);
#else
static BT_U32BIT  _stdcall runPlaybackv6(LPVOID);
#endif
static BT_INT     _stdcall periodicUpdatev6(BT_UINT, API_INT_FIFO*);
static void       writeReformattedDataToBufferv6(BT_UINT);
static int        filterRecordv6(BT_UINT);
static void       fillLookAheadBufferv6(BT_UINT);
static BT_U32BIT  playbackBufferFreeSpacev6(BT_UINT);
static BT_U32BIT  lookAheadBufferDataAvailablev6(BT_INT,BT_UINT);
#ifdef _UNIX_
static void * readLogFileBlockv6(void *);
#else
static BT_U32BIT  _stdcall readLogFileBlockv6(LPVOID);
#endif
static void       setupAPI_INT_FIFOv6(BT_UINT);
static BT_INT     setupTheDiskBufferv6(BT_UINT);
static BT_INT     setupTheLookAheadBufferv6(BT_UINT);
static BT_INT     setupThePlaybackBufferv6(BT_UINT);
static BT_INT     setUpThreadsAndEventsv6(BT_UINT);
static void       advancePlaybackHeadPTRv6(BT_U32BIT,BT_UINT);

#define _TIME_CYCLE

static UINT timerId = 0;          // Timer ID used to close the timer callback.
#endif //_PLAYBACK_

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback
*
*  FUNCTION
*     This routine handles starting bus playback.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_FILE   -> Error in openning .bmd file
*     API_PLAYBACK_INIT_ERROR -> Playback initialization Error
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_PlaybackV6(
   BT_UINT cardnum,           // (i) card number (0 - based)
   API_PLAYBACK playbackData)  // (io) Control and Status structures
{
#if defined(_PLAYBACK_)

#ifdef _UNIX_
   pthread_t pbThread;         // Handle to the main playback thread.
   int status=0;
#else
   HANDLE    pbThread;         // Handle to the main playback thread.
   BT_U32BIT ThreadID;         // Thread ID of the main playback thread.
#endif
   int       i;                // Loop counter
   int scnt=0;

   union bigtime
   {
      CEI_UINT64 time64;
      BT1553_TIME ttime;
   }bt;

   // initialize all bits to zero
   bt.time64 = 0;

   // Verify that the selected board supports playback
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   // Save the Playback Status Structure pointer in our local table.

   pbv6[cardnum].playbackStatus = playbackData.status;
   if(pbv6[cardnum].filterFlag == messageNumberFilter)
   {
      fseek(pbv6[cardnum].filePTR,((pbv6[cardnum].messageNumberStartPoint-1)*sizeof(API_BM_MBUF)) + sizeof(BMDX_HEADER),SEEK_SET);
      fread(&pbv6[cardnum].BMRecord,sizeof(API_BM_MBUF),1,pbv6[cardnum].filePTR);
      fseek(pbv6[cardnum].filePTR,((pbv6[cardnum].messageNumberStartPoint-1)*sizeof(API_BM_MBUF)) + sizeof(BMDX_HEADER),SEEK_SET);
      if(nsTimeTag)
      {
         bt.ttime = pbv6[cardnum].BMRecord.time;
         bt.time64/=1000;
         pbv6[cardnum].startTimeDelay = bt.time64;
      }
      else {    
         bt.ttime = pbv6[cardnum].BMRecord.time;
         pbv6[cardnum].startTimeDelay = bt.time64;
      } 
   }
   else
   {
      fseek(pbv6[cardnum].filePTR,sizeof(BMDX_HEADER),SEEK_SET);  //set the start at the first bmdx record
      fread(&pbv6[cardnum].BMRecord,sizeof(API_BM_MBUF),1,pbv6[cardnum].filePTR);
      fseek(pbv6[cardnum].filePTR,sizeof(BMDX_HEADER),SEEK_SET);  //set the start at the first bmdx record
      if(nsTimeTag)
      {
         bt.ttime = pbv6[cardnum].BMRecord.time;
         bt.time64/=1000;
         pbv6[cardnum].startTimeDelay = bt.time64;
      }
      else { 
         bt.ttime = pbv6[cardnum].BMRecord.time;
         pbv6[cardnum].startTimeDelay = bt.time64;
      } 

   }
   i = 0;  // Fill all 4 of the buffers
   do
   {
#ifdef _UNIX_

      pthread_mutex_lock(&pbv6[cardnum].blockReadMutex); 
      status = pthread_cond_signal(&pbv6[cardnum].blockReadEvent);  //fill the 2048 bm record disk buffer
      pthread_mutex_unlock(&pbv6[cardnum].blockReadMutex); 
#else
      SetEvent(pbv6[cardnum].blockReadEvent);  // this fill the 2048 bm record disk buffer
      WaitForSingleObject(pbv6[cardnum].readCompleteEvent, INFINITE);
#endif
   }
   while (pbv6[cardnum].diskFileNotEmpty && (++i < numberOfBlocks) );

   fillLookAheadBufferv6(cardnum);  // get initial data from disk and store in lookAheadBuffer
   writeReformattedDataToBufferv6(cardnum);   // move data in lookAheadBuffer to the play to playBackBuffer
   playback_is_running[cardnum] = 1;
   if(pbv6[cardnum].moreDataInFile)
   {
      fillLookAheadBufferv6(cardnum);  // fill the lookahead buffer again so its filled for start of playback
   }
   // Create the main playback thread here then return status to calling routine.

#ifdef _UNIX_

   status = pthread_create(&pbThread,NULL,runPlaybackv6,(void *)cardnum);
   if(status)
   {
      pbv6[cardnum].playbackStatus->playbackError = API_SUCCESS;
      return API_PLAYBACK_BAD_THREAD;   // Return error
   }
    sleep(9000);
#else
   pbThread = CreateThread(NULL, stackSize, runPlaybackv6,
                           (LPVOID)((CEI_UINT64)cardnum), CREATE_SUSPENDED, &ThreadID); /* extraneous CEI_UINT64 cast for Wp64 warning */
   if ( pbThread == NULL ) // Make sure the thread was created properly
   {
      return API_PLAYBACK_BAD_THREAD;   // Return error
   }                                             // Set the thread priority to normal
   SetThreadPriority(pbThread, THREAD_PRIORITY_ABOVE_NORMAL);
   // Everything's set, let it run.  It will execute playback.
   ResumeThread(pbThread);
   // Close the thread handle since we don't need it anymore.
   CloseHandle(pbThread);
#endif
   pbv6[cardnum].playbackStatus->playbackError = API_SUCCESS;
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

#ifdef _PLAYBACK_
/****************************************************************************
*
*  PROCEDURE NAME - runPlayback
*
*  FUNCTION
*     This routine runs bus Playback as a separate thread.  When it is time
*     to terminate, this routine cleans up all of the handles and releases
*     all of the allocated memory.  It also unregisters the polling function.
*
*   Parameters
*     LPVOID Param            -> cardnum
*
*   Returns
*     API_SUCCESS             -> success
****************************************************************************/
#ifdef _UNIX_
void * runPlaybackv6(void * Param)
#else
static BT_U32BIT _stdcall runPlaybackv6(LPVOID Param)
#endif
{
   BT_UINT cardnum;
#ifdef _UNIX_
   BT_INT status;
#endif

   cardnum = (BT_UINT)((CEI_UINT64)Param); /* extraneous CEI_UINT64 cast for Wp64 warning */

   // register the periodicUpdate function to run periodically
   BusTools_RegisterFunction(cardnum,&pbv6[cardnum].intFifo, REGISTER_FUNCTION);

   //set the playback runbit to start playback running

   if(board_is_v5_uca[cardnum])
      vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x0001);
   else
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x1 + PB_SET);
   // wait for end of playback
#ifdef _UNIX_
//   pthread_mutex_lock(&pbv6[cardnum].hExitMutex);

   status = pthread_cond_wait(&pbv6[cardnum].hExitEvent,&pbv6[cardnum].hExitMutex);
//   pthread_mutex_unlock(&pbv6[cardnum].hExitMutex);
#else
   WaitForSingleObject(pbv6[cardnum].hExitEvent, INFINITE);
#endif
   //stop the periodic update function from being called
   if(board_is_v5_uca[cardnum])
      vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x0); // make sure the run bit is cleared
   else
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x0000FFFF + PB_CLEAR); // make sure the run bit is cleared

   BusTools_RegisterFunction(cardnum,&pbv6[cardnum].intFifo, UNREGISTER_FUNCTION);
   // Wait for the other Playback threads to exit.

   MSDELAY(40);   // Give the other threads a chance to shutdown

   // Clean up the handles and free all of the memory blocks we allocated
#ifdef _UNIX_
   pthread_cond_destroy(&pbv6[cardnum].hExitEvent);
   pthread_cond_destroy(&pbv6[cardnum].blockReadEvent);
   pthread_cond_destroy(&pbv6[cardnum].readCompleteEvent);
#else
   CloseHandle(pbv6[cardnum].hExitEvent);
   CloseHandle(pbv6[cardnum].blockReadEvent);
   CloseHandle(pbv6[cardnum].readCompleteEvent);
#endif
   free(pbv6[cardnum].messagePacket);
   free(pbv6[cardnum].diskBuffer);
   free(pbv6[cardnum].lookAheadBuffer);
   fclose(pbv6[cardnum].filePTR);
#ifdef _UNIX_
   pthread_exit(0);
#else
   ExitThread(0);
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - initPlayback
*
*  FUNCTION
*     This routine initializes playback
*
*   Parameters
*     API_PLAYBACK playbackData -> Structure of playback data used for initialization
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
*
****************************************************************************/
static BT_INT initPlaybackv6(BT_UINT cardnum, API_PLAYBACK playbackData)
{
   BT_INT    status;
   BT_U32BIT controlData;

   // Magic numbers
   pbv6[cardnum].blockCount    = 0x0400;
   pbv6[cardnum].eopMessage    = 0x01e00;
   // initialize data
   pbv6[cardnum].endRecord.messno = 0;
   pbv6[cardnum].bufferCount      = 0;
   pbv6[cardnum].moreDataInFile   = TRUE;
   pbv6[cardnum].timeOffset       = 0;
   pbv6[cardnum].lastTagTime      = 0;
   pbv6[cardnum].lastRespTime     = 0;
   pbv6[cardnum].endOfFilterData  = FALSE;
   pbv6[cardnum].diskFileNotEmpty = TRUE;
   pbv6[cardnum].startTimeDelay   = 0;

   // Stop playback just in case
   if(board_is_v5_uca[cardnum])
   {
      // Stop playback just in case
      controlData = (BT_U32BIT)vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);
      if((controlData & runBit)!=0)
      {
         vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x20);
         vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
         return API_PLAYBACK_RUNNING;
      }
      // clear error just in case
      vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
   }
   else
   {
      controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);
      if((controlData & runBit)!=0)
      {
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x20 + PB_SET);
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
         return API_PLAYBACK_RUNNING;
      }
      // clear error just in case
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
   }

   // allocate space for message packet this should be enough for max case
   pbv6[cardnum].messagePacket = (BT_U16BIT *)malloc(256);
   if(pbv6[cardnum].messagePacket==NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   getPlaybackDatav6(playbackData,cardnum);
   if((status = setupTheDiskBufferv6(cardnum))!=API_SUCCESS)
   {
      return status;
   }

   if((status = setupTheLookAheadBufferv6(cardnum))!=API_SUCCESS)
   {
      return status;
   }
   status = BusTools_BC_Init(cardnum,0,0,0,16,14,1000,1);
   if(status == API_SUCCESS)
   {
      status = setupThePlaybackBufferv6(cardnum);
   }

   if(status == API_SUCCESS)
   {
      setupAPI_INT_FIFOv6(cardnum); //set up the time interrut
      status = setUpThreadsAndEventsv6(cardnum);
   }
   
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupTheLookAheadBuffer
*
*  FUNCTION
*     This routine sets up the look ahead buffer used to store reformatted
*     Bus Monitor records.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
****************************************************************************/
static BT_INT setupTheLookAheadBufferv6(BT_UINT cardnum)
{
   pbv6[cardnum].lookAheadBuffer = (BT_U16BIT*)malloc((size_t)sizeOfLookAheadBufferv6);
   if(pbv6[cardnum].lookAheadBuffer == NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   pbv6[cardnum].lookAheadHeadPTR = pbv6[cardnum].lookAheadBuffer;
   pbv6[cardnum].endOfBuffer = (pbv6[cardnum].lookAheadBuffer+sizeOfLookAheadBufferv6/bytesPerWord);
   pbv6[cardnum].lookAheadTailPTR = pbv6[cardnum].lookAheadBuffer;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupTheDiskBuffer
*
*  FUNCTION
*     This routine sets up the disk buffer used to store Bus Monitor records.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
*
****************************************************************************/
static BT_INT setupTheDiskBufferv6(BT_UINT cardnum)
{
   pbv6[cardnum].diskBuffer = (API_BM_MBUF*)malloc((size_t)sizeOfDiskBufferv6);

   if(pbv6[cardnum].diskBuffer == NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   pbv6[cardnum].diskBufferPTR = pbv6[cardnum].diskBuffer;
   pbv6[cardnum].diskReadPTR = pbv6[cardnum].diskBuffer;
   pbv6[cardnum].endOfDiskBuffer = (pbv6[cardnum].diskBuffer+readSize*numberOfBlocks);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupThePlaybackBuffer
*
*  FUNCTION
*     This routine sets up the playback buffer on the PCI 1553. This is a circular
*     buffer that the firmware uses to run playback.  This routine allocates memory,
*     set the head and tail pointers, and determines the end of the buffer.
*
*   Returns
*     API_SUCCESS -> success
****************************************************************************/
static BT_INT setupThePlaybackBufferv6(BT_UINT cardnum)
{
   BT_INT    status;                 // Function return status
   BT_U32BIT address_of_end_of_blk;  // Actual end of buffer
   BT_U16BIT playbackBuffer16;       // Register version
   BT_U16BIT endOfPlaybackBuffer16;  // Register version

   // Allocate a large block of BusTools memory.
   status = BusTools_BC_MessageAlloc(cardnum, numberOfMessages);
   if ( status != API_SUCCESS )
   {
      return status;
   }
   // Get the starting and ending addresses of the allocated memory block.
   // The playback buffer must start on an address which is a multiple of
   //  8 words; PCI-1553 BC messages start on such an address.
   status = BusTools_GetAddr(cardnum, GETADDR_BCMESS,
                             &pbv6[cardnum].playbackBuffer32,
                             &address_of_end_of_blk);

   if ( status != API_SUCCESS )
   {
      return status;
   }
   // Initialize PB Start, Head and Tail registers to the buffer head pointer.

   pbv6[cardnum].playbackHeadPTR32 = pbv6[cardnum].playbackBuffer32;

   if(board_is_v5_uca[cardnum])
   {
       playbackBuffer16  = (BT_U16BIT)(pbv6[cardnum].playbackBuffer32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_START_POINTER, playbackBuffer16);
      vbtSetFileRegister(cardnum, PB_HEAD_POINTER,  playbackBuffer16);
      vbtSetFileRegister(cardnum, PB_TAIL_POINTER,  playbackBuffer16);
   }
   else
   {
      vbtSetRegister[cardnum](cardnum, PB_START_V6POINTER, RAM_ADDR(cardnum,pbv6[cardnum].playbackBuffer32));
      vbtSetRegister[cardnum](cardnum, PB_HEAD_V6POINTER,  RAM_ADDR(cardnum,pbv6[cardnum].playbackBuffer32));
      vbtSetRegister[cardnum](cardnum, PB_TAIL_V6POINTER,  RAM_ADDR(cardnum,pbv6[cardnum].playbackBuffer32));
   }
   // Compute End of Buffer.  Buffer size must be a multiple of 8 words/16 bytes.
   pbv6[cardnum].sizeOfPlaybackBuffer  = address_of_end_of_blk - pbv6[cardnum].playbackBuffer32;
   if(board_is_v5_uca[cardnum])
      pbv6[cardnum].sizeOfPlaybackBuffer &= 0xFFFFFFFF << shiftBits;
   else
      pbv6[cardnum].sizeOfPlaybackBuffer+=1;

   pbv6[cardnum].endOfPlaybackBuffer32 = pbv6[cardnum].playbackBuffer32+pbv6[cardnum].sizeOfPlaybackBuffer;

   // write end address of playback buffer to register (multiple of 16 bytes)

   if(board_is_v5_uca[cardnum])
   {
      endOfPlaybackBuffer16 = (BT_U16BIT)(pbv6[cardnum].endOfPlaybackBuffer32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_END_POINTER, endOfPlaybackBuffer16);
      vbtSetFileRegister(cardnum, PB_INT_THRSHLD, (BT_U16BIT)pbv6[cardnum].blockCount);
      vbtSetFileRegister(cardnum, PB_THRSHLD_CNT, (BT_U16BIT)pbv6[cardnum].blockCount);
   }
   else
   {
      vbtSetRegister[cardnum](cardnum, PB_END_V6POINTER, RAM_ADDR(cardnum,pbv6[cardnum].endOfPlaybackBuffer32)-2);
      vbtSetRegister[cardnum](cardnum, PB_INT_V6THRSHLD, (BT_U32BIT)pbv6[cardnum].blockCount);
      vbtSetRegister[cardnum](cardnum, PB_THRSHLD_V6CNT, (BT_U32BIT)pbv6[cardnum].blockCount);
   }
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - setUpThreadsAndEvents
*
*  FUNCTION
*     This routine hsets up the thread and events used to run playback..
*
*   Parameters
*     none
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_EVENT  -> Error in creating an event
*     API_PLAYBACK_BAD_THREAD -> Error in creating a thread
*
****************************************************************************/

static BT_INT setUpThreadsAndEventsv6(BT_UINT cardnum)
{

#ifdef _UNIX_
   int status;
   pthread_t readThread;
#else
   HANDLE    readThread;     // Handle to the (readLogFileBlock) thread.
   BT_U32BIT readThreadID;   // Thread ID of the (readLogFileBlock) thread.
#endif
   // Create the events that the read file (readLogFileBlock) thread uses:
   // Create the exit event for playback completion
#ifdef _UNIX_
   status = pthread_cond_init(&pbv6[cardnum].hExitEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_mutex_init(&pbv6[cardnum].hExitMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pbv6[cardnum].hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if(pbv6[cardnum].hExitEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Create the read event for playback disk reads
#ifdef _UNIX_
   status = pthread_mutex_init(&pbv6[cardnum].blockReadMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_cond_init(&pbv6[cardnum].blockReadEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pbv6[cardnum].blockReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   if(pbv6[cardnum].blockReadEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Create the read completion event for playback disk reads
#ifdef _UNIX_
   status = pthread_cond_init(&pbv6[cardnum].readCompleteEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_mutex_init(&pbv6[cardnum].readCompleteMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pbv6[cardnum].readCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   if(pbv6[cardnum].readCompleteEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Don't pass in a dynamic structure which gets deleted; that is not reliable.
#ifdef _UNIX_
   status = pthread_create(&readThread,NULL,readLogFileBlockv6,(void *)cardnum);
   if(status)
      return API_PLAYBACK_BAD_THREAD;
#else
   readThread = CreateThread(NULL, stackSize, readLogFileBlockv6,
                             (LPVOID)((CEI_UINT64)cardnum), CREATE_SUSPENDED, &readThreadID); /* extraneous CEI_UINT64 cast for Wp64 warning */
   if ( readThread == NULL )      // Make sure the thread was created properly
   {
      return API_PLAYBACK_BAD_THREAD;
   }
   // Set the thread priority to normal
   SetThreadPriority(readThread, THREAD_PRIORITY_NORMAL);

   ResumeThread(readThread);
   // Close the thread handle since we don't need it anymore.
   CloseHandle(readThread);
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupAPI_INT_FIFO
*
*  FUNCTION
*     This routine sets data in the API_INT_FIFO that controls the timer
*     interrupt used to invoke the playback periodicUpdate function.
*     The timer is set for 50 MS.
*
*   Parameters
*     intFifo (Global) is initialized.  All required parameters are set.
*
*   Returns
*     none
*
****************************************************************************/

static void setupAPI_INT_FIFOv6(BT_UINT cardnum)
{
   pbv6[cardnum].intFifo.function       = periodicUpdatev6;         // Function to call
   pbv6[cardnum].intFifo.iPriority      = THREAD_PRIORITY_ABOVE_NORMAL; // Thread priority
   pbv6[cardnum].intFifo.dwMilliseconds = 50;  // 50 millisecond timed interrupt
   pbv6[cardnum].intFifo.iNotification  = 0;   // Don't call at startup or shutdown
   pbv6[cardnum].intFifo.bForceShutdown = 0;   // Don't call at startup or shutdown
   pbv6[cardnum].intFifo.FilterType     = 0;   // Don't call for any 1553 bus events
   pbv6[cardnum].intFifo.cardnum        = cardnum;
}

/****************************************************************************
*
*  PROCEDURE NAME - getPlaybackData
*
*  FUNCTION
*     This routine gets the data in the API_PLAYBACK structure and stored the
*     data.  It also set up any filter parameters.
*
*   Parameters
*     API_PLAYBACK playbackData -> Structure containing the playback data passed
*                                  calling routine.
*
*   Returns
*     none
*
****************************************************************************/

static void getPlaybackDatav6(API_PLAYBACK playbackData, BT_UINT cardnum)
{
   BT_U32BIT maskBit;
   BT_INT index;

   pbv6[cardnum].playbackStatus = playbackData.status;
   pbv6[cardnum].playbackStatus->recordsProcessed = 0;
   pbv6[cardnum].filterFlag = playbackData.filterFlag;
   switch(pbv6[cardnum].filterFlag)   // 0 no filter, 1 = time filter, 2 = message number filter
   {
      case 0:
         pbv6[cardnum].noFilter = TRUE;
         break;
      case 1:
         pbv6[cardnum].startTagTime = playbackData.timeStart;
         pbv6[cardnum].stopTagTime  = playbackData.timeStop;

         //pbv6[cardnum].startTagTime.microseconds = playbackData.timeStart.microseconds;
         //pbv6[cardnum].startTagTime.topuseconds = playbackData.timeStart.topuseconds;
         //pbv6[cardnum].stopTagTime.microseconds = playbackData.timeStop.microseconds;
         //pbv6[cardnum].stopTagTime.topuseconds  = playbackData.timeStop.topuseconds;
         pbv6[cardnum].noFilter = FALSE;
         break;
      case 2:
         pbv6[cardnum].messageNumberStartPoint = playbackData.messageStart;
         pbv6[cardnum].messageNumberStopPoint  = playbackData.messageStop;
         pbv6[cardnum].noFilter = FALSE;
         break;
      default:
         pbv6[cardnum].noFilter = TRUE;
         break;
   }

   maskBit = 1;
   for(index = 0; index < numberOfRT; index++)
   {
      pbv6[cardnum].RTarray[index] = playbackData.activeRT & maskBit;
      maskBit = maskBit<<1;
   }
   
   if(playbackData.Subadress31Flag == 1)   // 0 = SA 31 is not a mode code, 1 = SA31 is mode code
   {
      pbv6[cardnum].modeCode2 = 31;
   }
   else
   {
      pbv6[cardnum].modeCode2 = 0;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - processTheRecord
*
*  FUNCTION
*     This routine determines the 1553 message type and invokes the proper
*     method to process the message into the format required by playback.
*
*   Parameters
*     none
*
*   Returns
*     int bufferSize -> Size of the reformatted playback record stored in the
*     messagePacket buffer.
*
****************************************************************************/
static int processTheRecordv6(BT_UINT cardnum)
{
   int bufferSize;
   int BrdcMC = FALSE;

   // test for broadcast mode code messages: RT31, s/a == 0
   // or (if enabled) s/a == 0x1f AND if the word count 
   // is less than 0x10 (mode codes with no data)
   if(pbv6[cardnum].BMRecord.int_status & BT1553_INT_MODE_CODE)
   {
      if(pbv6[cardnum].BMRecord.int_status & BT1553_INT_BROADCAST)
         BrdcMC = TRUE;
      else 
         BrdcMC = FALSE;

   }
   
   //Test for RT_RT message

   if((pbv6[cardnum].BMRecord.int_status & BT1553_INT_RT_RT_FORMAT) != 0)
   {
       bufferSize = processRT_RTv6(cardnum);
   }
   else
   {
      if(pbv6[cardnum].BMRecord.command1.tran_rec)
      {
		  if (BrdcMC)
			  bufferSize = processBC_RTv6(cardnum);
		  else
	         bufferSize = processRT_BCv6(cardnum);
      }
      else
      {
         bufferSize = processBC_RTv6(cardnum);
      }

   }
   return bufferSize;
}

/****************************************************************************
*
*  PROCEDURE NAME - writeReformattedDataToBufferv6
*
*  FUNCTION
*     This routine writes the formatted playback data stored in the look ahead
*     buffer into the playback buffer located on the card.  This function
*     determines the available space in the playback buffer and the amount of
*     data in the look ahead buffer and writes the smaller of the two number bytes
*     to the playback buffer.   After the write, the look ahead buffer tail
*     pointer and playback buffer head pointer are advanced.
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
static void writeReformattedDataToBufferv6(BT_UINT cardnum)
{
   BT_U32BIT space, data;

   space = playbackBufferFreeSpacev6(cardnum);         // available space in playback buffer
   data  = lookAheadBufferDataAvailablev6(0,cardnum);  // available data in look ahead buffer

#ifdef showspace
#endif

   if(data != 0 && space != 0)
   {
      if (space > data)   // if there is more space in the play back buffer than available data, then write only data
      {
         //block write of data 64Kbytes
         if(board_is_v5_uca[cardnum])
            vbtWrite(cardnum, (LPSTR)pbv6[cardnum].lookAheadTailPTR, pbv6[cardnum].playbackHeadPTR32, data);
         else 
            vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)pbv6[cardnum].lookAheadTailPTR, pbv6[cardnum].playbackHeadPTR32, data/2);
         pbv6[cardnum].lookAheadTailPTR+=(data/bytesPerWord);   //advance tailPTR in look ahead
         if(pbv6[cardnum].lookAheadTailPTR >= pbv6[cardnum].endOfBuffer)
         {
            pbv6[cardnum].lookAheadTailPTR = (pbv6[cardnum].lookAheadTailPTR-pbv6[cardnum].endOfBuffer)+pbv6[cardnum].lookAheadBuffer;
         }
         advancePlaybackHeadPTRv6(data,cardnum);
      }
      else
      {
         //block write of space bytes
         if(board_is_v5_uca[cardnum])
            vbtWrite(cardnum, (LPSTR)pbv6[cardnum].lookAheadTailPTR, pbv6[cardnum].playbackHeadPTR32, space);
         else
            vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)pbv6[cardnum].lookAheadTailPTR, pbv6[cardnum].playbackHeadPTR32, space/2);
         pbv6[cardnum].lookAheadTailPTR+=(space/bytesPerWord);  //advance tailPTR in look ahead
         if(pbv6[cardnum].lookAheadTailPTR >= pbv6[cardnum].endOfBuffer)
         {
            pbv6[cardnum].lookAheadTailPTR = (pbv6[cardnum].lookAheadTailPTR-pbv6[cardnum].endOfBuffer)+pbv6[cardnum].lookAheadBuffer;
         }
         advancePlaybackHeadPTRv6(space,cardnum);
      }
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - advancePlaybackHeadPTR
*
*  FUNCTION
*     This functioon advance the playback buffer head pointer.
*
*   Parameters
*     BT_U32BIT count -> number of bytes to advance head ponter.
*
*   Returns
*     none
*
****************************************************************************/
static void advancePlaybackHeadPTRv6(BT_U32BIT count,BT_UINT cardnum)
{
   BT_U16BIT playbackHeadPTR16;


   pbv6[cardnum].playbackHeadPTR32+=count;

   if(pbv6[cardnum].playbackHeadPTR32 >= pbv6[cardnum].endOfPlaybackBuffer32)
   {
      pbv6[cardnum].playbackHeadPTR32 = pbv6[cardnum].playbackBuffer32;
   }

   if(board_is_v5_uca[cardnum])
   {
      playbackHeadPTR16 = (BT_U16BIT)(pbv6[cardnum].playbackHeadPTR32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_HEAD_POINTER, playbackHeadPTR16);
   }
   else
      vbtSetRegister[cardnum](cardnum, PB_HEAD_V6POINTER, RAM_ADDR(cardnum,pbv6[cardnum].playbackHeadPTR32));
}

/****************************************************************************
*
*  PROCEDURE NAME - processBC_RT
*
*  FUNCTION
*     This function convert BC_RT message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacket buffer
*
****************************************************************************/
static int processBC_RTv6(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;

   messagePTR = pbv6[cardnum].messagePacket;

   messagePTR = processTagTimev6(messagePTR,cardnum);
   messagePTR = processCommandv6(messagePTR,1,cardnum);
   messagePTR = processDatav6(messagePTR,cardnum);
   if((pbv6[cardnum].RTarray[pbv6[cardnum].BMRecord.command1.rtaddr]) != 0)
   {
      if((pbv6[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP) == 0)
      {
         messagePTR = processResponseGapv6(pbv6[cardnum].BMRecord.response1.time, (pbv6[cardnum].numberOfDataWords+1)*timePerWord,messagePTR,cardnum);
         if(pbv6[cardnum].BMRecord.command1.rtaddr != 31)
         {
            messagePTR = processStatusv6(messagePTR,1,cardnum);
         }
      }

   }

   return (BT_INT)(messagePTR - pbv6[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processRT_BC
*
*  FUNCTION
*     This function convert RT_BC message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacketg buffer
*
****************************************************************************/
static int processRT_BCv6(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;

   messagePTR = pbv6[cardnum].messagePacket;

   messagePTR = processTagTimev6(messagePTR,cardnum);
   messagePTR = processCommandv6(messagePTR,1,cardnum);
   if((pbv6[cardnum].RTarray[pbv6[cardnum].BMRecord.command1.rtaddr]) != 0)
   {
      if((pbv6[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP) == 0)
      {
         messagePTR = processResponseGapv6(pbv6[cardnum].BMRecord.response1.time,timePerWord,messagePTR,cardnum);
         messagePTR = processStatusv6(messagePTR,1,cardnum);
         messagePTR = processDatav6(messagePTR,cardnum);
      }

   }

   return (BT_INT)(messagePTR - pbv6[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processRT_RT
*
*  FUNCTION
*     This function convert RT_RT message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacketg buffer
*
****************************************************************************/
static int processRT_RTv6(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;
   BT_U32BIT additionalTime;
   BT_UINT rtime;

   additionalTime = 40;
   messagePTR = pbv6[cardnum].messagePacket;

   messagePTR = processTagTimev6(messagePTR,cardnum);
   messagePTR = processCommandv6(messagePTR,1,cardnum); // #1
   messagePTR = processCommandv6(messagePTR,2,cardnum); // #2

   if(pbv6[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP)//if no response then we are done
      return (BT_INT)(messagePTR - pbv6[cardnum].messagePacket);  //

   if((pbv6[cardnum].RTarray[pbv6[cardnum].BMRecord.command2.rtaddr]) != 0)
   {
      if(pbv6[cardnum].BMRecord.response1.time != 0)
      {
         messagePTR = processResponseGapv6(pbv6[cardnum].BMRecord.response1.time, additionalTime ,messagePTR,cardnum); // #1
         messagePTR = processStatusv6(messagePTR,1,cardnum); //#1
         messagePTR = processDatav6(messagePTR,cardnum);
      }
   }
   else
   {
      pbv6[cardnum].numberOfDataWords = pbv6[cardnum].BMRecord.command1.wcount;
      if(pbv6[cardnum].numberOfDataWords == 0)
      {
         pbv6[cardnum].numberOfDataWords = 32;
      }
   }
   // note that if rt_bcst_enabled[cardnum] == 1;    // RT address 31 is broadcast.
   // otherwise RT address 31 is just another address.

   if(pbv6[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP)//if no response then we are done
      return (BT_INT)(messagePTR - pbv6[cardnum].messagePacket);

   if((pbv6[cardnum].RTarray[pbv6[cardnum].BMRecord.command1.rtaddr] != 0));// && (pbv6[cardnum].BMRecord.command1.rtaddr != broadcast))
   {
      if(pbv6[cardnum].BMRecord.response2.time != 0)
      {
         rtime = ((BT_UINT)pbv6[cardnum].BMRecord.response1.time)>>1;
         additionalTime = ((commandWords+pbv6[cardnum].numberOfDataWords)*timePerWord)+rtime;
         messagePTR = processResponseGapv6(pbv6[cardnum].BMRecord.response2.time, additionalTime,messagePTR,cardnum); // #2
         messagePTR = processStatusv6(messagePTR,2,cardnum); //#2
      }
   }
   return (BT_INT)(messagePTR - pbv6[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processCommand
*
*  FUNCTION
*     This function formats 1553 command word.
*
*   Parameters
*     BT_U16BIT* messagePtr -> pointer to the next messagePackage buffer location
*     int comIndex          -> 1 = command 1, 2 = command 2;
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processCommandv6(BT_U16BIT* messagePtr, int comIndex, BT_UINT cardnum)
{   
   union sData
   {
      XMIT_MESSAGE message;
      BT_U16BIT msgData;
   } xmit;

   union comData
   {
      BT1553_COMMAND command;
      BT_U16BIT cData;
   }com;

   if(comIndex == 1)
   {
      xmit.message = buildXmitMessagev6 (pbv6[cardnum].BMRecord.status_c1, commandCount, pSync,cardnum);
      // write command xmit_message to buffer.
      com.command = pbv6[cardnum].BMRecord.command1;
   }
   else
   {   
      xmit.message = buildXmitMessagev6 (pbv6[cardnum].BMRecord.status_c2, commandCount, pSync,cardnum);
      // write command xmit_message to buffer.
      com.command = pbv6[cardnum].BMRecord.command2;
   }

   *messagePtr = xmit.msgData;
   messagePtr++;
   *messagePtr = com.cData;
   messagePtr++;
   return messagePtr;
}

/****************************************************************************
*
*  PROCEDURE NAME - processResponseGap
*
*  FUNCTION
*     This function calculates the response gap stores the resutls in Playback
*     format in the messagePacket buffer.
*
*   Parameters
*     BT_U16BIT responseTime   -> Message response time
*     BT_U32BIT additionalTime -> additional time for command, status and data words
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processResponseGapv6(
   BT_U16BIT responseTime,
   BT_U32BIT additionalTime,
   BT_U16BIT* messagePTR,
   BT_UINT cardnum)
{
   BT_U32BIT timeData;
   BT_U16BIT lsbTime;
   BT_U16BIT msbTime;

   *messagePTR = gapMessage;
   messagePTR++;
   
   timeData = pbv6[cardnum].hwTagTime + responseTime + ((additionalTime-2) << 1); //Calculate the absolute time of response

   pbv6[cardnum].lastRespTime = timeData;

   lsbTime = (BT_U16BIT)(timeData & 0xffff);
   msbTime = (BT_U16BIT)((timeData & 0xffff0000) >> 16);

   *messagePTR = lsbTime;
   messagePTR++;
   *messagePTR = msbTime;
   messagePTR++;

   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processDataX
*
*  FUNCTION
*     This function formats the data associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processDatav6(BT_U16BIT* messagePTR,BT_UINT cardnum)
{
   union messageData
   {
      XMIT_MESSAGE msg;
      BT_U16BIT    mData;
   } msgDat;

   BT_U16BIT *tempPTR;
   BT_U16BIT  dataCount = 0;
   int        i;

   BT_U16BIT dataStatus;

   if(pbv6[cardnum].BMRecord.int_status & BT1553_INT_MODE_CODE)
   {
      pbv6[cardnum].numberOfDataWords = modeCodeData[pbv6[cardnum].BMRecord.command1.wcount];
   }
   else
   {
      pbv6[cardnum].numberOfDataWords = pbv6[cardnum].BMRecord.command1.wcount;
      if( pbv6[cardnum].numberOfDataWords == 0)
      {
         pbv6[cardnum].numberOfDataWords = 32;
      }
      if((pbv6[cardnum].BMRecord.int_status & BT1553_INT_LOW_WORD) != 0)
      {
         pbv6[cardnum].numberOfDataWords = 1;
      }
   }
   tempPTR = messagePTR;

   if(pbv6[cardnum].numberOfDataWords != 0)
   {
      dataStatus = (BT_U16BIT)(pbv6[cardnum].BMRecord.status[0] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY));
      msgDat.msg = buildXmitMessagev6(pbv6[cardnum].BMRecord.status[0], 0,nSync,cardnum);

      messagePTR++; //increment past command word until data count is known
      //add data;

      *messagePTR = pbv6[cardnum].BMRecord.value[0];
      messagePTR++;
      dataCount++;

      for(i=1;i<pbv6[cardnum].numberOfDataWords;i++)
      {
         if(dataStatus == (pbv6[cardnum].BMRecord.status[i] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY )))
         { 
            // add data
            *messagePTR = pbv6[cardnum].BMRecord.value[i];
            messagePTR++;
            dataCount++;
         }
         else
         {
            //write the xmit word to the buffer
            msgDat.msg.wcount = dataCount;
            *tempPTR = msgDat.mData;
            dataCount = 1;
            tempPTR = messagePTR;
            
            dataStatus = (BT_U16BIT)(pbv6[cardnum].BMRecord.status[i] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY));
            msgDat.msg = buildXmitMessagev6((BT_U16BIT)pbv6[cardnum].BMRecord.status[i],0,nSync,cardnum);

            messagePTR++;
            //add data
            *messagePTR = pbv6[cardnum].BMRecord.value[i];
            messagePTR++;
         }
      }
   }
   else
   {
      dataCount = 0;
   }
   msgDat.msg.wcount = dataCount;

   *tempPTR = msgDat.mData;
   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processStatusv6
*
*  FUNCTION
*     This function formats the status associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*     nt statIndex             -> 1 = status 1; 2 = status
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processStatusv6(BT_U16BIT* messagePTR, int statIndex, BT_UINT cardnum)
{
   union sData
   {
      XMIT_MESSAGE message;
      BT_U16BIT msg;
   } statMSG;

   union statusData
   {
      BT1553_STATUS stat;
      BT_U16BIT sdat;
   }statDat;

   if(statIndex == 1)
   {
      statDat.stat = pbv6[cardnum].BMRecord.status1;
      statMSG.message = buildXmitMessagev6(pbv6[cardnum].BMRecord.status_s1,statusCount,pSync,cardnum);
   }
   else
   {
      statDat.stat = pbv6[cardnum].BMRecord.status2;
      statMSG.message = buildXmitMessagev6(pbv6[cardnum].BMRecord.status_s2,statusCount,pSync,cardnum); // changed status_s1 to status_s2
   }

   *messagePTR = statMSG.msg;
   messagePTR++;
   *messagePTR = statDat.sdat;
   messagePTR++;
   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processTagTimev6
*
*  FUNCTION
*     This function formats the tag time associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processTagTimev6(BT_U16BIT* messagePTR, BT_UINT cardnum)
{
   BT1553_TIME tagTime;

   union bigtime
   {
      CEI_UINT64 time64;
      BT1553_TIME ttime;
   }bt, tt; 

   BT_U16BIT lsbTime;
   BT_U16BIT msbTime;
   BT_U16BIT TTmessage = 0x2800;

   // initialize all bits of bt and tt to 0
   bt.time64 = 0;
   tt.time64 = 0;

   bt.ttime = pbv6[cardnum].BMRecord.time;
   if(nsTimeTag)
      bt.time64/=1000;
   tagTime = bt.ttime;
   tt.ttime = bt.ttime;     // Perform timetag checks with the entire timetag, not just the lower 32-bits

#ifdef _TIME_CYCLE
   if(pbv6[cardnum].lastTagTime > tt.time64)    // Has time gone backwards?
   {
      pbv6[cardnum].timeOffset+=1000000;        // yes, add 1 second in uSec to avoid lockup
   }
#endif //_TIME_CYCLE

   // Use the entire timetag to calculate the new hw tag time
   pbv6[cardnum].lastTagTime = tt.time64;
   pbv6[cardnum].hwTagTime = (BT_U32BIT)(((tt.time64+startLatency+pbv6[cardnum].timeOffset)-pbv6[cardnum].startTimeDelay) << 1);
   *messagePTR = TTmessage;
   messagePTR++;

   lsbTime = (BT_U16BIT) (pbv6[cardnum].hwTagTime & 0x0000ffff);
   msbTime = (BT_U16BIT)((pbv6[cardnum].hwTagTime & 0xffff0000) >> 16);

   *messagePTR = lsbTime;
   messagePTR++;
   *messagePTR = msbTime;
   messagePTR++;

   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - buildXmitMessage
*
*  FUNCTION
*     This function builds the playback transmit message
*
*   Parameters
*     BT_U16BIT status    -> Status word used to evaluate message status
*     int count           -> Data word count
*     int message_sync    -> message sync 0 = positive sync; 1 = negative sync
*
*   Returns
*     XMIT_MESSAGE -> 16 bit transmit message structure
*
****************************************************************************/
static XMIT_MESSAGE buildXmitMessagev6(BT_U16BIT status, int count, int message_sync, BT_UINT cardnum)
{
   // Build xmit message code

   XMIT_MESSAGE message;
   message.xmit_code = 1;   // set transmit code
   message.gap_code = 0;    //set gap code to 0;
   message.notUsed = 0;     // set not used bits to 0;
   message.bitCount = 0;
   message.eop = 0;

   if((pbv6[cardnum].BMRecord.int_status & BT1553_INT_BUSB) == 0)
   {
      message.channel = 0;
   }
   else
   {
      message.channel = 1;
   }

   if((status & BT1553_INT_PARITY) == 0)
   {
      message.parityError = 0;
   }
   else
   {
      message.parityError = 1;
   }

   message.bitCount = 0; // Not used for now
   // set sync polarity

   if((status & BT1553_INT_INVERTED_SYNC) == 0)
   {
      message.syncPolarity = (BT_U16BIT)message_sync;
   }
   else
   {
      message.syncPolarity = (BT_U16BIT)(message_sync^1); //
   }
   message.wcount = (BT_U16BIT)count;

   return message;
}

/****************************************************************************
*
*  PROCEDURE NAME - filterRecord
*
*  FUNCTION
*     This function filters the *.bmd record based on the filtering criteria
*
*   Parameters
*     none
*
*   Returns
*     int -> Include Record = 1; Exclude Record = 0;
*
****************************************************************************/
static int filterRecordv6(BT_UINT cardnum)
{

   BT1553_TIME tagTime;

   union bigtime
   {
      CEI_UINT64 time64;
      BT1553_TIME ttime;
   }bt;

   // initialize all bits to zero
   bt.time64 = 0;

   bt.ttime = pbv6[cardnum].BMRecord.time;
   if(nsTimeTag)
      bt.time64/=1000;
   tagTime = bt.ttime;

   switch(pbv6[cardnum].filterFlag)   // 0 no filter, 1 = time filter, 2 = message filter
   {
      case 0:
         return includeRecord;
      case 1:
         bt.ttime = pbv6[cardnum].BMRecord.time;
         if(nsTimeTag)
            bt.time64/=1000;
         tagTime = bt.ttime;
         if((tagTime.topuseconds >= pbv6[cardnum].startTagTime.topuseconds) && 
            (tagTime.topuseconds <= pbv6[cardnum].stopTagTime.topuseconds))
         {
            if((tagTime.microseconds >= pbv6[cardnum].startTagTime.microseconds) && 
               (tagTime.microseconds <= pbv6[cardnum].stopTagTime.microseconds))
            {
               return includeRecord;
            }
         }
         return excludeRecord;
      case 2:
         if((pbv6[cardnum].BMRecord.messno >= pbv6[cardnum].messageNumberStartPoint) && 
          (pbv6[cardnum].BMRecord.messno <= pbv6[cardnum].messageNumberStopPoint))
         {
            return includeRecord;
         }
         if(pbv6[cardnum].BMRecord.messno > pbv6[cardnum].messageNumberStopPoint)
         {
            pbv6[cardnum].endOfFilterData = TRUE;
         }
         return excludeRecord;
      default:
         return includeRecord;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - periodicUpdate
*
*  FUNCTION
*     this function is invoked by the periodic update.  It checks the playback
*     status by reading the playback control register and checking for errors 
*     or end of playback
*
*   Parameters
*     unsigned cardnum        -> card number 
*     API_INT_FIFO *intFifo   -> Structure of data passed from timer interrupt 
*
*   Returns
*     BT_INT -> API_SUCCESS
*
****************************************************************************/

static BT_INT _stdcall periodicUpdatev6(BT_UINT cardnum, API_INT_FIFO *intFifo)
{
   //read the register and see if we need to send more data over
   BT_U32BIT temp;
   BT_U32BIT controlData;

   if(board_is_v5_uca[cardnum])
      temp = vbtGetFileRegister(cardnum,PB_TAIL_POINTER);
   else
      temp = vbtGetRegister[cardnum](cardnum,PB_TAIL_V6POINTER);  // This is a status monitor. Is tail pointer advancing?

   pbv6[cardnum].playbackStatus->tailPointer = temp;

   if(board_is_v5_uca[cardnum])
      controlData = vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);
   else
      controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);

   pbv6[cardnum].playbackStatus->playbackStatus = (BT_U16BIT)controlData;

   if((controlData & errorBit) != 0)
   {
      if(board_is_v5_uca[cardnum])
         vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);  // clear error with write data does not matter
      else
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);  // clear error with write data does not matter
   }

   if((controlData & bufferEmpty) != 0)
   {
      pbv6[cardnum].playbackStatus->playbackError = API_PLAYBACK_BUF_EMPTY;
   }

   if((controlData & runBit) == 0)
   {
      if(!pbv6[cardnum].moreDataInFile)
      {
         if(board_is_v5_uca[cardnum])
            vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
         else
            vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);

         pbv6[cardnum].playbackStatus->playbackError = API_SUCCESS;
#ifdef _UNIX_
//         pthread_mutex_lock(&pbv6[cardnum].hExitMutex);
	     pthread_cond_broadcast(&pbv6[cardnum].hExitEvent);
//         pthread_mutex_unlock(&pbv6[cardnum].hExitMutex);
#else
         SetEvent(pbv6[cardnum].hExitEvent);
#endif
         return API_SUCCESS;
      }
      else
      {
         pbv6[cardnum].playbackStatus->playbackError = API_PLAYBACK_BAD_EXIT;
#ifdef _UNIX_
//         pthread_mutex_lock(&pbv6[cardnum].hExitMutex);
	     pthread_cond_broadcast(&pbv6[cardnum].hExitEvent);
//         pthread_mutex_unlock(&pbv6[cardnum].hExitMutex);
#else
         SetEvent(pbv6[cardnum].hExitEvent);
#endif

         return API_SUCCESS;
      }

   }
   if( lookAheadBufferDataAvailablev6(1,cardnum) > 0 )
   {
      writeReformattedDataToBufferv6(cardnum);
      if(pbv6[cardnum].moreDataInFile)
      {
         fillLookAheadBufferv6(cardnum);
      }
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - playbackBufferFreeSpace
*
*  FUNCTION
*     this function is calculates the free space in the playback buffer.  Free
*     space is determined as the space between the end of the buffer and the 
*     head pointer if teh ehad pointer is greater than the tail pointer, or the
*     tail pointer minus the head pointer if the tail pointer is greater than
*     the head pointer.  This allow the maximum amount of data to be written
*     without having to worry about wrapping around the the end of the buffer.
*
*   Parameters
*     none 
*
*   Returns
*     BT_U32BIT -> amount of free space (in bytes) available
*
****************************************************************************/
static BT_U32BIT playbackBufferFreeSpacev6(BT_UINT cardnum)
{
   BT_U32BIT pbTail;
   BT_U32BIT pbHead32;
   BT_U32BIT pbTail32;

   if(board_is_v5_uca[cardnum])
   {
      pbTail   = vbtGetFileRegister(cardnum, PB_TAIL_POINTER);
      pbTail32 = pbTail << shiftBits;
   }
   else
   { 
      pbTail32  = vbtGetRegister[cardnum](cardnum, PB_TAIL_V6POINTER);
      pbTail32  = REL_ADDR(cardnum,pbTail32);
   }

   pbHead32 = pbv6[cardnum].playbackHeadPTR32 + bytesPerWord;
   if(pbHead32 == pbv6[cardnum].endOfPlaybackBuffer32)
   {
      pbHead32 = pbv6[cardnum].playbackBuffer32;
   }

   if(pbTail32 == pbHead32)  // full
   {
      return 0;
   }

   if(pbv6[cardnum].playbackHeadPTR32 > pbTail32)
   {
      if(pbTail32 == pbv6[cardnum].playbackBuffer32)
      {
         return (pbv6[cardnum].endOfPlaybackBuffer32-pbv6[cardnum].playbackHeadPTR32)-2;
      }
      else
      {
         return (pbv6[cardnum].endOfPlaybackBuffer32-pbv6[cardnum].playbackHeadPTR32);
      }
   }
   else
   {
      if(pbTail32 == pbv6[cardnum].playbackHeadPTR32)
      {
         return pbv6[cardnum].sizeOfPlaybackBuffer-2;
      }
      else
      {
         return (pbTail32 - pbv6[cardnum].playbackHeadPTR32)-2;
      }
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - lookAheadBufferDataAvailablev6
*
*  FUNCTION
*     this function is calculates the data available in the look ahead buffer.
*     Data available is determined as the data between the end of the buffer and the
*     tail pointer if the tail pointer is greater than the head pointer, or the
*     head pointer minus the tail pointer if the head pointer is greater than
*     the tail pointer.  This datermine the maximum amount of data available
*     without having to worry about wrapping around the the end of the buffer.
*
*   Parameters
*     none
*
*   Returns
*     BT_U32BIT -> amount of free space (in bytes) available
*
****************************************************************************/
static BT_U32BIT lookAheadBufferDataAvailablev6(BT_INT total, BT_UINT cardnum)
{
   if(pbv6[cardnum].lookAheadHeadPTR == pbv6[cardnum].lookAheadTailPTR)  // empty
   {
      return 0;
   }

   if(pbv6[cardnum].lookAheadHeadPTR <= pbv6[cardnum].lookAheadTailPTR)
   {
      if(total==1)
      {
         return (BT_U32BIT)((pbv6[cardnum].endOfBuffer-pbv6[cardnum].lookAheadTailPTR)+
          (pbv6[cardnum].lookAheadHeadPTR-pbv6[cardnum].lookAheadBuffer))*bytesPerWord;
      }
      else
      {
         return (BT_U32BIT)(pbv6[cardnum].endOfBuffer-pbv6[cardnum].lookAheadTailPTR)*bytesPerWord;
      }
   }
   else
   {
      return (BT_U32BIT)((((pbv6[cardnum].lookAheadHeadPTR-pbv6[cardnum].lookAheadTailPTR))*bytesPerWord));
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - readLogFileBlock
*
*  FUNCTION
*     This this function read a 64k Byte block of data from the specified .bmd 
*     file.  This function for now operates as a separate thread.
*
*   Parameters
*     LPVOID Param -> cardnum
*
*   Returns
*     BT_U32BIT -> API_SUCCESS
*
****************************************************************************/
#ifdef _UNIX_
static void *  readLogFileBlockv6(void * Param)
#else
static BT_U32BIT _stdcall readLogFileBlockv6(LPVOID Param)
#endif
{
   size_t  count;            // Number of BM messages we read
   BT_UINT cardnum;          // Current BusTools-API cardnum
   DWORD   status;           // Tells if we woke up just to terminate
#ifdef _UNIX_
   DWORD   statusExit = 99;  // Tells if we woke up just to terminate
#else 
   HANDLE  EventHandles[2];  // Events we wait for
#endif

   cardnum = (BT_UINT)((CEI_UINT64)Param); /* extraneous CEI_UINT64 cast for Wp64 warning */

#ifdef __WIN32__
   EventHandles[0] = pbv6[cardnum].blockReadEvent;  // Wait for either read or
   EventHandles[1] = pbv6[cardnum].hExitEvent;      //   exit.
#endif
   // reads 512 API_BM_MBUF records from the file per read
   while(pbv6[cardnum].diskFileNotEmpty)
   {
#ifdef _UNIX_

      if((status = pthread_cond_wait(&pbv6[cardnum].blockReadEvent,&pbv6[cardnum].blockReadMutex))==0
        || (statusExit = pthread_cond_wait(&pbv6[cardnum].hExitEvent,&pbv6[cardnum].hExitMutex))==0 )
      {
         pthread_mutex_unlock(&pbv6[cardnum].blockReadMutex);
//         pthread_mutex_unlock(&pbv6[cardnum].hExitMutex);
//         usleep(200);
      }
      else
        break;
      if(statusExit == 0)
      {
         printf("readLogFileBlock: statusExit = 0\n");
         break;
      }
     
      pthread_mutex_lock(&pbv6[cardnum].blockReadMutex);

#else
      status = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
      if ( status != WAIT_OBJECT_0 )
         break;   // Exit and distroy this thread if the exit event gets signaled.
#endif
      count = fread(pbv6[cardnum].diskReadPTR,sizeof(API_BM_MBUF),readSize,pbv6[cardnum].filePTR);
      if(count == readSize)
      {
         pbv6[cardnum].diskReadPTR+=readSize;
         if(pbv6[cardnum].diskReadPTR == pbv6[cardnum].endOfDiskBuffer)
         {
            pbv6[cardnum].diskReadPTR  = pbv6[cardnum].diskBuffer;
         }
      }
      else
      {
         if(feof(pbv6[cardnum].filePTR) != 0)
         {
            pbv6[cardnum].diskReadPTR+=count;
            *pbv6[cardnum].diskReadPTR = pbv6[cardnum].endRecord;
            pbv6[cardnum].diskReadPTR++;
            pbv6[cardnum].diskFileNotEmpty = FALSE;
         }
         if( ferror(pbv6[cardnum].filePTR) !=0 )
         {
            pbv6[cardnum].playbackStatus->playbackError = API_PLAYBACK_DISK_READ;
            *pbv6[cardnum].diskReadPTR = pbv6[cardnum].endRecord;
            clearerr(pbv6[cardnum].filePTR);
         }
      }
#ifdef _UNIX_
        pthread_mutex_unlock(&pbv6[cardnum].blockReadMutex);
//        MSDELAY(10);
      pthread_mutex_lock(&pbv6[cardnum].readCompleteMutex);
//      pthread_cond_broadcast(&pbv6[cardnum].readCompleteEvent);
//      pthread_mutex_unlock(&pbv6[cardnum].readCompleteMutex);
#else
      SetEvent(pbv6[cardnum].readCompleteEvent);
#endif
   }
#ifdef _UNIX_
   pthread_exit((void *)0);
#else
   ExitThread(0);
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - getNextRecord
*
*  FUNCTION
*     This this function get the next .bmd record in the disk buffer.  When 512
*     records are read this function issues a blockReadEvent to signal that
*     there is space to read the next 64k Byte block of data from the file.
*
*   Parameters
*     none
*
*   Returns
*     API_BM_MBUF -> structure of Bus Monitor data.
*
****************************************************************************/
static API_BM_MBUF getNextRecordv6(BT_UINT cardnum)
{
   API_BM_MBUF BMRecord;
#ifdef _UNIX_
   BT_INT status;
#endif
   BMRecord = *pbv6[cardnum].diskBufferPTR;

   pbv6[cardnum].bufferCount++;
   if( pbv6[cardnum].bufferCount == 512 && pbv6[cardnum].diskFileNotEmpty)
   {
#ifdef _UNIX_

      pthread_mutex_lock(&pbv6[cardnum].blockReadMutex); 
      status = pthread_cond_signal(&pbv6[cardnum].blockReadEvent);
      pthread_mutex_unlock(&pbv6[cardnum].blockReadMutex); 
      pbv6[cardnum].bufferCount = 0;
      //MSDELAY(10);
//	  pthread_mutex_lock(&pbv6[cardnum].readCompleteMutex);
//	  status = pthread_cond_wait(&pbv6[cardnum].readCompleteEvent,&pbv6[cardnum].readCompleteMutex);
//	  pthread_mutex_unlock(&pbv6[cardnum].readCompleteMutex);
//          MSDELAY(50);
#else
      SetEvent(pbv6[cardnum].blockReadEvent);
      pbv6[cardnum].bufferCount = 0;
      WaitForSingleObject(pbv6[cardnum].readCompleteEvent, INFINITE);
#endif
   }

   pbv6[cardnum].diskBufferPTR++;
   if(pbv6[cardnum].diskBufferPTR == pbv6[cardnum].endOfDiskBuffer)
   {
      pbv6[cardnum].diskBufferPTR = pbv6[cardnum].diskBuffer;
   }
   return BMRecord;
}

/****************************************************************************
*
*  PROCEDURE NAME - fillLookAheadBufferv6
*
*  FUNCTION
*     This this function fills the look ahead buffer with data from data stored
*     in the disk buffer.
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
static void fillLookAheadBufferv6(BT_UINT cardnum)
{
   BT_U16BIT *message;
   int     messageIndex;
   BT_UINT msgSize = 164;
   int     bufferSize;

   BT_INT total = 1;
   while((sizeOfLookAheadBufferv6 - lookAheadBufferDataAvailablev6(total,cardnum)) >= msgSize)
   {
      pbv6[cardnum].BMRecord = getNextRecordv6(cardnum);  // need to look at start stop info to see where to start/stop read
      if((pbv6[cardnum].BMRecord.messno != 0) && (!pbv6[cardnum].endOfFilterData))
      {
         if(filterRecordv6(cardnum))
         {
            pbv6[cardnum].playbackStatus->recordsProcessed++;
            message = pbv6[cardnum].messagePacket;      // point to start of next message
            bufferSize = processTheRecordv6(cardnum);   // This includes parsing the BM struct, calculating gap, determining errors, etc.
            for(messageIndex=0;messageIndex<bufferSize;messageIndex++) // This is the initial file of the circ buffer
            {
               *pbv6[cardnum].lookAheadHeadPTR = *message;
               pbv6[cardnum].lookAheadHeadPTR++;
               message++;

               if(pbv6[cardnum].lookAheadHeadPTR == pbv6[cardnum].endOfBuffer)
               {
                  pbv6[cardnum].lookAheadHeadPTR = pbv6[cardnum].lookAheadBuffer;
               }
            }
         }
      }
      else
      {
         // need to write an end of of playback record here.
         *pbv6[cardnum].lookAheadHeadPTR = pbv6[cardnum].eopMessage;
         pbv6[cardnum].lookAheadHeadPTR++;
         pbv6[cardnum].moreDataInFile = FALSE;
         break;
      }
   }
}
#endif //_PLAYBACK_

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_Stop
*
*  FUNCTION
*     This routine halts playback
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV Playback_Stopv6(
   BT_UINT cardnum)         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   BT_U32BIT controlData;

   // Cause the Playback threads to exit.
#ifdef _UNIX_
//   pthread_mutex_lock(&pbv6[cardnum].hExitMutex);
   pthread_cond_broadcast(&pbv6[cardnum].hExitEvent);
//   pthread_mutex_unlock(&pbv6[cardnum].hExitMutex);
#else
   SetEvent(pbv6[cardnum].hExitEvent);
#endif

   controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);
   if((controlData & runBit) == 0)  // reset for normal shutdown, clear errors
   {
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
   }
   else  // reset for forced shutdown, clear errors
   {
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x20 + PB_SET);
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
   }
   MSDELAY(20); //

   controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);

   if((controlData & runBit) == 0)
      return API_SUCCESS;

   vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x40 + PB_SET); // Stop Playback did not work so halt
   MSDELAY(20);

   if((controlData & runBit) == 0)
      return API_SUCCESS;
   else
     return 666;   
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_checkv6
*
*  FUNCTION
*     This routine checks playback for time gaps and time sequence problems
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_Playback_Checkv6(char * bmdfile, BT_UINT mins)         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   FILE * rptr;

   BT_INT scount=0,gcount=0;
   CEI_UINT64 last_big_time = 0;
   API_BM_MBUF bm_mbuf;
   CEI_UINT64 LARGE_GAP = (1000000 * 60 * mins); //  
   int last_big_time_valid = FALSE;

   union ttm
   {
      CEI_UINT64 big_time;
      BT1553_TIME updated_time;
   } TTM;
 
   TTM.big_time = 0;
   last_big_time = 0x0;  // 

   if((rptr=fopen(bmdfile,"rb"))==NULL)
   {  
      return API_PLAYBACK_BAD_FILE; // error opening the bmd file
   }

   if (fread(&bmdHeader,sizeof(BMDX_HEADER),1,rptr)!= 1)
   {
      fclose(rptr);
      return API_PLAYBACK_FILE_ERR;
   }

   if(bmdHeader.uFormatInfo & 0x1)
      nsTimeTag = 1;
   else
      nsTimeTag = 0;

   //while(!feof(rptr))
   while (1)
   { 
      if (fread(&bm_mbuf,sizeof(API_BM_MBUF),1,rptr)!=1)
      {
         if(feof(rptr))
         {
            break;
         }
         else
         {
            fclose(rptr);
            return API_PLAYBACK_FILE_ERR;
         }
      }

      TTM.big_time = (((CEI_UINT64)(bm_mbuf.time.topuseconds))<<32) | (bm_mbuf.time.microseconds);

      if(nsTimeTag)
         TTM.big_time/=1000; //Convert to usec from nanoseconds
      
      bm_mbuf.time = TTM.updated_time;

      if (last_big_time_valid)
      {
          if(TTM.big_time <= last_big_time)
             scount++;
          else if(last_big_time != 0)
          {
             if((TTM.big_time - last_big_time) > LARGE_GAP)
              gcount++;
          }
      }
      last_big_time = TTM.big_time;
      last_big_time_valid = TRUE;
   }


   fclose(rptr);

   if(scount)
      return API_PLAYBACK_TIME_ORDER;
   if(gcount)
      return API_PLAYBACK_TIME_GAP;
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}  

/************** Legacy Playback for bmd files on v5 and earlier ***************/

/*============================================================================*
 * FILE:                     P L A Y B A C K . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 1999-2010 BY ABACO SYSTEMS, INC. 
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
 *             Bus Playback function.
 *
 *             This module read a specified .bmd file and controls the playback
 *             of the data in the file over the 1553 bus.
 *
 *             Playback work by reading data from a BusMonitor file (.bmd) and
 *             converting the data into a format used by the playback firmware 
 *             to recreate the signal on the 1553 bus.  
 *
 *             To do this three buffer are used to store the data.  The first 
 *             buffer, the disk buffer, stores the raw bus monitor data.  This 
 *             allows playback read a block of data from the disk.  Playback
 *             then converts this data into the required format and stores that 
 *             information into a lookahead buffer.  The data in the lookahead  
 *             buffer is then written to the buffer on the board.
 *             For large .bmd files a pipeline is created from the disk file  
 *             playback buffer on the PCI board.  The speed at which the PCI card 
 *             through the disk buffer and lookahead buffer to the processes 
 *             playback message determines the rate at which the playback buffer 
 *             is replenished.  The playback software setups up a timed interrupt
 *             that invokes the periodicUpdate routine. This routine checks for
 *             the end-of-playback and fills the playback buffer, lookahead   
 *             buffer and disk buffer as needed. 
 *
 * EXTERNAL ENTRY POINTS:
 *
 *    BusTools_Playback       Starts the playback process.
 *    BusTools_Playback_Check Checks the BMD file for time gap or time sequence problems.
 *    BusTools_Playback_Stop  Stops the playback process.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *  initPlayback    - Initializes Playback data and hardware
 *  setupDiskBuffer - Sets up the disk buffer used to store the raw playback data
 *  setupLookAheadBuffer  - Sets up the LookAhead Buffer used to store processed data
 *  setupPlaybackBuffer   - Sets up the playback buffer on teh PCI 1553 card
 *  setupThreadsAndEvents - Sets up the runPlayback Thread and events
 *  setupAPI_INT_FIFO     - Sets up the API_INT_FIFO used to setup the time interrupt
 *  setupTheLookAheadBuffer - Set up the look ahead buffer that stores formatted 
 *                            records
 *  setupThePlaybackBuffer  - Sets up the RAM playback buffer on the PCI 1553
 *  setUpThreadsAndEvents   - Sets upo the Threads and events used to control playback
 *  readLogFileBlock        - Reads a block of data form the disk file
 *  processTheRecord        - Processes on file record into playback format
 *  processBC_RT            - Formats a BC to RT (receive) command
 *  processRT_BC            - Formats a RT to BC (transmit) command
 *  processRT_RT            - Formats a RT to RT command
 *  buildXmitMessage        - Builds the playback command message
 *  processCommand          - Formats the 1553 command record
 *  processResponseGap      - Calculate the response gap
 *  processData             - Format the message data
 *  processStatus           - Format the status record
 *  processTagTime          - Format tag time.
 *  fillLookAheadBuffer     - Fills the lookahead buffer with formatted records
 *  writeReformattedDataToBuffer - Write the formatted playback record to the
 *                                 playback buffer
 *  advancePlaybackHeadPTR - Keeps track of playback buffer head pointer
 *  playbackBufferFreeSpace      - Calculates the free space in the playback buffer
 *  lookAheadBufferDataAvailable - Calcultes the data available in the lookahead buffer
 *  filterRecord    - Filters the .bmd records based on message number
 *  periodicUpdate  - Update routine called periodically to process records.
 *  runPlayback     - Starts playback after initialization and setup complete
 *===========================================================================*/

/* $Revision:  5.40 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/05/1999   Initial version. V3.23 rhc
  01/18/2000   Added test for valid boards which support playback.V4.00.ajh
  03/07/2001   Added playback to Linux API V4.30-RHC rhc
  2/15/2002    Added support for modular API. v4.48
  07/11/2005   Added Fix for Playback stop.
  08/12/2005   Added New Function BusTools_Playback_Check to test for time gaps.
 */

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback
*
*  FUNCTION
*     This routine handles starting bus playback.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_FILE   -> Error in openning .bmd file
*     API_PLAYBACK_INIT_ERROR -> Playback initialization Error
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_PlaybackV5(
   BT_UINT cardnum,           // (i) card number (0 - based)
   API_PLAYBACK playbackData)  // (io) Control and Status structures
{
#if defined(_PLAYBACK_)

#ifdef _UNIX_
   pthread_t pbThread;         // Handle to the main playback thread.
   int status=0;
#else
   HANDLE    pbThread;         // Handle to the main playback thread.
   BT_U32BIT ThreadID;         // Thread ID of the main playback thread.
#endif
   int       i;                // Loop counter
   int scnt=0;

     union bigtime
   {
      CEI_UINT64 time64;
      BT1553_TIMEV5 ttime;
   }bt;

   // initialize all bits to zero
   bt.time64 = 0;

   // Verify that the selected board supports playback
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   // Save the Playback Status Structure pointer in our local table.

   pb[cardnum].playbackStatus = playbackData.status;

   if(pb[cardnum].filterFlag == messageNumberFilter)
   {
      fseek(pb[cardnum].filePTR,(pb[cardnum].messageNumberStartPoint-1)*sizeof(API_BM_MBUFV5),SEEK_SET);
      fread(&pb[cardnum].BMRecord,sizeof(API_BM_MBUFV5),1,pb[cardnum].filePTR);
      fseek(pb[cardnum].filePTR,(pb[cardnum].messageNumberStartPoint-1)*sizeof(API_BM_MBUFV5),SEEK_SET);
//      pb[cardnum].startTimeDelay = pb[cardnum].BMRecord.time.microseconds;
      bt.ttime = pb[cardnum].BMRecord.time;
      pb[cardnum].startTimeDelay = bt.time64;
   }
   else
   {
      fread(&pb[cardnum].BMRecord,sizeof(API_BM_MBUFV5),1,pb[cardnum].filePTR);  // Read first record
      fseek(pb[cardnum].filePTR,0,SEEK_SET);
//      pb[cardnum].startTimeDelay = pb[cardnum].BMRecord.time.microseconds;
      bt.ttime = pb[cardnum].BMRecord.time;
      pb[cardnum].startTimeDelay = bt.time64;
   }
   i = 0;  // Fill all 4 of the buffers
   do
   {
#ifdef _UNIX_

      pthread_mutex_lock(&pb[cardnum].blockReadMutex); 
      status = pthread_cond_signal(&pb[cardnum].blockReadEvent);  //fill the 2048 bm record disk buffer
      pthread_mutex_unlock(&pb[cardnum].blockReadMutex); 
#else
      SetEvent(pb[cardnum].blockReadEvent);  // this fill the 2048 bm record disk buffer
      WaitForSingleObject(pb[cardnum].readCompleteEvent, INFINITE);
#endif
   }
   while (pb[cardnum].diskFileNotEmpty && (++i < numberOfBlocks) );

   fillLookAheadBuffer(cardnum);  // get initial data from disk and store in lookAheadBuffer
   writeReformattedDataToBuffer(cardnum);   // move data in lookAheadBuffer to the play to playBackBuffer
   playback_is_running[cardnum] = 1;
   if(pb[cardnum].moreDataInFile)
   {
      fillLookAheadBuffer(cardnum);  // fill the lookahead buffer again so its filled for start of playback
   }

   // Create the main playback thread here then return status to calling routine.

#ifdef _UNIX_

   status = pthread_create(&pbThread,NULL,runPlayback,(void *)cardnum);
   if(status)
   {
      pb[cardnum].playbackStatus->playbackError = API_SUCCESS;
      return API_PLAYBACK_BAD_THREAD;   // Return error
   }
   usleep(9000);
#else
   pbThread = CreateThread(NULL, stackSize, runPlayback,
                           (LPVOID)((CEI_UINT64)cardnum), CREATE_SUSPENDED, &ThreadID); /* extraneous CEI_UINT64 cast for Wp64 warning */
   if ( pbThread == NULL ) // Make sure the thread was created properly
   {
      return API_PLAYBACK_BAD_THREAD;   // Return error
   }                                             // Set the thread priority to normal
   SetThreadPriority(pbThread, THREAD_PRIORITY_ABOVE_NORMAL);

   // Everything's set, let it run.  It will execute playback.
   ResumeThread(pbThread);

#endif
   pb[cardnum].playbackStatus->playbackError = API_SUCCESS;
   return API_SUCCESS;

#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

#ifdef _PLAYBACK_
/****************************************************************************
*
*  PROCEDURE NAME - runPlayback
*
*  FUNCTION
*     This routine runs bus Playback as a separate thread.  When it is time
*     to terminate, this routine cleans up all of the handles and releases
*     all of the allocated memory.  It also unregisters the polling function.
*
*   Parameters
*     LPVOID Param            -> cardnum
*
*   Returns
*     API_SUCCESS             -> success
****************************************************************************/
#ifdef _UNIX_
void * runPlayback(void * Param)
#else
static BT_U32BIT _stdcall runPlayback(LPVOID Param)
#endif
{
   BT_UINT cardnum;
//#ifdef _UNIX_
   BT_INT status=0;
//#endif

   cardnum = (BT_UINT)((CEI_UINT64)Param); /* extraneous CEI_UINT64 cast for Wp64 warning */

   // register the periodicUpdate function to run periodically
   BusTools_RegisterFunction(cardnum,&pb[cardnum].intFifo, REGISTER_FUNCTION);

   //set the playback runbit to start playback running

   if(board_is_v5_uca[cardnum])
      vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x0001);
   else
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x1 + PB_SET);

   // wait for end of playback
#ifdef _UNIX_
//   pthread_mutex_lock(&pb[cardnum].hExitMutex);

   status = pthread_cond_wait(&pb[cardnum].hExitEvent,&pb[cardnum].hExitMutex);
//   pthread_mutex_unlock(&pb[cardnum].hExitMutex);
#else
   WaitForSingleObject(pb[cardnum].hExitEvent, INFINITE);
#endif
   //stop the periodic update function from being called
   if(board_is_v5_uca[cardnum])
      vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x0); // make sure the run bit is cleared
   else
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x0000FFFF + PB_CLEAR); // make sure the run bit is cleared

   BusTools_RegisterFunction(cardnum,&pb[cardnum].intFifo, UNREGISTER_FUNCTION);
   // Wait for the other Playback threads to exit.

   MSDELAY(40);   // Give the other threads a chance to shutdown

   // Clean up the handles and free all of the memory blocks we allocated
#ifdef _UNIX_
   pthread_cond_destroy(&pb[cardnum].hExitEvent);
   pthread_cond_destroy(&pb[cardnum].blockReadEvent);
   pthread_cond_destroy(&pb[cardnum].readCompleteEvent);
#else
//   CloseHandle(pb[cardnum].hExitEvent);
//   CloseHandle(pb[cardnum].blockReadEvent);
//   CloseHandle(pb[cardnum].readCompleteEvent);
#endif
   free(pb[cardnum].messagePacket);
   free(pb[cardnum].diskBuffer);
   free(pb[cardnum].lookAheadBuffer);
   fclose(pb[cardnum].filePTR);

   CEI_THREAD_EXIT(&status, NULL);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - initPlayback
*
*  FUNCTION
*     This routine initializes playback
*
*   Parameters
*     API_PLAYBACK playbackData -> Structure of playback data used for initialization
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
*
****************************************************************************/
static BT_INT initPlayback(BT_UINT cardnum, API_PLAYBACK playbackData)
{
   BT_INT    status;
   BT_U32BIT controlData;

   // Magic numbers
   pb[cardnum].blockCount    = 0x0400;
   pb[cardnum].eopMessage    = 0x01e00;
   // initialize data
   pb[cardnum].endRecord.messno = 0;
   pb[cardnum].bufferCount      = 0;
   pb[cardnum].moreDataInFile   = TRUE;
   pb[cardnum].timeOffset       = 0;
   pb[cardnum].lastTagTime      = 0;
   pb[cardnum].lastRespTime     = 0;
   pb[cardnum].endOfFilterData  = FALSE;
   pb[cardnum].diskFileNotEmpty = TRUE;
   pb[cardnum].startTimeDelay   = 0;

   // Stop playback just in case
   if(board_is_v5_uca[cardnum])
   {
      // Stop playback just in case
      controlData = (BT_U32BIT)vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);
      if((controlData & runBit)!=0)
      {
         vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x20);
         vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
         return API_PLAYBACK_RUNNING;
      }
      // clear error just in case
      vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
   }
   else
   {
      controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);
      if((controlData & runBit)!=0)
      {
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x20 + PB_SET);
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
         return API_PLAYBACK_RUNNING;
      }
      // clear error just in case
      vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);
   }

   // allocate space for message packet this should be enough for max case
   pb[cardnum].messagePacket = (BT_U16BIT *)malloc(256);
   if(pb[cardnum].messagePacket==NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   getPlaybackData(playbackData,cardnum);
   if((status = setupTheDiskBuffer(cardnum))!=API_SUCCESS)
   {
      return status;
   }

   if((status = setupTheLookAheadBuffer(cardnum))!=API_SUCCESS)
   {
      return status;
   }
   status = BusTools_BC_Init(cardnum,0,0,0,16,14,1000,1);
   if(status == API_SUCCESS)
   {
      status = setupThePlaybackBuffer(cardnum);
   }

   if(status == API_SUCCESS)
   {
      setupAPI_INT_FIFO(cardnum); //set up the time interrut
      status = setUpThreadsAndEvents(cardnum);
   }
   
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupTheLookAheadBuffer
*
*  FUNCTION
*     This routine sets up the look ahead buffer used to store reformatted
*     Bus Monitor records.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
****************************************************************************/
static BT_INT setupTheLookAheadBuffer(BT_UINT cardnum)
{
   pb[cardnum].lookAheadBuffer = (BT_U16BIT*)malloc((size_t)sizeOfLookAheadBuffer);
   if(pb[cardnum].lookAheadBuffer == NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   pb[cardnum].lookAheadHeadPTR = pb[cardnum].lookAheadBuffer;
   pb[cardnum].endOfBuffer = (pb[cardnum].lookAheadBuffer+sizeOfLookAheadBuffer/bytesPerWord);
   pb[cardnum].lookAheadTailPTR = pb[cardnum].lookAheadBuffer;
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupTheDiskBuffer
*
*  FUNCTION
*     This routine sets up the disk buffer used to store Bus Monitor records.
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_MEMORY -> Error in allocating host memory
*
****************************************************************************/
static BT_INT setupTheDiskBuffer(BT_UINT cardnum)
{
   pb[cardnum].diskBuffer = (API_BM_MBUFV5*)malloc((size_t)sizeOfDiskBuffer);
   if(pb[cardnum].diskBuffer == NULL)
   {
      return API_PLAYBACK_BAD_MEMORY;
   }
   pb[cardnum].diskBufferPTR = pb[cardnum].diskBuffer;
   pb[cardnum].diskReadPTR = pb[cardnum].diskBuffer;
   pb[cardnum].endOfDiskBuffer = (pb[cardnum].diskBuffer+readSize*numberOfBlocks);
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupThePlaybackBuffer
*
*  FUNCTION
*     This routine sets up the playback buffer on the PCI 1553. This is a circular
*     buffer that the firmware uses to run playback.  This routine allocates memory,
*     set the head and tail pointers, and determines the end of the buffer.
*
*   Returns
*     API_SUCCESS -> success
****************************************************************************/
static BT_INT setupThePlaybackBuffer(BT_UINT cardnum)
{
   BT_INT    status;                 // Function return status
   BT_U32BIT address_of_end_of_blk;  // Actual end of buffer
   BT_U16BIT playbackBuffer16;       // Register version
   BT_U16BIT endOfPlaybackBuffer16;  // Register version

   // Allocate a large block of BusTools memory.
   status = BusTools_BC_MessageAlloc(cardnum, numberOfMessages);
   if ( status != API_SUCCESS )
   {
      return status;
   }
   // Get the starting and ending addresses of the allocated memory block.
   // The playback buffer must start on an address which is a multiple of
   //  8 words; BC messages start on such an address.
   status = BusTools_GetAddr(cardnum, GETADDR_BCMESS,
                             &pb[cardnum].playbackBuffer32,
                             &address_of_end_of_blk);
   if ( status != API_SUCCESS )
   {
      return status;
   }

   // Initialize PB Start, Head and Tail registers to the buffer head pointer.
   pb[cardnum].playbackHeadPTR32 = pb[cardnum].playbackBuffer32;

   if(board_is_v5_uca[cardnum])
   {
      playbackBuffer16  = (BT_U16BIT)(pb[cardnum].playbackBuffer32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_START_POINTER, playbackBuffer16);
      vbtSetFileRegister(cardnum, PB_HEAD_POINTER,  playbackBuffer16);
      vbtSetFileRegister(cardnum, PB_TAIL_POINTER,  playbackBuffer16);
   }
   else
   {
      vbtSetRegister[cardnum](cardnum, PB_START_V6POINTER, RAM_ADDR(cardnum,pb[cardnum].playbackBuffer32));
      vbtSetRegister[cardnum](cardnum, PB_HEAD_V6POINTER,  RAM_ADDR(cardnum,pb[cardnum].playbackBuffer32));
      vbtSetRegister[cardnum](cardnum, PB_TAIL_V6POINTER,  RAM_ADDR(cardnum,pb[cardnum].playbackBuffer32));
   }

   // Compute End of Buffer.  Buffer size must be a multiple of 8 words/16 bytes.
   pb[cardnum].sizeOfPlaybackBuffer  = address_of_end_of_blk - pb[cardnum].playbackBuffer32;
   if(board_is_v5_uca[cardnum])
      pb[cardnum].sizeOfPlaybackBuffer &= 0xFFFFFFFF << shiftBits;
   else
      pb[cardnum].sizeOfPlaybackBuffer+=1;
      
   pb[cardnum].endOfPlaybackBuffer32 = pb[cardnum].playbackBuffer32+pb[cardnum].sizeOfPlaybackBuffer;

   // write end address of playback buffer to register (multiple of 16 bytes)
   if(board_is_v5_uca[cardnum])
   {
      endOfPlaybackBuffer16 = (BT_U16BIT)(pb[cardnum].endOfPlaybackBuffer32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_END_POINTER, endOfPlaybackBuffer16);
      vbtSetFileRegister(cardnum, PB_INT_THRSHLD, (BT_U16BIT)pb[cardnum].blockCount);
      vbtSetFileRegister(cardnum, PB_THRSHLD_CNT, (BT_U16BIT)pb[cardnum].blockCount);
   }
   else
   {
      vbtSetRegister[cardnum](cardnum, PB_END_V6POINTER, RAM_ADDR(cardnum,pb[cardnum].endOfPlaybackBuffer32)-2);
      vbtSetRegister[cardnum](cardnum, PB_INT_V6THRSHLD, (BT_U32BIT)pb[cardnum].blockCount);
      vbtSetRegister[cardnum](cardnum, PB_THRSHLD_V6CNT, (BT_U32BIT)pb[cardnum].blockCount);
   }
   return status;
}

/****************************************************************************
*
*  PROCEDURE NAME - setUpThreadsAndEvents
*
*  FUNCTION
*     This routine hsets up the thread and events used to run playback..
*
*   Parameters
*     none
*
*   Returns
*     API_SUCCESS             -> success
*     API_PLAYBACK_BAD_EVENT  -> Error in creating an event
*     API_PLAYBACK_BAD_THREAD -> Error in creating a thread
*
****************************************************************************/

static BT_INT setUpThreadsAndEvents(BT_UINT cardnum)
{

#ifdef _UNIX_
   int status;
   pthread_t readThread;
#else
   HANDLE    readThread;     // Handle to the (readLogFileBlock) thread.
   BT_U32BIT readThreadID;   // Thread ID of the (readLogFileBlock) thread.
#endif
   // Create the events that the read file (readLogFileBlock) thread uses:
   // Create the exit event for playback completion
#ifdef _UNIX_
   status = pthread_cond_init(&pb[cardnum].hExitEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_mutex_init(&pb[cardnum].hExitMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pb[cardnum].hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if(pb[cardnum].hExitEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Create the read event for playback disk reads
#ifdef _UNIX_
   status = pthread_mutex_init(&pb[cardnum].blockReadMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_cond_init(&pb[cardnum].blockReadEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pb[cardnum].blockReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   if(pb[cardnum].blockReadEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Create the read completion event for playback disk reads
#ifdef _UNIX_
   status = pthread_cond_init(&pb[cardnum].readCompleteEvent,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
   status = pthread_mutex_init(&pb[cardnum].readCompleteMutex,NULL);
   if(status)
      return API_PLAYBACK_BAD_EVENT;
#else
   pb[cardnum].readCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   if(pb[cardnum].readCompleteEvent == NULL)
   {
      return API_PLAYBACK_BAD_EVENT;
   }
#endif
   // Don't pass in a dynamic structure which gets deleted; that is not reliable.
#ifdef _UNIX_
   status = pthread_create(&readThread,NULL,readLogFileBlock,(void *)cardnum);
   if(status)
      return API_PLAYBACK_BAD_THREAD;
#else
   readThread = CreateThread(NULL, stackSize, readLogFileBlock,
                             (LPVOID)((CEI_UINT64)cardnum), CREATE_SUSPENDED, &readThreadID); /* extraneous CEI_UINT64 cast for Wp64 warning */
   if ( readThread == NULL )      // Make sure the thread was created properly
   {
      return API_PLAYBACK_BAD_THREAD;
   }
   // Set the thread priority to normal
   SetThreadPriority(readThread, THREAD_PRIORITY_NORMAL);

   ResumeThread(readThread);
   // Close the thread handle since we don't need it anymore.
//   CloseHandle(readThread);
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - setupAPI_INT_FIFO
*
*  FUNCTION
*     This routine sets data in the API_INT_FIFO that controls the timer
*     interrupt used to invoke the playback periodicUpdate function.
*     The timer is set for 50 MS.
*
*   Parameters
*     intFifo (Global) is initialized.  All required parameters are set.
*
*   Returns
*     none
*
****************************************************************************/

static void setupAPI_INT_FIFO(BT_UINT cardnum)
{
   pb[cardnum].intFifo.function       = periodicUpdate;         // Function to call
   pb[cardnum].intFifo.iPriority      = THREAD_PRIORITY_ABOVE_NORMAL; // Thread priority
   pb[cardnum].intFifo.dwMilliseconds = 50;  // 50 millisecond timed interrupt
   pb[cardnum].intFifo.iNotification  = 0;   // Don't call at startup or shutdown
   pb[cardnum].intFifo.bForceShutdown = 0;   // Don't call at startup or shutdown
   pb[cardnum].intFifo.FilterType     = 0;   // Don't call for any 1553 bus events
   pb[cardnum].intFifo.cardnum        = cardnum;
}

/****************************************************************************
*
*  PROCEDURE NAME - getPlaybackData
*
*  FUNCTION
*     This routine gets the data in the API_PLAYBACK structure and stored the
*     data.  It also set up any filter parameters.
*
*   Parameters
*     API_PLAYBACK playbackData -> Structure containing the playback data passed
*                                  calling routine.
*
*   Returns
*     none
*
****************************************************************************/

static void getPlaybackData(API_PLAYBACK playbackData, BT_UINT cardnum)
{
   BT_U32BIT maskBit;
   BT_INT index;

   pb[cardnum].playbackStatus = playbackData.status;
   pb[cardnum].playbackStatus->recordsProcessed = 0;
   pb[cardnum].filterFlag = playbackData.filterFlag;
   switch(pb[cardnum].filterFlag)   // 0 no filter, 1 = time filter, 2 = message number filter
   {
      case 0:
         pb[cardnum].noFilter = TRUE;
         break;
      case 1:
         pb[cardnum].startTagTime.microseconds = playbackData.timeStart.microseconds;
         pb[cardnum].startTagTime.topuseconds  = (BT_U16BIT)playbackData.timeStart.topuseconds;
         pb[cardnum].stopTagTime.microseconds  = playbackData.timeStop.microseconds;
         pb[cardnum].stopTagTime.topuseconds   = (BT_U16BIT)playbackData.timeStop.topuseconds;
         pb[cardnum].noFilter = FALSE;
         break;
      case 2:
         pb[cardnum].messageNumberStartPoint = playbackData.messageStart;
         pb[cardnum].messageNumberStopPoint  = playbackData.messageStop;
         pb[cardnum].noFilter = FALSE;
         break;
      default:
         pb[cardnum].noFilter = TRUE;
         break;
   }

   maskBit = 1;
   for(index = 0; index < numberOfRT; index++)
   {
      pb[cardnum].RTarray[index] = playbackData.activeRT & maskBit;
      maskBit = maskBit<<1;
   }
   
   if(playbackData.Subadress31Flag == 1)   // 0 = SA 31 is not a mode code, 1 = SA31 is mode code
   {
      pb[cardnum].modeCode2 = 31;
   }
   else
   {
      pb[cardnum].modeCode2 = 0;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - processTheRecord
*
*  FUNCTION
*     This routine determines the 1553 message type and invokes the proper
*     method to process the message into the format required by playback.
*
*   Parameters
*     none
*
*   Returns
*     int bufferSize -> Size of the reformatted playback record stored in the
*     messagePacket buffer.
*
****************************************************************************/
static int processTheRecord(BT_UINT cardnum)
{
   int bufferSize;
   int BrdcMC = FALSE;

   // test for broadcast mode code messages: RT31, s/a == 0
   // or (if enabled) s/a == 0x1f AND if the word count 
   // is less than 0x10 (mode codes with no data)
   if(pb[cardnum].BMRecord.int_status & BT1553_INT_MODE_CODE)
   {
      if(pb[cardnum].BMRecord.int_status & BT1553_INT_BROADCAST)
         BrdcMC = TRUE;
      else 
         BrdcMC = FALSE;

   }
   
   //Test for RT_RT message

   if((pb[cardnum].BMRecord.int_status & BT1553_INT_RT_RT_FORMAT) != 0)
   {
       bufferSize = processRT_RT(cardnum);
   }
   else
   {
      if(pb[cardnum].BMRecord.command1.tran_rec)
      {
		  if (BrdcMC)
			  bufferSize = processBC_RT(cardnum);
		  else
	         bufferSize = processRT_BC(cardnum);
      }
      else
      {
         bufferSize = processBC_RT(cardnum);
      }

   }
   return bufferSize;
}

/****************************************************************************
*
*  PROCEDURE NAME - writeReformattedDataToBuffer
*
*  FUNCTION
*     This routine writes the formatted playback data stored in the look ahead
*     buffer into the playback buffer located on the PCI 1553 card.  This function
*     determines the available space int he playback buffer and the amount of
*     data and the look ahead buffer and writes the smaller of the two number bytes
*     to the playback buffer.   After the write, the look ahead buffer tail
*     pointer and playback buffer head pointer are advanced.
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
static void writeReformattedDataToBuffer(BT_UINT cardnum)
{
   BT_U32BIT space, data;

   space = playbackBufferFreeSpace(cardnum);        // available space in playback buffer
   data  = lookAheadBufferDataAvailable(0,cardnum);  // available data in look ahead buffer

#ifdef showspace
#endif

   if(data != 0 && space != 0)
   {
      if (space > data)   // if there is more space in the play back buffer than available data, then write only data
      {
         //block write of data 64Kbytes
         if(board_is_v5_uca[cardnum])
            vbtWrite(cardnum, (LPSTR)pb[cardnum].lookAheadTailPTR, pb[cardnum].playbackHeadPTR32, data);
         else 
            vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)pb[cardnum].lookAheadTailPTR, pb[cardnum].playbackHeadPTR32, data/2);
         pb[cardnum].lookAheadTailPTR+=(data/bytesPerWord);   //advance tailPTR in look ahead
         if(pb[cardnum].lookAheadTailPTR >= pb[cardnum].endOfBuffer)
         {
            pb[cardnum].lookAheadTailPTR = (pb[cardnum].lookAheadTailPTR-pb[cardnum].endOfBuffer)+pb[cardnum].lookAheadBuffer;
         }
         advancePlaybackHeadPTR(data,cardnum);
      }
      else
      {
         //block write of space bytes
         if(board_is_v5_uca[cardnum])
            vbtWrite(cardnum, (LPSTR)pb[cardnum].lookAheadTailPTR, pb[cardnum].playbackHeadPTR32, space);
         else
            vbtWriteRAM[cardnum](cardnum, (BT_U16BIT *)pb[cardnum].lookAheadTailPTR, pb[cardnum].playbackHeadPTR32, space/2);
         pb[cardnum].lookAheadTailPTR+=(space/bytesPerWord);  //advance tailPTR in look ahead
         if(pb[cardnum].lookAheadTailPTR >= pb[cardnum].endOfBuffer)
         {
            pb[cardnum].lookAheadTailPTR = (pb[cardnum].lookAheadTailPTR-pb[cardnum].endOfBuffer)+pb[cardnum].lookAheadBuffer;
         }
         advancePlaybackHeadPTR(space,cardnum);
      }
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - advancePlaybackHeadPTR
*
*  FUNCTION
*     This functioon advance the playback buffer head pointer.
*
*   Parameters
*     BT_U32BIT count -> number of bytes to advance head ponter.
*
*   Returns
*     none
*
****************************************************************************/
static void advancePlaybackHeadPTR(BT_U32BIT count,BT_UINT cardnum)
{
   BT_U16BIT playbackHeadPTR16;

   pb[cardnum].playbackHeadPTR32+=count;

   if(pb[cardnum].playbackHeadPTR32 >= pb[cardnum].endOfPlaybackBuffer32)
   {
      pb[cardnum].playbackHeadPTR32 = pb[cardnum].playbackBuffer32;
   }

   if(board_is_v5_uca[cardnum])
   {
      playbackHeadPTR16 = (BT_U16BIT)(pb[cardnum].playbackHeadPTR32 >> shiftBits);
      vbtSetFileRegister(cardnum, PB_HEAD_POINTER, playbackHeadPTR16);
   }
   else
      vbtSetRegister[cardnum](cardnum, PB_HEAD_V6POINTER, RAM_ADDR(cardnum,pb[cardnum].playbackHeadPTR32));
}

/****************************************************************************
*
*  PROCEDURE NAME - processBC_RT
*
*  FUNCTION
*     This function convert BC_RT message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacket buffer
*
****************************************************************************/
static int processBC_RT(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;

   messagePTR = pb[cardnum].messagePacket;

   messagePTR = processTagTime(messagePTR,cardnum);
   messagePTR = processCommand(messagePTR,1,cardnum);
   messagePTR = processData(messagePTR,cardnum);
   if((pb[cardnum].RTarray[pb[cardnum].BMRecord.command1.rtaddr]) != 0)
   {
      if((pb[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP) == 0)
      {
         messagePTR = processResponseGap(pb[cardnum].BMRecord.response1.time, (pb[cardnum].numberOfDataWords+1)*timePerWord,messagePTR,cardnum);
         if(pb[cardnum].BMRecord.command1.rtaddr != 31)
         {
            messagePTR = processStatus(messagePTR,1,cardnum);
         }
      }

   }

   return (BT_INT)(messagePTR - pb[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processRT_BC
*
*  FUNCTION
*     This function convert RT_BC message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacketg buffer
*
****************************************************************************/
static int processRT_BC(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;

    messagePTR = pb[cardnum].messagePacket;

   messagePTR = processTagTime(messagePTR,cardnum);
   messagePTR = processCommand(messagePTR,1,cardnum);
   if((pb[cardnum].RTarray[pb[cardnum].BMRecord.command1.rtaddr]) != 0)
   {
      if((pb[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP) == 0)
      {
         messagePTR = processResponseGap(pb[cardnum].BMRecord.response1.time,timePerWord,messagePTR,cardnum);
         messagePTR = processStatus(messagePTR,1,cardnum);
         messagePTR = processData(messagePTR,cardnum);
      }

   }

   return (BT_INT)(messagePTR-pb[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processRT_RT
*
*  FUNCTION
*     This function convert RT_RT message into the playback format.
*
*   Parameters
*     none
*
*   Returns
*     int -> number of data words in the messagePacketg buffer
*
****************************************************************************/
static int processRT_RT(BT_UINT cardnum)
{
   BT_U16BIT *messagePTR;
   BT_U32BIT additionalTime;
   BT_UINT rtime;

   additionalTime = 40;
   messagePTR = pb[cardnum].messagePacket;

   messagePTR = processTagTime(messagePTR,cardnum);
   messagePTR = processCommand(messagePTR,1,cardnum); // #1
   messagePTR = processCommand(messagePTR,2,cardnum); // #2

   if(pb[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP)//if no response then we are done
      return (BT_INT)(messagePTR - pb[cardnum].messagePacket);  //

   if((pb[cardnum].RTarray[pb[cardnum].BMRecord.command2.rtaddr]) != 0)
   {
      if(pb[cardnum].BMRecord.response1.time != 0)
      {
         messagePTR = processResponseGap(pb[cardnum].BMRecord.response1.time, additionalTime ,messagePTR,cardnum); // #1
         messagePTR = processStatus(messagePTR,1,cardnum); //#1
         messagePTR = processData(messagePTR,cardnum);
      }
   }
   else
   {
      pb[cardnum].numberOfDataWords = pb[cardnum].BMRecord.command1.wcount;
      if(pb[cardnum].numberOfDataWords == 0)
      {
         pb[cardnum].numberOfDataWords = 32;
      }
   }
   // note that if rt_bcst_enabled[cardnum] == 1;    // RT address 31 is broadcast.
   // otherwise RT address 31 is just another address.

   if(pb[cardnum].BMRecord.int_status & BT1553_INT_NO_RESP)//if no response then we are done
      return (BT_INT)(messagePTR - pb[cardnum].messagePacket);

   if((pb[cardnum].RTarray[pb[cardnum].BMRecord.command1.rtaddr] != 0));// && (pb[cardnum].BMRecord.command1.rtaddr != broadcast))
   {
      if(pb[cardnum].BMRecord.response2.time != 0)
      {
         rtime = ((BT_UINT)pb[cardnum].BMRecord.response1.time)>>1;
         additionalTime = ((commandWords+pb[cardnum].numberOfDataWords)*timePerWord)+rtime;
         messagePTR = processResponseGap(pb[cardnum].BMRecord.response2.time, additionalTime,messagePTR,cardnum); // #2
         messagePTR = processStatus(messagePTR,2,cardnum); //#2
      }
   }
   return (BT_INT)(messagePTR - pb[cardnum].messagePacket);
}

/****************************************************************************
*
*  PROCEDURE NAME - processCommand
*
*  FUNCTION
*     This function formats 1553 command word.
*
*   Parameters
*     BT_U16BIT* messagePtr -> pointer to the next messagePackage buffer location
*     int comIndex          -> 1 = command 1, 2 = command 2;
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processCommand(BT_U16BIT* messagePtr, int comIndex, BT_UINT cardnum)
{   
   union sData
   {
      XMIT_MESSAGE message;
      BT_U16BIT msgData;
   } xmit;

   union comData
   {
      BT1553_COMMAND command;
      BT_U16BIT cData;
   }com;

   if(comIndex == 1)
   {
      xmit.message = buildXmitMessage (pb[cardnum].BMRecord.status_c1, commandCount, pSync,cardnum);
      // write command xmit_message to buffer.
      com.command = pb[cardnum].BMRecord.command1;
   }
   else
   {   
      xmit.message = buildXmitMessage (pb[cardnum].BMRecord.status_c2, commandCount, pSync,cardnum);
      // write command xmit_message to buffer.
      com.command = pb[cardnum].BMRecord.command2;
   }

   *messagePtr = xmit.msgData;
   messagePtr++;
   *messagePtr = com.cData;
   messagePtr++;
   return messagePtr;
}

/****************************************************************************
*
*  PROCEDURE NAME - processResponseGap
*
*  FUNCTION
*     This function calculates the response gap stores the resutls in Playback
*     format in the messagePacket buffer.
*
*   Parameters
*     BT_U16BIT responseTime   -> Message response time
*     BT_U32BIT additionalTime -> additional time for command, status and data words
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processResponseGap(
   BT_U16BIT responseTime,
   BT_U32BIT additionalTime,
   BT_U16BIT* messagePTR,
   BT_UINT cardnum)
{
   BT_U32BIT timeData;
   BT_U16BIT lsbTime;
   BT_U16BIT msbTime;

   *messagePTR = gapMessage;
   messagePTR++;
   
   timeData = pb[cardnum].hwTagTime + responseTime + ((additionalTime-2) << 1); //Calculate the absolute time of response

   pb[cardnum].lastRespTime = timeData;

   lsbTime = (BT_U16BIT)(timeData & 0xffff);
   msbTime = (BT_U16BIT)((timeData & 0xffff0000) >> 16);

   *messagePTR = lsbTime;
   messagePTR++;
   *messagePTR = msbTime;
   messagePTR++;

   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processData
*
*  FUNCTION
*     This function formats the data associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processData(BT_U16BIT* messagePTR,BT_UINT cardnum)
{
   union messageData
   {
      XMIT_MESSAGE msg;
      BT_U16BIT    mData;
   } msgDat;

   BT_U16BIT *tempPTR;
   BT_U16BIT  dataCount = 0;
   int        i;

   BT_U16BIT dataStatus;

   if(pb[cardnum].BMRecord.int_status & BT1553_INT_MODE_CODE)
//   if(pb[cardnum].BMRecord.command1.subaddr == modeCode1 || 
//      pb[cardnum].BMRecord.command1.subaddr == pb[cardnum].modeCode2)
   {
      pb[cardnum].numberOfDataWords = modeCodeData[pb[cardnum].BMRecord.command1.wcount];
   }
   else
   {
      pb[cardnum].numberOfDataWords = pb[cardnum].BMRecord.command1.wcount;
      if( pb[cardnum].numberOfDataWords == 0)
      {
         pb[cardnum].numberOfDataWords = 32;
      }
      if((pb[cardnum].BMRecord.int_status & BT1553_INT_LOW_WORD) != 0)
      {
         pb[cardnum].numberOfDataWords = 1;
      }
   }
   tempPTR = messagePTR;

   if(pb[cardnum].numberOfDataWords != 0)
   {
      dataStatus = (BT_U16BIT)(pb[cardnum].BMRecord.status[0] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY));
      msgDat.msg = buildXmitMessage(pb[cardnum].BMRecord.status[0], 0,nSync,cardnum);

      messagePTR++; //increment past command word until data count is known
      //add data;

      *messagePTR = pb[cardnum].BMRecord.value[0];
      messagePTR++;
      dataCount++;

      for(i=1;i<pb[cardnum].numberOfDataWords;i++)
      {
         if(dataStatus == (pb[cardnum].BMRecord.status[i] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY )))
         { 
            // add data
            *messagePTR = pb[cardnum].BMRecord.value[i];
            messagePTR++;
            dataCount++;
         }
         else
         {
            //write the xmit word to the buffer
            msgDat.msg.wcount = dataCount;
            *tempPTR = msgDat.mData;
            dataCount = 1;
            tempPTR = messagePTR;
            
            dataStatus = (BT_U16BIT)(pb[cardnum].BMRecord.status[i] & (BT1553_INT_INVERTED_SYNC | BT1553_INT_PARITY));
            msgDat.msg = buildXmitMessage((BT_U16BIT)pb[cardnum].BMRecord.status[i],0,nSync,cardnum);

            messagePTR++;
            //add data
            *messagePTR = pb[cardnum].BMRecord.value[i];
            messagePTR++;
         }
      }
   }
   else
   {
      dataCount = 0;
   }
   msgDat.msg.wcount = dataCount;

   *tempPTR = msgDat.mData;
   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processStatus
*
*  FUNCTION
*     This function formats the status associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*     nt statIndex             -> 1 = status 1; 2 = status
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processStatus(BT_U16BIT* messagePTR, int statIndex, BT_UINT cardnum)
{
   union sData
   {
      XMIT_MESSAGE message;
      BT_U16BIT msg;
   } statMSG;

   union statusData
   {
      BT1553_STATUS stat;
      BT_U16BIT sdat;
   }statDat;

   if(statIndex == 1)
   {
      statDat.stat = pb[cardnum].BMRecord.status1;
      statMSG.message = buildXmitMessage(pb[cardnum].BMRecord.status_s1,statusCount,pSync,cardnum);
   }
   else
   {
      statDat.stat = pb[cardnum].BMRecord.status2;
      statMSG.message = buildXmitMessage(pb[cardnum].BMRecord.status_s2,statusCount,pSync,cardnum); // changed status_s1 to status_s2
   }

   *messagePTR = statMSG.msg;
   messagePTR++;
   *messagePTR = statDat.sdat;
   messagePTR++;
   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - processTagTime
*
*  FUNCTION
*     This function formats the tag time associated with a 1553 message
*
*   Parameters
*     BT_U16BIT* messagePTR    -> pointer to the next messagePackage buffer location
*
*   Returns
*     BT_U16BIT* -> pointer to the next messagePackage buffer location
*
****************************************************************************/
static BT_U16BIT* processTagTime(BT_U16BIT* messagePTR, BT_UINT cardnum)
{
   BT1553_TIMEV5 tagTime;

   BT_U16BIT lsbTime;
   BT_U16BIT msbTime;
   BT_U16BIT TTmessage = 0x2800;

   union bigtime
   {
      CEI_UINT64 time64;
      BT1553_TIMEV5 ttime;
   }bt, tt; 

   bt.time64 = 0;                   // initialize structure to clear upper bits
   tt.time64 = 0;                   // initialize to 0
   bt.ttime = pb[cardnum].BMRecord.time;
   tagTime  = bt.ttime;
   tt.ttime = bt.ttime;             // Perform timetag checks with the entire timetag, not just the lower 32-bits 


#ifdef _TIME_CYCLE
   if(pb[cardnum].lastTagTime > tt.time64)      // Has time gone backwards?
   {
      pb[cardnum].timeOffset+=1000000;          // yes, add 1 second in uSec to avoid lockups
   }
#endif //_TIME_CYCLE

   // Use the entire timetag to calculate the new hw tag time
   pb[cardnum].lastTagTime = tt.time64; 
   pb[cardnum].hwTagTime = (BT_U32BIT)(((tt.time64+startLatency+pb[cardnum].timeOffset)-pb[cardnum].startTimeDelay) << 1);  // fix startTimeDelay.
   *messagePTR = TTmessage;
   messagePTR++;

   lsbTime = (BT_U16BIT) (pb[cardnum].hwTagTime & 0x0000ffff);
   msbTime = (BT_U16BIT)((pb[cardnum].hwTagTime & 0xffff0000) >> 16);

   *messagePTR = lsbTime;
   messagePTR++;
   *messagePTR = msbTime;
   messagePTR++;

   return messagePTR;
}

/****************************************************************************
*
*  PROCEDURE NAME - buildXmitMessage
*
*  FUNCTION
*     This function builds the playback transmit message
*
*   Parameters
*     BT_U16BIT status    -> Status word used to evaluate message status
*     int count           -> Data word count
*     int message_sync    -> message sync 0 = positive sync; 1 = negative sync
*
*   Returns
*     XMIT_MESSAGE -> 16 bit transmit message structure
*
****************************************************************************/
static XMIT_MESSAGE buildXmitMessage(BT_U16BIT status, int count, int message_sync, BT_UINT cardnum)
{
   // Build xmit message code

   XMIT_MESSAGE message;
   message.xmit_code = 1;   // set transmit code
   message.gap_code = 0;   //set gap code to 0;
   message.notUsed = 0;   // set not used bits to 0;
   message.bitCount = 0;
   message.eop = 0;

   if((pb[cardnum].BMRecord.int_status & BT1553_INT_BUSB) == 0)
   {
      message.channel = 0;
   }
   else
   {
      message.channel = 1;
   }

   if((status & BT1553_INT_PARITY) == 0)
   {
      message.parityError = 0;
   }
   else
   {
      message.parityError = 1;
   }

   message.bitCount = 0; // Not used for now
   // set sync polarity

   if((status & BT1553_INT_INVERTED_SYNC) == 0)
   {
      message.syncPolarity = (BT_U16BIT)message_sync;
   }
   else
   {
      message.syncPolarity = (BT_U16BIT)(message_sync^1); //
   }
   message.wcount = (BT_U16BIT)count;

   return message;
}

/****************************************************************************
*
*  PROCEDURE NAME - filterRecord
*
*  FUNCTION
*     This function filters the *.bmd record based on the filtering criteria
*
*   Parameters
*     none
*
*   Returns
*     int -> Include Record = 1; Exclude Record = 0;
*
****************************************************************************/
static int filterRecord(BT_UINT cardnum)
{
   switch(pb[cardnum].filterFlag)   // 0 no filter, 1 = time filter, 2 = message filter
   {
      case 0:
         return includeRecord;
      case 1:
         if((pb[cardnum].BMRecord.time.topuseconds >= pb[cardnum].startTagTime.topuseconds) && 
          (pb[cardnum].BMRecord.time.topuseconds <= pb[cardnum].stopTagTime.topuseconds))
         {
            if((pb[cardnum].BMRecord.time.microseconds >= pb[cardnum].startTagTime.microseconds) && 
            (pb[cardnum].BMRecord.time.microseconds <= pb[cardnum].stopTagTime.microseconds))
            {
               return includeRecord;
            }
         }
         return excludeRecord;
      case 2:
         if((pb[cardnum].BMRecord.messno >= pb[cardnum].messageNumberStartPoint) && 
          (pb[cardnum].BMRecord.messno <= pb[cardnum].messageNumberStopPoint))
         {
            return includeRecord;
         }
         if(pb[cardnum].BMRecord.messno > pb[cardnum].messageNumberStopPoint)
         {
            pb[cardnum].endOfFilterData = TRUE;
         }
         return excludeRecord;
      default:
         return includeRecord;
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - periodicUpdate
*
*  FUNCTION
*     this function is invoked by the periodic update.  It checks the playback
*     status by reading the playback control register and checking for errors 
*     or end of playback
*
*   Parameters
*     unsigned cardnum        -> card number 
*     API_INT_FIFO *intFifo   -> Structure of data passed from timer interrupt 
*
*   Returns
*     BT_INT -> API_SUCCESS
*
****************************************************************************/

static BT_INT _stdcall periodicUpdate(BT_UINT cardnum, API_INT_FIFO *intFifo)
{
   //read the register and see if we need to send more data over
   BT_U32BIT temp;
   BT_U32BIT controlData;

   if(board_is_v5_uca[cardnum])
      temp = vbtGetFileRegister(cardnum,PB_TAIL_POINTER);
   else
      temp = vbtGetRegister[cardnum](cardnum,PB_TAIL_V6POINTER);  // This is a status monitor. Is tail pointer advancing?

   pb[cardnum].playbackStatus->tailPointer = temp;

   if(board_is_v5_uca[cardnum])
      controlData = vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);
   else
      controlData = vbtGetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL);

   pb[cardnum].playbackStatus->playbackStatus = (BT_U16BIT)controlData;

   if((controlData & errorBit) != 0)
   {
      if(board_is_v5_uca[cardnum])
         vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);  // clear error with write data does not matter
      else
         vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);  // clear error with write data does not matter
   }

   if((controlData & bufferEmpty) != 0)
   {
      pb[cardnum].playbackStatus->playbackError = API_PLAYBACK_BUF_EMPTY;
   }

   if((controlData & runBit) == 0)
   {
      if(!pb[cardnum].moreDataInFile)
      {
         if(board_is_v5_uca[cardnum])
            vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
         else
            vbtSetRegister[cardnum](cardnum, HWREG_PB_V6CONTROL, 0x18 + PB_CLEAR);

         pb[cardnum].playbackStatus->playbackError = API_SUCCESS;
#ifdef _UNIX_
//         pthread_mutex_lock(&pb[cardnum].hExitMutex);
	     pthread_cond_broadcast(&pb[cardnum].hExitEvent);
//         pthread_mutex_unlock(&pb[cardnum].hExitMutex);
#else
         SetEvent(pb[cardnum].hExitEvent);
#endif
         return API_SUCCESS;
      }
      else
      {
         pb[cardnum].playbackStatus->playbackError = API_PLAYBACK_BAD_EXIT;
#ifdef _UNIX_
//         pthread_mutex_lock(&pb[cardnum].hExitMutex);
	     pthread_cond_broadcast(&pb[cardnum].hExitEvent);
//         pthread_mutex_unlock(&pb[cardnum].hExitMutex);
#else
         SetEvent(pb[cardnum].hExitEvent);
#endif

         return API_SUCCESS;
      }

   }
   if( lookAheadBufferDataAvailable(1,cardnum) > 0 )
   {
      writeReformattedDataToBuffer(cardnum);
      if(pb[cardnum].moreDataInFile)
      {
         fillLookAheadBuffer(cardnum);
      }
   }
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - playbackBufferFreeSpace
*
*  FUNCTION
*     this function is calculates the free space in the playback buffer.  Free
*     space is determined as the space between the end of the buffer and the 
*     head pointer if teh ehad pointer is greater than the tail pointer, or the
*     tail pointer minus the head pointer if the tail pointer is greater than
*     the head pointer.  This allow the maximum amount of data to be written
*     without having to worry about wrapping around the the end of the buffer.
*
*   Parameters
*     none 
*
*   Returns
*     BT_U32BIT -> amount of free space (in bytes) available
*
****************************************************************************/
static BT_U32BIT playbackBufferFreeSpace(BT_UINT cardnum)
{
   BT_U32BIT pbTail;
   BT_U32BIT pbHead32;
   BT_U32BIT pbTail32;

   if(board_is_v5_uca[cardnum])
   {
      pbTail   = vbtGetFileRegister(cardnum, PB_TAIL_POINTER);
      pbTail32 = pbTail << shiftBits;
   }
   else
   { 
      pbTail32  = vbtGetRegister[cardnum](cardnum, PB_TAIL_V6POINTER);
      pbTail32  = REL_ADDR(cardnum,pbTail32);
   }

   pbHead32 = pb[cardnum].playbackHeadPTR32 + bytesPerWord;
   if(pbHead32 == pb[cardnum].endOfPlaybackBuffer32)
   {
      pbHead32 = pb[cardnum].playbackBuffer32;
   }

   if(pbTail32 == pbHead32)  // full
   {
      return 0;
   }

   if(pb[cardnum].playbackHeadPTR32 > pbTail32)
   {
      if(pbTail32 == pb[cardnum].playbackBuffer32)
      {
         return (pb[cardnum].endOfPlaybackBuffer32-pb[cardnum].playbackHeadPTR32)-2;
      }
      else
      {
         return (pb[cardnum].endOfPlaybackBuffer32-pb[cardnum].playbackHeadPTR32);
      }
   }
   else
   {
      if(pbTail32 == pb[cardnum].playbackHeadPTR32)
      {
         return pb[cardnum].sizeOfPlaybackBuffer-2;
      }
      else
      {
         return (pbTail32 - pb[cardnum].playbackHeadPTR32)-2;
      }
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - lookAheadBufferDataAvailable
*
*  FUNCTION
*     this function is calculates the data available in the look ahead buffer.
*     Data available is determined as the data between the end of the buffer and the
*     tail pointer if teh tail pointer is greater than the head pointer, or the
*     head pointer minus the tail pointer if the head pointer is greater than
*     the tail pointer.  This datermine the maximum amount of data available
*     without having to worry about wrapping around the the end of the buffer.
*
*   Parameters
*     none
*
*   Returns
*     BT_U32BIT -> amount of free space (in bytes) available
*
****************************************************************************/
static BT_U32BIT lookAheadBufferDataAvailable(BT_INT total, BT_UINT cardnum)
{
   if(pb[cardnum].lookAheadHeadPTR == pb[cardnum].lookAheadTailPTR)  // empty
   {
      return 0;
   }

   if(pb[cardnum].lookAheadHeadPTR <= pb[cardnum].lookAheadTailPTR)
   {
      if(total==1)
      {
         return (BT_U32BIT)((pb[cardnum].endOfBuffer-pb[cardnum].lookAheadTailPTR)+
          (pb[cardnum].lookAheadHeadPTR-pb[cardnum].lookAheadBuffer))*bytesPerWord;
      }
      else
      {
         return (BT_U32BIT)(pb[cardnum].endOfBuffer-pb[cardnum].lookAheadTailPTR)*bytesPerWord;
      }
   }
   else
   {
      return (BT_U32BIT)((((pb[cardnum].lookAheadHeadPTR-pb[cardnum].lookAheadTailPTR))*bytesPerWord));
   }
}

/****************************************************************************
*
*  PROCEDURE NAME - readLogFileBlock
*
*  FUNCTION
*     This this function read a 64k Byte block of data from the specified .bmd 
*     file.  This function for now operates as a separate thread.
*
*   Parameters
*     LPVOID Param -> cardnum
*
*   Returns
*     BT_U32BIT -> API_SUCCESS
*
****************************************************************************/
#ifdef _UNIX_
static void *  readLogFileBlock(void * Param)
#else
static BT_U32BIT _stdcall readLogFileBlock(LPVOID Param)
#endif
{
   size_t  count;            // Number of BM messages we read
   BT_UINT cardnum;          // Current BusTools-API cardnum
   DWORD   status;           // Tells if we woke up just to terminate
#ifdef _UNIX_
   DWORD   statusExit = 99;  // Tells if we woke up just to terminate
#else 
   HANDLE  EventHandles[2];  // Events we wait for
#endif

   cardnum = (BT_UINT)((CEI_UINT64)Param); /* extraneous CEI_UINT64 cast for Wp64 warning */

#ifdef __WIN32__
   EventHandles[0] = pb[cardnum].blockReadEvent;  // Wait for either read or
   EventHandles[1] = pb[cardnum].hExitEvent;      //   exit.
#endif
   // reads 512 API_BM_MBUFV5 records from the file per read
   while(pb[cardnum].diskFileNotEmpty)
   {
#ifdef _UNIX_

      if((status = pthread_cond_wait(&pb[cardnum].blockReadEvent,&pb[cardnum].blockReadMutex))==0
        || (statusExit = pthread_cond_wait(&pb[cardnum].hExitEvent,&pb[cardnum].hExitMutex))==0 )
      {
         pthread_mutex_unlock(&pb[cardnum].blockReadMutex);
//         pthread_mutex_unlock(&pb[cardnum].hExitMutex);
//         usleep(200);
      }
      else
        break;
      if(statusExit == 0)
      {
         printf("readLogFileBlock: statusExit = 0\n");
         break;
      }
     
      pthread_mutex_lock(&pb[cardnum].blockReadMutex);

#else
      status = WaitForMultipleObjects(2, EventHandles, FALSE, INFINITE);
      if ( status != WAIT_OBJECT_0 )
         break;   // Exit and distroy this thread if the exit event gets signaled.
#endif
      count = fread(pb[cardnum].diskReadPTR,sizeof(API_BM_MBUFV5),readSize,pb[cardnum].filePTR);
      if(count == readSize)
      {
         pb[cardnum].diskReadPTR+=readSize;
         if(pb[cardnum].diskReadPTR == pb[cardnum].endOfDiskBuffer)
         {
            pb[cardnum].diskReadPTR  = pb[cardnum].diskBuffer;
         }
      }
      else
      {
         if(feof(pb[cardnum].filePTR) != 0)
         {
            pb[cardnum].diskReadPTR+=count;
            *pb[cardnum].diskReadPTR = pb[cardnum].endRecord;
            pb[cardnum].diskReadPTR++;
            pb[cardnum].diskFileNotEmpty = FALSE;
         }
         if( ferror(pb[cardnum].filePTR) !=0 )
         {
            pb[cardnum].playbackStatus->playbackError = API_PLAYBACK_DISK_READ;
            *pb[cardnum].diskReadPTR = pb[cardnum].endRecord;
            clearerr(pb[cardnum].filePTR);
         }
      }
#ifdef _UNIX_
        pthread_mutex_unlock(&pb[cardnum].blockReadMutex);
//        MSDELAY(10);
      pthread_mutex_lock(&pb[cardnum].readCompleteMutex);
//      pthread_cond_broadcast(&pb[cardnum].readCompleteEvent);
//      pthread_mutex_unlock(&pb[cardnum].readCompleteMutex);
#else
      SetEvent(pb[cardnum].readCompleteEvent);
#endif
   }
#ifdef _UNIX_
   pthread_exit((void *)0);
#else
   status = 0;
   CEI_THREAD_EXIT(&status, NULL);
#endif
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - getNextRecord
*
*  FUNCTION
*     This this function get the next .bmd record in the disk buffer.  When 512
*     records are read this function issues a blockReadEvent to signal that
*     there is space to read the next 64k Byte block of data from the file.
*
*   Parameters
*     none
*
*   Returns
*     API_BM_MBUFV5 -> structure of Bus Monitor data.
*
****************************************************************************/
static API_BM_MBUFV5 getNextRecord(BT_UINT cardnum)
{
   API_BM_MBUFV5 BMRecord;
#ifdef _UNIX_
   BT_INT status;
#endif
   BMRecord = *pb[cardnum].diskBufferPTR;

   pb[cardnum].bufferCount++;
   if( pb[cardnum].bufferCount == 512 && pb[cardnum].diskFileNotEmpty)
   {
#ifdef _UNIX_

      pthread_mutex_lock(&pb[cardnum].blockReadMutex); 
      status = pthread_cond_signal(&pb[cardnum].blockReadEvent);
      pthread_mutex_unlock(&pb[cardnum].blockReadMutex); 
      pb[cardnum].bufferCount = 0;
      //MSDELAY(10);
//	  pthread_mutex_lock(&pb[cardnum].readCompleteMutex);
//	  status = pthread_cond_wait(&pb[cardnum].readCompleteEvent,&pb[cardnum].readCompleteMutex);
//	  pthread_mutex_unlock(&pb[cardnum].readCompleteMutex);
//          MSDELAY(50);
#else
      SetEvent(pb[cardnum].blockReadEvent);
      pb[cardnum].bufferCount = 0;
      WaitForSingleObject(pb[cardnum].readCompleteEvent, INFINITE);
#endif
   }

   pb[cardnum].diskBufferPTR++;
   if(pb[cardnum].diskBufferPTR == pb[cardnum].endOfDiskBuffer)
   {
      pb[cardnum].diskBufferPTR = pb[cardnum].diskBuffer;
   }
   return BMRecord;
}

/****************************************************************************
*
*  PROCEDURE NAME - fillLookAheadBuffer
*
*  FUNCTION
*     This this function fills the look ahead buffer with data from data stored
*     in the disk buffer.
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
static void fillLookAheadBuffer(BT_UINT cardnum)
{
   BT_U16BIT *message;
   int     messageIndex;
   BT_UINT msgSize = 164;
   int     bufferSize;

   BT_INT total = 1;
   while((sizeOfLookAheadBuffer - lookAheadBufferDataAvailable(total,cardnum)) >= msgSize)
   {
      pb[cardnum].BMRecord = getNextRecord(cardnum);  // need to look at start stop info to see where to start/stop read
      if((pb[cardnum].BMRecord.messno != 0) && (!pb[cardnum].endOfFilterData))
      {
         if(filterRecord(cardnum))
         {
            pb[cardnum].playbackStatus->recordsProcessed++;
            message = pb[cardnum].messagePacket;      // point to start of next message
            bufferSize = processTheRecord(cardnum);   // This includes parsing the BM struct, calculating gap, determining errors, etc.
            for(messageIndex=0;messageIndex<bufferSize;messageIndex++) // This is the initial file of the circ buffer
            {
               *pb[cardnum].lookAheadHeadPTR = *message;
               pb[cardnum].lookAheadHeadPTR++;
               message++;

               if(pb[cardnum].lookAheadHeadPTR == pb[cardnum].endOfBuffer)
               {
                  pb[cardnum].lookAheadHeadPTR = pb[cardnum].lookAheadBuffer;
               }
            }
         }
      }
      else
      {
         // need to write an end of of playback record here.
         *pb[cardnum].lookAheadHeadPTR = pb[cardnum].eopMessage;
         pb[cardnum].lookAheadHeadPTR++;
         pb[cardnum].moreDataInFile = FALSE;
         break;
      }
   }
}
#endif //_PLAYBACK_

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_Stop
*
*  FUNCTION
*     This routine halts playback
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV Playback_Stopv5(
   BT_UINT cardnum)         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   BT_U16BIT controlData;

   //MessageBox(NULL, "enter BusTools_Playback_Stop", "BusTools_Playback_Stop", MB_OK | MB_SYSTEMMODAL);
   // Cause the Playback threads to exit.
#ifdef _UNIX_
//   pthread_mutex_lock(&pb[cardnum].hExitMutex);
   pthread_cond_broadcast(&pb[cardnum].hExitEvent);
//   pthread_mutex_unlock(&pb[cardnum].hExitMutex);
#else
   SetEvent(pb[cardnum].hExitEvent);
#endif

   controlData = (BT_U16BIT)vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);
   if((controlData & runBit) == 0)  // reset for normal shutdown, clear errors
   {
      vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
   }
   else  // reset for forced shutdown, clear errors
   {
      vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x20);
      vbtSetHWRegister(cardnum, HWREG_PB_CLEAR, 0);
   }
   MSDELAY(20); //

   controlData = (BT_U16BIT)vbtGetHWRegister(cardnum, HWREG_PB_CONTROL);

   if((controlData & runBit) == 0)
      return API_SUCCESS;

   vbtSetHWRegister(cardnum, HWREG_PB_CONTROL, 0x40); // Stop Playback did not work so halt
   MSDELAY(20);

   if((controlData & runBit) == 0)
      return API_SUCCESS;
   else
     return 666;   
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_Checkv5
*
*  FUNCTION
*     This routine checks playback for time gaps and time sequence problems
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_Playback_Checkv5(char * bmdfile, BT_UINT mins )         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   FILE * fptr;
   int scount=0,gcount=0;
   CEI_UINT64 big_time;
   CEI_UINT64 last_big_time;
   API_BM_MBUFV5 bm_mbuf;
   CEI_UINT64 LARGE_GAP = (1000000 * 60 * mins); //   
   int last_big_time_valid = FALSE;

   last_big_time = 0x0;  // 

   if((fptr=fopen(bmdfile,"rb"))==NULL)
   {
      return API_PLAYBACK_BAD_FILE; // error opening the bmd file
   }
    //while(!feof(fptr))
   while (1)
   { 
   //fread(&bm_mbuf,sizeof(API_BM_MBUFV5),1,fptr);
      if (fread(&bm_mbuf,sizeof(API_BM_MBUFV5),1,fptr) != 1)
      {
         if(feof(fptr))
         {
            break;
         }
         else
         {
            fclose(fptr);
            return API_PLAYBACK_FILE_ERR;
         }
      }
      big_time = (((CEI_UINT64)(bm_mbuf.time.topuseconds))<<32) | (bm_mbuf.time.microseconds);
      if (last_big_time_valid)
      {
          if(big_time <= last_big_time)
             scount++;
          else if(last_big_time != 0)
          {
             if((big_time - last_big_time) > LARGE_GAP)
                gcount++;
          }
      }
      last_big_time = big_time;
      last_big_time_valid = TRUE;
   }

   fclose(fptr);

   if(scount)
      return API_PLAYBACK_TIME_ORDER;
   if(gcount)
      return API_PLAYBACK_TIME_GAP;
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

NOMANGLE BT_INT CCONV BusTools_Playback(BT_UINT cardnum,API_PLAYBACK playbackData)
{
#if defined(_PLAYBACK_)
   char *fstr;
   BT_INT board_is_bmdx=0, board_is_bmd=0, cmpVal;
   BT_INT status,indx;
   char tempname[PB_LOGFILE_LEN];
   char fileApp[PB_LOGFILE_LEN];
   char c;

   strcpy(tempname, playbackData.fileName);

   fstr = strtok(tempname,".");
   while (fstr != NULL)
   {
      strcpy(fileApp,fstr);
      fstr = strtok(NULL,".");
   }

   indx=0;
   while (fileApp[indx])
   {
     c=fileApp[indx];
     fileApp[indx] = tolower(c);
     indx++;
   }

   board_is_bmdx = strcmp(fileApp,"bmdx");
   board_is_bmd  = strcmp(fileApp,"bmd");
   if(board_is_bmdx == 0)
      cmpVal=0;
   else if (board_is_bmd==0)
      cmpVal = 1;
   else
      return API_PLAYBACK_FILE_ERR;
      
   if(cmpVal==0)
      status=initPlaybackv6(cardnum,playbackData);
   else
      status=initPlayback(cardnum,playbackData);

   if(status == API_SUCCESS )
   {
      if(cmpVal==0)
      {
         pbv6[cardnum].filePTR = fopen(playbackData.fileName,"rb");
         if(pbv6[cardnum].filePTR == NULL)
         {
            pbv6[cardnum].playbackStatus->playbackError = API_PLAYBACK_BAD_FILE;
            return API_PLAYBACK_BAD_FILE;
         }
      }
      else
      {
         pb[cardnum].filePTR = fopen(playbackData.fileName,"rb");
         if(pb[cardnum].filePTR == NULL)
         {
            pb[cardnum].playbackStatus->playbackError = API_PLAYBACK_BAD_FILE;
            return API_PLAYBACK_BAD_FILE;
         }
      }

      if(cmpVal==0) //This is a BMDX file so read header record.
      { 
         fread(&bmdHeader,sizeof(BMDX_HEADER),1,pbv6[cardnum].filePTR);  
         if(bmdHeader.uFormatInfo & 0x1)
            nsTimeTag = 1;  
         else
            nsTimeTag = 0;
      } 

      if(cmpVal == 0)
         return BusTools_PlaybackV6(cardnum,playbackData);
      else
         return BusTools_PlaybackV5(cardnum,playbackData);

   }
   else
      return API_PLAYBACK_INIT_ERROR;
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_Stop
*
*  FUNCTION
*     This routine halts playback
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_Playback_Stop(
   BT_UINT cardnum)         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   if(board_is_v5_uca[cardnum])
      return Playback_Stopv5(cardnum);
   else
      return Playback_Stopv6(cardnum);
   playback_is_running[cardnum] = 0;
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
}    
   

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_Playback_Check
*
*  FUNCTION
*     This routine checks playback for time gaps and time sequence problems
*
*   Parameters
*     none
*
*   Returns
*     none
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_Playback_Check(char * bmdfile, BT_UINT mins )         // (i) card number (0 - based)
{
#if defined(_PLAYBACK_)
   BT_INT cmpVal=0;
   BT_INT board_is_bmdx=0, board_is_bmd=0;
   BT_INT indx;

   char *fstr;
   char tempname[PB_LOGFILE_LEN];
   char fileApp[PB_LOGFILE_LEN];
   char c;
     
   
   strcpy(tempname, bmdfile);

   fstr = strtok(tempname,".");
   if (fstr == NULL)
       return API_PLAYBACK_BAD_FILE;

   while (fstr != NULL)
   {
      strcpy(fileApp,fstr);
      fstr = strtok(NULL,".");
   }

   indx=0;
   while (fileApp[indx])
   {
     c=fileApp[indx];
     fileApp[indx] = tolower(c);
     indx++;
   }

   board_is_bmdx = strcmp(fileApp,"bmdx");
   board_is_bmd  = strcmp(fileApp,"bmd");

   if(board_is_bmdx == 0)
      cmpVal=0;
   else if (board_is_bmd==0)
      cmpVal = 1;
   else
      return API_PLAYBACK_FILE_ERR;
   
    
   if(cmpVal)
      return  BusTools_Playback_Checkv5(bmdfile,mins );
   else
      return  BusTools_Playback_Checkv6(bmdfile,mins );
#else
   return API_NO_BUILD_SUPPORT;
#endif //_PLAYBACK_
} 


  
