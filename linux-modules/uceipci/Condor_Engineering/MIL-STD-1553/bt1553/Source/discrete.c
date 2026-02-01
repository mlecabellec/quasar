 /*============================================================================*
 * FILE:                        D I S C R E T E . C
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
 * FUNCTION:   This file contains all function related to discrete and trigger
 *             functions.
 *
 * USER ENTRY POINTS: 
 *     BusTools_DiscreteGetIO        - Reads the discrete input signals setting
 *     BusTools_PIO_GetIO            - Reads the PIO input setting
 *     BusTools_DiscreteRead         - Reads the discrete input signals values
 *     BusTools_PIO_Read             - Reads the PIO input signals values
 *     BusTools_DiscreteReadRegister - Read the discrete registers
 *     BusTools_DiscreteSetIO        - Configures the discrete signals as input/outputs
 *     BusTools_PIO_SetIO            - Configures the PIO signals as input/outputs
 *     BusTools_DiscreteWrite        - Writes the discrete output signals
 *     BusTools_PIO_Write            - Writes the PIO output signals
 *     BusTools_DiscreteTriggerOut   - Sets which channels use discrete 7 and 8
 *     BusTools_DiscreteTriggerIn    - Sets the input trigger
 *     BusTools_DiffTriggerOut       - Sets which channel uses the differential output
 *     BusTools_ExtTrigIntEnable     - Enables interrupt on External Trigger.
 *     BusTools_RS485_TX_Enable      - Enables the 8 485-transmit signals as outputs
 *     BusTools_RS485_Set_TX_Data    - Sets the RS-485 transmit data
 *     BusTools_RS485_ReadRegs       - Read the 485 reigsters
 *     BusTools_SetMultipleExtTrig   - Enables multiple output triggers
 *     BusTools_SetTermEnable        - Set the differential discrete termination (RXMC2 and LPCIe)
 *     BusTools_V6_SetDiscreteBlock  - Set a block of discretes
 *     BusTools_SetV6TrigOut         - Trigger out settings for V6 boards
 *     BusTools_SetV6TrigIn          - Trigger in setting for V6 boards
 *     BusTools_RS485_V6Ctrl         - V6 485 control
 *     BusTools_RS485_V6Data         - V6 485 data 
 *     BusTools_GetValidPio          - Reads Global PIO option register to get valid board PIO.
 *     BusTools_GetValidDiff         - Reads Global Differential option register to get valid board diff.
 *     BusTools_GetValidDiscrete     - Reads Global Discrete option register to get valid board discretes.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *===========================================================================*/

/* $Revision:  8.28 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  11/19/2007   Added support for the R15-EC in functions BusTools_DiscreteGetIO, 
               BusTools_DiscreteRead, BusTools_DiscreteSetIO, and BusTools_DiscreteWrite
  12/28/2009   Add PIO functions for RXMC-1553
  04/11/2011   Add support for multiple output trigger for RXMC2-1553 and LPCIE-1553
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API  
  08/02/2012   Added new functions for V6 differential discretes
  04/26/2013   Update PIO function for V6 boards.
  04/26/2013   Add support for LPCIE-1553.
  12/04/2019   Modified BusTools_GetValidDiscrete for FW v5. bch
*/

#include <stdio.h>
#include <stdlib.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"


/****************************************************************
*
*  PROCEDURE NAME - BusTools_V6_SetDiscreteBlock
*
*  FUNCTION
*     This routine writes the discrete output signals setting for
*     all discrete channels on the board.  This also sets
*     up output discrete by setting the ouput channels high.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_V6_SetDiscreteBlock(
   BT_UINT cardnum,            // (i) card number
   BT_U32BIT disValue)         // (i) discrete values 0-ground 1-pull-up to 3.3v or for input
                               //     bit encoded with bit 1 = discrete 1; bit 2 = discrete 2....
                               //     bit 0 is not used.
{
   BT_U32BIT tval;
   BT_UINT dindex,somevalue,someindex;
	
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if((_HW_FPGARev[cardnum] &0x0fff) > 0x600)
   {
      someindex=1;
      somevalue=0;
   }
   else
   {
       someindex=0;
       somevalue=1;
   }         

   for(dindex=someindex; dindex<=numDiscretes[cardnum];dindex++)  
   { 
      tval = dindex+somevalue;                           
      if(disValue & (0x1 << dindex))
         tval |= 0x100; // set the data bit.  

      vbtSetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT,tval);
   }
 
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteGetIO
*
*  FUNCTION
*     This routine read the discrete input signals setting
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteGetIO(
   BT_UINT cardnum,        // (i) card number
   BT_U32BIT *disDirValue)    // (o) discrete values
{	
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum])
   {
      BT_U32BIT temp1,temp2;
      BT_U32BIT temp3;

      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);
      temp3 = (BT_U32BIT)((temp1 & 0xffff) | (temp2 << 16));
      temp3 &= bt_dismask[cardnum];
 
      if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
         temp3>>=6;

      *disDirValue = temp3;
   }
   else
   {
      *disDirValue = vbtGetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT);
   }
 
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_GetIO
*
*  FUNCTION
*     This routine read the PIO input signals setting
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_GetIO(
   BT_UINT cardnum,        // (i) card number
   BT_U32BIT *pioDirValue)    // (o) discrete values
{
   BT_U32BIT temp1,temp2;
   BT_U32BIT temp3;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum])
   {	
      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);
      temp3 = (BT_U32BIT)((temp1 & 0xffff) | (temp2 << 16));
      temp3 &= bt_piomask[cardnum];
 
      temp3>>=4;
      *pioDirValue = temp3;
   }
   else
   {
      return API_HARDWARE_NOSUPPORT;  // This can be done with V6.
   }
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteRead
*
*  FUNCTION
*     This routine reads the discrete input signals
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteRead(
   BT_UINT cardnum,        // (i) card number
   BT_INT disSel,          // (i) Discrete select = 0 all
   BT_U32BIT *disValue)    // (o) discrete values
{
   BT_U32BIT  testbit = 1;
   BT_U32BIT  temp1,temp2;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum])
   {
      if(disSel > (BT_INT)numDiscretes[cardnum])
         return API_BAD_DISCRETE;

      if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
      {
         if(disSel != 0)
         {
            disSel+=6;
         }
      }
      
      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_IN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_IN2);
   
      *disValue = (BT_U32BIT)(temp1 | (temp2<<16));
      *disValue &= bt_dismask[cardnum];

      if(disSel == 0)
      {
	     return API_SUCCESS;
      }
      else
      {
          *disValue &= (testbit << (disSel-1));
	      if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
	      {
		      *disValue>>=6;
	      }
      }
      return API_SUCCESS;
   }
   else
   {
      BT_U32BIT somevalue;
     
      if((_HW_FPGARev[cardnum] &0x0fff) > 0x600)
      {
         somevalue = 0; 
      }
      else 
      {
         somevalue = 1;  
      }

      *disValue = vbtGetDiscrete[cardnum](cardnum,V6_DISCRETE_IN); 

      if(disSel == 0)
      {
	      return API_SUCCESS;
      }
      else
      {
         *disValue &= (testbit << (disSel-somevalue));
      }
      return API_SUCCESS;
   } 
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_Read
*
*  FUNCTION
*     This routine read the discrete input signals
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_Read(
   BT_UINT cardnum,        // (i) card number
   BT_INT pioSel,          // (i) Discrete select = 0 all
   BT_U32BIT *pioValue)    // (o) discrete values
{
   BT_U32BIT  temp1,temp2;
   BT_U32BIT  testbit = 1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;
      
   if(pioSel > numPIO[cardnum])
      return API_BAD_DISCRETE;

   if(board_is_v5_uca[cardnum])
   {
      if(pioSel != 0)
      {
         pioSel+=4;
      }

      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_IN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_IN2);
   
      *pioValue = (BT_U32BIT)(temp1 | (temp2<<16));
      *pioValue &= bt_piomask[cardnum];

      if(pioSel == 0)
      {
	      return API_SUCCESS;
      }
      else
      {
          *pioValue &= (testbit << (pioSel-1));
	      *pioValue>>=4;
      }
   }
   else
   {
      *pioValue = v6GetDiscrete(cardnum,GLBREG_PIO_CTRL);    
      return API_SUCCESS;
   }
   return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteReadRegister
*
*  FUNCTION
*     This routine reads the discrete/PIO/485 input signals
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteReadRegister(
   BT_UINT cardnum,        // (i) card number
   BT_INT regnum,          // (i) Discrete select = 0 all
   BT_U32BIT *disValue)    // (o) discrete values
{
   BT_U32BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

  
   temp1 = vbtGetDiscrete[cardnum](cardnum,regnum);
   *disValue = temp1;

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteSetIO
*
*  FUNCTION
*     This routine configures the discrete signals as input/outputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteSetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disSet,        // (i) discrete flags
   BT_U32BIT mask)          // (i) mask
{
   BT_U32BIT  disval;
   BT_U32BIT  temp,tval;
   BT_U32BIT  temp1,temp2;
   BT_UINT dindex;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum]) 
   {
      if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
      {
         mask &=0x3; //only 2 discretes
         mask<<=6;   //only discrete 7 and 8 active so shift over
	     disSet<<=6;
      }
      else //for QPCI-1553 and Q104-1553 and Q104-1553P
         mask &= bt_dismask[cardnum];    //

      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);
      disval = (BT_U32BIT)(temp1 | (temp2<<16));
   
      disval = (BT_U32BIT)((disval & ~mask) | (disSet & mask));
   
      temp1 = (BT_U16BIT)(disval & 0xffff);
      temp2 = (BT_U16BIT)((disval & 0xffff0000)>>16);
 
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1,temp1);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2,temp2);
   }
   else
   {
      mask &= bt_dismask[cardnum];  
      temp = vbtGetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT);
      disval = (BT_U32BIT)((temp & ~mask) | (disSet & mask));
      for(dindex=1; dindex<numDiscretes[cardnum];dindex++)
      {
         tval = dindex;
         if(disval & (0x1 << dindex))
            tval |= 0x100; // set the data bit

         vbtSetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT,tval);
      }
   }

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_SetIO
*
*  FUNCTION
*     This routine configures the PIO as input/outputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_SetIO(
   BT_UINT cardnum,         // (i) card number
   BT_U32BIT disSet,        // (i) discrete flags
   BT_U32BIT mask)          // (i) mask
{
   BT_U32BIT  disval;
   BT_U32BIT  temp1,temp2;
   BT_U32BIT  dindex,tval;
   BT_U32BIT  dtemp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum]) 
   {
      mask = (mask<<4);
      mask &= bt_piomask[cardnum];    //
      disSet = disSet << 4;

      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);
      disval = (BT_U32BIT)(temp1 | (temp2<<16));
   
      disval = (BT_U32BIT)((disval & ~mask) | (disSet & mask));
   
      temp1 = (BT_U16BIT)(disval & 0xffff);
      temp2 = (BT_U16BIT)((disval & 0xffff0000)>>16);

      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1,temp1);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2,temp2);
   }
   else
   {
      dtemp = bt_piomask[cardnum];
      mask &= dtemp;    // only look at the available PIO discretes
      disval = (BT_U32BIT)(disSet & mask);
      // for all potential PIO discretes
      for(dindex=0; dindex<32;dindex++)
      { 
         // if not defined, keep looking
         if((dtemp & (1<<dindex)) == 0)
            continue;
         // set output or input as selected by the caller. PIOs are 0,1,2...
         tval = dindex;
         if(disval & (0x1 << dindex)) 
            tval |= 0x800; // this is a output PIO
         else
            tval |= 0x400; // this is a input PIO

         v6SetDiscrete(cardnum,GLBREG_PIO_CTRL,tval);
      }
   }
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteWrite
*
*  FUNCTION
*     This routine writes the discrete output signals
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiscreteWrite(
   BT_UINT cardnum,        // (i) card number
   BT_INT  disSel,         // (i) Discrete select 0 = all
   BT_UINT disFlag)        // (i) 0 set the discrete 1 reset the discrete
{
   BT_U32BIT  temp1,temp2,tval,temp3,temp4;
   BT_U32BIT  testbit = 1;
   BT_U32BIT  disValue;
   BT_U32BIT  disdir;
   BT_UINT dindex;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum])
   {

      if(disSel > (BT_INT)numDiscretes[cardnum])
         return API_BAD_DISCRETE;
      
      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);

      temp3 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUT1);
      temp4 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUT2);

      disdir = (BT_U32BIT)(temp1 | (temp2<<16));
      disValue = (BT_U32BIT)(temp3 | (temp4<<16));

      if(disSel == 0)  //Set all output discrete to the value of disFlag
      {
	     if(disFlag)
            disValue = disdir;
         else
            disValue = 0;
      }
      else
      {
         if(CurrentCardType[cardnum]==PCCD1553 || CurrentCardType[cardnum]==R15EC)
            disSel+=6;

         if(disFlag)
            disValue |= (testbit << (disSel-1));
         else
            disValue &= ~(testbit <<(disSel-1));

      }

      disValue &= bt_dismask[cardnum];
      temp1 = (BT_U16BIT)(disValue & 0xffff);
      temp2 = (BT_U16BIT)((disValue & 0xffff0000) >> 16);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUT1,temp1);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUT2,temp2);
   }
   else
   {
      BT_U32BIT dtemp;
      BT_INT someindex,somevalue;
      
      if((_HW_FPGARev[cardnum] &0x0fff) > 0x600)
      {
         someindex=1;
         somevalue=0;
      }
      else
      {
         someindex=0;
         somevalue=1;
      }         

      dtemp = bt_dismask[cardnum];
      if(disSel == 0)
      {
         for(dindex=someindex; dindex<32;dindex++)
         {  
            if((dtemp & (1<<dindex)) == 0)
               continue;

            if(disFlag)
               tval = 0x100; // set the data bit low
            else
               tval = 0x0;   // set the data bit high

            tval += dindex + somevalue;  // add in the discrete channel
            vbtSetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT,tval);
         }
      }
      else
      {
         if(disFlag)
            tval = 0x100; // set the data bit low
         else
            tval = 0x0;   // set the data bit high

         tval += disSel;  // add in the discrete channel
         vbtSetDiscrete[cardnum](cardnum,V6_DISCRETE_OUT,tval);
      }          
   }

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_PIO_Write
*
*  FUNCTION
*     This routine writes the PIO output signals
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_PIO_Write(
   BT_UINT cardnum,        // (i) card number
   BT_INT  pioSel,         // (i) Discrete select (0 = all for V5) (-1 all for V6)
   BT_UINT pioFlag)        // (i) 0 reset the discrete 1 set the discrete
{
   BT_U32BIT  temp1,temp2,temp3,temp4;
   BT_U32BIT  testbit = 1;
   BT_U32BIT  disValue;
   BT_U32BIT  disdir;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(pioSel > numPIO[cardnum])
         return API_BAD_DISCRETE;

   if(board_is_v5_uca[cardnum])
   {
      temp1 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN1);
      temp2 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUTEN2);

      temp3 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUT1);
      temp4 = vbtGetDiscrete[cardnum](cardnum,DISREG_DIS_OUT2);

      disdir = (BT_U32BIT)(temp1 | (temp2<<16));
      disValue = (BT_U32BIT)(temp3 | (temp4<<16));

      if(pioSel == 0)  //Set all output discrete to the value of disFlag
      {
	     if(pioFlag)
            disValue = disdir;
         else
            disValue = 0;
      }
      else
      {
         pioSel+=4;

         if(pioFlag)
            disValue |= (testbit << (pioSel-1));
         else
            disValue &= ~(testbit <<(pioSel-1));
      }

      disValue &= bt_piomask[cardnum];
      temp1 = (BT_U16BIT)(disValue & 0xffff);
      temp2 = (BT_U16BIT)((disValue & 0xffff0000) >> 16);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUT1,temp1);
      vbtSetDiscrete[cardnum](cardnum,DISREG_DIS_OUT2,temp2);
   }
   else
   {
      BT_U32BIT dtemp,tval;
      BT_INT dindex;
      

      dtemp = bt_piomask[cardnum];
      if(pioSel == -1)
      {
         for(dindex=0; dindex<32;dindex++)
         {  
            if((dtemp & (1<<dindex)) == 0)
               continue;

            if(pioFlag)
               tval = 0x200;   // set the data bit high
            else
               tval = 0x100;   // set the data bit low

            tval += dindex;    // add in the discrete channel
            v6SetDiscrete(cardnum,GLBREG_PIO_CTRL,tval);
         }
      }
      else
      {
         if(pioFlag)
            tval = 0x200; // set the data bit high
         else
            tval = 0x100; // set the data bit low

         tval += pioSel;  // add in the discrete channel
         v6SetDiscrete(cardnum,GLBREG_PIO_CTRL,tval);
      }          
   }

   return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE NAME - BusTools_GetValidPio
*
*  FUNCTION
*     This routine reads the Global Discrete option register to
*     determine the valid discrete available for the board. 
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetValidPio(
   BT_UINT cardnum,           // (i) card number
   BT_U32BIT * pioSetting)    // (i) returns valid PIO
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_pio[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if((_HW_FPGARev[cardnum] &0x0fff) < 0x601)
      return API_HARDWARE_NOSUPPORT;

   *pioSetting = vbtGetCSCRegister[cardnum](cardnum,GLBREG_PIO_OPT);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_GetValidDiff
*
*  FUNCTION
*     This routine reads the Global Discrete option register to
*     determine the valid discrete available for the board. 
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetValidDiff(
   BT_UINT cardnum,           // (i) card number
   BT_U32BIT * diffSetting)    // (i) returns valid differential
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_differential[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if((_HW_FPGARev[cardnum] &0x0fff) < 0x601)
      return API_HARDWARE_NOSUPPORT;

   *diffSetting = vbtGetCSCRegister[cardnum](cardnum,GLBREG_DIFF_OPT);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_GetValidDiscrete
*
*  FUNCTION
*     This routine reads the Global Discrete option register to
*     determine the valid discrete available for the board. 
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetValidDiscrete(
   BT_UINT cardnum,           // (i) card number
   BT_U32BIT * disSetting)    // (i) returns valid discrete
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if((_HW_FPGARev[cardnum] & 0x0fff) < 0x601)
     return API_HARDWARE_NOSUPPORT;

   *disSetting = vbtGetCSCRegister[cardnum](cardnum,GLBREG_DISCR_OPT);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - v5_DiscreteTriggerOut
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger outputs.  You must program discrete 7 and 8 as 
*     outputs when you call BusTools_DiscreteSetIO.
*
****************************************************************/
NOMANGLE BT_INT CCONV v5_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag)          // (i) TRIGGER_OUT_DIS_7 - use Discrete 7 as trigger out
                            //     TRIGGER_OUT_DIS_8 - use Discrete 8 as trigger out
                            //     TRIGGER_OUT_DIS_NONE - use neither
{
   BT_U16BIT temp;
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   temp = (BT_U16BIT)vbtGetDiscrete[cardnum](cardnum,DISREG_TRIG_CLK);

   switch(CurrentCardSlot[cardnum])
   {
   case 0:
      temp &= ~0x11;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x1;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x10; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 1:
      temp &= ~0x22;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x2;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x20; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 2:
      temp &= ~0x44;  // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x4;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x40; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   case 3:
      temp &= ~0x88;   // clear the disrete setting 
      if(disflag == TRIGGER_OUT_DIS_7)
         temp |= 0x8;  // set discrete 7 as trigger
	  else if(disflag == TRIGGER_OUT_DIS_8)
         temp |= 0x80; // set discrete 8 as trigger
      else if(disflag == TRIGGER_OUT_NONE)
         break;        // already done
      else
         return  API_BAD_PARAM; //this is an error
	  break;
   }

   vbtSetDiscrete[cardnum](cardnum,DISREG_TRIG_CLK,temp);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - v6_DiscreteTriggerOut
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger outputs.  You must program discrete 7 and 8 as 
*     outputs when you call BusTools_DiscreteSetIO.
*
****************************************************************/
NOMANGLE BT_INT CCONV v6_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag)          // (i) select one or more output trigger
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   vbtSetTrigRegister[cardnum](cardnum,TRIG_DIS_OUT_SEL,disflag);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiscreteTriggerOut
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger outputs.  You must program discrete 7 and 8 as 
*     outputs when you call BusTools_DiscreteSetIO.
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT disflag)          // (i) select one or more output trigger
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_DiscreteTriggerOut[cardnum](cardnum,disflag);
}

/****************************************************************
*
*  PROCEDURE NAME - v5_DiscreteTriggerIn
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger input.  You must program discrete 7 and 8 as 
*     inputs
*
****************************************************************/

NOMANGLE BT_INT CCONV v5_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT  trigflag)        // (i) 0=none, 1=Differential, 2=dis7 3=dis8 
{
	BT_U16BIT tempVal[4][4] = {
	{0x0,0x100,0x200,0x300},
	{0x0,0x400,0x800,0xc00},
	{0x0,0x1000,0x2000,0x3000},
	{0x0,0x4000,0x8000,0xc000}
	};

   BT_U16BIT temp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(trigflag>3)
      return API_BAD_PARAM;

   temp = (BT_U16BIT)vbtGetDiscrete[cardnum](cardnum,DISREG_TRIG_CLK);      // Read the Setting
   temp &= ~tempVal[CurrentCardSlot[cardnum]][3];       // Clear the Channel setting
   temp |= tempVal[CurrentCardSlot[cardnum]][trigflag]; // Set the Channel
 
   vbtSetDiscrete[cardnum](cardnum,DISREG_TRIG_CLK,temp);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - v6_DiscreteTriggerIn
*
*  FUNCTION
*     This routine configures which channels use discrete 7 and 8
*     as trigger input.  You must program discrete 7 and 8 as 
*     inputs
*
****************************************************************/

NOMANGLE BT_INT CCONV v6_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag)         // (i) 0=none,  1 to  31 discrete channel
                            //             -1 to -31 differential channel
{
   union{                         
           V6_TRIG_CTRL  trgInCtrl; 
           BT_U32BIT     trgInVal;  
   }trg; 

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(abs(trigflag)>31)
      return API_BAD_PARAM;

   if(trigflag) //Set trigger if selected discrete channel is not zero
   {
      trg.trgInVal = 0;     // each setting overwrites the previous setting.
      trg.trgInCtrl.trigCH = trigflag;
      trg.trgInCtrl.trigSrc = DISCRETE;
      vbtSetTrigRegister[cardnum](cardnum,EXT_TRGIN_CTRL,trg.trgInVal);
   }
   else//Clear Trigger
   {
      trg.trgInVal = 0; 
      vbtSetTrigRegister[cardnum](cardnum,EXT_TRGIN_CTRL,trg.trgInVal);
   }

   return API_SUCCESS;
}

NOMANGLE BT_INT CCONV BusTools_DiscreteTriggerIn(
   BT_UINT cardnum,         // (i) card number
   BT_INT trigflag)         // (i) 0=none,  1 to  31 discrete channel
                            //             -1 to -31 differential channel
{   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   return BT_DiscreteTriggerIn[cardnum](cardnum,trigflag);
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_DiffTriggerOut
*
*  FUNCTION
*     This routine configures which channels use the differential
*     outputs.  You must program discrete 7 and 8 as 
*     outputs
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_DiffTriggerOut(
   BT_UINT cardnum,         // (i) card number
   BT_INT  chflag,          // (i) differential channel(s) bit encoded
   BT_INT  diffen)          // (i) not used 
{
   BT_U16BIT temp;

   BT_U16BIT tval[4]={1,2,4,8};
   BT_U16BIT diff_enable = 0x10;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_differential[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(board_is_v5_uca[cardnum])
   {
      temp =  (BT_U16BIT)vbtGetDiscrete[cardnum](cardnum,DISREG_DIFF_IO);
      if(chflag)
        temp |= tval[CurrentCardSlot[cardnum]];
      else
        temp &= ~tval[CurrentCardSlot[cardnum]];

      if(diffen)
         temp |= diff_enable;
      else
         temp &= ~diff_enable;

      vbtSetDiscrete[cardnum](cardnum,DISREG_DIFF_IO,temp);
      return API_SUCCESS;
   }
   else
      return API_HARDWARE_NOSUPPORT;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_TX_ENABLE
*
*  FUNCTION
*     This routine enables the 8 485-transmit signals as outputs
*     on the QPM-1553 boards.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_TX_Enable(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT enable,        // (i) discrete flags
   BT_U16BIT mask)          // (i) mask
{
   BT_U32BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(enable > 0xff || mask > 0xff)
      return API_BAD_DISCRETE;

   if(board_is_v5_uca[cardnum])
   {
      temp1 = vbtGetDiscrete[cardnum](cardnum,RS485_TX_ENABLE);
      temp1 &= 0xff;  // only 8 RS 485 transmitters.
   
      temp1 = ((temp1 & ~mask) | (enable & mask));

      vbtSetDiscrete[cardnum](cardnum,RS485_TX_ENABLE,temp1);
      return API_SUCCESS;
   }
   else
      return API_HARDWARE_NOSUPPORT;
}

/*****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_V6CTRL
*
*  FUNCTION
*     This routine enables up to 8 485-transmit signals as outputs
*     on V6 F/W boards.
*
******************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_V6Ctrl(
   BT_UINT cardnum,         // (i) card number
   BT_UINT dchan,            // (i) diff channel (1 - 8)
   BT_UINT ddata,            // (i) Data. Set this bit for 485+ pin = 1 and 485- pin = 0. 
                            //     Clear the bit for 485+ pin = 0 and 485- pin = 1.
   BT_UINT dtxen,            // (i) Transmit Enable. Set the bit for the 485 device to transmit
                            //     Clear the bit for the device to receive.
   BT_UINT dterm)           // (i) Termination. Setting this bit enables a 120 ohm termination within the 485 transceiver. 
                            // (i) Clear this bit for no termination. This feature is board specific.
{
   BT_U32BIT  temp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(dchan > 8)
      return API_BAD_PARAM;

   temp = dchan;  // set the differential channel
   
   if(ddata)
      temp |= 0x100; //set the data bit high

   if(dtxen)
      temp |= 0x200; //set tx enable
   
   if(dterm)
      temp |= 0x400; //set 120 ohm termination

   v6SetDiscrete(cardnum, GLBREG_RS485_CTL,temp);   

   return API_SUCCESS;
}

/*****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_V6Data
*
*  FUNCTION
*     This routine enables up to 8 485-transmit signals as outputs
*     on V6 F/W boards.
*
******************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_V6Data(
   BT_UINT cardnum,         // (i) card number
   BT_INT  difFlag,         // (i) Set for RS485_TXDA_REG for transmit data
                            //     Set for RS485_RXDA_REG for receive data
   BT_U32BIT *ddata)        // (i) RS485 Data (BIT 0 not used)
{
   BT_U32BIT  temp;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   temp = v6GetDiscrete(cardnum, GLBREG_RS485_DATA);
   
   if(difFlag == RS485_TXDA_REG)
      *ddata = (temp & 0xfffe0000) >> 16;
   else
      *ddata = temp & 0xffe;

   return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_SET_TX_DATA
*
*  FUNCTION
*     This routine sets/reset the 8 485-transmit signals.
*
*****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_Set_TX_Data(
   BT_UINT cardnum,         // (i) card number
   BT_U16BIT rsdata,        // (i) transmit data pattern
   BT_U16BIT mask)          // (i) mask
{
   BT_U32BIT  temp1;

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(rsdata > 0xff || mask > 0xff)
      return API_BAD_DISCRETE;   

   temp1 = vbtGetDiscrete[cardnum](cardnum,RS485_TX_DATA);
   temp1 &= 0xff;  // only 8 RS 485 transmitters.
   
   temp1 = (temp1 & ~mask) | (rsdata & mask);

   vbtSetDiscrete[cardnum](cardnum,RS485_TX_DATA,temp1);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_RS485_READ_REGS
*
*  FUNCTION
*     This routine enables the 8 485-Receive signals as inputs
*     on the QPM-1553 boards.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_RS485_ReadRegs(
   BT_UINT cardnum,            // (i) card number
   BT_INT  regval,             // (i) RS 485 register to read
                               //     RS485_TXEN_REG
                               //     RS485_TXDA_REG
                               //     RS485_RXDA_REG
   BT_U32BIT *rsdata)          // (o) RS 485 register data
{
   BT_UINT reg_index[] = {RS485_TX_ENABLE,
                          RS485_TX_DATA,
                          RS485_RX_DATA};

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if (!board_has_485_discretes[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(regval > RS485_RX_DATA)
      return API_BAD_PARAM;   
   
   *rsdata = vbtGetDiscrete[cardnum](cardnum,reg_index[regval]);
   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetMultipleExtTrig
*
*  FUNCTION
*     This routine select which discrete/PIO/485 channel will act
*     as external triggers.  A single channel can program multiple
*     output triggers and multiple channels can select the same
*     trigger channel.
*
*     The RXMC has multiple out configurations. You must know the 
*     configuration of RXMC-1553 to correctly select triggers.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetMultipleExtTrig(
   BT_UINT cardnum,            // (i) card number
   BT_INT  trigOpt,			   // (i) PIO,DISCRETE,EIA485
   BT_UINT tvalue,             // (i) trigger channel (1 - 12)
   BT_INT  enableFlag)         // (i) ENABLE_TRIG or DISABLE_TRIG
{
   BT_U32BIT trig_reg;

   BT_U32BIT eia485val[] = {0x0,0x1000,0x2000,0x4000,0x8000};
   BT_U32BIT disval[] = {0x0,0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800};
   BT_U32BIT pioval[] = {0x0,0x10,0x20,0x40,0x80,0x100,0x200,0x400,0x800};
   BT_UINT trig_addr[] = {EXT_TRIG_OUT_CH1, EXT_TRIG_OUT_CH2, EXT_TRIG_OUT_CH3, EXT_TRIG_OUT_CH4};

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(!boardHasMultipleTriggers[cardnum])
      return API_HARDWARE_NOSUPPORT;

   trig_reg = vbtGetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]]);

   if(CurrentCardType[cardnum]==RXMC1553)
   {
      switch(rxmc_output_config[cardnum])
      {
      case NO_OUTPUT:
         return API_HARDWARE_NOSUPPORT;
      case PIO_OPN_GRN:
      case PIO_28V_OPN:
         //This configuration has 12 possible triggers 4 Discrete + 8 PIO
         switch(trigOpt)
         {
            case EIA485:
               return API_HARDWARE_NOSUPPORT;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                     vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                     vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               if((tvalue < 1) || (tvalue > 8))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | pioval[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~pioval[tvalue]));
               break;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      case DIS_OPN_GRN:
      case DIS_28V_OPN:
         //This configuration has 12 possible 4 + 8 Discrete
         switch(trigOpt)
         {
            case EIA485:
               return API_HARDWARE_NOSUPPORT;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 12))
                  return API_HARDWARE_NOSUPPORT;
               else
                   if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               return API_HARDWARE_NOSUPPORT;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      case EIA485_OPN_GRN:
      case EIA485_28V_OPN:

         //This configuration has 8 possible triggers 4 EIA485 + 4 Discrete
         switch(trigOpt)
         {
            case EIA485:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | eia485val[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~eia485val[tvalue]));
               break;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               return API_HARDWARE_NOSUPPORT;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;

       case EIA485_OPN_GRN_DIS8:

         //This configuration has 11 possible triggers 3 EIA485 + 8 Discrete
         switch(trigOpt)
         {
            case EIA485:
               if( (tvalue < 2) || (tvalue > 4))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | eia485val[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~eia485val[tvalue]));
               break;
            case DISCRETE:
               if( (tvalue < 1) || (tvalue > 8))
                  return API_HARDWARE_NOSUPPORT;
               else
                  if(enableFlag)
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | disval[tvalue]));
                  else
                      vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~disval[tvalue]));
               break;
            case PIO:
               return API_HARDWARE_NOSUPPORT;
           default:
              return API_HARDWARE_NOSUPPORT;
         }
         break;
      default:
         return API_HARDWARE_NOSUPPORT;
      }
      return API_SUCCESS;
   }
   else
   {
      if( (tvalue < 1) || (tvalue > numDiscretes[cardnum]))
         return API_BAD_PARAM;
      else
         if(enableFlag)
            vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg | disval[tvalue]));
         else
            vbtSetDiscrete[cardnum](cardnum,trig_addr[CurrentCardSlot[cardnum]],(BT_U16BIT)(trig_reg & ~disval[tvalue]));

      return API_SUCCESS;
   }
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetV6TrigIn
*
*  FUNCTION
*     This routine selects which discrete channel will act
*     as external trigger input for the 1553 channel. 
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetV6TrigIn(
   BT_UINT cardnum,            // (i) card number
   BT_INT  trigOpt,			   // (i) PIO,DISCRETE,EIA485
   BT_UINT tvalue)             // (i) trigger channel (1 - 18) 
{
   union{   
          V6_TRIG_CTRL trgInCtrl; 
          BT_U32BIT   trgInVal;
   }trg;  

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;

   if(CurrentCardType[cardnum] == R15USB)
   {
      if((trigOpt == PIO) || (trigOpt == EIA485))
         return API_HARDWARE_NOSUPPORT; 
   }
      
   trg.trgInVal = 0;     
   trg.trgInCtrl.trigCH  = tvalue;
   trg.trgInCtrl.trigSrc = trigOpt;
   vbtSetTrigRegister[cardnum](cardnum,EXT_TRGIN_CTRL,trg.trgInVal);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetV6TrigOut
*
*  FUNCTION
*     This routine selects which discrete channel will act
*     as external trigger output for the 1553 channel. 
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetV6TrigOut(
   BT_UINT cardnum,            // (i) card number
   BT_INT  trigOpt,			   // (i) PIO,DISCRETE,EIA485   
   BT_UINT tvalue)             // (i) trigger channel (1 - 18) 
{ 

   union {   
          V6_TRIG_CTRL trgOutCtrl; 
          BT_U32BIT    trgOutVal;
   }trg;  

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(board_is_v5_uca[cardnum])
      return API_HARDWARE_NOSUPPORT;
 
   if(CurrentCardType[cardnum] == R15USB)
   {
      if((trigOpt == PIO) || (trigOpt == EIA485))
         return API_HARDWARE_NOSUPPORT; 
   }

   // each setting overwrites the previous setting.
   // Thus the "enable/disable" flag is no longer needed. The caller
   // just sets "tvalue" = zero to disable triggers.
   trg.trgOutVal = 0;     
   trg.trgOutCtrl.trigCH  = tvalue;
   trg.trgOutCtrl.trigSrc = trigOpt;
   vbtSetTrigRegister[cardnum](cardnum,EXT_TRGOUT_CTRL,trg.trgOutVal);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_SetTermEnable
*
*  FUNCTION
*     This routine enables/disable termination on the differential
*     discrete lines.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_SetTermEnable(
   BT_UINT cardnum,            // (i) card number
   BT_U16BIT  tEnable)         // (i) enable/disable setting 
{

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   vbtSetDiscrete[cardnum](cardnum,DISREG_TERM_EN,tEnable);

   return API_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE NAME - BusTools_GetTermEnable
*
*  FUNCTION
*     This routine reads the enables/disable differential
*     termination settings.
*
****************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetTermEnable(
   BT_UINT cardnum,    // (i) card number
   BT_U32BIT *tEnable) // (o) Enable setting
{

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   *tEnable = vbtGetDiscrete[cardnum](cardnum,DISREG_TERM_EN);

   return API_SUCCESS;
}
