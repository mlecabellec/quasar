/*===========================================================================*
 * FILE:                     T S T _ B C . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 1995 - 2018 BY
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
 *              application shows how to setup interrupts on a RT messages.
 *              This program uses BusTools_RegisterFunction and is only 
 *              applicable to Win32 and Linux hosts.
 *
 *===========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "busapi.h"

int BusController_Init(int,int);
int BusMonitor_Init(int);
int RemoteTerminal_Init(int);
void setupThe_BC_IntFIFO(int);
void setupThe_BM_IntFIFO(int);
void setupThe_RT_IntFIFO(int);

API_INT_FIFO  sIntFIFO1;     // Thread FIFO structure for BC
API_INT_FIFO  sIntFIFO2;     // Thread FIFO structure for RT
API_INT_FIFO  sIntFIFO3;     // Thread FIFO structure for BM

int main(int argc, char **argv)                     
{
   int i, status=0, channel=0;
   unsigned int cardnum=0, device=0, pwFlag=1;
   DeviceList dlist;

   if(argc >= 2)
      device = atoi(argv[1]);
   if(argc == 3)
      channel = atoi(argv[2]);

   printf("Starting Test for Device %d.  Hit Enter to stop\n\n",device);
   status = BusTools_API_Close(cardnum);

   printf("Initializing Test\n");

   status = BusTools_ListDevices(&dlist);
   for(i=0; i<dlist.num_devices; i++)
   {
     printf("Board Type %s ID %x is device %d\n",dlist.name[i],dlist.device_name[i],dlist.device_num[i]);
   } 

   status = BusTools_API_OpenChannel(&cardnum,pwFlag,device,channel);
   printf("BusTools_API_OpenChannel status = %d\n",status);

   status = BusTools_SetInternalBus(cardnum,0);                  // Set up internal bus
   printf("BusTools_SetInternalBus status = %d\n",status);

#ifdef IRIG_B
   status = BusTools_IRIG_Config(cardnum,IRIG_EXTERNAL,IRIG_OUT_DISABLE);
   printf("BusTools_IRIG_Config status IRIG_EXTERNAL IRIG_OUT_ENABLE = %d\n",status);

   status = BusTools_TimeTagMode( 0, API_TTD_IRIG, API_TTI_ZERO, API_TTM_IRIG, NULL, 0, 0, 0 );
   printf("BusTools_TimeTagMode status = %d\n",status);

   status = BusTools_IRIG_Calibration(cardnum,1);
   printf("BusTools_IRIG_Calibration status = %d\n",status); 

   status = BusTools_IRIG_Valid(cardnum);
   if(status == API_IRIG_NO_SIGNAL)
      printf("API_IRIG_NO_SIGNAL\n");
   else
	  printf("BusTools_IRIG_Valid status = %d\n",status);
#endif
 
   status = BusMonitor_Init(cardnum);
   status = BusController_Init(cardnum,0);
 
/* You can select one or all of these interrupt options by uncommenting the 3 lines below */
   setupThe_BC_IntFIFO(cardnum); //Set up BC interrupts using BusTools_RegisterFunction

   status = BusTools_BC_StartStop(cardnum,1);
   printf("BusTools_BC_StartStop status = %d\n",status);

   while(getc(stdin) != '\n');

   printf("main: closing API\n");

   status = BusTools_BC_StartStop(cardnum,0);
   printf("BusTools_BC_StartStop status = %d\n",status);

   status = BusTools_RegisterFunction(cardnum,&sIntFIFO1,0);
   printf("BusTools_RegisterFunction status = %d\n",status);

   status = BusTools_API_Close(cardnum);
   printf("BusTools_API_Close status = %d\n",status);

   return 0;
}

int BusController_Init(int cardnum,int sa_index)
{
   int status, messno;
   API_BC_MBUF bcmessage;

   status = BusTools_BC_Init(cardnum,0,BT1553_INT_END_OF_MESS,0,20,20,1000000,2);
   printf("BusTools_BC_Init status = %d\n",status);

   status = BusTools_BC_MessageAlloc(cardnum,50);
   printf("BusTools_BC_MessageAlloc status = %d\n",status); 

   messno = 0;
   memset((char*)&bcmessage,0,sizeof(bcmessage));    
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_MFRAME_BEG;
   bcmessage.mess_command1.rtaddr   = 2;
   bcmessage.mess_command1.subaddr  = 2;
   bcmessage.mess_command1.wcount   = 2;
   bcmessage.mess_command1.tran_rec = 0;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.
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
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.
 
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
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.

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
   bcmessage.mess_command1.wcount   = 5;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.

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
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.

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
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 12;
   bcmessage.mess_command1.wcount   = 6;
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;    // 8 microsecond inter-message gap.

   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   //printf("BusTools_BC_MessageWrite status = %d\n",status);

   /* Mode Code */
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 0;
   bcmessage.mess_command1.wcount   = 18;  
   bcmessage.mess_command1.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 20;    // 12 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);


   /* RT - RT Message */
   messno++;
   memset((char*)&bcmessage,0,sizeof(bcmessage));
   bcmessage.messno = messno;
   bcmessage.messno_next = (BT_U16BIT)(messno + 1);
   bcmessage.control = BC_CONTROL_MESSAGE;        // show as a message
   bcmessage.control |= BC_CONTROL_BUFFERA;       // use buffer A
   bcmessage.control |= BC_CONTROL_CHANNELA;   // Transmit on Channel A
   bcmessage.control |= BC_CONTROL_RTRTFORMAT;
   bcmessage.control |= BC_CONTROL_INTERRUPT;
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.mess_command1.rtaddr   = 1;
   bcmessage.mess_command1.subaddr  = 10;
   bcmessage.mess_command1.wcount   = 6;
   bcmessage.mess_command1.tran_rec = 0;

   bcmessage.mess_command2.rtaddr   = 3;
   bcmessage.mess_command2.subaddr  = 6;
   bcmessage.mess_command2.wcount   = 6;
   bcmessage.mess_command2.tran_rec = 1;
   bcmessage.errorid = 0;     // Default error injection buffer (no errors)
   bcmessage.gap_time = 200;    // 12 microsecond inter-message gap.
 
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage); 
   printf("BusTools_BC_MessageWrite status = %d\n",status);

   messno++;
   bcmessage.messno = messno;
   bcmessage.messno_next = 0x0;
   bcmessage.messno_branch = 0;
   bcmessage.control |= BC_CONTROL_MFRAME_END;
   bcmessage.control = BC_CONTROL_NOP;        // Do it again and again.
   status = BusTools_BC_MessageWrite(cardnum,messno,&bcmessage);
   printf("BusTools_BC_MessageWrite status = %d\n",status);

#ifdef ERROR_INJECTION
   {
      API_EIBUF ebuf;
      int i;

      ebuf.buftype = EI_BC_REC;

      ebuf.error[0].etype = EI_BITCOUNT;
      ebuf.error[0].edata = 16;

      for ( i = 1; i < EI_COUNT; i++ )
      {
	     ebuf.error[i].etype = EI_NONE;
	     ebuf.error[i].edata = 0;
      }

      status = BusTools_EI_EbufWrite(cardnum, 1, &ebuf);
   }
#endif

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

int BusMonitor_Init(int cardnum)
{
   int status;
   BT_UINT actual;

   status = BusTools_BM_Init(cardnum,1,1);
   printf("BusTools_BM_Init status = %d\n",status);

   status = BusTools_BM_MessageAlloc(cardnum, 3, &actual, 0xffffffff ); //interrupt on END_OF_MESS
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
   sIntFIFO1.iPriority    = THREAD_PRIORITY_CRITICAL;
   sIntFIFO1.dwMilliseconds = INFINITE;
   sIntFIFO1.iNotification  = 0;
   sIntFIFO1.FilterType     = EVENT_BC_MESSAGE;

   for ( rt=0; rt < 32; rt++ )
      for (tr = 0; tr < 2; tr++ )
         for (sa = 0; sa < 32; sa++ )
            sIntFIFO1.FilterMask[rt][tr][sa] =  0xffffffff;  // Enable all Word Counts on all RT/SA/TR combos

/* Use this if to get notified only on No response, late response of message error 
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
