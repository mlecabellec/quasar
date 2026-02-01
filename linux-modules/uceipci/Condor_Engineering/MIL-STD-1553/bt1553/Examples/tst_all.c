/*===========================================================================*
 * FILE:                     T S T _ A L L . C
 *===========================================================================*
 *
 *      COPYRIGHT (C) 1995 - 2019 BY ABACO SYSTEMS, INC. 
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
 *============================================================================*
 *
 * FUNCTION:    Demonstration of BusTools/1553-API routines.
 *
 *===========================================================================*/

/* $Revision:  2.07 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  6/11/2001     initial release
  11/18/2008    merged x86 and PPC versions. added support for QPM-1553. added
                 BusTools_GetRevision. bch
  10/22/2012    added status checking, configuration options, and support for
                 UCA32 boards. bch
  06/20/2013    added option for internal/external bus. bch
  05/05/2016    added option to select BC, BM or RT functionality. bch
  10/10/2016    added option to dump board memory and get board info. bch
  02/20/2018    added option to select either primary or secondary bus. added
                 initialization for VME boards. display RAR15-XMC front/rear
                 I/O config. bch
  12/09/2019    minor mod. bch
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "busapi.h"


#define TSTALL_VER  "2.07"

/* Configuration(required): select the bus functionality to enable.  If using a single/dual function board need to disabled BC or RT. */
#define ENABLE_MODE_BC
#define ENABLE_MODE_RT
#define ENABLE_MODE_BM

/*  CONFIGURATION(required): select one option for the interrupt mode: "software interrupts", "hardware interrupts", or both  */
#define BT_PWFLAG  API_SW_INTERRUPT
/* #define BT_PWFLAG  API_HW_ONLY_INT */
/* #define BT_PWFLAG  API_HW_INTERRUPT */

/*  CONFIGURATION(required): select the minor frame rate for the BC  */
#define MINOR_FRAME_RATE  1000000

/*  CONFIGURATION(required): select the number of BM message buffers to allocate.  */
#define BM_NUM_MSG_BUFFER  300

/*  CONFIGURATION(optional): enable internal bus, otherwise external bus  */
#define ENABLE_INTERNAL_BUS

/*  CONFIGURATION(optional): select either the "primary" bus or the "secondary" bus on the 1553 channel */
#define ENABLE_PRIMARY_CHANNEL_BUS
/* #define ENABLE_SECONDARY_CHANNEL_BUS */

/*  CONFIGURATION(optional): select which function to monitor traffic using BusTools_RegisterFunction  */
/*   setup BM interrupts   */
#define CONFIG_BM_INTFIFO
/*   setup RT interrupts   */
/* #define CONFIG_RT_INTFIFO */
/*   setup BC interrupts   */
/* #define CONFIG_BC_INTFIFO */

/*  CONFIGURATION(optional): select one option for IRIG-B timing:  IRIG_B_EXTERNAL will require an external IRIG-B signal, while IRIG_B_INTERNAL will use the internal IRIG.  */
/* #define IRIG_B_EXTERNAL */
/* #define IRIG_B_INTERNAL */

/*  CONFIGURATION(optional): BC "one shot" frame  */
/* #define ENABLE_ONE_FRAME_ONLY */

/*  CONFIGURATION(optional): increment RT data words  */
/* #define ENABLE_RT_AUTO_INCREMENT */

/*  CONFIGURATION(optional): call BusTools_DumpMemory after starting the bus functionality  */
/* #define ENABLE_BRD_MEMORY_DUMP */

/*  CONFIGURATION(optional): configures the BC to assert errors on the bus  */
/* #define ENABLE_ERROR_INJECTION */

/*  CONFIGURATION(optional): if supported with compiler/OS, this appends additional information for debugging  */
/* #define ENABLE_DEBUG_EXT */

/* end of CONFIGURATION options */

/*  set the TEST_HWINT compiler define to compile with hardware interrupts  */
#ifdef TEST_HWINT
 #undef BT_PWFLAG
 #define BT_PWFLAG  API_HW_ONLY_INT
#endif

/* check for active or secondary bus */
#if !defined(ENABLE_PRIMARY_CHANNEL_BUS) && !defined(ENABLE_SECONDARY_CHANNEL_BUS)
 #pragma message "Primary/secondary bus not selected, enabling primary bus"
 #define ENABLE_PRIMARY_CHANNEL_BUS
#endif
/* set the BC control word for either the primary or secondary bus */
#ifdef ENABLE_PRIMARY_CHANNEL_BUS
 #define BC_CHANNEL_BUS BC_CONTROL_BUSA
#elif defined(ENABLE_SECONDARY_CHANNEL_BUS)
 #define BC_CHANNEL_BUS BC_CONTROL_BUSB
#endif

/*  check for a bus function */
#if !defined(ENABLE_MODE_BC) && !defined(ENABLE_MODE_RT) && !defined(ENABLE_MODE_BM)
  #pragma message "No valid bus function selected, enabling as a BC"
  #define ENABLE_MODE_BC
#endif

/*  check for a valid function */
#if !defined(CONFIG_BC_INTFIFO) && !defined(CONFIG_RT_INTFIFO) && !defined(CONFIG_BM_INTFIFO)
  #pragma message "No valid function selected for BusTools_RegisterFunction, enabling BC"
  #define CONFIG_BC_INTFIFO
#endif

#ifdef ENABLE_DEBUG_EXT
  /* displays the function, error string and status when an API function error occurs */
  #define display_api_err(_1_,_2_) printf("Fail:  %s status \'%s.\' (%s:%d)\n", _1_, BusTools_StatusGetString(_2_), __func__, __LINE__)
#else
  /* displays the function and error string */
  #define display_api_err(_1_,_2_) printf("Fail:  %s status \'%s.\'\n", _1_, BusTools_StatusGetString(_2_))
#endif

/*  prototypes  */
BT_INT init_BM(BT_UINT cardnum);
BT_INT init_RT(BT_UINT cardnum);
BT_INT init_BC(BT_UINT cardnum, BT_INT sa_index);
BT_INT _stdcall bm_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO);
BT_INT setupThe_BM_IntFIFO(BT_UINT cardnum);
BT_INT setupThe_RT_IntFIFO(BT_UINT cardnum);
BT_INT setupThe_BC_IntFIFO(BT_UINT cardnum);

/*  globals  */
API_INT_FIFO  sIntFIFO_BC;
API_INT_FIFO  sIntFIFO_RT;
API_INT_FIFO  sIntFIFO_BM;


int main(int argc, char **argv) {
   BT_INT i, status=0, build=0, channel=0, time_ns=0, brd_func=0, brd_irig=0, brd_uca=0;
   BT_UINT cardnum=0, device=0, ucode_rev=0, api_rev=0, val32=0, flag=BT_PWFLAG;
   BT_U16BIT csc=0, acr=0;
   float wrev, lrev;
   DeviceList dlist;

   
   if(argc >= 2)
     device = atoi(argv[1]);
   if(argc >= 3)
     channel = atoi(argv[2]);

   printf("\nTSTALL v%s\n\n", TSTALL_VER);
   BusTools_API_Close(cardnum);

   /* BusTools_API_OpenChannel is not supported on LynxOS or with VME boards on Linux and Integrity */
  #if defined(LYNXOS) || defined(_LINUX_X86_VME_) || defined(INTEGRITY_VME_PPC)
   printf("\nInitializing channel %d\n", channel);
   {
    #ifdef LYNXOS_PCI
     /* need to change "cardtype" if not using a QPM-1553 */	   
     BT_UINT cardtype=QPM1553, carrier=NATIVE, mapping=CARRIER_MAP_DEFAULT, ioaddr=0;
     BT_U32BIT base_addr=device;
    #else
     BT_UINT cardtype=QVME1553, carrier=NATIVE_32, mapping=CARRIER_MAP_A32;
     /* need to set the A16 address and the A32/A24 address for (R)QVME-1553 boards */
     BT_UINT ioaddr=0;
     BT_U32BIT base_addr=0;
    #endif
     if((status = BusTools_API_InitExtended(cardnum, base_addr, ioaddr, &flag, PLATFORM_PC, cardtype, carrier, channel, mapping)) != API_SUCCESS) {
       display_api_err("BusTools_API_InitExtended", status);
       return -1;
     }
   }
  #else
   if((status = BusTools_ListDevices(&dlist)) != API_SUCCESS)
     display_api_err("BusTools_ListDevices", status);
   else {
     printf("Device List\n");
     if(dlist.num_devices <= 0) {
       printf("  No boards detected. Check installation.\n");
       return -1;
     }
     for(i=0; i<dlist.num_devices; i++)
       printf("  Device %d:  %s (ID 0x%04X)\n", dlist.device_num[i], dlist.name[i], dlist.device_name[i]);
   } 

   printf("\nInitializing device %d, channel %d\n", device, channel);
   if((status = BusTools_API_OpenChannel(&cardnum, flag, device, channel)) != API_SUCCESS) {
     display_api_err("BusTools_API_OpenChannel", status);
     return -1;
   }
  #endif

   if((status = BusTools_API_Reset(cardnum, 0)) != API_SUCCESS)
     display_api_err("BusTools_API_Reset", status);

   status = BusTools_BoardHasIRIG(cardnum);
   if((status != API_SUCCESS) && (status != API_FEATURE_SUPPORT)) {    
     display_api_err("BusTools_BoardHasIRIG", status);
    #if defined(IRIG_B_EXTERNAL) || defined(IRIG_B_INTERNAL)
     BusTools_API_Close(cardnum);
     return -1;
    #endif
   }
   else
     brd_irig = 1;
   
   status = BusTools_BoardIsV6(cardnum);
   switch(status) {
    case API_FEATURE_SUPPORT:
      brd_uca = 1; 
      time_ns = API_TT_NANO; /* set time stamps for nanosecond resolution */
    case API_SUCCESS:
      break;
    default:
      display_api_err("BusTools_BoardIsV6", status);
      BusTools_API_Close(cardnum);
      return -1;
   };

   brd_func = BusTools_BoardIsMultiFunction(cardnum);
   switch(brd_func) {
    case API_MULTI_FUNCTION:
      printf("  Multi-function");
      break;
    case API_DUAL_FUNCTION:
      printf("  Dual-function");
      break;
    case API_SINGLE_FUNCTION:
      printf("  Single-function");
      break;
    case API_SINGLE_RT:
      printf("  Single-RT");
      break;
    default: 
     display_api_err("BusTools_BoardIsMultiFunction", status);
     BusTools_API_Close(cardnum);
     return -1;
   };

   // if the board is a RAR15-XMC, then check if front or rear I/O
   if(BusTools_GetBoardType(cardnum) == 0x360) {
     // read the global register "Board Specific Register" (bytes 0x4-0x7)
     if((status = BusTools_MemoryRead2(cardnum, HIF, 0x4, 1, &val32)) != API_SUCCESS)
       display_api_err("BusTools_MemoryRead2", status);
     else {
       switch(val32 & 0x7){
       case 0x1:
         printf(", front I/O");
         break;
       case 0x2:
         printf(", rear I/O (P14)");
         break;
       case 0x4:
         printf(", rear I/O (P16)");
         break;
       default:
         printf(", err");
       };
     }
   }

   printf(", %d channels, %s, %s\n", BusTools_GetChannelCount(cardnum), brd_irig == 1?"IRIG":"No IRIG", brd_uca == 1?"UCA32":"UCA");
      
   if((status = BusTools_GetRevision(cardnum, &ucode_rev, &api_rev)) != API_SUCCESS)
     display_api_err("BusTools_GetRevision", status);
   else
     printf("  Microcode rev 0x%X, API rev %d\n", ucode_rev, api_rev);
   
   if((status = BusTools_GetFWRevision(cardnum, &wrev, &lrev, &build)) != API_SUCCESS)
     display_api_err("BusTools_GetFWRevision", status);
   else {
     printf("  WCS rev %.2f, LPU rev %.2f", wrev, lrev);
     if(brd_uca == 1)
       printf(", FPGA rev %X\n", build);
     else
       printf(" build %d\n", build);
   }

   if((status = BusTools_GetSerialNumber(cardnum, &val32)) != API_SUCCESS)
     display_api_err("BusTools_GetSerialNumber", status);
   else
     printf("  Serial Number %d\n", val32);      

  #if !defined(_LINUX_X86_VME_) || !defined(INTEGRITY_VME_PPC)
   if((status = BusTools_GetCSCRegs(cardnum, &csc, &acr)) != API_SUCCESS)
     display_api_err("BusTools_GetCSCRegs", status);
   else
     printf("  CSC 0x%04X, ACR 0x%04X\n\n", csc, acr);
  #endif
    
   printf("\n>>>  Hit any key to exit  <<<\n\n");
   
  #ifdef ENABLE_INTERNAL_BUS
   val32 = INTERNAL_BUS;
  #else
   val32 = EXTERNAL_BUS;
  #endif
   if((status = BusTools_SetInternalBus(cardnum, val32)) != API_SUCCESS)
     display_api_err("BusTools_SetInternalBus", status);

   if((status = BusTools_TimeTagMode(cardnum, API_TTD_DATE|time_ns, API_TTI_ZERO, API_TTM_FREE, NULL, 0, 0, 0 )) != API_SUCCESS)
     display_api_err("BusTools_TimeTagMode", status);
   
  #if defined(IRIG_B_EXTERNAL)
   if(brd_irig) {
     if((status = BusTools_IRIG_Config(cardnum, IRIG_EXTERNAL, IRIG_OUT_DISABLE)) != API_SUCCESS)
       display_api_err("BusTools_IRIG_Config", status);

     if((status = BusTools_TimeTagMode(cardnum, API_TTD_IRIG|time_ns, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 )) != API_SUCCESS)
       display_api_err("BusTools_TimeTagMode", status);

     if((status = BusTools_IRIG_Calibration(cardnum, 1)) != API_SUCCESS);
       display_api_err("BusTools_IRIG_Calibration", status);

     /*  wait for IRIG-B  */
     MSDELAY(2000);

     if((status = BusTools_IRIG_Valid(cardnum)) != API_SUCCESS)
       display_api_err("BusTools_IRIG_Valid", status);
   }
  #elif defined(IRIG_B_INTERNAL)
   if(brd_irig) {
     if((status = BusTools_IRIG_Config(cardnum, IRIG_INTERNAL, IRIG_OUT_DISABLE)) != API_SUCCESS)
       display_api_err("BusTools_IRIG_Config", status);

     if((status = BusTools_IRIG_SetTime(cardnum, -1, 1)) != API_SUCCESS)
       display_api_err("BusTools_IRIG_SetTime", status);

     if((status = BusTools_TimeTagMode(cardnum, API_TTD_IRIG|time_ns, API_TTI_IRIG, API_TTM_IRIG, NULL, 0, 0, 0 )) != API_SUCCESS)
       display_api_err("BusTools_TimeTagMode", status);

     /*  wait for IRIG-B */
     MSDELAY(2000);
   }
  #endif

   /*  initialize the BM  */
   if((status = init_BM(cardnum)) != API_SUCCESS)
     return -1;

   /*  initialize the RT  */
  #if defined(ENABLE_MODE_RT)
   if((status = init_RT(cardnum)) != API_SUCCESS)
     return -1;
  #endif

   /*  initialize the BC  */
  #if defined(ENABLE_MODE_BC)
   if((status = init_BC(cardnum, 0)) != API_SUCCESS)
     return -1;
  #endif
   
  #ifdef CONFIG_BM_INTFIFO
   if(setupThe_BM_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

  #ifdef CONFIG_RT_INTFIFO
   if(setupThe_RT_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

  #ifdef CONFIG_BC_INTFIFO
   if(setupThe_BC_IntFIFO(cardnum) != API_SUCCESS)
     return -1;
  #endif

   /*  start the BM  */
  #if defined(ENABLE_MODE_BM)
   if((status = BusTools_BM_StartStop(cardnum, 1)) != API_SUCCESS) {
     display_api_err("BusTools_BM_StartStop", status);
     BusTools_API_Close(cardnum);
     return -1;
   }
  #endif

   /*  start the RT  */
  #if defined(ENABLE_MODE_RT)
   if((status = BusTools_RT_StartStop(cardnum, 1)) != API_SUCCESS) {
     display_api_err("BusTools_RT_StartStop", status);
     BusTools_API_Close(cardnum);
     return -1;
   }
  #endif

   /*  start the BC  */
  #if defined(ENABLE_MODE_BC)
   if((status = BusTools_BC_StartStop(cardnum, 1)) != API_SUCCESS) {
     display_api_err("BusTools_BC_StartStop", status);
     BusTools_API_Close(cardnum);
     return -1;
   }
  #endif

   /*  read board memory and write it to a file */
  #if defined(ENABLE_BRD_MEMORY_DUMP)
   {
    char fn[40], fh[50];
    time_t _time=time(NULL);
    struct tm *_tm=gmtime(&_time);

    memset(fh, 0, sizeof(fh));
    memset(fn, 0, sizeof(fn));
    
    sprintf(fh, "TSTALL v%s (UTC %02d:%02d:%02d)", TSTALL_VER, _tm->tm_hour, _tm->tm_min, _tm->tm_sec); 
    sprintf(fn, "tstall_md_%02d%02d%02d.txt", _tm->tm_hour, _tm->tm_min, _tm->tm_sec);

    MSDELAY(100); /* wait for some bus traffc */
    if((status = BusTools_DumpMemory(cardnum, 0xFFFFFFFF, fn, fh)) != API_SUCCESS)
      display_api_err("BusTools_DumpMemory", status);
   }
  #endif

   /* wait for user */
   getc(stdin);

   printf("main: closing device %d\n", device);

  #if defined(ENABLE_MODE_BC)
   if((status = BusTools_BC_StartStop(cardnum,0)) != API_SUCCESS)
     display_api_err("BusTools_BC_StartStop", status);
  #endif

  #if defined(ENABLE_MODE_BM)
   if((status = BusTools_BM_StartStop(cardnum,0)) != API_SUCCESS)
     display_api_err("BusTools_BM_StartStop", status);
  #endif
   
  #if defined(ENABLE_MODE_RT)
   if((status = BusTools_RT_StartStop(cardnum,0)) != API_SUCCESS)
     display_api_err("BusTools_RT_StartStop", status);
  #endif
   
  #ifdef CONFIG_BC_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BC, 0)) != API_SUCCESS)
     display_api_err("BusTools_RegisterFunction", status);
  #endif

  #ifdef CONFIG_RT_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_RT, 0)) != API_SUCCESS)
     display_api_err("BusTools_RegisterFunction", status);
  #endif

  #ifdef CONFIG_BM_INTFIFO
   if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BM, 0)) != API_SUCCESS)
     display_api_err("BusTools_RegisterFunction", status);
  #endif

   if((status = BusTools_API_Close(cardnum)) != API_SUCCESS)
     display_api_err("BusTools_API_Close", status);

   return 0;
}


#ifdef ENABLE_MODE_BM
BT_INT init_BM(BT_UINT cardnum) {
  BT_INT status=0;
  BT_UINT actual=0;

  if((status = BusTools_BM_Init(cardnum, 1, 1)) != API_SUCCESS) {
    display_api_err("BusTools_BM_Init", status);
    return status;
  }

  if((status = BusTools_BM_MessageAlloc(cardnum, BM_NUM_MSG_BUFFER, &actual, 0xFFFFFFFF)) != API_SUCCESS) {
    display_api_err("BusTools_BM_MessageAlloc", status);
    return status;
  }

  if(actual != BM_NUM_MSG_BUFFER)
    printf("BM: requested %d but only allocated memory for %d messages\n", BM_NUM_MSG_BUFFER, actual);

  return API_SUCCESS;
}
#endif


BT_INT init_RT(BT_UINT cardnum) {
  BT_INT status=0, i, rt=0, sa=0, tr=0;
  API_RT_CBUF cbuf;
  API_RT_MBUF_WRITE mbuf;
  API_RT_ABUF abuf;


  if((status = BusTools_RT_Init(cardnum, 0)) != API_SUCCESS) {
    display_api_err("BusTools_RT_Init", status);
    return status;
  }

  cbuf.legal_wordcount = 0xFFFFFFFF;
  for(rt = 1; rt <= 5; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 31; sa++) {
        if((status = BusTools_RT_CbufWrite(cardnum, rt, sa, tr, 4, &cbuf)) != API_SUCCESS) {
          printf("Fail:  BusTools_RT_CbufWrite status %d - %s, RT %d, SA %d, TR %d\n", status, BusTools_StatusGetString(status), rt, sa, tr);
          return status;
        }
      }
    }
  }

  memset(&mbuf, 0, sizeof(mbuf));
  /*  Interrupt enables on end of message  */
  mbuf.enable = BT1553_INT_END_OF_MESS;

  for(rt = 1; rt <= 5; rt++) {
    for(sa = 0; sa < 31; sa++) {
      for(i = 0; i < 32; i++)
        mbuf.mess_data[i] = (BT_U16BIT)(0xF000 + i);
      for(i = 0; i < 4; i++) {
        if((status = BusTools_RT_MessageWrite(cardnum, rt, sa, 1, i, &mbuf)) != API_SUCCESS) {
          printf("Fail:  BusTools_RT_MessageWrite status %d - %s, RT %d, SA %d, msg %d\n", status, BusTools_StatusGetString(status), rt, sa, i);
          return status;
        }
      }
    }
  }

  memset(&abuf, 0, sizeof(abuf));
 #ifdef ENABLE_PRIMARY_CHANNEL_BUS
  abuf.enable_a = 1;
 #endif
 #ifdef ENABLE_SECONDARY_CHANNEL_BUS
  abuf.enable_b = 1;
 #endif
  for(rt = 1; rt <= 5; rt++) {
    if((status = BusTools_RT_AbufWrite(cardnum, rt, &abuf)) != API_SUCCESS) {
      display_api_err("BusTools_RT_AbufWrite", status);
      return status;
    }
  }

#ifdef ENABLE_RT_AUTO_INCREMENT
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 2, 2, 1, 0, 1, 1, 0, 1)) != API_SUCCESS)
    display_api_err("BusTools_RT_AutoIncrMessageData (RT 2, SA 2)", status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 3, 3, 0, 0, 1, 1, 0, 1)) != API_SUCCESS)
    display_api_err("BusTools_RT_AutoIncrMessageData (RT 3, SA 3)", status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 4, 4, 3, 0, 1, 1, 0, 1)) != API_SUCCESS)
    display_api_err("BusTools_RT_AutoIncrMessageData (RT 4, SA 4)", status);
  if((status = BusTools_RT_AutoIncrMessageData(cardnum, 5, 5, 4, 0, 1, 1, 0, 1)) != API_SUCCESS)
    display_api_err("BusTools_RT_AutoIncrMessageData (RT 5, SA 5)", status);
#endif

   return API_SUCCESS;
}


BT_INT init_BC(BT_UINT cardnum, BT_INT sa_index) {
  BT_INT status=0;
  BT_U16BIT messno=0;
  API_BC_MBUF bcmessage;

  if((status = BusTools_BC_Init(cardnum, 0, BT1553_INT_END_OF_MESS, 0, 20, 20, MINOR_FRAME_RATE, 1)) != API_SUCCESS) {
     display_api_err("BusTools_BC_Init", status);
     return status;
  }

  if((status = BusTools_BC_MessageAlloc(cardnum, 10)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageAlloc", status);
    return status;
  }

  /*  BC -> RT Message  */
  messno = 0;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_MFRAME_BEG | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 2;
  bcmessage.mess_command1.subaddr  = 2;
  bcmessage.mess_command1.wcount   = 2;
  bcmessage.mess_command1.tran_rec = 0;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  bcmessage.data[0][0] = 1;
  bcmessage.data[0][1] = 1;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> BC Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 3;
  bcmessage.mess_command1.subaddr  = 3;
  bcmessage.mess_command1.wcount   = 3;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> BC Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 4;
  bcmessage.mess_command1.subaddr  = 4;
  bcmessage.mess_command1.wcount   = 4;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> BC Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 5;
  bcmessage.mess_command1.subaddr  = 5;
  bcmessage.mess_command1.wcount   = 5;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> BC Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 2;
  bcmessage.mess_command1.subaddr  = 8;
  bcmessage.mess_command1.wcount   = 8;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> BC Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 12;
  bcmessage.mess_command1.wcount   = 6;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  Mode Code  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 0;
  bcmessage.mess_command1.wcount   = 18;
  bcmessage.mess_command1.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 20;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  RT -> RT Message  */
  messno++;
  memset((char*)&bcmessage,0,sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.messno_next = messno + 1;
  bcmessage.control = BC_CONTROL_MESSAGE | BC_CONTROL_BUFFERA | BC_CHANNEL_BUS | BC_CONTROL_RTRTFORMAT | BC_CONTROL_INTERRUPT;
  bcmessage.mess_command1.rtaddr   = 1;
  bcmessage.mess_command1.subaddr  = 10;
  bcmessage.mess_command1.wcount   = 6;
  bcmessage.mess_command1.tran_rec = 0;
  bcmessage.mess_command2.rtaddr   = 3;
  bcmessage.mess_command2.subaddr  = 6;
  bcmessage.mess_command2.wcount   = 6;
  bcmessage.mess_command2.tran_rec = 1;
  bcmessage.errorid = 0;
  bcmessage.gap_time = 200;
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

  /*  end frame */
  messno++;
  memset((char*)&bcmessage, 0, sizeof(bcmessage));
  bcmessage.messno = messno;
  bcmessage.control = BC_CONTROL_MFRAME_END;
 #ifdef ENABLE_ONE_FRAME_ONLY
  bcmessage.control |= BC_CONTROL_LAST;
 #endif
  if((status = BusTools_BC_MessageWrite(cardnum, messno, &bcmessage)) != API_SUCCESS) {
    display_api_err("BusTools_BC_MessageWrite", status);
    return status;
  }

 #ifdef ENABLE_ERROR_INJECTION
  {
    API_EIBUF ebuf;
    int i;

    ebuf.buftype = EI_BC_REC;
    ebuf.error[0].etype = EI_BITCOUNT;
    ebuf.error[0].edata = 16;
    for(i=1;i<EI_COUNT;i++) {
      ebuf.error[i].etype = EI_NONE;
      ebuf.error[i].edata = 0;
    }

    if((status = BusTools_EI_EbufWrite(cardnum, 1, &ebuf)) != API_SUCCESS)
      display_api_err("BusTools_EI_EbufWrite", status);
  }
 #endif

  return API_SUCCESS;
}


BT_INT _stdcall bm_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  API_BM_MBUF mbuf;
  BT_INT status=0, tail=0;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType & EVENT_BM_MESSAGE) {
      if((status = BusTools_BM_MessageRead(cardnum, sIntFIFO->fifo[tail].bufferID, &mbuf)) != API_SUCCESS) {
        display_api_err("BusTools_BM_MessageRead", status);
        return status;
      }
      if(mbuf.command1.tran_rec)
        printf("BM:  RT->BC\n");
      else
        printf("BM:  BC->RT\n");
      printf("BM:  RT(%d), SA(%d), WC(%d)\n", mbuf.command1.rtaddr, mbuf.command1.subaddr, mbuf.command1.wcount);
      printf("BM:  int_stat (0x%08x)\n", mbuf.int_status);

      BusTools_TimeGetString(&mbuf.time, outbuf);
      printf("BM:  time tag = %s\n\n", outbuf);
    }

    tail++;
    tail &= sIntFIFO->mask_index;
    sIntFIFO->tail_index = tail;
  }

  return API_SUCCESS;
}


BT_INT _stdcall rt_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  BT_INT status=0, tail=0, wcount=0, i=0, mc=0;
  BT_UINT messno=0, rt=0, sa=0, tr=0;
  API_RT_MBUF_READ rt_mbuf;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType == EVENT_RT_MESSAGE) {
      messno = sIntFIFO->fifo[tail].bufferID;
      rt = sIntFIFO->fifo[tail].rtaddress;
      sa = sIntFIFO->fifo[tail].subaddress;
      tr = sIntFIFO->fifo[tail].transrec;
      mc = 0;
      memset(&rt_mbuf, 0, sizeof(rt_mbuf));
      if((status = BusTools_RT_MessageRead(cardnum, rt, sa, tr, messno, &rt_mbuf)) != API_SUCCESS) {
        display_api_err("BusTools_RT_MessageRead", status);
        return status;
      }

      if(rt_mbuf.status & BT1553_INT_RT_RT_FORMAT)
        printf("RT:  RT<->RT\n");
      else if(rt_mbuf.status & BT1553_INT_MODE_CODE) {
        printf("RT:  Mode Code\n");
        mc = 1;
      }
      else if (rt_mbuf.mess_command.tran_rec)
        printf("RT:  RT->BC\n");
      else
        printf("RT:  BC->RT\n");

      printf("RT:  RT(%d), SA(%d), WC(%d), msg status(0x%04x), int status(0x%08x)\n", rt_mbuf.mess_command.rtaddr, rt_mbuf.mess_command.subaddr, rt_mbuf.mess_command.wcount, *((BT_U16BIT*)&rt_mbuf.mess_status), rt_mbuf.status);

      wcount = rt_mbuf.mess_command.wcount;
      if(mc) {
        if(wcount >= 16)
          wcount = 1;
      }
      else if(wcount == 0)
        wcount = 32;
      printf("RT:  DATA - ");
      for(i = 0; i < wcount; i++)
        printf("0x%04x ", rt_mbuf.mess_data[i]);
      printf("\n");

      BusTools_TimeGetString(&rt_mbuf.time, outbuf);
      printf("RT:  time tag = %s\n\n", outbuf);
    }

    tail++;
    tail &= sIntFIFO->mask_index;
    sIntFIFO->tail_index = tail;
  }

  return API_SUCCESS;
}


BT_INT _stdcall bc_intFunction(BT_UINT cardnum, struct api_int_fifo *sIntFIFO) {
  API_BC_MBUF bcmessage;
  BT_INT i, bufnum=0, status=0, wcount=0, tail=0, rtrt=0, mc=0;
  BT_UINT messno=0;
  char outbuf[100];

  tail = sIntFIFO->tail_index;
  while(tail != sIntFIFO->head_index) {
    if(sIntFIFO->FilterType == EVENT_BC_MESSAGE) {
      messno = sIntFIFO->fifo[tail].bufferID;
      if((status = BusTools_BC_MessageRead(cardnum, messno, &bcmessage)) != API_SUCCESS) {
       display_api_err("BusTools_BC_MessageRead", status);
        return status;
      }
      mc=rtrt=0;
      if(bcmessage.status & BT1553_INT_RT_RT_FORMAT) {
        printf("BC:  RT<->RT\n");
        rtrt=1;
      }
      else if(bcmessage.status & BT1553_INT_MODE_CODE) {
        printf("BC:  Mode Code\n");
        mc=1;
      }
      else if(bcmessage.mess_command1.tran_rec)
        printf("BC:  RT->BC\n");
      else
        printf("BC:  BC->RT\n");

      printf("BC:  RT(%d), SA(%d), WC(%d)\n", bcmessage.mess_command1.rtaddr, bcmessage.mess_command1.subaddr, bcmessage.mess_command1.wcount);
      if(rtrt)
        printf("BC:  RT(%d), SA(%d), WC(%d)\n", bcmessage.mess_command2.rtaddr, bcmessage.mess_command2.subaddr, bcmessage.mess_command2.wcount);

      if(bcmessage.status & BT1553_INT_NO_RESP) {
	printf("BC:  Error - no response from RT\n");
      }
      else {
        printf("BC:  status-1 (0x%04x)\n", *((BT_U16BIT*)&bcmessage.mess_status1));
        if(rtrt)
          printf("BC:  status-2 (0x%04x)\n", *((BT_U16BIT*)&bcmessage.mess_status2));
        printf("BC:  int_stat (0x%08x)\n", bcmessage.status);

        if(bcmessage.control & BC_CONTROL_BUFFERA)
          bufnum = 0;
        else
          bufnum = 1;

        wcount = bcmessage.mess_command1.wcount;
        if(wcount == 0)
          wcount = 32;
        if(mc) {
          if(wcount >= 16)
            wcount = 1;
        }
      
        printf("BC:  data - ");
        // if an error then display no data (xxxx)
        for(i = 0; i < wcount; i++) {
          if(i >= wcount)
            printf("    ");
          else if(bcmessage.mess_command1.tran_rec && (bcmessage.mess_status1.me || (bcmessage.status & BT1553_INT_NO_RESP)))
            printf("xxxx ");
          else
            printf("0x%04x ", bcmessage.data[bufnum][i]);
        }
        printf("\n");
      }

      BusTools_TimeGetString(&bcmessage.time_tag, outbuf);
      printf("BC:  time tag = %s\n\n", outbuf);


      tail++;
      tail &= sIntFIFO->mask_index;
      sIntFIFO->tail_index = tail;
    }
  }

  return API_SUCCESS;
}


BT_INT setupThe_BM_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_BM, 0, sizeof(sIntFIFO_BM));
  sIntFIFO_BM.function       = bm_intFunction;
  sIntFIFO_BM.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_BM.dwMilliseconds = INFINITE;
  sIntFIFO_BM.FilterType     = EVENT_BM_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_BM.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BM, 1)) != API_SUCCESS) {
    display_api_err("BusTools_RegisterFunction", status);
    return status;
  }

  return API_SUCCESS;
}


BT_INT setupThe_RT_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_RT, 0, sizeof(sIntFIFO_RT));
  sIntFIFO_RT.function       = rt_intFunction;
  sIntFIFO_RT.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_RT.dwMilliseconds = INFINITE;
  sIntFIFO_RT.FilterType     = EVENT_RT_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_RT.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_RT, 1)) != API_SUCCESS) {
    display_api_err("BusTools_RegisterFunction", status);
    return status;
  }

  return API_SUCCESS;
}


BT_INT setupThe_BC_IntFIFO(BT_UINT cardnum) {
  BT_INT rt=0, tr=0, sa=0, status=0;

  memset(&sIntFIFO_BC, 0, sizeof(sIntFIFO_BC));
  sIntFIFO_BC.function       = bc_intFunction;
  sIntFIFO_BC.iPriority      = THREAD_PRIORITY_CRITICAL;
  sIntFIFO_BC.dwMilliseconds = INFINITE;
  sIntFIFO_BC.FilterType     = EVENT_BC_MESSAGE;

  for(rt = 0; rt < 32; rt++) {
    for(tr = 0; tr < 2; tr++) {
      for(sa = 0; sa < 32; sa++)
        sIntFIFO_BC.FilterMask[rt][tr][sa] =  0xFFFFFFFF;
    }
  }

  if((status = BusTools_RegisterFunction(cardnum, &sIntFIFO_BC, 1)) != API_SUCCESS) {
    display_api_err("BusTools_RegisterFunction", status);
    return status;
  }

  return API_SUCCESS;
}


