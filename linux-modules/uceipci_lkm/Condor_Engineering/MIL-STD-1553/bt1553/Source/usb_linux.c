/*============================================================================*
 * FILE:                     U S B _ L I N U X . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 2014 - 2015 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:   
 *
 *===========================================================================*/

/* $Revision:  1.01 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  08/28/2014   initial
  04/10/2015   modified usb_open to set the device ID. removed USB_BM_MessageAlloc. bch
*/

#include <stdlib.h>
#include <stdio.h>
#include "lowlevel.h"
#include "busapi.h"
#include "btdrv.h"
#include "apiint.h"
#include "globals.h"


// prototypes from ll_usb
CEI_INT usb_dev_open(PDEVMAP_T pDMap, CEI_INT opt);
CEI_INT usb_dev_close(PDEVMAP_T pDMap);
CEI_INT usb_dev_read(PDEVMAP_T pDMap, CEI_UINT addr, CEI_INT size, CEI_CHAR* buf);
CEI_INT usb_dev_write(PDEVMAP_T pDMap, CEI_UINT addr, CEI_INT size, CEI_CHAR* buf);
BT_INT usb_error(BT_INT cardnum, const CEI_CHAR *fname, CEI_INT status);

// globals
static DEVMAP_T usb_dev_map[MAX_BTA];
static CEI_UINT close_count[MAX_BTA] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define GET_PDEVMAP(_1_)          &usb_dev_map[api_device[_1_]]
#define BT1553_TIME_SIZE  8
#define BTMEM_IQ_V6_SIZE  8  // sizeof(IQ_MBLOCK_V6)
#define USB_HWREG_SIZE    1024  // (sizeof(BT_U32BIT) * 0x100)
#define USB_IQ_SIZE       4096  // (BTMEM_IQ_V6_SIZE * 512)


/****************************/
BT_INT usb_open(BT_UINT cardnum) {
  CEI_INT status=0, opt=0;
  PDEVMAP_T pDevMap=GET_PDEVMAP(cardnum);

  /* need to set the device number and the USB device id for the R15-USB */
  pDevMap->device = api_device[cardnum];
  pDevMap->DeviceID = 0x7503;

  // set the channel number in the options to be used only for client processes with the multi-process server
  if(CurrentCardSlot[cardnum] == CHANNEL_1)
    opt = 1;
  else if(CurrentCardSlot[cardnum] == CHANNEL_2)
    opt = 2;

  status = usb_dev_open(pDevMap, opt);

  return status;
}


void usb_close(BT_INT cardnum) {
  CEI_INT status=0;

  status = usb_dev_close(GET_PDEVMAP(cardnum));

  if(ptrMbuf[cardnum] != NULL) {
    CEI_FREE(ptrMbuf[cardnum]);
    ptrMbuf[cardnum] = NULL;
  }

  if(usb_host_hwreg[cardnum] != NULL) {
    CEI_FREE(usb_host_hwreg[cardnum]);
    usb_host_hwreg[cardnum] = NULL;
  }
  if(usb_int_queue[cardnum] != NULL) {
    CEI_FREE(usb_int_queue[cardnum]);
    usb_int_queue[cardnum] = NULL;
  }
}


#ifdef _64_BIT_
// ignores the warnings for the cast of bt_PageAddr to a 32-bit address
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif

/****************************/
// updates the BM buffers in the host-side HWREG ("hardware register") buffer
BT_INT usb_get_bm_data(BT_INT cardnum) {
  CEI_INT status=0, BM_head_size=0;
  BT_U32BIT bm_head_ptr=0;

  // get the current BM head pointer from the host-side HWREG
  bm_head_ptr = REL_ADDR(cardnum, usb_host_hwreg[cardnum][HWREG_BM_HEAD_PTR]);

  // if no new BM messages detected then return
  if(bm_head_ptr == bm_rec_prev[cardnum])
    return API_SUCCESS;

  // check that the BM head pointer is within the BM message buffer
  if((bm_head_ptr < btmem_bm_mbuf[cardnum]) || (bm_head_ptr > btmem_bm_mbuf_next[cardnum]))
    return 6789;

  // if the BM buffer did not wrap then only need to read from the BM tail pointer to the BM head pointer on the device
  if(bm_head_ptr > bm_rec_prev[cardnum]) {
    if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + bm_rec_prev[cardnum]), bm_head_ptr - bm_rec_prev[cardnum], (CEI_CHAR*)ptrMbuf[cardnum] + (bm_rec_prev[cardnum] - btmem_bm_mbuf[cardnum]))) != BTD_OK)
      return status;
  }
  else {
    // first read from the BM tail pointer to the end of the BM message buffer
    if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + bm_rec_prev[cardnum]), btmem_bm_mbuf_next[cardnum] - bm_rec_prev[cardnum], (CEI_CHAR*)(ptrMbuf[cardnum]) + (bm_rec_prev[cardnum] - btmem_bm_mbuf[cardnum]))) != BTD_OK)
      return status;
    // next read from the start of the BM message buffer to the BM head pointer
    if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + btmem_bm_mbuf[cardnum]), bm_head_ptr - btmem_bm_mbuf[cardnum], (CEI_CHAR*)ptrMbuf[cardnum])) != BTD_OK)
      return status;
  }

  // update the BM tail pointer
  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][HWREG_PAGE] + (HWREG_BM_TAIL_PTR * 4)), 4, (CEI_CHAR*)&usb_host_hwreg[cardnum][HWREG_BM_HEAD_PTR])) != BTD_OK)
    return status;

  return API_SUCCESS;
}

/****************************/
BT_U32BIT usbGetBMHeadPtr(BT_UINT cardnum) {
  return usb_host_hwreg[cardnum][HWREG_BM_HEAD_PTR];
}

// time
/****************************/
BT_U32BIT usbGetTTRegister(BT_UINT cardnum, BT_U32BIT regnum) {
  CEI_INT status=0;
  BT_U32BIT regval=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  return regval;
}

/****************************/
void usbSetTTRegister(BT_UINT cardnum, BT_U32BIT regnum, BT_U32BIT regval) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadTimeTag(BT_UINT cardnum, BT_U32BIT *timetag) {
  CEI_INT status=0;
  BT1553_TIME bt_time;

  memset(&bt_time, 0, sizeof(bt_time));

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (TTREG_LATCH*4)), 4, (CEI_CHAR*)&bt_time)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (TTREG_READ_LOW*4)), 8, (CEI_CHAR*)&bt_time)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  timetag[0] = bt_time.microseconds;
  timetag[1] = bt_time.topuseconds;
}

/****************************/
void usbWriteTimeTag(BT_UINT cardnum, BT1553_TIME *timetag) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (TTREG_LOAD_LOW*4)), BT1553_TIME_SIZE, (CEI_CHAR*)timetag)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbWriteTimeTagIncr(BT_UINT cardnum, BT_U32BIT incr) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TTREG_PAGE] + (TTREG_INCREMENT*4)), 4, (CEI_CHAR*)&incr)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

// discrete
/****************************/
void usbSetDiscrete(BT_UINT cardnum, BT_U32BIT regnum, BT_U32BIT regval) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
BT_U32BIT usbGetDiscrete(BT_UINT cardnum, BT_U32BIT regnum) {
  CEI_INT status=0;
  BT_U32BIT val=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&val)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  return val;
}

/****************************/
BT_U32BIT usbGetTrigRegister(BT_UINT cardnum, BT_U32BIT regnum) {
  CEI_INT status=0;
  BT_U32BIT val=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TRIG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&val)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  return val;
}

/****************************/
void usbSetTrigRegister(BT_UINT cardnum, BT_U32BIT regnum, BT_U32BIT regval) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][TRIG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

// HIF/CSC
/****************************/
void usbWriteHIF(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + byteOffset), wordsToWrite * 4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadHIF(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + byteOffset), wordsToRead * 4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
BT_U32BIT usbGetCSCRegister(BT_UINT cardnum, BT_U32BIT regnum) {
  CEI_INT status=0;
  BT_U32BIT regval=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  return regval;
}

/****************************/
void usbSetCSCRegister(BT_UINT cardnum, BT_U32BIT regnum, BT_U32BIT regval) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][CSC_REG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

// interrupt queue
/****************************/
BT_U32BIT usbGetIqHeadPtr(BT_UINT cardnum) {
   return usb_host_hwreg[cardnum][HWREG_IQ_HEAD_PTR];
}

/****************************/
void usbReadIntQueue(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset) {
   BT_UINT index = (byteOffset - BTMEM_IQ_V6)/BTMEM_IQ_V6_SIZE;

   ((IQ_MBLOCK_V6*)lpbuffer)->msg_ptr = usb_int_queue[cardnum][index].msg_ptr;
   ((IQ_MBLOCK_V6*)lpbuffer)->mode    = usb_int_queue[cardnum][index].mode;
}


/****************************/
BT_INT usb_get_int_data(BT_INT cardnum) {
  CEI_INT status=0;

  // read the entire hardware register
  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)bt_PageAddr[cardnum][HWREG_PAGE], USB_HWREG_SIZE, (CEI_CHAR*)usb_host_hwreg[cardnum])) != BTD_OK)
    return 0xC001;

  // read the entire interrupt queue
  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + BTMEM_IQ_V6), USB_IQ_SIZE, (CEI_CHAR*)usb_int_queue[cardnum])) != BTD_OK)
    return 0xC002;

  return BTD_OK;
}

// read memory
/****************************/
void usbReadRAM(BT_UINT cardnum, BT_U16BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + byteOffset), wordsToRead * 2, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadBMRAM(BT_UINT cardnum, BT_U16BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT i;
  BT_U16BIT *addr = (BT_U16BIT*)(bt_PageAddr[cardnum][BM_SIM_PAGE] + (byteOffset - btmem_bm_mbuf[cardnum]));

  memcpy(lpbuffer, addr, wordsToRead*2);
}

/****************************/
void usbReadRelRAM(BT_UINT cardnum, BT_U16BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][BASE_PAGE] + byteOffset), wordsToRead*2, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + byteOffset), wordsToRead*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadBMRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT i;
  BT_U32BIT *addr=(BT_U32BIT*)(bt_PageAddr[cardnum][BM_SIM_PAGE] + (byteOffset - btmem_bm_mbuf[cardnum]));

  memcpy(lpbuffer, addr, wordsToRead*4);
}

/****************************/
void usbReadRelRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][BASE_PAGE] + byteOffset), wordsToRead*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbReadSharedMemory(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToRead) {
  CEI_INT status=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][SHR_MEM_PAGE] + byteOffset), wordsToRead*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
BT_U32BIT usbGetRegister(BT_UINT cardnum, BT_U32BIT regnum) {
  CEI_INT status=0;
  BT_U32BIT regval=0;

  if((status = usb_dev_read(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][HWREG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);

  usb_host_hwreg[cardnum][regnum] = regval;

  return regval;
}

// write memory
/****************************/
void usbWriteRAM(BT_UINT cardnum, BT_U16BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + byteOffset), wordsToWrite*2, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbWriteRelRAM(BT_UINT cardnum, BT_U16BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][BASE_PAGE] + byteOffset), wordsToWrite*2, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbWriteRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][RAM_PAGE] + byteOffset), wordsToWrite*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbWriteBMRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT i;
  BT_U32BIT *addr=((BT_U32BIT*)(bt_PageAddr[cardnum][BM_SIM_PAGE] + byteOffset));

  memcpy(addr, lpbuffer, wordsToWrite*4);
}

/****************************/
void usbWriteRelRAM32(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][BASE_PAGE] + byteOffset), wordsToWrite*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbWriteSharedMemory(BT_UINT cardnum, BT_U32BIT *lpbuffer, BT_U32BIT byteOffset, BT_UINT wordsToWrite) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][SHR_MEM_PAGE] + byteOffset), wordsToWrite*4, (CEI_CHAR*)lpbuffer)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

/****************************/
void usbSetRegister(BT_UINT cardnum, BT_U32BIT regnum, BT_U32BIT regval) {
  CEI_INT status=0;

  if((status = usb_dev_write(GET_PDEVMAP(cardnum), (CEI_UINT)(bt_PageAddr[cardnum][HWREG_PAGE] + (regnum*4)), 4, (CEI_CHAR*)&regval)) != BTD_OK)
    usb_error(cardnum, __func__, status);
}

// misc
/****************************/
BT_INT usb_error(BT_INT cardnum, const CEI_CHAR *fname, CEI_INT status) {
  CEI_INT _status=0;

  _status = BusTools_API_Close(cardnum);

  sprintf(szMsg, "\nApplication closing. Unrecoverable USB error: %s, status: %d, %d.\n", fname, status, _status);
  printf("%s", szMsg);

  exit(1);
}
