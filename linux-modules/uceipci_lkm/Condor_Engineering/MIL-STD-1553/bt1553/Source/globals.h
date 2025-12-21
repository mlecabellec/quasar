/*============================================================================*
 * FILE:                    G L O B A L S . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2016 BY ABACO SYSTEMS, INC. 
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
 *===========================================================================*/

/* $Revision:  8.20 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/21/1996   Changed segment allocation to be compatible with Borland C.
               Changed flags from WORD to BYTE to reduce size & improve speed.
               Moved the short arrays into DGROUP to reduce code size and
               improve speed (saved over 2K of data, over 2200 bytes of code).
               Made some data arrays into constants.ajh
  04/22/1996   Changed RT_subunit definitions to support multiple boards in
               RT mode.  Bug Report A-000070.ajh
  03/17/1997   Merged 16- and 32-bit versions.V2.21.ajh
  03/29/1997   Added support for IP.V2.22.ajh
  07/13/1997   Added support for the 64 Kw IP module, RT mode.V2.30.ajh
  07/30/1997   Added support for IP on 616 VME carrier under VMIC.V2.30.ajh
  10/12/1997   Added variable for setting response_reg after run bit set.V2.37.ajh
  06/10/1998   Added VxWorks support, minor changes for V2.00 IP firmware.V2.49.ajh
  09/16/1998   Added code to support the PCI1553.V2.81.ajh
  01/10/1999   Release version of the PCI-1553.V3.00.ajh
  01/18/2000   Added ISA-1553 support.V4.00.ajh
  06/28/2000   Added second default BM CBUF.V4.06.ajh
  01/15/2001   Added enhanced BM trigger buffer definition and modified mrmory
               map to support the longer buffer.V4.31.ajh
  02/15/2002   Add support for modular API. v4.48
  12/10/2003   Add Mode Code 17 support
  02/19/2004   Add support for BusTools_API_OpenChannel
  01/02/2006   Portibility updates
  12/07/2006   O/S independence changes
  11/19/2007   Add new variable board_has_plx_dma.
  01/04/2010   Add support for RXMC-1553
  07/29/2010   Modified size of install_error_string to coincide with CEI_instal 
  01/18/2011   Support for Single RT Val mode
  10/10/2012   Changes to support V6 boards
  03/10/2017   Add R15-USB-MON support
 */

#ifndef _GLOBALS_H_
#define EXTERN extern
#else
#define EXTERN
#undef _GLOBALS_H_
#endif
//-----------------------------------------------------------------------------
// Global declarations
//-----------------------------------------------------------------------------
EXTERN BT_INT  DBC_Enable[MAX_BTA];    /* Enable Dynamic Bus Control mode code */
EXTERN BT_INT  _HW_1Function[MAX_BTA]; /* Hardware supports single function.   */
EXTERN BT_U32BIT _HW_FPGARev[MAX_BTA]; /* PCI/ISA LPU(FPGA) Version.           */
EXTERN BT_INT  _HW_WCSRev[MAX_BTA];    /* Writable Control Store Version.      */
EXTERN BT1553_TIME bmrec_timetag[MAX_BTA]; /* Simulated message timetag, BTDEMO.C */
EXTERN BT_U32BIT sched_tt_clear[MAX_BTA];/* Schedules call to TimeTagClearFlag */
EXTERN BT_INT api_polling_interval;    /* Polling resolution/interval, in ms   */
EXTERN BT_INT ext_trig_bc[MAX_BTA];    /* 1/-1 enables BC trig from TTL in.    */
EXTERN BT_UINT bt_ucode_rev[MAX_BTA];  /* Board microcode rev reported to user */
EXTERN BT_UINT CurrentPlatform[MAX_BTA]; /* Platform (PC/VMIC/etc).            */
EXTERN BT_UINT CurrentCardType[MAX_BTA]; /* Card type (PC/IP/PCI/etc).         */
EXTERN BT_UINT CurrentCarrier[MAX_BTA];  /* Carrier type (ISA/PCI/VME/VXI/etc).*/
EXTERN BT_UINT CurrentCardSlot[MAX_BTA]; /* Card slot (SLOT_A, B, C, D, etc)   */
EXTERN BT_UINT CurrentMemMap[MAX_BTA];   /* Carrier memory map (MAP_DEFAULT)   */
EXTERN BM_V6TBUF_ENH BMV6TriggerSave[MAX_BTA];/* Saved BM trigger buffer to restore*/
EXTERN BT_INT   BMTrigOutput[MAX_BTA];   /* Trigger output on every BM msg int.*/
EXTERN BT_INT DumpOnBMStop[MAX_BTA];     /* Call BusTools_DumpMemory on BM stop*/
EXTERN BT_INT MultipleBMTrigEnable[MAX_BTA]; /* Repetitive triggers enabled.   */
EXTERN BT_INT BroadcastIntEnable[MAX_BTA][32]; /* Broadcast ints enabled.      */
EXTERN char     szMsg[400];              /* Global error reporting buffer.     */
EXTERN char *boardBaseAddress[MAX_BTA];  /* Base Address of Board Mapping      */
EXTERN BT_INT procBmOnInt[MAX_BTA];      /* process BM data on H/W interrupts  */
EXTERN BT_UINT assigned_cards[MAX_BTA+1];
EXTERN char install_error_string[512];   /* Installation error string          */
EXTERN BT_U32BIT rt_status_options[MAX_BTA][32]; /* value of abuf option       */
EXTERN CEI_ULONG pShareTable[MAX_BTA];     /* Pointer to the channel share table */
EXTERN BT_UINT CurrentUSBCardType[MAX_BTA]; /* USB Card type R15-USB or R15-USB-MON */

EXTERN BT_INT hw_addr_shift[MAX_BTA];  /* Shift count; HW to byte address(1/4) */
EXTERN BT_INT board_access_32[MAX_BTA];
EXTERN BT_INT board_is_paged[MAX_BTA];
EXTERN BT_INT board_has_bc_timetag[MAX_BTA];
EXTERN BT_UINT board_using_frame_start_timing[MAX_BTA];
EXTERN BT_UINT board_using_extended_timing[MAX_BTA];
EXTERN BT_U16BIT hwreg_value[MAX_BTA][HWREG_COUNT2];
EXTERN BM_TBUF_ENH BMTriggerSave[MAX_BTA];/* Saved BM trigger buffer to restore*/

/**********************************************************************
*  BusTools flag indicating that API has been initialized
**********************************************************************/

EXTERN BT_U16BIT bt_inited[MAX_BTA];         // API Initialized
// Flag indicating BTDRV state: 0=Not Inited, 1=HW version, -1=SW version.
EXTERN BT_INT bt_inuse[MAX_BTA];
EXTERN BT_INT bt_interrupt_enable[MAX_BTA];
EXTERN BT_INT bt_op_mode[MAX_BTA];               // 1553A or 1553B
EXTERN BT_U32BIT bt_disdir[MAX_BTA];
EXTERN BT_U32BIT bt_disval[MAX_BTA];
EXTERN BT_UINT numDiscretes[MAX_BTA];
EXTERN BT_UINT numDifferentials[MAX_BTA];
EXTERN BT_INT numPIO[MAX_BTA];
EXTERN BT_U32BIT bt_dismask[MAX_BTA];
EXTERN BT_U32BIT bt_difmask[MAX_BTA];
EXTERN BT_U32BIT bt_piomask[MAX_BTA];
EXTERN BT_INT hwRTAddr[MAX_BTA];
EXTERN BT_INT board_has_irig[MAX_BTA];
EXTERN BT_INT board_has_discretes[MAX_BTA];
EXTERN BT_INT board_has_differential[MAX_BTA];
EXTERN BT_INT board_has_pio[MAX_BTA];
EXTERN BT_INT board_has_testbus[MAX_BTA];
EXTERN BT_INT board_has_hwrtaddr[MAX_BTA];
EXTERN BT_INT board_has_variable_volt[MAX_BTA];
EXTERN BT_INT board_has_485_discretes[MAX_BTA];
EXTERN BT_INT board_has_acr[MAX_BTA];
EXTERN BT_INT board_has_rtaddr_latch[MAX_BTA];
EXTERN BT_INT board_has_plx_dma[MAX_BTA];
EXTERN BT_INT board_is_channel_mapped[MAX_BTA];
EXTERN BT_INT board_is_bm_only[MAX_BTA];
EXTERN BT_INT board_is_dual_function[MAX_BTA];
EXTERN BT_INT board_has_temp_sensor[MAX_BTA];
EXTERN BT_UINT board_using_msg_schd[MAX_BTA];
EXTERN BT_UINT board_has_serial_number[MAX_BTA];
EXTERN BT_UINT boardHasMultipleTriggers[MAX_BTA];
EXTERN BT_U32BIT heartbeat[MAX_BTA];
EXTERN API_CHANNEL_STATUS channel_status[MAX_BTA];
EXTERN BT_INT channel_is_shared[MAX_BTA];
EXTERN BT_INT channel_using_multiple_bc_buffers[MAX_BTA];
EXTERN BT_U32BIT rxmc_output_config[MAX_BTA];
EXTERN CHANNEL_SHARE cshare[MAX_BTA];
EXTERN BT_INT channel_sRT_addr[MAX_BTA];
EXTERN BT_INT board_is_sRT[MAX_BTA];
EXTERN BT_INT bt_rt_mode[MAX_BTA][32];
EXTERN BT_U32BIT bm_rec_prev[MAX_BTA];       // Previous spot in BM_MBUF's (to detect wrap)
EXTERN BT_INT playback_is_running[MAX_BTA];
EXTERN BT_INT board_is_v5_uca[MAX_BTA];
EXTERN BT_INT board_using_shared_memory[MAX_BTA];


EXTERN BT_UINT NAPI_BM_V6BUFFERS[MAX_BTA]; /* Length of API BM message queue               */
/**********************************************************************
*  BC control information
**********************************************************************/

EXTERN BT_UINT  bc_inited[MAX_BTA];       // Non-zero indicates BC initialized
EXTERN BT_UINT  bc_running[MAX_BTA];      // Non-zero indicates BC is running
EXTERN BT_UINT * mblock_addr[MAX_BTA];     // table of pointers to block addresses
EXTERN BT_U16BIT frametime[MAX_BTA];       // Minor frame time, 1us LSB.
EXTERN BT_U32BIT wResponseReg4[MAX_BTA];   // response_reg[0x04] contents.

EXTERN BT_U32BIT frametime32[MAX_BTA];

/**********************************************************************
*  BM control information
**********************************************************************/

EXTERN BT_UINT  bm_inited[MAX_BTA];           // BM has been initialized
EXTERN BT_UINT  bm_running[MAX_BTA];          // BM is running flag

EXTERN BT_UINT   bm_hw_queue_ovfl[MAX_BTA];  // BM hw queue overflow flag.
EXTERN BT_INT    nAPI_BM_Head[MAX_BTA];      // API BM msg queue head pointer.
EXTERN BT_INT    nAPI_BM_Tail[MAX_BTA];      // API BM msg queue tail pointer.
EXTERN char      *lpAPI_BM_Buffers[MAX_BTA]; // API BM msg queue ptr.
EXTERN BT_INT    BM_INT_ON_RTRT_TX[MAX_BTA];

EXTERN BT_UINT   bm_count[MAX_BTA];          // Current number of BM HW buffers in circle
EXTERN BT_UINT   nBM_MBUF_len[MAX_BTA];      // Actual length(bytes) of a HW BM_MBUF, including pad

EXTERN CEI_MUTEX hFrameCritSect[MAX_BTA];  // Frame register Crit Sect.

/**********************************************************************
*  RT subunit control structures
**********************************************************************/

EXTERN BT_UINT rt_inited[MAX_BTA];          // RT initialized flag.
EXTERN BT_UINT rt_running[MAX_BTA];         // RT running flag.
EXTERN BT_INT rt_bcst_enabled[MAX_BTA];    // RT address 31 is broadcast flag.
EXTERN BT_INT rt_sa31_mode_code[MAX_BTA];  // RT subaddress 31 is mode code.

/**********************************************************************
*  Hardware memory addresses (all values are BYTE offsets on the board)
**********************************************************************/

EXTERN BT_U16BIT bc_mblock_count   [MAX_BTA];  /* BC messages allocated    */
EXTERN BT_U32BIT btmem_bc          [MAX_BTA];  /* BC message buffers start */
EXTERN BT_U32BIT btmem_bc_next     [MAX_BTA];  /* BC message buffers end   */

EXTERN BT_U32BIT btmem_bm_cbuf     [MAX_BTA];  /* BM control buffers start */
EXTERN BT_U32BIT btmem_bm_cbuf_next[MAX_BTA];  /* BM control buffers end   */

EXTERN BT_U32BIT btmem_bm_mbuf     [MAX_BTA];  /* BM message buffers start */
EXTERN BT_U32BIT btmem_bm_mbuf_next[MAX_BTA];  /* BM message buffers end   */

EXTERN BT_U32BIT btmem_tail1       [MAX_BTA];  // Mem is avail above this value,
                                               //  for structures allocated from
                                               //  bottom up in first segment.
EXTERN BT_U32BIT btmem_tail2       [MAX_BTA];  // Mem is avail below this value,
                                               //  for structures allocated from
                                               //  top down.

EXTERN BT_U32BIT btmem_rt_top_avail[MAX_BTA];  // Memory above here allocated
                                               //  to RT in seg 1

EXTERN BT_U32BIT btmem_pci1553_next[MAX_BTA];  /* Next avail memory above     */
                                               /*  64 KW, PCI-1553.  Init to  */
                                               /*  64 KW, increment as alloc. */
EXTERN BT_U32BIT btmem_pci1553_rt_mbuf[MAX_BTA];/* Top of memory              */
                                                /* Decrement as RT mbuf's are */
                                                /* allocated from top down.   */

#ifdef INCLUDE_USB_SUPPORT
EXTERN pCEI_VOID gpDevice[MAX_BTA];
BT_U32BIT *usb_host_hwreg[MAX_BTA];
IQ_MBLOCK_V6 *usb_int_queue[MAX_BTA];
#endif //INCLUDE_USB_SUPPORT
BT_U32BIT * ptrMbuf[MAX_BTA];

#define NAPI_BM_BUFFERS  8192 /* Length of API BM message queue, a power of 2 */
#if NAPI_BM_BUFFERS & NAPI_BM_BUFFERS-1
#error The value of NAPI_BM_BUFFERS is not a power of 2!
#endif
/******************************************************************************
 * Layout for new V6 F/W
 ******************************************************************************/
#define BTMEM_HWREGS         0x0000         /* Byte address of first HW reg  */

#define BTMEM_BM_V6TBUF      (0x0)          /* BM Trigger @0x0: 44/88 words/bytes RAM Region    */
#define BTMEM_BM_V6TBUF_NEXT (BTMEM_BM_V6TBUF + sizeof(BM_V6TBUF_ENH))

#define BTMEM_BM_V6CBUF_DEF  BTMEM_BM_V6TBUF_NEXT /* @0x44: 8 words.V4.06           */
#define BTMEM_BM_V6CBUF_NEXT (BTMEM_BM_V6CBUF_DEF + 2 * sizeof(BM_CBUF))

#define BTMEM_IQ_V6          0x100        /* @0x100: 512*4 = 1024 words           */ 
#define BTMEM_IQ_V6_NEXT     (BTMEM_IQ_V6 + IQ_V6_SIZE * sizeof(IQ_MBLOCK_V6))

#define BTMEM_EI_V6          BTMEM_IQ_V6_NEXT/* @0x1124: 30*33 = 990 words           */
#define BTMEM_EI_V6_NEXT     (BTMEM_EI_V6 + EI_MAX_ERROR_NUM*sizeof(EI_MESSAGE))

#define BTMEM_BM_V6FBUF      ((BTMEM_EI_V6_NEXT + 0x0FFF) & 0xF000)  /* @0x2000: 2048 words             */
#define BTMEM_BM_V6FBUF_NEXT (BTMEM_BM_V6FBUF + sizeof(BM_V6FBUF))  /*                                 */

#define BTMEM_CH_V6SHARE     BTMEM_BM_V6FBUF_NEXT
#define BTMEM_CH_V6SHARE_NEXT (BTMEM_CH_V6SHARE + sizeof(CHANNEL_SHARE))

#define BTMEM_RT_V6FBUF      (2*0xF000L) /* RT Filter buffer, top of 2nd seg      */
#define BTMEM_RT_V6ABUF      (HWREG_RT_ABUF_ADDR) //(0x80)      /* RT Addr buff now part of HWREGs    */
#define BTMEM_RT_V6MBUF_DEF  (2*0xe980L) /* RT Default Message Buffers            */
#define BTMEM_RT_V6CBUF_DEF  (2*0xe900L) /* RT Default Control Buffers            */
#define BTMEM_RT_V6CBUFBROAD (2*0xd800L) /* RT Default Broadcast Control Bufs     */
#define BTMEM_RT_V6CBUFBSA31 (2*0xd900L) /* Top (bro/sa31=mode code) avail RT     */ 
#define BTMEM_RT_V6TOP_NOBRO (2*0xe900L) /* Top addr(nobro) avail in RT memory    */
#define BTMEM_V6DIFF_IO      (2*0x80)    /* Start of Discrete registers           */

/***********************************************************************************
 * Compatibility with older F/W versions
 **********************************************************************************/

#define BTMEM_RAMREGS      0x0020         /* Byte address of first RAM reg */

#define BTMEM_BM_TBUF      (BTMEM_RAMREGS + 2*RAMREG_COUNT)   /* BM Trigger @0x080: 53 words */
#define BTMEM_BM_TBUF_NEXT (BTMEM_BM_TBUF + sizeof(BM_TBUF_ENH))

#define BTMEM_BM_CBUF_DEF  BTMEM_BM_TBUF_NEXT                 /* @0x0B5: 8 words.V4.06  */
#define BTMEM_BM_CBUF_NEXT (BTMEM_BM_CBUF_DEF + 2*sizeof(BM_CBUF) + 2*12)

#define BTMEM_BC_RETRY_BUF (BTMEM_BM_CBUF_DEF + 2*sizeof(BM_CBUF) + 6) /* @0x0BD: 9 words */

#define BTMEM_IQ           BTMEM_BM_CBUF_NEXT                 /* @0x0BD: 296*3 = 900 words */
#define BTMEM_IQ_NEXT      (BTMEM_IQ + IQ_SIZE*sizeof(IQ_MBLOCK))

#define BTMEM_EI           BTMEM_IQ_NEXT                      /* @0x441: 30*33 = 990 words */
#define BTMEM_EI_NEXT      (BTMEM_EI + EI_MAX_ERROR_NUM*sizeof(EI_MESSAGE))

#define BTMEM_BM_FBUF      ((BTMEM_EI_NEXT + 0x0FFF) & 0xF000)/* @0x800: 2048 words */
#define BTMEM_BM_FBUF_NEXT (BTMEM_BM_FBUF + sizeof(BM_FBUF))  /*         to 0x1000  */

#define BTMEM_CH_SHARE      BTMEM_BM_FBUF_NEXT
#define BTMEM_CH_SHARE_NEXT (BTMEM_CH_SHARE + sizeof(CHANNEL_SHARE))

/******************************************************************************
* RT - Segment 2 mapping, byte offsets.  Note that the most significant bit
*      is NOT set, it must be obtained from "btmem_rt_begin[MAX_BTA]". 
******************************************************************************/
#define BTMEM_RT_FBUF      (2*0xF800L) /* RT Filter buffer, top of 2nd seg   */
#define BTMEM_RT_ABUF      (2*0xF780L) /* RT Addr buff, just below RTFBUF    */
#define BTMEM_RT_MBUF_DEF  (2*0xF180L) /* RT Default Message Buffers         */
#define BTMEM_RT_CBUF_DEF  (2*0xF120L) /* RT Default Control Buffers         */
#define BTMEM_RT_CBUFBROAD (2*0xE160L) /* RT Default Broadcast Control Bufs  */
#define BTMEM_RT_CBUFBSA31 (2*0xE1DEL) /* Top (bro/sa31=mode code) avail RT  */
#define BTMEM_RT_TOP_NOBRO (2*0xF120L) /* Top addr(nobro) avail in RT memory */
#define BTMEM_DIFF_IO      (2*0x80)    /* Start of Discrete registers        */

/*============================================================================*

 *============================================================================*
 *  Function pointers to the BusTools API functions which are defined in a
 *  user interface DLL.  These functions are called by the API (if they exist)
 *  whenever the associated API function is called.
 *===========================================================================*/
#if defined(_USER_DLL_)
// Called by BusTools_API_Close():
EXTERN NOMANGLE BT_INT (CCONV *pUsrAPI_Close[MAX_BTA])(BT_UINT cardnum);

// Called by BusTools_BC_MessageAlloc():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBC_MessageAlloc[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *count);         // (i) number of BC messages to allocate

// Called by BusTools_BC_MessageRead():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBC_MessageRead[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *messno,         // (i) index of BC message to read
   API_BC_MBUF *api_message); // (o) user's buffer to write message into

// Called by BusTools_BC_MessageUpdate():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBC_MessageUpdate[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mblock_id,      // (i) index of BC message to update
   BT_U16BIT *buffer);      // (i) pointer to data to copy to BC message

// Called by BusTools_BC_MessageWrite():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBC_MessageWrite[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *messno,         // (i) index of BC message to write to
   API_BC_MBUF *api_message); // (i) pointer to user-specified BC message

// Called by BusTools_BC_StartStop():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBC_StartStop[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> start BC (at message 0), 0 -> stop BC

// Called by BusTools_BM_MessageAlloc():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBM_MessageAlloc[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mbuf_count,
   BT_UINT *mbuf_actual,
   BT_U32BIT *enable);

// Called by BusTools_BM_MessageRead():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBM_MessageRead[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *mbuf_id,
   API_BM_MBUF *mbuf);

// Called by BusTools_BM_StartStop():
EXTERN NOMANGLE BT_INT (CCONV *pUsrBM_StartStop[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> Start BM, 0 -> Stop BM

// Called by BusTools_RT_CbufWrite():
EXTERN NOMANGLE BT_INT (CCONV *pUsrRT_CbufWrite[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *rtaddr,         // (i) RT address (0 - based)
   BT_UINT *subaddr,        // (i) RT subaddress (0 - based)
   BT_UINT *tr,             // (i) Transmit/Receive flag (1->rt transmit)
   BT_INT  *mbuf_count,     // if negative, one pass through buffers only
   API_RT_CBUF *apicbuf);   // (i) pointer to API RT control buf

// Called by BusTools_RT_MessageRead():
EXTERN NOMANGLE BT_INT (CCONV *pUsrRT_MessageRead[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *rtaddr,         // (i) RT address (0 - based)
   BT_UINT *subaddr,        // (i) RT subaddress (0 - based)
   BT_UINT *tr,             // (i) Transmit/Receive flag (1->rt transmit)
   BT_UINT *mbuf_id,        // (i) RT MBUF number
   API_RT_MBUF_READ *mbuf);

// Called by BusTools_RT_StartStop():
EXTERN NOMANGLE BT_INT (CCONV *pUsrRT_StartStop[MAX_BTA])(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT *flag);          // (i) 1 -> start RT, 0 -> stop

#endif // defined(_USER_DLL_)

#undef EXTERN
