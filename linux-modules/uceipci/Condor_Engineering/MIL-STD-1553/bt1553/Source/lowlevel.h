/*============================================================================*
 * FILE:                      L O W L E V E L . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2017 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:    Header file for low-level device driver access.
 *
 *              This file defines the interface to the low-level device 
 *              interface routines.
 *
 *              Warning: this file is shared by many products, so please 
 *              exercise caution when making modifications.
 *
 *===========================================================================*/

/* $Revision:  8.22 Release $
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
  07/26/2005   Merged with CEI-x20 version.CEI-x20 v3.93.skb
  01/02/2006   Add CEI Thread and interrupt function support
  03/21/2006   Added x20 support for platform-independent types.CEI-x20 v3.95.skb
  11/14/2006   Added vbtHookInterruptX20, vbtUnhookInterruptX20, and 
               vbtIntsSupportedX20.CEI-x20 v3.96.skb
  12/03/2006   Added prototype from CEI_INTERRUPT.h and eliminated CEI_INTERRUPT.h
  05/14/2007   Added routines providing limited CEI-x20 support for the 
               following: interrupt handling, multi-thread support, multi-process
               support.CEI-x20 v3.97.skb
  11/14/2007   Updated CEI-x20 board mapping routine to store a textual error 
               description upon failure.CEI-x20 v4.10.skb
  05/06/2008   Add Error codes
  07/02/2010   Added vbtDMASetup, vbtDMAClose, vbtDMARead.bch
  04/18/2011   Added CEI_GET_ENV_VAR and CEI_SET_ENV_VAR.bch
  07/12/2012   Added CEI_THREAD_EXIT, CEI_MUTEX_LOCK, CEI_MUTEX_UNLOCK,
               CEI_HANDLE_DESTROY.bch
  02/26/2015   Added error codes for semaphore, event, and USB protocol. bch
  04/14/2015   Added pragma pack for 2-byte struct alignment. bch
  11/16/2017   Updated prototype for currently-uncalled vbtDMASetup function.
 */

#ifndef _LOWLEVEL_H_
#define _LOWLEVEL_H_

/*---------------------------------------------------------------------------*
 * Include definitions of common platform-independent types
 *---------------------------------------------------------------------------*/
#include "cei_types.h"
#include "target_defines.h"


/*---------------------------------------------------------------------------*
 * Miscellaneous constants, types, and macros
 *---------------------------------------------------------------------------*/

/* maximum number of memory regions supported by a device */
#define MAX_MEMORY   6     

/* maximum number of ports supported by a device */
#define MAX_PORTS    2

/* host bus types supported */
typedef enum _BUS_TYPE 
{
    BUS_INTERNAL,
    BUS_ISA,
    BUS_PCI,
    BUS_VME,
    BUS_PCMCIA,
    BUS_USB,
    BUS_OTHER
} BUS_TYPE;           

/* maximum number of physical devices supported */
#define MAX_DEVICES   MAX_BTA

/* max number of logical devices (cardnum's) supported */
#define MAX_CARDNUM   MAX_BTA          


/*---------------------------------------------------------------------------*
 * Define dummy thread and sync types (if not already declared)
 *---------------------------------------------------------------------------*/
#ifndef CEI_THREAD
typedef CEI_VOID *CEI_THREAD;
#endif
#ifndef CEI_EVENT
typedef CEI_VOID *CEI_EVENT;
#endif
#ifndef CEI_MUTEX
typedef CEI_VOID *CEI_MUTEX;
#endif
#ifndef CEI_SHARED_COND
typedef CEI_VOID *CEI_SHARED_COND;
#endif
#ifndef CEI_SHARED_MUTEX
typedef CEI_VOID *CEI_SHARED_MUTEX;
#endif

/*---------------------------------------------------------------------------*
 * CEI-x20 memory mapping structure
 *---------------------------------------------------------------------------*/
typedef struct CEIX20_MEMORY_MAP_T 
{
   CEI_UINT32 num_mem_rgns_mapped;      /* the number of memory regions that */
                                        /* have been mapped                  */
   CEI_CHAR *rgn_host_addr[MAX_MEMORY]; /* the base address of each mapped   */
                                        /* memory region in host space       */
   CEI_CHAR *rgn_phys_addr[MAX_MEMORY]; /* the physical base address of each */
                                        /* memory region                     */
} CEIX20_MEMORY_MAP_T, *PCEIX20_MEMORY_MAP_T;


/*---------------------------------------------------------------------------*
 * CEI-x20 internal interrupt handler
 *---------------------------------------------------------------------------*/
typedef CEI_VOID (*AR_INT_CALLBACK_FUNC)(CEI_INT16 board);


/*---------------------------------------------------------------------------*
 * CEI-x20 low-level capability flags (see vbtGetLowlevelCapsX20)
 *---------------------------------------------------------------------------*/
#define CEIX20_LLCAPS_INTERRUPT   0
#define CEIX20_LLCAPS_MULTIPROC   1
#define CEIX20_LLCAPS_MULTITHRD   2


/*---------------------------------------------------------------------------*
 * CEI-x20 low-level routines
 *---------------------------------------------------------------------------*/


/*===========================================================================*
 * ENTRY POINT: vbtMapBoardAddressesExtX20
 *===========================================================================*
 *
 * FUNCTION:    Fill out the supplied CEI-x20 memory mapping structure.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Attempt to map all memory regions associated with the given 
 *              CEI-x20 device.  If successful, the supplied mapping structure
 *              will contain valid mapping info (refer to the definition of 
 *              the CEIX20_MEMORY_MAP_T structure for more information).
 *
 *===========================================================================*/
CEI_INT32 vbtMapBoardAddressesExtX20
(
   CEI_UINT32 board,            /* (i) board index (0 - 9)                   */
   PCEIX20_MEMORY_MAP_T map,    /* (o) receives mapping info (if successful) */
   CEI_CHAR *err,               /* (o) receives textual description of       */
                                /*     error (upon failure)                  */
   CEI_UINT32 err_max           /* (i) capacity of 'err' buffer (in bytes)   */
);   

/*===========================================================================*
 * ENTRY POINT: vbtFreeBoardAddressesX20
 *===========================================================================*
 *
 * FUNCTION:    Free system resources associated with the given device.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Release all system resources currently allocated to support
 *              the given CEI-x20 device.
 *
 *===========================================================================*/
void vbtFreeBoardAddressesX20
(
   CEI_UINT32 board,            /* (i) board index (0 - 9)                   */
   PCEIX20_MEMORY_MAP_T map     /* (i) mapping structure obtained by prior   */
                                /*     call to vbtMapBoardAddressesX20       */
);

/*===========================================================================*
 * ENTRY POINT: vbtHookInterruptExtX20
 *===========================================================================*
 *
 * FUNCTION:    Hook the interrupt associated with the given board.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Install a low-level interrupt service routine (ISR).  When
 *              executed, the ISR should check the interrupt control register
 *              (ICR) to determine if the associated board generated the 
 *              interrupt (that is, check if bit 0 in the ICR is 1).  If the 
 *              board generated the interrupt, the ISR should then clear the 
 *              interrupt (by clearing bit 0 in the ICR) and call the 
 *              provided interrupt callback function.
 *
 *===========================================================================*/
CEI_INT32 vbtHookInterruptExtX20
(
   CEI_UINT32 board,              /* (i) board index (0 - 9)                 */
   PCEIX20_MEMORY_MAP_T map,      /* (i) mapping structure obtained by prior */
                                  /*     call to vbtMapBoardAddressesExtX20  */
   CEI_UINT32 board_type,         /* (i) base board type (e.g., CEI_520A)    */
   CEI_UINT32 icr_offset,         /* (i) byte offset from the base address   */
                                  /*     of the 4-byte interrupt control     */
                                  /*     register                            */
   AR_INT_CALLBACK_FUNC callback, /* (i) call this function whenever a       */
                                  /*     hardware interrupt occurs           */
   CEI_UINT32 *reserved,          /* (i) reserved - must be NULL             */
   CEI_CHAR *err,                 /* (o) receives textual description of     */
                                  /*     error (upon failure)                */
   CEI_UINT32 err_max             /* (i) capacity of 'err' buffer (in bytes) */
);

/*===========================================================================*
 * ENTRY POINT: vbtUnhookInterruptExtX20
 *===========================================================================*
 *
 * FUNCTION:    Unhook the interrupt associated with the given board.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Uninstall the low-level interrupt service routine that was 
 *              previously installed using a call to vbtHookInterruptX20.
 *
 *===========================================================================*/
CEI_INT32 vbtUnhookInterruptExtX20
(
   CEI_UINT32 board,            /* (i) board index (0 - 9)                   */
   PCEIX20_MEMORY_MAP_T map,    /* (i) mapping structure obtained by prior   */
                                /*     call to vbtMapBoardAddressesX20       */
   CEI_CHAR *err,               /* (o) receives textual description of       */
                                /*     error (upon failure)                  */
   CEI_UINT32 err_max           /* (i) capacity of 'err' buffer (in bytes)   */
);

/*===========================================================================*
 * ENTRY POINT: vbtGetLowlevelCapsX20
 *===========================================================================*
 *
 * FUNCTION:    Determine if the specified feature is supported by the low-
 *              level library on the current system and specified device.
 *
 * RETURN VAL:  TRUE  - the feature is supported
 *              FALSE - the feature is not supported
 *
 * DESCRIPTION: Certain low-level library features are not supported by all
 *              boards on all target platforms.  The features include:
 * 
 *                 feature                       capability flag
 *                 ----------------------------  ----------------------------
 *                 interrupt handling            CEIX20_LLCAPS_INTERRUPT
 *                 multi-process support         CEIX20_LLCAPS_MULTIPROC
 *                 multi-thread support          CEIX20_LLCAPS_MULTITHRD
 *           
 *              Return TRUE if the specified capability is supported by the 
 *              given device under the current operating system.  Otherwise, 
 *              return FALSE.
 *
 *===========================================================================*/
CEI_INT32 vbtGetLowlevelCapsX20
(
   CEI_UINT32 board,            /* (i) board index (0 - 9)                   */
   CEI_UINT32 cap_flag,         /* (i) capability flag (CEIX20_LLCAPS_xxx)   */
   CEI_UINT32 pci_dev_id        /* (i) pci device id - ignored unless        */
                                /*     cap_flag is CEIX20_LLCAPS_INTERRUPT   */
);

/*===========================================================================*
 * ENTRY POINT: vbtGetPCIDeviceIDExt
 *===========================================================================*
 *
 * FUNCTION:    Obtain the PCI device ID associated with the given board.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Read the PCI device ID from the PCI configuration space
 *              associated with the given board index.
 *
 *===========================================================================*/
CEI_INT32 vbtGetPCIDeviceIDExt
(
   CEI_UINT32 board,            /* (i) board index (0 - 9)                   */
   CEI_UINT32 *deviceID,        /* (o) receives corresponding PCI device ID  */
   CEI_CHAR *err,               /* (o) receives textual description of       */
                                /*     error (upon failure)                  */
   CEI_UINT32 err_max           /* (i) capacity of 'err' buffer (in bytes)   */
);

/*===========================================================================*
 * ENTRY POINT: vbtConfigLockExtX20
 *===========================================================================*
 *
 * FUNCTION:    Acquire or release global shared CEI-x20 config lock.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: A configuration lock is used to ensure that at most one thread
 *              (and process) is opening or closing a board at any given time.
 *              Recursive locking of a configuration lock is not supported.
 *
 *===========================================================================*/
CEI_INT32 vbtConfigLockExtX20
(
   CEI_UCHAR config_lock_id,    /* (i) unique global config lock id (0-15)   */
   CEI_UINT32 is_acquire,       /* (i) true if the lock should be acquired,  */
                                /*     false if it should be released        */
   CEI_CHAR *err,               /* (o) receives textual description of       */
                                /*     error (upon failure)                  */
   CEI_UINT32 err_max           /* (i) capacity of 'err' buffer (in bytes)   */
);

/*===========================================================================*
 * ENTRY POINT: vbtMapSharedMemoryExtX20
 *===========================================================================*
 *
 * FUNCTION:    Map the desired shared global memory block into the calling
 *              process's address space, allocating the block if it doesn't
 *              already exist.  
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: If a shared global memory block associated with the given 
 *              application-specific id has already been allocated on the 
 *              target system, map the memory block into the calling 
 *              process's address space and set 'already_exists' to true.  
 *              Otherwise, allocate and map the specified global memory block
 *              and set 'already_exists' to false.
 *
 *              If the memory block is being allocated for the first time, 
 *              zero out the entire block.
 *
 *===========================================================================*/
CEI_INT32 vbtMapSharedMemoryExtX20
(
   CEI_UCHAR block_id,          /* (i) unique global block id (0-15)         */
   CEI_UINT32 block_size,       /* (i) memory block size (in bytes)          */
   CEI_VOID **block_ptr,        /* (o) receives pointer to memory block      */
   CEI_VOID **block_handle,     /* (o) receives platform-specific handle     */
                                /*     to the shared memory block            */
   CEI_UINT32 *already_exists,  /* (o) receives true if the block already    */
                                /*     exists, false otherwise               */
   CEI_UINT32 *is_exclusive,    /* (o) receives true if no other processes   */
                                /*     currently have the block mapped,      */
                                /*     false otherwise                       */
   CEI_CHAR *err,               /* (o) receives textual description of       */
                                /*     error (upon failure)                  */
   CEI_UINT32 err_max           /* (i) capacity of 'err' buffer (in bytes)   */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedMutexInitExtX20
 *===========================================================================*
 *
 * FUNCTION:    Initialize the desired shared global mutex.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Initialize the mutex associated with the given unique lock id.
 *              If 'attach_to_existing' is true, 'mutex_local' receives a
 *              local mapping of 'mutex_global' that can be used in subsequent 
 *              vbtSharedMutexLockX20 and vbtSharedMutexUnlockX20 calls by the
 *              calling process.  If 'attach_to_existing' is false, the shared 
 *              global mutex is first created (and stored in 'mutex_global') 
 *              before the local mapping is stored in 'mutex_local'.  
 *
 *              Recursive locking is supported (that is, vbtSharedMutexLockX20 
 *              returns immediately if the caller already owns the lock), but 
 *              each call to vbtSharedMutexLockX20 must be matched with a 
 *              call to vbtSharedMutexUnlockX20.
 *
 *===========================================================================*/
CEI_INT32 vbtSharedMutexInitExtX20
(
   CEI_UCHAR mutex_id,             /* (i) unique global lock id (0-255)      */
   CEI_UINT32 attach_to_existing,  /* (i) true if the lock exists (see desc) */
   CEI_SHARED_MUTEX *mutex_global, /* (i/o) ptr to global mutex (see desc)   */
   CEI_SHARED_MUTEX **mutex_local, /* (o) receives local mutex ptr (see      */
                                   /*     desc)                              */
   CEI_CHAR *err,                  /* (o) receives textual description of    */
                                   /*     error (upon failure)               */
   CEI_UINT32 err_max              /* (i) capacity of 'err' buffer (in       */
                                   /*     bytes)                             */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedMutexLockX20
 *===========================================================================*
 *
 * FUNCTION:    Acquire the given lock.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Acquire the shared global mutex associated with the given 
 *              local mutex mapping that was returned by a prior call to 
 *              vbtSharedMutexInitX20.
 *
 *===========================================================================*/
CEI_INT32 vbtSharedMutexLockX20
(
   CEI_SHARED_MUTEX *mutex_local   /* (i) local mutex pointer (see desc)     */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedMutexUnlockX20
 *===========================================================================*
 *
 * FUNCTION:    Release the given lock.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Release the shared global mutex associated with the given 
 *              local mutex mapping that was returned by a prior call to 
 *              vbtSharedMutexInitX20.
 *
 *===========================================================================*/
CEI_INT32 vbtSharedMutexUnlockX20
(
   CEI_SHARED_MUTEX *mutex_local   /* (i) local mutex pointer (see desc)     */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedCondInitExtX20
 *===========================================================================*
 *
 * FUNCTION:    Initialize the desired shared global condition variable.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Initialize the condition variable associated with the given 
 *              unique condition variable id.  If 'attach_to_existing' is 
 *              true, 'cond_local' receives a local mapping of 'cond_global' 
 *              that can be used in subsequent vbtSharedCondWaitX20 and 
 *              vbtSharedCondSignalX20 calls by the calling process.  If 
 *              'attach_to_existing' is false, the shared global condition 
 *              variable is first created (and stored in 'cond_global') before
 *              the local mapping is stored in 'cond_local'.  
 *
 *===========================================================================*/
CEI_INT32 vbtSharedCondInitExtX20
(
   CEI_UCHAR cond_id,              /* (i) unique global cond var id (0-255)  */
   CEI_UINT32 attach_to_existing,  /* (i) true if the cond exists (see desc) */
   CEI_SHARED_COND *cond_global,   /* (i/o) ptr to global cond (see desc)    */
   CEI_SHARED_COND **cond_local,   /* (o) receives local cond ptr (see desc) */
   CEI_CHAR *err,                  /* (o) receives textual description of    */
                                   /*     error (upon failure)               */
   CEI_UINT32 err_max              /* (i) capacity of 'err' buffer (in       */
                                   /*     bytes)                             */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedCondWaitX20
 *===========================================================================*
 *
 * FUNCTION:    Wait until the condition variable is signaled.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Wait until the global condition variable associated with the
 *              given local condition variable mapping (that was returned by a
 *              prior call to vbtSharedCondInitX20) is signaled with a call to
 *              vbtSharedCondSignalX20.  The caller must own 'mutex_local'.  
 *              Atomically release the given mutex and enter wait state.  Upon
 *              return, the mutex is again owned by the caller.
 *
 *===========================================================================*/
CEI_INT32 vbtSharedCondWaitX20
(
   CEI_SHARED_COND *cond_local,    /* (i) local cond var pointer (see desc)  */
   CEI_SHARED_MUTEX *mutex_local   /* (i) local mutex pointer (see desc)     */
);

/*===========================================================================*
 * ENTRY POINT: vbtSharedCondSignalX20
 *===========================================================================*
 *
 * FUNCTION:    Wake up a thread waiting on a condition variable.
 *
 * RETURN VAL:  BTD_OK - operation completed successfully
 *              other  - refer to BTD_x error codes listed in lowlevel.h
 *
 * DESCRIPTION: Wake up a thread waiting for the global condition variable 
 *              associated with the given local condition variable mapping 
 *              (that was returned by a prior call to vbtSharedCondInitX20)
 *              to be signaled.  
 *
 *===========================================================================*/
CEI_INT32 vbtSharedCondSignalX20
(
   CEI_SHARED_COND *cond_local     /* (i) local cond var pointer (see desc)  */
);



/*---------------------------------------------------------------------------*
 *
 *
 * The following sections support the BusTools/1553-API
 *
 *
 *---------------------------------------------------------------------------*/


/***********************************************************************
* BusTools Low Level Device Driver error return constants
***********************************************************************/

#ifndef _LOW_LEVEL_ERRORS_
#define _LOW_LEVEL_ERRORS_

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

#define BTD_ERR_SEM_OBJ       97   /* failed to open/close/destroy a semaphore */
#define BTD_ERR_SEM_OPER      98   /* failed to lock/unlock a semaphore */
#define BTD_ERR_EVENT_OBJ     99   /* failed to create/destroy event object */
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
#define BTD_ERR_FIFO_OPER     117  /* error in the FIFO operation */

#endif /*_LOW_LEVEL_ERRORS_ */

#if defined(VXW_VME_PPC) || defined(VXW_PCI_PPC) || defined(VXW_PCI_X86) ||defined(VXW_VME_X86) || defined(CEIX20_TARGET_VXWORKS)
#define _UNIX_
#define HANDLE int
#define UINT unsigned
#endif /* defined(VXW_VME_PPC) ...... */

#if defined(_QNXNTO_PCI_X86_) || defined(_QNXNTO_PCI_PPC_)
#ifndef _UNIX_
#define _UNIX_
#ifndef HANDLE
#define HANDLE int
#endif
#define UINT unsigned
#endif /*_UNIX_ */
#endif /* defined(_QNXNTO_PCI_X86_) ...... */

#if defined(_LINUX_X86_) || defined(_LINUX_PPC_) || defined(_LINUX_X86_VME_) || defined(LYNXOS)
#ifndef _UNIX_
#define _UNIX_
#endif
#ifndef HANDLE
#define HANDLE int
#endif
#ifndef UINT
#define UINT unsigned int
#endif
#endif /* (_LINUX_X86_) ...... */

#if defined(INTEGRITY_VME_PPC) || defined (INTEGRITY_PCI_PPC)
#define _UNIX_
#define HANDLE int
#define UINT unsigned
#endif /* INTEGRITY */

/**********************************************************************
*  vbtHookInterrupt-invoked interrupt function return codes:
**********************************************************************/
#define BTD_NEXT_INTERRUPT    0
#define BTD_EXIT_INTERRUPT    1

/**********************************************************************
*  Lowlevel address mapping structure
**********************************************************************/
#ifdef PRAGMA_PACK
#pragma pack(push, _LL_BUSAPI, 2)   /* struct alignment set to 2-byte boundary */
#endif
typedef struct _DEVMAP_T {
   int            busType;                    /* One of BUS_TYPE. */
   int            interruptNumber;            /* Interrupt number. */
   int            memSections;                /* Number of memory regions defined. */
   int            flagMapToHost[MAX_MEMORY];  /* Set to map region into host space. */
   char         * memHostBase[MAX_MEMORY];    /* Base address of region in host space. */
   unsigned long  memStartPhy[MAX_MEMORY];    /* Physical base address of region. */
   unsigned int   memLengthBytes[MAX_MEMORY]; /* Length of region in bytes. */
   int            portSections;               /* Number of I/O port regions. */
   unsigned long  portStart[MAX_PORTS];       /* I/O Address of first byte. */
   unsigned int   portLength[MAX_PORTS];      /* Number of bytes in region. */
   int            llDriverVersion;            /* Low Level driver version. */
   int            KernelDriverVersion;        /* Kernel driver version. */
   CEI_HANDLE     hKernelDriver;              /* Handle to the kernel driver. */
   unsigned int   VendorID;                   /* Vendor ID if PCI card */
   unsigned int   DeviceID;                   /* Device ID if PCI card */
   int            device;                     /* This the device */
   int            use_count;                  /* Number of user channels */
   int            use_channel_map;            /* */
   int            mapping;                    /* Mapping option 0 - physical 1 - virtual */
} DEVMAP_T, *PDEVMAP_T;

typedef struct _DEVINFO_T {
   int            busType;                    /* One of BUS_TYPE. */
   int            nchan;                      /* number of channeld. */
   int            irig;                       /* IRIG option 0=no 1=yes. */
   int            mode;                       /* mode 0=single 1=multi. */
   int            memSections;                /* Number of PCI Memory section */
   unsigned long  base_address;               /* Base_address of the board */
   unsigned int   VendorID;                   /* Vendor ID if PCI card */
   unsigned int   DeviceID;                   /* Device ID if PCI card */
   unsigned short host_interface;              /* host interface data */
} DEVINFO_T, *PDEVINFO_T;
#ifdef PRAGMA_PACK
#pragma pack(pop, _LL_BUSAPI)  /* reset struct alignment */
#endif

int vbtGetInterrupt(UINT device);

int vbtFreeBoardAddresses(PDEVMAP_T pMap);  /* Pointer to region mapping structure */
int vbtMap1553BoardAddresses (UINT cardnum,
                              UINT device, /* Device to open (0 - 9) */
                              PDEVMAP_T pMap);  /* Pointer to region mapping structure */
int vbtMapBoardAddresses (UINT device, /* Device to open (0 - 9) */
                          PDEVMAP_T pMap);  /* Pointer to region mapping structure */

#if defined(__WIN32__)
int vbtHookInterrupt(UINT    device,
                     UINT    cardnum,         /* (i) BusTools card number */
                     void (_stdcall *fn)(int),/* (i) Address of user function to call */
                     int iPriority,           /* (i) Thread priority */
                     HANDLE  *phThread,       /* (o) Returned handle to thread */
                     HANDLE  *hIntEvent,      /* (o) Returned handle to interrupt event */
                     UINT  CurrentCardType,   /* (i) Board type for interrupt setup */
                     UINT  CurrentCardSlot,   /* (i) Carrier slot or board channel being setup */
                     UINT  CurrentCarrier);    /* (i) Type of IP carrier for interrupt setup  */
#endif

int  vbtCloseInterrupt(UINT device, UINT cardnum);
#if defined(LYNXOS_VME)
int   vbtMapIoAddress(unsigned int cardnum, unsigned long* p_addr);	
int   vbtMapBoardAddress(unsigned int cardnum, unsigned long* p_addr, unsigned long* vme_addr);
#else
int   vbtMapIoAddress(unsigned base_address, char **addr);  /* for VME carriers under VxWorks  */
int   vbtMapBoardAddress(unsigned    cardnum,     /* Card Number to open   */
                         unsigned    device,      /* WinRT device to open (0 - 9) */
                         char     **addr,      /* computed host address */
                         void    *MemorySize,  /* byte size of region mapped */
                         int      addressing_mode); /* for VME boards only */
#endif

void vbtFreeBoardAddress(UINT cardnum);

#if !defined(VXW_VME_68K) || !defined(LYNXOS_VME)
int  vbtGetPCIConfigRegister(CEI_UINT    device,  /* WinRT device to open (0 - 9) */
                             CEI_UINT    offset,  /* Offset of PCI config register */
                             CEI_UINT    length,  /* Number of bytes to read (1,2,4) */
                             CEI_UINT   *value);  /* Returned value of the register */

#else
#define vbtGetPCIConfigRegister(a,b,c,d) 0
#endif

int vbtGetDevInfo(int, char*, void*);
int vbtGetCEIDevInfo(UINT device,       /* WinRT device to open (0 - 15) */
                  DEVINFO_T *pInfo);   /* Pointer to device info structure */

void * LocalInterruptThreadFunction(void *);

#ifdef WIN32
CEI_UINT64 PerformanceCounter(void);
#endif

CEI_INT vbtDMASetup(CEI_UINT device, CEI_UINT channel, CEI_UINT count, CEI_UINT flag, pCEI_VOID* phy_addr, pCEI_VOID* usr_addr);
CEI_INT vbtDMAClose(CEI_UINT device, CEI_UINT channel);
CEI_INT vbtDMARead(CEI_UINT device, CEI_UINT channel, CEI_UINT addr_offset, CEI_UINT size, CEI_UINT flag, CEI_UINT* buff);

/**************************************************************************
 * Thread Function Prototypes                           
 * Implement these functions in lowlevel.c mem_unix.c mem_Solaris.c
 * mem_vxWorks.c, mem_integrity.c or mem_qnx.c to implement interrupt
 * processing.  
 *************************************************************************/
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD *pThread);
CEI_INT CEI_THREAD_CREATE(CEI_THREAD *pThread, CEI_INT iPriority, void *thread_func, void *thread_func_arg);
CEI_INT CEI_EVENT_CREATE(CEI_EVENT *pEvent);
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT *pEvent);
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT *pEvent, CEI_INT iDelay, CEI_MUTEX *pMutex);
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT *pEvent, CEI_MUTEX *pMutex);
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX *pMutex);
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX *pMutex);
CEI_INT CEI_THREAD_EXIT(CEI_VOID* pStatus, CEI_VOID* pVal);
CEI_INT CEI_MUTEX_LOCK(CEI_MUTEX *pMutex);
CEI_INT CEI_MUTEX_UNLOCK(CEI_MUTEX *pMutex);
CEI_INT CEI_HANDLE_DESTROY(CEI_VOID *pHandle);

CEI_INT CEI_GET_ENV_VAR(CEI_CHAR* name, CEI_CHAR* buf, CEI_INT size);
CEI_INT CEI_SET_ENV_VAR(CEI_CHAR* name, CEI_CHAR* buf);
CEI_INT CEI_GET_FILE_PATH(CEI_CHAR* name, CEI_CHAR* buf);

#endif  /* #ifndef _LOWLEVEL_H_ */
