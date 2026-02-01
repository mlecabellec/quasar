/*============================================================================*
 * FILE:                         L L . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 2014 BY ABACO SYSTEMS, INC.
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

/* $Revision:  1.00 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/31/2014   Initial
 */

#ifndef _LL_H_
#define _LL_H_

#include "lowlevel.h"
#include "ll_posix.h"


#define LSP_MAJOR_VERSION  1
#define LSP_MINOR_VERSION  36

#define CEI_VENDOR_ID 0x13C6

#ifndef NO_SYSFS_SUPPORT
#define CEI_PCI_SYSFS_DIR "/sys/bus/pci/drivers/uceipci"
#define CEI_PCC_SYSFS_DIR "/sys/bus/pcmcia/drivers/cei_pcc1553_cs"
#endif

// ioctls
#define SET_REGION           0x13c61
#define GET_REGION_MEM       0x13c62
#define GET_REGION_SIZE      0x13c63
#ifdef KERNEL_24
 #define SET_REGION           0x13c61
 #define GET_REGION_MEM       0x13c62
 #define GET_REGION_SIZE      0x13c63
 #define GET_IRQ              0x13c65
 #define SET_PID              0x13c66
 #define SET_SIGNAL           0x13c67
 #define SET_SIGVAL           0x13c68
 #define GET_PCIREGION_BYTE   0x13c69
 #define GET_PCIREGION_WORD   0x13c6a
 #define GET_PCIREGION_DWORD  0x13c6b
 #define SET_INTERRUPT_MODE   0x13c6c
 #define SET_WAIT_QUEUE       0x13c6d
#endif


// PROTOTYPES
CEI_INT openDevice(CEI_UINT device);
CEI_INT closeDevice(CEI_UINT device);

#ifdef NO_SYSFS_SUPPORT
 CEI_INT getDevRes_conf(CEI_UINT device);
 CEI_INT openCEIConfigData(CEI_VOID);
 CEI_VOID closeCEIConfigData(CEI_VOID);
 CEI_INT getConfigDataInt(CEI_CHAR* data);
 CEI_INT findConfFile(CEI_CHAR* path);
 CEI_VOID removeWhiteSpace(CEI_CHAR* buf, CEI_CHAR* retbuf);
#else
 CEI_INT getSysfsAttr(CEI_UINT device, CEI_CHAR* attr_name, CEI_VOID* pData);
 CEI_INT getDevRes_sysfs(CEI_UINT device);
 CEI_INT readSysfsInode(CEI_INT minor, CEI_CHAR* inode, CEI_CHAR* val);
 CEI_INT findSysfsInode(CEI_CHAR* inode, CEI_CHAR* val, CEI_CHAR* path);
#endif

#ifdef LL_USB
CEI_INT usb_dev_open(PDEVMAP_T pDMap, CEI_INT opt);
CEI_INT usb_dev_close(PDEVMAP_T pDMap);
CEI_INT usb_dev_read(PDEVMAP_T pDMap, CEI_UINT addr, CEI_INT size, CEI_CHAR* buf);
CEI_INT usb_dev_write(PDEVMAP_T pDMap, CEI_UINT addr, CEI_INT size, CEI_CHAR* buf);
CEI_INT usb_ll_info(CEI_CHAR *ver);
#endif


// GLOBALS
DEVMAP_T cei_pMap[MAX_DEVICES];

#endif  // _LL_H_
