/*============================================================================*
 * FILE:                       M E M . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 1994 - 2018 BY ABACO SYSTEMS, INC.
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

/* $Revision:  1.15 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  04/20/2005   Initial
  07/15/2005   removed the use of Libsysfs for accesses to the sysfs filesystem.
               modified the calls to the device driver's "read" for the kernel
                2.6 PCI device driver. bch
  01/19/2006   modified for 64-bit support. bch
  02/20/2006   modified with common data types. bch
  04/14/2006   moved getSysfsAttr, getDevRes_sysfs, and getDevRes_conf to 
                config.c, renamed "mem_unix.c" to "mem.c". bch
  05/25/2006   added SET_SIGNAL and SET_SIGVAL to ioctls, added
                vbtConfigInterrupt. bch
  05/30/2007   added vbtInterruptMode and vbtWaitForInterrupt. modified
                vbtConfigInterrupt and vbtGetInterrupt. modified
                vbtMapBoardAddresses and vbtFreeBoardAddresses to handle
                multiple instances. bch
  09/12/2007   modified vbtMapBoardAddresses and vbtFreeBoardAddresses. bch
  11/18/2008   modified vbtMapBoardAddresses, vbtGetPCIConfigRegister, and
                vbtGetDevInfo. bch
  02/27/2009   modified vbtWaitForInterrupt. bch
  03/26/2009   replaced CEI_U8 with CEI_UCHAR. replaced CEI_U16 with
                CEI_UINT16. replaced CEI_U32 with CEI_UINT32. bch
  10/20/2009   modified vbtGetDevInfo. bch
  10/11/2011   added vbtInterruptWait. modified vbtMapBoardAddresses and 
                vbtFreeBoardAddresses. bch
  11/19/2013   modified vbtMapBoardAddresses. bch
  10/31/2014   added vbtGetLibInfo. bch
  02/22/2018   modified vbtMapBoardAddress and vbtFreeBoardAddress. bch
*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h> 
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "ll.h"


// GLOBALS
CEI_MUTEX mtx_pmap;

// external functions
CEI_INT vbtMapBoardAddresses(CEI_UINT device, PDEVMAP_T pMap) {
   CEI_INT i, j, fd, status=-1;
   off_t offset=0;
   CEI_ULONG membase=0;
   PDEVMAP_T _pMap = &cei_pMap[device];
  #ifdef KERNEL_24
   CEI_ULONG val=0;
  #endif

   // if mutex does not exist, create it
   if((status = CEI_MUTEX_LOCK(&mtx_pmap)) != 0) {
     if(status == EINVAL) {
       if((status = CEI_MUTEX_CREATE(&mtx_pmap)) != 0) {
         printf("vbtMapBoardAddresses:  failed CEI_MUTEX_CREATE for device %d, status(%d)\n", device, status);
         return -1;
       }
       status = CEI_MUTEX_LOCK(&mtx_pmap);
     }
     if(status != 0) {
       printf("vbtMapBoardAddresses:  failed CEI_MUTEX_LOCK for device %d, status(%d)\n", device, status);
       return -1;
     }
   } 

   // prevents re-initializing the device with the same or different user's DEVMAP_T, copy to the user's DEVMAP_T
   if(_pMap->use_count > 0) {
     _pMap->use_count++;
     memcpy(pMap, _pMap, sizeof(DEVMAP_T));
     CEI_MUTEX_UNLOCK(&mtx_pmap);
     return BTD_OK;
   }

   _pMap->hKernelDriver = -1;
   _pMap->llDriverVersion = (LSP_MAJOR_VERSION << 8) || LSP_MINOR_VERSION;

  #ifndef NO_SYSFS_SUPPORT  
   _pMap->device = device;
   if((status = getDevRes_sysfs(device)) != 0) {
     CEI_MUTEX_UNLOCK(&mtx_pmap);
     return status;
   }
   status = openDevice(device);
  #else
   status = getDevRes_conf(device);
  #endif
   if(status != 0) {
     CEI_MUTEX_UNLOCK(&mtx_pmap);
     return status;
   }

   fd = _pMap->hKernelDriver;

   if(_pMap->memSections == 0) {
     CEI_MUTEX_UNLOCK(&mtx_pmap);
     return BTD_NO_REGIONS_TO_MAP;
   }

   for(j=0,i=0;i<_pMap->memSections;i++) {
     if(status != 0)
       break;	     
     if(_pMap->flagMapToHost[i] == 0)
       continue;
     if(_pMap->memLengthBytes[i] == 0) {
       printf("vbtMapBoardAddresses:  invalid memory size for region %d for device %d\n", i, device);
       _pMap->flagMapToHost[i] = 0;
       status = BTD_ERR_BADADDRMAP;
       continue;
     }
    #ifdef KERNEL_24	
     // set PCI region in kernel 2.4 driver
     if(_pMap->busType != BUS_PCMCIA) {
       for(;j<MAX_MEMORY;j++) {
         if(ioctl(fd,SET_REGION,&j) == -1) {
           status = BTD_IOCTL_SET_REG;
           break;
         }
         if(ioctl(fd,GET_REGION_MEM,&val) == -1) {
           status = BTD_IOCTL_GET_REG;
	   break;
         }
         if(val > 0) {
	   j++;
           break;
	 }
       }
       if(status != 0)
         break;
     }
    #endif
     membase = (CEI_ULONG)mmap(NULL,(size_t)_pMap->memLengthBytes[i],PROT_READ|PROT_WRITE,MAP_SHARED,fd,offset);
     if((CEI_CHAR*)membase == MAP_FAILED) {
       printf("vbtMapBoardAddresses:  failed to map region %d for device %d, errno(%d)\n", i, device, errno);
       _pMap->flagMapToHost[i] = 0;
       status = BTD_ERR_BADADDRMAP;
     } 
     _pMap->memHostBase[i] = (CEI_CHAR*)(membase | (_pMap->memStartPhy[i] & (getpagesize()-1)));
   }

   // if an error occurs unmap all memory regions and close the device
   if(status != 0) {
     for(j=0;j<_pMap->memSections;j++) {
       if((_pMap->flagMapToHost[j] == 1) && (_pMap->memHostBase[j] != NULL)) {
         membase = (CEI_ULONG)_pMap->memHostBase[j] & ~(getpagesize()-1);
         if(munmap((CEI_CHAR*)membase, _pMap->memLengthBytes[j]) != 0)
           printf("vbtFreeBoardAddresses:  failed to unmap region %d for device %d, errno(%d)\n", j, device, errno);
       }
     }
     closeDevice(device);
     CEI_MUTEX_UNLOCK(&mtx_pmap);
     return status;
   }

   _pMap->use_count = 1;
   memcpy(pMap, _pMap, sizeof(DEVMAP_T));
   CEI_MUTEX_UNLOCK(&mtx_pmap);

   return BTD_OK;
}


CEI_VOID vbtFreeBoardAddresses(PDEVMAP_T pMap) {
  CEI_INT i, device = pMap->device;
  PDEVMAP_T _pMap = &cei_pMap[device];
  CEI_ULONG membase=0;

  CEI_MUTEX_LOCK(&mtx_pmap);

  if(_pMap->use_count == 0) {
    CEI_MUTEX_UNLOCK(&mtx_pmap);
    return;
  }
  if(--_pMap->use_count != 0) {
    CEI_MUTEX_UNLOCK(&mtx_pmap);
    return;
  }

  for(i=0;i<_pMap->memSections;i++) {
    if((_pMap->flagMapToHost[i] == 1) && (_pMap->memHostBase[i] != NULL)) {
      membase = (CEI_ULONG)_pMap->memHostBase[i] & ~(getpagesize()-1);
      if(munmap((CEI_CHAR*)membase, _pMap->memLengthBytes[i]) != 0)
        printf("vbtFreeBoardAddresses:  failed to unmap region %d for device %d, errno(%d)\n", i, device, errno);
    }
  }

  if(closeDevice(_pMap->device) != 0) 
    printf("vbtFreeBoardAddresses:  failed to close device %d (%d).\n", device, _pMap->hKernelDriver);

 #ifdef NO_SYSFS_SUPPORT
  closeCEIConfigData();
 #endif

  memset(_pMap, 0, sizeof(DEVMAP_T));
  memset(pMap, 0, sizeof(DEVMAP_T));
  CEI_MUTEX_UNLOCK(&mtx_pmap);
  CEI_MUTEX_DESTROY(&mtx_pmap);
}


CEI_INT vbtGetPCIConfigRegister(CEI_UINT device, CEI_UINT offset, CEI_UINT length, CEI_VOID* value) {
   CEI_INT fd;
   CEI_UINT32 data=0;

   if((fd = cei_pMap[device].hKernelDriver) == -1)
     return BTD_NO_DRV_MOD;

  #ifndef KERNEL_24 
   data = (0x1<<30) + offset;  // read PCI config space
   if(read(fd, &data, length) == -1) {
     printf("vbtGetPCIConfigRegister:  failed to read device %d, errno(%d)\n", device, errno);
     return -1; //BTD_DRV_READ_FAIL;
   }
  #else
   data = offset;
   switch(length) {
   case 1:
     if(ioctl(fd, GET_PCIREGION_BYTE, &data) == -1)
       return -1;//BTD_DRV_IOCTL_FAIL + 1000;
     break;
   case 4:
     if(ioctl(fd, GET_PCIREGION_DWORD, &data) == -1)
       return -1;//BTD_DRV_IOCTL_FAIL + 4000;
     break;
   case 2:
   default:
     if(ioctl(fd, GET_PCIREGION_WORD, &data) == -1)
       return -1;//BTD_DRV_IOCTL_FAIL + 2000;
     break;
    };
  #endif

   if(length == 1)
     *((CEI_UCHAR*)value) = (CEI_UCHAR)data;
   else if(length == 4)
     *((CEI_UINT32*)value) = (CEI_UINT32)data;
   else
     *((CEI_UINT16*)value) = (CEI_UINT16)data;

   return BTD_OK;
}


//CEI_INT vbtGetInterrupt(CEI_UINT device, CEI_INT* irq) {
CEI_INT vbtGetInterrupt(CEI_UINT device) {
  CEI_INT fd, val=0;

  if((fd = cei_pMap[device].hKernelDriver) == -1)
    return 0;//BTD_NO_DRV_MOD;

 #ifndef KERNEL_24
  #ifndef NO_SYSFS_SUPPORT
   CEI_INT status=-1;
   if((status = getSysfsAttr(device, "irq", &val)) != 0) {
     printf("vbtGetInterrupt:  getSysfsAttr failed for device %d, status(%d)\n", device, status);
//     return status;
     return 0;
   }
  #else 
   val = (0x2<<30) + 2;  // read device attribute
   if(read(fd, &val, 4) == -1) {
     printf("vbtGetInterrupt:  failed to read device %d, errno(%d)\n", device, errno);
//     return BTD_DRV_READ_FAIL;	
     return 0;
   }
  #endif
 #else  // kernel 2.4
   if(ioctl(fd, GET_IRQ, &val) == -1) {
     printf("vbtGetInterrupt:  failed ioctl for device %d, errno(%d)\n", device, errno);
     return 0;
   }
 #endif

  return val;
}

	
CEI_INT vbtInterruptWait(CEI_UINT device, CEI_INT mode, CEI_INT val) {
   CEI_INT fd=-1;
   CEI_INT status=0;

   if(device >= MAX_DEVICES)
     return BTD_NO_DRV_MOD;

   if((fd = cei_pMap[device].hKernelDriver) == -1)
     return BTD_NO_DRV_MOD;

  #ifdef KERNEL_24
   if(mode == 1)
     status = ioctl(fd, SET_WAIT_QUEUE, &val);  // block
   else
     return -1;
  #else
   if(mode == 0)
     status = write(fd, &val, 5); // this will close the specified wait queue
   else if(mode == 1) {
     CEI_INT tmp = (0x3<<30) + val;
     status = read(fd, &tmp, 4);  // block
   }
   else
     return -1;
  #endif
   if(status < 0) {
     printf("vbtInterruptWait:  error detected, val(%d/%d), status(%d), errno(%d)\n", val, mode, status, errno);
     return -1;
   }
  
   return BTD_OK;
}


CEI_INT vbtWaitForInterrupt(CEI_UINT device) {
  return vbtInterruptWait(device, 1, 0);
}


// set to 1 to enable hardware interrupts, set to 0 to disable
CEI_INT vbtInterruptMode(CEI_UINT device, CEI_INT mode) {
   CEI_INT status, fd=-1;

   if((fd = cei_pMap[device].hKernelDriver) == -1)
     return BTD_NO_DRV_MOD;

  #ifdef KERNEL_24
   status = ioctl(fd, SET_INTERRUPT_MODE, &mode);
  #else
   status = write(fd, &mode, 4);
  #endif
   if(status != 0) {
     printf("vbtInterruptMode:  failed to set hardware interrupt mode %d, errno(%d)\n", mode, errno);
     return -1;  // BTD_DRV_WRITE_FAIL;
   } 
       	
   return BTD_OK;
}


CEI_INT vbtConfigInterrupt(CEI_UINT device, CEI_INT signal, CEI_INT pid, CEI_INT val) {
  CEI_INT fd=-1;
 #ifndef KERNEL_24
  CEI_INT status=0;
 #endif

  if((fd = cei_pMap[device].hKernelDriver) == -1)
    return BTD_NO_DRV_MOD;

 #ifndef KERNEL_24
  // set the PID to be signaled, if 0 then disable PID
  if((status = write(fd, &pid, 1)) != 0)
    printf("vbtConfigInterrupt:  failed to set PID %d, errno(%d)\n", pid, errno);
  // set the signal number, set to 0 to disable signals
  if((status = write(fd, &signal, 2)) != 0) 
    printf("vbtConfigInterrupt:  failed to signal %d, errno(%d)\n", signal, errno);
  // set the value of the signal in the ID, if 0 then use default value
  if((status = write(fd, &val, 3)) != 0) 
    printf("vbtConfigInterrupt:  failed to set the signal value %d, errno(%d)\n", val, errno);
  if(status != 0)
    return -1; // BTD_DRV_WRITE_FAIL;
 #else
  pid = getpid();
  if(ioctl(fd, SET_PID, &pid) == -1)
    return -1;//BTD_DRV_IOCTL_FAIL;
  if(ioctl(fd, SET_SIGNAL, &signal) == -1)
    return -1;//BTD_DRV_IOCTL_FAIL;
  if(ioctl(fd, SET_SIGVAL, &val) == -1)
    return -1;//BTD_DRV_IOCTL_FAIL;
 #endif

  return BTD_OK;
}


CEI_INT vbtGetDevInfo(CEI_UINT device, CEI_CHAR* devInfo, CEI_VOID* pData) {
   CEI_INT status=-1;
   CEI_UINT32 val=0;

  #ifndef NO_SYSFS_SUPPORT
   if((status = getSysfsAttr(device, devInfo, &val)) != 0)
     return status;
  #else
   CEI_CHAR hstrng[20]="";

   // the "ceidev.conf" file has different names then used for "SYSFS", and
   // doesn't have vendor ID or device ID
   if(strcmp(devInfo, "mode") == 0)
     sprintf(hstrng,"func");
   else if(strcmp(devInfo, "bus_type") == 0)
     sprintf(hstrng,"bus");
   else if(strcmp(devInfo, "vendor") == 0) {
     *((CEI_UINT32*)pData)  = 0x13C6;  
     return BTD_OK;
   }
   else
     sprintf(hstrng,"%s",devInfo);
   if(device != -1) 
     sprintf(hstrng,"%s%i",hstrng, device);

   if((status = openCEIConfigData()) != BTD_OK)
     return status;

   val = getConfigDataInt(hstrng);
//   closeCEIConfigData();
   if(val == BTD_NO_HASH_ENTRY)
     return BTD_NO_HASH_ENTRY;
  #endif	

   *((CEI_UINT32*)pData) = val;

   return BTD_OK;
}


CEI_INT vbtGetLibInfo(CEI_CHAR* ver) {
 #ifdef LL_USB
  CEI_CHAR _ver[50];

  memset(_ver, 0, sizeof(_ver));
  if(usb_ll_info(_ver) != 0)
    return -1;

  sprintf(ver, "%d.%d::%s", LSP_MAJOR_VERSION, LSP_MINOR_VERSION, _ver);
 #else
  sprintf(ver, "%d.%d", LSP_MAJOR_VERSION, LSP_MINOR_VERSION);
 #endif

  return BTD_OK;
}


// internal functions
CEI_INT openDevice(CEI_UINT device) {
   CEI_CHAR dev_path[16]="";
   CEI_INT fd=-1;
   PDEVMAP_T _pMap = &cei_pMap[device];

   if(_pMap->hKernelDriver != -1) 
     return BTD_OK;

   switch(_pMap->busType) {
   case BUS_PCI:
     sprintf(dev_path, "/dev/uceipci_%d", _pMap->device);
     break;
   case BUS_ISA: 
     sprintf(dev_path, "/dev/uceiisa_%d", _pMap->device);
     break;
   case BUS_PCMCIA: 
     sprintf(dev_path, "/dev/pcc1553_%d", _pMap->device);
     break;
   default: 
     printf("openDevice: unsupported Bus Type = %d\n", _pMap->busType);
     return BTD_UNKNOWN_BUS;
   };

// O_DIRECT may need to have "_GNU_SOURCE" defined
   if((fd = open(dev_path, O_RDWR)) == -1) {
     printf("openDevice:  failed to open device %d (%s), errno(%d)\n", device, dev_path, errno);
     return BTD_ERR_BADOPEN;
   }

   _pMap->hKernelDriver = fd;

   return BTD_OK;
}


CEI_INT closeDevice(CEI_UINT device) {
   PDEVMAP_T _pMap = &cei_pMap[device]; 

   if(_pMap->hKernelDriver == -1) {
     printf("closeDevice:  device %d already closed\n", _pMap->device);
     return BTD_NO_DRV_MOD;
   }

   if(close(_pMap->hKernelDriver) == -1) {
     printf("closeDevice:  failed to close device %d, errno(%d)\n", _pMap->device, errno);
     return -1;
   }

   return BTD_OK;
}


/*  These are the wrapper functions for VME boards in 32-bit environments, that
 *  will need to be development with the VME interface from the SBC's BSP or
 *  distribution's VME library.
 */
CEI_INT vbtMapBoardAddress(CEI_UINT base_address, CEI_UINT length, CEI_CHAR** addr, CEI_VOID* cardnum, CEI_INT addressing_mode) {
   return BTD_NO_PLATFORM;
}

CEI_VOID vbtFreeBoardAddress(CEI_UINT cardnum) {
}
