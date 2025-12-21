/*******************************************************************************
**
**  Module  : tsync_drv_2_6_13_1.c
**  Date    : 03/31/06
**  Purpose : This driver provides an interface to the TPRO-PCI
**
**  Copyright(C) 2006 Spectracom Corporation. All Rights Reserved.
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
********************************************************************************
**
**     05/28/07 Modifications for multi-processor operation.
**     01/23/07 Added support for multiple simultaneous users
**     03/31/06 Updated from Linux (2.4.24) to Linux (2.6.11.12)
**
*******************************************************************************/
#ifndef NIOS

/*******************************************************************************
**          Includes
*******************************************************************************/
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>

#include <linux/sched.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "kernel_compat.h"
#include "tsyncpci.h"
#include "ddtsync.h"
#include "ddtpro.h"
#include "tpro_func.h"
#include "os_wait.h"
#include "tsync_gpio_stamp_queue.h"
#include "tsync_hw_func.h"
#include "tsync_pps.h"
#include "tsync_ptp.h"
#include "tsync_ptp_host_reference.h"

/*******************************************************************************
**          Defines
*******************************************************************************/
#ifndef DRIVER_VERSION
#define DRIVER_VERSION "9.9.9"
#endif
#define IO_BASE_ADDR_REG  (3)
#define IO_BASE_ADDR_REG_32  (1)
#define MEMORY_BASE_ADDR_REG_TSYNC (0)
#define CLASS_NAME        "tsyncpci"

/*******************************************************************************
**          Kernel Module Description
*******************************************************************************/
MODULE_AUTHOR("Spectracom Corporation (spectracom.com)");
MODULE_DESCRIPTION("Spectracom TSync Timing Board");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

/*******************************************************************************
**          Brief Identification Strings
*******************************************************************************/
char tsyncpci_driver_name[]    = TSYNC_PCI_DRV_NAME;
char tsyncpci_driver_string[]  = TSYNC_PCI_DRV_STRING;
char tsyncpci_driver_version[] = DRIVER_VERSION;
char tsyncpci_copyright[]      = TSYNC_PCI_DRV_COPYRIGHT;

/*******************************************************************************
**          Local Function Prototypes
*******************************************************************************/

static void           tsyncpci_cleanup     (void);

static int            tsyncpci_close       (struct inode *inode,
                                           struct file *filp);

static tsyncpci_dev_t* tsyncpci_find_device (struct inode *inode);

static int            tsyncpci_init        (void);

static int            tsyncpci_init_one    (struct pci_dev             *pdev,
                                           const struct pci_device_id *ent);

static long            tsyncpci_ioctl      (struct file   *filp,
                                           unsigned int  cmd,
                                           unsigned long arg);
static int            tsyncpci_open        (struct inode *inode,
                                           struct file *filp);

static void           tsyncpci_remove_one  (struct pci_dev *pdev);

static irqreturn_t    tpropci_isr(int irq, void *dev_id);
static irqreturn_t    tsync_isr(int irq, void *dev_id);

/*******************************************************************************
**          Typedef's and Structures
*******************************************************************************/

/*
** PCI Device table
*/
static struct pci_device_id tsyncpci_pci_tbl[] __refdata =
{
//    {
//        vendor:      TPRO_VENDOR_ID,
//        device:      TPRO_5V_DEVICE_ID,
//        subvendor:   TPRO_SUBSYSTEM_VENDOR_ID,
//        subdevice:   PCI_ANY_ID,
//        class:       0,
//        class_mask:  0,
//        driver_data: 0
//    },
//    {
//        vendor:      TPRO_VENDOR_ID,
//        device:      TPRO_3V_DEVICE_ID,
//        subvendor:   TPRO_SUBSYSTEM_VENDOR_ID,
//        subdevice:   PCI_ANY_ID,
//        class:       0,
//        class_mask:  0,
//        driver_data: 0
//    },
//    {
//        vendor:      TPRO_U66_VENDOR_ID,
//        device:      TPRO_U66_DEVICE_ID,
//        subvendor:   TPRO_U66_SUBSYSTEM_VENDOR_ID,
//        subdevice:   PCI_ANY_ID,
//        class:       0,
//        class_mask:  0,
//        driver_data: 0
//    },
    {
        vendor:      TSYNC_VENDOR_ID,
        device:      TSYNC_DEVID,
        subvendor:   TSYNC_VENDOR_ID,
        subdevice:   PCI_ANY_ID,
        class:       0,
        class_mask:  0,
        driver_data: 0
    },
    {0, }
};

MODULE_DEVICE_TABLE(pci, tsyncpci_pci_tbl);

/*
**  tsyncpci_driver PCI driver information structure
*/
static struct pci_driver tsyncpci_driver =
{
    name:     TSYNC_PCI_DRV_NAME,
    id_table: tsyncpci_pci_tbl,
    probe:    tsyncpci_init_one,
    remove:   tsyncpci_remove_one,
    shutdown: tsyncpci_remove_one,
};

/*
** Driver setup structure
*/
static struct file_operations tsyncpci_fops =
{
    owner:   THIS_MODULE,
    unlocked_ioctl:   tsyncpci_ioctl,
    compat_ioctl:     tsyncpci_ioctl,
    open:    tsyncpci_open,
    release: tsyncpci_close
};

static struct class         *tsyncpci_cs;
static struct device        *tsyncpci_cd[TSYNC_PCI_MAX_NUM];
static struct cdev           tsyncpci_cdev;     /* Character dev struct.     */
static        dev_t          tsyncpci_devNum;   /* Driver's base Dev Number  */
static        tsyncpci_dev_t *tsyncpci_devp[TSYNC_PCI_MAX_NUM];
static        char           NameStr[TSYNC_PCI_MAX_NUM][32];

// Define the 16 register interface functions
extern FuncTable_t tsyncpci_16bit;

// Define the 32 register interface functions
extern FuncTable_t tsyncpci_32bit;

// Define the TSync register interface functions
extern FuncTable_t tsyncpci_tsync;


/*******************************************************************************
**
** Function: tsyncpci_cleanup()
**
** Description: Routine called just before the driver is removed from memory.
**
** Parameters:
**     IN: None
**
**     RETURNS: None
**
*******************************************************************************/
static void tsyncpci_cleanup(void)
{
    /* Unregister the driver with the PCI subsystem */
    pci_unregister_driver(&tsyncpci_driver);

    cdev_del(&tsyncpci_cdev);
    unregister_chrdev_region(tsyncpci_devNum, TSYNC_PCI_MAX_NUM);

    class_destroy(tsyncpci_cs);

    TPRO_PRINT("%s removed\n", tsyncpci_driver_string);

} /* End - tsyncpci_cleanup() */

/*******************************************************************************
**
** Function: tsyncpci_close()
**
** Description: Close a tsyncpci device
**
** Parameters:
**     IN: *inode - inode pointer
**         *filp  - file pointer
**
**     RETURNS: -EIO - Couldn't find matching tsyncpci device
**                 0 - Success
**
*******************************************************************************/
static int tsyncpci_close(struct inode *inode, struct file *filp)
{
    tsyncpci_user_t * user = NULL;
    tsyncpci_dev_t *  hw = NULL;
    int status = 0;

    /* Retrieve the user structure from the file pointer. */
    user = (tsyncpci_user_t *)(filp->private_data);

    if (user == NULL)
    {
        status = -EFAULT;
        TPRO_LOG_ERR;
        goto fxnExit;
    }

    /* Retrieve the device pointer from the user structure. */
    hw = user->pDevice;
    if (hw == NULL)
    {
        status = -EFAULT;
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: Null device pointer in user struct %p!!!\n",
                  jiffies, __FUNCTION__, user));
    }

#ifdef TPRO_CHECK
    /* Retrieve the device pointer from the inode and verify. */
    if (hw != tsyncpci_find_device(inode))
    {
        status = -EFAULT;
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: Device pointer mismatch"
                  " in user struct %p!!!\n",
                  jiffies, __FUNCTION__, user));
        goto fxnExit;
    }
#endif

    /*
    ** Remove the user structure from the list and deallocate it.
    ** If this is the only user, shut down the device.
    */

    TPRO_SPIN_LOCK(&hw->userLock);

#ifdef TPRO_CHECK
    /* Make sure the first and last user pointers are not NULL. */
    if ((hw->pFirstUser == NULL) || (hw->pLastUser == NULL))
    {
        TPRO_SPIN_UNLOCK(&hw->userLock);
        status = -EFAULT;
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: hw->pFirstUser=%p, hw->pLastUser=%p in"
                  " device struct %p!!!\n",
                 jiffies, __FUNCTION__, hw->pFirstUser, hw->pLastUser,
                 user));
        goto fxnExit;
    }
#endif

    if (user == hw->pFirstUser)
    {
        if (user == hw->pLastUser)
        {
            /* This is the only user. */
#ifdef TPRO_CHECK
            if ((user->pPrev != user) || (user->pNext != user))
            {
                TPRO_LOG(TPRO_LOG_ERROR,
                         ("[J%lu]%s: Only user is %p but pPrev = %p"
                          " and pNext = %p",
                          jiffies, __FUNCTION__, user, user->pPrev,
                          user->pNext));
            }
#endif
            hw->pFirstUser = NULL;
            hw->pLastUser = NULL;
        }
        else
        {
            /* This is the first, but not the only, user. */
            user->pPrev->pNext = hw->pFirstUser = user->pNext;
            user->pNext->pPrev = user->pPrev;
        }
    }
    else if (user == hw->pLastUser)
    {
        /* This user was added last and is not the only user. */
        user->pNext->pPrev = hw->pLastUser = user->pPrev;
        user->pPrev->pNext = user->pNext;
    }
    else
    {
        /* Neither first nor last user.  Remove from the middle of the list. */
        user->pNext->pPrev = user->pPrev;
        user->pPrev->pNext = user->pNext;
    }

    TPRO_SPIN_UNLOCK(&hw->userLock);

    /* Free the user structure. */
    kfree(user);

    TPRO_LOG(TPRO_LOG_CLOSE,
             ("[J%lu]%s: Closed user struct %p\n",
              jiffies, __FUNCTION__, user));

 fxnExit:
    return status;

} /* End - tsyncpci_close() */

/*******************************************************************************
**
** Function: tsyncpci_find_device()
**
** Description: Determines which tsyncpci_dev_t stucture the inode is referring
**              to and return a pointer to it.  Returns NULL if no matching
**              tsyncpci_dev_t could be found.
**
** Parameters:
**     IN: *inode - info to match with a tsyncpci_dev_t sturcture
**
**     RETURNS: tsyncpci_dev_t* - NULL if none is found.
**
*******************************************************************************/
static tsyncpci_dev_t* tsyncpci_find_device(struct inode *inode)
{
    tsyncpci_dev_t *retDevp = NULL;
    unsigned int   idx;

    TPRO_LOG(TPRO_LOG_FIND_DEV,
             ("[J%lu]%s: Finding device, INODE=%p, DEVNUM=0x%08X\n",
              jiffies, __FUNCTION__, inode, inode->i_rdev));

    for (idx = 0; idx < TSYNC_PCI_MAX_NUM; idx++)
    {
        if (tsyncpci_devp[idx] != NULL)
        {
            if (tsyncpci_devp[idx]->devNum == inode->i_rdev)
            {
                retDevp = tsyncpci_devp[idx];
            }
        }
    }

    return retDevp;

} /* End - tsyncpci_find_device() */

/*******************************************************************************
**
** Function: tsyncpci_init()
**
** Description: TPRO kernel module entry point
**
** Parameters:
**     IN: None
**
**     RETURNS: 0 on success, or negative error code.
**
*******************************************************************************/
static int tsyncpci_init(void)
{
    int rv;

    /* Print the driver ID strings. */
    TPRO_PRINT("%s - version %s\n%s\n",
               tsyncpci_driver_string,
               tsyncpci_driver_version,
               tsyncpci_copyright);

    /*
    ** Create class
    */
    tsyncpci_cs = TSYNC_CLASS_CREATE(THIS_MODULE, TSYNC_PCI_DRV_NAME);

    if (IS_ERR(tsyncpci_cs))
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ERR "[J%lu]%s: Error creating sysfs class.\n",
                  jiffies, __FUNCTION__));
        return PTR_ERR(tsyncpci_cs);
    }

    /*
    ** Allocate character device region
    */
    rv = alloc_chrdev_region(&tsyncpci_devNum,
                             TSYNC_PCI_BASE_MINOR,
                             TSYNC_PCI_MAX_NUM,
                             TSYNC_PCI_DRV_NAME);

    if (rv)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ALERT "[J%lu]%s: FAILED alloc_chrdev_region() err=%d\n",
                  jiffies, __FUNCTION__, rv));
        class_destroy(tsyncpci_cs);
        return rv;
    }

    /*
    ** Initialize char device structure
    */
    cdev_init(&tsyncpci_cdev, &tsyncpci_fops);

    tsyncpci_cdev.owner = THIS_MODULE;
    tsyncpci_cdev.ops   = &tsyncpci_fops;

    /*
    ** Add the char device to the system
    */
    rv = cdev_add(&tsyncpci_cdev, tsyncpci_devNum, TSYNC_PCI_MAX_NUM);

    if (rv)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ERR "[J%lu]%s: FAILED cdev_add()\n",
                 jiffies, __FUNCTION__));
        unregister_chrdev_region(tsyncpci_devNum, TSYNC_PCI_MAX_NUM);
        class_destroy(tsyncpci_cs);
        return rv;
    }

    /*
    **  register the driver with the PCI subsytem
    */
    rv = pci_register_driver(&tsyncpci_driver);

    if (rv) {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ERR "[J%lu]%s: pci_register_driver returned %d\n",
                  jiffies, __FUNCTION__, rv));
    }

    return rv;

} /* End - tsyncpci_init() */

/*******************************************************************************
**
** Function: tsyncpci_init_one()
**
** Description: Initialize new device to be handled by this driver
**
** Parameters:
**     IN: *pdev - PCI device structure pointer
**         *ent  - PCI device ID pointer
**
**     RETURNS: 0 - Success, <0 - Failure
**
*******************************************************************************/
static int tsyncpci_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int          rv = 0;
    unsigned int idx,temp;
    unsigned char initBoardResult;
	unsigned long irq_flags = 0;

    TPRO_LOG(TPRO_LOG_LOAD,
             ("[J%lu]%s\n",
              jiffies, __FUNCTION__));

    /* Find the next available device pointer... */
    for (idx = 0; idx < TSYNC_PCI_MAX_NUM; idx++)
    {
        /* If the currently indexed pointer hasn't been allocated yet... */
        if (tsyncpci_devp[idx] == NULL)
        {
            /* Have found available device pointer */
            break;
        }
    }

    /* If all device pointers have been already allocated... */
    if (idx >= TSYNC_PCI_MAX_NUM)
    {
        TPRO_LOG(TPRO_LOG_LOAD,
                 (KERN_ALERT "[J%lu]%s, devices already allocated\n",
                  jiffies, __FUNCTION__));
        return -ENOSPC;
    }

    /* Allocate device structure for new device */
    tsyncpci_devp[idx] = kmalloc(sizeof(tsyncpci_dev_t), GFP_KERNEL);

    /* If memory couldn't be allocated... */
    if (tsyncpci_devp[idx] == NULL)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ALERT "[J%lu]%s: No memory\n", jiffies, __FUNCTION__));
        return -ENOMEM;
    }

    /*
    **  prepare device driver access context
    */
    tsyncpci_devp[idx]->intCnt     = 0;
    tsyncpci_devp[idx]->pFirstUser = NULL;
    tsyncpci_devp[idx]->pLastUser  = NULL;
    tsyncpci_devp[idx]->headIndex = 0;

    /*
    **  preclear interrupt info to zero
    */
    for (temp = 0; temp < TSYNC_INTERRUPT_COUNT; temp++) {
        tsyncpci_devp[idx]->intCounter[temp]      = 0;
        tsyncpci_devp[idx]->intTime[temp].seconds = 0;
        tsyncpci_devp[idx]->intTime[temp].ns      = 0;
    }

    TPRO_MUTEX_INIT(&tsyncpci_devp[idx]->deviceLock);
    TPRO_SPIN_LOCK_INIT(&tsyncpci_devp[idx]->userLock);

    /*
    **  set PCI configuration register to enable PCI I/O and Memory access
    */
    rv = pci_enable_device(pdev);
    if (rv) {
        if (tsyncpci_devp[idx]) {
            kfree(tsyncpci_devp[idx]);
            tsyncpci_devp[idx] = NULL;
        }
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ALERT "[J%lu]%s: pci_enable_device()\n",
                 jiffies, __FUNCTION__));
        return -EIO;
    }

    /*
    **  get base address of device and amount of I/O space requested
    */
    if (pdev->device == TSYNC_DEVID) {
        unsigned long phys_addr =
            pci_resource_start(pdev, MEMORY_BASE_ADDR_REG_TSYNC);
        tsyncpci_devp[idx]->ioSize  = pci_resource_len(pdev,   MEMORY_BASE_ADDR_REG_TSYNC);
        tsyncpci_devp[idx]->ioAddr  = (unsigned long) ioremap(phys_addr, tsyncpci_devp[idx]->ioSize);
    }
    else if (pdev->device == TPRO_U66_DEVICE_ID) {
        tsyncpci_devp[idx]->ioAddr  = pci_resource_start(pdev, IO_BASE_ADDR_REG_32);
        tsyncpci_devp[idx]->ioSize  = pci_resource_len(pdev,   IO_BASE_ADDR_REG_32);
    }
    else {
        tsyncpci_devp[idx]->ioAddr  = pci_resource_start(pdev, IO_BASE_ADDR_REG);
        tsyncpci_devp[idx]->ioSize  = pci_resource_len(pdev,   IO_BASE_ADDR_REG);
    }

    tsyncpci_devp[idx]->device  = pdev->device;
    tsyncpci_devp[idx]->options = pdev->subsystem_device;
    tsyncpci_devp[idx]->irq     = pdev->irq;
    tsyncpci_devp[idx]->devNum  = MKDEV(MAJOR(tsyncpci_devNum), idx);

    sprintf(NameStr[idx], "tsyncpci%d", idx);

    /* Select the function set based on the board type. */
    /* The selection is based on the Device ID, it is one value */
    /* for the 32 bit version of the boards and one of two */
    /* for the 16 bit boards. */
    if (pdev->device == TSYNC_DEVID) {
        TPRO_PRINT("TSYNC version of board: %s\n", NameStr[idx]);
        tsyncpci_devp[idx]->pFunctionTable = &tsyncpci_tsync;
    } else if (pdev->device == TPRO_U66_DEVICE_ID)
    {
        TPRO_PRINT("TPro_PCI 32 bit version of board: %s\n", NameStr[idx]);
        tsyncpci_devp[idx]->pFunctionTable = &tsyncpci_32bit;
    }
    else {
        TPRO_PRINT("TPro_PCI 16 bit version of board: %s\n", NameStr[idx]);
        tsyncpci_devp[idx]->pFunctionTable = &tsyncpci_16bit;
    }

    /*
    ** Create class device
    */
    tsyncpci_cd[idx] = device_create(tsyncpci_cs, NULL,
                                           MKDEV(MAJOR(tsyncpci_devNum), idx),
                                           NULL, NameStr[idx]);

    if (IS_ERR(tsyncpci_cd[idx]))
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ALERT "[J%lu]%s: device creation failed\n",
                 jiffies, __FUNCTION__));
        return PTR_ERR(tsyncpci_cd[idx]);
    }

    sprintf(NameStr[idx], "tsyncpci%d", MINOR(tsyncpci_devp[idx]->devNum));

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
    irq_flags |= IRQF_DISABLED;
    irq_flags |= IRQF_SHARED;
#else
    // description: https://github.com/torvalds/linux/commit/d8bf368d0631d4bc2612d8bf2e4e8e74e620d0cc
    irq_flags |= IRQF_SHARED;
#endif

    /* Map device IRQ */
    if (pdev->device == TSYNC_DEVID) {
        rv = request_irq(tsyncpci_devp[idx]->irq,
                         tsync_isr, irq_flags,
                         NameStr[idx],
                         tsyncpci_devp[idx]);
    }
    else {
        rv = request_irq(tsyncpci_devp[idx]->irq,
                         tpropci_isr, irq_flags,
                         NameStr[idx],
                         tsyncpci_devp[idx]);
    }

    if (rv != 0)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: Could NOT map IRQ!!!\n",
                  jiffies, __FUNCTION__));
        return -EIO;
    }

    /* Initialize the board.  Used to be done in IOCTL. */
    initBoardResult =
        tsyncpci_devp[idx]->pFunctionTable->InitializeBoard(tsyncpci_devp[idx]);
    if (initBoardResult != 0) {
        TPRO_LOG(TPRO_LOG_ERROR,
             ("[J%lu]%s: Board Initialization failed.\n",
                 jiffies, __FUNCTION__));
        return 1;
    }

    tsyncpci_devp[idx]->slotPosition = (unsigned char)idx;

    rv = tsync_pps_register(tsyncpci_devp[idx]);
    if (rv != 0)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
             ("[J%lu]%s: Failed to register PPS.\n",
                 jiffies, __FUNCTION__));
        return rv;
    }

    rv = tsync_ptp_register(tsyncpci_devp[idx]);
    if (rv != 0)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
             ("[J%lu]%s: Failed to register PTP device.\n",
                 jiffies, __FUNCTION__));
        return rv;
    }

    rv = tsync_ptp_host_reference_register(tsyncpci_devp[idx], tsyncpci_cd[idx]);
    if (rv != 0)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
             ("[J%lu]%s: Failed to register Host Reference device.\n",
                 jiffies, __FUNCTION__));
        return rv;
    }
    return 0;

} /* End - tsyncpci_init_one() */

/*******************************************************************************
**
** Function: tsyncpci_ioctl()
**
** Description: Called when an ioctl operation is requested.
**
** Parameters:
**     IN: *inode - inode struct for the device
**         *filp  - file struct for the device
**          cmd   - the ioctl command
**          arg   - typecast for the arg, can be a value/ptr
**
**     RETURNS: Negative on failure or the ioctl call return value (if any)
**
*******************************************************************************/
static long tsyncpci_ioctl(struct file   *filp,
                         unsigned int  cmd,
                         unsigned long arg)
{
    tsyncpci_ioctl_cmd_t *cmdHnd     = (tsyncpci_ioctl_cmd_t*)arg;
    tsyncpci_dev_t       *hw         = NULL;
    tsyncpci_user_t      *user       = NULL;
    tsyncpci_user_t      *currUser   = NULL;
    long                  status     = 0;

    /* Validate the command. */
    if (_IOC_NR(cmd) >= IOCTL_TPRO_CMD_COUNT)
    {
        status = -EFAULT;
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: [CMD:0x%08X, USER:%p]: Invalid IOCTL command\n",
                 jiffies, __FUNCTION__, cmd, user));
        goto fxnExit;
    }

    /* Retrieve the user structure from the file pointer. */
    user = (tsyncpci_user_t *)(filp->private_data);
    if (user == NULL)
    {
        status = -EFAULT;
        TPRO_LOG_ERR;
        goto fxnExit;
    }

    TPRO_LOG(TPRO_LOG_IOCTL,
             ("[J%lu]%s: [CMD:0x%08X, USER:%p, HW:%p]\n",
             jiffies, __FUNCTION__, cmd, user, user->pDevice));

    /* Retrieve the device pointer from the user structure. */
    hw = user->pDevice;
    if (hw == NULL)
    {
        status = -EFAULT;
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: [CMD:0x%08X, USER:%p]: Null device pointer"
                  " in user struct!!!\n",
                  jiffies, __FUNCTION__, cmd, user));
        goto fxnExit;
    }

    /* Verify access to the command object (read/write) */
    if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)cmdHnd, sizeof(tsyncpci_ioctl_cmd_t)))
    {
        status = -EFAULT;
        goto fxnExit;
    }

    /* Make sure the call is currently available to the user. */
    if ((cmd == IOCTL_TPRO_OPEN) || (cmd == IOCTL_TSYNC_WAIT))
    {
        /* These commands are always available. */
    }
    else
    {
        int inUse;

        /* These commands are available to only one user at a time. */
        TPRO_SPIN_LOCK(&hw->userLock);
        currUser = (currUser == NULL) ? user : currUser;
        inUse    = (currUser == user) ? 1 : 0;
        TPRO_SPIN_UNLOCK(&hw->userLock);
        if (inUse == 0)
        {
            status = -EFAULT;
            TPRO_LOG(TPRO_LOG_INFO_1,
                     ("[J%lu]%s: [CMD:0x%08X, USER:%p, HW:%p]"
                      ": Multiuser conflict!!!\n",
                      jiffies, __FUNCTION__, cmd, user, user->pDevice));
            goto fxnExit;
        }
    }

    switch (cmd)
    {
    case IOCTL_TPRO_OPEN:
    {
        BoardObj board;

        /* verify access to board object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(BoardObj))) {
            status = -EFAULT;
            goto fxnExit;
        }

        /* copy object from user space */
        if (copy_from_user(&board, &cmdHnd->argVec[0], sizeof (BoardObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* copy board options to board object */
        board.options = hw->options;

        /* copy local object to user object */
        if (copy_to_user(&cmdHnd->argVec[0], &board, sizeof (BoardObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_ALTITUDE:
    {
        unsigned char getAltResult;
        AltObj drvAlt;

        /* verify access to altitude object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(AltObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get altitude from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getAltResult = hw->pFunctionTable->getAltitude(hw, &drvAlt);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getAltResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvAlt, sizeof (AltObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_DATE:
    {
        DateObj drvDate;
        unsigned char getDateResult;

        /* verify access to date object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(DateObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get date from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getDateResult = hw->pFunctionTable->getDate(hw, &drvDate);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getDateResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvDate, sizeof(DateObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_DRIVER:
    {
        char version[7];

        /* verify access to version string (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(version)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* copy version to local array */
        strcpy(version, DRIVER_VERSION);

        /* copy string to user space */
        if (copy_to_user(&cmdHnd->argVec[0], version, sizeof (version)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_FIRMWARE:
    {
        unsigned char firmware[5];
        unsigned char getFirmwareResult;

        /* verify access to firmware bytes (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(firmware)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get firmware from the tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getFirmwareResult = hw->pFunctionTable->getFirmware(hw, firmware);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getFirmwareResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy string to user space */
        if (copy_to_user(&cmdHnd->argVec[0], firmware, sizeof (firmware)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_LATITUDE:
    {
        LatObj drvLat;
        unsigned char getLatitudeResult;

        /* verify access to latitude object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(LatObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get latitude from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getLatitudeResult = hw->pFunctionTable->getLatitude(hw, &drvLat);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getLatitudeResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvLat, sizeof (LatObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_LONGITUDE:
    {
        LongObj drvLong;
        unsigned char getLongitudeResult;

        /* verify access to longitude object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof (LongObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get longitude from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getLongitudeResult = hw->pFunctionTable->getLongitude(hw, &drvLong);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getLongitudeResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvLong, sizeof (LongObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_SAT_INFO:
    {
        SatObj drvSat;
        unsigned char getSatInfoResult;

        /* verify access to satellite info object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof (SatObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get satellite info from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getSatInfoResult = hw->pFunctionTable->getSatInfo(hw, &drvSat);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getSatInfoResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvSat, sizeof (SatObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_TIME:
    {
        TimeObj drvTime;

        /* verify access to time object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof (TimeObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get time from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->getTime(hw, &drvTime);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvTime, sizeof (TimeObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_RESET_FIRMWARE:
    {
        unsigned char resetResult;
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        resetResult = hw->pFunctionTable->resetFirmware(hw);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (resetResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SET_HEARTBEAT:
    {
        HeartObj drvHeart;
        unsigned char setHeartResult;

        /* copy object from user space */
        if (copy_from_user(&drvHeart, &cmdHnd->argVec [0], sizeof (HeartObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* set heartbeat on tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setHeartResult = hw->pFunctionTable->setHeartbeat(hw, &drvHeart);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setHeartResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        break;
    }

    case IOCTL_TPRO_SET_MATCHTIME:
    {
        MatchObj drvMatch;
        unsigned char setMatchTimeResult;

        /* copy object from user space */
        if (copy_from_user(&drvMatch, &cmdHnd->argVec[0], sizeof (MatchObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* set match time on tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setMatchTimeResult =
            hw->pFunctionTable->setMatchTime(hw, &drvMatch);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setMatchTimeResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SET_PROP_DELAY_CORR:
    {
        int microseconds;
        unsigned char setPropResult;

        /* If parameter is NULL, just return success. */
        if (cmdHnd == NULL)
        {
            break;
        }

        /* get microsecond value from user */
        __get_user(microseconds, (int *) &cmdHnd->argVec[0]);

        /* set propagation delay correction value on tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setPropResult = hw->pFunctionTable->setPropDelayCorr(hw, &microseconds);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setPropResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SET_TIME:
    {
        TimeObj drvTime;
        unsigned char setTimeStatus;

        /* copy object from user space */
        if (copy_from_user(&drvTime, &cmdHnd->argVec [0], sizeof (TimeObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* set time on tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setTimeStatus = hw->pFunctionTable->setTime(hw, &drvTime);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setTimeStatus != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SET_YEAR:
    {
        unsigned short year;
        unsigned char setYearStatus;

        /* get year value from user */
        __get_user(year, (unsigned short *) &cmdHnd->argVec[0]);

        /* set year to tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setYearStatus = hw->pFunctionTable->setYear(hw, &year);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setYearStatus != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SIM_EVENT:
    {
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->simEvent(hw);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);
        break;
    }

    case IOCTL_TPRO_SYNCH_CONTROL:
    {
        int ctrl;
        unsigned char synchControlResult;

        /* get control value from user */
        __get_user(ctrl, (int *) &cmdHnd->argVec[0]);

        /* set synchronization control value to tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        synchControlResult =
            hw->pFunctionTable->synchControl(hw, (unsigned char*)&ctrl);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (synchControlResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_SYNCH_STATUS:
    {
        unsigned char stat = 0xFF;
        unsigned char synchControlResult;

        /* verify access to synch status character (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof (unsigned char)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get synchronization status from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        synchControlResult =
            hw->pFunctionTable->synchStatus(hw, &stat);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (synchControlResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy status to user-space */
        if (copy_to_user(&cmdHnd->argVec[0], &stat, sizeof (unsigned char)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_WAIT_EVENT:
    {
        WaitObj drvWaitEvent;

        unsigned int waitRequired = 0;
        unsigned char getEventResult;

        /* verify access to wait object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof (WaitObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* copy object from user space */
        if (copy_from_user(&drvWaitEvent,
                           &cmdHnd->argVec[0],
                           sizeof (WaitObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, FIFO_IRQ_ENABLE);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (hw->device == TSYNC_DEVID) {
            unsigned int count;
            gpioQueueCount(hw, 0, &count);

            if (count == 0) {
                waitRequired = 1;
            }
        }
        else {
            waitRequired = (hw->headIndex == hw->tailIndex);
        }

        if (waitRequired) {
            /* wait for event if necessary */
            if (hw->headIndex == hw->tailIndex)
                {
                    /* clear flag condition */
                    atomic_set (&hw->eventFlag, 0);

                    /* wait on semaphore */
                    status = os_waitPend(&hw->event_wait,
                                         drvWaitEvent.jiffies,
                                         &hw->eventFlag);

                    /* return on timeout or waitaphore error */
                    if (status == OS_WAIT_STATUS_TIMEOUT)
                        {
                            TPRO_MUTEX_LOCK(&hw->deviceLock);
                            (void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
                            TPRO_MUTEX_UNLOCK(&hw->deviceLock);
                            return 1;
                        }

                    if (status == OS_WAIT_STATUS_NG)
                        {
                            TPRO_MUTEX_LOCK(&hw->deviceLock);
                            (void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
                            TPRO_MUTEX_UNLOCK(&hw->deviceLock);
                            return -1;
                        }
                }
        }

        /* read event buffer into object */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
        getEventResult =
            hw->pFunctionTable->getEvent(hw, &drvWaitEvent);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getEventResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy object back to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &drvWaitEvent, sizeof (WaitObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_WAIT_HEART:
    {
        int ticks;

        /* get microsecond value from user */
        __get_user(ticks, (int *) &cmdHnd->argVec[0]);

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, HEART_IRQ_ENABLE);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        /* clear flag condition */
        atomic_set(&hw->heartFlag, 0);

        /* wait on semaphore */
        status = os_waitPend(&hw->heart_wait, ticks, &hw->heartFlag);

        if (status == OS_WAIT_STATUS_TIMEOUT) status = 1;
        if (status == OS_WAIT_STATUS_NG) status = -1;

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);
        break;
    }

    case IOCTL_TPRO_WAIT_MATCH:
    {
        int ticks;

        /* get microsecond value from user */
        __get_user(ticks, (int *) &cmdHnd->argVec[0]);

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, MATCH_IRQ_ENABLE);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        /* clear flag condition */
        atomic_set(&hw->matchFlag, 0);

        /* wait on semaphore */
        status = os_waitPend(&hw->match_wait, ticks, &hw->matchFlag);

        if (status == OS_WAIT_STATUS_TIMEOUT) status = 1;
        if (status == OS_WAIT_STATUS_NG) status = -1;

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);
        break;
    }

    case IOCTL_TPRO_PEEK:
    {
        MemObj Reg;

        /* verify access to memory object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(MemObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* copy object from user space */
        if (copy_from_user(&Reg, &cmdHnd->argVec [0], sizeof (MemObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get call the peek routine */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->peek(hw, &Reg);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        /* copy object back to user space */
        if (copy_to_user(&cmdHnd->argVec[0], &Reg, sizeof (MemObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_POKE:
    {
        MemObj Reg;

        /* copy object from user space */
        if (copy_from_user(&Reg, &cmdHnd->argVec [0], sizeof (MemObj)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* call the poke routine */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        (void)hw->pFunctionTable->poke(hw, &Reg);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);
        break;
    }

    case IOCTL_TPRO_GET_NTP_TIME:
#ifdef CONFIG_X86_64
    case IOCTL_TPRO_GET_NTP_TIME_COMPAT:
#endif
    {
        NtpTimeObj ntpTime;
        NtpTimeObj_Compat ntpTime_Compat;
        unsigned char getNTPResult;
        unsigned int struct_size;
        void *data;
        bool compat = (cmd == IOCTL_TPRO_GET_NTP_TIME_COMPAT);

        if (!compat) {
                struct_size = sizeof(ntpTime);
                data = &ntpTime;
        } else {
                struct_size = sizeof(ntpTime_Compat);
                data = &ntpTime_Compat;
        }

        /* verify access to NTP time object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       struct_size))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get time from tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getNTPResult =
            hw->pFunctionTable->getNtpTime(hw, &ntpTime);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getNTPResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

	/* If subsystem device id is a TPRO family reference */
        if (hw->options < TSAT_PCI)
        {
            memcpy(&ntpTime.refId, "IRIG", 4);
        }
        else /* Else, TSAT reference */
        {
            memcpy(&ntpTime.refId, "GPS ", 4);
        }

        /*
         * WARNING: This effectively will cause the 2038 bug on 64-bit systems
         * using an 32-bit userspace.
         */
        if (compat) {
                ntpTime_Compat.tv.tv_sec = ntpTime.tv.tv_sec;
                ntpTime_Compat.tv.tv_usec = ntpTime.tv.tv_usec;
                ntpTime_Compat.refId = ntpTime.refId;
                memcpy(&ntpTime_Compat.timeObj, &ntpTime.timeObj, sizeof(TimeObj));
        }

        /* copy object to user space */
        if (copy_to_user(&cmdHnd->argVec[0], data, struct_size))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TPRO_GET_FPGA:
    {
        unsigned char fpgaVer[6];
        unsigned char getFPGAResult;

        /* verify access to firmware bytes (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(fpgaVer)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

        /* get firmware from the tpro card */
        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getFPGAResult =
            hw->pFunctionTable->getFpgaVersion(hw, fpgaVer);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getFPGAResult != 0) {
            status = -EINVAL;
            goto fxnExit;
        }

        /* copy string to user space */
        if (copy_to_user(&cmdHnd->argVec[0], fpgaVer, sizeof (fpgaVer)))
        {
            status = -EFAULT;
            goto fxnExit;
        }
        break;
    }

    case IOCTL_TSYNC_GET:
    {
        int getStatus;

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        getStatus = hw->pFunctionTable->get(hw, &cmdHnd->argVec[0]);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (getStatus != 0) {
            status = -EFAULT;
            goto fxnExit;
        }

    }
    break;

    case IOCTL_TSYNC_SET:
    {
        int setStatus;

        TPRO_MUTEX_LOCK(&hw->deviceLock);
        setStatus = hw->pFunctionTable->set(hw, &cmdHnd->argVec[0]);
        TPRO_MUTEX_UNLOCK(&hw->deviceLock);

        if (setStatus != 0) {
            status = -EFAULT;
            goto fxnExit;
        }
    }
    break;

    case IOCTL_TSYNC_WAIT:
    {
        ioctl_trans_di_wait transaction;

        /* verify access to date object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(ioctl_trans_di_wait))) {
            status = -EFAULT;
            goto fxnExit;
        }

        if (copy_from_user(&transaction, &cmdHnd->argVec[0],
                           sizeof (ioctl_trans_di_wait))) {
            status = -EFAULT;
            goto fxnExit;
        }

        status = hw->pFunctionTable->wait(hw, &transaction);

        /* copy local object back to user object */
        if (copy_to_user(&cmdHnd->argVec[0], &transaction,
                         sizeof (ioctl_trans_di_wait)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

    }
    break;

    case IOCTL_TSYNC_WAIT_TO:
    {
        ioctl_trans_di_wait_to transaction;

        /* verify access to date object (write) */
        if (!TSYNC_ACCESS_OK(VERIFY_WRITE,
                       (void*)&cmdHnd->argVec[0],
                       sizeof(ioctl_trans_di_wait_to))) {
            status = -EFAULT;
            goto fxnExit;
        }

        if (copy_from_user(&transaction, &cmdHnd->argVec[0],
                           sizeof (ioctl_trans_di_wait_to))) {
            status = -EFAULT;
            goto fxnExit;
        }

        status = hw->pFunctionTable->waitTo(hw, &transaction);

        /* copy local object back to user object */
        if (copy_to_user(&cmdHnd->argVec[0], &transaction,
                         sizeof (ioctl_trans_di_wait_to)))
        {
            status = -EFAULT;
            goto fxnExit;
        }

    }
    break;

    default:
    {
        TPRO_LOG(TPRO_LOG_INFO_1,
                 ("[J%lu]%s: [CMD:0x%08X, USER:%p, HW:%p]"
                  ": Unhandled command\n",
                  jiffies, __FUNCTION__, cmd, user, user->pDevice));
        status = 1;
        goto fxnExit;
    }
    }

    fxnExit:
        TPRO_SPIN_LOCK(&hw->userLock);
        currUser = (currUser == user) ? NULL : currUser;
        TPRO_SPIN_UNLOCK(&hw->userLock);
        return status;

} /* End - tsyncpci_ioctl() */

/*****************************************************************************
**
** Function:    tsync_isr()
** Description: The tsync driver ISR routine
**
** Parameters:
**     IN:  irq    - interrupt
**         *dev_id - tsync device pointer
**         *regs   - processor's context
**
**     RETURNS: IRQ_HANDLED - IRQ was handled properly
**
*****************************************************************************/
static irqreturn_t tsync_isr(int irq, void *dev_id)
{
    tsyncpci_dev_t *hw          = (tsyncpci_dev_t *) dev_id;
    uint16_t        status;
    int             iInterrupt;
    struct          timespec ts = {0};

    /* Capture time stamp */
    getnstimeofday(&ts);

    /* read the interrupt status register.  Filter out the interrupts
     * we aren't expecting right now.  */
    status = tsync_read_interrupt_status(hw);

    for (iInterrupt = 0; iInterrupt < TSYNC_INTERRUPT_COUNT; iInterrupt++) {
        uint16_t intMask = 1 << iInterrupt;
        if (status & intMask) {

            if (iInterrupt == TSYNC_IRQ_PPS)
            {
                tsync_pps_isr_handler((int)hw->slotPosition);
            }

            hw->intCounter[iInterrupt]++; /* increment interrupt counter */

            /* Store interrupt time stamp*/
            hw->intTime[iInterrupt].seconds = ts.tv_sec;
            hw->intTime[iInterrupt].ns      = ts.tv_nsec;

            /* Wake anybody waiting on this interrupt */
            if (atomic_read(&hw->tsyncInterrupts[iInterrupt].flag) == 0) {
                atomic_inc(&hw->tsyncInterrupts[iInterrupt].flag);
                os_waitPost(&hw->tsyncInterrupts[iInterrupt].waitQueue);
            }

            /* Handle special cases: */
            if (iInterrupt == TSYNC_IRQ_HEART_BEAT) {
                if (atomic_read(&hw->heartFlag) == 0) {
                    atomic_inc(&hw->heartFlag);
                    os_waitPost(&hw->heart_wait);
                }
            }
            else if (iInterrupt == TSYNC_IRQ_MATCH) {
                if (atomic_read(&hw->matchFlag) == 0) {
                    atomic_inc(&hw->matchFlag);
                    os_waitPost(&hw->match_wait);
                }
            }
            else if (iInterrupt == TSYNC_IRQ_TIMESTAMP) {
                hw->eventCnt++;

                gpioQueueDrainBoardFIFO(hw);

                if (atomic_read(&hw->eventFlag) == 0) {
                    atomic_inc(&hw->eventFlag);
                    os_waitPost(&hw->event_wait);
                }
            }

        }
    }

    hw->intCnt++;

    /* IRQ_RETVAL returns HANDLED for non-zero and NONE for zero.
     * Conveniently, our filtered status is zero if there was nothing
     * to handle and non-zero if we handled something. */
    return IRQ_RETVAL(status);

} /* End - tsync_isr() */


/*******************************************************************************
**
** Function:    tpropci_isr()
** Description: The tpropci driver ISR routine
**
** Parameters:
**     IN:  irq    - interrupt
**         *dev_id - tpropci device pointer
**         *regs   - processor's context
**
**     RETURNS: IRQ_HANDLED - IRQ was handled properly
**
*******************************************************************************/
/*static irqreturn_t tpropci_isr(int irq, void *dev_id, struct pt_regs *regs)*/
static irqreturn_t tpropci_isr(int irq, void *dev_id)
{
    tsyncpci_dev_t* hw = (tsyncpci_dev_t *) dev_id;
    unsigned short status;
    MemObj Reg;

    /* read the interrupt status register */
    Reg.offset = hw->pFunctionTable->cmdStatOffset;
    hw->pFunctionTable->peek(hw, &Reg);
    status = Reg.value;

    /* check if the interrupt is ours */
    /* match int mask bit 6 */
    /* match Flag bit 3 */
    /* heartbeat int mask bit 5 */
    /* heartbeat Flag bit 4 */
    /* FIFO ready int mask bit 7 */
    /* FIFO empty Flag bit 0 */
    if (((status & 0x30) != 0x30) && /* heartbeat int and heartbeat flag */
        ((status & 0x48) != 0x48) && /* match int and match flag*/
        ((status & 0x81) != 0x80))   /* fifo Int and fifo rdy */
    {
        return IRQ_NONE;
    }

    /* If the heartbeat interrupt... */
    if (status & 0x10)
    {
        Reg.offset = hw->pFunctionTable->clrFlagsOffset;
        Reg.value = 0x10;
        Reg.l_value = 0x10;
        hw->pFunctionTable->poke(hw, &Reg);

        /* if we are waiting for interrupt post notification */
        if (atomic_read(&hw->heartFlag) == 0)
        {
            atomic_inc(&hw->heartFlag);
            os_waitPost(&hw->heart_wait);
        }
    }

    /* If the match interrupt... */
    if (status & 0x8)
    {
        Reg.offset = hw->pFunctionTable->clrFlagsOffset;
        Reg.value = 0x08;
        Reg.l_value = 0x08;
        hw->pFunctionTable->poke(hw, &Reg);

        /* if we are waiting for interrupt post notification */
        if (atomic_read(&hw->matchFlag) == 0)
        {
            atomic_inc(&hw->matchFlag);
            os_waitPost(&hw->match_wait);
        }
    }

    /* If the time-tag interrupt... */
    if (!(status & 0x1))
    {
        unsigned short *ptr = &hw->fifoVector [hw->headIndex];
        unsigned char eventIndex;

        hw->eventCnt++;

        /* read event */
        for (eventIndex = 0; eventIndex < 10; eventIndex++)
        {
            /* wait for data to be ready */
            Reg.offset = hw->pFunctionTable->cmdStatOffset;
            do
            {
                hw->pFunctionTable->peek(hw, &Reg);
                status = Reg.value;
            }
            while (status & 0x01);

            /* read word */
            Reg.offset = hw->pFunctionTable->fifoIntCtrlOffset;
            hw->pFunctionTable->peek(hw, &Reg);
            ptr[eventIndex] = Reg.value;
        }

        /* increment head index */
        hw->headIndex = ((hw->headIndex + 10) % (sizeof(hw->fifoVector)/sizeof(hw->fifoVector[0])));

        /* if we are waiting for interrupt post notification */
        if (atomic_read(&hw->eventFlag) == 0)
        {
            atomic_inc(&hw->eventFlag);
            os_waitPost(&hw->event_wait);
        }
    }

    hw->intCnt++;

    return IRQ_HANDLED;

} /* End - tpropci_isr() */

/*******************************************************************************
**
** Function: tsyncpci_open()
**
** Description: Open the TPRO-PCI device.
**
** Parameters:
**     IN: *inode - inode struct for the device
**         *file  - file pointer to return TPRO-PCI object reference with
**
**     RETURNS: 0 on success, or negative error code.
**
*******************************************************************************/
static int tsyncpci_open(struct inode *inode, struct file *filp)
{
    tsyncpci_dev_t * hw;
    tsyncpci_user_t *user;

    hw = tsyncpci_find_device(inode);

    if (hw == NULL)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 ("[J%lu]%s: Couldn't find matching device!!!\n",
                  jiffies, __FUNCTION__));
        return -EIO;
    }

    /* Allocate user structure and store device pointer. */
    user = kmalloc (sizeof(tsyncpci_user_t), GFP_KERNEL);
    if (user == NULL)
    {
        TPRO_LOG(TPRO_LOG_ERROR,
                 (KERN_ALERT "[J%lu]%s: No memory\n",
                  jiffies, __FUNCTION__));
        return -ENOMEM;
    }
    user->pDevice = hw;

    /* Lock the user list access.  If this is the first user, the lock must be maintained
     * around the device initialization as well. */
    TPRO_SPIN_LOCK(&hw->userLock);

    if (hw->pFirstUser != NULL)
    {
        /* Not the first user, so just insert the user structure into the linked list. */
        user->pNext = hw->pFirstUser;
        user->pPrev = hw->pLastUser;
        user->pNext->pPrev = user->pPrev->pNext = hw->pLastUser = user;
    }
    else
    {
        /* First user, so initialize the device. */
        hw->intCnt = 0;
        hw->eventCnt = 0;
        hw->headIndex = hw->tailIndex = 0;

        /* Since this is the first user, everything points to this user structure. */
        user->pNext = user->pPrev = hw->pFirstUser = hw->pLastUser = user;
    }

    /* Release access to the user list. */
    TPRO_SPIN_UNLOCK(&hw->userLock);

    TPRO_LOG(TPRO_LOG_OPEN,
             ("[J%lu]%s: Adding user struct %p to device %p\n",
              jiffies, __FUNCTION__, user, hw));

    /* set private data */
    filp->private_data = user;

    return 0;

} /* End - tsyncpci_open */

/*******************************************************************************
**
** Function: tsyncpci_remove_one()
**
** Description: Removes a TPRO PCI device from the system
**
** Parameters:
**     IN: *pdev - tsyncpci device pointer
**
**     RETURNS: None
**
*******************************************************************************/
static void tsyncpci_remove_one(struct pci_dev *pdev)
{
    tsyncpci_dev_t       *hw = NULL;
    struct device *cd = NULL;
    unsigned int         idx;

    for (idx = 0; idx < TSYNC_PCI_MAX_NUM; idx++)
    {
        if (tsyncpci_devp[idx] != NULL)
        {
            if (tsyncpci_devp[idx]->irq == pdev->irq)
            {
                hw = tsyncpci_devp[idx];
                cd = tsyncpci_cd[idx];
                tsyncpci_devp[idx] = NULL;
                tsyncpci_cd[idx]   = NULL;
                break;
            }
        }
    }

    if (hw)
    {
        if (hw->device == TSYNC_DEVID) {
            // iounmap((void*)tsyncpci_devp[idx]->ioAddr);
        }

        tsync_ptp_host_reference_unregister(cd);
        tsync_pps_unregister(hw);
        tsync_ptp_unregister();

        /* Disable interrupts on the device. */
        (void)hw->pFunctionTable->InitializeBoard(hw);

        /* Free the IRQ. */
        free_irq(hw->irq, hw);

        /* Destroy the spinlock and mutex. */
        TPRO_SPIN_LOCK_DESTROY(&hw->userLock);
        TPRO_MUTEX_DESTROY(&hw->deviceLock);

        // Set configuration register to disable PCI I/O & Memory Access
        pci_disable_device(pdev);

        /* Free the device object. */
        kfree(hw);
    }

    if (cd)
    {
        /* Destroy class device. */

        device_destroy(tsyncpci_cs,
                             MKDEV(MAJOR(tsyncpci_devNum), idx));
    }

} /* End - tsyncpci_remove_one */


/*******************************************************************************
            Explicit Module Routine Declarations
*******************************************************************************/
module_init(tsyncpci_init);    /*-- init module ---------*/
module_exit(tsyncpci_cleanup); /*-- cleanup module ------*/


#endif/* #ifndef NIOS */
