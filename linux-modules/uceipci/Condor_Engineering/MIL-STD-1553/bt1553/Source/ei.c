/*============================================================================*
 * FILE:                          E I . C
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
 *===========================================================================*
 *
 * FUNCTION:   BusTools/1553-API Library:
 *             Error Injection routines.
 *             These functions assume that the BusTools_API_Init-() function
 *             has been successfully called.
 *
 * USER ENTRY POINTS:
 *    BusTools_EI_EbufRead  - Error Injection buffer read.
 *    BusTools_EI_EbufWrite - Error Injection buffer write.
 *    BusTools_EI_Getaddr   - Returns the address of the specified entry
 *                            in the EI injection table.
 *    BusTools_EI_Getid     - Returns the error id associated with the
 *                            specified error injection address.
 *    BusTools_EI_EbufWriteENH - Enhance error injection buffer write
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    BusTools_EI_Init      - Initialize EI operations.
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  8.14 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/17/1997   Merged 16- and 32-bit versions together.V2.21.ajh
  09/28/1997   Added additional LabView interface definitions. V2.32.ajh
  02/15/2002   Added spport for modular API. v4.46
  03/15/2002   Added Bi-Phase error V4.48
  03/16/2008   Expanded error injection (data gap and mid parity)
  01/18/2011   Extended Error injection for Bus Tester mode
  04/21/2011   Add BusTools_EI_EnhEbufWrite for support F/W 5.0 errors
  05/11/2012   Change to combine V6 F/W and V5 F/W into single API
  11/05/2012   Add user allocated block in the dump file
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_EbufRead
*
*  FUNCTION
*     This function reads an error injection buffer from the
*     BusTools memory.  The error type is saved by the busapi
*     software.
*
*  RETURNS
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_EI_EbufRead(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,
   API_EIBUF * ebuf)
{
#ifdef ERROR_INJECTION 
  /************************************************
   *  Check initial conditions
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   /*******************************************************************
   *  Check for bigger than the largest allowed
   *  error message number
   *******************************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return(API_EI_ILLERRORNO);

   /*******************************************************************
   *  Determine how many words to process based on message type
   *******************************************************************/

   switch ( ebuf->buftype )
   {
        case EI_BC_REC:
            break;
        case EI_BC_TRANS:
            break;
        case EI_BC_RTTORT:
            break;
        case EI_RT_REC:
            break;
        case EI_RT_TRANS:
            break;
   }
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_EbufWrite
*
*  FUNCTION
*     This function writes an error injection buffer to the
*     BusTools memory.  The error type is saved by the BusAPI
*     software.
*
*  PARAMETERS
*
*  RETURNS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_EbufWrite(
   BT_UINT  cardnum,        // (i) card number (0 - based)
   BT_UINT  errorid,
   API_EIBUF * ebuf)
{

#ifdef ERROR_INJECTION
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_INT       i;
   BT_INT       errortype;
   BT_INT       nWords=0;
   BT_U32BIT    addr;
   EI_MESSAGE   error;

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   if((_HW_FPGARev[cardnum]&0xfff) >= 0x500)
      return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Check for bigger than the largest allowed error message number.
   *******************************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return API_EI_ILLERRORNO;

   /*******************************************************************
   *  Process buffer based on error buffer type.
   *******************************************************************/
   errortype = ebuf->buftype;

   /*******************************************************************
   * First figure out how many words go into the error buffer.  Each
   *  error buffer is the same fixed length, but the number of words
   *  used in the buffer depends on the message type associated with
   *  the specific buffer.
   * Error buffers are specified by the 1553 message type, and are
   *  only used for that message type.  This is a "feature" of BusTools,
   *  and is not required by the hardware or by this API.  The only
   *  thing which limits the usage of an error injection buffer is
   *  the fact that the hardware does not support all error types
   *  for all messages types, command, status and data words.
   *******************************************************************/
   switch ( errortype )
   {
      case EI_BC_REC:
      case EI_RT_TRANS:
         nWords = EI_COUNT;
         break;
      case EI_RT_REC:
      case EI_BC_TRANS:
         nWords = 1;
         break;
      case EI_BC_RTTORT:
         nWords = 2;
         break;
   }
   // End of code to setup the number of words in the error injection buffer.

   // Now copy the specified number of words into the HW error injection buf.
   memset((void*)&error,0,sizeof(error));  // Initialize the result.
   for ( i = 0; i < nWords; i++ )
   {
       switch ( ebuf->error[i].etype )
       {
        case EI_NONE:            error.data[i] = EI_HW_NONE;          break;
        case EI_BITCOUNT:        error.data[i] = EI_HW_BITCOUNT;      break;
        case EI_SYNC:            error.data[i] = EI_HW_SYNC;          break;
        case EI_DATAWORDGAP:     error.data[i] = EI_HW_DATAWORDGAP;   break;
        case EI_PARITY:          error.data[i] = EI_HW_PARITY;        break;
        case EI_WORDCOUNT:       error.data[i] = EI_HW_WORDCOUNT;     break;
        case EI_LATERESPONSE:    error.data[i] = EI_HW_LATERESPONSE;  break;
        case EI_BADADDR:         error.data[i] = EI_HW_BADADDR;       break;
        case EI_MIDSYNC:         error.data[i] = EI_HW_MIDSYNC;       break;
        case EI_MIDBIT:          error.data[i] = EI_HW_MIDBIT;        break;
        case EI_MIDPARITY:       error.data[i] = EI_HW_MIDPARITY;     break;
        case EI_RESPWRONGBUS:    error.data[i] = EI_HW_RESPWRONGBUS;  break;
		case EI_BIPHASE:         error.data[i] = EI_HW_BIPHASE;       break;
        default:
            return(API_EI_BADMSGTYPE);
      }
      switch ( ebuf->error[i].etype )
      {
        case EI_BITCOUNT:
        case EI_SYNC:
        case EI_DATAWORDGAP:
        case EI_WORDCOUNT:
		case EI_BIPHASE: 
        case EI_BADADDR:
        case EI_MIDBIT:
        case EI_LATERESPONSE:
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
      }
   }

   /*******************************************************************
   *  Write the error buffer to hardware
   *******************************************************************/

   addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
   vbtWrite(cardnum,(LPSTR)&error,addr,sizeof(error));
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION

}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Getaddr
*
*  FUNCTION
*     This function returns the address of the specified entry
*     in the EI injection table.
*
*  PARAMETERS
*       BT_UINT cardnum    BTA Card Number
*       WORD errorid    Error ID
*
*  RETURNS
*
****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_Getaddr(
   BT_UINT cardnum,              // (i) card number
   BT_UINT errorid,              // (i) Error Injection buffer number
   BT_U32BIT * addr)         // (o) address of error injection buffer
{
#ifdef ERROR_INJECTION
   /************************************************
   *  local variables
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  check for bigger than the largest allowed
   *  error message number
   ************************************************/

   if (errorid > EI_MAX_ERROR_NUM)
      return(API_EI_ILLERRORNO);

   /************************************************
   *  Calculate address from base address & index.
   ************************************************/
   if(board_is_v5_uca[cardnum]) 
      *addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
   else
      *addr = BTMEM_EI_V6 + errorid * EI_MESSAGE_SIZE;
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION
}

/**********************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Getid
*
*  FUNCTION
*     This function returns the error id associated with the
*     specified error injection address.
*
*  PARAMETERS
*       BT_UINT cardnum    BTA Card Number
*       BT_U16BIT errorid    Error ID
*
*  RETURNS
*       0 -> success
*       API_EI_BADMSGTYPE   -> Specified message type is not defined
*       API_EI_ILLERRORADDR -> Error address is not defined
*
**********************************************************************/

NOMANGLE BT_INT CCONV BusTools_EI_Getid(
   BT_UINT   cardnum,              // (i) card number
   BT_U32BIT addr,           // (i) address of buffer to locate
   BT_UINT * errorid)        // (o) buffer number
{
#ifdef ERROR_INJECTION
   /************************************************
   *  local variables
   ************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   /************************************************
   *  If address is out of range, return an error
   ************************************************/

   if (addr < BTMEM_EI_V6)
      return API_EI_ILLERRORADDR;

   if (addr > BTMEM_EI_V6_NEXT)
      return API_EI_ILLERRORADDR;

   /************************************************
   *  Calculate error id number
   ************************************************/
   if(board_is_v5_uca[cardnum])
      *errorid = (BT_U16BIT)((addr - BTMEM_EI) / sizeof(EI_MESSAGE));
   else
      *errorid = (addr - BTMEM_EI_V6) / EI_MESSAGE_SIZE;
   return API_SUCCESS;
#else
   return API_NO_BUILD_SUPPORT;
#endif //ERROR_INJECTION
}

/**********************************************************************
*
*  PROCEDURE NAME - BusTools_EI_Init
*   
*  FUNCTION
*       Initialize EI operations.  Each of the "EI_MAX_ERROR_NUM"
*       error injection buffers between "BTMEM_EI" and "BTMEM_EI_NEXT"
*       are initialized to zero.  BusTools and this API supports a
*       fixed number of error injection buffers, which is fixed-allocated
*       by the API in the lower 64 K of memory.
*
*  Returns
*     Nothing
*
**********************************************************************/

NOMANGLE void BusTools_EI_Init(BT_UINT cardnum)
{
#ifdef ERROR_INJECTION
   /************************************************
   *  Local variables
   ************************************************/

   BT_UINT    i;
   BT_U32BIT  addr;
   EI_MESSAGE error;

   /************************************************
   *  Check for legal call
   ************************************************/

   if (cardnum >= MAX_BTA)
      return;

   if (bt_inited[cardnum] == 0)
      return;

   /*******************************************************************
   *  Clear all of the buffers and output them to the hardware.
   *******************************************************************/

   memset((void*)&error, 0, sizeof(error));
   for ( i = 0; i < EI_MAX_ERROR_NUM; i++ )
   {
      if(board_is_v5_uca[cardnum]) 
      {
         addr = BTMEM_EI + i * sizeof(EI_MESSAGE);
         vbtWrite(cardnum, (LPSTR)&error, addr, sizeof(error));
      }
      else
      {
         addr = BTMEM_EI_V6 + i * EI_MESSAGE_SIZE;
         vbtWriteRAM32[cardnum](cardnum, (BT_U32BIT *)&error, addr, EI_MESSAGE_DWSIZE);
      }
   }
   return;
#else
   return;
#endif //ERROR_INJECTION
}


/************************************************************************************
*    PROCEDURE NAME - BusTools_EI_EnhEbufWrite (DEPRECATED)
* 
*   FUNCTION 
*   This function writes an error injection buffer to the
*   BusTools memory.  The error type is saved by the BusAPI
*   software.  
*
*   The "RTVAL Tester" version of the firmware adds some new error injection 
*   capability that is not supported in BusTools_EI_EbufWrite.
* 
*   Bi-phase Error
*   
*   A bi-phase bit error is when there is no zero-crossing for the entire bit time.  
*   This error injection inhibits a zero crossing for the selected bit. It may be 
*   programmed as a logic high or a logic low.
*   
*   The four lsb’s of the error_info[3:0] bits determine the selected bit of the 
*   MIL-STD-1553 word: 0 through 15.  Bit 0 is the LSB of the 16-bit word, however 
*   it is the last bit to be transmitted in the serial stream.  Similarly, bit 15 
*   is the MSB and is the first bit to be transmitted.  
*   
*   The error_info[4] bit is used in conjunction with error_info[3:0] bits to inject 
*   a bi-phase error onto the parity bit time. If error_info[4:0] is programmed to 
*   binary 11111, then the parity bit time will be selected for error injection.
*   The error_info[5] bit determines if the injected bit will be at a logic high 
*   or a logic low state for the entire bit time. Setting err_info[5] will force 
*   the signal to a logic high state throughout the bit time; clearing err_info[5]
*   will force the signal to a logic low state throughout the bit time.
*   
*   A bi-phase bit error is when there is no zero-crossing for the entire bit time. 
*   It is not predictable how a 1553 decoder will interpret a word when a bi-phase 
*   error is injected on onto one of the first two bits, as it depends on timing 
*   and the states of the first two bits.  When the bit doesn't cross zero, it may 
*   stay in one state for 1½  microseconds and the decoder might try to re-establish 
*   sync.  Therefore, the API does not support injection in either of the first two 
*   bits, but the user should be aware of the possible behavior should this case 
*   occur at the decoder.
* 
********************************************************************************************/ 
NOMANGLE BT_INT CCONV BusTools_EI_EnhEbufWrite(BT_UINT cardnum, BT_UINT errorid, BT_UINT enhData, API_EIBUF *ebuf)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_INT       i;
   BT_INT       errortype;
   BT_INT       nWords = 0;
   BT_U32BIT    addr;
   EI_MESSAGE   error;

   BT_U16BIT    enhHalfBit[] = {0x8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,
                                0x18,0x1a,0x1c,0x1e,0x20,0x22,0x24,0x26};

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (bm_inited[cardnum] == 0)
      return API_BM_NOTINITED;

   if((_HW_FPGARev[cardnum]&0xfff) < 0x500)
      return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Process buffer based on error buffer type.
   *******************************************************************/
   errortype = ebuf->buftype;

   /*******************************************************************
   * First figure out how many words go into the error buffer.  Each
   *  error buffer is the same fixed length, but the number of words
   *  used in the buffer depends on the message type associated with
   *  the specific buffer.
   * Error buffers are specified by the 1553 message type, and are
   *  only used for that message type.  This is a "feature" of BusTools,
   *  and is not required by the hardware or by this API.  The only
   *  thing which limits the usage of an error injection buffer is
   *  the fact that the hardware does not support all error types
   *  for all messages types, command, status and data words.
   *******************************************************************/
   switch ( errortype )
   {
      case EI_BC_REC:
      case EI_RT_TRANS:
         nWords = EI_COUNT;
         break;
      case EI_RT_REC:
      case EI_BC_TRANS:
         nWords = 1;
         break;
      case EI_BC_RTTORT:
         nWords = 2;
         break;
   }
   // End of code to setup the number of words in the error injection buffer.

   // Now copy the specified number of words into the HW error injection buf.
   memset((void*)&error,0,sizeof(error));  // Initialize the result.
   for ( i = 0; i < nWords; i++ )
   {
       switch ( ebuf->error[i].etype )
       {
        case EI_NONE:            error.data[i] = EI_HW_NONE;          break;
        case EI_BITCOUNT:        error.data[i] = EI_HW_BITCOUNT;      break;
        case EI_SYNC:            error.data[i] = EI_HW_SYNC;          break;
        case EI_DATAWORDGAP:     error.data[i] = EI_HW_DATAWORDGAP;   break;
        case EI_PARITY:          error.data[i] = EI_HW_PARITY;        break;
        case EI_WORDCOUNT:       error.data[i] = EI_HW_WORDCOUNT;     break;
        case EI_LATERESPONSE:    error.data[i] = EI_HW_LATERESPONSE;  break;
        case EI_BADADDR:         error.data[i] = EI_HW_BADADDR;       break;
        case EI_RESPWRONGBUS:    error.data[i] = EI_HW_RESPWRONGBUS;  break;
        case EI_MIDSYNC:         error.data[i] = EI_HW_ENH_MIDSYNC;   break;
        case EI_MIDBIT:          error.data[i] = EI_HW_ENH_MIDBIT;    break;
        case EI_MIDPARITY:       error.data[i] = EI_HW_ENH_MIDPARITY; break;
	    case EI_BIPHASE:         error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_LOW:     error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_HI:      error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_PAR_HI:  error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_PAR_LOW: error.data[i] = EI_HW_BIPHASE;       break;
        case EI_ENH_ZEROXNG:     error.data[i] = EI_HW_ENH_ZEROXNG;   break;
        case EI_TCENH_ZEROXNG:   error.data[i] = EI_HW_TCENH_ZEROXNG; break;
        case EI_BS3_FIXER:       error.data[i] = 0;                   break;
        default:
           return(API_EI_BADMSGTYPE);
      }
      switch ( ebuf->error[i].etype )
      {
        case EI_BITCOUNT:
        case EI_SYNC:           // Modification for RTVAL testing
        case EI_DATAWORDGAP:
        case EI_WORDCOUNT:
        case EI_BADADDR:
		case EI_BIPHASE:        // same as current version of API: not enhanced
        case EI_MIDBIT:
        case EI_BS3_FIXER:
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;
        case EI_LATERESPONSE:
            if(ebuf->error[i].edata < 8)
               ebuf->error[i].edata = 8;
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;

   /********************************************************************************** 
   The error_info[4] bit is used in conjunction with error_info[3:0] bits to inject 
   a bi-phase error onto the parity bit time. If error_info[4:0] is programmed to 
   binary 11111, then the parity bit time will be selected for error injection.
   The error_info[5] bit determines if the injected bit will be at a logic high 
   or a logic low state for the entire bit time. Setting err_info[5] will force 
   the signal to a logic high state throughout the bit time; clearing err_info[5]
   will force the signal to a logic low state throughout the bit time.
   ************************************************************************************/
        case EI_MIDPARITY:
            error.data[i] += enhHalfBit[(ebuf->error[i].edata & 0x003F)-1];
        case EI_BIPHASE_LOW: 
            // remove error_info[5] to select "bi-phase == LOW": Same as original bi_phase_error
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x001F);
            break;
		case EI_BIPHASE_HI: 
            ebuf->error[i].edata = ebuf->error[i].edata | 0x20; // set error_info[5]
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;
        case EI_BIPHASE_PAR_HI:
            // set error_info[5] and set the data pattern to
            // set the error in the parity bit
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F); 
            break;
        case EI_BIPHASE_PAR_LOW:
            // reset error_info[5] and set the data pattern to
            // set the error in the parity bit
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F); 
            break;
        case EI_ENH_ZEROXNG:
        case EI_TCENH_ZEROXNG:
            // The bits (other than the 
            error.data[i] += (enhData & 0x1FFF);
            if(!board_is_v5_uca[cardnum])
            {
               if((errortype==EI_RT_TRANS) && (i==0))
                  error.data[i] |= 0x4000;
            }   
            break;
      }
   }

   /*******************************************************************
   *  Write the error buffer to hardware
   *******************************************************************/
   if(board_is_v5_uca[cardnum]) 
   {
      // calculate the address of the EI buffer
      addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
      // and write it.
      vbtWrite(cardnum,(LPSTR)&error,addr,sizeof(error));
   }
   else
   {
      // calculate the address of the EI buffer
      addr = BTMEM_EI_V6 + errorid * EI_MESSAGE_SIZE;
      // and write it.
      vbtWriteRAM[cardnum](cardnum,(BT_U16BIT *)&error,addr,EI_MESSAGE_WSIZE);
   }

   return API_SUCCESS;
}

/************************************************************************************
*    PROCEDURE NAME - BusTools_EI_EbufWriteENH
* 
*   FUNCTION 
*   This function writes an error injection buffer to the
*   BusTools memory.  The error type is saved by the BusAPI
*   software.  
*
*   The "RTVAL Tester" version of the firmware adds some new error injection 
*   capability that is not supported in BusTools_EI_EbufWrite.
* 
*   Bi-phase Error
*   
*   A bi-phase bit error is when there is no zero-crossing for the entire bit time.  
*   This error injection inhibits a zero crossing for the selected bit. It may be 
*   programmed as a logic high or a logic low.
*   
*   The four lsb’s of the error_info[3:0] bits determine the selected bit of the 
*   MIL-STD-1553 word: 0 through 15.  Bit 0 is the LSB of the 16-bit word, however 
*   it is the last bit to be transmitted in the serial stream.  Similarly, bit 15 
*   is the MSB and is the first bit to be transmitted.  
*   
*   The error_info[4] bit is used in conjunction with error_info[3:0] bits to inject 
*   a bi-phase error onto the parity bit time. If error_info[4:0] is programmed to 
*   binary 11111, then the parity bit time will be selected for error injection.
*   The error_info[5] bit determines if the injected bit will be at a logic high 
*   or a logic low state for the entire bit time. Setting err_info[5] will force 
*   the signal to a logic high state throughout the bit time; clearing err_info[5]
*   will force the signal to a logic low state throughout the bit time.
*   
*   A bi-phase bit error is when there is no zero-crossing for the entire bit time. 
*   It is not predictable how a 1553 decoder will interpret a word when a bi-phase 
*   error is injected on onto one of the first two bits, as it depends on timing 
*   and the states of the first two bits.  When the bit doesn't cross zero, it may 
*   stay in one state for 1½  microseconds and the decoder might try to re-establish 
*   sync.  Therefore, the API does not support injection in either of the first two 
*   bits, but the user should be aware of the possible behavior should this case 
*   occur at the decoder.
* 
********************************************************************************************/ 
NOMANGLE BT_INT CCONV BusTools_EI_EbufWriteENH(BT_UINT cardnum, BT_UINT errorid, API_ENH_EIBUF *ebuf)
{
   /*******************************************************************
   *  Local variables
   *******************************************************************/

   BT_INT       i;
   BT_INT       errortype;
   BT_INT       nWords = 0;
   BT_INT       zeroxng_in_use = 0;
   BT_U32BIT    addr;
   EI_MESSAGE   error;

   BT_U16BIT    enhHalfBit[] = {0x8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,
                                0x18,0x1a,0x1c,0x1e,0x20,0x22,0x24,0x26};

   /*******************************************************************
   *  Check initial conditions
   *******************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((_HW_FPGARev[cardnum]&0xfff) < 0x500)
      return API_HARDWARE_NOSUPPORT;

   /*******************************************************************
   *  Process buffer based on error buffer type.
   *******************************************************************/
   errortype = ebuf->buftype;

   /*******************************************************************
   * First figure out how many words go into the error buffer.  Each
   *  error buffer is the same fixed length, but the number of words
   *  used in the buffer depends on the message type associated with
   *  the specific buffer.
   * Error buffers are specified by the 1553 message type, and are
   *  only used for that message type.  This is a "feature" of BusTools,
   *  and is not required by the hardware or by this API.  The only
   *  thing which limits the usage of an error injection buffer is
   *  the fact that the hardware does not support all error types
   *  for all messages types, command, status and data words.
   *******************************************************************/
   switch ( errortype )
   {
      case EI_BC_REC:
      case EI_RT_TRANS:
         nWords = EI_COUNT;
         break;
      case EI_RT_REC:
      case EI_BC_TRANS:
         nWords = 1;
         break;
      case EI_BC_RTTORT:
         nWords = 2;
         break;
   }
   // End of code to setup the number of words in the error injection buffer.

   // Now copy the specified number of words into the HW error injection buf.
   memset((void*)&error,0,sizeof(error));  // Initialize the result.
   for ( i = 0; i < nWords; i++ )
   {
       switch ( ebuf->error[i].etype )
       {
        case EI_NONE:            error.data[i] = EI_HW_NONE;          break;
        case EI_BITCOUNT:        error.data[i] = EI_HW_BITCOUNT;      break;
        case EI_SYNC:            error.data[i] = EI_HW_SYNC;          break;
        case EI_DATAWORDGAP:     error.data[i] = EI_HW_DATAWORDGAP;   break;
        case EI_PARITY:          error.data[i] = EI_HW_PARITY;        break;
        case EI_WORDCOUNT:       error.data[i] = EI_HW_WORDCOUNT;     break;
        case EI_LATERESPONSE:    error.data[i] = EI_HW_LATERESPONSE;  break;
        case EI_BADADDR:         error.data[i] = EI_HW_BADADDR;       break;
        case EI_RESPWRONGBUS:    error.data[i] = EI_HW_RESPWRONGBUS;  break;
        case EI_MIDSYNC:         error.data[i] = EI_HW_ENH_MIDSYNC;   break;
        case EI_MIDBIT:          error.data[i] = EI_HW_ENH_MIDBIT;    break;
        case EI_MIDPARITY:       error.data[i] = EI_HW_ENH_MIDPARITY; break;
        case EI_BIPHASE:         error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_LOW:     error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_HI:      error.data[i] = EI_HW_BIPHASE;       break;
        case EI_BIPHASE_PAR_HI:  error.data[i] = EI_HW_BIPHASE;       break;
	      case EI_BIPHASE_PAR_LOW: error.data[i] = EI_HW_BIPHASE;       break;
	      case EI_ENH_ZEROXNG:     error.data[i] = EI_HW_ENH_ZEROXNG;   break;
        case EI_TCENH_ZEROXNG:   error.data[i] = EI_HW_TCENH_ZEROXNG; break;
        case EI_BS3_FIXER:       error.data[i] = 0;                   break;
        default:
           return(API_EI_BADMSGTYPE);
      }
      switch ( ebuf->error[i].etype )
      {
        case EI_BITCOUNT:
        case EI_SYNC:           // Modification for RTVAL testing
        case EI_DATAWORDGAP:
        case EI_WORDCOUNT:
        case EI_BADADDR:
		    case EI_BIPHASE:        // same as current version of API: not enhanced
        case EI_MIDBIT:
        case EI_BS3_FIXER:
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;
        case EI_LATERESPONSE:
            if(ebuf->error[i].edata < 8)
               ebuf->error[i].edata = 8;
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;

   /********************************************************************************** 
   The error_info[4] bit is used in conjunction with error_info[3:0] bits to inject 
   a bi-phase error onto the parity bit time. If error_info[4:0] is programmed to 
   binary 11111, then the parity bit time will be selected for error injection.
   The error_info[5] bit determines if the injected bit will be at a logic high 
   or a logic low state for the entire bit time. Setting err_info[5] will force 
   the signal to a logic high state throughout the bit time; clearing err_info[5]
   will force the signal to a logic low state throughout the bit time.
   ************************************************************************************/
        case EI_MIDPARITY:
            error.data[i] += enhHalfBit[(ebuf->error[i].edata & 0x003F)-1];
        case EI_BIPHASE_LOW: 
            // remove error_info[5] to select "bi-phase == LOW": Same as original bi_phase_error
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x001F);
            break;
		    case EI_BIPHASE_HI: 
            ebuf->error[i].edata = ebuf->error[i].edata | 0x20; // set error_info[5]
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F);
            break;
        case EI_BIPHASE_PAR_HI:
            // set error_info[5] and set the data pattern to
            // set the error in the parity bit
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F); 
            break;
        case EI_BIPHASE_PAR_LOW:
            // reset error_info[5] and set the data pattern to
            // set the error in the parity bit
            error.data[i] += (BT_U8BIT)(ebuf->error[i].edata & 0x003F); 
            break;
        case EI_ENH_ZEROXNG:
        case EI_TCENH_ZEROXNG:
            // The bits (other than the
            if(zeroxng_in_use)
               return API_ZEROXNG_SET;  //Only one enhanced zero crossing error per message
            zeroxng_in_use++;
            error.data[i] += (ebuf->error[i].edata & 0x1FFF);
            if(!board_is_v5_uca[cardnum])
            {
               if((errortype==EI_RT_TRANS) && (i==0))
                  error.data[i] |= 0x4000;
            }   
            break;
      }
   }

   /*******************************************************************
   *  Write the error buffer to hardware
   *******************************************************************/
   if(board_is_v5_uca[cardnum]) 
   {
      // calculate the address of the EI buffer
      addr = BTMEM_EI + errorid * sizeof(EI_MESSAGE);
      // and write it.
      vbtWrite(cardnum,(LPSTR)&error,addr,sizeof(error));
   }
   else
   {
      // calculate the address of the EI buffer
      addr = BTMEM_EI_V6 + errorid * EI_MESSAGE_SIZE;
      // and write it.
      vbtWriteRAM[cardnum](cardnum,(BT_U16BIT *)&error,addr,EI_MESSAGE_WSIZE);
   }

   return API_SUCCESS;
}

