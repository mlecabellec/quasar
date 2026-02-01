/*============================================================================*
 * FILE:                      L O W L E V E L . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1997 - 2015 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:    Header file for low level device driver access.  This file
 *              defines the error codes returned from the memory mapping
 *              functions.
 *
 *===========================================================================*/

/* $Revision:  5.23 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  12/23/1997   Created file from busapi.h to support CEI-220 program loader.ajh.
  01/05/1998   Added function prototypes to make this a reusable component.
  12/30/1999   Added vbtGetPCIRevision() function.V3.30.ajh
  01/18/2000   Added support for the ISA-1553.V4.00.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  10/03/2001   Modify for improved initialization. V4.43 rhc
  01/07/2002   Added support for Quad-PMC and Dual Channel IP V4.46 rhc
  02/15/2002   Added support for modular API. v4.48
  02/19/2004   Add support for BusTools_API_OpenChannel
  02/19/2004   Add support for mapping devices only once
  04/06/2005   modified for new Linux lowlevel. bch
  01/19/2006   modified for 64-bit support. bch
  02/20/2006   added common data types. bch
  05/25/2006   added vbtConfigInterrupt. bch
  06/15/2007   added vbtInterruptMode. added vbtWaitForInterrupt. modified
                vbtGetInterrupt. bch
  08/07/2007   added CEI_TYPES_H check. bch 
  11/18/2008   added "cei_types.h" and removed duplicate typedefs. bch
  03/26/2009   removed CEI_* types. bch
  10/11/2011   added vbtInterruptWait. bch
  07/31/2012   added variables and packed DEVMAP_T and DEVINFO_T. modified 
                MAX_DEVICES. bch
  08/28/2014   added error codes. bch
  04/14/2015   added pragma pack for 2-byte struct alignment. bch
 */

#ifndef _LOWLEVEL_H_
#define _LOWLEVEL_H_

#include "cei_types.h"

// Define the max number of physical devices we support
#define MAX_DEVICES   16

/**********************************************************************
*  Lowlevel address mapping structure
**********************************************************************/
/* sets the struct alignment to 2-bytes */
#define ENABLE_STRUCT_ALIGNMENT

#define MAX_MEMORY   6     /* Number of supported memory regions */
#define MAX_PORTS    2     /* Number of supported IO regions     */

#ifdef ENABLE_STRUCT_ALIGNMENT
#pragma pack(push, 2)  /* struct alignment set to 2-byte boundary */
#endif
typedef enum _BUS_TYPE {    /* Host bus types supported           */
    BUS_INTERNAL,
    BUS_ISA,
    BUS_PCI,
    BUS_VME,
    BUS_PCMCIA,
    BUS_USB,
    BUS_OTHER
} BUS_TYPE;           // These are the bus types supported by lowlevel

/* structure for communicating memory mapping information about a device. */
typedef struct _DEVMAP_T {
   CEI_INT       busType;                    // One of BUS_TYPE.
   CEI_INT       interruptNumber;            // Interrupt number.
   CEI_INT       memSections;                // Number of memory regions defined.
   CEI_INT       flagMapToHost[MAX_MEMORY];  // Set to map region into host space.
   CEI_CHAR*     memHostBase[MAX_MEMORY];    // Base address of region in host space.
   CEI_ULONG     memStartPhy[MAX_MEMORY];    // Physical base address of region.
   CEI_UINT      memLengthBytes[MAX_MEMORY]; // Length of region in bytes.
   CEI_INT       portSections;               // Number of I/O port regions.
   CEI_ULONG     portStart[MAX_PORTS];       // I/O Address of first byte.
   CEI_UINT      portLength[MAX_PORTS];      // Number of bytes in region.
   CEI_INT       llDriverVersion;            // Low Level driver version.
   CEI_INT       KernelDriverVersion;        // Kernel driver version.
   CEI_INT       hKernelDriver;              // Handle to the kernel driver.
   CEI_UINT      VendorID;                   // Vendor ID if PCI card
   CEI_UINT      DeviceID;                   // Device ID if PCI card
   CEI_INT       device;                     // This the device
   CEI_INT       use_count;                  // Number of user channels
   CEI_INT       use_channel_map;
   CEI_INT       mapping;      
} DEVMAP_T, *PDEVMAP_T;

typedef struct _DEVINFO_T {
   CEI_INT      busType;                    // the bus type.
   CEI_INT      nchan;                      // number of channels
   CEI_INT      irig;                       // IRIG option 0=no 1=yes.
   CEI_INT      mode;                       // mode 0=single 1=multi.
   CEI_INT      memSections;                // number of PCI memory sections
   CEI_ULONG    base_address;               // base address
   CEI_UINT     VendorID;                   // Vendor ID if PCI card
   CEI_UINT     DeviceID;                   // Device ID if PCI card
   CEI_UINT16   host_interface;             // host interface
} DEVINFO_T, *PDEVINFO_T;
#ifdef ENABLE_STRUCT_ALIGNMENT
#pragma pack(pop)  /* reset struct alignment */
#endif

// prototypes
// device addressing for PCI, ISA, and PCMCIA
CEI_INT vbtMapBoardAddresses(CEI_UINT device, // Device to open (0 - 9)
                             PDEVMAP_T pMap);  // Pointer to region mapping structure
CEI_VOID vbtFreeBoardAddresses(PDEVMAP_T pMap);  // Pointer to region mapping structure

// reads PCI config space
CEI_INT vbtGetPCIConfigRegister(CEI_UINT device,  // Device to open (0 - 9)
                                CEI_UINT offset,  // Offset of PCI config register
                                CEI_UINT length,  // Number of bytes to read (1,2,4)
                                CEI_VOID* value);     // Returned value of the register

// Retrieves device information
CEI_INT vbtGetDevInfo(CEI_UINT device, CEI_CHAR* devInfo, CEI_VOID* pData);

// handles hardware interrupts
CEI_INT vbtGetInterrupt(CEI_UINT device);
CEI_INT vbtConfigInterrupt(CEI_UINT device, CEI_INT signal, CEI_INT pid, CEI_INT val);
CEI_INT vbtInterruptMode(CEI_UINT device, CEI_INT mode);
CEI_INT vbtInterruptWait(CEI_UINT device, CEI_INT mode, CEI_INT val);
CEI_INT vbtWaitForInterrupt(CEI_UINT device);

// device addressing for VME boards
CEI_INT vbtMapBoardAddress(CEI_UINT base_address, CEI_UINT length, CEI_CHAR** addr, CEI_VOID* cardnum, CEI_INT addressing_mode);
CEI_VOID vbtFreeBoardAddress(CEI_UINT cardnum);
//CEI_INT vbtMapIoAddress(CEI_ULONG base_address, CEI_CHAR** addr);

CEI_INT vbtGetLibInfo(CEI_CHAR* ver);

/**********************************************************************
*  Error return codes from LOWLEVEL routines
**********************************************************************/
#define BTD_OK                0     /* success */
#define BTD_ERR_PARAM         1     /* invalid parameter */
#define BTD_ERR_NOACCESS      2     /* unable to map/access adapter */
#define BTD_ERR_INUSE         3     /* adapter already in use */
#define BTD_ERR_BADADDR       4     /* invalid address */
#define BTD_ERR_NODETECT      5     /* I/O or Config register ID invalid, board detect fail */
#define BTD_ERR_BADCFG        6     /* board jumper configuration invalid or not supported */
#define BTD_ERR_NOTSETUP      7     /* adapter has not been setup */
#define BTD_ERR_FPGALOAD      8     /* FPGA load failure */
#define BTD_ERR_WRITEFAIL     9     /* not used */
#define BTD_ERR_NOMEMORY      10    /* error allocating memory in SW version */
#define BTD_BAD_DEVICE_ID     11    /* device ID out of range */
#define BTD_ERR_BADADDRMAP    12    /* bad initial mapping of address */
#define BTD_ERR_BADEXTMEM     13    /* bad extended memory mapping */
#define BTD_ERR_BADBOARDTYPE  14    /* Unknown board type */
#define BTD_ERR_BADWCS        15    /* Verify failure reading Writable Control Store */

#define BTD_NO_PLATFORM       18    /* Platform specified unknown or not supported */
#define BTD_BAD_MANUFACTURER  19    /* IP ID PROM Manufacturer code not 0x79 */
#define BTD_BAD_MODEL         20    /* IP ID PROM Model number not 0x05(MF) or 0x08(SF) */
#define BTD_BAD_SERIAL_PROM   21    /* IP Serial PROM needs update, no support for this version */
#define BTD_NEW_SERIAL_PROM   22    /* Serial PROM too new, not supported by this software */
#define BTD_CHAN_NOT_PRESENT  23    /* Channel not present (on multi-channel board) */
#define BTD_NON_SUPPORT       24    /* Bus/Carrier/OS combination not supported by API */
#define BTD_BAD_HW_INTERRUPT  25    /* Hardware interrupt number bad or not defined in registry */
#define BTD_FPGA_NOT_CLEAR    26    /* The FPGA configuration failed to clear */
#define BTD_NEW_PCCARD_FW     27    /* PCC-1553 firmware is too new for this version of the API */
#define BTD_OLD_PCCARD_FW     28    /* PCC-1553 firmware is too old, use the JAM Player to update it */
#define BTD_BAD_CONF_FILE     29    /* Unable to open ceidev.conf */
#define BTD_NO_DRV_MOD        30    /* No Driver Module found */
#define BTD_IOCTL_DEV_ERR     31    /* Error in ioctl get device */
#define BTD_IOCTL_SET_REG     32    /* Error in ioctl set region */
#define BTD_IOCTL_REG_SIZE    33    /* Error in getting ioclt region size */
#define BTD_IOCTL_GET_REG     34    /* Error in ioctl get region */
#define BTD_BAD_SIZE          35    /* Region size is 0 */
#define BTD_BAD_PROC_ID       36    /* Unable to set process ID */
#define BTD_HASH_ERR          37    /* Unable to setup hash table */
#define BTD_NO_HASH_ENTRY     38    /* No hash table entry found */
#define BTD_WRONG_BOARD       39    /* Wrong board type for command */
#define BTD_MODE_MISMATCH     40    /* IPD1553 mismatch in the mode. */
#define BTD_IRIG_NO_LOW_PEAK  41    /* No lower peak on IRIG DAC calibration */
#define BTD_IRIG_NO_HIGH_PEAK 42    /* No upper peak on IRIG DAC calibration */
#define BTD_IRIG_LEVEL_ERR    43    /* Delta between MAX and MIN DAC peak values less than required */
#define BTD_IRIG_NO_SIGNAL    44    /* No IRIG Signal Detected  */
#define BTD_RTADDR_PARITY     45    /* Parity Error on Hardwired RT address lines */
#define BTD_BAD_RAM_ADDR      46    /* Address is not in RAM */
#define BTD_BAD_BYTE_COUNT    47    /* Byte count not on 4 byte boundary */
#define BTD_TIMER_FAIL        48    /* failed to create a timer */

/**********************************************************************
*  Error return codes from LOWLEVEL routines
**********************************************************************/
#define BTD_ERR_NOWINRT       50    /* WinRT driver not loaded/started */
#define BTD_ERR_BADREGISTER   51    /* WinRT parameters don't match registry */
#define BTD_ERR_BADOPEN       52    /* WinRT device open failed */
#define BTD_UNKNOWN_BUS       53    /* Bus is not PCI, ISA or VME */
#define BTD_BAD_LL_VERSION    54    /* Unsupported lowlevel driver installed */
#define BTD_BAD_INT_EVENT     55    /* Unable to create/destroy interrupt event */
#define BTD_ISR_SETUP_ERROR   56    /* Error setting up the ISR driver */
#define BTD_CREATE_ISR_THREAD 57    /* Error creating/destroying the ISR thread */
#define BTD_NO_REGIONS_TO_MAP 58    /* No regions requested in call to vbtMapBoardAddresses */

#define BTD_RESOURCE_ERR      60    /* Integrity Resource Error */
#define BTD_READ_IODEV_ERR    61    /* Integrity IO Device Read Error */
#define BTD_MEMREG_ERR        62    /* Integrity error getting memory region */
#define BTD_MEM_MAP_ERR       63    /* Integrity Memory Mapping error */
#define BTD_CLK_RATE_NOT_SET  64    /* Error setting clk rate */

/**************************************************************************
* Error code from BTVXIMAP.C
**************************************************************************/
#define BTD_VIOPEN_FAIL       70    /* viOpen Error */
#define BTD_VIMAPADDRESS_FAIL 71    /* viMapAddress Error */
#define BTD_VIOPENDEFAULTRM   72    /* viOpenDefaultRM Error */
#define BTD_VIUNMAP_ERR       73    /* viUnMapAddress Error */

#define BTD_SEM_CREATE        80    /* Error Creating semaphore */
#define BTD_TASK_CREATE       81    /* Error spawning task */
#define BTD_EVENT_WAIT_FAILED    -82   /* CEI-ERROR -- Event Wait Failure */
#define BTD_EVENT_WAIT_ABANDONED -83   /* CEI-ERROR -- Event Wait Abandoned */
#define BTD_EVENT_WAIT_TIMEOUT   84   /* CEI-INFO -- Timeout on Event Wait */
#define BTD_EVENT_WAIT_UNKNOWN   -85   /* CEI-ERROR -- Unknown Event Error */
#define BTD_EVENT_SIGNAL_ERR  86   /* CEI-ERROR -- Error Occurred During Event Signal */
#define BTD_SET_PRIORITY_ERR  87   /* CEI-ERROR -- Error Setting Thread Priority */
#define BTD_THRD_CREATE_FAIL  88   /* CEI-ERROR -- Thread Create Failure */

#define BTD_CLOSE_ERR         90   /* CEI-ERROR -- Failed to close 1553 device */
#define BTD_OPEN_ERR          91   /* CEI-ERROR -- Failed to open 1553 device */
#define BTD_VBT_OPEN_ERR      92   /* CEI-ERROR -- Failure in vbtOpen1553Channel */
#define BTD_FIND_DEV_ERR      93   /* CEI_ERROR -- Failure in BusTools_FindDevice */
#define BTD_LIST_DEV_ERR      94   /* CEI_ERROR -- Failure in BusTools_ListDevices */

#define BTD_ERR_SHMEM_OBJ     100  /* failed to create/destroy shared memory mapping object */
#define BTD_ERR_SHMEM_MAP     101  /* failed to map/unmap shared memory block */
#define BTD_ERR_SHMEM_ATTR    102  /* failed to obtain shared memory block attributes */
#define BTD_ERR_MUTEX_OBJ     103  /* failed to create/destroy mutex object */
#define BTD_ERR_MUTEX_ATTR    104  /* failed to update mutex attributes */
#define BTD_ERR_MUTEX_LOCK    105  /* failed to acquire/release mutex */
#define BTD_ERR_COND_OBJ      106  /* failed to create/destroy condition variable object */
#define BTD_ERR_COND_ATTR     107  /* failed to update condition variable attributes */
#define BTD_ERR_COND_OPER     108  /* failed to wait/signal condition variable */
#define BTD_ERR_IPC_KEY       109  /* failed to obtain unique IPC key */
#define BTD_ERR_CFGLOCK_OBJ   110  /* failed to create/destroy config lock object */
#define BTD_ERR_CFGLOCK_ATTR  111  /* failed to get/set config lock attributes */
#define BTD_ERR_CFGLOCK_LOCK  112  /* failed to acquire/release config lock */
#define BTD_ERR_READ_PCI_CFG  113  /* failed to read PCI configuration register */
#define BTD_ERR_LOAD_CEIINST  114  /* failed to load cei_install library */

#define BTD_ERR_USB_PROT      115  /* error in the USB protocol */
#define BTD_ERR_USB_OPER      116  /* error in the USB operation */
#define BTD_ERR_SEM_OBJ       117  /* failed to open/close/destroy a semaphore */
#define BTD_ERR_SEM_OPER      118  /* failed to lock/unlock a semaphore */
#define BTD_ERR_FIFO_OPER     119  /* error in the FIFO operation */
#define BTD_ERR_EVENT_OBJ     120  /* failed to create/destroy event object */

#endif  // _LOWLEVEL_H_
