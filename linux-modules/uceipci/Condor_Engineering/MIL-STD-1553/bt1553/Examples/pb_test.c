/*===========================================================================*
 * FILE:                    P B _ T E S T . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 1995 - 2001 BY
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
 * Notes:       This program can be compiled for either MS-DOS, 16-bit Windows,
 *              Win32, Linux or VxWorks.  When compiling for Win32, use the 
 *              Borland C/C++ or Microsoft Visual C/C++ compiler.  When 
 *              compiling for VxWorks or Linux use the GNU C compiler.
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
#include <stdlib.h>
#include "busapi.h"


int main(int argc, char **argv)                     
{
   int status=0, pwFlag=0, device=0, channel=0;
   unsigned int cardnum=0;
   API_PLAYBACK pb_dat;


   if(argc >= 2)
      device = atoi(argv[1]);
   if(argc == 3)
      channel = (atoi(argv[2]))-1;

   pwFlag = 0x1; // Use software interrupts


   printf("Starting Test. \n\n");

   status = BusTools_API_OpenChannel(&cardnum,pwFlag,device,channel);
   printf("BusTools_API_InitExtended status = %d\n",status);
   if(status)
   {
      BusTools_API_Close(cardnum);
      return 0;
   }

   status = BusTools_Playback_Stop(cardnum);
   printf("BusTools_Playback_Stop status = %d\n",status);

   status = BusTools_BM_Init(cardnum,1,1);

   status = BusTools_SetInternalBus(cardnum,0);                  // Set up external bus
   printf("BusTools_SetInternalBus status = %d\n",status);
   do
   { 
      API_PLAYBACK_STATUS pb_stat;
      pb_dat.filterFlag = 0;
      pb_dat.messageStart = 0;
      pb_dat.messageStop = 0;
      // use "pb_test.bmd" if using BusTools/1553 API before v8.xx
      strcpy(pb_dat.fileName, "pb_test.bmdx");
      pb_dat.activeRT = 0xffffffff;
      pb_dat.status = &pb_stat;
      printf("Calling BusTools_Playback\n");
      status = BusTools_Playback(cardnum,pb_dat);
      if(status)
      {
         printf("playback failure status = %s\n",BusTools_StatusGetString(status));
         BusTools_API_Close(cardnum);
         printf("API Closed now we exit\n");
         return 1;
      }
      else
      {
         printf("Return from Playback\n");
      }

      while(pb_stat.playbackStatus & 0x5)
      {
         printf("| %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("/ %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("- %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("\\%d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("| %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("/ %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("- %d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
         printf("\\%d - %d     \r",pb_stat.recordsProcessed,pb_stat.playbackStatus);
      }

      printf("\nPlayback done %d messages sent status = %x\n",pb_stat.recordsProcessed,pb_stat.playbackStatus);

   }while(getchar()!='q');
   BusTools_API_Close(cardnum);

   return 0;
}
