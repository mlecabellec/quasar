/*============================================================================*
 * FILE:                        C O N F I G . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 1997 - 2018 BY ABACO SYSTEMS, INC.
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
 *===========================================================================*
 *
 * FUNCTION:   Internal lowlevel UNIX configuration functions.
 *
 *             This module will either read from the sysfs filesystem or read
 *             from the ceidev.conf file to determine the board attributes
 *             based on the board's minor number.  
 *
 * EXTERNAL Routines:
 *
 *    readSysfsInode          Reads the specified board's device attribute from
 *                            the sysfs filesystem (if mounted).
 *
 *    openCEIConfigData       Opens the ceidev.conf file that is located by the
 *                            function findConfFile.  This function creates a
 *                            hash table of the ceidev.conf data.
 *
 *    closeCEIConfigData      Destroys the hash table created by openCEIConfigData.
 *
 *
 * INTERNAL Routines:
 *
 *    getConfigDataInt        Returns integer data from the hash table. 
 *
 *    getConfigDataString     Returns character string data from the hash table.
 * 
 *    findSysfsInode          Determines the full directory path to the
 *                            specified board's device attribute in the sysfs
 *                            filesystem (if mounted).
 *
 *    findConfFile            Determines the full directory path to the
 *                            ceidev.conf file.
 *
 *===========================================================================*/

/* $Revision:  4.73 - Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  ----------   ---------------------------------------------------------------
  07/15/2005   Added findSysfsInode, readSysfsInode, and findConfFile
                functions.  modified openCEIConfigData.bch
  11/29/2005   modified findConfFile.bch
  02/20/2006   modified with common data types.bch
  04/14/2006   added getSysfsAttr, getDevRes_sysfs, and getDevRes_conf from 
                mem.c, renamed "unix_config.c" to "config.c".bch
  03/13/2007   modified getDevRes_conf.bch
  05/31/2007   modified readSysfsInode.bch
  06/01/2007   modified getDevRes_sysfs.bch
  10/03/2007   modified readSysfsInode, getDevRes_sysfs, and findSysfsInode.bch
  11/18/2008   modified openCEIConfigData.bch
  03/26/2009   replaced CEI_U32 with CEI_UINT32.bch
  10/20/2009   modified openCEIConfigData, findSysfsInode and getDevRes_conf.bch
  02/12/2010   modified getDevRes_conf.bch
  08/12/2011   modified getSysfsAttr. bch
  02/03/2017   modified findSysfsInode. bch
  01/09/2018   modified openCEIConfigData and closeCEIConfigData. bch
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "ll.h"


// globals
CEI_INT hash_table_status=0;

#ifndef NO_SYSFS_SUPPORT
CEI_INT getSysfsAttr(CEI_UINT device, CEI_CHAR* attr_name, CEI_VOID* pData) {
   CEI_INT i, status=0;
   CEI_CHAR attr_str[20]="";
   CEI_CHAR attr_buf[344]="";
   CEI_CHAR *pStart=NULL;
   CEI_CHAR *pEnd=NULL;
  #if defined(_32BIT)
   CEI_UINT64 *pBar_reg;
  #else
   CEI_ULONG *pBar_reg;
  #endif

   if(strcmp(attr_name, "resource") == 0) {
     sprintf(attr_buf, "%d", (int)sizeof(attr_buf));
     if((status = readSysfsInode(device, attr_name, attr_buf)) != 0)
       return status;
     pStart = strtok(attr_buf, " ");
     for(i=0;i<6;i++) {
       // BAR address
      #if defined(LP64) || defined(_32BIT)
       pBar_reg = (((CEI_UINT64*)pData) + (i*3));
       *pBar_reg = strtoull(pStart, &pEnd, 0);
       pStart = strtok(NULL, " ");
       // BAR size
       *(pBar_reg + 1) = ((strtoull(pStart, &pEnd, 0))+1) - *pBar_reg;
       pStart = strtok(NULL, "\n");
       // BAR type (IO/MEM)
       *(pBar_reg + 2) = (0x1 & strtoull(pStart, &pEnd, 0));
      #else
       pBar_reg = (((CEI_ULONG*)pData) + (i*3));
       *pBar_reg = strtoul(pStart, &pEnd, 0);
       pStart = strtok(NULL, " ");
       // BAR size
       *(pBar_reg + 1) = ((strtoul(pStart, &pEnd, 0))+1) - *pBar_reg;
       pStart = strtok(NULL, "\n");
       // BAR type (IO/MEM)
       *(pBar_reg + 2) = (0x1 & strtoul(pStart, &pEnd, 0));
      #endif
       pStart = strtok(NULL, "\n ");
       if(pStart == NULL)
         break;
     }
   }
   else if((strcmp(attr_name,"pci_id") == 0) ||
           (strcmp(attr_name,"board_name") == 0)) {
     sprintf(attr_str, "%d", (int)sizeof(attr_str));
     if((status = readSysfsInode(device, attr_name, attr_str)) != 0)
       return status;
     sscanf((CEI_CHAR*)pData,"%s",attr_str);
   }
   else {
     if((status = readSysfsInode(device, attr_name, attr_str)) != 0)
       return status;
     *((CEI_UINT32*)pData) = strtoul(attr_str, &pEnd, 0);
   }

   return 0;
}


CEI_INT getDevRes_sysfs(CEI_UINT device) {
   CEI_INT i, status=-1;
   CEI_ULONG bar_addr=0;
   CEI_ULONG bar_size=0;
   CEI_ULONG data=0;
   PDEVMAP_T _pMap = &cei_pMap[device];
  #if defined(_32BIT)
   CEI_UINT64 pci_bar_regions[6][3];  //  0 - BAR address, 1- BAR size, 2 - BAR type
  #else
   CEI_ULONG pci_bar_regions[6][3];  //  0 - BAR address, 1- BAR size, 2 - BAR type
  #endif

   // check for valid minor
   if((status = getSysfsAttr(_pMap->device, "minor", &data)) != 0) {
     printf("No board found for device (%d), status (%d)\n", _pMap->device, status);
     return status;
   }

   if((status = getSysfsAttr(_pMap->device, "id", &data)) != 0) {
     printf("No board id found for device (%d), status (%d)\n", _pMap->device, status);
     return status;
   }

   // program for PCMCIA boards
   if((data == 0xA0) || (data == 0x200)) {  // PCMCIA 
     _pMap->busType = BUS_PCMCIA;
     if(getSysfsAttr(_pMap->device, "manf_id", &data) == 0)
       _pMap->VendorID = data;
     if(getSysfsAttr(_pMap->device, "card_id", &data) == 0)
       _pMap->DeviceID = data;
     if((status = openDevice(device)) != 0)
       return status;
     if(ioctl(_pMap->hKernelDriver,GET_REGION_MEM,&data) == -1)
       return BTD_IOCTL_GET_REG;
     _pMap->memStartPhy[0] = data;
     if(ioctl(_pMap->hKernelDriver,GET_REGION_SIZE,&data) == -1)
       return BTD_IOCTL_REG_SIZE;
     _pMap->memLengthBytes[0] = data;
     _pMap->memSections++;
     _pMap->flagMapToHost[0] = 1;
     return BTD_OK;
   }

   // program for PCI boards
   _pMap->busType = BUS_PCI;

   if((status = getSysfsAttr(_pMap->device, "vendor", &data)) != 0) {
     printf("No Vendor ID found for device (%d), status (%d)\n", _pMap->device, status);
     return status;
   }
   else 
     _pMap->VendorID = data;

   if((status = getSysfsAttr(_pMap->device, "device", &data)) != 0) {
     printf("No Device ID found for device (%d), status (%d)\n", _pMap->device, status);
     return status;
   }
   else
     _pMap->DeviceID = data;

   memset(pci_bar_regions, 0, sizeof(pci_bar_regions));
   //  read the attributes:  BAR addresses and size
   if((status = getSysfsAttr(_pMap->device, "resource", pci_bar_regions)) != 0) {
     printf("No memory resources found for device (%d), status (%d)\n", _pMap->device, status);
     return status;
   }

   for(i=0;i<MAX_MEMORY;i++) {
     if(_pMap->flagMapToHost[i] == -1) { // do not map region
       _pMap->flagMapToHost[i] = 0;
       continue;
     }

     bar_addr = pci_bar_regions[i][0];
     if(bar_addr > 0) {
       bar_size = pci_bar_regions[i][1];
       if(bar_size == 0)
         return BTD_BAD_SIZE;
       if(bar_size < getpagesize()) 
         bar_size = getpagesize();
       if(pci_bar_regions[i][2] == 0) {  // if MEM region then map
         _pMap->memStartPhy[_pMap->memSections]    = bar_addr;
         _pMap->memLengthBytes[_pMap->memSections] = bar_size;
         _pMap->flagMapToHost[_pMap->memSections]  = 1;  // only map MEM not IO
         _pMap->memSections++;
       }
       else {
         if(_pMap->portSections == MAX_PORTS) {
           printf("Exceeded number of supported IO port regions.\n");
           return -1;
         }
         _pMap->portStart[_pMap->portSections]  = bar_addr;
         _pMap->portLength[_pMap->portSections] = bar_size;
         _pMap->portSections++;
       }
     }
   }

   return 0;
}

CEI_INT readSysfsInode(CEI_INT minor, CEI_CHAR* inode, CEI_CHAR* val) {
  CEI_INT status=0;
  CEI_INT fd_inode=0;
  CEI_CHAR path[200]="";
  CEI_CHAR inode_path[200]="";
  CEI_CHAR inode_val[100]="";
  CEI_INT val_size=10;

  // determine if PCMCIA or PCI driver is loaded
  sprintf(inode_path, "%s/devnum", CEI_PCC_SYSFS_DIR);
  if((fd_inode = open(inode_path, O_RDONLY)) != -1) 
      strcpy(path, CEI_PCC_SYSFS_DIR);
  else {
    sprintf(inode_path, "%s/devnum", CEI_PCI_SYSFS_DIR);
    if((fd_inode = open(inode_path, O_RDONLY)) == -1) {
      printf("Failed: no sysfs path for a Condor Engineering driver.  Check the driver.\n");
      return -1;//BTD_SYSFS_FAIL;
    }
    strcpy(path, CEI_PCI_SYSFS_DIR);
  }
  close(fd_inode);

  if(minor != -1) {
    sprintf(inode_val, "%d", minor);
    if((status = findSysfsInode("minor", inode_val, path)) != 1) {
      if(status == 0) {
        status = -1;
        printf("Failed: \"minor\" inode not present in device sysfs path or invalid \"minor\" number (%d).\n", minor);
      }
      return status;
    }
    // do not continue since already checked for the inode "minor"
    if(strcmp(inode, "minor") == 0)
      return 0;
  }

  // retrieves PCI id (bus, device) 
  if(strcmp(inode, "pci id") == 0) {
    strcpy(val, strtok(path, CEI_PCI_SYSFS_DIR));
    return BTD_OK;
  }

  // find inode path 
  if((status = findSysfsInode(inode, val, path)) != 1) {
    if(status == 0) {
      status = -1;
      printf("Failed: inode \"%s\" not present in driver sysfs path.\n", inode);
    }
    return status;
  }

  // read inode  
  snprintf(inode_path, sizeof(inode_path), "%s/%s", path, inode);
  if((fd_inode = open(inode_path, O_RDONLY)) == -1) {
    printf("Failed: open for inode(%s), errno: %d\n", inode, errno);
    return -1;  //BTD_SYSFS_ATTR_FAIL;
  }
  if(atoi(val) != 0)
    val_size = atoi(val);  // gets size of string buffer
  if((read(fd_inode, val, val_size)) < 0) {
    close(fd_inode);
    printf("Failed: read for inode(%s), errno: %d\n", inode, errno);
    return -1;  //BTD_SYSFS_ATTR_FAIL;
  }
  close(fd_inode);

  return 0;
} 


CEI_INT findSysfsInode(CEI_CHAR* inode, CEI_CHAR* val, CEI_CHAR* path) {
  CEI_INT status=0;
  DIR* pDir=NULL;
  struct dirent* dir_ent;
  CEI_CHAR inode_val[100]="";
  CEI_CHAR inode_path[300]="";
  CEI_INT fd_inode=0;

  if((pDir = opendir(path)) == NULL) {
    if(errno == ENOTDIR)
      return 0;  // the path is not a directory
    printf("Failed: opendir (%s), errno: %d\n", path, errno);
    return -1; //BTD_SYSFS_FAIL;
  }

  while((dir_ent = readdir(pDir)) != NULL) {
    // disregarded directories
    if((strcmp(dir_ent->d_name,".") == 0) || (strcmp(dir_ent->d_name,"..") == 0) ||
       (strcmp(dir_ent->d_name,"bus") == 0) || (strcmp(dir_ent->d_name,"driver") == 0) ||
       (strcmp(dir_ent->d_name,"module") == 0) || (strcmp(dir_ent->d_name,"power") == 0) ||
       (strcmp(dir_ent->d_name,"subsystem") == 0) || (strcmp(dir_ent->d_name,"firmware_node") == 0) ||
       (strncmp(dir_ent->d_name,"iommu",5) == 0))
      continue;
    if(strcmp(dir_ent->d_name, inode) == 0) {
      closedir(pDir); 
      if(strcmp(inode, "minor") == 0) {  // check for minor number
        sprintf(inode_path, "%s/%s", path, inode);
        if((fd_inode = open(inode_path, O_RDONLY)) == -1) {
          printf("Failed:  open for inode(%s), errno: %d\n", inode, errno);
          return -1; //BTD_SYSFS_FAIL;
        }
        if((read(fd_inode, inode_val, 20)) < 0) {
          close(fd_inode);
          printf("Failed:  read for inode(%s), errno: %d\n", inode, errno);
          return -1; //BTD_SYSFS_ATTR_FAIL;
        } 
        close(fd_inode);
        if(atoi(val) == atoi(inode_val)) 
          return 1;
        return 0;
      }
      return 1;
    }

    sprintf(inode_path, "%s/%s", path, dir_ent->d_name); 
    if((status = findSysfsInode(inode, val, inode_path)) != 0) 
      break;
  }
  strcpy(path, inode_path);
  closedir(pDir);

  return status; 
}
#else  // NO_SYSFS_SUPPORT

#ifdef LL_DEBUG 
 #define DEBUG_OUTPUT printf(" <LL_DEBUG> error in getDevRes_conf - status: %d\n", status);
#else
 #define DEBUG_OUTPUT
#endif
#define STATUS_CHK(status) {  \
  if(status != BTD_OK) {      \
    DEBUG_OUTPUT              \
    closeDevice(device);      \
    return status;            \
  }                           \
}

// read configuration information from ceidev.conf file
CEI_INT getDevRes_conf(CEI_UINT device) {
   CEI_INT i, status=-1;
   CEI_INT fd=-1;
   CEI_INT board_regions=0;
   CEI_ULONG membase=0;
   CEI_ULONG size=0;
   CEI_UINT32 data=0;
   PDEVMAP_T _pMap = &cei_pMap[device];
  #ifndef KERNEL_24
   #ifdef _32BIT
   // need if running a 32-bit app on a 64-bit OS
   unsigned long long pci_bar_regions[6][3];
   #else
   unsigned long pci_bar_regions[6][3];
   #endif
  #endif  

   if((status = vbtGetDevInfo(-1, "devnum", &data)) != 0)
     return status;
   if(data <= _pMap->device)  
     return BTD_BAD_DEVICE_ID; //BTD_BAD_DEVICE_NUM;

   if((status = vbtGetDevInfo(device, "minor", &data)) != 0)
     return status;
   _pMap->device = (CEI_INT)data;  

   if((status = vbtGetDevInfo(device, "bus", &data)) != 0)
     return status;
   _pMap->busType = (CEI_INT)data;

   _pMap->VendorID = CEI_VENDOR_ID;

   if((status = vbtGetDevInfo(device, "id", &data)) != 0)
     return status;
   _pMap->DeviceID = data;

   if((status = openDevice(device)) != 0)
     return status;

   fd = _pMap->hKernelDriver;

   switch(_pMap->busType) {
   case BUS_PCI:
     board_regions = MAX_MEMORY;
    #ifndef KERNEL_24
     memset(&pci_bar_regions, 0, sizeof(pci_bar_regions));
     pci_bar_regions[0][0] = (0x2<<30) + 1;  // read device attribute
     if(read(fd, &pci_bar_regions, sizeof(pci_bar_regions)) == -1) {
       printf("vbtMapBoardAddresses:  failed to read for device %d, errno: %d\n", device, errno);
       STATUS_CHK(-1);  //BTD_DRV_READ_FAIL);
     }
    #endif  
     break;
   case BUS_ISA:
   case BUS_PCMCIA: 
     board_regions = 1;
     break;
   default: 
     printf("vbtMapBoardAddresses: unsupported Bus Type = %d\n",_pMap->busType );
     STATUS_CHK(BTD_UNKNOWN_BUS);
   };

   // Extract all of the memory region addresses and sizes.
   for(i=0;i<board_regions;i++) {
     if(_pMap->flagMapToHost[i] == -1) { // do not map region
       _pMap->flagMapToHost[i] = 0;
       continue;
     }
    #ifndef KERNEL_24
     switch(_pMap->busType ) {
     case BUS_PCI:
       membase = pci_bar_regions[i][0];
       size = pci_bar_regions[i][1];
       break;
     case BUS_ISA:
     case BUS_PCMCIA:
       if(ioctl(fd,SET_REGION,&i) == -1) 
         STATUS_CHK(BTD_IOCTL_SET_REG);
       if(ioctl(fd,GET_REGION_MEM,&membase) == -1) 
         STATUS_CHK(BTD_IOCTL_GET_REG);
       if(ioctl(fd,GET_REGION_SIZE,&size) == -1) 
         STATUS_CHK(BTD_IOCTL_REG_SIZE);
     };
    #else
     if(ioctl(fd,SET_REGION,&i) == -1)
       STATUS_CHK(BTD_IOCTL_SET_REG);
     if(ioctl(fd,GET_REGION_MEM,&membase) == -1)
       STATUS_CHK(BTD_IOCTL_GET_REG);
     if(ioctl(fd,GET_REGION_SIZE,&size) == -1)
       STATUS_CHK(BTD_IOCTL_REG_SIZE);
    #endif
     if(membase == 0)
       continue;
     if(membase == 0xFFFFFFFF)
       STATUS_CHK(BTD_ERR_BADADDR);
     if(size == 0 || size == 0xFFFFFFFF)
       STATUS_CHK(BTD_BAD_SIZE);
     if(size < 0x1000)
       size = 0x1000;
     _pMap->memStartPhy[_pMap->memSections]    = membase;
     _pMap->memLengthBytes[_pMap->memSections] = size;
     _pMap->flagMapToHost[_pMap->memSections]  = 1;
     _pMap->memSections++;
   }

   return BTD_OK;
}


CEI_CHAR ht_key[40][20];
CEI_CHAR ht_val[40][20];
CEI_INT openCEIConfigData(CEI_VOID) {
   CEI_INT status=BTD_OK;
   FILE* fp;
   CEI_CHAR path_conf[256]="";
   CEI_CHAR *sVal, *sKey;
   ENTRY entry, *ePtr;
   CEI_INT count=0;
   CEI_CHAR buf[80]="";

   if(hash_table_status)
     return BTD_OK; 

   // find the "ceidev.conf" file
   if(findConfFile(path_conf) != 0)
     return BTD_BAD_CONF_FILE;
   strcat(path_conf, "/ceidev.conf");

   // open the "ceidev.conf" file
   if((fp = fopen(path_conf,"r")) == NULL) {
     printf("Failed to open %s.  Errno: %d\n", path_conf, errno); 
     return BTD_BAD_CONF_FILE;
   }

   hdestroy();  // destroy hash tables if existing
   if(hcreate(40) == 0) 
     status = BTD_HASH_ERR;
   else {
     memset(ht_key, 0, sizeof(ht_key));
     memset(ht_val, 0, sizeof(ht_val));

     while((fgets(buf,80,fp)) != NULL) {
       if(buf[0] != '#') {
         sKey = strtok(buf,"=");
         if((sVal = strtok(NULL,"=")) != NULL) {
           removeWhiteSpace(sKey, (char*)(&ht_key[count]));
           entry.key = (char*)ht_key[count];
           removeWhiteSpace(sVal, (char*)(&ht_val[count]));
           entry.data = (char*)ht_val[count];
           count++;
           if((ePtr = hsearch(entry,ENTER)) == NULL) {
             status = BTD_NO_HASH_ENTRY;
             break;
           }
         }
       }
       memset(buf,0,sizeof(buf));
     }
   }

   hash_table_status = 1;

   fclose(fp);

   return status;
}


CEI_VOID closeCEIConfigData(CEI_VOID) {
   if(hash_table_status) {
     hdestroy();
     hash_table_status = 0;
   }
}


CEI_INT getConfigDataInt(CEI_CHAR* data) {
   ENTRY entry, *ePtr;

   entry.key = data;
   ePtr = hsearch(entry,FIND);
   if(ePtr == NULL)
     return BTD_NO_HASH_ENTRY;
   return atoi(ePtr->data);
}


CEI_CHAR* getConfigDataString(CEI_CHAR* key) {
   ENTRY entry, *ePtr;

   entry.key = key;
   ePtr = hsearch(entry,FIND);
   if(ePtr == NULL)
     return NULL;
   return ePtr->data;
}


CEI_VOID removeWhiteSpace(CEI_CHAR* buf, CEI_CHAR* retbuf) {
   CEI_INT i=0;
   CEI_INT count=0;

   while(buf[i] != '\n') {
     if(!(isspace(buf[i])))
       retbuf[count++] = buf[i++];
   }
   retbuf[count] = '\0';
}


CEI_INT findConfFile(CEI_CHAR* path) {
   CEI_INT i;
   CEI_CHAR* val;
   CEI_CHAR path_dir[200]="";
   CEI_CHAR buf[200]="";
   DIR* pDir=NULL;
   struct dirent* dir_ent;
   CEI_INT cnt_dir=0;
   CEI_INT cnt_conf=0;

   // will determine path based on presidence
   for(i=0;i<3;i++) {
     switch(i) {
     case 0:  // check the local directory path
       if(getcwd(path_dir, sizeof(path_dir)) == NULL) 
         continue;
       break;
     case 1:  // check "Condor Engineering" in the local path
       if(getcwd(buf, sizeof(buf)) == NULL) 
         continue;
       if(strcmp(buf, "/root/Condor_Engineering") == 0)
         continue;
       memset(path_dir, 0, sizeof(path_dir));
       val = strtok(buf, "/");
       while(val != NULL) {
         sprintf(path_dir, "%s/%s", path_dir, val);
         if(strcmp(val, "Condor_Engineering") == 0) 
           break;
         val = strtok(NULL, "/");
       }
       if(val == NULL)
         continue;
       getcwd(buf, sizeof(buf));
       if(strcmp(buf, path_dir) == 0)
         continue;
       break;
     case 2:  // check "Condor Engineering" in the "root" path
       strcpy(path_dir, "/root/Condor_Engineering");
       break;
     // add other search criteria here ...
     };

     if((pDir = opendir(path_dir)) == NULL) 
       continue;
     cnt_dir++;
     while((dir_ent = readdir(pDir)) != NULL) {
       if(strcmp(dir_ent->d_name, "ceidev.conf") == 0) {
         if(cnt_conf++ == 0)
           strcpy(path, path_dir);
        #ifdef LL_DEBUG
         printf(" <LL_DEBUG> Located %s/ceidev.conf\n", path_dir);
        #endif
         break;
       }
     }
     closedir(pDir);
   }

   if(cnt_dir == 0) {
     printf("Failed to get a directory path for the \"ceidev.conf\" file.\n"); 
     return -1;
   }
   if(cnt_conf == 0) {
     printf("Cannot locate a \"ceidev.conf\" file.  Check the installation.\n");
     return -1;
   }
  #ifdef LL_DEBUG   
   else if(cnt_conf > 1)
     printf(" <LL_DEBUG>  Detected %d \"ceidev.conf\" files.  Using: %s/ceidev.conf\n", cnt_conf, path);
  #endif

   return 0;
}
#endif
