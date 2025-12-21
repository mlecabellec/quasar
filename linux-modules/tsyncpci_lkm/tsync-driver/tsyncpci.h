/*******************************************************************************
**
**  Module  : tsyncpci.h
**  Date    : 08/25/08
**  Purpose : This driver provides an interface to the TSYNC-PCIe
**
**  Copyright(C) 2008 Spectracom Corporation. All Rights Reserved.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
*******************************************************************************/
#ifndef TSYNC_PCI_H
#define TSYNC_PCI_H

#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/wait.h>

#include "kernel_compat.h"
#include "ddtsync.h"
#include "tsync_types.h"
#include "tsync_hw.h"
#include "tsync_gpio_stamp_queue_structs.h"

/*******************************************************************************
** Synchronization Definitions 
*******************************************************************************/

/*
** Spin lock.
*/
#define TPRO_SPIN_LOCK_INIT(p1) \
    do { \
        spin_lock_init(&(p1)->lock); \
        TPRO_LOG(TPRO_LOG_SPINLOCK, \
	         ("[J%lu]%s: Spinlock %p initialized\n", \
		 jiffies, __FUNCTION__, (p1))); \
    } while (0)
#define TPRO_SPIN_LOCK(p1) \
    do { \
        TPRO_LOG(TPRO_LOG_SPINLOCK, \
	         ("[J%lu]%s: Locking spinlock %p...\n", \
                 jiffies,  __FUNCTION__, (p1))); \
        spin_lock_irqsave(&(p1)->lock, (p1)->flags); \
    } while (0)
#define TPRO_SPIN_UNLOCK(p1) \
    do { \
      spin_unlock_irqrestore(&(p1)->lock, (p1)->flags); \
        TPRO_LOG(TPRO_LOG_SPINLOCK, \
	         ("[J%lu]%s: Spinlock %p unlocked\n", \
                 jiffies, __FUNCTION__, (p1))); \
    } while (0)
#define TPRO_SPIN_LOCK_DESTROY(p1) \
    do { \
        /* Nothing to do for destroy. */ \
        TPRO_LOG(TPRO_LOG_SPINLOCK, \
	         ("[J%lu]%s: Spinlock %p destroyed\n", \
                 jiffies, __FUNCTION__, (p1))); \
    } while (0)

/*
** Mutex.
*/
#define TPRO_MUTEX_INIT(p1) \
    do { \
        sema_init((p1), 1); \
        TPRO_LOG(TPRO_LOG_MUTEX, \
	         ("[J%lu]%s: Mutex %p initialized\n", \
		 jiffies, __FUNCTION__, (p1))); \
    } while (0)
#define TPRO_MUTEX_LOCK(p1) \
    do { \
        TPRO_LOG(TPRO_LOG_MUTEX, \
	         ("[J%lu]%s: Locking mutex %p...\n", \
                 jiffies, __FUNCTION__, (p1))); \
        down(p1); \
        TPRO_LOG(TPRO_LOG_MUTEX, \
	         ("[J%lu]%s: Mutex %p locked\n", \
                 jiffies, __FUNCTION__, (p1))); \
    } while (0)
#define TPRO_MUTEX_UNLOCK(p1) \
    do { \
        up(p1); \
        TPRO_LOG(TPRO_LOG_MUTEX, \
	         ("[J%lu]%s: Mutex %p unlocked\n", \
                 jiffies, __FUNCTION__, (p1))); \
    } while (0)
#define TPRO_MUTEX_DESTROY(p1) \
    do { \
        /* Nothing to do for destroy. */ \
        TPRO_LOG(TPRO_LOG_MUTEX, \
	         ("[J%lu]%s: Mutex %p destroyed\n", \
                 jiffies, __FUNCTION__, (p1))); \
    } while (0)

/*******************************************************************************
** Logging Definitions
*******************************************************************************/

/* Logging flags. */
#define TPRO_LOG_NOTHING      0x00000000
#define TPRO_LOG_LOAD         0x00000001
#define TPRO_LOG_OPEN         0x00000002
#define TPRO_LOG_IOCTL        0x00000004
#define TPRO_LOG_CLOSE        0x00000008
#define TPRO_LOG_UNLOAD       0x00000010
#define TPRO_LOG_FIND_DEV     0x00000020
#define TPRO_LOG_MUTEX        0x00000040
#define TPRO_LOG_SPINLOCK     0x00000080
#define TPRO_LOG_WAITQ        0x00000100
#define TPRO_LOG_SCOPE_ENTER  0x08000000
#define TPRO_LOG_SCOPE_EXIT   0x10000000
#define TPRO_LOG_INFO_1       0x20000000
#define TPRO_LOG_ERROR        0x40000000
#define TPRO_LOG_ALWAYS       0x80000000
#define TPRO_LOG_EVERYTHING   0xFFFFFFFF

#ifdef TPRO_DEBUG
    #define TPRO_PRINT(fmt, args...) printk(KERN_DEBUG fmt, ## args)

    /* If defined, TPRO_CHECK performs data integrity checking. */
    #define TPRO_CHECK
    #define TPRO_LOG_FLAGS    TPRO_LOG_EVERYTHING
    #define TPRO_LOG_ERR          TPRO_PRINT("TPRO ERROR, Line %d, File '%s'\n", \
                                      __LINE__, __FILE__)
    #define TPRO_LOG(p1, p2)      if ((p1) & (TPRO_LOG_FLAGS)) TPRO_PRINT p2
    #define TPRO_LOG_STAMP        TPRO_PRINT("[J%lu]%s, Line %d, File '%s'\n", \
                                      jiffies, __FUNCTION__, \
                                      __LINE__, __FILE__))
#else
    #define TPRO_PRINT(fmt, args...)
    #define TPRO_LOG(p1, p2)
    #define TPRO_LOG_STAMP
    #define TPRO_LOG_ERR
#endif //TPRO_DEBUG

/*
** Maximum number of TSYNC-PCI boards the driver will support
*/
#define TSYNC_PCI_MAX_NUM          (8)

#define TSYNC_PCI_BASE_MINOR       (0)
#define TSYNC_PCI_DRV_NAME         "tsyncpci"
#define TSYNC_PCI_DRV_STRING       "Spectracom TSync Timing Board"
#define TSYNC_PCI_DRV_COPYRIGHT    "Copyright (c) 2013 Spectracom Corporation"

/*
**  the Vendor/Device ID for the hardware
*/
#define TPRO_VENDOR_ID            (0x10B5)
#define TPRO_5V_DEVICE_ID         (0x9050)
#define TPRO_3V_DEVICE_ID         (0x9030)
#define TPRO_SUBSYSTEM_VENDOR_ID  (0x1347)

#define TPRO_U66_VENDOR_ID        (0x1AD7)
#define TPRO_U66_DEVICE_ID        (0x9100)
#define TPRO_U66_SUBSYSTEM_VENDOR_ID (0x1AD7)

#define TSYNC_VENDOR_ID           (0x1AD7)
#define TSYNC_DEVID               (0x8000)

/*
** Subsystem Device IDs
*/
#define TPRO_PCI                  (0x9050)
#define TSAT_PCI                  (0x9070)
#define SECURESYNC                (0x8100)
#define TSYNC_PCIe                (0x8000)
#define TSYNC_CTC                 (0x7500)
#define TSYNC_CPCI                (0x7800)
#define TSYNC_VPX                 (0x8700)
#define TSYNC_PMC                 (0x7400)
#define TSYNC_PCI104              (0x8500)

#define TSYNC_IRQ_PPS        0
#define TSYNC_IRQ_TIMESTAMP  10
#define TSYNC_IRQ_HEART_BEAT 11
#define TSYNC_IRQ_MATCH      12

typedef struct tpropci_spinlock_tag_t
{
  spinlock_t    lock;
  unsigned long flags;
} tpropci_spinlock_t;

/*
** Forward references for the device and user structures.
*/
struct tsyncpci_user_tag_t;
struct tsyncpci_dev_tag_t;

struct funcTable_s;

/*
** TSYNC USER OBJECT 
*/
typedef struct tsyncpci_user_tag_t {

  struct tsyncpci_user_tag_t * pNext;     /* Pointer to next node */
  struct tsyncpci_user_tag_t * pPrev;     /* Pointer to previous node */
  struct tsyncpci_dev_tag_t  * pDevice;   /* Address of the device structure */

} tsyncpci_user_t;

typedef struct {
  wait_queue_head_t waitQueue;  
  atomic_t          flag;   
} interrupt_context_t;


/*
**  TSYNC DEVICE OBJECT
*/
typedef struct tsyncpci_dev_tag_t {

  int            hDevice;      /* handle to our device */
  unsigned long  ioAddr;       /* io port address */
  int            ioSize;       /* requested io size */
  int            intVector;    /* interrupt vector */
  int            irq;          /* interrupt request line */
  volatile int   intCnt;       /* interrupt counter */
  volatile int   eventCnt;     /* external event counter */
  unsigned char  intrMask;     /* interrupt mask */
  unsigned short device;       /* board device ID */
  unsigned short options;      /* board options SAT/PRO */
  unsigned char  slotPosition; /* slot position */
  dev_t          devNum;

  /*
  **  TSYNC INTERRUPT INFO BUFFERS
  */

  volatile unsigned int intCounter[TSYNC_INTERRUPT_COUNT];
  TSYNCD_TimeSecondsObj intTime[TSYNC_INTERRUPT_COUNT];


  /*
  **  function pointer table
  */
  struct funcTable_s * pFunctionTable; /* function table for low level functions */
    
  /*
  **  heartbeat interrupt context
  */
  wait_queue_head_t heart_wait;  /* heartbeat waitaphore */
  atomic_t          heartFlag;   /* heartbeat interrupt flag */

  /*
  **  match time interrupt context
  */
  wait_queue_head_t match_wait;  /* match time waitaphore */
  atomic_t          matchFlag;   /* match time interrupt flag */

  /*
  **  event interrupt context
  */
  wait_queue_head_t event_wait;  /* event waitaphore */
  atomic_t          eventFlag;   /* event interrupt flag */

    interrupt_context_t tsyncInterrupts[TSYNC_INTERRUPT_COUNT];

  /*
  **  circular event buffer context
  */
  unsigned short          fifoVector[510]; /* FIFO vector */
  volatile unsigned short headIndex;       /* head index in circ buffer */
  volatile unsigned short tailIndex;       /* tail index in circ buffer */

#include "tsync_instance_fields.h"

  /*
  **  DEVICE: access context
  */
  struct semaphore            deviceLock; /* device access lock */
  tpropci_spinlock_t          userLock;   /* user list access lock */
  struct tsyncpci_user_tag_t * pFirstUser; /* first user has extra capability */
  struct tsyncpci_user_tag_t * pLastUser;  /* new users are added to list here */

} tsyncpci_dev_t;

typedef struct {

  int argVec[4];  /* input/output argument vector */

} tsyncpci_ioctl_cmd_t;

/*
 * Helper functions
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||\
	(defined(RHEL_MAJOR) && RHEL_MAJOR >= 8)
#define TSYNC_ACCESS_OK(vfy, ptr, siz) \
    access_ok(ptr, siz)
#else
#define TSYNC_ACCESS_OK(vfy, ptr, siz) \
    access_ok(vfy, ptr, siz)
#endif


#endif  // TSYNC_PCI_H
