 /*============================================================================*
 * FILE:                        F L A S H _ C O N F I G . C
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
 *             This file contains the configuration function for the FLASH and
 *             PLX.
 *
 * USER ENTRY POINTS: 
 *
 *
 *===========================================================================*/

/* $Revision:  8.18 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  12/03/2010   Initial release.
  06/06/2011   Add support for RXMC2-1553 and LPCIe-1553
  03/27/2015   Add Flashlog erase, write and read APIs and Add firmwarereload API
               for the RAR15XMCXT card and firmware build 0x176.
  09/07/2016   Added support for new hardware MPCIE
*/

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#define _GLOBALS_H_

#include <stdio.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"

// defines for RAR15XMC_XT flash
#define RAR15XMC_XT_Block63_ADDR_END         0x3FFFFF
#define RAR15XMC_XT_Block63_ADDR             0x3F0000
#define RAR15XMC_XT_Block62_ADDR_END         0x3EFFFF
#define RAR15XMC_XT_Block62_ADDR             0x3E0000
#define RAR15XMC_XT_Block61_ADDR_END         0x3DFFFF
#define RAR15XMC_XT_Block61_ADDR             0x3D0000
#define RAR15XMC_XT_Block60_ADDR_END         0x3CFFFF
#define RAR15XMC_XT_Block60_ADDR             0x3C0000
#define RAR15XMC_XT_Flash_PAGE_SIZE          256           //256 bytes
#define SPI_CONTROL_PORT                     (0xA0 * 4)    // convert from 32bits (4 bytes)  address to 8 bit (byte) address
#define SPI_RELOAD_PORT                      (0xA1 * 4)    // convert from 32bits (4 bytes)  address to 8 bit (byte) address

#define RAR15XMC_XT_Flash_Block_SIZE         64*1024       //64k bytes

CEI_INT spi_flash_update (CEI_UINT cardnum, CEI_UCHAR * fdata, CEI_UINT fsize);
CEI_INT spi_flash_verify (CEI_UINT cardnum, pCEI_UCHAR fdata, CEI_UINT fsize);
CEI_UINT spi_page_program (CEI_UINT cardnum, CEI_UINT spi_address, unsigned char * data);
CEI_UINT32 spi_read_status (CEI_UINT cardnum);
void spi_write_enable (CEI_UINT cardnum);
CEI_INT spi_read_data (CEI_UINT cardnum, CEI_UINT spi_address, CEI_UINT bcount, pCEI_UCHAR buffer);
CEI_UINT spi_block_erase (CEI_UINT cardnum, CEI_UINT spi_address);
void psWrite32(CEI_UINT cardnum, pCEI_UINT32 pWord, CEI_UINT32 byteOffset);
void psRead32(CEI_UINT cardnum, pCEI_UINT32 pWord, CEI_UINT32 byteOffset);
CEI_UINT32 HiResDelay(CEI_UINT32 nMicroSeconds);
//
#define FLASH_WORD_ADDR_LSB  0xa1 //142
#define FLASH_WORD_ADDR_MSB  0xa2 //144
#define FLASH_DATA_WRITE_REG 0xa3 //146
#define FLASH_DATA_READ_REG  0xa4 //148
#define READ_MODE 0xff
#define STATUS_REG 0x70
#define ERASE_SETUP 0x20
#define ERASE_CONFIRM 0xd0
#define WRITE_MODE 0x40
#define DATA_BLOCK_0 0x2000
#define DATA_BLOCK_1 0x3000
#define DATA_BLOCK_2 0x4000
#define DATA_BLOCK_3 0x5000
#define DATA_BLOCK_4 0x6000

#define INVERTBITS(b)   (~(b))
#define REVERSEBITS(b)  (BitReverseTable[b])

static CEI_UCHAR BitReverseTable[256] =
{
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_ReadFlashConfigData
*
*  FUNCTION
*     The function returns the contents of flash Config registers.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_ReadFlashConfigData(BT_UINT cardnum, BT_U16BIT * fdata)
{
   BT_INT status;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   status = vbtReadFlashData(cardnum, fdata);
   return status;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataWrite
*
*  FUNCTION
*     The function writes data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataWrite(BT_UINT cardnum, BT_U16BIT **saData, BT_U32BIT saEnable)
{
   BT_U16BIT dbuf,rdata;
   static int pindx;
   BT_UINT rindx,sa_indx;
   BT_U16BIT edata[2];

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);

   //Place in erase setup mode
   dbuf = ERASE_SETUP;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   //Write erase block  address
   dbuf = DATA_BLOCK_0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);  

   //Erase the block
   dbuf = ERASE_CONFIRM;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);


   pindx = 0x0;

   //now write the data
   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      { 
         //Place in write mode by writing data 0x40 to FLASH_DATA_REG
         dbuf = WRITE_MODE;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = saData[sa_indx][rindx];
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         do{
            //Place in read status mode
            dbuf = STATUS_REG;
            vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

            //some short delay???
            vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
         } while ((rdata & 0x80) == 0);
      }
   }

   edata[0] = (BT_U16BIT)(saEnable & 0xffff);
   edata[1] = (BT_U16BIT)((saEnable & 0xffff0000)>>16);

   //write out the SA enable register. 
   for(rindx = 0; rindx<2; rindx++)
   {  
      dbuf = WRITE_MODE;  //Place in write mode by writing data 0x40 to FLASH_DATA_REG
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      dbuf = DATA_BLOCK_0 + 0x400 + rindx;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
      dbuf = edata[rindx];
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      do{
         //Place in read status mode
         dbuf = STATUS_REG;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         //some short delay???
         vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
      } while ((rdata & 0x80) == 0);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashWrite
*
*  FUNCTION
*     The function writes data into the flash data block 0 only
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashWrite(BT_UINT cardnum, BT_INT paddr, BT_INT wcount, BT_U16BIT *saData )
{
   BT_U16BIT dbuf,rdata;
   static int rindx;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
      //some short delay???
      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);

   //Place in erase setup mode
   dbuf = ERASE_SETUP;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   //Write erase block  address
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);
   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);  

   //Erase the block
   dbuf = ERASE_CONFIRM;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   do{
      //Place in read status mode
      dbuf = STATUS_REG;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

      vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
   }while((rdata & 0x80) == 0);
 
   for(rindx=0; rindx<wcount; rindx++)
   { 

      //Place in write mode by writing data 0x40 to FLASH_DATA_REG
      dbuf = WRITE_MODE;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
    
      dbuf = 0 + paddr++;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

      dbuf = saData[rindx];
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

      do{
         //Place in read status mode
         dbuf = STATUS_REG;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

         vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);
      } while ((rdata & 0x80) == 0);
   }

   return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME - BusTools_1760_DataRead
*
*  FUNCTION
*     The function reads data into the flash for 1760 start message data
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_1760_DataRead(BT_UINT cardnum, BT_U32BIT **saData, BT_U32BIT *saEnable)
{
   BT_U16BIT dbuf,ebuf1,ebuf2,rdata;
   BT_UINT rindx,sa_indx,pindx=0;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   //Place in read status mode
   dbuf = STATUS_REG;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
   //some short delay???
   vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);

   //Place in read mode by writing data 0xff to FLASH_DATA_REG
   dbuf = READ_MODE;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   for(sa_indx=0; sa_indx<32; sa_indx++)
   {
      for(rindx=0; rindx<32; rindx++)
      {    
         dbuf = DATA_BLOCK_0 + pindx++;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

         dbuf = 0x0;
         vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

         dbuf = 0x0;
         vbtReadHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_READ_REG*2,2);
         saData[sa_indx][rindx] = dbuf;
      }
   }

   //read enable LSB
   dbuf = DATA_BLOCK_0 + 0x400;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf1,FLASH_DATA_READ_REG*2,2);

   //read enable MSB
   dbuf = DATA_BLOCK_0 + 0x401;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

   dbuf = 0x0;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);
   vbtReadHIF(cardnum,(LPSTR)&ebuf2,FLASH_DATA_READ_REG*2,2);
   
   *saEnable = (BT_U32BIT)((BT_U32BIT)ebuf1 | (BT_U32BIT)(ebuf2 << 16));

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashRead
*
*  FUNCTION
*     The function reads data in the flash data block 0 only
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashRead(BT_UINT cardnum, BT_INT paddr, BT_INT wcount, BT_U16BIT *saData )
{
   BT_U16BIT dbuf,rdata;
   BT_UINT pindx=0; 
   BT_INT rindx;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   //Place in read status mode
   dbuf = STATUS_REG;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);
   vbtReadHIF(cardnum,(LPSTR)&rdata,FLASH_DATA_READ_REG*2,2);

   //Place in read mode by writing data 0xff to FLASH_DATA_REG
   dbuf = READ_MODE;
   vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_WRITE_REG*2,2);

   for(rindx=0; rindx<wcount; rindx++)
   {    
      dbuf =  pindx++;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_LSB*2,2);

      dbuf = 0x0;
      vbtWriteHIF(cardnum,(LPSTR)&dbuf,FLASH_WORD_ADDR_MSB*2,2);

      dbuf = 0x0;
      vbtReadHIF(cardnum,(LPSTR)&dbuf,FLASH_DATA_READ_REG*2,2);
      saData[rindx] = dbuf;
   }

   return API_SUCCESS;
}

NOMANGLE BT_INT CCONV BusTools_LockFlash(BT_UINT cardnum)
{
   BT_INT status = 0;
   BT_U16BIT rdata[2]={0,0};

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

#define LOCK_BIT 0x200

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;   

   if(CurrentCardType[cardnum] != QVME1553)
      return API_HARDWARE_NOSUPPORT;

   //Read the CSC and ACR
   //status = BusTools_FlashRead(cardnum, 0x0, 2, rdata );
   if(status==API_SUCCESS)
   {
      printf("BusTools_LockFlash CSC value = %x\n",rdata[0]);
      printf("BusTools_LockFlash ACR value = %x\n",rdata[1]);

      rdata[1] &= ~(LOCK_BIT);

      //status = BusTools_FlashWrite(cardnum, 0x0, 2, rdata );
   }
   return API_SUCCESS;

}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t R e a d S e r i a l N u m b e r
 *===========================================================================*
 *
 * FUNCTION:    reads the serial number on selected boards.
 *
 * DESCRIPTION: This functions read the serial number from flash memory.
 *
 *      It will return:
 *              .API_SUCCESS
 *===========================================================================*/
BT_INT vbtReadSerialNumber(BT_UINT cardnum, BT_U16BIT *serial_number)
{
   BT_U16BIT tdata;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(CurrentCardType[cardnum] == R15EC  || CurrentCardType[cardnum] == RXMC1553 )
   {
      BT_INT time_count=0;
      BT_U8BIT sn_lsb, sn_msb;

      //Read SN LSB

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_lsb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];
      
      //Read SN MSB
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(0x1);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x1e);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

      while(1)
      {
         tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
         if((tdata & 0x8)==0)
            break;
         MSDELAY(1);
         time_count++;
         if(time_count > 5)
            return API_TIMER_ERR;         
      }
      sn_msb = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];     
   
      sn_lsb = REVERSEBITS(sn_lsb);
      sn_msb = REVERSEBITS(sn_msb);

      tdata = (CEI_UINT16)sn_lsb | (CEI_UINT16)(sn_msb << 8);
   }
   else if(CurrentCardType[cardnum] == R15AMC  || 
		   CurrentCardType[cardnum] == RPCIe1553)
   {
       BT_U32BIT indx,addr32,rdata32[4];
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32[0] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32[0] & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);

       for(indx = 0;indx<2;indx++)
       {
          addr32 = 0x80000000 + 0x520000 + indx;
          ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
          MSDELAY(1);
          
          rdata32[indx] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
          rdata32[indx] = little_endian_32(rdata32[indx]);
          rdata32[indx] = REVERSEBITS(rdata32[indx] & 0xff);
       }
#ifdef NON_INTEL_WORD_ORDER
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[1]&0xff) + (((BT_U16BIT)rdata32[0]&0xff)<<8));
#else
       tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[0]&0xff) + (((BT_U16BIT)rdata32[1]&0xff)<<8));
#endif
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   } 
   else if(CurrentCardType[cardnum] == R15XMC2)
   {
       BT_U32BIT addr32,rdata32;
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32 & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);

       addr32 = 0x80000000 + 0x290000;
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
       MSDELAY(1);
          
       rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
       rdata32 = little_endian_32(rdata32);
       tdata = (BT_U16BIT)flipws(rdata32);

       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   } 
   //??????
   else if(CurrentCardType[cardnum] == LPCIE1553 || CurrentCardType[cardnum] == MPCIE1553)
   {
       BT_U8BIT sn_data[2];
       BT_U16BIT rdata;

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21e;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2];
      sn_data[0] = (BT_U8BIT)(rdata & 0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21e;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x201; 
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
      rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2];
      sn_data[1] = (BT_U8BIT)(rdata & 0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      
      tdata = (CEI_UINT16)sn_data[0] | (CEI_UINT16)(sn_data[1] << 8);
      
   }
   else
   {
      //clear status
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0x50);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata  = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      //read array
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = (BT_U16BIT)flipws(0xff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x140a);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x0838);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3];
      flipw(&tdata); // for PMC on PowerPC
   }
   *serial_number = tdata;
   return API_SUCCESS;
}

/*===========================================================================*
 * EXTERNAL ENTRY POINT:       v b t R e a d F l a s h D a t a
 *===========================================================================*
 *
 * FUNCTION:    Reads all flash config data.
 *
 * DESCRIPTION: This functions read the serial number from flash memory.
 *
 *      It will return:
 *              .API_SUCCESS
 *===========================================================================*/
BT_INT vbtReadFlashData(BT_UINT cardnum, BT_U16BIT *fdata)
{
   BT_INT findx;

   BT_U16BIT tdata;

   if(!board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;


   if(CurrentCardType[cardnum] == R15EC  || CurrentCardType[cardnum] == RXMC1553 )
   {
      BT_INT time_count;

      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x144/2] = flipws(0x001f);
      for(findx=0; findx<64; findx++)
      {
         //Read SN LSB
         time_count=0;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x142/2] = flipws(findx);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = flipws(0x8);

         while(1)
         {
            tdata = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x140];
            if((tdata & 0x8) == 0)
               break;
            MSDELAY(1);
            time_count++;
            if(time_count > 5)
               return API_TIMER_ERR;         
         }
         ((BT_U8BIT *)fdata)[findx] = ((BT_U8BIT *)bt_PageAddr[cardnum][3])[0x148];
      }
   }
   else if(CurrentCardType[cardnum] == R15AMC || CurrentCardType[cardnum] == RPCIe1553)
   {
       BT_U32BIT indx,addr32,rdata32[4];
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32[0] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32[0] & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);
       for(findx=0; findx < 32; findx++)
       {
          for(indx = 0;indx<2;indx++)
          {
             addr32 = 0x80000000 + 0x5e0000 + findx*2 + indx;
             ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
             MSDELAY(1);
          
             rdata32[indx] = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
             rdata32[indx] = little_endian_32(rdata32[indx]);
             rdata32[indx] = REVERSEBITS(rdata32[indx] & 0xff);
          }
#ifdef NON_INTEL_WORD_ORDER
          tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[1]&0xff) + (((BT_U16BIT)rdata32[0]&0xff)<<8));
#else
          tdata = (BT_U16BIT)(((CEI_UINT16)rdata32[0]&0xff) + (((BT_U16BIT)rdata32[1]&0xff)<<8));
#endif
          fdata[findx]=tdata;
      }
      ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   }
   else if(CurrentCardType[cardnum] == R15XMC2)
   {
       BT_U32BIT addr32,rdata32;
       BT_INT ecount=0;
       
       //clear status
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x50));
       
       //ReadStatusRegister
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0x70));
       do{
            rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
            if(ecount++ == 0x100)
               break;
       }while((rdata32 & 0xff) != 0x80);
       
       if(ecount==0x100)
       {
    	   return 0x777;
       }

       //read array
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x80000000));
       ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0] = (BT_U32BIT)(little_endian_32(0xff));
       MSDELAY(1);
       for(findx=0; findx < 32; findx++)
       {
          addr32 = 0x80000000 + 0x2f0000 + findx;
          ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(addr32));
          MSDELAY(1);
          
          rdata32 = ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x144))[0];
          rdata32 = little_endian_32(rdata32);
          tdata = (BT_U16BIT)(rdata32 & 0x0000ffff); 
          fdata[findx]= flipws(tdata);
      }
      ((BT_U32BIT *)(bt_PageAddr[cardnum][3] + 0x140))[0] = (BT_U32BIT)(little_endian_32(0x00000000));
   }
   else if(CurrentCardType[cardnum] == LPCIE1553 ||CurrentCardType[cardnum] == MPCIE1553)
   {
       BT_U8BIT sn_data[2];
       BT_U16BIT rdata;
  
      for(findx=0;findx<32;findx+=2)
      {
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21f;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200 + findx; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
         rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140];
         sn_data[0] = (BT_U8BIT)(rdata & 0xff);

         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x203;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x21f;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200;
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200 + findx + 1; 
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x200; 
         rdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140];
         sn_data[1] = (BT_U8BIT)(rdata & 0xff);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0x140/2] = 0x0;
      
         fdata[findx] = (CEI_UINT16)sn_data[0] | (CEI_UINT16)(sn_data[1] << 8);
     }
      
   }
   else
   {

      //clear status
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = flipws(0x50);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata  = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      //read array
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws(0x0);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x8800);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3] = (BT_U16BIT)flipws(0x00ff);
      ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
      do{
         tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
         flipw(&tdata); // for PMC on PowerPC
      }while(tdata != 1);

      for(findx = 0; findx<32; findx++)
      {
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa1] = flipws((0x1406 + findx));
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa2] = flipws(0x0839);
         ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0] = flipws(0x1406);
         do{
            tdata = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa0];
            flipw(&tdata); // for PMC on PowerPC
         }while(tdata != 1);

         fdata[findx] = ((BT_U16BIT *)bt_PageAddr[cardnum][3])[0xa3];
         flipw(&fdata[findx]); // for PMC on PowerPC
      }
   }
   return API_SUCCESS;
}

/*===========================================================================*
 * ENTRY POINT:         psRead32
 *===========================================================================*
 *
 * FUNCTION:    Read data from selected range of HIF memory into a caller
 *              supplied buffer.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/

void psRead32(CEI_UINT cardnum, pCEI_UINT32 pDword, CEI_UINT32 byteOffset)
{
   /*pCEI_UCHAR board = (pCEI_UCHAR)boardBaseAddress;
   
   *pDword = *((pCEI_UINT32)(board + byteOffset));
   
   fliplong(pDword); */
 
   //HiResDelay(6);

   v6ReadHIF(cardnum, pDword, byteOffset, 1); 
   //HiResDelay(6);  - not need to wait on reads to flash for the RAR15-XMC�XT or IT anymore 
   //MSDELAY(1);
}

/*===========================================================================*
 * ENTRY POINT:      psWrite32 
 *===========================================================================*
 *
 * FUNCTION:    Write data from caller supplied buffer into adapter memory.
 *
 * DESCRIPTION:
 *
 *      It will return:
 *              nothing
 *===========================================================================*/

void psWrite32(CEI_UINT cardnum, pCEI_UINT32 pDword, CEI_UINT32 byteOffset)
{
   /*pCEI_UCHAR board = (pCEI_UCHAR)(boardBaseAddress+byteOffset);
   
   fliplong(pDword);

   *(pCEI_UINT32)board = *pDword; */

   v6WriteHIF(cardnum, pDword, byteOffset, 1);   
 
  // HiResDelay(6);not need to wait on writes to flash for the RAR15-XMC�XT or IT anymore 
   //MSDELAY(1);

}


/*===========================================================================*
* ENTRY POINT:  spi_read_data                     
*===========================================================================*
*              
* This routine reads a byte from a specified flash location, using the SPI 
* programming sequence to enable read access, assign the flash source
* address, then read the data value.
*              
*===========================================================================*/
CEI_INT spi_read_data(CEI_UINT cardnum, CEI_UINT spi_address, CEI_UINT bcount, CEI_UCHAR *buffer)
{
   CEI_UINT32 wdata;
   CEI_UINT index;


  
   for (index = 0; index < bcount; index++)
   {
     
      wdata = 0x0;
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);
      
      wdata = 0x203;
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);  /* SPI Read Data */

      wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF0000) >> 16));
      psWrite32(cardnum, &wdata,  SPI_CONTROL_PORT);  /* SPI Address MSByte */
      
      wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF00) >> 8));
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);     /* SPI Address middle byte */
      
      wdata = (CEI_UINT32)(0x200 + (spi_address & 0xFF) + index);
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);              /* SPI Address LSByte */
      
      wdata = 0x200;
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);

      psRead32(cardnum,&wdata, SPI_CONTROL_PORT);
      
      buffer[index] = (unsigned char)(wdata & 0xFF); 
      
      wdata = 0x0;
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);
      
      //printf("spi_read_data  index = %d buffer value =%d\n",index,buffer[index]);

   }
   return 0;
}


/*===========================================================================*
* ENTRY POINT:  spi_flash_page_update                     
*===========================================================================*
*              
* This routine 
* programs 256-byte pages one at a time 
* This routine periodically displays and asterisk in the console window to 
* provide some indication of progress during the erase and program processes.             
*              
*===========================================================================*/
CEI_INT spi_flash_page_update (CEI_UINT cardnum, CEI_UINT32 blockAddr, BT_INT pagenum, CEI_UCHAR *pagedata) /* file size */
{  
   CEI_UINT  spi_address = 0;
   CEI_INT   spi_status = 0;
   

   spi_address = blockAddr + pagenum * RAR15XMC_XT_Flash_PAGE_SIZE;
   
   spi_status = spi_page_program (cardnum, spi_address, pagedata);
   
   return(spi_status);
}
/*===========================================================================*
* ENTRY POINT:  spi_page_program                     
*===========================================================================*
*              
* This routine programs a 256-byte "page" of flash memory based on the
* supplied flash memory offset and 256-byte array of data.
*              
*===========================================================================*/
CEI_UINT spi_page_program (CEI_UINT cardnum, CEI_UINT spi_address, CEI_UCHAR * data)
{
   CEI_INT index;
   CEI_UCHAR spi_status;
   CEI_UINT32 wdata;

   spi_write_enable(cardnum);

   wdata = 0x202;
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);              /* SPI Page Program (PP) - 1 to 256 bytes */
   wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF0000) >> 16));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);  /* SPI Address MSByte */
   wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF00) >> 8));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);     /* SPI Address middle byte */
   wdata = (CEI_UINT32)(0x200 + (spi_address & 0xFF));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);              /* SPI Address LSByte */
   for (index=0; index<=255; index++)
   {
      wdata = (CEI_UINT32)(0x200 + data[index]);
      psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);
   }
   wdata = 0x0;
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);

   while ((spi_status = (CEI_UCHAR)(spi_read_status(cardnum) & 0x01)) != 0);  /* Wait for SPI Page Program cycle to complete */

   return (CEI_UINT)spi_status;
}

/*===========================================================================*
* ENTRY POINT:  spi_write_enable                     
*===========================================================================*
*              
* This routine enables write access to the RAR-PCIE flash component.
*              
*===========================================================================*/
void spi_write_enable (CEI_UINT cardnum)
{
   CEI_UINT32 wdata;

   wdata = 0x0;
   psWrite32(cardnum,&wdata,SPI_CONTROL_PORT);

   wdata = 0x206;
   psWrite32(cardnum,&wdata,SPI_CONTROL_PORT);  /* SPI Write Enable (WREN) */

   wdata = 0x0;
   psWrite32(cardnum,&wdata,SPI_CONTROL_PORT);

}

/*===========================================================================*
* ENTRY POINT:  spi_write_enable                     
*===========================================================================*
*              
* This routine enables write access to the RAR-PCIE flash component.
*              
*===========================================================================*/
CEI_UINT32 HiResDelay(CEI_UINT32 nMicroSeconds)
{
#ifdef WIN32
   MSDELAY(1);
   /*LARGE_INTEGER ticksToWait, freq, start, end, now;
   QueryPerformanceFrequency(&freq);

   ticksToWait.QuadPart = (LONGLONG)(freq.QuadPart * (nMicroSeconds * 1e-6));

   QueryPerformanceCounter(&start);
   end.QuadPart = start.QuadPart + ticksToWait.QuadPart;

   while (1)
   {
      QueryPerformanceCounter(&now);
      if (now.QuadPart >= end.QuadPart) 
      {
         break;
      }
   }
   */
#elif (VXW_653)
   {
      RETURN_CODE_TYPE returnCode;
      TIMED_WAIT(1000*nMicroSeconds, &returnCode);
   }

#else
  // vxbUsDelay(nMicroSeconds);
   static struct timespec mdelay;
   mdelay.tv_sec=0;
   mdelay.tv_nsec=1000*nMicroSeconds;
   nanosleep(&mdelay,NULL);
#endif
   return 0;
}



/*===========================================================================*
* ENTRY POINT:  spi_read_status                     
*===========================================================================*
*              
* This routine reads the SPI Status Register to obtain the state of any
* flash programming activity, (active/inactive).
*               
*===========================================================================*/
CEI_UINT32 spi_read_status (CEI_UINT cardnum)
{
   CEI_UINT32 spi_status, spi_data;

   spi_data = 0x0;
   psWrite32(cardnum, &spi_data, SPI_CONTROL_PORT);
   spi_data = 0x205;
   psWrite32(cardnum, &spi_data, SPI_CONTROL_PORT);  /* SPI Read Status Register (RDSR) */
   spi_data = 0x200;
   psWrite32(cardnum, &spi_data, SPI_CONTROL_PORT);  /* Dummy byte */

   psRead32 (cardnum, &spi_status, SPI_CONTROL_PORT);

   spi_data = 0x0;
   psWrite32(cardnum, &spi_data, SPI_CONTROL_PORT);
   return (spi_status);
}
/*===========================================================================*
* ENTRY POINT:  spi_block_erase                     
*===========================================================================*
*              
* This routine erases the flash sector based on the supplied flash location.
*              
*===========================================================================*/
CEI_UINT spi_block_erase (CEI_UINT cardnum, CEI_UINT spi_address)
{
   CEI_UINT32 spi_status;
   CEI_UINT32 wdata;
   
   spi_write_enable(cardnum); 
   
   wdata = 0x2D8;
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT); /* SPI Sector Erase (SE) */

   wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF0000) >> 16));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);  /* SPI Address MSByte */

   wdata = (CEI_UINT32)(0x200 + ((spi_address & 0xFF00) >> 8));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);     /* SPI Address middle byte */

   wdata = (CEI_UINT32)(0x200 + (spi_address & 0xFF));
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);              /* SPI Address LSByte */

   wdata = 0x0;
   psWrite32(cardnum, &wdata, SPI_CONTROL_PORT);

   while ((spi_status = spi_read_status(cardnum) & 0x01) != 0);  /* Wait for SPI Page Program cycle to complete */
   return (CEI_UINT)spi_status;
}

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashLogErase
*
*  FUNCTION
*     The function erase a block of data.  The size of the block data is 64k bytes 
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashLogErase(BT_UINT cardnum, BT_INT sector)
{
 
   CEI_UINT32 spi_status = 0, blockAddr = 0;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;
  
  
   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (CurrentCardType[cardnum] != RAR15XMCXT)
   {
      return API_HARDWARE_NOSUPPORT;
   }
   
   spi_status = spi_read_status (cardnum);
   if(spi_status == 0x7f)
   {
      //printf("Flash access error: Check write protect\n");
      return API_HARDWARE_NOSUPPORT;
   }

   if (sector == 1)
   {
      blockAddr = RAR15XMC_XT_Block63_ADDR;
   }
   else if (sector == 2)
   {
      blockAddr = RAR15XMC_XT_Block62_ADDR;
   }
   else if (sector == 3)
   {
      blockAddr = RAR15XMC_XT_Block61_ADDR;
   }
   else if (sector == 4)
   {
      blockAddr = RAR15XMC_XT_Block60_ADDR;
   }
   else
   {
      return API_BAD_PARAM;
   }

   spi_status = spi_block_erase(cardnum, blockAddr);
   
   return spi_status;
  
}
/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashLogWrite
*
*  FUNCTION
*     The function writes data in the flash data 
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashLogWrite(BT_UINT cardnum, BT_INT sector, BT_INT pagenum, BT_U8BIT *pageData )
{
     
   CEI_UINT32 spi_status = 0, blockAddr = 0;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (CurrentCardType[cardnum] != RAR15XMCXT)
   {
      return API_HARDWARE_NOSUPPORT;
   }
   
   spi_status = spi_read_status (cardnum);
   if(spi_status == 0x7f)
   {
      //printf("Flash access error.  Check write protect\n");
      return API_HARDWARE_NOSUPPORT;
   }

   if (sector == 1)
   {
      blockAddr = RAR15XMC_XT_Block63_ADDR;
   }
   else if (sector == 2)
   {
      blockAddr = RAR15XMC_XT_Block62_ADDR;
   }
   else if (sector == 3)
   {
      blockAddr = RAR15XMC_XT_Block61_ADDR;
   }
   else if (sector == 4)
   {
      blockAddr = RAR15XMC_XT_Block60_ADDR;
   }
   else
   {
      return API_BAD_PARAM;
   }
   if (pagenum <0 || pagenum >255)
   {
      return API_BAD_PARAM;
   }

   spi_status = spi_flash_page_update(cardnum, blockAddr, pagenum, pageData);
    
   return spi_status;
}
/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FlashLogRead
*
*  FUNCTION
*     The function reads data in the flash block 
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FlashLogRead(BT_UINT cardnum, BT_INT sector, BT_UINT paddr, BT_UINT bcount, BT_U8BIT *rData )
{
   
   CEI_UINT32 spi_status = 0, blockAddr = 0;
   BT_UINT paddrmax = paddr + bcount;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (CurrentCardType[cardnum] != RAR15XMCXT)
   {
      return API_HARDWARE_NOSUPPORT;
   }
   
   
  
   if (sector == 1)
   {
      blockAddr = RAR15XMC_XT_Block63_ADDR;
   }
   else if (sector == 2)
   {
      blockAddr = RAR15XMC_XT_Block62_ADDR;
   }
   else if (sector == 3)
   {
      blockAddr = RAR15XMC_XT_Block61_ADDR;
   }
   else if (sector == 4)
   {
      blockAddr = RAR15XMC_XT_Block60_ADDR;
   }
   else
   {
      return API_BAD_PARAM;
   }

   // verify the address
   if (paddr >= 0 && paddrmax <= RAR15XMC_XT_Flash_Block_SIZE)
   {
      spi_status = spi_read_data(cardnum, blockAddr + paddr, bcount, rData);
   
      return spi_status;
   }
   else
   {
      return API_BAD_PARAM;
   }
 }

/****************************************************************************
*
*  PROCEDURE NAME - BusTools_FirmwareReload
*
*  FUNCTION
*     The function reload firmware 
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_FirmwareReload(BT_UINT cardnum)
{
       
   CEI_UCHAR spi_status = 0;
   BT_INT  build = 0;
   CEI_UINT32 wdata = 0xA1B2C3D4;
    


   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if (CurrentCardType[cardnum] != RAR15XMCXT)
   {
      return API_HARDWARE_NOSUPPORT;
   }
   
   build = vbtGetCSCRegister[cardnum](cardnum,GLBREG_FPGA_REV);

   if (build < 0x200)
   {
      return API_HARDWARE_NOSUPPORT;
   }
       
   if((hw_int_enable[cardnum] >= API_DEMO_MODE) && (hw_int_enable[cardnum] < API_MANUAL_INT))
   {
       vbtInterruptClose(cardnum);      // Close down interrupt and thread processing.
   }
   psWrite32(cardnum, &wdata, SPI_RELOAD_PORT); 
   return spi_status;
}
