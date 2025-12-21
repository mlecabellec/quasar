/******************************************************************************
 *                                                                            *
 * File:        pcie6509.h                                                    *
 *                                                                            *
 * Description: The interface to the PCIE6509 Linux device driver.            *
 *                                                                            *
 * Date:         02/13/2014                                                   *
 * History:                                                                   *
 *                                                                            *
 *  1  02/13/14  D. Dubash                                                    *
 *               Initial release                                              *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef __PCIE6509_H_INCLUDED__
#define __PCIE6509_H_INCLUDED__

#include "pcie6509_debug.h"

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (!TRUE)
#endif

/*****************************************************************
 *   DO NOT CHANGE THESE NAMES AS THEY ARE SEARCHED BY build_rpm * 
 *****************************************************************/
#define PCIE6509_SLERT_VERSION         "10-13_0"
#define PCIE6509_RedHawk_VERSION       "6.3.x_0"
#define PCIE6509_HARDWARE_DESC "High-drive (24mA), 96-bit digital input/output (DIO)"
/************************************************************/

/* module name(s) */
#define PCIE6509_VERSION  PCIE6509_RedHawk_VERSION

/******************************************************************************
 ***                                                                        ***
 ***              <<<< Define Board Specific Errors Here >>>>               ***
 ***                                                                        ***
 ******************************************************************************/
typedef struct _pcie6509_error_t {
    uint    error;
    char    *name;
    char    *desc;
} pcie6509_error_t;

#define PCIE6509_DEFINE_ERR(Command,Desc) { \
     Command, #Command, Desc \
}

/****
 **** WARNING!!! ERROR MESSAGES IN STRUCT BELOW MUST BE IN THE
 ****            SAME ORDER AS THOSE SPECIFIED IN THE ENUM LOCATED
 ****            IN THE `pcie6509_user.h' FILE.
 ****/
pcie6509_error_t    pcie6509_device_error[]= {
    PCIE6509_DEFINE_ERR(PCIE6509_SUCCESS,       "No error"),
    PCIE6509_DEFINE_ERR(PCIE6509_INVALID_PARAMETER, 
                    "Invalid Parameter or Argument Supplied"),
    PCIE6509_DEFINE_ERR(PCIE6509_TIMEOUT,       "Timeout Occurred"),
    PCIE6509_DEFINE_ERR(PCIE6509_OPERATION_CANCELLED, 
                                                "Operation was Cancelled"),
    PCIE6509_DEFINE_ERR(PCIE6509_RESOURCE_ALLOCATION_ERROR,
                                                "Failed to Acquire Resource"),
    PCIE6509_DEFINE_ERR(PCIE6509_INVALID_REQUEST,
                                                "Invalid Request"),
    PCIE6509_DEFINE_ERR(PCIE6509_FAULT_ERROR,   "A Fault was Detected"),
    PCIE6509_DEFINE_ERR(PCIE6509_BUSY,          "Operation is Busy"),
    PCIE6509_DEFINE_ERR(PCIE6509_ADDRESS_IN_USE,"Address Already in Use"),

    { 0,    NULL, NULL }    /* end of table */
};

#define get_user_ret(val, from, retval) {                   \
    if(access_ok(VERIFY_READ, from, sizeof(*(from)))) {     \
        __get_user(val, from);                              \
    }                                                       \
    else {                                                  \
        return(retval);                                     \
    }                                                       \
}

#define put_user_ret(val, to, retval) {                     \
    if(access_ok(VERIFY_WRITE, to, sizeof(*(to)))) {        \
        __put_user(val, to);                                \
    }                                                       \
    else {                                                  \
        return(retval);                                     \
    }                                                       \
}

#include <linux/interrupt.h>

#define MAX_BOARDS 10 /* maximum number of boards that driver supports. 
                         Arbitrarily chosen, change if you can actully 
                         support more. Various scripts will need to be
                         enhanced as well. */


#if !defined(U64)
typedef union _U64
{
    struct
    {
        u32  LowPart;
        u32  HighPart;
    }u;

    u64 QuadPart;
} U64;
#endif

enum {
    REGION_0 ,          /* Region  0 */
    REGION_1 ,          /* Region  1 */
    REGION_2 ,          /* Region  2 */
    REGION_3 ,          /* Region  3 */
    REGION_4 ,          /* Region  4 */
    REGION_5 ,          /* Region  5 */
    REGION_6 ,          /* Region  6 */
    REGION_7 ,          /* Region  7 */
    REGION_8 ,          /* Region  8 */
    REGION_9 ,          /* Region  9 */
    REGION_10,          /* Region 10 */
    REGION_11,          /* Region 11 */
    REGION_12,          /* Region 12 */
    REGION_13,          /* Region 13 */
    REGION_14,          /* Region 14 */
    REGION_15,          /* Region 15 */
    REGION_16,          /* Region 16 */
    REGION_17,          /* Region 17 */
};

/*** run_flags ***/
#define RF_OPEN_BUSY        0x00000001      /* open busy flag */
#define RF_READ_BUSY        0x00000002      /* read busy flag */
#define RF_WRITE_BUSY       0x00000004      /* write busy flag */
#define RF_INIT_PENDING     0x00000008      /* init pending */
#define RF_RESET_PENDING    0x00000010      /* reset pending */

#define RFLAG(Flag)        (pcie6509_device->run_flags & Flag)
#define RFLAG_CLEAR(Flag)  (pcie6509_device->run_flags &= ~Flag)
#define RFLAG_SET(Flag)    (pcie6509_device->run_flags |= Flag)
/*****************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
struct timeval {
    __kernel_old_time_t	tv_sec;		/* seconds */
	__kernel_suseconds_t	tv_usec;	/* microseconds */
};
#endif
/* card descriptor structure */
struct pcie6509_board {
    struct  timer_list     watchdog_timer;

    struct                 pci_dev *pdev;   /* the kernel PCI device descriptor */
    int                    open_cnt;        /* open count */
    int                    timeout;         /* timeout flag */
    int                    timeout_seconds; /* seconds to wait before timeout */
    int                    wakeup_pending;  /* interrupt pending */
    unsigned int           run_flags;       /* run time flags */
    int                    irqlevel;        /* interrupt level */
    int                    minor;           /* device minor number */
    int                    error;           /* driver error code */
    wait_queue_head_t      ioctlwq;         /* queue of tasks sleeping in ioctl */
    int                    board_index;

    spinlock_t             smp_lock;        /* lock for critical section in ISR*/
    unsigned long          smp_flags;

    struct pcie6509_board   *next;           /* next board in linked list */
    pcie6509_dev_region_t   mem_region[PCIE6509_MAX_REGION];
    unsigned long           mmap_reg_select; /* select LOCAL, CONFIG or PHYSMEM */
    pcie6509_dev_region_t   *local_region;   /* local region pointer */
    unsigned int            *phys_mem;      /* physical mem: physical address */
    unsigned int            *virt_mem;      /* physical mem: virtual address */
    unsigned int            phys_mem_size;  /* physical mem: memory size - bytes */
    unsigned int            phys_mem_size_freed; /* physical memory: freed size */
    unsigned int            *phys_shadow_reg;  /* physical shadow registers: physical address */
    pcie6509_shadow_regs_t  *shadow_reg;    /* physical shadow registers: virtual address */

    unsigned int            irq_added;      /* IRQ added flag */

    struct   timeval timestamp_read_start;  /* read timestamp start */
    struct   timeval timestamp_read_end;    /* read timestamp end */
    struct   timeval timestamp_write_start; /* write timestamp start */
    struct   timeval timestamp_write_end;   /* write timestamp end */

    wait_queue_head_t   intwq;              /* wait for interrupt */
    int                 wakeup;             /* wakeup flag */

    pcie6509_driver_info_t info;               /* driver/user information */
    int					ioctl_processing;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
        struct  mutex   ioctl_mtx;
#endif
};

/* non-static function prototypes */
int device_ioctl(struct inode *inode, struct file *fp, unsigned int num, 
                 unsigned long arg);
//static irqreturn_t device_interrupt(int , void *);

#define MSECS_TO_SLEEP(MS)      ((MS * HZ) / 1000)

#define PCIE6509_LOCK_INIT() { spin_lock_init(&pcie6509_device->smp_lock); }

#define PCIE6509_LOCK() {                                            \
    spin_lock_irqsave(&pcie6509_device->smp_lock, pcie6509_device->smp_flags);    \
}

#define PCIE6509_UNLOCK() {                                                    \
        spin_unlock_irqrestore(&pcie6509_device->smp_lock, pcie6509_device->smp_flags);    \
}

void writelocal(struct pcie6509_board *,unsigned value, unsigned address);
unsigned readlocal(struct pcie6509_board *,unsigned address);
static int device_mmap(struct file *, struct vm_area_struct *);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_device_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg);
#endif

/******************************************************************************
 * Driver Tokens                                                              *
 ******************************************************************************/
typedef struct {
    char    *token;
    int     which;
} _tokens_t;

/* token descriptions */
#define D_PCIE6509_DEBUG_MASK   "pcie6509_debug_mask"
#define D_PCIE6509_DEBUG_CTRL   "pcie6509_debug_ctrl"

/* << PLACE ADDITIONAL TOKENS HERE >> */

enum {
    T_PCIE6509_DEBUG_MASK,
    T_PCIE6509_DEBUG_CTRL,

    /* << PLACE ADDITIONAL TOKENS HERE >> */
};

_tokens_t Tokens[]= {
    {D_PCIE6509_DEBUG_MASK, T_PCIE6509_DEBUG_MASK},
    {D_PCIE6509_DEBUG_CTRL, T_PCIE6509_DEBUG_CTRL},

    /* << PLACE ADDITIONAL TOKENS HERE >> */

    {NULL,                  0},                 /* end-of-list */
};

#endif    /* __PCIE6509_H_INCLUDED__ */
