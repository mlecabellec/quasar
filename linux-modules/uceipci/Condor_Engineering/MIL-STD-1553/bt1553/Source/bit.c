/*============================================================================*
 * FILE:                            B I T . C
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
 *             This file contains the API routines which perform either an
 *             internal or an external wrap test on a single BusTools board,
 *             or between two BusTools boards.
 *
 * USER ENTRY POINTS:
 *    BusTools_BIT_StructureAlignmentCheck - check alignment
 *    BusTools_BIT_InternalBit  - Performs an internal BC/BM/RT BIT test.
 *    BusTools_BIT_TwoBoardWrap - Performs an external wrap test between two boards.
 *    BusTools_BIT_CableWrap    - Performs an external wrap test between a channels
 *                                A and B bus.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  8.28 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  12/05/2000   Initial release version.V4.26.ajh
  08/18/2005   Change the message values to test all bit
  11/05/2014   Fix size of BC_CTRLDATA structure
  12/04/2019   Fixed BusTools_BIT_TwoBoardWrap to execute two passes. bch
 */

#include <stdio.h>
#include <string.h>

#include "busapi.h"
#include "apiint.h"
#include "globals.h"

#define FRAME_RATE     10000L      /* BC minor frame period in micro seconds */
#define RT_ADDR        14           /* This is the RT we will simulate        */
#define RT_SADDR       20          /* This is the subaddress we will use     */
#define RT_MBUF_COUNT  2           /* Number of data buffers per RT          */

#define BM_BUFFERS       8

// Define the messages that we will be sending
static BT_U16BIT pMessage0[] = { 0x0, 0x55, 0xaa, 0xff, 0xf0, 0x0f, 0xa5, 0x5a, 9,10,11,12,13,14,15,16,
                                17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
static BT_U16BIT pMessage1[] = {32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
                                16,15,14,13,12,11,10, 9, 0x5a, 0xa5, 0x0f, 0xf0, 0xff, 0xaa, 0x55, 0x0};

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BIT_InternalBit
*
* FUNCTION
*     This function performs a simple internal wrap test of a single board or
*     a single channel of a dual-channel board.
*     This test is performed in Internal Wrap mode, so the external connections
*     to the 1553 primary and secondary busses are not relevant.
*
*  First the External Bus is disabled.
*   Then setup the BM, the BC and the RT.  The BC sends two transmit messages
*   to the RT; one on the primary bus and the second on the secondary bus.
*   It compares the data received by the BC to the data setup in the RT
*   transmit buffers, and if they differ then the test fails.
*
*  When this function returns the board is still open, but the previous board
*  setup has been lost.  It is necessary that all of the setup functions be
*  called again, beginning with BusTools_BM_Init.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BIT_FAIL_PRI        -> Failure detected on primary bus
*     API_BIT_FAIL_SEC        -> Failure detected on secondary bus
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_InternalBit(
   BT_UINT cardnum,        // (i) Card number of open board to test
   BT_INT  NumMessages)    // (i) Number of messages to test
{
   API_BC_MBUF        bcmessage;      // BC message being written to board
   BT_U16BIT          uBCMessage[32]; // BC data buffer read from board
   BT_U32BIT          dwIntenable;    // BC interrupt enables
   BT_U32BIT          TimeOutBC;      // Counter that times out the BC
   BT_UINT            flag,mcnt;           // Flag indicates when BC has stopped

   API_RT_ABUF        RTAbuf;         // RT Address Buffer
   API_RT_CBUF        RTCbuf;         // RT Control Buffer
   API_RT_MBUF_WRITE  RTmbuf;         // RT Message Buffer

   API_BM_MBUF        bmmessage[10];  // BM message being written to board
   BT_INT             status;         // Status from BusTools/1553-API functions
   BT_UINT            count;          // Actual number of BM messages allocated
   int                i;              // Loop counter

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   BusTools_API_Reset(cardnum,0); //reset to clear out previous setup.


   /*******************************************************************
   *  Perform the internal wrap test the primary and secondary busses.
   *******************************************************************/
   // Disable the external bus on the board
   BusTools_SetInternalBus(cardnum, INTERNAL_BUS);    // Enable internal bus only
   // Make sure that the BC, BM and RT are shutdown
   status = BusTools_BC_StartStop(cardnum, BC_STOP);
   status = BusTools_BM_StartStop(cardnum, BM_STOP);
   status = BusTools_RT_StartStop(cardnum, RT_STOP);

   // Initialize the BM but do not start it, effectively resetting the board.
   // Enable both the primary and secondary busses.
   status = BusTools_BM_Init(cardnum, BM_ENABLE_BUS, BM_ENABLE_BUS);
   if ( status )
      return status;

   // Allocate an odd number of Bus Monitor message buffers
   dwIntenable = BT1553_INT_END_OF_MESS;

   status = BusTools_BM_MessageAlloc(cardnum, 3,&count,dwIntenable);
   if ( status )
      return status;

   // Initialize the bus controller with one data buffer per message.
   dwIntenable = BT1553_INT_END_OF_MESS;     // Enable end-of-message
   status = BusTools_BC_Init(cardnum,0,dwIntenable,0,20,20,FRAME_RATE,1);
   if ( status )
      return status;

   // Allocate three BC messages
   status = BusTools_BC_MessageAlloc(cardnum, 10);
   if ( status )
      return status;

   // Create and write the first message to the board.
   // This is a RT_ADDR, subaddress = RT_SADDR, transmit, 32 word message, chan A.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 1;                   // Next message to execute
   bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA; // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_MFRAME_BEG;
   bcmessage.mess_command1.rtaddr   = RT_ADDR;
   bcmessage.mess_command1.subaddr  = RT_SADDR;
   bcmessage.mess_command1.wcount   = 0;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer(no errors)
   bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
   status = BusTools_BC_MessageWrite(cardnum, 0, &bcmessage);
   if ( status )
      return status;

   // Create and write the second message to the board.
   // This is a RT_ADDR, subaddress = RT_SADDR, transmit, 32 word message, chan B.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 2;                   // Next message to execute
   bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELB; // Transmit on Channel B
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.mess_command1.rtaddr   = RT_ADDR;
   bcmessage.mess_command1.subaddr  = RT_SADDR;
   bcmessage.mess_command1.wcount   = 0;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer(no errors)
   bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
   status = BusTools_BC_MessageWrite(cardnum, 1, &bcmessage);
   if ( status )
      return status;

   // Create and write the third(STOP) message to the board.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 0;
   bcmessage.messno_branch = 0;
   bcmessage.control = BC_CONTROL_LAST;         // End of list (1 shot mode)
   status = BusTools_BC_MessageWrite(cardnum, 2, &bcmessage);
   if ( status != API_SUCCESS )
      return status;

   // Initialize the RT
   status = BusTools_RT_Init(cardnum,0);
   if ( status )
      return status;

   // Enable RT for RT RT_ADDR
   RTAbuf.enable_a = 1;           // Enable channel A
   RTAbuf.enable_b = 1;           // Enable channel B
   RTAbuf.inhibit_term_flag = 0;  // Terminal flag
   RTAbuf.status   = 0;           // status word flags
   RTAbuf.bit_word = 0;           // BIT word

   status = BusTools_RT_AbufWrite(cardnum, RT_ADDR, &RTAbuf);
   if ( status )
     return status;

   // Create two message buffers for RT_ADDR, subaddress RT_SADDR, transmit
   RTCbuf.legal_wordcount = 0xFFFFFFFF;      // Enable all word counts
   BusTools_RT_CbufWrite(cardnum, RT_ADDR, RT_SADDR, 1, RT_MBUF_COUNT, &RTCbuf);

   // Write unique data to RT_ADDR, subaddress RT_SADDR, Transmit, buffer 0
   RTmbuf.enable = BT1553_INT_END_OF_MESS;   // Enable interrupt conditions
   RTmbuf.error_inj_id = 0;                  // Use the default error injection buffer
   memcpy(RTmbuf.mess_data, pMessage0, sizeof(pMessage0));
   status = BusTools_RT_MessageWrite(cardnum, RT_ADDR, RT_SADDR, 1, 0, &RTmbuf);
   if ( status )
      return status;

   // Write unique data to RT_ADDR, subaddress RT_SADDR, Transmit, buffer 1
   memcpy(RTmbuf.mess_data, pMessage1, sizeof(pMessage1));
   status = BusTools_RT_MessageWrite(cardnum, RT_ADDR, RT_SADDR, 1, 1, &RTmbuf);
   if ( status != API_SUCCESS )
      return status;

   // Loop through the following code the specified number of times
   for ( i = 0; i < NumMessages; i++ )
   {
      // Start the Bus Controller
    
      status = BusTools_BM_StartStop(cardnum, BM_START_BIT);
      if(status)
         break;
      status = BusTools_RT_StartStop(cardnum, RT_START_BIT);
      if(status)
         break;
      status = BusTools_BC_StartStop(cardnum, BC_START_BIT);
      if(status)
         break;

      /* Wait for the two BC messages to be transacted. */
      TimeOutBC = 20;    /* set a 40ms time-out */
      do
      {
         MSDELAY(2);
         BusTools_BC_IsRunning(cardnum, &flag);		
      }
      while ( flag && --TimeOutBC );

      if(TimeOutBC==0)
          return API_BC_HALTERROR;

      BusTools_BC_StartStop(cardnum, BC_STOP);
      BusTools_BM_StartStop(cardnum, BM_STOP);
      BusTools_RT_StartStop(cardnum, RT_STOP);

#ifdef INCLUDE_USB_SUPPORT
	  if(CurrentCardType[cardnum] == R15USB)
      {
         BT_INT retval;
         retval = usb_get_int_data(cardnum);
         if(retval)
            return retval;
       
         retval = usb_get_bm_data(cardnum);  // the new messages since last update
         if(retval)
            return retval;
      }
#endif
      // Read the data from the first BC message
      status = BusTools_BC_MessageReadData(cardnum, 0, uBCMessage);
      if ( status )
         break;

      // Compare the data that the BC received from the RT to the data
      // that we loaded into the RT transmit buffers.
      // Compare BC Message 0 to RT transmit message buffer 0
      status = memcmp(pMessage0, uBCMessage, sizeof(pMessage0));
	  if ( status )
      {
         status = API_BIT_BC_RT_FAIL_PRI;
         break;
      }

      // Read the data from the second BC message
      status = BusTools_BC_MessageReadData(cardnum, 1, uBCMessage);
      if ( status != API_SUCCESS )
         break;

      // Compare BC Message 1 to RT transmit message buffer 1
      status = memcmp(pMessage1, uBCMessage, sizeof(pMessage1));
      if ( status )
      {
         status = API_BIT_BC_RT_FAIL_SEC;
         break;
      }
     
      status = BusTools_BM_ReadLastMessageBlock(cardnum,-1,-1,-1,&mcnt,bmmessage);
      if ( status )
         break;

      if(mcnt !=2)
      {
         status = API_BIT_BM_RT_FAIL_PRI;
         break;
      }
      // Read the first message from the BM
      // status = BusTools_BM_MessageRead(cardnum, bmMsgBufNum+0, &bmmessage);
      // Compare first BM Message to RT transmit message buffer 0
      status = memcmp(pMessage0, bmmessage[0].value, sizeof(pMessage0));
      if ( status )
      {
         status = API_BIT_BM_RT_FAIL_PRI;
         break;
      }

      // Read the data from the second BM buffer
      // Compare second BM Message to RT transmit message buffer 1
      status = memcmp(pMessage1, bmmessage[1].value, sizeof(pMessage1));
      if ( status )
      {
         status = API_BIT_BM_RT_FAIL_SEC;
         break;
      }
   }

#ifdef FILE_SYSTEM
   if(status)
      BusTools_DumpMemory(cardnum,0xffffffff, "InternalBit_Error.txt",BusTools_StatusGetString(status));
   // Make sure that the BC, BM and RT are shutdown
#endif

   BusTools_BC_StartStop(cardnum, BC_STOP);
   BusTools_BM_StartStop(cardnum, BM_STOP);
   BusTools_RT_StartStop(cardnum, RT_STOP);

   BusTools_API_Reset(cardnum,0); //reset to clear out BIT test setup.

   return status;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BIT_TwoBoardWrap
*
* FUNCTION
*     This function performs a simple wrap test between two boards (or the
*     two channels of a dual-channel board).
*     The primary bus of the first board is wired to the primary bus of the
*     second board, likewise the secondary busses are connected together.
*     Bus wiring to be per MIL-STD-1553 requirements.  Either direct or
*     transformer coupling may be used; the boards must have been opened
*     by the caller and either direct or transformer coupling selected.
*
*  First the External Bus is enabled.
*   Then one board is setup as a BC, the other board is setup as an RT.  The
*   BC sends two transmit messages to the RT; one on the primary bus and the
*   second on the secondary bus.  It compares the data received by the BC to
*   the data setup in the RT transmit buffers, and if they differ then the
*   test fails.
*
*  The test is then repeated, swapping the BC and RT functions.
*
*  When this function returns the board is still open, but the previous board
*  setup has been lost.  It is necessary that all of the setup functions be
*  called again, beginning with BusTools_BM_Init.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BIT_FAIL_PRI        -> Failure detected on primary bus
*     API_BIT_FAIL_SEC        -> Failure detected on secondary bus
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_TwoBoardWrap(
   BT_UINT FirstCard,      // (i) Card number of first open board in wrap test
   BT_UINT SecondCard,     // (i) Card number of second open board in wrap test
   BT_INT  TestPrimary,    // (i) Set TRUE if we should test the primary bus
   BT_INT  TestSecondary,  // (i) Set TRUE if we should test the secondary bus
   BT_INT  NumMessages,    // (i) Number of messages to test
   BT_INT  RT_addr,        // (i) RT address to test
   BT_INT  RT_subaddr)     // (i) RT subaddress to test
{
   BT_UINT            Cardnum1;       // First card's number(or half of a dual-chan board)
   BT_UINT            Cardnum2;       // Second card's number
   API_BC_MBUF        bcmessage;      // BC message being written to board
   BT_U16BIT          uBCMessage[32]; // BC data buffer read from board
   BT_U32BIT          dwIntenable;    // BC interrupt enables
   BT_U32BIT          TimeOutBC;      // Counter that times out the BC
   BT_UINT            flag;           // Flag indicates when BC has stopped

   API_BM_MBUF        uBMMessage[296];// BM Message Buffer
 
   BT_UINT            count;          // Actual number of BM messages allocated

   API_RT_ABUF        RTAbuf;         // RT Address Buffer
   API_RT_CBUF        RTCbuf;         // RT Control Buffer
   API_RT_MBUF_WRITE  RTmbuf;         // RT Message Buffer

   BT_INT             status;         // Status from BusTools/1553-API functions
   int                passcnt;        // Count of passes through this function
   int                i;
   BT_UINT            mcnt;              // Loop counter

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if ( FirstCard >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[FirstCard] == 0)
      return API_BUSTOOLS_NOTINITED;

   if ( SecondCard >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[SecondCard] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Perform the external wrap test in two passes swapping BC and RT.
   *******************************************************************/
   // Enable the external bus on both boards or channels
 
   for( passcnt = 0; passcnt < 2; passcnt++ )
   {
      // Select the board that is to be the bus controller(Cardnum1) and
      //  the board that is to be the Remote Terminal(Cardnum2).
      if (passcnt == 0)
      {
         Cardnum1 = FirstCard;
         Cardnum2 = SecondCard;
      }
      else
      {
         Cardnum2 = FirstCard;
         Cardnum1 = SecondCard;
      }

	  // Make sure that the BM and RT is shutdown on both cards:
      BusTools_BC_StartStop(Cardnum1, BC_STOP);
      BusTools_BM_StartStop(Cardnum1, BM_STOP);
      BusTools_RT_StartStop(Cardnum1, RT_STOP);
      BusTools_BC_StartStop(Cardnum2, BC_STOP);
      BusTools_BM_StartStop(Cardnum2, BM_STOP);
      BusTools_RT_StartStop(Cardnum2, RT_STOP);

	  BusTools_SetInternalBus(FirstCard,  EXTERNAL_BUS);
      BusTools_SetInternalBus(SecondCard, EXTERNAL_BUS);

      // Initialize the BM but do not start it, effectively resetting the board.
      status = BusTools_BM_Init(Cardnum1, BM_ENABLE_BUS, BM_ENABLE_BUS);
      if ( status )
         return status;

      // Allocate some Bus Monitor message buffers, not a multiple of 2!

      dwIntenable = 0xffffffffL;
      status = BusTools_BM_MessageAlloc(Cardnum1,50,&count,dwIntenable);
      if ( status )
         return status;

      // Initialize the bus controller with one data buffer per message.
      dwIntenable = 0xFFFFFFFF;     // Enable all BC interrupt conditions
      status = BusTools_BC_Init(Cardnum1,0,dwIntenable,0,20,20,FRAME_RATE,1);
      if ( status )
         return status;

      // Allocate three BC messages
      status = BusTools_BC_MessageAlloc(Cardnum1, 10);
      if ( status )
         return status;

      // Create and write the first message to the board.
      // This is a RT_addr, subaddress = RT_subaddr, transmit, 32 word message, primary chan A.
      memset((char*)&bcmessage,0,sizeof(bcmessage));
      bcmessage.messno_next = 1;                   // Next message to execute
      bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
      bcmessage.control |= BC_CONTROL_INTERRUPT;   // Enable interrupts on this message
      bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
      if ( TestPrimary )
         bcmessage.control |= BC_CONTROL_CHANNELA; // Transmit on Channel A
      else
         bcmessage.control |= BC_CONTROL_CHANNELB; // Transmit on Channel B
      bcmessage.control |= BC_CONTROL_MFRAME_BEG;
      bcmessage.mess_command1.rtaddr   = (BT_U16BIT)RT_addr;
      bcmessage.mess_command1.subaddr  = (BT_U16BIT)RT_subaddr;
      bcmessage.mess_command1.wcount   = 0;
      bcmessage.mess_command1.tran_rec = 1;
      bcmessage.errorid = 0;     // Default error injection buffer(no errors)
      bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
      status = BusTools_BC_MessageWrite(Cardnum1, 0, &bcmessage);
      if ( status )
         return status;

      // Create and write the second message to the board.
      // This is a RT_addr, subaddress = RT_subaddr, transmit, 32 word message, secondary chan B.
      memset((char*)&bcmessage,0,sizeof(bcmessage));
      bcmessage.messno_next = 2;                   // Next message to execute
      bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
      bcmessage.control |= BC_CONTROL_INTERRUPT;   // Enable interrupts on message
      bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
      if ( TestSecondary )
         bcmessage.control |= BC_CONTROL_CHANNELB; // Transmit on Channel B
      else
         bcmessage.control |= BC_CONTROL_CHANNELA; // Transmit on Channel A
      bcmessage.mess_command1.rtaddr   = (BT_U16BIT)RT_addr;
      bcmessage.mess_command1.subaddr  = (BT_U16BIT)RT_subaddr;
      bcmessage.mess_command1.wcount   = 0;
      bcmessage.mess_command1.tran_rec = 1;
      bcmessage.errorid = 0;     // Default error injection buffer(no errors)
      bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
      status = BusTools_BC_MessageWrite(Cardnum1, 1, &bcmessage);
      if ( status )
         return status;

      // Create and write the third(STOP) message to the board.
      memset((char*)&bcmessage,0,sizeof(bcmessage));
      bcmessage.messno_next = 0;
      bcmessage.messno_branch = 0;
      bcmessage.control = BC_CONTROL_LAST;         // End of list(1 shot mode)
      status = BusTools_BC_MessageWrite(Cardnum1, 3, &bcmessage);
      if ( status != API_SUCCESS )
         return status;

      // Start the BM so it is ready when the Bus Controller starts.
      status = BusTools_BM_StartStop(Cardnum1, BM_START);
      if ( status )
         return status;

      /*******************************************************************
      *  Initialize the second channel.
      *******************************************************************/
      status = BusTools_BM_Init(Cardnum2, BM_ENABLE_BUS, BM_ENABLE_BUS);
      if ( status )
         return status;

      // Allocate some Bus Monitor message buffers, not a multiple of 2!
      dwIntenable = 0xffffffffL;
	  status = BusTools_BM_MessageAlloc(Cardnum2,50,&count,dwIntenable);
      if ( status != API_SUCCESS )
         return status;

      // Initialize the RT on the second "board"
      status = BusTools_RT_Init(Cardnum2,0);
      if ( status )
         return status;

      // Enable RT for RT RT_ADDR
      RTAbuf.enable_a = 1;           // Enable primary channel A
      RTAbuf.enable_b = 1;           // Enable secondary channel B
      RTAbuf.inhibit_term_flag = 0;  // Terminal flag
      RTAbuf.status   = 0;           // status word flags
      RTAbuf.bit_word = 0;           // BIT word

      status = BusTools_RT_AbufWrite(Cardnum2, RT_addr, &RTAbuf);
      if ( status )
         return status;

      // Create two message buffers for RT_ADDR, subaddress RT_SADDR, transmit
      RTCbuf.legal_wordcount = 0xFFFFFFFF;   // Enable all word counts
      BusTools_RT_CbufWrite(Cardnum2, RT_addr, RT_subaddr, 1, RT_MBUF_COUNT, &RTCbuf);

      // Start the BM so it is ready when the Bus Controller starts.
      status = BusTools_BM_StartStop(Cardnum2, BM_START);
      if ( status )
         return status;

      // Start the RT so it is ready when the Bus Controller starts.
      status = BusTools_RT_StartStop(Cardnum2, BM_START);
      if ( status )
         return status;

      // Loop through the following code the specified number of times
      for( i = 0; i < NumMessages; i++ )
      {
         // Write unique data to RT_ADDR, subaddress RT_SADDR, Transmit, buffer 0
         RTmbuf.enable = 0xFFFFFFFF;   // Enable all interrupt conditions
         RTmbuf.error_inj_id = 0;      // Use the default error injection buffer
         pMessage0[0]++;               // Update the message data
         memcpy(RTmbuf.mess_data, pMessage0, sizeof(pMessage0));
         status = BusTools_RT_MessageWrite(Cardnum2, RT_addr, RT_subaddr, 1, 0, &RTmbuf);
         if ( status )
            return status;

         // Write unique data to RT_ADDR, subaddress RT_subaddr, Transmit, buffer 1
         pMessage1[0]--;               // Update the message data
         memcpy(RTmbuf.mess_data, pMessage1, sizeof(pMessage1));
         status = BusTools_RT_MessageWrite(Cardnum2, RT_addr, RT_subaddr, 1, 1, &RTmbuf);
         if ( status != API_SUCCESS )
            return status;

         // Start the Bus Controller
         status = BusTools_BC_StartStop(Cardnum1, BC_START);
         if ( status )
            return status;

         /* Wait for the two BC messages to be transacted. */
         TimeOutBC = 20;    /* set a 40ms time-out */
         do
         {
           MSDELAY(2);
           BusTools_BC_IsRunning(Cardnum1, &flag);		
         }
		 while ( flag && --TimeOutBC );

         // Read the data from the first BC message
         status = BusTools_BC_MessageReadData(Cardnum1, 0, uBCMessage);
         if ( status )
            return status;

         // Compare the data that the BC received from the RT to the data
         //  that we loaded into the RT transmit buffers.
         // Compare BC Message 0 to RT transmit message buffer 0
         status = memcmp(pMessage0, uBCMessage, sizeof(pMessage0));
         if ( status )
		 {
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,0xffffffff, "twoboardwrap_Error.txt","API_BIT_BC_RT_FAIL_PRI");
#endif
            return API_BIT_BC_RT_FAIL_PRI;
		 }

         // Read the data from the second BC message
         status = BusTools_BC_MessageReadData(Cardnum1, 1, uBCMessage);
         if ( status != API_SUCCESS )
            return status;

         // Compare BC Message 1 to RT transmit message buffer 1
         status = memcmp(pMessage1, uBCMessage, sizeof(pMessage1));
         if ( status )
		 {
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,0xffffffff, "twoboardwrap_Error.txt","API_BIT_BC_RT_FAIL_SEC");
#endif
            return API_BIT_BC_RT_FAIL_SEC;
		 }
            
         // Now read data from the next two BM buffers on the first card and
         //  verify that the data is correct.
         // Read the data from the Bm messages

		 BusTools_BM_ReadLastMessageBlock(Cardnum1,-1,-1,-1,&mcnt,uBMMessage);
		 if(mcnt != 2)
         {
#ifdef FILE_SYSTEM
             BusTools_DumpMemory(Cardnum1,DUMP_ALL, "twoboardwrap_Error.txt","API_BIT_BM_RT_FAIL_PRI");
#endif
			 return API_BIT_BM_RT_FAIL_PRI;
         }
         // Compare the data that the BC received from the RT to the data
         //  that we loaded into the RT transmit buffers.
         // Compare BC Message 0 to RT transmit message buffer 0
         status = memcmp(pMessage0, uBMMessage[0].value, sizeof(pMessage0));

         if ( status )
		 { 
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,DUMP_ALL, "twoboardwrap_Error.txt","API_BIT_BC_RT_FAIL_PRI");
#endif 
            return API_BIT_BC_RT_FAIL_PRI;
		 }

         // Compare BC Message 1 to RT transmit message buffer 1
         status = memcmp(pMessage1, uBMMessage[1].value, sizeof(pMessage1));
         if ( status )
		 {
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,DUMP_ALL, "twoboardwrap_Error.txt","API_BIT_BC_RT_FAIL_SEC"); 
#endif
            return API_BIT_BC_RT_FAIL_SEC;
		 }
         // Now read data from the next two BM buffers on the second card and
         //  verify that the data is correct.
         // Read the data from the first BM message

		 BusTools_BM_ReadLastMessageBlock(Cardnum2,-1,-1,-1,&mcnt,uBMMessage);
		 if(mcnt != 2)
         {
#ifdef FILE_SYSTEM
             BusTools_DumpMemory(Cardnum1,0xffffffff, "twoboardwrap_Error.txt","API_BIT_BM_RT_FAIL_SEC"); 
#endif
			 return API_BIT_BM_RT_FAIL_SEC;
         }
         // Compare the data that the BC received from the RT to the data
         //  that we loaded into the RT transmit buffers.
         // Compare BC Message 0 to RT transmit message buffer 0

         status = memcmp(pMessage0, uBMMessage[0].value, sizeof(pMessage0));
         if ( status )
		 {
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,DUMP_ALL, "twoboardwrap_Error.txt","API_BIT_BC_RT_FAIL_PRI");
#endif 
            return API_BIT_BC_RT_FAIL_PRI;
		 }

         // Compare the next BM Message to RT transmit message buffer 1
         status = memcmp(pMessage1, uBMMessage[1].value, sizeof(pMessage1));
         if ( status )
		 {
#ifdef FILE_SYSTEM
            BusTools_DumpMemory(Cardnum1,DUMP_ALL, "twoboardwrap_Error.txt","API_BIT_BM_RT_FAIL_SEC"); 
#endif
            return API_BIT_BM_RT_FAIL_SEC;
		 }
      }
   }
   // Make sure that the BM and RT is shutdown on both cards:
   BusTools_BC_StartStop(Cardnum1, BC_STOP);
   BusTools_BM_StartStop(Cardnum1, BM_STOP);
   BusTools_RT_StartStop(Cardnum1, RT_STOP);
   BusTools_BC_StartStop(Cardnum2, BC_STOP);
   BusTools_BM_StartStop(Cardnum2, BM_STOP);
   BusTools_RT_StartStop(Cardnum2, RT_STOP);

   BusTools_API_Reset(Cardnum1,0); //reset to clear out BIT test setup.
   BusTools_API_Reset(Cardnum2,0); //reset to clear out BIT test setup.

   return API_SUCCESS;
}


/****************************************************************************
*
* PROCEDURE NAME - BusTools_BIT_CableWrap
*
* FUNCTION
*     This function performs an external wrap test of a single board that has
*     the primary and secondary buses externally connected together.  It may
*     be used to test a single function (or multi-function) board.
*
*     This is NOT a MIL-STD-1553-supported configuration;
*     the PASS criteria is that an error is detected!
*
*     Direct or transformer coupling may be used; the board must have been
*     opened by the caller and either direct or transformer coupling selected
*     to agree with the bus wiring, before this function is called.
*
*     It is assumed that there are no other BC's or RT's connected to the bus
*     when this test is run.
*
*  First the External Bus is enabled, then the BC is setup.  The BC sends two
*  transmit messages; one on the primary bus and the other on the secondary bus.
*  It tests to see that the BT1553_INT_TWO_BUS error status is set, if not it
*  returns API_BIT_FAIL_PRI or API_BIT_FAIL_SEC.
*
*  If all iterations pass, this function will return with API_SUCCESS,
*   otherwise it will return one of the other error codes.
*
*  When this function returns the board is still open, but the previous board
*  setup has been lost.  It is necessary that all of the setup functions be
*  called again, beginning with BusTools_BM_Init.
*
*   Returns
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> Card number out of range
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_BIT_FAIL_PRI        -> Failure detected on primary bus
*     API_BIT_FAIL_SEC        -> Failure detected on secondary bus
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_CableWrap(
   BT_UINT cardnum,        // (i) Card number of open board to test
   BT_INT  NumMessages)    // (i) Number of messages to test
{
   API_BC_MBUF        bcmessage;      // BC message being written to board
   BT_U32BIT          dwIntenable;    // BC interrupt enables
   BT_UINT            flag;           // Flag indicates when BC has stopped
   BT_U32BIT          TimeOutBC;      // Counter that times out the BC
   BT_INT             status;         // Status from BusTools/1553-API functions
   int                i;              // Loop counter

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Perform the internal wrap test the primary and secondary busses.
   *******************************************************************/
   // Make sure that the BC, BM and RT are shutdown
   BusTools_BC_StartStop(cardnum, BC_STOP);
   BusTools_BM_StartStop(cardnum, BM_STOP);
   BusTools_RT_StartStop(cardnum, RT_STOP);
   // Enable the external bus on the board
   BusTools_SetInternalBus(cardnum, 0);

   // Initialize the BM but do not start it, effectively resetting the board.
   // Enable both the primary and secondary busses.
   status = BusTools_BM_Init(cardnum, BM_ENABLE_BUS, BM_ENABLE_BUS);
   if ( status )
      return status;

   // Initialize the bus controller with one data buffer per message.
   dwIntenable = 0xFFFFFFFFl;     // Enable all BC interrupt conditions
   status = BusTools_BC_Init(cardnum,0,dwIntenable,0,20,20,FRAME_RATE,1);
   if ( status )
      return status;

   // Allocate three BC messages
   status = BusTools_BC_MessageAlloc(cardnum, 3);
   if ( status )
      return status;

   // Create and write the first message to the board.
   // This is a RT_ADDR, subaddress = RT_SADDR, transmit, 32 word message, chan A.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 1;                   // Next message to execute
   bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
   bcmessage.control |= BC_CONTROL_INTERRUPT;   // Enable interrupts on message
   bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA; // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_MFRAME_BEG;
   bcmessage.mess_command1.rtaddr   = RT_ADDR;
   bcmessage.mess_command1.subaddr  = RT_SADDR;
   bcmessage.mess_command1.wcount   = 0;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer(no errors)
   bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
   status = BusTools_BC_MessageWrite(cardnum, 0, &bcmessage);
   if ( status )
      return status;

   // Create and write the second message to the board.
   // This is a RT_ADDR, subaddress = RT_SADDR, transmit, 32 word message, chan B.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 2;                   // Next message to execute
   bcmessage.control = BC_CONTROL_MESSAGE;      // show as a message
   bcmessage.control |= BC_CONTROL_INTERRUPT;   // Enable interrupts on message
   bcmessage.control |= BC_CONTROL_BUFFERA;     // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELB; // Transmit on Channel B
   bcmessage.mess_command1.rtaddr   = RT_ADDR;
   bcmessage.mess_command1.subaddr  = RT_SADDR;
   bcmessage.mess_command1.wcount   = 0;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer(no errors)
   bcmessage.gap_time = 12;   // 12 microsecond inter-message gap.
   status = BusTools_BC_MessageWrite(cardnum, 1, &bcmessage);
   if ( status )
      return status;

   // Create and write the third(STOP) message to the board.
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno_next = 0;
   bcmessage.messno_branch = 0;
   bcmessage.control = BC_CONTROL_LAST;         // End of list (1 shot mode)
   status = BusTools_BC_MessageWrite(cardnum, 2, &bcmessage);
   if ( status != API_SUCCESS )
      return status;

   // Loop through the following code the specified number of times
   for ( i = 0; i < NumMessages; i++ )
   {
      // Start the Bus Controller
      status = BusTools_BC_StartStop(cardnum, BC_START);
      if ( status )
         return status;

	  /* Wait for the two BC messages to be transacted. */
      TimeOutBC = 20;    /* set a 40ms time-out */
      do
      {
        MSDELAY(2);
        BusTools_BC_IsRunning(cardnum, &flag);		
      }
      while ( flag && --TimeOutBC );

      // Read the data from the first BC message
      status = BusTools_BC_MessageRead(cardnum, 0, &bcmessage);
      if ( status )
         return status;

      // Verify that the "BT1553_INT_TWO_BUS" bit is set in the interrupt
      //  status word.  This indicates that the opposite bus saw that data
      //  that was sent on the bus.
      // Verify BC message 0 error status
      if ( (bcmessage.status & BT1553_INT_TWO_BUS) != BT1553_INT_TWO_BUS )
      {
#ifdef FILE_SYSTEM
         BusTools_DumpMemory(cardnum,0xffffffff, "Cablewrap_Error.txt","API_BIT_BC_RT_FAIL_PRI"); 
#endif
         return API_BIT_BC_RT_FAIL_PRI;
      }
      // Read the data from the second BC message
      status = BusTools_BC_MessageRead(cardnum, 1, &bcmessage);
      if ( status != API_SUCCESS )
         return status;
   }
   // Make sure that the BC is shutdown
   BusTools_BC_StartStop(cardnum, BC_STOP);

   BusTools_API_Reset(cardnum,0); //reset to clear out BIT test setup.

   return API_SUCCESS;
}


//-----------------------------------------------------------------------------
// Expected sizes of API internal structures 
// (From the BusTools/1553-API Software Reference Manual)
#define SIZE_BC_V6MESSAGE		36   
#define SIZE_BC_V6CBUF		    36   
#define SIZE_BC_V6DBLOCK		92  
#define SIZE_BM_V6CBUF		    8
#define SIZE_BM_V6MBUF		    172
#define SIZE_BM_V6FBUF		    8192
#define SIZE_BM_V6TBUF_ENH	    92 
#define SIZE_EI_V6MESSAGE		66
#define SIZE_RT_V6ABUF_ENTRY	8
#define SIZE_RT_V6ABUF		    256
#define SIZE_RT_V6CBUF		    8
#define SIZE_RT_V6CBUFBROAD	    132
#define SIZE_RT_V6FBUF		    8192
#define SIZE_RT_V6MBUF_HW		96
#define SIZE_RT_V6MBUF_API	    8
#define SIZE_RT_V6MBUF		    104
#define SIZE_BT1553_V6TIME	    8
#define SIZE_IQ_V6MBLOCK		8

#define SIZE_BC_MESSAGE		48   // was 32   // was 24
#define SIZE_BC_CBUF		48   // was 32   // was 24
#define SIZE_BC_DBLOCK		68   // was 66
#define SIZE_BM_CBUF		8
#define SIZE_BM_MBUF		168
#define SIZE_BM_FBUF		4096
#define SIZE_BM_TBUF_ENH	106  // was SIZE_BM_TBUF  42
#define SIZE_EI_MESSAGE		66
#define SIZE_RT_ABUF_ENTRY	8
#define SIZE_RT_ABUF		256
#define SIZE_RT_CBUF		6
#define SIZE_RT_CBUFBROAD	126
#define SIZE_RT_FBUF		4096
#define SIZE_RT_MBUF_HW		88
#define SIZE_RT_MBUF_API	8
#define SIZE_RT_MBUF		96
#define SIZE_BT1553_V5TIME	6
#define SIZE_BT1553_TIME    8
#define SIZE_IQ_MBLOCK		6


#define dprint(a,b) if(flag){printf(a,b);}
/****************************************************************************
*
* PROCEDURE NAME - BusTools_BIT_StructureAlignmentCheck
*
* FUNCTION
*   This BIT test check for the correct structure alignment for the critical
*   API structures.  If these structure are not packed and aligned correctly
*   The data return from API called will get shifted in the structures and
*   appear corrupt.
*
*   Returns
*     API_SUCCESS             -> success
*     API_STRUCT_ALIGN        -> Alignment Error
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_StructureAlignmentCheckV6(BT_INT flag)
{
   int status=API_SUCCESS;

   dprint("sizeof BC_V6MESSAGE\t\t= %d \t",BC_MSGBUF_SIZE);
   if (BC_MSGBUF_SIZE != SIZE_BC_V6MESSAGE) 
   {
       status = API_STRUCT_ALIGN;
       dprint("***** ERROR - Should be %d *****\n", SIZE_BC_V6MESSAGE);
   }
   else
      dprint(" %s\n","OK");    
    
    dprint("sizeof BC_CBUF\t\t\t= %d \t",sizeof(BC_CTRLBUF));
	if (BC_CTRLBUF_SIZE != SIZE_BC_V6CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BC_V6CBUF);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BC_MSGDATA\t\t= %d \t",sizeof(BC_MSGDATA));
	if (sizeof(BC_MSGDATA) != SIZE_BC_V6DBLOCK) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BC_V6DBLOCK);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BC_CTRLDATA\t\t= %d \t",sizeof(BC_CTRLDATA));
	if (sizeof(BC_CTRLDATA) != SIZE_BC_V6DBLOCK) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BC_V6DBLOCK);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BM_CBUF\t\t\t= %d \t",sizeof(BM_CBUF));
	if (sizeof(BM_CBUF) != SIZE_BM_V6CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_V6CBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BM_V6MBUF\t\t= %d \t",sizeof(BM_V6MBUF));
	if (sizeof(BM_V6MBUF) != SIZE_BM_V6MBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_V6MBUF);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BM_V6FBUF\t\t= %d \t",sizeof(BM_V6FBUF));
	if (sizeof(BM_V6FBUF) != SIZE_BM_V6FBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_V6FBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BM_TBUF_ENH\t\t= %d \t",sizeof(BM_V6TBUF_ENH));
	if (sizeof(BM_V6TBUF_ENH) != SIZE_BM_V6TBUF_ENH) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_V6TBUF_ENH);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof EI_MESSAGE\t\t= %d \t",sizeof(EI_MESSAGE));
	if (sizeof(EI_MESSAGE) != SIZE_EI_V6MESSAGE) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_EI_V6MESSAGE);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_V6ABUF_ENTRY\t\t= %d \t",sizeof(RT_V6ABUF_ENTRY));
	if (sizeof(RT_V6ABUF_ENTRY) != SIZE_RT_ABUF_ENTRY) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6ABUF_ENTRY);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_ABUF\t\t\t= %d \t",sizeof(RT_V6ABUF));
	if (sizeof(RT_V6ABUF) != SIZE_RT_V6ABUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6ABUF);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_CBUF\t\t\t= %d \t",sizeof(RT_V6CBUF));
	if (sizeof(RT_V6CBUF) != SIZE_RT_V6CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_CBUF);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_CBUFBROAD\t\t= %d \t",sizeof(RT_V6CBUFBROAD));
	if (sizeof(RT_V6CBUFBROAD) != SIZE_RT_V6CBUFBROAD) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6CBUFBROAD);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_FBUF\t\t\t= %d \t",sizeof(RT_V6FBUF));
	if (sizeof(RT_V6FBUF) != SIZE_RT_V6FBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6FBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF_HW\t\t= %d \t",sizeof(RT_MBUF_V6HW));
	if (sizeof(RT_MBUF_V6HW) != SIZE_RT_V6MBUF_HW) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6MBUF_HW);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF_API\t\t= %d \t",sizeof(RT_MBUF_API));
	if (sizeof(RT_MBUF_API) != SIZE_RT_V6MBUF_API) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6MBUF_API);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF\t\t\t= %d \t",sizeof(RT_V6MBUF));
	if (sizeof(RT_V6MBUF) != SIZE_RT_V6MBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_V6MBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BT1553_TIME\t\t= %d \t",sizeof(BT1553_TIME));
	if (sizeof(BT1553_TIME) != SIZE_BT1553_V6TIME) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BT1553_V6TIME);
	}
   else
      dprint(" %s\n","OK");
	
	dprint("sizeof IQ_MBLOCK_V6\t\t= %d \t",sizeof(IQ_MBLOCK_V6));
	if (sizeof(IQ_MBLOCK_V6) != SIZE_IQ_V6MBLOCK) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_IQ_V6MBLOCK);
	}
   else
      dprint(" %s\n","OK");

	if (status == API_STRUCT_ALIGN)
    {
        dprint("***** INCORRECT STRUCTURE SIZES - %s *****\n\n","TEST FAILED");
    }
	else 
    {
       dprint("\n***** %s *****\n","ALL TESTS PASSED");
    }
    return status;
}

/****************************************************************************
*
* PROCEDURE NAME - BusTools_BIT_StructureAlignmentCheck
*
* FUNCTION
*   This BIT test check for the correct structure alignment for the critical
*   API structures.  If these structure are not packed and aligned correctly
*   The data return from API called will get shifted in the structures and
*   appear corrupt.
*
*   Returns
*     API_SUCCESS             -> success
*     API_STRUCT_ALIGN        -> Alignment Error
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_BIT_StructureAlignmentCheck(BT_INT flag)
{
   int status=API_SUCCESS;

   dprint("sizeof BC_MESSAGE    = %d \t",sizeof(BC_MESSAGE));
   if (sizeof(BC_MESSAGE) != SIZE_BC_MESSAGE) 
   {
       status = API_STRUCT_ALIGN;
       dprint("***** ERROR - Should be %d *****\n", SIZE_BC_MESSAGE);
   }
   else
      dprint(" %s\n","OK");    
    
    dprint("sizeof BC_CBUF       = %d \t",sizeof(BC_CBUF));
	if (sizeof(BC_CBUF) != SIZE_BC_CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BC_CBUF);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BC_DBLOCK     = %d \t",sizeof(BC_DBLOCK));
	if (sizeof(BC_DBLOCK) != SIZE_BC_DBLOCK) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BC_DBLOCK);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BM_CBUF       = %d \t",sizeof(BM_CBUF));
	if (sizeof(BM_CBUF) != SIZE_BM_CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_CBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BM_MBUF       = %d \t",sizeof(BM_MBUF));
	if (sizeof(BM_MBUF) != SIZE_BM_MBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_MBUF);
	}
   else
      dprint(" %s\n","OK");

    dprint("sizeof BM_FBUF       = %d \t",sizeof(BM_FBUF));
	if (sizeof(BM_FBUF) != SIZE_BM_FBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_FBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BM_TBUF_ENH   = %d \t",sizeof(BM_TBUF_ENH));
	if (sizeof(BM_TBUF_ENH) != SIZE_BM_TBUF_ENH) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BM_TBUF_ENH);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof EI_MESSAGE    = %d \t",sizeof(EI_MESSAGE));
	if (sizeof(EI_MESSAGE) != SIZE_EI_MESSAGE) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_EI_MESSAGE);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_ABUF_ENTRY = %d \t",sizeof(RT_ABUF_ENTRY));
	if (sizeof(RT_ABUF_ENTRY) != SIZE_RT_ABUF_ENTRY) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_ABUF_ENTRY);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_ABUF       = %d \t",sizeof(RT_ABUF));
	if (sizeof(RT_ABUF) != SIZE_RT_ABUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_ABUF);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_CBUF       = %d \t",sizeof(RT_CBUF));
	if (sizeof(RT_CBUF) != SIZE_RT_CBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_CBUF);
	}	
   else
      dprint(" %s\n","OK");

    dprint("sizeof RT_CBUFBROAD  = %d \t",sizeof(RT_CBUFBROAD));
	if (sizeof(RT_CBUFBROAD) != SIZE_RT_CBUFBROAD) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_CBUFBROAD);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_FBUF       = %d \t",sizeof(RT_FBUF));
	if (sizeof(RT_FBUF) != SIZE_RT_FBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_FBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF_HW    = %d \t",sizeof(RT_MBUF_HW));
	if (sizeof(RT_MBUF_HW) != SIZE_RT_MBUF_HW) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_MBUF_HW);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF_API   = %d \t",sizeof(RT_MBUF_API));
	if (sizeof(RT_MBUF_API) != SIZE_RT_MBUF_API) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_MBUF_API);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof RT_MBUF       = %d \t",sizeof(RT_MBUF));
	if (sizeof(RT_MBUF) != SIZE_RT_MBUF) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_RT_MBUF);
	}
   else
      dprint(" %s\n","OK");
	
    dprint("sizeof BT1553_TIME   = %d \t",sizeof(BT1553_TIME));
	if (sizeof(BT1553_TIME) != SIZE_BT1553_TIME) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_BT1553_TIME);
	}
   else
      dprint(" %s\n","OK");
	
	dprint("sizeof IQ_MBLOCK     = %d \t",sizeof(IQ_MBLOCK));
	if (sizeof(IQ_MBLOCK) != SIZE_IQ_MBLOCK) {
		status = API_STRUCT_ALIGN;
		dprint("***** ERROR - Should be %d *****\n", SIZE_IQ_MBLOCK);
	}
   else
      dprint(" %s\n","OK");

	if (status == API_STRUCT_ALIGN)
    {
        dprint("***** INCORRECT STRUCTURE SIZES - %s *****\n\n","TEST FAILED");
    }
	else 
    {
       dprint("\n***** %s *****\n","ALL TESTS PASSED");
    }
    return status;
}
