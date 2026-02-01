/*============================================================================*
 * FILE:                      A P I I N T . H
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
 * FUNCTION:    BusTools API library:
 *              This file contains definitions used internally by the API code.
 *              Most of the definitions describe hardware related structures,
 *              word offsets and bit positions.
 *
 *===========================================================================*/

/* $Revision:  8.28 $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/11/1996   Modified RAMREG_RESERVED1 (Register 78) to be RAMREG_FLAGS.
               Added definition for the "new_minor_flag" bit in the RAM
               Register File word 78hex.  This should fix the problem
               in OneShot mode which caused the messages to be delayed by up
               to a full minor frame time.  Bug Report A-000012.ajh
  09/02/1996   Modified to support 32-bit operation under Win95 and WinNT.ajh
  03/17/1997   Merged 16- and 32-bit versions.V2.21.ajh
  04/04/1997   Modified to support IP module.  Removed unused space in the
               BM_TBUF and the BC_MESSAGE and the BC_CBUF.V2.22.ajh
  07/13/1997   Added support for the 64 Kw IP module, RT mode.V2.30.ajh
  01/05/1998   Removed WinRT function definitions into LOWLEVEL.H.V2.43.ajh
  05/13/1998   Split INIT.C and BTDRV.C into new files to help isolate the
               system-specific functions to individual files.V2.47.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  08/03/2000   Added RAMREG_RT_DISA and RAMREG_RT_DISB to support LPU V3.05 and
               WCS V3.07.  Added RAMREG_RT_ITF 8/9/00.v4.09.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  12/12/2000   Changed time tag register definitions for V3.08/3.06 FW.V4.28.ajh
  01/15/2001   Added enhanced BM trigger buffer definition, reduced the size of
               the EI buffer by one entry to fit the BM trigger buffer.V4.31.ajh
  04/18/2001   Changed the file register definitions for WCS V3.20 for
               non-IP-1553 products.V4.38.ajh
  02/07/2002   Added support for modular design v4.46
  03/15/2002   Added IRIG Support. V4.48
  11/19/2007   Added prototype for vbtSetPLXRegister8 and vbtSetPLXRegister32 to 
               support DMA for PLX 9056 boards.
  11/19/2007   Added PLX DMA Defines.
  11/19/2007   Add defines for Fixed gap monitor invalid commands and undefined 
               mode code illegal. 
  07/02/2010   Set structs BM_MBUF, RT_CBUF, RT_CBUFBROAD and RT_MBUF_HW to be
               packed.
  08/17/2011   Set CHANNEL_SHARE to be packed. Added pointer offset macro for 
               CHANNEL_SHARE defines offsets.bch
  03/29/2013   V6 updates
  01/27/2014   Update BC message for new branch addr.
  01/27/2014   Update BM for interrupt disable.
  11/05/2014   Add BusTools_RT_MessageBufferNext() API
  11/12/2015   Modified channel_share struct.bch
  03/12/2017   Add R15-USB-MON support
  11/16/2017   Changes to address warnings when using VS9 Wp64 compiler option.
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/
#ifndef APIINT_H
#define APIINT_H
#if defined(ADD_TRACE)
#include "apinames.h"  /* Only used by trace function */
#endif

/**********************************************************************
*  Swap the two words in a dword, two bytes in a word
**********************************************************************/

#ifdef NON_INTEL_WORD_ORDER
#define flip(longword) *(longword) = ((*(longword) >> 16) & 0x0000ffff) | ((*(longword) << 16) & 0xffff0000)
#else
#define flip(longword)
#endif
//------------------------------------------

#ifdef NON_INTEL_WORD_ORDER
#define little_endian_32(longword) (((longword & 0x000000ff) << 24) | \
                            ((longword & 0x0000ff00) << 8)  | \
                            ((longword & 0xff0000)   >> 8)  | \
                            ((longword & 0xff000000) >> 24))
#else
#define little_endian_32(longword) longword
#endif
//--------------------------------------------

#ifdef WORD_SWAP
#define flipw(word) *(word)= \
    (unsigned short)(((*(word) >> 8) & 0x00ff) | \
            ((*(word) << 8) & 0xff00))
#else
#define flipw(word)
#endif
//--------------------------------------------

#ifdef WORD_SWAP
#define flipws(word) (unsigned short)(((word >> 8) & 0x00ff) | ((word << 8) & 0xff00))
#else
#define flipws(word) word
#endif
//--------------------------------------------

#ifdef NON_INTEL_WORD_ORDER
#define flips(longword) (((longword>>16)&0x0000ffff) | ((longword<<16)&0xffff0000))
#else
#define flips(longword) longword
#endif
//--------------------------------------------

#if defined(WORD_SWAP)
#define fliplong(longword)  *(longword)= (((*longword & 0x000000ff) << 24) | \
                            ((*longword & 0x0000ff00) << 8)  | \
                            ((*longword & 0xff0000)   >> 8)  | \
                            ((*longword & 0xff000000) >> 24))
#elif defined(NON_INTEL_WORD_ORDER)
#define fliplong(longword) *(longword) = ((*(longword) >> 16) & 0x0000ffff) | ((*(longword) << 16) & 0xffff0000)
#else
#define fliplong(longword)
#endif
//--------------------------------------------

#if defined(WORD_SWAP)
#define fliplongs(longword)  (((longword & 0x000000ff) << 24) | \
                            ((longword & 0x0000ff00) << 8)  | \
                            ((longword & 0xff0000)   >> 8)  | \
                            ((longword & 0xff000000) >> 24))
#elif defined(NON_INTEL_WORD_ORDER)
#define fliplongs(longword) ((((longword) >> 16) & 0x0000ffff) | (((longword) << 16) & 0xffff0000))
#else
#define fliplongs(longword) longword
#endif

#if defined(NON_INTEL_WORD_ORDER)
#define flipll(longlong)  (((longlong & 0xffffffff00000000ll) >>32) | ((longlong & 0xffffffffll)<<32))
#else
#define flipll(longword) longword
#endif

//--------------------------------------------
#define wsizeof(x) (sizeof(x)>>2)

/**********************************************************************
*  CSC Definitions
**********************************************************************/
#define CSC_R15_USB_MON          0x04000000   // CSC bit 26 0 = R15-USB, 1 = BT3-USB-MON
#define CSC_IS_API_LOCKED        0x08000000   // CSC bit 27 0 = API not locked, 1 = API locked 
                                              // The R15-USB always has this bit set to 
                                              // This bit will show up in CSC bit 27 unless the “API lock” override command is issued.
#define R15USBMON                0x3001       /* This is the R15-USB-MON                   */ 
/**********************************************************************
*  Bus Controller Definitions
**********************************************************************/

#define BC_HWCONTROL_NOP        0x0000  //
#define BC_HWCONTROL_OP         0x0001  //
#define BC_HWCONTROL_SET_NOP    ~BC_HWCONTROL_OP // 0xFFFE
#define BC_HWCONTROL_SET_TNOP   0x4000
#define BC_HWCONTROL_CLEAR_TNOP 0xbfff// 1011 1111 1111 1111
 
// These bits defined only if BC Control word bit 0 = 0...
#define BC_HWCONTROL_MESSAGE    0x0002  // 1 -> BC Message
#define BC_HWCONTROL_LASTMESS   0x0004  // 1 -> last message in major frame
#define BC_HWCONTROL_CONDITION  0x0006  // 1 -> branch on condition

// These bits defined only if BC Control word bit 0 = 1....
#define BC_HWCONTROL_MFRAMEBEG  0x0008  // 1 -> beg of minor frame
#define BC_HWCONTROL_MFRAMEEND  0x0080  // 1 -> end of minor frame
#define BC_HWCONTROL_RTRTFORMAT 0x0020  // 1 -> rt-to-rt format
#define BC_HWCONTROL_RETRY      0x0400  // 1 -> retry enabled 

#define BC_HWCONTROL_INTERRUPT  0x0010  // 1 -> interrupts enabled for this msg
#define BC_HWCONTROL_INTQ_ONLY  0x1000  // 1 -> No H/W interrupt)

#define BC_HWCONTROL_BUFFERB    0x0000  // 0 -> use buffer b 
#define BC_HWCONTROL_BUFFERA    0x0040  // 1 -> use buffer a

#define BC_HWCONTROL_CHANNELA   0x0000  // 0 -> use channel a 
#define BC_HWCONTROL_CHANNELB   0x0100  // 1 -> use channel b 

/**************************************************
*  IRIG Register Definitions
**************************************************/
#define IRIG_CTL_BASE           0x0100
#define IRIG_DAC_REG            0x0000
#define IRIG_CNTL_REG           0x0002
#define IRIG_TOY_REG_LSB        0x0004
#define IRIG_TOY_REG_MSB        0x0006
#define DAC_MIN                 0x0000
#define DAC_MAX                 0x00ff
#define MIN_DAC_LEVEL           0x10
#define IRIG_DEFAULT_DAC        155

#define IRIG_V6_CTL             0x50 
#define IRIG_V6_TOY             0x51   
#define V6_IRIG_DAC_SEL         0x800   

/**************************************************
*  QPMC Discrete Register Definitions
***************************************************/
#define DISCRETE_OUT_LSB        0x0008
#define DISCRETE_OUT_MSB        0x000a

/**************************************************
*  QPCI BIT LEDS
***************************************************/
#define QPCI_BIT_FAIL 0x2
#define QPCI_BIT_PASS 0x1
#define QPCI_BIT_OFF  0x0

/**************************************************
*  PLX 9056 DMA DEFINES
***************************************************/
#define PLX_DMA_MODE0        0x80
#define PLX_DMA_HOST_ADDR0   0x84
#define PLX_DMA_BOARD_ADDR0  0x88
#define PLX_DMA_BYTE_COUNT0  0x8c
#define PLX_DMA_FLAG0        0x90
#define PLX_DMA_STATUS0      0xa8

#define PLX_DMA_MODE1        0x94
#define PLX_DMA_HOST_ADDR1   0x98
#define PLX_DMA_BOARD_ADDR1  0x9c
#define PLX_DMA_BYTE_COUNT1  0xa0
#define PLX_DMA_FLAG1        0xa4
#define PLX_DMA_STATUS1      0xa9

#define PLX_DMA_THRESHOLD    0xb0



/*****************************************************
*  User Allocated Memory Buffer Pointers and Function
******************************************************/
typedef struct bt_user_alloc_count_entry
{
		BT_U32BIT bcount;
		BT_U32BIT addr;
} BT_USER_ALLOC_COUNT_ENTRY;

BT_USER_ALLOC_COUNT_ENTRY bt_user_alloc_count[MAX_BTA][MAX_BT_USR_ALLOC_ENTRY]; 
	
BT_U32BIT bt_user_alloc_entry_index[MAX_BTA];

void DumpUserAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);

/**************************************************
*  BC Message Block 
**************************************************/

typedef struct bc_msgbuf
{
  BT_U32BIT      messno;          // Message Number                                            (0-3)
  BT_U32BIT      num_data_buffers;// Number of Data buffers for this message                   (4-7)
  BT_U32BIT      control_word;    // Control and function definition                           (8-11)
  BT1553_COMMAND mess_command1;   // First 1553 command word	                                 (12-13)						
  BT1553_COMMAND mess_command2;   // Second 1553 command word (RT->RT transmit)                (14-15)
  BT_U32BIT      addr_error_inj;  // Address of error injection buffer                         (16-19)
  BT_U32BIT      data_addr;       // Starting data buffer.                                     (20-23)
  BT_U32BIT      gap_time;        // Intermessage gap time in microseconds                     (24-27)
  BT_U16BIT      rep_rate;        // Message scheduling Start Frame                            (28-29)
  BT_U16BIT      start_frame;     // Message scheduling Repeat Rate                            (30-31)
  BT_U32BIT      addr_next;       // Address of next message to be processed                   (32-35)         
}BC_MSGBUF;  

#define BC_MSGBUF_SIZE   sizeof(BC_MSGBUF)
#define BC_MSGBUF_WSIZE  BC_MSGBUF_SIZE/2
#define BC_MSGBUF_DWSIZE BC_MSGBUF_SIZE/4

typedef struct bc_msgdata
{
   BT_U32BIT     msg_addr;        // Pointer to the message Buffer associated with this data  (0-3)
   BT_U32BIT     next_data;       // pointer to next data buffer                              (4-7)          
   BT1553_TIME   timetag;         // Time Tag                                                 (8-15)
   BT_U32BIT     mstatus;         // Interrupt status word                                    (16-19)
   BT1553_STATUS mess_status1;    // First 1553 status word (RT->RT transmit)                 (20-21)
   BT1553_STATUS mess_status2;    // Second 1553 status word (RT->RT receive)                 (22-23)
   BT_U16BIT     data_word[34];   // Data words up to 34                                      (24-91) 
}BC_MSGDATA;

#define BC_MSGDATA_SIZE   sizeof(BC_MSGDATA)
#define BC_MSGDATA_WSIZE  BC_MSGDATA_SIZE/2
#define BC_MSGDATA_DWSIZE BC_MSGDATA_SIZE/4

typedef struct bc_ctrlbuf
{
  BT_U32BIT      messno;          //                                                           (0-3)
  BT_U32BIT      num_data_buffers;// Number of Data buffers for this message                   (4-7)
  BT_U32BIT      control_word;    // Control and function definition                           (8-11)
  BT_U32BIT      reserved1;       //                                                           (12-15)
  BT_U32BIT      reserved2;       //                                                           (16-19)
  BT_U32BIT      data_addr;       // Start of the data buffers                                 (20-23) 
  BT_U32BIT      gap_time;        // Intermessage gap time in microseconds                     (24-27)
  BT_U32BIT      branch_msg_ptr;  // Address of message to execute if condition TRUE           (28-31)  
  BT_U32BIT      addr_next;       // Address of next message or msg if test FALSE              (32-35)
}BC_CTRLBUF;

#define BC_CTRLBUF_SIZE   sizeof(BC_CTRLBUF)
#define BC_CTRLBUF_WSIZE  BC_CTRLBUF_SIZE/2
#define BC_CTRLBUF_DWSIZE BC_CTRLBUF_SIZE/4

typedef struct bc_ctrldata
{
   BT_U32BIT     msg_addr;        // Pointer to the message Buffer associated with this data   (0-3)
   BT_U32BIT     next_data;       // Pointer to the next data buffer                           (4-7)
   BT1553_TIME   timetag;         // Time Tag                                                  (8-15)
   BT_U32BIT     tst_wrd_addr;    // Test word memory address                                  (16-19)
   BT_U32BIT     data_pattern;    // Fixed value to compare with test word                     (20-23)
   BT_U32BIT     bit_mask;        // Mask for compare                                          (24-27)    
   BT_U32BIT     cond_counter;    // Hardware cond counter   (Not used, set to zero)           (28-31) 
   BT_U32BIT     cond_count_val;  // Conditional count value (Not used, set to zero)           (32-35)
   BT_U32BIT     next_msg_addr;   // Address of the next message executed                      (36-39)
   BT_U16BIT     reserved[26];    // padding                                                   (40...)

}BC_CTRLDATA; 

#define BC_CTRLDATA_SIZE   sizeof(BC_CTRLDATA)
#define BC_CTRLDATA_WSIZE  BC_CTRLDATA_SIZE/2
#define BC_CTRLDATA_DWSIZE BC_CTRLDATA_SIZE/4

/**************************************************
*  BC Message v5
**************************************************/

typedef struct bc_message
   {
   BT_U16BIT      control_word;    // Control and function definition
   BT1553_COMMAND mess_command1;   // First 1553 control word
   BT_U16BIT      addr_error_inj;  // Address of error injection buffer
   BT_U16BIT      gap_time;        // Intermessage gap time in microseconds
   BT_U16BIT      addr_data1;      // Word address data buffer A 
   BT_U16BIT      addr_data2;      // Word address data buffer B 
   BT1553_COMMAND mess_command2;   // Second 1553 control word (RT->RT transmit) 
   BT1553_STATUS  mess_status1;    // First 1553 status word (RT->RT transmit)
   BT1553_STATUS  mess_status2;    // Second 1553 status word (RT->RT receive)
   BT_U16BIT      mstatus[2];      // Message status: protocol errors, et.al.  Avoid mis-aligned long.
   BT_U16BIT      addr_next;       // Address of next message to be processed 
   BT1553_V5TIME  timetag PACKED;  // Tag Time for F/W v3.97 and greater
   BT_U16BIT      start_frame;     // Used with word 5 (addr_data2) when using message scheduling other un-used 
   BT_U16BIT      gap_time2;
   BT_U16BIT      reserve[7];
   }
BC_MESSAGE;

#define BC_MESSAGE_CTRL_WD_OFFSET    0
#define BC_MESSAGE_CMD1_OFFSET       2
#define BC_MESSAGE_EIADDR_OFFSET     4
#define BC_MESSAGE_GAPTIME_OFFSET    6
#define BC_MESSAGE_DADDR1_OFFSET     8
#define BC_MESSAGE_DADDR2_OFFSET     10
#define BC_MESSAGE_CMD2_OFFSET       12
#define BC_MESSAGE_STAT1_OFFSET      14
#define BC_MESSAGE_STAT2_OFFSET      16
#define BC_MESSAGE_MSTAT_OFFSET      18
#define BC_MESSAGE_ADDR_NEXT_OFFSET  22
#define BC_MESSAGE_TTADDR_OFFSET     24
#define BC_MESSAGE_STRT_FRM_OFFSET   30
#define BC_MESSAGE_GAP_OFFSET        32
 
/********************************************************
*  BC Conditional/Control/Noop Message
********************************************************/
typedef struct bc_cbuf
   {
   BT_U16BIT  control_word;    // Control and function definition
   BT_U16BIT  tst_wrd_addr1;   // Most significant 16 bits, or entire addr
   BT_U16BIT  tst_wrd_addr2;   // Least significant 3 bits of word addr
   BT_U16BIT  data_pattern;    // Fixed value to compare with test word
   BT_U16BIT  bit_mask;        // Mask for compare
   BT_U16BIT  cond_count_val;  // Conditional count value (Not used, set to zero)
   BT_U16BIT  cond_counter;    // Hardware cond counter   (Not used, set to zero)
   BT_U16BIT  res1;
   BT_U16BIT  res2;
   BT_U16BIT  res3;
   BT_U16BIT  branch_msg_ptr;  // Address of message to execute if condition TRUE 
   BT_U16BIT  addr_next;       // Address of next message or msg if test FALSE
   BT1553_V5TIME timetag PACKED; // Tag Time for F/W v3.97 and greater only on a stop block
   BT_U16BIT  tbd1;
   BT_U16BIT  reserved[8];
   }
BC_CBUF;

#define BC_CBUF_CTRL_WD_OFFSET       0
#define BC_CBUF_TWD_ADDR1_OFFSET     2
#define BC_CBUF_TWD_ADDR2_OFFSET     4
#define BC_CBUF_DATA_PAT_OFFSET      6
#define BC_CBUF_BIT_MASK_OFFSET      8
#define BC_CBUF_COND_CNT_VAL_OFFSET  10
#define BC_CBUF_COND_CNT_OFFSET      12
#define BC_CBUF_BRCH_MSG_PTR_OFFSET  20
#define BC_CBUF_ADDR_MNEXT_OFFSET    22
#define BC_CBUF_TIME_TAG_OFFSET      24

/************************************************
*  BC Data Block
************************************************/

typedef struct bc_dblock
   {
   BT_U16BIT word[34];
   }
BC_DBLOCK;


#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)
#endif
/*****************************************************************
*  Channel Share structure for V5 boards
******************************************************************/
typedef struct channel_share
{
   BT_UINT  header;                                                  // - 0
   BT_UINT  share_count;     //                                      // - 4
   BT_UINT  bc_inited;       // Non-zero indicates BC initialized    // - 8
   BT_UINT  bc_running;      // Non-zero indicates BC is running     // - 12 (c)
   BT_UINT  bm_inited;       // BM has been initialized              // - 16 (10)
   BT_UINT  bm_running;      // BM is running flag                   // - 20 (14)
   BT_UINT  rt_inited;       // RT initialized flag.                 // - 24 (18)
   BT_UINT  rt_running;      // RT running flag.                     // - 28 (1c) 

   BT_UINT CurrentPlatform;                                          // - 32 (20)
   BT_UINT CurrentCardSlot;                                          // - 36 (24) 
   BT_UINT CurrentCarrier;                                           // - 40 (28)
   BT_UINT CurrentCardType;                                          // - 44 (2c)
   BT_UINT hw_int_enable;                                            // - 48 (30)
   BT_UINT bt_ucode_rev;                                             // - 52 (34)

   BT_INT board_has_irig;                                            // - 56 (38) 
   BT_INT board_has_testbus;                                         // - 60 (3c) 
   BT_INT board_has_discretes;                                       // - 64 (40)
   BT_INT board_has_differential;                                    // - 68 (44)
   BT_INT board_access_32;                                           // - 72 (48)
   BT_INT board_has_bc_timetag;                                      // - 76 (4c)
   BT_INT board_has_485_discretes;                                   // - 80 (50)
   BT_INT board_has_serial_number;                                   // - 84 (54)
   BT_INT board_has_hwrtaddr;                                        // - 88 (58)

   BT_INT hwRTAddr;                                                  // - 92 (5c)

   API_CHANNEL_STATUS channel_status;                                // - 96 (60)

   BT_INT bt_op_mode;                                                // - 100 (64) 
                                              
   BT_INT api_device;                                                // - 104 (68)                                                 
   BT_INT bt_interrupt_enable;                                       // - 108 (6c)

   BT_INT numDiscretes;                                              // - 112 (70)
   BT_U32BIT bt_dismask;                                             // - 116 (74)

   BT_INT _HW_1Function;                                             // - 120 (78)
   BT_INT _HW_FPGARev;                                               // - 124 (7c)
   BT_INT _HW_PROMRev;                                               // - 128 (80) 
   BT_INT _HW_WCSRev;                                                // - 132 (84)

   BT_U32BIT btmem_bm_mbuf;                                          // - 136 (88)
   BT_U32BIT btmem_bm_mbuf_next;                                     // - 140 (8c)
   BT_U32BIT btmem_bm_cbuf;                                          // - 144 (90)
   BT_U32BIT btmem_bm_cbuf_next;                                     // - 148 (94)
   BT_U32BIT btmem_bc;                                               // - 152 (98)
   BT_U32BIT btmem_bc_next;                                          // - 156 (9c)
   BT_U32BIT btmem_tail1;                                            // - 160 (a0)
   BT_U32BIT btmem_pci1553_next;                                     // - 164 (a4)
   BT_U32BIT btmem_pci1553_rt_mbuf;                                  // - 168 (a8)

   /* keep the pointer to the BM buffers last to handle both a 32-bit address and a 64-bit address */
   char *lpAPI_BM_Buffers;                                           // - 172 (ac)
}CHANNEL_SHARE;
#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)
#endif

#define CHANNEL_SHARED 0xca5eba7e //unique
#define CHANNEL_NOT_SHARED 0

#define SHARE_HEADER                     0
#define SHARE_COUNT                      4
#define SHARE_BC_INITED                  8
#define SHARE_BC_RUNNING                12 // (c)
#define SHARE_BM_INITED                 16 // (10)
#define SHARE_BM_RUNNING                20 // (14)
#define SHARE_RT_INITED                 24 // (18)
#define SHARE_RT_RUNNING                28 // (1c)
#define SHARE_PLATFORM                  32 // (20)
#define SHARE_CARDSLOT                  36 // (24) 
#define SHARE_CARRIER                   40 // (28)
#define SHARE_CardType                  44 // (2c)
#define SHARE_INT_ENABLE                48 // (30)
#define SHARE_UCODE_REV                 52 // (34)
#define SHARE_IRIG                      56 // (38) 
#define SHARE_TESTBUS                   60 // (3c) 
#define SHARE_DISCRETES                 64 // (40)
#define SHARE_DIFFERENTIAl              68 // (44)
#define SHARE_ACCESS_32                 72 // (48)
#define SHARE_BC_TIMETAG                76 // (4c)
#define SHARE_485_DISCRETES             80 // (50)
#define SHARE_SERIAL_NUMBER             84 // (54)
#define SHARE_HAS_HWRTADDR              88 // (48)
#define SHARE_HWRTADDR                  92 // (5c)
#define SHARE_CHANNEL_STATUS            96 // (60)
#define SHARE_BT_OP_MODE               100 // (64)
#define SHARE_DEVICE                   104 // (68)                                                 
#define SHARE_INTERRUPT_ENABLE         108 // (6c)
#define SHARE_NUMDISCRETES             112 // (70)
#define SHARE_BT_DISMASK               116 // (74)
#define SHARE_HW_1FUNCTION             120 // (78)
#define SHARE_HW_FPGAREV               124 // (7c)
#define SHARE_HW_PROMREV               128 // (80) 
#define SHARE_HW_WCSREV                132 // (84)
#define SHARE_BTMEM_BM_MBUF            136 // (88)
#define SHARE_BTMEM_BM_MBUF_NEXT       140 // (8c)
#define SHARE_BTMEM_BM_CBUF            144 // (90)
#define SHARE_BTMEM_BM_CBUF_NEXT       148 // (94)
#define SHARE_BTMEM_BC                 152 // (98)
#define SHARE_BTMEM_BC_NEXT            156 // (9c)
#define SHARE_BTMEM_TAIL1              160 // (a0)
#define SHARE_BTMEM_PCI1553_NEXT       164 // (a4)
#define SHARE_BTMEM_PCI1553_RT_MBUF    168 // (a8)
#define SHARE_LPAPI_BM_BUFFERS         172 // (ac)

#define RT_QUIT 1
#define BM_QUIT 2
#define BC_QUIT 4

/**********************************************************************
*  Bus Monitor Definitions
**********************************************************************/
#define BM_INT_ENABLE     0x0
#define BM_INT_DISABLE    0x1000

/************************************************
*  BM Control Buffer -- 4 words in structure, one
*  will be defined for each active subaddress
************************************************/

typedef struct bm_cbuf
   {
   BT_U32BIT wcount;            // enabled word counts or mode codes
   BT_U16BIT pass_count2;       // number of msgs before interrupt (original)
   BT_U16BIT pass_count;        // number of msgs before interrupt (counter)
   }
BM_CBUF;

#define BM_CBUF_SIZE sizeof(BM_CBUF)
#define BM_CBUF_WSIZE BM_CBUF_SIZE/2
#define BM_CBUF_DWSIZE BM_CBUF_SIZE/4

/**********************************************************************
*  BM Message Buffer
*  The first 10 words are independent of the BM message type --
*   after that, the contents of the buffer varies according
*   to the message type and the number of data words.
**********************************************************************/
#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif

typedef struct bm_header
{
#ifdef NON_INTEL_BIT_FIELDS
   BT_U32BIT headerID:16;    //header record id = 0xbeef
   BT_U32BIT bm_unused:7;
   BT_U32BIT bm_trigger:1;   //BM Trigger occurred
   BT_U32BIT byte_count:8;   //Number of bytes in the BM message
#else
   BT_U32BIT byte_count:8;   //Number of bytes in the BM message
   BT_U32BIT bm_trigger:1;   //BM Trigger occurred
   BT_U32BIT bm_unused:7;
   BT_U32BIT headerID:16;    //header record id = 0xbeef
#endif
}BM_HEADER;

typedef struct bm_cmd_word
{
   BT1553_COMMAND command;
   BT_U16BIT status_c;
}BM_CMD_WRD;

typedef struct bm_stat_word
{
   BT1553_STATUS status;
   BT_U16BIT     status_s;
}BM_STAT_WORD;

typedef struct bt1553_data
{
   BT_U16BIT      value;
   BT_U16BIT      msg_quality;
}BT1553_BMDATA;

#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif

typedef struct bm_mbuf
   {
   BT_U16BIT       next_mbuf PACKED;  // Address of next message in chain
   BT_U32BIT       int_enable PACKED; // Interrupt enable bits
   BT_U32BIT       int_status PACKED; // Status bits for this mbuf
   BT1553_V5TIME   time_tag PACKED;   // 45 bit time tag, 1 MHz clock
   BT1553_COMMAND  command1 PACKED;   // Command word #1
   BT_U16BIT       status_c1 PACKED;  // Command word #1 status
   BT_U16BIT       data[74] PACKED;   // The rest of the data
   }
BM_MBUF;

typedef struct bm_v6mbuf
   {
   BM_HEADER       header;     // bm header record                (0-3)         
   BT_U32BIT       int_status; // Status bits for this mbuf       (4-7)
   BT1553_TIME     time_tag;   // 64 bit time tag, 1 MHz clock    (8-15) 
   BM_CMD_WRD      command1;   // Command word #1                 (16-19) 
   BM_CMD_WRD      command2;   //                                 (20-23)
   BT_U32BIT       resp1;      // resp1 in lsb resp2 in msb       (24-27)
   BM_STAT_WORD    status1;    //                                 (28-31)
   BT_U32BIT       resp2;      //                                 (32-35)
   BM_STAT_WORD    status2;    //                                 (36-39)                               
   BT1553_BMDATA   data[33];   // up to 33 word of data           (40-  
   }
BM_V6MBUF;

#define BM_MBUF_SIZE sizeof(BM_V6MBUF)
#define BM_MBUF_WSIZE BM_MBUF_SIZE/2
#define BM_MBUF_DWSIZE BM_MBUF_SIZE/4

#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)        /* Align structure elements to 1 byte boundry */
#endif

/**********************************************************************
*  BM Filter Buffer
**********************************************************************/
typedef struct bm_v6fbuf
    {             //    32         *        2        *      32 = 2048 dwords
    BT_U32BIT addr[BM_ADDRESS_COUNT][BM_TRANREC_COUNT][BM_SUBADDR_COUNT];
    }
BM_V6FBUF;

typedef struct bm_fbuf
    {             //    32         *        2        *      32 = 2048 words
    BT_U16BIT addr[BM_ADDRESS_COUNT][BM_TRANREC_COUNT][BM_SUBADDR_COUNT];
    }
BM_FBUF;

/************************************************
*  Bit definitions for BM_TBUF header word
************************************************/

#define BM_TBUF_TRIG    0x0001  // trigger occurred
#define BM_TBUF_START   0x0002  // enables start trigger
#define BM_TBUF_STOP    0x0004  // enables stop trigger
#define BM_TBUF_EXT     0x0008  // enables external trigger
#define BM_TBUF_INT     0x0010  // (internal use)
#define BM_TBUF_ERROR   0x0020  // trigger on errors
//                      0x0040  // reserved
#define BM_TBUF_OUTPUT  0x0080  // external output on trigger
#define BM_TBUF_EVERY   0x0100  // external output on every BM interrupt

/************************************************
*  Field definitions for BM_TBUF word_type
************************************************/

#define BM_TBUF_TYPE_NULL      0x0000  // no word type
#define BM_TBUF_TYPE_CMD       0x0020  // command
#define BM_TBUF_TYPE_STAT      0x0040  // status
#define BM_TBUF_TYPE_DATA      0x0080  // data
#define BM_TBUF_TYPE_TIME      0x00A0  // time

/************************************************
*  Bit definitions for BM_TBUF control word
************************************************/

#define BM_TBUF_CONT_EVENT_MASK 0x001F  // event occurred indicator
#define BM_TBUF_CONT_OR         0x0000  // 'OR'
#define BM_TBUF_CONT_AND        0x0020  // 0 -> 'OR', 1 -> 'AND'
#define BM_TBUF_CONT_ARM        0x0040  // if set, causes event to ARM next event
#define BM_TBUF_CONT_END        0x0080  // specified last event to be processed

/**********************************************************************
*  Enhanced BM Trigger Buffer (53 words)
**********************************************************************/
typedef struct bm_v6tbuf_enh
   {
   BT_U32BIT trigger_header;            // Trigger header word (See below)

   //  Four words determine logic functions for triggering, and record the events
   //  as they occur for the FW.  The arming words are used anytime triggering
   //  is enabled; the armed words are used only if arming is selected.
   BT_U16BIT start_trigger_event;      // Start arming logic function
   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   start_event[3];                      // Start trigger events

   BT_U32BIT reserved_1[3];             // padding for even dword boundary

   BT_U16BIT stop_trigger_event;        // Stop  arming logic function
   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   stop_event[3];                       // Stop trigger events 

   BT_U32BIT reserved_2[3];             // padding for even dword boundary 
   }
BM_V6TBUF_ENH;

typedef struct bm_tbuf_enh
   {
   BT_U16BIT trigger_header;            // Trigger header word (See below)

   // Four words determine logic functions for triggering, and record the events
   //  as they occur for the FW.  The arming words are used anytime triggering
   //  is enabled; the armed words are used only if arming is selected.
   BT_U16BIT start_trigger_event;      // Start arming logic function
   BT_U16BIT stop_trigger_event;       // Stop  arming logic function

   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   start_event[4];              // Start trigger events
   struct
      {
      BT_U16BIT trigger_offset;         // Offset in BM msg buffer to trigger word
      BT_U16BIT trigger_event_value;    // Compare masked word to this value
      BT_U16BIT trigger_event_bit_mask; // Masks the selected word
      BT_U16BIT trigger_initial_count;  // Initial count of events
      BT_U16BIT trigger_event_count;    // Current count of events
      }
   stop_event[4];               // Stop trigger events 

   BT_U16BIT trig_save[10];  // padding to keep the same size as before
   }
BM_TBUF_ENH;

#define BM_TBUF_ENH_SIZE sizeof(BM_V6TBUF_ENH)
#define BM_TBUF_ENH_WSIZE BM_TBUF_ENH_SIZE/2
#define BM_TBUF_ENH_DWSIZE BM_TBUF_ENH_SIZE/4

/**********************************************************
*  Bit definitions for BM_TBUF_ENH trigger_header word
**********************************************************/

#define BM_ETBUF_INTE   0x0001  // Generate host interrupt & IQ entry on BM message.
                                // Set by API if no triggering, set by the FW when
                                //  all trigger conditions have been met.
#define BM_ETBUF_START  0x0002  // Enables start trigger.
#define BM_ETBUF_STOP   0x0004  // Enables stop trigger.
#define BM_ETBUF_EXT    0x0010  // Enables external trigger input.
//                              //  Start and stop trigger ignored if set.
#define BM_ETBUF_ERROR  0x0020  // Trigger on 1553 words with errors.
//                      0x0040  // Reserved
#define BM_ETBUF_OUTPUT 0x0080  // Enable external TTL output on trigger.
#define BM_ETBUF_EVERY  0x0100  // Enable external TTL output on every BM interrupt.
//                      0xFE00  // Reserved

#define EI_MAX_ERROR_NUM    64 //Number of error injection buffers supported

/************************************************
*  Error injection buffer definition
************************************************/

typedef struct ei_message
    {
    BT_U16BIT data[EI_COUNT];  // 33 words per buffer.
    }
EI_MESSAGE;

#define EI_MESSAGE_SIZE sizeof(EI_MESSAGE)
#define EI_MESSAGE_WSIZE EI_MESSAGE_SIZE/2
#define EI_MESSAGE_DWSIZE EI_MESSAGE_SIZE/4

/************************************************
*  Error injection hardware definitions
************************************************/

#define EI_HW_NONE          0x0000  // no error to inject
#define EI_HW_BITCOUNT      0x0040  // bit count error
#define EI_HW_SYNC          0x0080  // inverted sync error
#define EI_HW_PARITY        0x0100  // parity error
#define EI_HW_DATAWORDGAP   0x0200  // gap between words
#define EI_HW_WORDCOUNT     0x0400  // word count error
#define EI_HW_LATERESPONSE  0x0800  // programmable response time
#define EI_HW_BADADDR       0x1000  // response to wrong address
#define EI_HW_MIDSYNC       0x2000  // mid sync error
#define EI_HW_MIDBIT        0x4000  // mid bit error
#define EI_HW_MIDPARITY     0x6000  // Parity zero crossing
#define EI_HW_RESPWRONGBUS  0x8000  // response on wrong bus
#define EI_HW_BIPHASE       0xC000  // Bi-phase Error
#define EI_HW_ENH_ZEROXNG   0xE000  // RTVAL Tester only
#define EI_HW_TCENH_ZEROXNG 0xA000  // Transition Compensating Enhanced Zero Crossing RTVAL Tester only
#define EI_HW_ENH_MIDSYNC   0xEC04  // Zero-crossing mid sync error
#define EI_HW_ENH_MIDBIT    0xEC00  // Zero-crossing mid bit (need to append half bit value to complete)
#define EI_HW_ENH_MIDPARITY 0xEC28  // Zero-crossing mid parity

#define EI_BYTES 66

/**********************************************************************
*  IQ (Interrupt Queue) definitions
*  Number of hardware interrupt blocks; 3 word per block.
**********************************************************************/

#define IQ_V6_SIZE      512    // Number of interrupt blocks
#define IQ_SIZE         296    // Number of interrupt blocks

/**********************************************************************
*  RT Definitions
**********************************************************************/

/************************************************
*  RT Address Buffer -- 128 words
*  (4 words for each of 32 RT addresses)
************************************************/

typedef struct rt_v6abuf_entry
    {
    BT_U32BIT stat_option;         //  Status word and TRT options
    BT_U32BIT bit_cmd;             //  Bit Word and Last Command
    }
RT_V6ABUF_ENTRY;

typedef struct rt_v6abuf
    {
    RT_V6ABUF_ENTRY aentry[RT_ADDRESS_COUNT];
    }
RT_V6ABUF;

/************************************************
*  RT Address Buffer -- 128 words
*  (4 words for each of 32 RT addresses)
************************************************/

typedef struct rt_abuf_entry
    {
    BT_U16BIT enables;         //  RT specific condition enables
    BT_U16BIT status;          //  RT status word
    BT_U16BIT last_command;    //  Used for transmit last cmd mode code
    BT_U16BIT bit_word;        //  Built-In-Test (BIT) results
    }
RT_ABUF_ENTRY;

typedef struct rt_abuf
    {
    RT_ABUF_ENTRY aentry[RT_ADDRESS_COUNT];
    }
RT_ABUF;

/************************************************
*  Address buffer 'enables' bits
************************************************/

#define RT_MONITOR_MODE    0x0001  // Enable RT Monitor Mode
#define RT_ABUF_DISA       0x0001  // disable channel 'A'
#define RT_ABUF_DISB       0x0002  // disable channel 'B' or enable DBA (WCS >= 308)
#define RT_ABUF_ITF        0x0004  // inhibit terminal flag (both old/new firmware)
#define RT_ABUF_DBC_RT_OFF 0x8000  // Set this bit to shut down RT on valid DBA

#define RT_1553A           0x0040  // 1553 A Mode for a single RT. 
#define RT_1553B           0x0000  // 1553 B Mode for a single RT.


/**********************************************************************
*  RT Control Buffer Block -- 3 words.
*  Pointed to by the RT Filter Buffer.  Changed to 16-bit words.V4.01.ajh
**********************************************************************/
#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif

typedef struct rt_v6cbuf
    {
    BT_U32BIT message_pointer;  // addr of RT message buffer 
    BT_U32BIT legal_wordcount;  // legal word counts/mode codes (bit field 31-16)
    }
RT_V6CBUF;

typedef struct rt_cbuf
    {
    BT_U16BIT legal_wordcount0 PACKED;  // legal word counts/mode codes (bit field 15-1,0)
    BT_U16BIT legal_wordcount1 PACKED;  // legal word counts/mode codes (bit field 31-16)
    BT_U16BIT message_pointer  PACKED;  // addr of RT message buffer 
    }
RT_CBUF;

/************************************************
*  RT Control Buffer Block (broadcast) --
*  63 words for each broadcast subunit
* This structure contains mis-aligned long's!
************************************************/

typedef struct rt_v6cbufbroad
    {
    BT_U32BIT message_pointer;     // address of RT message buffer
    BT_U32BIT legal_wordcount[32]; // legal word count (bit field)
    }
RT_V6CBUFBROAD;

typedef struct rt_cbufbroad
    {
    BT_U16BIT message_pointer     PACKED;     // address of RT message buffer 
    BT_U32BIT legal_wordcount[31] PACKED;     // legal word count (bit field)
    }
RT_CBUFBROAD;

#define RT_CBUFBROAD_SIZE sizeof(RT_V6CBUFBROAD)
#define RT_CBUFBROAD_WSIZE RT_CBUFBROAD_SIZE/2
#define RT_CBUFBROAD_DWSIZE RT_CBUFBROAD_SIZE/4

#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)        /* Align structure elements to 1 byte boundry */
#endif

/************************************************
*  RT Filter Buffer, located on 2048 word boundry
*   in BusTools board memory.
************************************************/

typedef struct rt_v6fbuf
    {         //         32        *        2        *      32 = 4096 Words
    BT_U32BIT addr[RT_ADDRESS_COUNT][RT_TRANREC_COUNT][RT_SUBADDR_COUNT];
    }
RT_V6FBUF;

typedef struct rt_fbuf
    {         //         32        *        2        *      32 = 2048 Words
    BT_U16BIT addr[RT_ADDRESS_COUNT][RT_TRANREC_COUNT][RT_SUBADDR_COUNT];
    }
RT_FBUF;

/**********************************************************************
*  RT Message Buffer is composed of a hardware portion which is used
*   by the microcode, and an SW/API portion used only by the API.
*  Note that the hardware portion cannot be shared by both receive
*   and transmit, since the RT Status word gets stored at
*   different offsets in the buffer.  But the data is always in the
*   same relative locations in either type of buffer.
***********************************************************************/

/**********************************************************************
*  RT Message Buffer (hardware)
**********************************************************************/
#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif
typedef struct rt_mbuf_v6hw
    {
    // The following words are defined by the microcode:
    BT_U32BIT   nxt_msg_ptr;      // Address of next message 
    BT_U32BIT   ei_ptr;           // error injection message address (lower 64Kw only)
    BT_U32BIT   enable;           // RT interrupt enable bits
    BT_U32BIT   status;           // bits indicating what caused the interrupt
    BT1553_TIME   time_tag;       // 64 bit time tag

    BT1553_COMMAND mess_command  PACKED;   // 1553 command word
    BT1553_STATUS  mess_status   PACKED;   // 1553 status word (Transmit) (reserved Receive)
    BT_U16BIT      mess_data[33] PACKED;   // 1553 data words
    BT1553_STATUS  ext_status    PACKED;   // 1553 status word (Receive)
    }
RT_MBUF_V6HW;     // 48 words
#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)        /* Align structure elements to 1 byte boundry */
#endif

#ifdef PRAGMA_PACK
#pragma pack(push, _BUSAPI_PACK, 2)        /* Align structure elements to 2 byte boundry */
#endif
typedef struct rt_mbuf_hw
    {
    // The following words are defined by the microcode:
    BT_U16BIT   nxt_msg_ptr PACKED;  // Address of next message 
    BT_U16BIT   ei_ptr PACKED;       // error injection message address (lower 64Kw only)
    BT_U32BIT   enable PACKED;       // RT interrupt enable bits
    BT_U32BIT   status PACKED;       // bits indicating what caused the interrupt
    BT1553_V5TIME time_tag PACKED;   // 45 bit time tag, (1 MHz Clock)

    BT1553_COMMAND mess_command PACKED;  // 1553 command word
    BT1553_STATUS  mess_status1 PACKED;  // 1553 status word (Transmit) (reserved Receive)
    BT_U16BIT      mess_data[32] PACKED; // 1553 data words
    BT1553_STATUS  mess_status2 PACKED;  // 1553 status word (Receive)
    }
RT_MBUF_HW;     // 44 words
#ifdef PRAGMA_PACK
#pragma pack(pop, _BUSAPI_PACK)        /* Align structure elements to 1 byte boundry */
#endif


#define RT_MBUF_HW_SIZE    sizeof(RT_MBUF_V6HW)
#define RT_MBUF_HW_DSIZE   RT_MBUF_HW_SIZE/2
#define RT_MBUF_HW_DWSIZE  RT_MBUF_HW_SIZE/4

typedef struct rt_mbuf_api
    {
    // The following words are used ONLY by the API, NOT the microcode:
    BT1553_COMMAND mess_id;         // API RT#, T/R, SA#, Status word code.
    BT_U16BIT      mess_bufnum;     // API buffer number.
    BT_U16BIT      mess_statuswd;   // API 1553 Status Word.
    BT_U16BIT      mess_verify;     // API verification word (RT_VERIFICATION).
    }
RT_MBUF_API;    // 4 words-->52 words total

#define RT_MBUF_API_SIZE sizeof(RT_MBUF_API)
#define RT_MBUF_API_DSIZE RT_MBUF_API_SIZE/2
#define RT_MBUF_API_DWSIZE RT_MBUF_API_SIZE/4

#define RT_SET_RESERVE        0x8001   /* Used in mess_statuswd to set bits into 1553 */
#define RT_NOCHANGE_RESERVE   0x0001   /*  status word when msg is read or written.   */
#define RT_STATUSMASK     0x071F  /* Defines bits copied into RT status word. Added BCR bit 10/28/98 */
#define RT_STATUSMASK_RES 0x07FF  /* Bits settable in RT status word in ABUF. Added BCR bit 10/28/98 */
#define RT_VERIFICATION   0xBABE  /* Value of mess_verify indicates valid MBUF*/

typedef struct rt_v6mbuf
    {
    RT_MBUF_V6HW     hw;      // 48 words defined by the firmware.
    RT_MBUF_API      api;     //  4 words defined by the API.
    }
RT_V6MBUF;                    // 52 words

typedef struct rt_mbuf
    {
    RT_MBUF_HW     hw;      // 44 words defined by the firmware.
    RT_MBUF_API    api;     //  4 words defined by the API.
    }
RT_MBUF;                    // 48 words

#define RT_MBUF_SIZE sizeof(RT_V6MBUF)
#define RT_MBUF_WSIZE RT_MBUF_SIZE/2
#define RT_MBUF_DWSIZE RT_MBUF_SIZE/4

/************************************************************************
* V5 Hardware register WORD offsets;
************************************************************************/
#define HWREG_CONTROL1         0x00  // control register #1                  (R/W)
#define HWREG_CONTROL_T_TAG    0x01  // Control the time tag cntr            (RO).V4.28
#define HWREG_CLRTAG           0x02  // Clear time tag counter               (WO)
#define HWREG_PB_CONTROL       0x03  // Playback Control Register            (RW)
#define HWREG_RESPONSE         0x04  // response time out/late response      (WO)
#define HWREG_BC_EXT_SYNC      0x06  // Enable external TTL BC sync          (WO)
#define HWREG_PB_CLEAR         0x07  // Playback Status Bit Clear            (WO)
#define HWREG_PCI_VOLTAGE      0x08  // Set output voltage                   (WO)
#define HWREG_LPU_REVISION     0x09  // Read LPU Revision information        (RO).V4.31
#define HWREG_INTERRUPT_VEC    0x0A  // Set IP-D1553 interrupt vector        (WO)
#define HWREG_WRITE_INTERRUPT  0x0B  // Set/Clr bit 9 to set/clear interrupt (RW)
#define HWREG_READ_T_TAG       0x0C  // Read the current time tag counter val(RW).V4.28
#define HWREG_READ_T_TAG386    0x15  // Read the current time tag counter LPU 386 or greater
#define HWREG_MINOR_FRAME      0x0E  // Minor Frame interval, 25 us LSB      (WO)
#define HWREG_SRT_ADDR         0x1b  // Single-RT address                    (WR)
#define HWREG_MINOR_FRAME_LSB  0x1c  // Minor Frame interval, 1 us LSB       (WO)
#define HWREG_MINOR_FRAME_MSB  0x1d  // Minor Frame interval, 1 us LSB       (WO)
#define HWREG_CONTROL2         0x0F  // 2nd HWcontrol Register               (RW)
#define HWREG_WCS_REVISION     0x18  // WCS Revision regiseter               (RO)
#define HWREG_HEART_BEAT       0x19  // Heart Beat Register                  (RO)
#define HWREG_LPU_BUILD_NUMBER 0x1a  // LPU Build number                     (RO)

#define HWREG_RT_MODE1         0x20  // Specific which mode (A or B) for RT 0 - 15 (RW)
#define HWREG_RT_MODE2         0x21  // Specific which mode (A or B) for RT 16 - 31(RW) 

#define HWREG_COUNT            0x10  // size of hw register area in WORDS.V3.30.ajh
#define HWREG_COUNT2           0x30  // Real size 

/*****************************************************************************
*  Playback File Registers
*  WORD offsets from the beginning file register offset.
******************************************************************************/
#define RAMREG_DQ_RETRY         0x07       // mask for the Interrupt Status Word 1 bits

#define PB_START_POINTER        0x10       // Address of Playback Data Buffer
#define PB_END_POINTER          0x11       // End of Playback Data Buffer
#define PB_TAIL_POINTER         0x12       // Current FW loc in PB Data Buffer
#define PB_HEAD_POINTER         0x13       // Addr where host is adding data
#define PB_INT_THRSHLD          0x14       // Number of words between FW interrupts
#define PB_INT_STATUS           0x15       // Number of FW interrupts generated
#define PB_THRSHLD_CNT          0x16       // Interrupt counter used by FW
#define PB_CURRENT_WORD         0x17       // Reserved for the Playback firmware

#define RAMREG_HIGHAPTR         0x2A       // Addr of high-priority aperiodic msg list (RW)
#define RAMREG_LOWAPTR          0x2B       // Addr of low-priority aperiodic msg list  (RW)
#define RAMREG_HIGHAFLG         0x2E       // Flags used for high-priority aperiodics
#define RAMREG_LOW_AFLG         0x2F       // Flags used for low-priority aperiodics
#define RAMREG_RT_DISA          0x34       // RT Chan A Disables, registers 34-35
#define RAMREG_RT_DISB          0x36       // RT Chan B Disables, registers 36-37
#define RAMREG_CLRWORD          0x38       // Microcode constant register 38
#define RAMREG_SETWORD          0x39       // Microcode constant register 39
#define RAMREG_RETRYPTR         0x3A       // Microcode constant register 3A
#define RAMREG_INTBUSTRG        0x3B       // Microcode constant register 3B
#define RAMREG_RTMONITR         0x3C       // Microcode constant register 3C
#define RAMREG_MASK001F         0x3D       // Microcode constant register 3D
#define RAMREG_MASK003F         0x3E       // Microcode constant register 3E
#define RAMREG_RTMONITT         0x3F       // Microcode constant register 3F = 0x12 LPU V3.05+(V4.06)
#define RAMREG_BC_MSG_PTR      (0x40+0x00) // BC message pointer
#define RAMREG_BC_MSG_PTRSV    (0x40+0x01) // BC message pointer save
#define RAMREG_BC_DATA_PTR     (0x40+0x02) // BC data pointer
#define RAMREG_BC_INT_ENB1     (0x40+0x03) // BC interrupt enables 1
#define RAMREG_BC_INT_ENB2     (0x40+0x04) // BC interrupt enables 2
#define RAMREG_BC_RETRY        (0x40+0x05) // BC retry enable information
#define RAMREG_BCMSGPTR1       (0x40+0x06) // minor frame time register
#define RAMREG_BCMSGPTR2       (0x40+0x07) // minor frame counter
#define RAMREG_RT_CONT_PTR     (0x40+0x08) // RT control pointer
#define RAMREG_RT_MSG_PTR      (0x40+0x09) // RT message pointer
#define RAMREG_RT_MSG_PTRSV    (0x40+0x0A) // RT message pointer save
#define RAMREG_RT_MSG_PTR2     (0x40+0x0B) // RT message pointer 2 (RT to RT)
#define RAMREG_BM_PTR_SAVE     (0x40+0x0C) // BM pointer save (initialize by host)
#define RAMREG_BM_MBUF_PTR     (0x40+0x0D) // BM message pointer (used by firmware)
#define RAMREG_BC_OUT_WC       (0x40+0x0E) // output word count
#define RAMREG_BC_DAT_WC       (0x40+0x0F) // input word count
#define RAMREG_RT_ERRINJ1      (0x40+0x10) // RT error injection pointer 1
#define RAMREG_RT_ERRINJ2      (0x40+0x11) // RT error injection pointer 2
#define RAMREG_MPROC_ADDR      (0x40+0x12) // 
#define RAMREG_INT_QUE_PTR     (0x40+0x13) // HW interrupt queue pointer
#define RAMREG_RT_ABUF_PTR     (0x40+0x14) // RT address buffer base address
#define RAMREG_RT_FBUF_PTR     (0x40+0x15) // RT filter base address
#define RAMREG_BM_FBUF_PTR     (0x40+0x16) // BM filter base address
#define RAMREG_BM_TBUF_PTR     (0x40+0x17) // BM trigger event pointer
#define RAMREG_BC_RETRYPTR     (0x40+0x18) // BC retry buffer pointer
#define RAMREG_MESS_ERROR1     (0x40+0x19) // message error 1
#define RAMREG_MESS_ERROR2     (0x40+0x1A) // message error 2
#define RAMREG_ORPHAN          (0x40+0x1B) // orphaned hardware bits
#define RAMREG_MODECODE1       (0x40+0x1C) // Undefined mode code #1
#define RAMREG_MODECODE2       (0x40+0x1D) // Undefined mode code #2
#define RAMREG_MODECODE3       (0x40+0x1E) // Undefined mode code #3 
#define RAMREG_MODECODE4       (0x40+0x1F) // Undefined mode code #4

#define RAMREG_REG60           (0x40+0x20) // Microcode constant register 60
#define RAMREG_REG61           (0x40+0x21) // Microcode constant register 61
#define RAMREG_REG62           (0x40+0x22) // Microcode constant register 62
#define RAMREG_REG63           (0x40+0x23) // Microcode constant register 63
#define RAMREG_REG64           (0x40+0x24) // Microcode constant register 64
#define RAMREG_REG56           (0x40+0x25) // Microcode constant register 65
#define RAMREG_REG66           (0x40+0x26) // Microcode constant register 66
#define RAMREG_REG67           (0x40+0x27) // Microcode constant register 67
#define RAMREG_REG68           (0x40+0x28) // Microcode constant register 68
#define RAMREG_REG69           (0x40+0x29) // Microcode constant register 69
#define RAMREG_REG6A           (0x40+0x2A) // Microcode constant register 6A
#define RAMREG_REG6B           (0x40+0x2B) // Microcode constant register 6B
#define RAMREG_REG6C           (0x40+0x2C) // Microcode constant register 6C
#define RAMREG_REG6D           (0x40+0x2D) // Microcode constant register 6D
#define RAMREG_REG6E           (0x40+0x2E) // Microcode constant register 6E
#define RAMREG_REG6F           (0x40+0x2F) // Microcode constant register 6F
#define RAMREG_REG70           (0x40+0x30) // Microcode constant register 70
#define RAMREG_REG71           (0x40+0x31) // Microcode constant register 71
#define RAMREG_REG72           (0x40+0x32) // Microcode constant register 72
#define RAMREG_REG73           (0x40+0x33) // Microcode constant register 73
#define RAMREG_REG74           (0x40+0x34) // Microcode constant register 74
#define RAMREG_REG75           (0x40+0x35) // Microcode constant register 75
#define RAMREG_REG76           (0x40+0x36) // Microcode constant register 76
#define RAMREG_REG77           (0x40+0x37) // Microcode constant register 77
#define RAMREG_FLAGS           (0x40+0x38) // Microcode flags
#define RAMREG_ENDFLAGS        (0x40+0x39) // Microcode end flags
#define RAMREG_BC_CONTROL_SAVE (0x40+0x3A) // Microcode saved BM control word
#define RAMREG_CURRENT_WORD    (0x40+0x3B) // Microcode temp
#define RAMREG_DECODER_WORD    (0x40+0x3C) // Microcode temp
#define RAMREG_DECODER_MSG_FMT (0x40+0x3D) // Microcode temp
#define RAMREG_TEMP            (0x40+0x3E) // Microcode temp
#define RAMREG_RESERVED8       (0x40+0x3F) // Reserved

#define RAMREG_COUNT            128        // Number of RAM Registers

/* Time Tag registers */
#define TTREG_CONTROL   0x0
#define TTREG_LOAD_LOW  0x1
#define TTREG_LOAD_HIGH 0x2
#define TTREG_INCREMENT 0x3
#define TTREG_LATCH     0x4
#define TTREG_READ_LOW  0x5
#define TTREG_READ_HIGH 0x6

#define CREG_DAC_CONTROL  0xD8

/* Trigger Registers    */
#define TRIG_IN_SEL           0  
#define TRIG_DIS_OUT_SEL      4
#define TRIG_DIF_OUT_SEL      8  

#define DISCRETE_TRIGGER     0x100
#define DIFFERENTIAL_TRIGGER 0x200
/******************************************************
*  Hardware 1553_control_reg1 BIT definitions
******************************************************/
#define CR1_SMODE        0x0001       // Singal Mode Warning
#define CR1_IMW	         0x00000001   // Illegal Mode Warning (F/W 5.0)
#define CR1_BCRUN        0x00000002   // start bc running
#define CR1_RTRUN        0x00000004   // start rt running
#define CR1_BMRUN        0x00000008   // start bm running
#define CR1_EXT_TTL      0x00000010   // Set the external TTL Output
#define CR1_BCBUSY       0x00000020   // bc is busy
#define CR1_1553A        0x00000040   // 1553 A Mode
#define CR1_BC_EXT_SYNC  0x00000080   // external BC sync enabled (RO)
#define CR1_IN_WRAP      0x00000100   // internal wrap (disable external 1553 bus)
#define CR1_HOST_INTERP  0x00000200   // host interrupt is asserted  
#define CR1_RT31BCST     0x00000400   // rt 31 broadcast
#define CR1_SA31MC       0x00000800   // subaddress 31 mode code
#define CR1_TESTBUS      0x00001000   // Test Bus Setting
#define CR1_RTEXSYN      0x00002000   // rt external sync enable
#define CR1_INT_ENABLE   0x00004000   // Set bit to enable HW interrupts 
#define CR1_BUS_COUPLE   0x00008000   // Enable transformer coupling
#define CR1_FAC_RESERVE1 0x00010000
#define CR1_FAC_RESERVE2 0x00020000

#define CR1_EN_TRIG_INT  0x00100000   // Enable input trigger itnerrupt
#define CR1_MSG_SCHD     0x00200000   // Enable Message Scheduling
#define CR1_FIXED_GAP    0x00400000   // Fixed Gap message timing
#define CR1_MON_INV_CMD  0x00800000   // No frame end predition
#define CR1_UND_MC_ILL   0x01000000   // Undefined Mode Codes Illegal
#define CR1_SCHIP_FAIL   0x02000000   // Security chip fail
#define CR1_AMFO         0x04000000   // Allow message to overflow frame
#define CR1_FR_STRT_TIME 0x08000000   // Use Frame Start Timing option
#define CR1_IGNORE_HWD   0x10000000   // Not used
#define CR1_SRT_VAL      0x20000000   // RT Val mode (read only)
#define CR1_BM_ONLY      0x40000000   // BM Only (read only)
#define CR1_BUS_TESTER   0x80000000   // Bus Tester (read only)

#define CR2_EN_TRIG_INT  0x0010   // Enable input trigger itnerrupt
#define CR2_MSG_SCHD     0x0020   // Enable Message Scheduling
#define CR2_FIXED_GAP    0x0040   // Fixed Gap message timing
#define CR2_NO_PREDICT   0x0400   // No frame end predition
#define CR2_FR_STRT_TIME 0x0800   // Use Frame Start Timing option
#define CR2_EXTD_TIME    0x1000   // Use BC extended gap and frame timing 
#define CR2_MON_INV_CMD  0x0080   // Monitor Invalid Commands
#define CR2_UND_MC_ILL   0x0100   // Undefined Mode Codes Illegal
#define CR2_SCHIP_FAIL   0x0200   // Security chip fail
#define CR2_AMFO         0x0400   // Allow message to overflow frame
#define CR2_FRM_ST_TMG   0x0800   // Frame Start Timing
#define CR2_32BIT_TIME   0x1000   // Enable 32-bit timing
#define CR2_SRT_VAL      0x2000   // RT Val mode (read only)
#define CR2_BM_ONLY      0x4000   // BM Only (read only)
#define CR2_BUS_TESTER   0x8000   // Bus Tester (read only)

/******************************************************
*  Paged Hardware I/O Register bit definitions
******************************************************/
#define IO_CONFIG_WCS  0x2000   // Load the WCS into the board
#define IO_RUN_STATE   0x0000   // Transition board to runable state

/*******************************************************************
*  Discrete register defines
********************************************************************/
#define DISREG_V6DIS_OUT        0xc0
#define DISREG_V6DIS_IN         0xc4
#define DISREG_485_V6XMT_CTL    0xc8
#define DISREG_485_V6DATA       0xcc
#define DISREG_EXT_V6RTADDR     0xd0
#define DISREG_DAC_V6CTL        0xd8

#define V6_RT_ADDR_MASK         0x1f
#define V6_HW_RT_ADDR_VALID     0x20
         
#define DISREG_DIS_OUT1         0x84
#define DISREG_DIS_OUT2         0x85
#define DISREG_DIS_OUTEN1       0x86
#define DISREG_DIS_OUTEN2       0x87
#define DISREG_HW_RTADDR        0x88
#define DISREG_TRIG_CLK         0x89
#define DISREG_DIFF_IO          0x8a
#define DISREG_TERM_EN          0x8b
#define DISREG_DIS_IN1          0x8c
#define DISREG_DIS_IN2          0x8d 
#define DISREG_RTADDR_RD1       0x8e
#define DISREG_RTADDR_RD2       0x8f
#define RS485_TX_DATA           0x92
#define RS485_TX_ENABLE         0x93
#define RS485_RX_DATA           0x94
#define DAC_VALID               0x97
#define EXT_TRIG_OUT_CH1        0x98
#define EXT_TRIG_OUT_CH2        0x99
#define EXT_TRIG_OUT_CH3        0x9a
#define EXT_TRIG_OUT_CH4        0x9b

/*  v6 Trigger controls registers   */
#define EXT_TRGIN_CTRL          0x0
#define EXT_TRGOUT_CTRL         0x1
#define EXT_DIFFOUT_CTRL        0x2

typedef struct v6_trig_ctrl
{
   BT_U32BIT trigCH   :5;        //Selects the discrete channel used for input or output trigger
   BT_U32BIT res1     :3;
   BT_U32BIT trigSrc  :2;
   BT_U32BIT dedi     :1;
   BT_U32BIT res2     :21;
}V6_TRIG_CTRL;

#define HW_RT_ADDR_NOT_IN_USE 0x40
/*****************************************
*  RAM START AND SIZE registers 
*****************************************/
#define USER_RAM_START  0x88
#define USER_RAM_AMOUNT 0x8C


/* RXMC-1553 output options         */
#define NO_OUTPUT		   0
#define PIO_OPN_GRN		1
#define PIO_28V_OPN		2
#define DIS_OPN_GRN		3
#define DIS_28V_OPN		4
#define EIA485_OPN_GRN	5
#define EIA485_28V_OPN	6
#define EIA485_OPN_GRN_DIS8	7

/************************************************
*  Misc Bit definitions for RAM registers
************************************************/
#define BIT00 0x00000001 /* Generic LSB definition */
#define BIT01 0x00000002
#define BIT02 0x00000004
#define BIT03 0x00000008
#define BIT04 0x00000010
#define BIT05 0x00000020
#define BIT06 0x00000040
#define BIT07 0x00000080
#define BIT08 0x00000100
#define BIT09 0x00000200
#define BIT10 0x00000400
#define BIT11 0x00000800
#define BIT12 0x00001000
#define BIT13 0x00002000
#define BIT14 0x00004000
#define BIT15 0x00008000 
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000 /* Generic MSB definition */

/************************************************************
*  Bit definitions for RAMREG_ORPHAN (Now named F/W control)
*************************************************************/
#define RAMREG_ORPHAN_BM_DISABLE_A        0x0004  // F/W v4.99 and earlier
#define RAMREG_ORPHAN_BM_DISABLE_B        0x0008  // F/W v4.99 and earlier
#define RAMREG_ORPHAN_RESET_TIMETAG       0x0010
#define RAMREG_ORPHAN_FIRE_EXT_ON_SYNC_MC 0x0040
#define RAMREG_ORPHAN_MINORFRAME_OFLOW    0x0400

/************************************************
*  Bit definitions for Firmware Control
************************************************/
#define FW_MINORFRAME_OFLOW_ENA             0x0001
#define FW_RESET_TIMETAG                    0x0010
#define FW_FIRE_EXT_ON_SYNC_MC              0x0040


/**********************************************************************
*  Internal API procedure definitions (various files).
**********************************************************************/
void      api_sethwcbits    (BT_UINT cardnum,BT_U32BIT value);
void      api_clearhwcbits  (BT_UINT cardnum,BT_U32BIT value);      

BT_U16BIT api_readhwreg     (BT_UINT cardnum,BT_UINT regnum);
void      api_writehwreg    (BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);
void      api_writehwreg_or (BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);
void      api_writehwreg_and(BT_UINT cardnum,BT_UINT regnum,BT_U16BIT value);


void BusTools_EI_Init(BT_UINT cardnum);
void BusTools_InitSeg1 (BT_UINT cardnum);
void BusTools_InitV6Seg1 (BT_UINT cardnum);

void TimeTagClearFlag(BT_UINT cardnum);
void TimeTagConvert(  BT_UINT cardnum,
                      BT1553_V5TIME *time_tag,
                      BT1553_TIME  *time_actual);

BT_U32BIT TimeTagInterrupt(BT_UINT cardnum);
void TimeTagZeroModule(BT_UINT   cardnum);

void BM_V5MsgReadBlock(BT_UINT cardnum);
void BM_V6MsgReadBlock(BT_UINT cardnum);
BT_U32BIT RT_CbufbroadAddr(BT_UINT cardnum, BT_UINT sa, BT_UINT tr);


/**********************************************************************
*  OS-specific memory mapping functions (MEM*.C).
**********************************************************************/

#ifdef INCLUDE_VMIC
BT_INT vbtMapVMICAddress(BT_UINT cardnum, BT_U32BIT baseaddr,
                         BT_INT  ioaddress);
DWORD vbtFreeVMICAddress(BT_UINT cardnum);
#endif //_VMIC_

/**********************************************************************
*  Interrupt timing and BM recording time-out intervals.  The default
*   polling interval is 10 milliseconds.
**********************************************************************/
// Not all processors support less than a 10 ms polling interval(?).
#define MIN_POLLING          1    /* Min polling resolution/interval, in ms */
#define MAX_POLLING        100    /* Max polling resolution/interval, in ms */
#define BM_RECORD_TIME     500    /* BM recording interval in milliseconds  */

/**********************************************************************
*  Internal low-level API procedure definitions.
**********************************************************************/
// Memory test error reporting function
void API_ReportMemErrors(BT_UINT cardnum, int type,
                         BT_U32BIT base_address, BT_U32BIT length,
                         BT_U16BIT * bufout, BT_U16BIT * bufin,
                         BT_U16BIT * buf2, int * first_time);

void API_ReportMemErrors32(BT_UINT cardnum, int type,
                         BT_U32BIT base_address, BT_U32BIT length,
                         BT_U32BIT * bufout, BT_U32BIT * bufin,
                         BT_U32BIT * buf2, int * first_time);

BT_INT  vbtSetup(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT baseaddr,            // (i) 32 bit base address
   BT_U32BIT ioaddr,              // (i) io address
   BT_UINT   flag);               // (i) hw/sw flag: 0 -> sw, 1 -> hw

LPSTR vbtGetPagePtr(
   BT_UINT   cardnum,             // (i) card number.
   BT_U32BIT byteOffset,          // (i) offset within adapter memory.
   BT_UINT   bytesneeded,         // (i) number of bytes needed in page.
   LPSTR     local_board_buffer); // (io) scratch buffer V4.01

BT_U16BIT vbtGetRegister16(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetCSCRegister16(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);        // (i) register number, WORD offset

BT_U16BIT vbtGetFileRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetHWRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U16BIT vbtGetDiscrete16(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

void vbtSetFileRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U16BIT regval);

void vbtSetHWRegister(
   BT_UINT   cardnum,            // (i) card number (0 based)
   BT_U32BIT regnum,             // (i) register number, WORD offset
   BT_U16BIT regval);            // (i) new value

/****************** HW Register Get functions *********************/
BT_U32BIT (*vbtGetRegister[MAX_BTA])(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U32BIT v6GetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U32BIT usbGetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number
/*******************************************************************/
/****************** IQ Header Pointer functions ********************/
BT_U32BIT (*vbtGetIqHeadPtr[MAX_BTA])(
   BT_UINT cardnum);      // (i) card number (0 based)

BT_U32BIT v6GetIqHeadPtr(
   BT_UINT cardnum);       // (i) card number (0 based)

BT_U32BIT usbGetIqHeadPtr(
   BT_UINT cardnum);       // (i) card number (0 based)
/*******************************************************************/
/****************** BM Header Pointer functions ********************/
BT_U32BIT (*vbtGetBMHeadPtr[MAX_BTA])(
   BT_UINT cardnum);      // (i) card number (0 based)

BT_U32BIT v6GetBMHeadPtr(
   BT_UINT cardnum);       // (i) card number (0 based)

BT_U32BIT usbGetBMHeadPtr(
   BT_UINT cardnum);       // (i) card number (0 based)
/*******************************************************************/
/************************HW Register Set Functions *****************/
void (*vbtSetRegister[MAX_BTA])(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value

void v6SetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value

void v5SetRegister(
   BT_UINT   cardnum,            // (i) card number (0 based)
   BT_U32BIT regnum,             // (i) register number, WORD offset
   BT_U32BIT regval);            // (i) new value

void usbSetRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value
/******************************************************************/
/***********************CSC Register Get functions ***************/
BT_U32BIT (*vbtGetCSCRegister[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);        // (i) register number, WORD offset 

BT_U32BIT usbGetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);        // (i) register number, WORD offset 

BT_U32BIT v6GetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);        // (i) register number, WORD offset 

BT_U32BIT v5GetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);       // (i) register number, WORD offset
/*******************************************************************/
/***********************CSC Register Set functions******************/
void (*vbtSetCSCRegister[MAX_BTA])	(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);

void v6SetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);

void usbSetCSCRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);
/******************************************************************/

/****************** Trigger Register Get functions ****************/
BT_U32BIT (*vbtGetTrigRegister[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);       // (i) register number, WORD offset

BT_U32BIT v6GetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);       // (i) register number, WORD offset

BT_U32BIT usbGetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum);       // (i) register number, WORD offset
/*****************************************************************/
/******************* Trigger Register Set Functions **************/
void (*vbtSetTrigRegister[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);       // (i) new value

void v6SetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);       // (i) new value

void usbSetTrigRegister(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);       // (i) new value
/******************************************************************/
/*******************Time Tag Register Get Functions ***************/
BT_U32BIT (*vbtGetTTRegister[MAX_BTA])(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U32BIT v6GetTTRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number

BT_U32BIT usbGetTTRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum);             // (i) register number
/*****************************************************************/
/***************** Time Tag Register Set Functions ***************/
void (*vbtSetTTRegister[MAX_BTA])(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value

void v6SetTTRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value

void usbSetTTRegister(
   BT_UINT   cardnum,             // (i) card number
   BT_U32BIT regnum,              // (i) register number, offset
   BT_U32BIT regval);             // (i) new value
/*****************************************************************/
/*********************** HIF Read Functions **********************************/
void (*vbtReadGLBR[MAX_BTA])(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void v6ReadHIF(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void usbReadHIF(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy
/****************************************************************************/
/********************* Global Register Write Functions **********************************/
void (*vbtWriteGLBR[MAX_BTA])(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (i) host buffer (source)
   BT_U32BIT byteOffset,     // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite);  // (i) number of dwords to copy

void v6WriteHIF(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (i) host buffer (source)
   BT_U32BIT byteOffset,     // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite);  // (i) number of dwords to copy

void usbWriteHIF(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT *lpbuffer,      // (i) host buffer (source)
   BT_U32BIT byteOffset,     // (o) byte offset within adapter (destination)
   BT_UINT   wordsToWrite);  // (i) number of dword to copy
/****************************************************************************/
/******************************* Discrete Registers *************************/
BT_U32BIT (*vbtGetDiscrete[MAX_BTA])(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum);         // (i) register number

BT_U32BIT v6GetDiscrete(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum);         // (i) register number

BT_U32BIT v5GetDiscrete(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum);         // (i) register number

BT_U32BIT usbGetDiscrete(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum);         // (i) register number
/******************************************************************************/

void (*vbtSetDiscrete[MAX_BTA])(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum,          // (i) register number, offset
   BT_U32BIT regval);         // (i) new value

void v6SetDiscrete(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum,          // (i) register number, offset
   BT_U32BIT regval);         // (i) new value

void v5SetDiscrete(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_U32BIT regnum,        // (i) register number, WORD offset
   BT_U32BIT regval);        // (i) new value

void usbSetDiscrete(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT regnum,          // (i) register number, offset
   BT_U32BIT regval);         // (i) new value
/******************************************************************************/
void (*vbtReadTimeTag[MAX_BTA])(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT * timetag);      // (o) resulting 48-bit time value from HW

void v5ReadTimeTag(
   BT_UINT   cardnum,         // (i) card number (0 based)
   BT_U32BIT * timetag);      // (o) resulting 48-bit time value from HW

void v6ReadTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT * timetag);      // (o) resulting 48-bit time value from HW

void usbReadTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT_U32BIT * timetag);      // (o) resulting 48-bit time value from HW

/********************** Time Tag Write functions ***************************/
void (*vbtWriteTimeTag[MAX_BTA])(
   BT_UINT   cardnum,         // (i) card number
   BT1553_TIME * timetag);    // (i) 48-bit time value to load into register

void v5WriteTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT1553_TIME * timetag);    // (i) 48-bit time value to load into register

void v6WriteTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT1553_TIME * timetag);    // (i) 48-bit time value to load into register

void usbWriteTimeTag(
   BT_UINT   cardnum,         // (i) card number
   BT1553_TIME * timetag);    // (i) 48-bit time value to load into register
/********************* Time Tag Read functions ******************************/

void (*vbtWriteTimeTagIncr[MAX_BTA])(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT incr);          // (i) 32-bit time increment

void v5WriteTimeTagIncr(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT incr);          // (i) increment value

void v6WriteTimeTagIncr(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT incr);          // (i) 32-bit time increment

void usbWriteTimeTagIncr(
   BT_UINT   cardnum,        // (i) card number
   BT_U32BIT incr);          // (i) 32-bit time increment
/****************************************************************************/
void (*vbtReadRAM[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void v6ReadRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void v5ReadRAM(
   BT_UINT   cardnum,        // (i) card number
   BT_U16BIT *lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void usbReadRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy
/*******************************************************************************/
/****************************************************************************/
void (*vbtReadBMRAM[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void usbReadBMRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy
/*******************************************************************************/

void (*vbtReadRelRAM[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void v6ReadRelRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void usbReadRelRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

/************************ 32-BIT RAM Read *************************************/
void (*vbtReadRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void v6ReadRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void v6ReadRAM32S(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void v5ReadRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void usbReadRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy
/*******************************************************************************/
/************************ 32-BIT BM RAM Read *************************************/
void (*vbtReadBMRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void usbReadBMRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

/*******************************************************************************/
/************************ 32-BIT BM RAM Read *************************************/
void (*vbtWriteBMRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numWords);     // (i) number of bytes to copy

void usbWriteBMRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

/*******************************************************************************/

/************************ Read Interrupt Queue *********************************/
void (*vbtReadIntQueue[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset);    // (i) memory offset within adapter memory (source)

void v6ReadIntQueue(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset);    // (i) memory offset within adapter memory (source)

void usbReadIntQueue(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset);     // (i) memory offset within adapter memory (source)
/*******************************************************************************/

void (*vbtReadRelRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void v6ReadRelRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void usbReadRelRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy
/********************************************************************************/
void (*vbtWriteRAM[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void v6WriteRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void v5WriteRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void usbWriteRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy
/*******************************************************************************/
void (*vbtWriteRelRAM[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void v6WriteRelRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void usbWriteRelRAM(
   BT_UINT   cardnum,       // (i) card number
   BT_U16BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy
/******************************************************************************/
void (*vbtWriteRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void v6WriteRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void v6WriteRAM32S(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void v5WriteRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void usbWriteRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy
/********************************************************************************/
void (*vbtWriteRelRAM32[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void v6WriteRelRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void usbWriteRelRAM32(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy
/***********************************************************************************/
/********************************************************************************/
void (*vbtWriteSharedMemory[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void v6WriteSharedMemory(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy

void usbWriteSharedMemory(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numWords);     // (i) number of bytes to copy
/***********************************************************************************/
/*******************************************************************************/

void (*vbtReadSharedMemory[MAX_BTA])(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead);  // (i) number of word to copy

void v6ReadSharedMemory(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead);  // (i) number of bytes to copy

void usbReadSharedMemory(
   BT_UINT   cardnum,       // (i) card number
   BT_U32BIT *lpbuffer,     // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   wordsToRead);  // (i) number of bytes to copy

   BT_INT    Host_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count);
   
   /************************ 32-BIT RAM Read *************************************/
#ifdef INCLUDE_USB_SUPPORT
   BT_INT    USB_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count);

   
   BT_INT usb_open(BT_UINT cardnum);

   void usb_close(BT_INT cardnum);
   BT_INT usb_get_int_data(BT_INT cardnum);
   BT_INT usb_get_bm_data(BT_INT cardnum);
#endif

BT_INT vbtReadSerialNumber(
   BT_UINT cardnum, 
   BT_U16BIT *serial_number);

void vbtWrite(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtWrite32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT memoffset,     // (o) memory offset within adapter memory (dest)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtRead(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtRead32(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

void vbtReadHIF(
   BT_UINT   cardnum,        // (i) card number
   LPSTR     lpbuffer,       // (o) host buffer (destination)
   BT_U32BIT byteOffset,     // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);   // (i) number of bytes to copy

void vbtWriteHIF(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (i) host buffer (source)
   BT_U32BIT byteOffset,    // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite); // (i) number of bytes to copy

void vbtReadRAM8(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (destination)
   BT_U32BIT byteOffset,    // (i) byte offset within adapter memory (source)
   BT_UINT   bytesToRead);  // (i) number of bytes to copy

void vbtWriteRAM8(
   BT_UINT   cardnum,      // (i) card number
   LPSTR     lpbuffer,     // (i) host buffer (source)
   BT_U32BIT byteOffset,   // (o) byte offset within adapter (destination)
   BT_UINT   bytesToWrite);// (i) number of bytes to copy

BT_U16BIT vbtReadModifyWrite(
   BT_UINT   cardnum,       // (i) card number
   BT_UINT   region,        // (i) HWREG, FILEREG, or  RAM
   BT_U32BIT byteOffset,    // (i) memory offset within adapter memory (source)
   BT_U16BIT wNewWord,      // (i) new value to be written under mask
   BT_U16BIT wNewWordMask); // (i) mask for new value

void vbtRead_iq(
   BT_UINT   cardnum,       // (i) card number
   LPSTR     lpbuffer,      // (o) host buffer (dest)
   BT_U32BIT memoffset,     // (i) memory offset within adapter memory (source)
   BT_UINT   numbytes);     // (i) number of bytes to copy

/* V5 IRIG routines */
void vbtIRIGConfig(BT_UINT cardnum, BT_U16BIT value);
void vbtIRIGValid(BT_UINT cardnum, BT_U16BIT *valid);
void vbtIRIGSetTime(BT_UINT cardnum, BT_U16BIT time_lsb, BT_U16BIT time_msb);
void vbtIRIGWriteDAC(BT_UINT cardnum, BT_U16BIT value);
BT_INT vbtIRIGCal(BT_UINT cardnum, BT_INT flag);

BT_INT vbtIRIGCalV6(BT_UINT cardnum, BT_INT flag);

BT_INT vbtReadFlashData(BT_UINT cardnum, BT_U16BIT *fdata);

void vbtShutdown(BT_UINT cardnum);   // (i) card number

void (*vbtNotify[MAX_BTA]) (
   BT_UINT cardnum,                   // (i) card number of board/channel to process
   BT_U32BIT   *bmLocalRecordTime);   // (i) Time to call BM recorder if > BM_RECORD_TIME
void v5Notify(
   BT_UINT cardnum,                   // (i) card number of board/channel to process
   BT_U32BIT   *bmLocalRecordTime);   // (i) Time to call BM recorder if > BM_RECORD_TIME
void v6Notify(
   BT_UINT cardnum,                   // (i) card number of board/channel to process
   BT_U32BIT   *bmLocalRecordTime);   // (i) Time to call BM recorder if > BM_RECORD_TIME

void vbtMapUserDLL(BT_UINT cardnum);  // Setup user interface dlls


#ifdef _INTEGRITY_
DWORD timeGetTime(void); 
#endif

void API_InterruptInit(BT_UINT cardnum,BT_UINT flag);   // User interrupt initialization function.

#ifdef DEMO_CODE
void vbtHardwareSimulation(BT_UINT cardnum);  // Simulation/Demo mode function.
#endif

void bt_memcpy(void * dest, void * source, int numbytes); // Assembly WORD copy function.

BT_INT API_MemTest(BT_UINT cardnum);
BT_INT API_MemTest32(BT_UINT cardnum);
void get_64BitHostTimeTag(BT_INT mode, BT1553_TIME *host_time);
void RegisterFunctionOpen(BT_UINT cardnum);
void RegisterFunctionClose(BT_UINT cardnum);
void SignalUserThread(BT_UINT cardnum, BT_UINT nType, BT_U32BIT brdAddr, BT_UINT IQAddr);
void SignalV5UserThread(BT_UINT cardnum, BT_UINT nType, BT_U32BIT brdAddr, BT_UINT IQAddr);
BT_INT  vbtInterruptSetup(BT_UINT cardnum, BT_INT hw_int_enable, BT_INT hw_int_num);
void vbtInterruptClose(BT_UINT cardnum);
unsigned long CEI_GET_TIME(void);
BT_INT vbtSetPolling(BT_UINT polling,  // (i) polling interval
                     BT_UINT tflag);   // (i) timer option 0 - start; 1 - restart 

BT_INT vbtOpen1553Channel( BT_UINT   *chnd,       // (o) card number (0 - 12) (device number in 32-bit land)
                           BT_UINT   mode,        // (i) operational mode
                           BT_INT    devid,       // (i) linear base address of board/carrier memory area or WinRT device number
                           BT_UINT   channel);

#ifdef INCLUDE_VME_VXI_1553
BT_INT vbtReadVMEConfigRegs(
   BT_U16BIT *config_addr,      // A16 config address
   BT_U16BIT *config_data);     // cofiguration data storage

BT_INT vbtWriteVMEConfigRegs(
   BT_U16BIT  *vme_config_addr,  // A16 Address
   BT_UINT   offset,             // Byte offset of register  
   BT_U16BIT config_data);       // (i) data to write
#endif

// Debug only functions.  Convertable to other environments

void DumpBCmsg(             // Dump the BC message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpBMmsg(             // Dump the BM message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpBMflt(             // Dump the BM filter list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file

void V6DumpBCmsg(             // Dump the BC message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void V6DumpBMmsg(             // Dump the BC message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void V6DumpRTbufs(          // Dump the RT message list to output file
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);      // (i) handle of output file
void V6DumpRTDefaultBufs(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);      // (i) handle of output file
void V6DumpRTCBufBroadcast(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);       // (i) handle of output file
void DumpBMmsg(             // Dump the BM message list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpBMflt(             // Dump the BM filter list to output file
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpRTptrs(
   BT_UINT   cardnum,       // (i) card number (0 based)
   FILE  * hfMemFile);      // (i) handle of output file
void DumpTimeTag(
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file
void DumpRegisterFIFO(
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile);      // (i) handle of output file

#ifdef ADD_TRACE
void AddTrace(
   BT_UINT cardnum,         // (i) card number (0 - max)
   BT_INT  nFunction,       // (i) function number to log
   BT_INT  nParam1,         // (i) first parameter to log
   BT_INT  nParam2,         // (i) second parameter to log
   BT_INT  nParam3,         // (i) third parameter to log
   BT_INT  nParam4,         // (i) fourth parameter to log
   BT_INT  nParam5);        // (i) fifth parameter to log
#else
#define AddTrace(a,b,c,d,e,f,g)
#endif

#if defined(DO_BUS_LOADING)
void BM_BusLoadingFilter(BT_UINT cardnum);
#else
#define BM_BusLoadingFilter(p1)
#endif

void setReadWrite(BT_INT cardnum);
void SetFunctionsPTR(BT_UINT cardnum);

//void (*vbtSetCSCRegister[MAX_BTA])	(
NOMANGLE BT_INT (CCONV *BT_BC_Init[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers);  // (i) number of BC message buffers ( 1 or 2 for legacy)

NOMANGLE BT_INT CCONV v5_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers);   // (i) number of BC message buffers ( 1 or 2 for legacy)

NOMANGLE BT_INT CCONV v6_BC_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   bc_options,    // (i) Gap Time from start = 0xf; Msg Schd = 0x10; 
                            //     Frame Timing 0x20, no prediction logic 0x40
   BT_U32BIT Enable,        // (i) interrupt enables
   BT_UINT   wRetry,        // (i) retry enables
   BT_UINT   wTimeout1,     // (i) no response time out in microseconds
   BT_UINT   wTimeout2,     // (i) late response time out in microseconds
   BT_U32BIT frame,         // (i) minor frame period, in microseconds
   BT_UINT   num_buffers);   // (i) number of BC message buffers ( 1 or 2 for legacy)

NOMANGLE BT_INT (CCONV *BT_BC_MessageAlloc[MAX_BTA]) (
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT count);          // (i) number of BC messages to allocate

NOMANGLE BT_INT CCONV v5_BC_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT count);          // (i) number of BC messages to allocate

NOMANGLE BT_INT CCONV v6_BC_MessageAlloc(
   BT_UINT cardnum,         // (i) card number (0 - based)
   BT_UINT count);          // (i) number of BC messages to allocate

NOMANGLE BT_INT (CCONV *BT_BC_MessageNoop[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag);        // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
NOMANGLE BT_INT CCONV v6_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag);       // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
NOMANGLE BT_INT CCONV v5_BC_MessageNoop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid,       // (i) Number of BC message to modify
   BT_UINT NoopFlag,        // (i) Flag, 1 -> Set to NOOP, 0 -> Return to msg
                            //           0xf -> Timed Noop
   BT_UINT WaitFlag);       // (i) Flag, 1 -> Wait for BC to not be executing msg,
                            //           0 -> Do not test for BC executing msg.
NOMANGLE BT_INT (CCONV *BT_BC_MessageRead[MAX_BTA]) (
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_UINT mblock_id,          // (i) Number of BC message
   API_BC_MBUF * api_message); // (i) Pointer to buffer to receive msg

NOMANGLE BT_INT CCONV v5_BC_MessageRead(
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_UINT mblock_id,          // (i) Number of BC message
   API_BC_MBUF * api_message); // (i) Pointer to buffer to receive msg

NOMANGLE BT_INT CCONV v6_BC_MessageRead(
   BT_UINT   cardnum,          // (i) card number (0 based)
   BT_UINT mblock_id,          // (i) Number of BC message
   API_BC_MBUF * api_message); // (i) Pointer to buffer to receive msg

NOMANGLE BT_INT (CCONV *BT_BC_MessageWrite[MAX_BTA]) (
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_UINT   messageid,         // (i) BC Message number
   API_BC_MBUF * api_message);  // (i) pointer to user's buffer containing msg

NOMANGLE BT_INT CCONV v6_BC_MessageWrite(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_UINT   messageid,         // (i) BC Message number
   API_BC_MBUF * api_message);  // (i) pointer to user's buffer containing msg

NOMANGLE BT_INT CCONV v5_BC_MessageWrite(
   BT_UINT   cardnum,           // (i) card number (0 based)
   BT_UINT   messageid,         // (i) BC Message number
   API_BC_MBUF * api_message);  // (i) pointer to user's buffer containing msg

NOMANGLE BT_INT (CCONV *BT_BC_MessageUpdate[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV v5_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV v6_BC_MessageUpdate(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT (CCONV *BT_BC_MessageUpdateBuffer[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV v5_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT CCONV v6_BC_MessageUpdateBuffer(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mblock_id,     // (i) BC Message number
   BT_UINT   buffer_id,     // (i) BC data buffer 0=A 1=B
   BT_U16BIT * buffer);     // (i) pointer to user's data buffer

NOMANGLE BT_INT (CCONV *BT_BC_Start[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid);      // (i) BC Message number to start processing at

NOMANGLE BT_INT CCONV v5_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid);      // (i) BC Message number to start processing at

NOMANGLE BT_INT CCONV v6_BC_Start(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT messageid);      // (i) BC Message number to start processing at

NOMANGLE BT_INT (CCONV *BT_BC_ReadNextMessage[MAX_BTA]) (
   BT_INT cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v5_BC_ReadNextMessage(
   BT_INT cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v6_BC_ReadNextMessage(
   BT_INT cardnum,
   BT_UINT timeout,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT (CCONV *BT_BC_ReadlastMessage[MAX_BTA]) (
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v5_BC_ReadLastMessage(
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v6_BC_ReadLastMessage(
   BT_INT cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr,
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT (CCONV *BT_BC_ReadLastMessageBlock[MAX_BTA]) (
   BT_INT cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v5_BC_ReadLastMessageBlock(
   BT_INT cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf);

NOMANGLE BT_INT CCONV v6_BC_ReadLastMessageBlock(
   BT_INT cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BC_MBUF *pBC_mbuf);

/**************************************************************
 *  RT functions
 *************************************************************/
NOMANGLE BT_INT (CCONV *BT_RT_Init[MAX_BTA]) (
    BT_UINT cardnum,          // (i) card number
    BT_UINT testflag);        // (i) flag; ignored!

NOMANGLE BT_INT CCONV v5_RT_Init(
    BT_UINT cardnum,          // (i) card number
    BT_UINT testflag);        // (i) flag; ignored!

NOMANGLE BT_INT CCONV v6_RT_Init(
    BT_UINT cardnum,          // (i) card number
    BT_UINT testflag);        // (i) flag; ignored!

NOMANGLE BT_INT (CCONV *BT_RT_AbufRead[MAX_BTA]) (
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (o) Pointer to buffer to receive abuf data

NOMANGLE BT_INT CCONV v5_RT_AbufRead(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (o) Pointer to buffer to receive abuf data

NOMANGLE BT_INT CCONV v6_RT_AbufRead(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (o) Pointer to buffer to receive abuf data

NOMANGLE BT_INT (CCONV *BT_RT_AbufWrite[MAX_BTA]) (
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (i) Pointer to user's abuf structure

NOMANGLE BT_INT CCONV v5_RT_AbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (i) Pointer to user's abuf structure

NOMANGLE BT_INT CCONV v6_RT_AbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddress,         // (i) RT Terminal Address
   API_RT_ABUF * abuf);       // (i) Pointer to user's abuf structure

NOMANGLE BT_INT (CCONV *BT_RT_CbufbroadWrite[MAX_BTA]) (
   BT_UINT cardnum,           // (i) card number
   BT_UINT subaddr,           // (i) Subaddress to write
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   API_RT_CBUFBROAD * apicbuf); // (i) Buffer containing Control Buffer

NOMANGLE BT_INT CCONV v5_RT_CbufbroadWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT subaddr,           // (i) Subaddress to write
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   API_RT_CBUFBROAD * apicbuf); // (i) Buffer containing Control Buffer

NOMANGLE BT_INT CCONV v6_RT_CbufbroadWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT subaddr,           // (i) Subaddress to write
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   API_RT_CBUFBROAD * apicbuf); // (i) Buffer containing Control Buffer

NOMANGLE BT_INT (CCONV *BT_RT_CbufWrite[MAX_BTA]) (
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) Remote Terminal address to be setup
   BT_UINT subaddr,           // (i) Subaddress to be setup
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   BT_INT  mbuf_count,        // (i) Num buffers, If negative, one pass through only.
                              //     Current buffer number if second call.
   API_RT_CBUF * apicbuf);    // (i) User-supplied CBUF; legal word count mask

NOMANGLE BT_INT CCONV v5_RT_CbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) Remote Terminal address to be setup
   BT_UINT subaddr,           // (i) Subaddress to be setup
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   BT_INT  mbuf_count,        // (i) Num buffers, If negative, one pass through only.
                              //     Current buffer number if second call.
   API_RT_CBUF * apicbuf);    // (i) User-supplied CBUF; legal word count mask

NOMANGLE BT_INT CCONV v6_RT_CbufWrite(
   BT_UINT cardnum,           // (i) card number
   BT_UINT rtaddr,            // (i) Remote Terminal address to be setup
   BT_UINT subaddr,           // (i) Subaddress to be setup
   BT_UINT tr,                // (i) Non-zero indicates Transmit, zero Receive
   BT_INT  mbuf_count,        // (i) Num buffers, If negative, one pass through only.
                              //     Current buffer number if second call.
   API_RT_CBUF * apicbuf);    // (i) User-supplied CBUF; legal word count mask

NOMANGLE BT_INT (CCONV *BT_RT_MessageBufferNext[MAX_BTA])(
   BT_UINT cardnum,            // (i) card number
   BT_UINT rtaddr,             // (i) RT Terminal Address of message to read
   BT_UINT subaddr,            // (i) RT Subaddress of message to read
   BT_UINT tr,                 // (i) Receive or Transmit buffer is to be read
   BT_UINT *mbuf_id  );        // (i) ID number of message buffer (0-based)
  

NOMANGLE BT_INT (CCONV *BT_RT_MessageRead[MAX_BTA]) (
   BT_UINT cardnum,            // (i) card number
   BT_UINT rtaddr,             // (i) RT Terminal Address of message to read
   BT_UINT subaddr,            // (i) RT Subaddress of message to read
   BT_UINT tr,                 // (i) Receive or Transmit buffer is to be read
   BT_UINT mbuf_id,            // (i) ID number of message buffer (0-based)
   API_RT_MBUF_READ * apimbuf);// (o) User's buffer to receive data

NOMANGLE BT_INT CCONV v5_RT_MessageRead(
   BT_UINT cardnum,            // (i) card number
   BT_UINT rtaddr,             // (i) RT Terminal Address of message to read
   BT_UINT subaddr,            // (i) RT Subaddress of message to read
   BT_UINT tr,                 // (i) Receive or Transmit buffer is to be read
   BT_UINT mbuf_id,            // (i) ID number of message buffer (0-based)
   API_RT_MBUF_READ * apimbuf);// (o) User's buffer to receive data

NOMANGLE BT_INT CCONV v6_RT_MessageBufferNext(
   BT_UINT cardnum,          // (i) card number
   BT_UINT RT,              // (i) RT Terminal Address of message 
   BT_UINT SA,              // (i) RT Subaddress of message 
   BT_UINT TR,              // (i) Receive or Transmit buffer is to be read
   BT_UINT *mbuf_id);        // (o) User's variable to store the buffer ID

NOMANGLE BT_INT CCONV v5_RT_MessageBufferNext(
   BT_UINT cardnum,          // (i) card number
   BT_UINT RT,              // (i) RT Terminal Address of message 
   BT_UINT SA,              // (i) RT Subaddress of message 
   BT_UINT TR,              // (i) Receive or Transmit buffer is to be read
   BT_UINT *mbuf_id);        // (o) User's variable to store the buffer ID

NOMANGLE BT_INT CCONV v6_RT_MessageRead(
   BT_UINT cardnum,            // (i) card number
   BT_UINT rtaddr,             // (i) RT Terminal Address of message to read
   BT_UINT subaddr,            // (i) RT Subaddress of message to read
   BT_UINT tr,                 // (i) Receive or Transmit buffer is to be read
   BT_UINT mbuf_id,            // (i) ID number of message buffer (0-based)
   API_RT_MBUF_READ * apimbuf);// (o) User's buffer to receive data

NOMANGLE BT_INT (CCONV *BT_RT_MessageWriteDef[MAX_BTA]) (
   BT_UINT cardnum,              // (i) card number
   BT_UINT rtaddr,               // (i) RT Terminal Address
   API_RT_MBUF_WRITE * apimbuf); // (i) Pointer to user's RT Message Buffer

NOMANGLE BT_INT CCONV v5_RT_MessageWriteDef(
   BT_UINT cardnum,              // (i) card number
   BT_UINT rtaddr,               // (i) RT Terminal Address
   API_RT_MBUF_WRITE * apimbuf); // (i) Pointer to user's RT Message Buffer

NOMANGLE BT_INT CCONV v6_RT_MessageWriteDef(
   BT_UINT cardnum,              // (i) card number
   BT_UINT rtaddr,               // (i) RT Terminal Address
   API_RT_MBUF_WRITE * apimbuf); // (i) Pointer to user's RT Message Buffer

// Bus Monitor functions

NOMANGLE BT_INT (CCONV *BT_BM_FilterWrite[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   rtaddr,
   BT_UINT   subaddr,
   BT_UINT   tr,
   API_BM_CBUF * api_cbuf);

NOMANGLE BT_INT CCONV v5_BM_FilterWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   rtaddr,
   BT_UINT   subaddr,
   BT_UINT   tr,
   API_BM_CBUF * api_cbuf);

NOMANGLE BT_INT CCONV v6_BM_FilterWrite(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   rtaddr,
   BT_UINT   subaddr,
   BT_UINT   tr,
   API_BM_CBUF * api_cbuf);

NOMANGLE BT_INT (CCONV *BT_BM_MessageAlloc[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count,    // Message count converted to bytes;
   BT_UINT   * mbuf_actual, // Actual allocated count 
   BT_U32BIT enable);       // interrupt enables

NOMANGLE BT_INT CCONV v5_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count,    // Message count converted to bytes;
   BT_UINT   * mbuf_actual, // Actual allocated count 
   BT_U32BIT enable);       // interrupt enables

NOMANGLE BT_INT CCONV v6_BM_MessageAlloc(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT   mbuf_count,    // Message count converted to bytes;
   BT_UINT   * mbuf_actual, // Actual allocated count 
   BT_U32BIT enable);       // interrupt enables

NOMANGLE BT_INT (CCONV *BT_BM_StartStop[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag);      // (i) 0=stop
                            //     1=start
                            //     3=start + reset time tag
                            //     0xf BM_START_BIT

NOMANGLE BT_INT CCONV v5_BM_StartStop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag);      // (i) 0=stop
                            //     1=start
                            //     3=start + reset time tag
                            //     0xf BM_START_BIT

NOMANGLE BT_INT CCONV v6_BM_StartStop(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT startflag);      // (i) 0=stop
                            //     1=start
                            //     3=start + reset time tag
                            //     0xf BM_START_BIT

NOMANGLE BT_INT (CCONV *BT_BM_ReadNextMessage[MAX_BTA]) (
   int cardnum,BT_UINT timeout,
   BT_INT rt_addr,  
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v5_BM_ReadNextMessage(
   int cardnum,BT_UINT timeout,
   BT_INT rt_addr,  
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v6_BM_ReadNextMessage(
   int cardnum,BT_UINT timeout,
   BT_INT rt_addr,  
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT (CCONV *BT_BM_ReadLastMessage[MAX_BTA]) (
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v5_BM_ReadLastMessage(
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v6_BM_ReadLastMessage(
   int cardnum,
   BT_INT rt_addr,
   BT_INT subaddress,
   BT_INT tr, 
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT (CCONV *BT_BM_ReadLastMessageBlock[MAX_BTA]) (
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v5_BM_ReadLastMessageBlock(
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT CCONV v6_BM_ReadLastMessageBlock(
   int cardnum,
   BT_INT rt_addr_mask,
   BT_INT subaddr_mask,
   BT_INT tr, 
   BT_UINT *mcount,
   API_BM_MBUF *pBM_mbuf);

NOMANGLE BT_INT (CCONV *BT_BM_Init[MAX_BTA]) (
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT enable_a,
   BT_UINT enable_b);

NOMANGLE BT_INT CCONV v5_BM_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT enable_a,
   BT_UINT enable_b);

NOMANGLE BT_INT CCONV v6_BM_Init(
   BT_UINT   cardnum,       // (i) card number (0 based)
   BT_UINT enable_a,
   BT_UINT enable_b);

// Misc functions
NOMANGLE BT_INT (CCONV *BT_DiscreteTriggerOut[MAX_BTA]) (
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag);         // (i) select one or more output trigger

NOMANGLE BT_INT CCONV v5_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag);         // (i) TRIGGER_OUT_DIS_7 - use Discrete 7 as trigger out
                            //     TRIGGER_OUT_DIS_8 - use Discrete 8 as trigger out
                            //     TRIGGER_OUT_DIS_NONE - use neither

NOMANGLE BT_INT CCONV v6_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag);         // (i) select one or more output trigger

NOMANGLE BT_INT (CCONV *BT_DiscreteTriggerIn[MAX_BTA]) (
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag);        // (i) 0=none, 1=485, 2=dis7 3=dis8 (v5)
                            // (i) 0=none,  1 to  31 discrete channel (v6)
                            //             -1 to -31 differential channel (v6)

NOMANGLE BT_INT CCONV v5_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag);        // (i) 0=none, 1=485, 2=dis7 3=dis8

NOMANGLE BT_INT CCONV v6_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag);        // (i) 0=none,  1 to  31 discrete channel
                            //             -1 to -31 differential channel

NOMANGLE BT_INT (CCONV *BT_SetOptions[MAX_BTA]) (
   BT_UINT cardnum,        // (i) card number
   BT_UINT intflag,        // (i) 0x0001 -> suppress MF OFlow message
                           //     0x0002 -> Monitor Invalid Commands
                           //     0x0004 -> Dump on BM Stop
                           //     0x0008 -> BM trigger on message
                           //     0x0020 -> Process BM data on Interrupt
                           //     0x0040 -> Undefined is illegal
                           //     0x0080 -> MFO Interrupt Enable
   BT_UINT resettimer,     // (i) Reset TT on Sync Mode code
   BT_UINT trig_on_sync,   // (i) Trigger output on Sync Mode Code enable = 1
                           //     disable = 0;
   BT_UINT enable_rt);     // (i) RT start on trigger input enable = 1
                           //     Disable = 0;

NOMANGLE BT_INT CCONV v5_SetOptions(
   BT_UINT cardnum,        // (i) card number
   BT_UINT intflag,        // (i) 0x0001 -> suppress MF OFlow message
                           //     0x0002 -> Monitor Invalid Commands
                           //     0x0004 -> Dump on BM Stop
                           //     0x0008 -> BM trigger on message
                           //     0x0020 -> Process BM data on Interrupt
                           //     0x0040 -> Undefined is illegal
                           //     0x0080 -> MFO Interrupt Enable
   BT_UINT resettimer,     // (i) Reset TT on Sync Mode code
   BT_UINT trig_on_sync,   // (i) Trigger output on Sync Mode Code enable = 1
                           //     disable = 0;
   BT_UINT enable_rt);     // (i) RT start on trigger input enable = 1
                           //     Disable = 0;

NOMANGLE BT_INT CCONV v6_SetOptions(
   BT_UINT cardnum,        // (i) card number
   BT_UINT intflag,        // (i) 0x0001 -> suppress MF OFlow message
                           //     0x0002 -> Monitor Invalid Commands
                           //     0x0004 -> Dump on BM Stop
                           //     0x0008 -> BM trigger on message
                           //     0x0020 -> Process BM data on Interrupt
                           //     0x0040 -> Undefined is illegal
                           //     0x0080 -> MFO Interrupt Enable
   BT_UINT resettimer,     // (i) Reset TT on Sync Mode code
   BT_UINT trig_on_sync,   // (i) Trigger output on Sync Mode Code enable = 1
                           //     disable = 0;
   BT_UINT enable_rt);     // (i) RT start on trigger input enable = 1
                           //     Disable = 0;

#endif //APIINT_H



