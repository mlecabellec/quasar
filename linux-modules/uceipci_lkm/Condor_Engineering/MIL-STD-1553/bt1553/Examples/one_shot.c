/*===========================================================================*
 * FILE:                    O N E _ S H O T . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 1995 - 2007 BY
 *          ABACO SYSTEMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS 
 *
 *============================================================================*
 *
 * FUNCTION:    Demonstration of BusTools/1553-API routines. This console 
 *              application shows how setup a BC messages that transact just
 *              and then stop the Bus Controller.  This demonstrate how to set
 *              a 1Shot message frame
 *
 *===========================================================================*/

/* $Revision:  4.40 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  6/11/2001    initial release
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include "busapi.h"

int BusController_Init(int,int);
int BC_Message_Write(int cardnum);
int BusMonitor_Init(int);
int RemoteTerminal_Init(int);
void setupThe_BC_IntFIFO(int);


API_INT_FIFO  sIntFIFO1;     // Thread FIFO structure for BC

int main(int argc, char **argv)
{
   int status, RT_cardnum;
   unsigned int cardnum, mode,device;

   device=cardnum=0;
   RT_cardnum=1;

   mode=1;

   BusTools_API_Close(cardnum);
   BusTools_API_Close(RT_cardnum);

   printf("Initializing Test\n");

   //Initial the channel running the BC
   status = BusTools_API_OpenChannel(&cardnum,mode,device,CHANNEL_1);
   printf("BusTools_API_OpenChannel BC status = %d\n",status);

   //Set the Bus to External for BC
   status = BusTools_SetInternalBus(cardnum,0);  // Set up External bus 0 on Internal 1
   printf("BusTools_SetInternalBus BC status = %d\n",status);

   //Set the coupling for both channels
   status = BusTools_SetVoltage(cardnum,1950,1);  //Transformer coupling
   printf("BusTools_SetVoltage BC status = %d\n",status);

   //Set the time mode for both channels
   status = BusTools_TimeTagMode(cardnum, API_TTD_IRIG, API_TTI_IRIG, API_TTM_FREE, NULL, 0, 0, 0 );
   printf("BusTools_TimeTagMode BC status = %d\n",status);

   //Initialize the Bus Monitor for Both Channels
   status = BusMonitor_Init(cardnum);

   //Set up the BC for 1Shot mode
   status = BusController_Init(cardnum,0);

   //Once the BC is intialize you can write data into the BC buffers
   //This can go into a loops to update the message
   status = BC_Message_Write(cardnum);

   //Set up the Rt to respond to the BC
   status = RemoteTerminal_Init(cardnum);

   //Setting up a callback function to show the BC data
   setupThe_BC_IntFIFO(cardnum); //Set up BC interrupts using BusTools_RegisterFunction

   printf("Hit enter to start.  Hit q to stop\n");
   getchar();

   //Start the RT running 1st
   status = BusTools_RT_StartStop(cardnum,1);
   printf("BusTools_RT_StartStop status = %d\n",status);

   //Start the BC.  It will run until all message in the frame are sent
   status = BusTools_BC_StartStop(cardnum,1);
   printf("BusTools_BC_StartStop status = %d\n",status);

   {
      char ichar;
      while((ichar=getc(stdin))!='q')
      {}
   }

   printf("main: closing API\n");

   status = BusTools_BC_StartStop(cardnum,0); 
   printf("BusTools_BC_StartStop status = %d\n",status);

  status = BusTools_RT_StartStop(cardnum,0);
   printf("BusTools_RT_StartStop status = %d\n",status);

   status = BusTools_RegisterFunction(cardnum,&sIntFIFO1,0);
   printf("BusTools_RegisterFunction status = %d\n",status);

   status = BusTools_API_Close(cardnum);
   printf("BusTools_API_Close BC status = %d\n",status);

   return 0;
}

int BusController_Init(int cardnum,int sa_index)
{
   int status;

   /* Initialize the BC.  You can select the interrupt, timeouts and frame time.        */
   /* However Frame time does not come into affect since the frame is stop on competion */
   status = BusTools_BC_Init(cardnum,0,BT1553_INT_END_OF_MESS,0,16,14,500000,1);
   printf("BusTools_BC_Init status = %d\n",status);

   //Allocate enough BC Buffer to hold the maximum number of meesage you are using
   status = BusTools_BC_MessageAlloc(cardnum,50);
   printf("BusTools_BC_MessageAlloc status = %d\n",status);

   return status; 
}

int BC_Message_Write(int cardnum)
{
   int status, messno;
   API_BC_MBUF bcmessage;

   //Now create each message.  You can Update these eac
   messno = 0;
   memset((char*)&bcmessage,0,sizeof(bcmessage));    
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_RETRY;
   bcmessage.control |= BC_CONTROL_MFRAME_BEG;
   bcmessage.mess_command1.rtaddr   = 2;
   bcmessage.mess_command1.subaddr  = 2;
   bcmessage.mess_command1.wcount   = 2;
   bcmessage.mess_command1.tran_rec = 0;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;   // 20 microsecond inter-message gap.
   bcmessage.data[0][0] = 1;
   bcmessage.data[0][1] = 1;

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);
   
   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 3;
   bcmessage.mess_command1.subaddr  = 3;
   bcmessage.mess_command1.wcount   = 3;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 1000; // 1000 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 4;
   bcmessage.mess_command1.subaddr  = 4;
   bcmessage.mess_command1.wcount   = 4;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;   // 20 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 5;
   bcmessage.mess_command1.subaddr  = 5;
   bcmessage.mess_command1.wcount   = 9;
   bcmessage.mess_command1.tran_rec = 0;
   bcmessage.errorid = 0;       // Default error injection buffer (no errors)
   bcmessage.gap_time = 400;    // 400 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;                                              
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 2;
   bcmessage.mess_command1.subaddr  = 8;
   bcmessage.mess_command1.wcount   = 8;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 200 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

// Mode Code 
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.mess_command1.rtaddr   = 4;
   bcmessage.mess_command1.subaddr  = 0;
   bcmessage.mess_command1.wcount   = 18;  
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;   // 20 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   //This stops the BC at the end of the frame. 
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(0);
   bcmessage.control = BC_CONTROL_LAST;        // show as a message

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   return status;
}  

/****************************************************************************
*
*  Function:  bc_intFunction
*
*  Description:
*     This routine is the user callback function invoke when the Board detects
*     a end-of-message condition. This function read the data in the RT message
*     and print message data to the screen.
*
****************************************************************************/
BT_INT  _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO)
{   
   API_BC_MBUF bcmessage;
   int       bufnum;
   int       i,j;
   int       status;
   int       wcount;
   BT_INT tail, rtrt,mc;
   BT_UINT messno;

   tail = sIntFIFO->tail_index;
   while(tail != sIntFIFO->head_index)
   {
	  if(sIntFIFO->FilterType == EVENT_BC_MESSAGE)
	  {
		 messno = sIntFIFO->fifo[tail].bufferID;
               
          status = BusTools_BC_MessageRead(cardnum,messno,&bcmessage);

          if (status)
             return status;
          rtrt=0;
          mc=0;
          if(bcmessage.status & BT1553_INT_RT_RT_FORMAT)
          {
             printf("RT<->RT (%d) \n",messno);
             rtrt=1;
          }
          else if(bcmessage.status & BT1553_INT_MODE_CODE)
          {
             printf("Mode Code --  (%d) \n", messno);
             mc=1;
          }
          else if (bcmessage.mess_command1.tran_rec)
             printf("BC**RT->BC (%d)  \n", messno);
          else
             printf("BC->RT (%d)  \n", messno);
 
          printf("BC**RT-%d  \n",bcmessage.mess_command1.rtaddr);
          printf("BC**SA-%d  \n",bcmessage.mess_command1.subaddr);
          printf("BC**WC-%d  \n",bcmessage.mess_command1.wcount);
          if(rtrt)
          {   
             printf("BC**RT-%d  \n",bcmessage.mess_command2.rtaddr);
             printf("BC**SA-%d  \n",bcmessage.mess_command2.subaddr);
             printf("BC**WC-%d  \n",bcmessage.mess_command2.wcount);
          }
        
          printf("BC**status-1 0x%04x\n",*((WORD*)(&bcmessage.mess_status1)));
          if(rtrt)
             printf("BC**status-2 0x%04x\n",*((WORD*)(&bcmessage.mess_status2)));
          
          printf("BC**int_stat-0x%08x\n",*((DWORD*)(&bcmessage.status)));

          wcount = bcmessage.mess_command1.wcount;
          if (wcount == 0)
             wcount = 32;           // Zero word count means 32 words.

          if(mc)
            if(wcount >=16)
               wcount = 1;

          j = wcount;               // See if this msg shorted than previous
    
          if (bcmessage.control & BC_CONTROL_BUFFERA)
             bufnum = 0;            // Display data from first BC buffer
          else
             bufnum = 1;            // Display data from second BC buffer

          for ( i = 0; i < j; i++ ) // Display all of the data words
          {
             // If this is an RT->BC message, and there is a Message Error
             //  or No Response, than xxxx the data (since the RT did not
             //    send any data words).
             if ( i >= wcount )
             {
                printf("    ");     // Data does not apply to this msg
             }
             else if ( bcmessage.mess_command1.tran_rec &&
               (bcmessage.mess_status1.me || (bcmessage.status&BT1553_INT_NO_RESP) ) )
             {
                printf("xxxx ");      // No data was transfered because of error
             }
             else
             {
                printf("BC**%04x ",bcmessage.data[bufnum][i]);
             }
          }
          printf("\n");
      }
            
       tail++;
       tail &= sIntFIFO->mask_index;
       sIntFIFO->tail_index = tail;
   }

   return 0;
}

int RemoteTerminal_Init(int cardnum)
{
   int status;
   int RT_MBUF_COUNT;
   API_RT_CBUF cbuf;       // RT Control Buffer Legal Word Count Mask (2 words)
   API_RT_MBUF_WRITE mbuf;
   API_RT_ABUF abuf;
   int         tr;
   int         subaddress;
   int i,j,ii,jj;
   int rt_addr[] = {1,2,3,4,5,6,7,8};

   RT_MBUF_COUNT = 5;

   status = BusTools_RT_Init(cardnum,0);
   printf("BusTools_RT_Init status = %d\n",status);

   /**********************************************************************
   *  Output RT control buffers
   *   Enable or disable all word counts and mode codes as specified by
   *    the caller's arguements.
   **********************************************************************/

   cbuf.legal_wordcount = 0xffffffff;  // legailize all word counts
   for ( i = 0; i < 8; i++)
   {
      for ( tr = 0; tr < 2; tr++ )
      {
         for ( subaddress = 0; subaddress < 16; subaddress++ )
         {
            status = BusTools_RT_CbufWrite(cardnum,rt_addr[i],subaddress,tr,
			                            RT_MBUF_COUNT,&cbuf);
            if(status)
               return status;
         }
      }
   }

   /*******************************************************************
   *  Fill in message buffers for receive and transmit sub-units.
   *******************************************************************/
   mbuf.error_inj_id = 0;
   mbuf.enable = BT1553_INT_END_OF_MESS;   // Interrupt enables
   // For all subaddresses...
   for (ii = 0; ii<8; ii++)
   {  
      for (i=0; i<32; i++)
      {
         // For each data buffer...
         for (j=0; j<RT_MBUF_COUNT; j++)
         {
            for(jj=0;jj<32;jj++)
               mbuf.mess_data[jj] = (BT_U16BIT)(0xA000 + j);
            status = BusTools_RT_MessageWrite(cardnum,rt_addr[ii],i,0,j,&mbuf);
            status = BusTools_RT_MessageWrite(cardnum,rt_addr[ii],i,1,j,&mbuf);
         }
      }
   }

   /************************************************
   *  Setup RT address control block
   ************************************************/

   abuf.enable_a          = 1;    // Enable Bus A...Note that the API bits are
   abuf.enable_b          = 1;    // Enable Bus B... reversed from the HW bits.
   abuf.inhibit_term_flag = 0;
   abuf.status            = 0x0000;
   abuf.command           = 0;
   abuf.bit_word          = 0;

   for(i=0;i<8;i++)
   {
      status = BusTools_RT_AbufWrite(cardnum,rt_addr[i],&abuf);
   }

   return status;
}

int BusMonitor_Init(int cardnum)
{
   int status;
   BT_UINT actual;

   status = BusTools_BM_Init(cardnum,1,1);
   printf("BusTools_BM_Init status = %d\n",status);
 
   status = BusTools_BM_MessageAlloc(cardnum, 2, &actual ,0); 
   printf("BusTools_BM_MessageAlloc status = %d - actual =%d\n",status,actual);

   return status;
}


/****************************************************************************
*
*  Function:  setupThe_BC_IntFIFO
*
*  Description:
*     This routine setup the interrupt FIFO with the value needed for a 
*     interrupt on a BC message.  The interrupt is set for only the end-of
*     message.  
*
*     Any errors detected are reported to the user.  The user
*     has the option of terminating the test by striking any key.
*
****************************************************************************/
void setupThe_BC_IntFIFO(int cardnum)
{
   int rt,tr,sa,status;
   // Setup the FIFO structure for this board.
   memset(&sIntFIFO1, 0, sizeof(sIntFIFO1));
   sIntFIFO1.function     = bc_intFunction;
   sIntFIFO1.iPriority    = THREAD_PRIORITY_ABOVE_NORMAL;
   sIntFIFO1.dwMilliseconds = INFINITE;
   sIntFIFO1.iNotification  = 0;
   sIntFIFO1.FilterType     = EVENT_BC_MESSAGE;

   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO1.FilterMask[rt][tr][sa] =  0xffffffff;  // Enable all Word Counts on all RT/SA/TR combos
/*
 Use this if to get notified only on No response, late response of message error 
   sIntFIFO1.EventInit=USE_INTERRUPT_MASK;   
   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO1.EventMask[rt][tr][sa] = BT1553_INT_NO_RESP | BT1553_INT_ME_BIT //interrupt on ME and no-rsp
                                              | BT1553_INT_LATE_RESP; // interrupt on Late-response
*/
   // Call the register function to register and start the thread.
   status = BusTools_RegisterFunction(cardnum, &sIntFIFO1, 1);
   printf("BusTools_RegisterFunction status = %d\n",status);
}
