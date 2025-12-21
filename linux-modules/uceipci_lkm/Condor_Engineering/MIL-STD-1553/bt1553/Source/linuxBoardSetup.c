/*============================================================================*
 * FILE:                L I N U X B O A R D S E T U P . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 1994 - 2017 BY ABACO SYSTEMS, INC.
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
 *             Intialization and configuration of devices in Linux. 
 *
 * DESCRIPTION:
 *
 * API ENTRY POINTS:
 *    BusTools_API_OpenChannel
 *    BusTools_FindDevice
 *    BusTools_ListDevices
 *    BusTools_GetDevInfo
 *    BusTools_SetDumpPath
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    vbtBoardAccessSetup
 *    get_64BitHostTimeTag
 *
 * INTERNAL ROUTINES:
 *    vbtOpen1553Channel
 *
 *===========================================================================*/

/* $Revision:  1.16 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/13/2003   Initial
  04/20/2005   Added ISA-1553. replaced vbtMapMilBoardAddresses with
                vbtMapBoardAddresses in vbtBoardAccessSetup. added
                BusTools_API_OpenChannel, vbtOpen1553Channel,
                BusTools_FindDevice, BusTools_ListDevices. bch
  10/07/2005   Added CEI-220 and CEI-420 to BusTools_ListDevices. bch
  09/13/2006   Added QPM1553 and AMC1553 to vbtBoardAccessSetup. bch
  03/28/2007   Added QPCX1553 to vbtBoardAccessSetup and
                BusTools_ListDevices. bch
  06/18/2007   Minor mods to vbtOpen1553Channel and BusTools_ListDevices. bch
  12/11/2007   Added R15EC to vbtBoardAccessSetup and BusTools_ListDevices. bch
  12/16/2008   Added R15AMC and AR15VPX to vbtBoardAccessSetup and 
                BusTools_ListDevices. bch
  04/10/2009   Added INCLUDE_LEGACY_PCI to vbtBoardAccessSetup. bch
  07/29/2009   Added support for RXMC-1553. added get_48BitHostTimeTag. bch 
  10/21/2009   Added support for RPCIe-1553. added BusTools_GetDevInfo. bch
  04/13/2011   added support for RXMC2-1553 and R15-LPCIe. added
                BusTools_SetDumpPath. modified BusTools_FindDevice. bch
  10/10/2012   added support for RAR15-XMC and RAR15-XMC-XT. added
                get_64BitHostTimeTag. bch
  06/25/2013   removed support for: AMC-1553, cPCI-1553, VME-1553, Q104-1553,
                AR15-VPX, RAR-15XMC, and IPD-1553. bch
  05/20/2014   modified BusTools_GetDevInfo for R15-USB. bch
  10/10/2016   added support for the R15-MPCIe. bch
  11/20/2017   modified vbtBoardAccessSetup. bch
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "busapi.h"
#include "apiint.h"
#include "globals.h"
#include "btdrv.h"


#define VENDOR_ID_R15USB  0x21A8
#define DEVICE_ID_R15USB  0x7503


/****************************************************************
*
*  PROCEDURE - vbtBoardAccessSetup
*
*  FUNCTION
*     Initialize the page access data elements and map the board
*      into the host address space.
*
*  PARAMETERS
*     BT_UNIT   cardnum   - the card number representing the device/channel
*     BT_U32BIT phys_addr - operation mode or the VME A32 address
*
*  RETURNS
*     BTD_OK
*     BTD_ERR_BADBOARDTYPE
*     BTD_ERR_NOACCESS
*     BTD_NON_SUPPORT
*
****************************************************************/
BT_INT vbtBoardAccessSetup(
   BT_UINT   cardnum,  
   BT_U32BIT phys_addr)
{
   BT_INT     status;
   char      *hostaddr;        // Mapped address of the board in host space.
   BT_U32BIT mapLength;

   /*********************************************************************
   *  First verify that this is a board type we can handle.
   *********************************************************************/
   if ( (CurrentCardType[cardnum] != QPMC1553)  &&
        (CurrentCardType[cardnum] != QPM1553)   &&
        (CurrentCardType[cardnum] != QCP1553)   &&
        (CurrentCardType[cardnum] != QVME1553)  &&
        (CurrentCardType[cardnum] != PCCD1553)  &&
        (CurrentCardType[cardnum] != Q1041553P) &&
        (CurrentCardType[cardnum] != QPCI1553)  &&
        (CurrentCardType[cardnum] != QPCX1553)  &&
        (CurrentCardType[cardnum] != R15EC)     &&
        (CurrentCardType[cardnum] != R15AMC)    &&
        (CurrentCardType[cardnum] != RXMC1553)  &&
        (CurrentCardType[cardnum] != RPCIe1553) &&
        (CurrentCardType[cardnum] != R15XMC2)   &&
        (CurrentCardType[cardnum] != LPCIE1553) &&
        (CurrentCardType[cardnum] != RAR15XMCXT) &&
        (CurrentCardType[cardnum] != R15USB)    &&
        (CurrentCardType[cardnum] != MPCIE1553)) {
      return BTD_ERR_BADBOARDTYPE;    // Unknown board type
   }

   /*********************************************************************
   *  Now map the board address(es) and the I/O address.
  *********************************************************************/
   if ((CurrentPlatform[cardnum] == PLATFORM_PC) && (CurrentCardType[cardnum] != QVME1553)) {
      if(CurrentCardType[cardnum] == R15USB)
        hostaddr = 0;
      else if((status = vbtMapBoardAddresses(phys_addr, &bt_devmap[api_device[cardnum]])) != BTD_OK)
        return status;

      if(bt_devmap[api_device[cardnum]].memSections == 1) {
         hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[0];
      }
      else if (bt_devmap[api_device[cardnum]].memSections == 2) {
         if((bt_devmap[api_device[cardnum]].memLengthBytes[0] != 0x80)  &&
              (bt_devmap[api_device[cardnum]].memLengthBytes[0] != 0x1000))
            return BTD_ERR_NOACCESS;
         if(bt_devmap[api_device[cardnum]].memLengthBytes[1] != 0x00800000) {
            return BTD_ERR_NOACCESS;
         }
         hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[1];
         bt_iobase[cardnum] = bt_devmap[api_device[cardnum]].memHostBase[0];
      }
      else if (bt_devmap[api_device[cardnum]].memSections == 3) {
         if((bt_devmap[api_device[cardnum]].memLengthBytes[0] != 0x200) &&
              (bt_devmap[api_device[cardnum]].memLengthBytes[0] != 0x1000))
            return BTD_ERR_NOACCESS;
         if(bt_devmap[api_device[cardnum]].memLengthBytes[2] != 0x00800000) {
            return BTD_ERR_NOACCESS;
         }
         hostaddr = (char *)bt_devmap[api_device[cardnum]].memHostBase[2];
         bt_iobase[cardnum] = bt_devmap[api_device[cardnum]].memHostBase[0];
      }
   }
  #ifdef INCLUDE_VME_VXI_1553
   else if (CurrentCardType[cardnum] == QVME1553) {
      /* map the A16 address space */
      if((status = vbtMapBoardAddress(bt_iobase[cardnum], 0x40, (void *)&bt_PageAddr[cardnum][1], &cardnum, CARRIER_MAP_A16)) != BTD_OK)
         return status;
      bt_iobase[cardnum] = bt_PageAddr[cardnum][1];
      
      /* map the A32 address space for 4 channels (each channel is 2MB)  */
      if((status = vbtMapBoardAddress(phys_addr, 0x800000, (void *)&bt_PageAddr[cardnum][0], &cardnum, CurrentMemMap[cardnum])) != BTD_OK)
         return status;
      hostaddr = bt_PageAddr[cardnum][0];
   }
  #endif
   else
      return BTD_NON_SUPPORT;  /* platform specified unknown or not supported */

   boardBaseAddress[cardnum] = hostaddr;

   /*********************************************************************
   *  Setup the access parameters for the Quad-PMC-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == QPMC1553 ||
       CurrentCardType[cardnum] == QPM1553  ||
       CurrentCardType[cardnum] == R15AMC) {
      return vbtPageAccessSetupQPMC(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the QCP-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == QCP1553) {
      return vbtPageAccessSetupQCP(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the Q104-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == Q1041553P) {
      return vbtPageAccessSetupQ104(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the QPCX-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == QPCX1553 || CurrentCardType[cardnum] == QPCI1553) {
      return vbtPageAccessSetupQPCX(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the R15-EC.
    **********************************************************************/
   if (CurrentCardType[cardnum] == R15EC) {
      return vbtPageAccessSetupR15EC(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the RXMC-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == RXMC1553) {
      return vbtPageAccessSetupRXMC(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the RPCIe-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == RPCIe1553) {
      return vbtPageAccessSetupQPMC(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the RXMC2-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == R15XMC2) {
      return vbtPageAccessSetupRXMC2(cardnum, hostaddr);
   }
   /*********************************************************************
    * Setup the access parameters for the R15-LPCIe.
    **********************************************************************/
   if (CurrentCardType[cardnum] == LPCIE1553) {
      return vbtPageAccessSetupLPCIE(cardnum, hostaddr);
   }

   /*********************************************************************
    * Setup the access parameters for the RAR15-XMC-XT.
    **********************************************************************/
   if (CurrentCardType[cardnum] == RAR15XMCXT) {
      return vbtPageAccessSetupRAR15XT(cardnum, hostaddr);
   }

   /*********************************************************************
    * Setup the access parameters for the R15-MPCIe.
    **********************************************************************/
   if (CurrentCardType[cardnum] == MPCIE1553) {
      return vbtPageAccessSetupMPCIE(cardnum, hostaddr);
   }
   
#ifdef INCLUDE_PCCD
   /*********************************************************************
    * Setup the access parameters for the PCC-1553.
    **********************************************************************/
   if (CurrentCardType[cardnum] == PCCD1553 ) {
      return vbtPageAccessSetupPCCD(cardnum, hostaddr);
   }
#endif // INCLUDE_PCCD

#ifdef INCLUDE_USB_SUPPORT
   /*********************************************************************
   *  Setup the access parameters for the USB-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == (unsigned)R15USB) {
      return vbtPageAccessSetupUSB(cardnum);
   }
#endif

#ifdef INCLUDE_VME_VXI_1553
   /*********************************************************************
   *  Setup the access parameters for the QVME-1553 and RQVME2-1553.
   *********************************************************************/
   if (CurrentCardType[cardnum] == QVME1553) {
      return vbtPageAccessSetupVME(cardnum, phys_addr, hostaddr);
   }
#endif

   return BTD_NON_SUPPORT;
}


/****************************************************************
*
*  PROCEDURE - BusTools_API_OpenChannel
*
*  FUNCTION
*     Initializes the channel on the specified device.
*
*  PARAMETERS
*     BT_UNIT  *chnd   - the card number representing the device/channel
*     BT_UINT  mode    - operational mode
*     BT_INT   devid   - device ID
*     BT_UINT  channel - channel number
*
*  RETURNS
*     BTD_OK
*
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_API_OpenChannel(BT_UINT *chnd, BT_UINT mode, BT_INT devid, BT_UINT channel) {
   BT_INT status, cardnum;

   //clear the assigned cardnum table on initial start
   if(assigned_cards[MAX_BTA] != 0xbeef)
   {
       assigned_cards[MAX_BTA] = 0xbeef;
       for(cardnum = 0;cardnum<MAX_BTA;cardnum++)
          assigned_cards[cardnum] = 0;
   }

   if((status = vbtOpen1553Channel(chnd,mode,devid,channel))!= 0)
      return status;

   return(API_Init(*chnd, devid, 0, &mode));
}


/****************************************************************
*
*  PROCEDURE - vbtOpen1553Channel
*
*  FUNCTION
*     This function open a 1553 Channel and return a handle to
*     the channel.
*
*  PARAMETERS
*     BT_UNIT  *chnd   - the card number representing the device/channel
*     BT_UINT  mode    - operational mode
*     BT_INT   devid   - device ID
*     BT_UINT  channel - channel number
*
*  RETURNS
*     BTD_OK
*     API_BAD_DEVICE_ID
*     BTD_CHAN_NOT_PRESENT
*     API_MAX_CHANNELS_INUSE
*
****************************************************************/
int vbtOpen1553Channel(UINT *chnd, UINT mode, int devid, UINT channel) {
   int status=0, cardnum=0;
   unsigned int deviceID=0, chan_cnt=0;

   if((status = vbtGetDevInfo(devid, "id", (unsigned long*)&deviceID)) != 0)
     return status; 
   if(deviceID <= 0)
     return API_BAD_DEVICE_ID;

   if((status = vbtGetDevInfo(devid, "ch", (unsigned long*)&chan_cnt)) != 0) 
     return status; 
   if((chan_cnt == 0) || (channel > chan_cnt-1))
      return BTD_CHAN_NOT_PRESENT; 

   for(cardnum=0;cardnum<MAX_BTA;cardnum++) {
     if(assigned_cards[cardnum] == 0) {
       assigned_cards[cardnum] = 0xff;
       break;
     }
   }
   if(cardnum == MAX_BTA)
     return API_MAX_CHANNELS_INUSE;

   //assign the cardnum
   *chnd = cardnum;

   CurrentPlatform[cardnum] = PLATFORM_PC;          // Platform is PC.
   CurrentMemMap[cardnum]   = CARRIER_MAP_DEFAULT;  // No carrier mapping
   CurrentCardSlot[cardnum] = channel;              // Specified channel
   CurrentCarrier[cardnum]  = NATIVE;               // No Carrier.
   CurrentCardType[cardnum] = deviceID;             // Set card type

   return BTD_OK;
}


/****************************************************************
*
*  PROCEDURE - BusTools_FindDevice
*
*  FUNCTION
*     This function returns the device number for the device and
*      instance.
*
*  PARAMETERS
*     BT_INT  device_type  - the device id to find
*     BT_INT  instance     - the device instance
*
*  RETURNS
*     minor/device number
*     -1 if none found. 
*     
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_FindDevice(BT_INT device_type, BT_INT instance) {
   int count, index, status=-1;
   unsigned int dev_num, dev_id;

   dev_num = dev_id = count = 0;
  
   if((instance < 0) || (instance > MAX_BTA-1))
     return -1;

   if((status = vbtGetDevInfo(-1, "devnum", (unsigned long*)&dev_num)) != 0)
     return status;
   if(dev_num < 1)
     return -1;

   for(index=0;index < dev_num;index++) {
     if((status = vbtGetDevInfo(index, "id", (unsigned long*)&dev_id)) != 0)
       return status;
     if(dev_id == device_type) {
       if(++count == instance)
         return index;
     }
   }

   return -1;
}


/****************************************************************
*
*  PROCEDURE - BusTools_ListDevices
*
*  FUNCTION
*     This function list the MIL-STD-1553 devices that are
*      detected on the PCI bus.
*
*  PARAMETERS
*     DeviceList* dlist
*
*  RETURNS
*     API_SUCCESSS 
*     
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_ListDevices(DeviceList* dlist) {
   int index, dev_cnt, status=-1;
   unsigned int dev_num, dev_id;
   char dev_name[256]={""};

   memset(dlist,0,sizeof(DeviceList));
   dev_id=dev_cnt=dev_num=0;

   if((status = vbtGetDevInfo(-1, "devnum", (unsigned int*)&dev_num)) != 0)
     return status;

   for(index=0;index<dev_num;index++) {
      if((status = vbtGetDevInfo(index, "id", (unsigned int*)&dev_id)) != 0)
        return status;
      switch(dev_id) {
      case PCCD1553:
        strcpy(dev_name,"PCC-D1553");
        break;
      case QPM1553:
        strcpy(dev_name,"QPM-1553 / QPMC-1553");
        break;
      case QPCI1553:
        strcpy(dev_name,"QPCI-1553"); 
        break;
      case QPCX1553:
        strcpy(dev_name,"QPCX-1553"); 
        break;
      case QCP1553:
        strcpy(dev_name,"QCP-1553"); 
        break;
      case Q1041553P:
        strcpy(dev_name,"Q104-1553P");
        break;
      case QVME1553:
        strcpy(dev_name,"QVME-1553");
        break;
      case R15EC:
        strcpy(dev_name,"R15-EC");
        break;
      case R15AMC:
        strcpy(dev_name,"R15-AMC");
        break;
      case RXMC1553:
        strcpy(dev_name,"RXMC-1553");
        break;
      case RPCIe1553:
        strcpy(dev_name,"RPCIe-1553");
        break;
      case R15XMC2:
        strcpy(dev_name,"RXMC2-1553");
        break;
      case LPCIE1553:
        strcpy(dev_name,"R15-LPCIe");
        break;
      case RAR15XMCXT:
        strcpy(dev_name,"RAR15-XMC-XT");
        break;
      case R15USB:
        strcpy(dev_name,"R15-USB");
        break;
      case MPCIE1553:
        strcpy(dev_name,"R15-MPCIe");
        break;
      default:
        strcpy(dev_name,"UNKNOWN");
      }
      strcpy((dlist->name[dev_cnt]),dev_name);
      dlist->device_name[dev_cnt] = dev_id;
      dlist->device_num[dev_cnt] = index;
      dev_cnt++;  
   }
   dlist->num_devices = dev_cnt; 

   return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - BusTools_GetDevInfo
*
*  FUNCTION
*     This function retrieves the device's information
*
*  PARAMETERS
*     BT_UNIT device        -  device id
*     DEVICE_INFO *dev_info -  the board specific information 
*
*  RETURNS
*     API_SUCESSS 
*     BTD_NO_HASH_ENTRY
*     
****************************************************************/
NOMANGLE BT_INT CCONV BusTools_GetDevInfo(BT_UINT device, DEVICE_INFO *dev_info) {
  BT_INT status=0;
  BT_UINT val=0;


  if((status = vbtGetDevInfo(device, "bus_type", &val)) == BTD_OK)
    dev_info->busType = val;
  else 
    dev_info->busType = -1;

  if((status = vbtGetDevInfo(device, "ch", &val)) == BTD_OK)
    dev_info->nchan = val; 
  else
    dev_info->nchan = -1;

  if((status = vbtGetDevInfo(device, "irig", &val)) == BTD_OK)
    dev_info->irig = val; 
  else
    dev_info->irig = -1; 

  if((status = vbtGetDevInfo(device, "mode", &val)) == BTD_OK)
    dev_info->mode = val; 
  else
    dev_info->mode = -1; 

  // if a R15-USB, then hardcode the USB Vendor ID and the Device ID
  if(dev_info->busType == BUS_USB) {
    dev_info->VendorID = VENDOR_ID_R15USB;
    dev_info->DeviceID = DEVICE_ID_R15USB;
  }
  else {
    if((status = vbtGetDevInfo(device, "vendor", &val)) == BTD_OK) 
      dev_info->VendorID = val;
    else
      dev_info->VendorID = 0;
    if((status = vbtGetDevInfo(device, "device", &val)) == BTD_OK) 
      dev_info->DeviceID = val;
    else
      dev_info->DeviceID = 0;
  }

  dev_info->memSections = 0;

  return API_SUCCESS;
}


/****************************************************************************
*
*  PROCEDURE NAME -    BusTools_SetDumpPath()
*
*  FUNCTION
*       This function creates a environment variable CONDOR_HOME used to set
*       the path to where the dumpfiles are output.
*
*  RETURNS
*       API_SUCCESS
*       API_BAD_PARAM - this return code indicate the path is invalid
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_SetDumpPath(char* dpath) {
  BT_INT status=0;
  char dump_path[512];
  struct stat buf;


  if(stat(dpath, &buf) == -1)
    return API_BAD_PARAM;
  if(S_ISDIR(buf.st_mode) == 0)
    return API_BAD_PARAM;
 
  if((status = CEI_SET_ENV_VAR("CONDOR_HOME", dpath)) != BTD_OK)
    return status;

  memset(dump_path, 0, sizeof(dump_path));
  // verify that the path was set
  if((status = CEI_GET_ENV_VAR("CONDOR_HOME", dump_path, sizeof(dump_path))) != BTD_OK)
    return status;

  if(strcmp(dpath, dump_path) != 0) 
    return API_BAD_PARAM;	 

  return API_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - get_64BitHostTimeTag
*
*  FUNCTION
*      Get the host clock to initialize the time tag register
*
*  PARAMETERS
*    BT_INT       mode  
*    BT1553_TIME* host_time
*
*  RETURNS
*     
****************************************************************/
void get_64BitHostTimeTag(BT_INT mode, BT1553_TIME *host_time) {
  time_t    now=0;         // Current time, modify to get Time Reference Point.
  struct tm start;         // Formatted "now" current time, modified for TRP.
  time_t    then=0;        // Time Reference Point, Midnight or Jan 1.
  struct timespec ts;
  union bigtime {
    CEI_UINT64 rtn_time;
    BT1553_TIME rtn_val;
  }bt;
  union{
    CEI_UINT64 diff;            // This is the difference time we return in microseconds,
    BT_U32BIT tt[2];
  }utime;                          //  either time since Midnight or time since Jan 1.

  switch (mode) {
  case API_TTI_DAY:  // Initial time relative to midnight
  case API_TTI_DAY64:
    time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
    start = *localtime(&now); // Local time in seconds, min, hour, day, month
    start.tm_sec  = 0;        // Local
    start.tm_min  = 0;        //       time
    start.tm_hour = 0;        //            at midnight
    then = mktime(&start);    // local midnight to time in seconds since Jan 1, 1970
    break;
  case API_TTI_IRIG: // Initial time relative to Jan 1, this year
  case API_TTI_IRIG64:
    time(&now);               // Time in seconds since midnight Jan 1, 1970 GMT
    start = *localtime(&now); // Local
    start.tm_sec  = 0;        //   time
    start.tm_min  = 0;        //     at
    start.tm_hour = 0;        //       Jan 1
    start.tm_mday = 0;        //         midnight (This is day 1)
    start.tm_mon  = 0;        //           this
    start.tm_yday = 0;        //             year
    then = mktime(&start);    // local Jan 1 to time in seconds since Jan 1, 1970
                                // We do IRIG time, which makes Jan 1 "day 1" */
    break;
  }

  if(clock_gettime(CLOCK_REALTIME, &ts) != 0) {
   #ifdef BT1553_LL_DEBUG
    printf(" <BT1553_LL_DEBUG> get_64BitHostTimeTag (clock_gettime): errno - %d\n", errno);
   #endif
    return;
  }

  // only want the time since Midnight or Jan 1
  utime.diff = ts.tv_sec - then;

  switch(mode) {
  case API_TTI_IRIG:
    utime.diff -= 86400;  // adjust for 1 day
  case API_TTI_DAY:
    // convert time to microseconds
    utime.diff = (utime.diff * 1000000) + (ts.tv_nsec/1000);
    break;
  case API_TTI_IRIG64:
    utime.diff -= 86400;  // adjust for 1 day
  case API_TTI_DAY64:
    // convert time to nanoseconds
    utime.diff = (utime.diff * 1000000000) + ts.tv_nsec;
    break;
  };

 #if defined(__powerpc__)
  bt.rtn_val.topuseconds  = utime.tt[0];
  bt.rtn_val.microseconds = utime.tt[1];
 #else
  bt.rtn_time = utime.diff;
 #endif
  
  *host_time = bt.rtn_val; 
}


/****************************************************************
*
*  PROCEDURE - CEI_GET_ENV_VAR
*
*  FUNCTION
*     Get the value for the specified environment variable
*
*  RETURNS
*     BTD_OK
*     API_NULL_PTR
*     API_BAD_PARAM
*
****************************************************************/
CEI_INT CEI_GET_ENV_VAR(CEI_CHAR* name, CEI_CHAR* buf, CEI_INT size) {
  CEI_CHAR* tmp_buf=NULL;

  if((name == NULL) || (buf == NULL)) 
    return API_NULL_PTR;
  if(size == 0) 
    return API_BAD_PARAM; 
  
  if((tmp_buf = getenv(name)) == NULL) {
  #ifdef BT1553_LL_DEBUG
    printf(" <BT1553_LL_DEBUG> getenv: error - cannot find env: %s\n", name);
   #endif
    return API_BAD_PARAM;
  }

  strncpy(buf, tmp_buf, size); 
   
  return BTD_OK;
}


/****************************************************************
*
*  PROCEDURE - CEI_SET_ENV_VAR
*
*  FUNCTION
*     Set the specified environment variable with the provide value  
*
*  RETURNS
*     BTD_OK
*     API_NULL_PTR
*     API_BAD_PARAM
*     BTD_ERR_NOMEMORY
*
****************************************************************/
CEI_INT CEI_SET_ENV_VAR(CEI_CHAR* name, CEI_CHAR* buf) {
  CEI_INT status=0;

  if((name == NULL) || (buf == NULL)) 
    return API_NULL_PTR; 

  if(setenv(name, buf, 1) == -1) {
    status = errno;
    if(status == ENOMEM) {
     #ifdef BT1553_LL_DEBUG
      printf(" <BT1553_LL_DEBUG> CEI_SET_ENV_VAR: error - %d\n", status);
     #endif
      return BTD_ERR_NOMEMORY;
    }
   #ifdef BT1553_LL_DEBUG
    printf(" <BT1553_LL_DEBUG> CEI_SET_ENV_VAR: error - %d, env: %s, val: %s\n", status, name, buf);
   #endif
    return API_BAD_PARAM;
  }

  return BTD_OK;
}


/****************************************************************
*
*  PROCEDURE - CEI_GET_FILE_PATH
*
*  FUNCTION
*     Checks that the file that is provided has a valid directory
*
*  RETURNS
*     BTD_OK
*     API_BAD_PARAM
****************************************************************/
CEI_INT CEI_GET_FILE_PATH(CEI_CHAR* name, CEI_CHAR* buf) {
  CEI_UINT status=0;
  CEI_CHAR tmp_buf[512]="";
  CEI_CHAR* tok="/";
  struct stat tmp_stat;

  if(name == NULL)
    return API_BAD_PARAM;

  memset(tmp_buf, 0, sizeof(tmp_buf));
  // check if the user supplied file name contains a path, if so verify  
  // that it is a directory
  if(strchr(name, tok[0]) != NULL) {
    strncpy(tmp_buf, name, strlen(name) - strlen(strrchr(name, tok[0]) + 1)); 
    status = stat(tmp_buf, &tmp_stat);
    if((status != 0) || (!(tmp_stat.st_mode & S_IFDIR))) {
     #ifdef BT1553_LL_DEBUG
      printf(" <BT1553_LL_DEBUG> CEI_GET_FILE_PATH: error - %d, env: %s, val: %s\n", status, tmp_buf, tmp_stat.st_mode);
     #endif
      return API_BAD_PARAM;
    }
    sprintf(buf, "%s", name);
  }
  else {
    // use the path from the "CONDOR_HOME" environment variable
    if(CEI_GET_ENV_VAR("CONDOR_HOME", tmp_buf, sizeof(tmp_buf)) == BTD_OK) {
      status = stat(tmp_buf, &tmp_stat);
      if((status != 0) || (!(tmp_stat.st_mode & S_IFDIR))) {
       #ifdef BT1553_LL_DEBUG
        printf(" <BT1553_LL_DEBUG> CEI_GET_FILE_PATH: error - %d, env: %s, val: %s\n", status, tmp_buf, tmp_stat.st_mode);
       #endif
        return API_BAD_PARAM;
      }
      sprintf(buf, "%s%s%s", tmp_buf, tok, name); 
    }
    else
      sprintf(buf, "%s", name);  // set path to local
  }

  return BTD_OK;
}
