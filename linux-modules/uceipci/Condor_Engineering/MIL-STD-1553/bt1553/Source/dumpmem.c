/*============================================================================*
 * FILE:                  D U M P M E M . C
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
 *             This file contains the API routines for memory dump and trace
 *             dump diagnostic functions.
 *
 * USER ENTRY POINTS: 
 *    BusTools_DumpMemory  - Function dumps all board memory to a file.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    AddTrace             - Function adds a entry into the trace buffer.
 *
 * INTERNAL ROUTINES:
 *    API_Names            - Helper; returns BusTools function name string.
 *    DumpTrace            - Helper for BusTools_DumpMemory; dumps trace buffer.
 *    PerformanceCounter   - Returns the elapsed time in microseconds.
 *
 *===========================================================================*/

/* $Revision:  8.22 Release $
   Date        Revision
  --------     ---------------------------------------------------------------
  06/10/1999   Added BusTools_DumpMemory function for release.V3.11.ajh
  11/10/1999   Added RegisterFunction FIFO dump.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  03/15/2002   Add support for different O/S version V4.48.rhc
  02/24/2003   Add QPCI-1553 support
  02/18/2004   PCCard-D1553 Supoprt
  04/13/2011   Allow memory dumps to screen for processors with file systems  
  04/18/2011   Modified BusTools_DumpMemory to use CEI_GET_ENV_VAR. bch
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API 
  09/07/2016   Added support for new hardware MPCIE
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

#if defined(ADD_TRACE)

#define MAX_TRACE (2*4096)
#if MAX_TRACE & (MAX_TRACE-1)
#error Trace buffer is not a power of two in length!
#endif
typedef struct trace_buffer
{
   BT_U16BIT cardnum;         // card number
   BT_U16BIT nFunction;       // function number to log
   BT_INT    nParam1;         // first parameter to log
   BT_INT    nParam2;         // second parameter to log
   BT_INT    nParam3;         // third parameter to log
   BT_INT    nParam4;         // fourth parameter to log
   BT_INT    nParam5;         // fourth parameter to log
   __int64   time;            // time trace entry was logged.
} TRACE_BUFFER;

static TRACE_BUFFER trace[MAX_TRACE];
static int trace_pointer = 0;
static int trace_wrap_around = 0;
static int time_correction;           // Time correction value

/****************************************************************************
*
*  PROCEDURE NAME -    API_Names()
*
*  FUNCTION
*       This procedure returns the string value of an API name index.
*
****************************************************************************/
static char * API_Names(int function)
{
   /**********************************************************************
   *  Return the symbolic name for the API function number
   **********************************************************************/
   switch (function)
   {
   case NBUSTOOLS_BC_MESSAGEREAD:        return "BC_MessageRead     ";
   case NBUSTOOLS_BC_START:              return "BC_Start           ";
   case NBUSTOOLS_BC_STARTSTOP:          return "BC_StartStop       ";
   case NBUSTOOLS_BM_MESSAGEREAD:        return "BM_MessageRead     ";
   case BM_MSGREADBLOCK:                 return "BM_MsgReadBlock    ";
   case NBUSTOOLS_BM_STARTSTOP:          return "BM_StartStop       ";
   case NBUSTOOLS_RT_MESSAGEGETID:       return "RT_MessageGetid    ";
   case NBUSTOOLS_RT_MESSAGEREAD:        return "RT_MessageRead     ";
   case NBUSTOOLS_RT_STARTSTOP:          return "RT_StartStop       ";
   case NBM_MESSAGECONVERT:              return "BM_MessageConvert  ";
   case NBM_TRIGGER_OCCUR:               return "BM Trigger Occured ";
   case NCALLUUSERTHREAD:                return "Calling User Thread";
   case NINTQUEUEENTRY:                  return "Interrupt Queuq Ent";
   case NSIGNALUUSERTHREAD:              return "SignalUserThread   ";
   case NTIME_TAG_CLEARFLAG:             return "Time Tag Clear Flag";
   case NTIME_TAG_INTERRUPT:             return "Time Tag Interrupt ";
   case NVBTNOTIFY:                      return "vbtNotify          ";
   case NVBTSETUP:                       return "vbtSetup           ";
   case NVBTSHUTDOWN:                    return "vbtShutDown        ";
   case NBUS_LOADING_FILTER:             return "BusLoadingFilter   ";
   default:                              return ">>Unknown function<";
   }
}

/****************************************************************************
*
*  PROCEDURE NAME -    AddTrace()
*
*  FUNCTION
*       This procedure logs the sequence of procedure calls made by a
*       user's program into the BusTools/1553-API.  This log is dumped
*       by a call to BusTools_DumpMemory to a user-specified file.
*
****************************************************************************/
void AddTrace(
   BT_UINT cardnum,         // (i) card number (0 based)
   BT_INT  nFunction,       // (i) function number to log
   BT_INT  nParam1,         // (i) first parameter to log
   BT_INT  nParam2,         // (i) second parameter to log
   BT_INT  nParam3,         // (i) third parameter to log
   BT_INT  nParam4,         // (i) fourth parameter to log
   BT_INT  nParam5)         // (i) fifth parameter to log
{

   if ( DumpTraceMask &  (1 << nFunction) )
      return;    // Logging of this function is disabled.

   trace[trace_pointer].cardnum   = (BT_U16BIT)cardnum;
   trace[trace_pointer].nFunction = (BT_U16BIT)nFunction;
   trace[trace_pointer].nParam1   = nParam1;
   trace[trace_pointer].nParam2   = nParam2;
   trace[trace_pointer].nParam3   = nParam3;
   trace[trace_pointer].nParam4   = nParam4;
   trace[trace_pointer].nParam5   = nParam5;
   trace[trace_pointer].time      = PerformanceCounter();
   trace_pointer++;
   if ( trace_pointer >= MAX_TRACE )
   {
      trace_pointer = 0;
      trace_wrap_around = 1;
   }
}


/****************************************************************************
*
*  PROCEDURE NAME -    DumpTrace()
*
*  FUNCTION
*       This procedure outputs the trace buffer to a file.  There are two
*       cases:
*       ->Trace buffer has not filled.
*         -Dump from entry zero to entry trace_pointer-1.
*       ->Trace buffer has filled up and wrapped around.
*         -Dump from entry trace_pointer to end,
*          then dump from zero to trace_pointer-1
*
****************************************************************************/
static void DumpTrace(
   FILE         *hfMemFile) // (i) file handle to write trace buffer to.
{
   int     Oldest, last;             // Trace buffer pointers
   //char    time_string[30];           // int 64 value in ASCII

   if ( trace_wrap_around )
   {
      Oldest = trace_pointer;          // Oldest entry in the trace buffer
      last    = trace_pointer-1;
   }
   else
   {
      Oldest = 0;                      // Oldest entry in the trace buffer
      last    = trace_pointer-1;
   }
   if ( last < 0 )
      last = MAX_TRACE - 1;

   // Now dump the trace entries to the debug output file.
//printf(hfMemFile, "\nTrace Buffer Output\n"
//       " Performance Counter Frequency = %dHz Time correction = %d\n\n",
//       (int)liFreq, time_correction);

   while (1)
   {
      fprintf(hfMemFile,
              "%4d-%s(%d) @ %I64d/%6d us %8.8X(%6d) %8.8X %8.8X %8.8X %8.8X\n",
              Oldest, API_Names(trace[Oldest].nFunction),
              trace[Oldest].cardnum, trace[Oldest].time,
              (int)(trace[Oldest].time - trace[(MAX_TRACE-1)&(Oldest-1)].time),
              trace[Oldest].nParam1, trace[Oldest].nParam1,
              trace[Oldest].nParam2,
              trace[Oldest].nParam3,
              trace[Oldest].nParam4,
              trace[Oldest].nParam5);
      if ( Oldest == last ) break;
      Oldest++;
      if ( Oldest >= MAX_TRACE ) Oldest = 0;
   }
}
#else
#define DumpTrace(p1)
#endif // #if defined(ADD_TRACE)

static char* hwRegNames[] = 
{
//0 
    "r0      ", "r1      ", "r2      ", "r3      ", "r4      ", "r5      ", "r6      ", "r7      ", 
    "r8      ", "r9      ", "rA      ", "rB      ", "rC      ", "rD      ", "rE      ", "rF      ", 
//10
    "LPU Vers", "LPU Bld ", "Mic Vers", "Mic Bld ", "Heart   ", "Reserved", "HW Cntrl", "Reserved", 
    "1553A En", "NRLR    ", "r1A     ", "r1B     ", "r1C     ", "r1D     ", "RespTime", "r1F     ",
//20
    "r20     ", "r21     ", "r22     ", "r23     ", "r24     ", "r25     ", "r26     ", "r27     ", 
    "r28     ", "r29     ", "r2A     ", "r2B     ", "r2C     ", "r2D     ", "r2E     ", "r2F     ", 
//30
    "FW Cntrl", "r31     ", "r32     ", "r33     ", "r34     ", "r35     ", "r36     ", "r37     ", 
    "r38     ", "r39     ", "*IQStart", "*IQ End ", "*IQ Head", "Int Vctr", "r3E     ", "r3F     ", 
//40   
    "*BCInitW", "BC IntEn", "BC MrFrm", "r43     ", "RtryEnbl", "RtryEnbl", "RtryList", "r47     ",
    "BusSwOff", "r49     ", "*HiPriAp", "*LoPriAp", "r4C     ", "BC Flags", "r4E     ", "r4F     ",
//50
    "r50     ", "r51     ", "r52     ", "r53     ", "r54     ", "r55     ", "r56     ", "r57     ",
    "r58     ", "r59     ", "r5A     ", "r5B     ", "r5C     ", "r5D     ", "r5E     ", "r5F     ", 
//60
    "HWRTAddr", "Addr SRT", "RTA Enbl", "RTB Enbl", "RTFilter", "r65     ", "*RTInitW", "r67     ", 
    "r68     ", "r69     ", "r6A     ", "r6B     ", "r6C     ", "r6D     ", "r6E     ", "r6F     ", 
//70
    "r70     ", "r71     ", "r72     ", "r73     ", "r74     ", "r75     ", "r76     ", "r77     ", 
    "r78     ", "r79     ", "r7A     ", "r7B     ", "r7C     ", "r7D     ", "r7E     ", "r7F     ", 
//80
    "RTSWOp 0", "RTBLCW 0", "RTSWOp 1", "RTBLCW 1", "RTSWOp 2", "RTBLCW 2", "RTSWOp 3", "RTBLCW 3", 
    "RTSWOp 4", "RTBLCW 4", "RTSWOp 5", "RTBLCW 5", "RTSWOp 6", "RTBLCW 6", "RTSWOp 7", "RTBLCW 7",  
//90                                                                                                                                                                                                
    "RTSWOp 8", "RTBLCW 8", "RTSWOp 9", "RTBLCW 9", "RTSWOp10", "RTBLCW10", "RTSWOp11", "RTBLCW11", 
    "RTSWOp12", "RTBLCW12", "RTSWOp13", "RTBLCW13", "RTSWOp14", "RTBLCW14", "RTSWOp15", "RTBLCW15", 
//A0                                                                                                                                                                                                
    "RTSWOp16", "RTBLCW16", "RTSWOp17", "RTBLCW17", "RTSWOp18", "RTBLCW18", "RTSWOp19", "RTBLCW19", 
    "RTSWOp20", "RTBLCW20", "RTSWOp21", "RTBLCW21", "RTSWOp22", "RTBLCW22", "RTSWOp23", "RTBLCW23",
//B0
    "RTSWOp24", "RTBLCW24", "RTSWOp25", "RTBLCW25", "RTSWOp26", "RTBLCW26", "RTSWOp27", "RTBLCW27", 
    "RTSWOp28", "RTBLCW28", "RTSWOp29", "RTBLCW29", "RTSWOp30", "RTBLCW30", "RTSWOp31", "RTBLCW31", 
//C0   
    "*BMStBuf", "*BMEndBf", "*BMHead ", "*BMTail ", "BMFltBas", "*BMTrgBf", "BM IntEn", "rC7     ", 
    "rC8     ", "rC9     ", "rCA     ", "rCB     ", "rCC     ", "rCD     ", "rCE     ", "rCF     ", 
//D0
    "rD0     ", "BMOVFLCt", "BMContrl", "rD3     ", "PBCntrSt", "rD5     ", "rD6     ", "rD7     ", 
    "*PB Strt", "*PB End ", "*PB Tail", "*PB Head", "PBTWdcnt", "PBintsta", "PBIThrCn", "*PBcurwd", 
//E0
    "rE0     ", "rE1     ", "rE2     ", "rE3     ", "rE4     ", "rE5     ", "rE6     ", "rE7     ",
    "rE8     ", "rE9     ", "rEA     ", "rEB     ", "rEC     ", "rED     ", "rEE     ", "rEF     ", 
//F0
    "rF0     ", "rF1     ", "rF2     ", "rF3     ", "rF4     ", "rF5     ", "rF6     ", "rF7     ",
    "rF8     ", "rF9     ", "rFA     ", "rFB     ", "rFC     ", "rFD     ", "rFE     ", "rFF     ", 
};

static char* globalRegNames[] = 
{
//0 
    "Brd Type", "Brd Spec", "RTOffs14", "RTOffs58", "Options1", "Options2", "Options3", "Options4", 
    "Options5", "Options6", "Options7", "Options8", "SerialNo", "rD      ", "rE      ", "rF      ",
//10 
    "r10     ", "r11     ", "r12     ", "r13     ", "r14     ", "r15     ", "r16     ", "r17     ",
    "r18     ", "r19     ", "r1A     ", "r1B     ", "r1C     ", "r1D     ", "r1E     ", "r1F     ",
//20
    "FPGA Rev", "Int Stat", "RAMStart", "RAM Size", "LED Ctrl", "TempS Rd", "TempS Wr", "Board SW", 
    "r28     ", "r29     ", "r2A     ", "r2B     ", "r2C     ", "r2D     ", "r2E     ", "r2F     ",
//30
    "DiscrOut", "Discr IN", "RS485Ctrl", "RS485Da", "EXT RT  ", "r36     ", "DAC Ctrl", "r37     ",
    "r38     ", "r39     ", "r3A     ", "r3B     ", "r3C     ", "r3D     ", "r3E     ", "r3F     ",
//40
    "r40     ", "r41     ", "r42     ", "r43     ", "r44     ", "r45     ", "r46     ", "r47     ",
    "r48     ", "r49     ", "r4A     ", "r4B     ", "r4C     ", "r4D     ", "r4E     ", "r4F     ",
//50
    "IRIG Cnt", "IRIG TOY", "r52     ", "r53     ", "r54     ", "r55     ", "r56     ", "r57     ", 
    "r58     ", "r59     ", "r5A     ", "r5B     ", "r5C     ", "r5D     ", "r5E     ", "r5F     ", 
};

/****************************************************************************
*
*   PROCEDURE NAME - DumpUserAlloc () 
*
*   FUNCTION
*       This function dumps the User allocated memory block
*
*   RETURNS
*       nothing
*
*****************************************************************************/
void DumpUserAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT   i = 0, j = 0;                // Read 16+ words per line
          
   
   BT_USER_ALLOC_COUNT_ENTRY *tempaddr = NULL;
   BT_U32BIT linenum = 1;
   BT_U32BIT entryindex = bt_user_alloc_entry_index[cardnum];
   
   fprintf(hfMemFile, "\n\nUser Allocated Memory:\n\n");
   fprintf(hfMemFile, "Address   Bytes   Data Contents\n");
   
   for ( j = 0; j < entryindex; j++)
   {
	  tempaddr = &(bt_user_alloc_count[cardnum][j]);
	  
	  //fprintf(hfMemFile, "%08X: %4d", bt_PageAddr[cardnum][RAM_PAGE]+tempaddr->addr, tempaddr->bcount);
	  fprintf(hfMemFile, "%08X: %4d", RAM_ADDR(cardnum,tempaddr->addr), tempaddr->bcount);
	  
      for (i = 0; i < tempaddr->bcount/4; i++ )
	  {
		  if (i == 0)
		  {
			  fprintf(hfMemFile, "    %08X", ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+tempaddr->addr))[i]);
		      linenum = 1;
		  }
		  else if (i - 6*linenum == 0)
		  {
			  linenum++;
			  fprintf(hfMemFile, "\n                  %08X", ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+tempaddr->addr))[i]);
		  }
		  else
		      fprintf(hfMemFile, " %08X ", ((BT_U32BIT *)(bt_PageAddr[cardnum][RAM_PAGE]+tempaddr->addr))[i]);
		
	  }
	 
	  fprintf(hfMemFile, "\n");
   }
   fprintf(hfMemFile, "\n");
}
/****************************************************************************
*
*   PROCEDURE NAME - DumpHWregs ()
*
*   FUNCTION
*       This function dumps the Hardware Register block
*
*   RETURNS
*       nothing
*
*****************************************************************************/
void DumpHWregs (
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    data[0x100];               // Read 16+ words per line
   int     j;              // Loop indexes

   fprintf(hfMemFile, "\n\nHardware Registers:\n");

   BusTools_DumpHWRegisters(cardnum,data);
   for ( j = 0; j < 0x100; j+=8)
   {
      fprintf(hfMemFile, "%04X: %s %s %s %s %s %s %s %s", 
       j, hwRegNames[j],   hwRegNames[j+1], hwRegNames[j+2], hwRegNames[j+3],
          hwRegNames[j+4], hwRegNames[j+5], hwRegNames[j+6], hwRegNames[j+7]);
      fprintf(hfMemFile, "\n");
      fprintf(hfMemFile, "      %08X %08X %08X %08X %08X %08X %08X %08X", 
             data[j],   data[j+1], data[j+2], data[j+3],
             data[j+4], data[j+5], data[j+6], data[j+7]);
       fprintf(hfMemFile, "\n");
   }
   fprintf(hfMemFile, "\n");
}

/****************************************************************************
*
*   PROCEDURE NAME - V6DumpGlobalRegs ()
*
*   FUNCTION
*       This function dumps the Global Register area
*
*   RETURNS
*       nothing
*
*****************************************************************************/
void V6DumpGlobalRegs (
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    data[0x100];               // Read 16+ words per line
   int     j;              // Loop indexes

   fprintf(hfMemFile, "\n\nGlobal Registers:\n");
   BusTools_MemoryRead2(cardnum, HIF, 0x0, 0x60, data);

   for ( j = 0; j < 0x60; j+=8)
   {
      fprintf(hfMemFile, "%04X: %s %s %s %s %s %s %s %s", 
       j, globalRegNames[j],   globalRegNames[j+1], globalRegNames[j+2], globalRegNames[j+3],
          globalRegNames[j+4], globalRegNames[j+5], globalRegNames[j+6], globalRegNames[j+7]);
      fprintf(hfMemFile, "\n");
      fprintf(hfMemFile, "      %08X %08X %08X %08X %08X %08X %08X %08X", 
             data[j],   data[j+1], data[j+2], data[j+3],
             data[j+4], data[j+5], data[j+6], data[j+7]);
       fprintf(hfMemFile, "\n");
   }
   fprintf(hfMemFile, "\n");
}

/****************************************************************************
*
*   PROCEDURE NAME - V6DumpTriggerRegs ()
*
*   FUNCTION
*       This function dumps the Global Register area
*
*   RETURNS
*       nothing
*
*****************************************************************************/
void V6DumpTriggerRegs (
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile)       // (i) handle of output file
{
   BT_U32BIT    externalTrigIn, externalTrigOut;
   BT_U32BIT    timetagControl;
   BT_U32BIT    timetagLoadLow;
   BT_U32BIT    timetagLoadHigh;
   BT_U32BIT    timetagIncrement;
   BT_U32BIT    timetagLatch;
   BT_U32BIT    timetagReadLow;
   BT_U32BIT    timetagReadHigh;

   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_CONTROL,     1, &timetagControl);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_LOAD_LOW,    1, &timetagLoadLow);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_LOAD_HIGH,   1, &timetagLoadHigh);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_INCREMENT,   1, &timetagIncrement);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_LATCH,       1, &timetagLatch);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_READ_LOW,    1, &timetagReadLow);
   BusTools_MemoryRead2 (cardnum, TTREG, TTREG_READ_HIGH,   1, &timetagReadHigh);

   BusTools_MemoryRead2 (cardnum, TRIGREG,   EXT_TRGIN_CTRL, 1, &externalTrigIn);
   BusTools_MemoryRead2 (cardnum, TRIGREG,   EXT_TRGOUT_CTRL, 1, &externalTrigOut);

   fprintf(hfMemFile, "\nTimetag Control Registers:\n");   
   fprintf(hfMemFile, "Timetag Control: %08X. ", timetagControl);
   fprintf(hfMemFile, "Timetag Load Low: %08X. ", timetagLoadLow);
   fprintf(hfMemFile, "Timetag Load High: %08X. ", timetagLoadHigh);
   fprintf(hfMemFile, "Timetag Increment: %08X.\n", timetagIncrement);
   fprintf(hfMemFile, "Timetag Latch: %08X. ", timetagLatch);
   fprintf(hfMemFile, "Timetag Read Low: %08X. ", timetagReadLow);
   fprintf(hfMemFile, "Timetag Read High: %08X. ", timetagReadHigh);

   fprintf(hfMemFile, "\nTrigger Input/Output Registers:\n");   
   fprintf(hfMemFile, "Trigger Input  %08X. ", externalTrigIn);
   fprintf(hfMemFile, "Trigger Output %08X.\n", externalTrigOut);
   fprintf(hfMemFile, "\n");

}


/****************************************************************************
*
*  PROCEDURE NAME -    BusTools_DumpMemoryV6()
*
*  FUNCTION
*       This procedure outputs a dump of all requested data areas.  Regions
*       whose bit is set in region_mask are dumped to the specified 8.3 file.
*       region_mask == 1 dumps the trace buffer, successive bits dump the
*       regions mapped by BusTools_GetAddr().
*
*  RETURNS
*       API_BUSTOOLS_NO_FILE
*       API_BUSTOOLS_BADCARDNUM
*       API_SUCCESS
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_DumpMemoryV6(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT region_mask,   // (i) mask of memory regions to dump (bit 1 = region 1)       
   char * file_name,        // (i) pointer to name of output file to create             
   char * heading)          // (i) pointer to message to display in file                      
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j;                        // Loop index
   BT_UINT      num_per_line;             // Number of values per line
   BT_UINT      block_id;                 // Loop index
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U32BIT    data[0x100];               // Read 16+ words per line
   FILE         *hfMemFile;               // Error file handle is not open.
   time_t       tdate;                    // Date this test was run

#if defined(FILE_SYSTEM)
   BT_INT status=0;
   char dump_file[512]="";

   memset(dump_file, 0, sizeof(dump_file));
   if((status = CEI_GET_FILE_PATH(file_name, dump_file)) != BTD_OK)
     return status;

   hfMemFile = fopen(dump_file, "w+t");   // Write+Text mode.
   if(hfMemFile == NULL) {
     // Just return a code if the output file could not be created.V4.25.ajh
     return API_BUSTOOLS_NO_FILE;        // File could not be created.
   }
#else //FILE_SYSTEM
   hfMemFile = stdout;
#endif

   /******************************************************************
   *  Check for legal call
   *******************************************************************/
   if (cardnum >= MAX_BTA)
   {
      fprintf(hfMemFile, "Bad card number specified: %d\n", cardnum);
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_BADCARDNUM;
   }

   // If no regions were requested, dump them all
   if ( !region_mask ) region_mask = 0xFFFFFFFF;

   // Log the user supplied message
   fprintf(hfMemFile, "%s\n\n", heading);

   // Log the board type, etc.
   if ( CurrentCardType[cardnum] == QPCI1553 )
      fprintf(hfMemFile, " QPCI-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCX1553 )
      fprintf(hfMemFile, " QPCX-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCX1553 )
      fprintf(hfMemFile, " QPCX-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == Q1041553P )
      fprintf(hfMemFile, " Q104-1553P WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QCP1553 )
      fprintf(hfMemFile, " QCP-1553 WCS V%d, FPGA V%x, Channel%d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15EC )
      fprintf(hfMemFile, " R15-EC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15AMC )
      fprintf(hfMemFile, " R15-AMC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15XMC2 )
      fprintf(hfMemFile, " RXMC2-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == LPCIE1553 )
      fprintf(hfMemFile, " LPCIE-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == MPCIE1553 )
      fprintf(hfMemFile, " MPCIE-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RPCIe1553 )
      fprintf(hfMemFile, " RPCIe-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPM1553 )
      fprintf(hfMemFile, " QPM-1553/QPMC_1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RXMC1553 )
      fprintf(hfMemFile, " RXMC-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15PMC )
      fprintf(hfMemFile, " R15-PMC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RAR15XMCXT )
      fprintf(hfMemFile, " RAR15-XMC-XT/IT/FIO WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QVME1553/RQVME2 )
      fprintf(hfMemFile, " QVME-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15USB )
   {
      if (CurrentUSBCardType[cardnum] == R15USBMON)
      {
         fprintf(hfMemFile, " R15-USB-MON WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
  
      }
      else
      {
         fprintf(hfMemFile, " R15-USB WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
  
      }
   }
   // Print the API version and the firmware version.
   fprintf(hfMemFile, " API Ver = %s/%s\n", API_VER, API_TYPE);
   fprintf(hfMemFile, " Build Options = %s/%s\n", BUILD_OPTIONS, BUILD_OPTIONS_INT); // V4.20

   // Time tag the output file.
   tdate = time(NULL);
   fprintf(hfMemFile, " Memory dump at %s\n\n", ctime(&tdate));  // V4.30

   fprintf(hfMemFile, " Init:bt=%d,inuse=%d,int-enable=%d,Polling-interval=%d, cardnum=%d\n"
                      " Init:bc=%d,bm=%d,rt=%d Running bc=%d,bm=%d,rt=%d HW Interrupts=%d\n",
           bt_inited[cardnum],  bt_inuse[cardnum], bt_interrupt_enable[cardnum],
           api_polling_interval, cardnum,
           bc_inited[cardnum],  bm_inited[cardnum],  rt_inited[cardnum],
           bc_running[cardnum], bm_running[cardnum], rt_running[cardnum],
           hw_int_enable[cardnum]);

   // Memory management pointers.  Output as WORD offsets on the board...
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Counts:\n");
   fprintf(hfMemFile, " bc_mblock_count       - %d\n",bc_mblock_count[cardnum]);
   fprintf(hfMemFile, " bm_count              - %d\n\n",bm_count[cardnum]);
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Addresses:\n");
   fprintf(hfMemFile, "                         (Byte Offsets)\n");
   fprintf(hfMemFile, " btmem_bc              - %5.5X\n",RAM_ADDR(cardnum,btmem_bc[cardnum]));
   fprintf(hfMemFile, " btmem_bc_next         - %5.5X\n",RAM_ADDR(cardnum,btmem_bc_next[cardnum]));
   fprintf(hfMemFile, " btmem_bm_cbuf         - %5.5X\n",RAM_ADDR(cardnum,btmem_bm_cbuf[cardnum]));
   fprintf(hfMemFile, " btmem_bm_cbuf_next    - %5.5X\n",RAM_ADDR(cardnum,btmem_bm_cbuf_next[cardnum]));
   fprintf(hfMemFile, " btmem_bm_mbuf         - %5.5X\n",RAM_ADDR(cardnum,btmem_bm_mbuf[cardnum]));
   fprintf(hfMemFile, " btmem_bm_mbuf_next    - %5.5X\n",RAM_ADDR(cardnum,btmem_bm_mbuf_next[cardnum]));
   fprintf(hfMemFile, " btmem_tail1           - %5.5X\n",RAM_ADDR(cardnum,btmem_tail1[cardnum]));
   fprintf(hfMemFile, " btmem_tail2           - %5.5X\n",RAM_ADDR(cardnum,btmem_tail2[cardnum]));
   fprintf(hfMemFile, " btmem_rt_top_avail    - %5.5X\n",RAM_ADDR(cardnum,btmem_rt_top_avail[cardnum])); 
   fprintf(hfMemFile, " btmem_pci1553_next    - %5.5X\n",RAM_ADDR(cardnum,btmem_pci1553_next[cardnum]));
   fprintf(hfMemFile, " btmem_pci1553_rt_mbuf - %5.5X\n\n",RAM_ADDR(cardnum,btmem_pci1553_rt_mbuf[cardnum]));

   // Board memory pointers.
   fprintf(hfMemFile,"                              RAM / HWREG / RAMREG / HIF\n");
   fprintf(hfMemFile, " bt_PageAddr[cardnum][0..6] = %p / %p / %p / %p\n IO-Base = %p\n",
           bt_PageAddr[cardnum][RAM_PAGE], bt_PageAddr[cardnum][HWREG_PAGE],
           bt_PageAddr[cardnum][TTREG_PAGE], bt_PageAddr[cardnum][CSC_REG_PAGE], bt_iobase[cardnum]);

   // Dump the stuff we can, return here if the board has not been init'ed.V4.09.ajh
   if (bt_inited[cardnum] == 0)  // V4.09.ajh
   {
      fprintf(hfMemFile, "API has not been initialized!!!\n");
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_NOTINITED;
   }

   // Add the following settings to the dump...
   // BusTools_SetInternalBus   - Sets the flag for external or internal bus
   // BusTools_SetOptions       - Sets Illegal command, Reset Timetag options
   // BusTools_SetVoltage       - Sets the voltage hardware register
   fprintf(hfMemFile, "\n SA 31 is Mode Code=%d, Broadcast Enabled=%d\n\n",
           rt_sa31_mode_code[cardnum], rt_bcst_enabled[cardnum]);

   // Dump the time tag information.
   {
      BT1553_TIME ctime;
      char outbuf[80];

      BusTools_TimeTagRead(cardnum, &ctime);
      BusTools_TimeGetString(&ctime,outbuf);
      fprintf(hfMemFile, " Time Tag Register: top usec = %x  microseconds = %x\n",ctime.topuseconds,ctime.microseconds);
      fprintf(hfMemFile, " Display Time = %s\n\n",outbuf);
   }

   DumpTimeTag(cardnum, hfMemFile);

   // Dump the local copies of the hardware registers and RAM registers.

   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      if ((block_id == GETADDR_HWREG) || (block_id == GETADDR_RAMREG) || (block_id == GETADDR_RTADDRESS) || (block_id == GETADDR_DIFF_IO))
          continue;
      BusTools_GetAddr(cardnum, block_id, &first, &last);
      fprintf(hfMemFile, "%s Start %8.8X End %8.8X (Byte Offsets)\n",
              BusTools_GetAddrName(block_id), RAM_ADDR(cardnum,first), RAM_ADDR(cardnum,last));
   }

   // Dump the global and hardware registers  
   V6DumpGlobalRegs (cardnum, hfMemFile);

   // Dump the trigger registers 
   V6DumpTriggerRegs (cardnum, hfMemFile); 
   
   // Dump the hardware registers
   DumpHWregs (cardnum, hfMemFile);

   // Dump all of the BusTools_RegisterFunction() FIFO's.
   DumpRegisterFIFO(cardnum, hfMemFile);

   // Dump the RT message block pointers
   DumpRTptrs(cardnum, hfMemFile);

   // Dump the specified memory regions to the specified file.
   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      /* This is where the specified buffers live */
      BusTools_GetAddr(cardnum, block_id, &first, &last);
         num_per_line = 12;
      if ( block_id == GETADDR_IQ )
          num_per_line = 12;
      if ( block_id == GETADDR_RTCBUF_DEF)
          num_per_line = 8;
      if ( block_id == GETADDR_RTDATA)  
          num_per_line = 8;                 
      if ( block_id == GETADDR_RTADDRESS)
          num_per_line = 12;
      if (block_id == GETADDR_RTFILTER)
          num_per_line = 16;  

      // If region was not requested, don't dump it.
      if ( (region_mask & (1<<block_id)) == 0 )
         continue;
      if ( block_id == GETADDR_BCMESS )
      {
         if (playback_is_running[cardnum]==0)
             V6DumpBCmsg(cardnum, hfMemFile);         // Dump the BC msg area.
      }
      else if ( block_id == GETADDR_RTCBUF_BRO)
         V6DumpRTCBufBroadcast (cardnum, hfMemFile);
      else if ( block_id == GETADDR_BMMESSAGE )
         V6DumpBMmsg(cardnum, hfMemFile);         // Dump the BM msg area.
      else if ( block_id == GETADDR_BMFILTER )
         DumpBMflt(cardnum, hfMemFile);           // Dump the BM filter area.
      else if ( block_id == GETADDR_PCI_RTDATA)
         V6DumpRTbufs(cardnum, hfMemFile);        // Dump the RT message list to output file
      else if ( block_id == GETADDR_RTMBUF_DEF)
         V6DumpRTDefaultBufs(cardnum, hfMemFile);
      else if ((block_id == GETADDR_HWREG) || (block_id == GETADDR_RAMREG) || (block_id == GETADDR_RTADDRESS) || (block_id == GETADDR_DIFF_IO))
      {
          data[0] = 0;  // print nothing
      }
      else
      {
         fprintf(hfMemFile, "\n%s.Start %8.8X End %8.8X (Byte Offset)\n",
            BusTools_GetAddrName(block_id), RAM_ADDR(cardnum,first), RAM_ADDR(cardnum,last));

         // Dump all other requested regions to the file.
         for ( i = first; i < last; ) // i += num_per_line )
         {
            // Read the current line of num_per_line data words from the board.
            BusTools_MemoryRead2(cardnum, RAM32, i, num_per_line*4, data);
            fprintf(hfMemFile, "%8.8X:", RAM_ADDR(cardnum,i));
            for ( j = 0; j < num_per_line && i <= last; j++ ) 
            {
               fprintf(hfMemFile, " %8.8X", data[j]);
               i+=4;
            }
            fprintf(hfMemFile, "\n");
         }
      }
   }
   // Lastly dump the trace buffer, if available, and if any.
   DumpTrace(hfMemFile);

   //dump user alloc entries
   DumpUserAlloc(cardnum,hfMemFile);      

   fprintf(hfMemFile, "\n-- End --\n");

   fclose(hfMemFile);         //  Close and save the debug data.
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -    BusTools_DumpMemory()
*
*  FUNCTION
*       This procedure outputs a dump of all requested data areas.  Regions
*       whose bit is set in region_mask are dumped to the specified 8.3 file.
*       region_mask == 1 dumps the trace buffer, successive bits dump the
*       regions mapped by BusTools_GetAddr().
*
*  RETURNS
*       API_BUSTOOLS_NO_FILE
*       API_BUSTOOLS_BADCARDNUM
*       API_SUCCESS
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_DumpMemoryV5(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT region_mask,   // (i) mask of memory regions to dump (bit 1 = region 1)       
   char * file_name,        // (i) pointer to name of output file to create             
   char * heading)          // (i) pointer to message to display in file                      
{
   BT_U32BIT    i;                        // Loop index
   BT_UINT      j;                        // Loop index
   BT_UINT      num_per_line;             // Number of values per line
   BT_UINT      block_id;                 // Loop index
   BT_U32BIT    first;                    // Offset to first list word
   BT_U32BIT    last;                     // Offset to last list word
   BT_U16BIT    data[0x80];               // Read 16+ words per line
   FILE         *hfMemFile;               // Error file handle is not open.
   time_t       tdate;                    // Date this test was run
#if defined(FILE_SYSTEM)
   BT_INT status=0;
   char dump_file[512]="";

   memset(dump_file, 0, sizeof(dump_file));
   if((status = CEI_GET_FILE_PATH(file_name, dump_file)) != BTD_OK)
     return status;

   hfMemFile = fopen(dump_file, "w+t");   // Write+Text mode.
   if(hfMemFile == NULL) {
     // Just return a code if the output file could not be created.V4.25.ajh
     return API_BUSTOOLS_NO_FILE;        // File could not be created.
   }
#else //FILE_SYSTEM
   hfMemFile = stdout;
#endif

   /******************************************************************
   *  Check for legal call
   *******************************************************************/
   if (cardnum >= MAX_BTA)
   {
      fprintf(hfMemFile, "Bad card number specified: %d\n", cardnum);
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_BADCARDNUM;
   }

   // If no regions were requested, dump them all
   if ( !region_mask ) region_mask = 0xFFFFFFFF;

   // Log the user supplied message
   fprintf(hfMemFile, "%s\n\n", heading);

   // Log the board type, etc.
   if ( CurrentCardType[cardnum] == PCCD1553 )
      fprintf(hfMemFile, " PCC-D1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCI1553 )
      fprintf(hfMemFile, " QPCI-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPCX1553 )
      fprintf(hfMemFile, " QPCX-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == Q1041553P )
      fprintf(hfMemFile, " Q104-1553P WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QCP1553 )
      fprintf(hfMemFile, " QCP-1553 WCS V%d, FPGA V%x, Channel%d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15EC )
      fprintf(hfMemFile, " R15-EC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15AMC )
      fprintf(hfMemFile, " R15-AMC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15XMC2 )
      fprintf(hfMemFile, " RXMC2-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == LPCIE1553 )
      fprintf(hfMemFile, " LPCIE-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RPCIe1553 )
      fprintf(hfMemFile, " RPCIe-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QPM1553 )
      fprintf(hfMemFile, " QPM-1553/QPMC_1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RXMC1553 )
      fprintf(hfMemFile, " RXMC-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == R15PMC )
      fprintf(hfMemFile, " R15-PMC WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == RAR15XMCXT )
      fprintf(hfMemFile, " RAR15-XMC-XT/IT/FIO WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);
   else if ( CurrentCardType[cardnum] == QVME1553/RQVME2 )
      fprintf(hfMemFile, " QVME-1553 WCS V%d, FPGA V%x, Channel %d",
              _HW_WCSRev[cardnum], _HW_FPGARev[cardnum], CurrentCardSlot[cardnum]+1);

   // Print the API version and the firmware version.
   fprintf(hfMemFile, " API Ver = %s/%s\n", API_VER, API_TYPE);
   fprintf(hfMemFile, " Build Options = %s/%s\n", BUILD_OPTIONS, BUILD_OPTIONS_INT); // V4.20

   // Time tag the output file.
   tdate = time(NULL);
   fprintf(hfMemFile, " Memory dump at %s\n\n", ctime(&tdate));  // V4.30

   fprintf(hfMemFile, " Init:bt=%d,inuse=%d,int-enable=%d,Polling-interval=%d, cardnum=%d\n"
                      " Init:bc=%d,bm=%d,rt=%d Running bc=%d,bm=%d,rt=%d HW Interrupts=%d\n",
           bt_inited[cardnum],  bt_inuse[cardnum], bt_interrupt_enable[cardnum],
           api_polling_interval, cardnum,
           bc_inited[cardnum],  bm_inited[cardnum],  rt_inited[cardnum],
           bc_running[cardnum], bm_running[cardnum], rt_running[cardnum],
           hw_int_enable[cardnum]);

   // Memory management pointers.  Output as WORD offsets on the board...
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Counts:\n");
   fprintf(hfMemFile, " bc_mblock_count       - %d\n",bc_mblock_count[cardnum]);
   fprintf(hfMemFile, " bm_count              - %d\n\n",bm_count[cardnum]);
   fprintf(hfMemFile, "\n BusTools/1553-API Memory Management Variables - Addresses:\n");
   fprintf(hfMemFile, "                         (Word/Byte)\n");
   fprintf(hfMemFile, " btmem_bc              - %5.5X/%5.5X\n",btmem_bc[cardnum]/2,btmem_bc[cardnum]);
   fprintf(hfMemFile, " btmem_bc_next         - %5.5X/%5.5X\n",btmem_bc_next[cardnum]/2,btmem_bc_next[cardnum]);
   fprintf(hfMemFile, " btmem_bm_cbuf         - %5.5X/%5.5X\n",btmem_bm_cbuf[cardnum]/2,btmem_bm_cbuf[cardnum]);
   fprintf(hfMemFile, " btmem_bm_cbuf_next    - %5.5X/%5.5X\n",btmem_bm_cbuf_next[cardnum]/2,btmem_bm_cbuf_next[cardnum]);
   fprintf(hfMemFile, " btmem_bm_mbuf         - %5.5X/%5.5X\n",btmem_bm_mbuf[cardnum]/2,btmem_bm_mbuf[cardnum]);
   fprintf(hfMemFile, " btmem_bm_mbuf_next    - %5.5X/%5.5X\n",btmem_bm_mbuf_next[cardnum]/2,btmem_bm_mbuf_next[cardnum]);
   fprintf(hfMemFile, " btmem_tail1           - %5.5X/%5.5X\n",btmem_tail1[cardnum]/2,btmem_tail1[cardnum]);
   fprintf(hfMemFile, " btmem_tail2           - %5.5X/%5.5X\n",btmem_tail2[cardnum]/2,btmem_tail2[cardnum]);
   fprintf(hfMemFile, " btmem_rt_top_avail    - %5.5X/%5.5X\n",btmem_rt_top_avail[cardnum]/2,btmem_rt_top_avail[cardnum]); 
   fprintf(hfMemFile, " btmem_pci1553_next    - %5.5X/%5.5X\n",btmem_pci1553_next[cardnum]/2,btmem_pci1553_next[cardnum]);
   fprintf(hfMemFile, " btmem_pci1553_rt_mbuf - %5.5X/%5.5X\n\n",btmem_pci1553_rt_mbuf[cardnum]/2,btmem_pci1553_rt_mbuf[cardnum]);

   // Board memory pointers.
   fprintf(hfMemFile,"                              RAM / HWREG / RAMREG / HIF\n");
   fprintf(hfMemFile, " bt_PageAddr[cardnum][0..3] = %p / %p / %p / %p\n IO-Base = %p\n",
           bt_PageAddr[cardnum][0], bt_PageAddr[cardnum][1],
           bt_PageAddr[cardnum][2], bt_PageAddr[cardnum][3], bt_iobase[cardnum]);

   // Dump the stuff we can, return here if the board has not been init'ed.V4.09.ajh
   if (bt_inited[cardnum] == 0)  // V4.09.ajh
   {
      fprintf(hfMemFile, "API has not been initialized!!!\n");
      fclose(hfMemFile);         //  Close and save the debug data.
      return API_BUSTOOLS_NOTINITED;
   }

   // Add the following settings to the dump...
   // BusTools_SetInternalBus   - Sets the flag for external or internal bus
   // BusTools_SetOptions       - Sets Illegal command, Reset Timetag options
   // BusTools_SetVoltage       - Sets the voltage hardware register
   fprintf(hfMemFile, "\n SA 31 is Mode Code=%d, Broadcast Enabled=%d\n\n",
           rt_sa31_mode_code[cardnum], rt_bcst_enabled[cardnum]);

   // Dump the time tag information.
   {
      BT1553_TIME ctime;
      char outbuf[80];

      BusTools_TimeTagRead(cardnum, &ctime);
      BusTools_TimeGetString(&ctime,outbuf);
      fprintf(hfMemFile, " Time Tag Register: top usec = %x  microseconds = %x\n",ctime.topuseconds,ctime.microseconds);
      fprintf(hfMemFile, " Display Time = %s\n\n",outbuf);
   }

   DumpTimeTag(cardnum, hfMemFile);

   // Dump the local copies of the hardware registers and RAM registers.

   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      BusTools_GetAddr(cardnum, block_id, &first, &last);
      first /= 2;   // Convert to word offset
      last  /= 2;   // Convert to word offset
      fprintf(hfMemFile, "%s Start %6.6X End %6.6X (Word Offsets)\n",
              BusTools_GetAddrName(block_id), first, last);
   }
   
   // Dump all of the BusTools_RegisterFunction() FIFO's.
   DumpRegisterFIFO(cardnum, hfMemFile);

   // Dump the RT message block pointers
   DumpRTptrs(cardnum, hfMemFile);

   // Dump the specified memory regions to the specified file.
   for ( block_id = 1; block_id <= GETADDR_COUNT; block_id++ )
   {
      /* This is where the specified buffers live */
      BusTools_GetAddr(cardnum, block_id, &first, &last);
      first /= 2;   // Convert to word offset
      last  /= 2;   // Convert to word offset
      fprintf(hfMemFile, "\n%s.Start %6.6X End %6.6X (Word Offsets)\n",
              BusTools_GetAddrName(block_id), first, last);
      if ( block_id == GETADDR_IQ )
         num_per_line = 15;
      else if ( block_id == GETADDR_RTCBUF_DEF )
         num_per_line = 15;
      else
         num_per_line = 16;
      // If region was not requested, don't dump it.
      if ( (region_mask & (1<<block_id)) == 0 )
         continue;
      if ( block_id == GETADDR_BCMESS )
         DumpBCmsg(cardnum, hfMemFile);           // Dump the BC msg area.
      else if ( block_id == GETADDR_BMMESSAGE )
         DumpBMmsg(cardnum, hfMemFile);           // Dump the BM msg area.
      else if ( block_id == GETADDR_BMFILTER )
         DumpBMflt(cardnum, hfMemFile);           // Dump the BM filter area.
      else if (block_id == GETADDR_HWREG)
      {      
         BusTools_DumpHWRegisters(cardnum,data);
         fprintf(hfMemFile, "%4.4X:",0x0);
         for ( j = 0; j < 16; j++)
         {
            fprintf(hfMemFile, " %4.4X", data[j]);
         }
         fprintf(hfMemFile, "\n");
         fprintf(hfMemFile, "%4.4X:",0x10);
         for ( j = 16; j < 32; j++)
         {
            fprintf(hfMemFile, " %4.4X", data[j]);
         }
         fprintf(hfMemFile, "\n");
      }
      else if (block_id == GETADDR_RAMREG)
      {      
         BusTools_DumpRAMRegisters(cardnum,data);
         for ( i=0; i<0x8; i++) // i += num_per_line )
         {
            fprintf(hfMemFile, "%4.4X:", i*0x10);
            for(j = 0;j<0x10;j++)
            {
               fprintf(hfMemFile, " %4.4X", data[j+i*0x10]);
            }
               fprintf(hfMemFile, "\n");
         }
      }
      else
      {
         // Dump all other requested regions to the file.
         for ( i = first; i < last; ) // i += num_per_line )
         {
            // Read the current line of num_per_line data words from the board.
            BusTools_MemoryRead(cardnum, i*2, num_per_line*2, data);
            fprintf(hfMemFile, "%4.4X:", i);
            for ( j = 0; j < num_per_line && i <= last; j++, i++ )
               fprintf(hfMemFile, " %4.4X", data[j]);
            fprintf(hfMemFile, "\n");
         }
      }
   }
   // Lastly dump the trace buffer, if available, and if any.
   DumpTrace(hfMemFile);

   fclose(hfMemFile);         //  Close and save the debug data.
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME -    BusTools_DumpMemory()
*
*  FUNCTION
*       This procedure outputs a dump of all requested data areas.  Regions
*       whose bit is set in region_mask are dumped to the specified 8.3 file.
*       region_mask == 1 dumps the trace buffer, successive bits dump the
*       regions mapped by BusTools_GetAddr().
*
*  RETURNS
*       API_BUSTOOLS_NO_FILE
*       API_BUSTOOLS_BADCARDNUM
*       API_SUCCESS
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_DumpMemory(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT region_mask,   // (i) mask of memory regions to dump (bit 1 = region 1)       
   char * file_name,        // (i) pointer to name of output file to create             
   char * heading)          // (i) pointer to message to display in file                      
{
   if(board_is_v5_uca[cardnum]) 
      return BusTools_DumpMemoryV5(cardnum,region_mask,file_name,heading);
   else      
      return BusTools_DumpMemoryV6(cardnum,region_mask,file_name,heading);
}



