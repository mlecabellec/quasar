/*============================================================================*
 * FILE:                      U C E I P C I . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 2005 - 2019 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:   Universal PCI device driver
 *
 *             This module provides the interface between Abaco Systems
 *             avionics PCI/PCIe products and the Linux kernels 3.x.x, 4.x.x
 *             and 5.x.x. It performs the necessary functions (registering,
 *             memory mapping,..) needed to operate in a Linux operating
 *             system. This module is utilized by multiple Abaco Systems
 *             avionics products, use care when performing modifications.
 * 
 *
 * EXTERNAL ENTRY POINTS:
 *    uceipci_open            opens a device
 *
 *    uceipci_read            reads the device's PCI config space or kernel
 *                            space, or driver data.
 *
 *    uceipci_write           write to the driver data
 *
 *    uceipci_release         closes a device
 *
 *    uceipci_mmap            map the device's memory into user space
 *
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    uceipci_init_module     initializes the module, calls register_uceipci
 *
 *    uceipci_exit_module     uninitializes the module, calls unregister_uceipci
 *
 *    uceipci_probe           probes the PCI bus for supported Condor
 *                            Engineering PCI devices, then intitializes
 *                            the device
 *
 *    uceipci_remove          uninitializes a device
 *
 *
 * INTERNAL ROUTINES: 
 *    register_uceipci        registers the module with the kernel
 *
 *    unregister_uceipci      unregisters the module from the kernel
 *
 *    get_devData             checks the vaildity of the device, and retrieves
 *                            the device's data
 *
 *    verify_id               checks that the instance of a process id is valid
 *
 *    intrpt_cntrl            enables/disables hardware interrupts on the device
 *
 *===========================================================================*/

/* $Revision:  1.14 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/11/2011   initial. bch
  10/12/2012   added support for RAR15-XMC and RAR15-XMC-XT. bch
  11/27/2012   modified kernel version check. bch
  04/24/2013   added support for CEI-830X820, support for kernel 3.7.x, 
                modified uceipci_init_module. bch
  06/13/2013   added support for kernel 3.9.x. bch
  11/19/2013   added support for CEI-830A. support for kernels 3.10.x, 3.11.x,
                and 3.12.x. bch
  01/14/2014   added support for R15-PMC. support for kernel 3.13.x. bch
  05/08/2014   added support for the RP-708. support for kernel 3.14.x. bch
  01/15/2015   added support for the RAR-XMC. support for kernel 3.15.x, 3.16.x,
                3.17.x, and 3.18.x. bch
  01/14/2016   added support for kernel 3.19.x, 4.0.x, 4.1.x, 4.2.x, 4.3.x,
                and 4.4.x. bch
  05/26/2016   added support for MSI. modified uceipci_release. bch
  10/10/2016   added support for the R15-MPCIe and the RAR-MPCIe. added
                support for kernel 4.5.x to 4.8.x. bch
  01/05/2018   added support for kernels 4.9.x to 4.14.x. bch
  11/27/2018   added support for kernels 4.15.x to 4.19.x. bch
  11/18/2019   added support for kernels 4.20.x and 5.0.x to 5.3.x. bch
*/


#include "uceipci.h"
#include "uceipci_1553.h"
#include "uceipci_arinc.h"


// optional:  disables verifying the PID of a process to support "fork" processes
//#define DISABLE_PID_VERIFICATION

// the highest supported linux kernel 3.x, 4.x and 5.x minor versions, indexed by major version 
int supported_kv3_minor[]={101, 10, 102, 8, 113, 7, 11, 10, 13, 11, 108, 10, 74, 11, 79, 10, 77, 8, 140, 8};
int supported_kv4_minor[]={9, 52, 8, 6, 202, 7, 7, 10, 17, 140, 17, 12, 14, 16, 154, 18, 18, 19, 20, 84, 17};
int supported_kv5_minor[]={21, 21, 21, 11};

#ifndef NO_SYSFS_SUPPORT   
 // attribute for the universal driver...
 static ssize_t debug_show(struct device_driver* drv, char* buf) {
 //  struct pci_driver* pdrv = to_pci_driver(drv);
   return sprintf(buf,"%d\n", uceipci_debug);
 };
 static ssize_t debug_store(struct device_driver* drv, const char* buf, size_t count) {
 //  struct pci_driver* pdrv = to_pci_driver(drv);
   sscanf(buf,"%d", &uceipci_debug); 
   return strlen(buf);	
 };
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
 static DRIVER_ATTR_RW(debug);
#else
 static DRIVER_ATTR(debug, 0664, debug_show, debug_store);
#endif

 // attribute for all boards
 static ssize_t devnum_show(struct device_driver* drv, char* buf) {
   return sprintf(buf,"%d\n", ucei_pdev_indx);
 };
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
 static DRIVER_ATTR_RO(devnum);
#else
 static DRIVER_ATTR(devnum, 0444, devnum_show, NULL);
#endif
#endif

// interal function
static int get_devData(unsigned int minor, DEV_DATA** pData) {
  struct pci_dev* pdev;

  if(minor >= ucei_pdev_indx) {
    printk(KERN_ERR "uceipci(%d):  no device for minor (%d).\n", minor, ucei_pdev_indx);
    return -ENODEV;
  }
  if((pdev = ucei_pdev[minor]) == NULL) {
    printk(KERN_ERR "uceipci(%d):  PCI device is non-existent.\n", minor);
    return -ENODEV;
  }  
  if((*pData = (DEV_DATA*)pci_get_drvdata(pdev)) == NULL) {
    printk(KERN_ERR "uceipci(%d):  no data for device (PCI: bus %d, dev %d).\n", minor, pdev->bus->number,(pdev->devfn)>>3); 
    return -EFAULT;
  }
  if((*pData)->minor != minor) {
    printk(KERN_ERR "uceipci(%d):  minor number mismatch (%d).\n", minor, (*pData)->minor);
    return -EINVAL;
  }

  return 0;
}

static int verify_id(struct file* filp, pid_t* pid) {
  int status=-1;
  DEV_DATA* pData;
  unsigned int minor = iminor(filp->f_path.dentry->d_inode);
  ID_DATA* pId=(ID_DATA*)filp->private_data;
  pid_t cur_pid = ((struct task_struct*)current)->pid;
  pid_t cur_tgid = ((struct task_struct*)current)->tgid;

  if(pId == NULL) {
    printk(KERN_ERR "uceipci(%d):  no data in file descriptor for PID(%d), TID(%d)\n", minor, cur_pid, cur_tgid);
    return -EFAULT;
  }
  if((pId->status & STATUS_OPEN) == 0) {
    printk(KERN_ERR "uceipci(%d):  ID instance %d not open\n", minor, pId->id_indx);
    return -EFAULT;
  }
  if((status = get_devData(minor, &pData)) != 0) {
    printk(KERN_ERR "uceipci(%d):  failed to get device data (%d)\n", minor, status);
    return status;
  }
  if((pData->status & STATUS_OPEN) == 0) {
    printk(KERN_ERR "uceipci(%d):  device not open (0x%x)\n", minor, pData->status);
    return -EFAULT;
  }

 #ifndef DISABLE_PID_VERIFICATION
  // check if a process or a thread to set the correct PID for the instance
  if((pId->status & STATUS_THREAD) && ((cur_pid == pId->tid) && (cur_tgid == pId->pid))) {  // thread
    *pid = pId->tid;
    if(uceipci_debug) 
      printk(KERN_INFO "uceipci(%d):  ID(%d) verified as thread - TID(%d), PID(%d)\n", minor, pId->id_indx, pId->tid, pId->pid);
  }
  else if(cur_pid == pId->pid) {  // process
    *pid = cur_pid;
    if(uceipci_debug) 
      printk(KERN_INFO "uceipci(%d):  ID(%d) verified as process - PID(%d), TGID(%d)\n", minor, pId->id_indx, pId->pid, cur_tgid);
  }
  else if(cur_tgid == pId->pid) { // a thread using a process FD
    *pid = cur_pid;
    if(uceipci_debug) 
      printk(KERN_INFO "uceipci(%d):  verified thread TID(%d) using process ID(%d), PID(%d)\n", minor, cur_pid, pId->id_indx, cur_tgid);
  }
  else {
    printk(KERN_ERR "uceipci(%d):  PID(%d) not associated with device\n", minor, cur_pid);
    return -EINVAL;
  }
 #endif

  return 0;
}


#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl(DEV_DATA* pData, int mode) {
  int status, device = ucei_pdev[pData->minor]->device;

  if(uceipci_debug >= 1)
    printk(KERN_INFO "uceipci(%d):  hardware interrupt mode (%d)\n", pData->minor, mode);

  switch(mode) {
  case 0:  // check if hardware interrupts disabled
    if(!(pData->status & STATUS_INT_ENABLE)) {
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  hardware interrupts already disabled\n", pData->minor);
      return 0;
    }
    // if this is not the last instance of the device
    if(pData->id_cnt != 1) {
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  hardware interrupt open count (%d)\n", pData->minor, pData->id_cnt);
      return 0;
    }
    break;
  case 1:  // check if hardware interrupts enabled
    if(pData->status & STATUS_INT_ENABLE) {
      if(uceipci_debug)
        printk(KERN_WARNING "uceipci(%d):  hardware interrupts already enabled\n", pData->minor);
      return 0;
    }
    if(pData->irq <= 0) {
      printk(KERN_ERR "uceipci(%d): invalid IRQ (%d) assigned\n", pData->minor, pData->irq);
      return -EINVAL;
    }
    break;
  default:
    printk(KERN_WARNING "uceipci(%d): invalid hardware interrupt mode (%d)\n", pData->minor, mode);
    return -EINVAL;
  };

  switch(device) {
  case 0x820:
  case 0x1004:
  case 0x430:
  case 0x430A:
  case 0x530:
  case 0x630:
  case 0x830:
  case 0x830A:
  case 0x831:
  case 0x832:
  case 0x100A:
  case 0x100B:
  case 0x100C:
  case 0x100D:
    if((status = intrpt_cntrl_arinc(pData, mode)) != 0)
      return status;
    break;
  case 0x1003:
  case 0x1008:
  case 0x1553:
  case 0x1554:
  case 0x1555:
  case 0x1556:
  case 0x1557:
  case 0x1559:
  case 0x155A:
  case 0x155B:
  case 0x155C:
  case 0x1530:
  case 0x1542:
  case 0x1544:
  case 0x1545:
    if((status = intrpt_cntrl_1553(pData, mode)) != 0)
      return status;
    break;
  default:
    printk(KERN_WARNING "uceipci(%d):  device (%d) does not support hardware interrupts\n", pData->minor, device);
    return -EFAULT;
  };

  switch(mode) {
  case 0:
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  hardware interrupts disabled (0x%x)\n", pData->minor, pData->status & (STATUS_HWINT_WQ | STATUS_HWINT_SIGNAL));
    break;
  case 1:
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  hardware interrupts enabled (0x%x)\n", pData->minor, pData->status & (STATUS_HWINT_WQ | STATUS_HWINT_SIGNAL));
    break;
  };

  return 0;
}
#endif


// the opened FD in users space has priority in the scope it was opened, cannot have multiple instances open
static int uceipci_open(struct inode* inode, struct file* filp) {
  int status=0;
  unsigned int id_indx=0;
  unsigned int minor = iminor(inode);
  struct pci_dev* pdev = ucei_pdev[minor];
  DEV_DATA* pData=NULL;
  pid_t cur_pid = ((struct task_struct*)current)->pid;
  pid_t cur_tgid = ((struct task_struct*)current)->tgid;
  ID_DATA* pId=(ID_DATA*)(filp->private_data);

  if(uceipci_debug)
    printk(KERN_INFO "uceipci: uceipci_open (major %d, minor %d)).\n", imajor(inode), minor);
 
  // check if the FD has been opened by the PID
  if(pId != NULL) {
    if(uceipci_debug)
      printk(KERN_ERR "uceipci(%d):  file descriptor already opened by PID(%d), TID(%d)\n", minor, pId->pid, pId->tid);
    if(pId->status & STATUS_OPEN) 
      printk(KERN_ERR "uceipci(%d):  ID instance %d is already opened by PID(%d), TID(%d)\n", minor, pId->id_indx, pId->pid, pId->tid);
    return -EBUSY;
  }

  if((status = get_devData(minor, &pData)) != 0) {
    printk(KERN_ERR "uceipci(%d):  failed to get device data (%d)\n", minor, status);
    return status;
  }

  if((pData->status & STATUS_INIT) == 0) {
    if(uceipci_debug)
      printk(KERN_WARNING "uceipci(%d):  device not initialized (0x%x)\n", minor, pData->status);
    return 0;
  }

  // find first available id for process/thread
  for(id_indx=0;id_indx<MAX_INSTANCES;id_indx++) {
    if(((1<<id_indx) & pData->ids_mask) == 0) 
      break;
  }
  if(id_indx == MAX_INSTANCES) {
    printk(KERN_ERR "uceipci(%d):  reached maximum number of ID instances for device\n", minor);
    return -EBUSY;
  }

  pData->ids_mask |= (1<<id_indx);
  pId = &(pData->ids[id_indx]);
  pId->id_indx = id_indx;
  if(cur_pid == cur_tgid) {  // a process
    pId->tid = 0;
    pId->pid = cur_pid;
  }
  else {  // a thread
    pId->tid = cur_pid;
    pId->pid = cur_tgid;
    pId->status |= STATUS_THREAD;
  }
  pId->status |= STATUS_OPEN;
  pData->id_cnt++;
  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d):  attached to PID(%d), ID (%d)\n",minor, cur_pid, pId->id_indx); 

 #if !defined(NO_HW_INTERRUPTS) && defined(HW_INTERRUPTS_SIGNAL)
  pId->sigpid = 0;
  pId->sigval = -1;
 #endif

  // set info to file descriptor
  filp->private_data = pId;

  if((pData->status & STATUS_OPEN) == 0) {
    pData->status |= STATUS_OPEN;
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  device opened (PCI: bus %d, dev %d)\n",minor,pdev->bus->number,(pdev->devfn)>>3); 
  }
  pci_set_drvdata(pdev, pData);

  return (ssize_t) status;
}


static ssize_t uceipci_read(struct file* filp, char __user* usr_buf, size_t count, loff_t* ppos) {
  int status=-1;
  unsigned int minor = iminor(filp->f_path.dentry->d_inode);
  struct pci_dev* pdev = ucei_pdev[minor];
  u8 b_val=0;
  u16 w_val=0;
  u32 d_val=0;
  u32 offset=0;
  DEV_DATA* pData=NULL;
  u8 usr_cmd=0;

  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d): uceipci_read\n", minor);
  
  if((status = get_devData(minor, &pData)) != 0)
    return status;
 
  if((status = copy_from_user(&offset, usr_buf, sizeof(offset))) != 0) {
    printk(KERN_ERR "uceipci(%d):  failed copy_from_user, data (%d), status (%d)\n", minor, offset, status); 
    return status;
  }
  usr_cmd = (u8)(offset>>30);
  offset &= ~(0x3<<30);
  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d):  read - cmd(%d), offset(0x%x), count(%d)\n", minor, usr_cmd, offset, (int)count);
  
  // flush out user buffer
  if((status = copy_to_user(usr_buf, &d_val, 4)) != 0) {
    if(uceipci_debug)  
      printk(KERN_ERR "uceipci(%d):  failed to flush user buffer, status (%d)\n", minor, status); 
  } 
  
  switch(usr_cmd) {
  // read the board's memory	  
  case 0x0:	  
    switch(count) {
    case 1:
      b_val = readb((u8*)(pData->pci_bar_laddr_membase + offset)); 
      if((status = copy_to_user(usr_buf, &b_val, 1)) == 0) {
        if(uceipci_debug)  
          printk(KERN_INFO "uceipci(%d):  read byte from board, offset - %d, value - 0x%x\n",minor,offset,b_val);    	  
      } 
      break;
    case 2:
      w_val = readw((u16*)(pData->pci_bar_laddr_membase + offset)); 
      if((status = copy_to_user(usr_buf, &w_val, 2)) == 0) {
        if(uceipci_debug)  
          printk(KERN_INFO "uceipci(%d):  read word from board, offset - %d, value - 0x%x\n",minor,offset,w_val);    	  
      }
      break;
    case 4:
      d_val = readl((u32*)(pData->pci_bar_laddr_membase + offset)); 
      if((status = copy_to_user(usr_buf, &d_val, 4)) == 0) {
        if(uceipci_debug)  
         printk(KERN_INFO "uceipci(%d):  read dword from board, offset - %d, value - 0x%x\n",minor,offset,d_val);    	  
      }
      break;
    default:
      if(uceipci_debug)  
        printk(KERN_INFO "uceipci(%d):  invalid request (cmd: %d, count: %d).\n",minor,usr_cmd,(int)count); 
      return -EINVAL; 
    };
    break;
  // read the board's PCI config space, uses a byte offset
  case 0x1:
    switch(count) {
    case 0x1:
      if((status = pci_read_config_byte(pdev, (int)offset, &b_val)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to read PCI config (byte), offset - %d\n",minor,offset);
        return status;
      }
      d_val = b_val;
      if((status = put_user(d_val, (unsigned int*)usr_buf)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to copy PCI config (byte) data to user space, value - 0x%x\n",minor,b_val);
        return status;
      }
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  read PCI config (byte), offset - %d, value - 0x%x\n",minor,offset,b_val);
      break;
    case 0x2:
      if((status = pci_read_config_word(pdev, (int)offset*2, &w_val)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to read PCI config (word), offset - %d\n",minor,offset);
        return status;
      }
      d_val = w_val;
      if((status = put_user(d_val, (unsigned int*)usr_buf)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to copy PCI config (word) data to user space, value - 0x%x\n",minor,w_val);
        return status;
      }
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  read PCI config (word), offset - %d, value - 0x%x\n",minor,offset,w_val);
      break;
    case 0x4:
      if((status = pci_read_config_dword(pdev, (int)offset*4, &d_val)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to read PCI config (dword), offset - %d\n",minor,offset);
        return status;
      }
      if((status = put_user(d_val, (unsigned int*)usr_buf)) != 0) {
        printk(KERN_ERR "uceipci(%d):  failed to copy PCI config (dword) data to user space, value - 0x%x\n",minor,d_val);
        return status;
      }
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  read PCI config (dword), offset - %d, value - 0x%x\n",minor,offset,d_val);
      break;
    default:
      if(uceipci_debug)  
        printk(KERN_ERR "uceipci(%d):  invalid request (cmd: %d, count: %d).\n",minor,usr_cmd,(int)count);
      return -EINVAL;
    };
    break;
  // device attributes
  case 0x2:
    switch(offset) {
    case 0x1:  // gets the PCI BAR region info
      if(count < sizeof(pData->pci_bar_attr)) {
        if(uceipci_debug)        
          printk(KERN_ERR "uceipci(%d):  user buffer size (%d) too small for driver data (%d).\n",minor,(int)count,(int)sizeof(pData->pci_bar_attr)); 
	    status = -EINVAL;
      }  
      else
        status = copy_to_user(usr_buf, pData->pci_bar_attr, sizeof(pData->pci_bar_attr));
      break;
    case 0x2:  // gets the IRQ
     #ifndef NO_HW_INTERRUPTS
      if(count < sizeof(pData->irq)) {
        if(uceipci_debug)
          printk(KERN_ERR "uceipci(%d):  user buffer size (%d) too small for driver data (%d).\n",minor,(int)count,(int)sizeof(pData->irq)); 
        status = -EINVAL;
      }
      else
        status = copy_to_user(usr_buf, &pData->irq, sizeof(pData->irq));
     #endif
      break;
    default:
      if(uceipci_debug)  
        printk(KERN_ERR "uceipci(%d):  invalid request (cmd: %d, sub-cmd: %d).\n",minor,usr_cmd,offset); 
      return -EINVAL; 
    };
    break;
  // process waits for the ISR to indicate that a hardware interrupt has occured
  case 0x3: 
   #if !defined(NO_HW_INTERRUPTS) && defined(HW_INTERRUPTS_WAITQUEUE)
    if(((pData->status & STATUS_HWINT_WQ) == 0) || (pData->status & STATUS_HWINT_SIGNAL))
      return -1;
    if(offset >= MAX_CHANNELS)
      return -1;
    if(pData->last_intrpt[offset] == pData->cur_intrpt[offset]) {
      if((status = wait_event_interruptible(pData->isr_wait_q[offset], (pData->last_intrpt[offset] != pData->cur_intrpt[offset]))) != 0) {
        if(uceipci_debug >= 2) 
          printk(KERN_ERR "uceipci(%d):  process wait queue (%d) interrupted by signal (status: %d)\n", pData->minor, offset, status);
	      if(status == -ERESTARTSYS)
          return -ERESTARTSYS;
        return -1;
      }
    }
    if(uceipci_debug >= 2) {
      if(pData->status & STATUS_INT_ENABLE)
        printk(KERN_INFO "uceipci(%d):  process wait queue (%d) interrupt count (%d/%d)\n", pData->minor, offset, pData->last_intrpt[offset], pData->cur_intrpt[offset]);
    }
    pData->last_intrpt[offset] = pData->cur_intrpt[offset];
    return 0;
   #else
    return -EINVAL; 
   #endif
    break;
  default:
    if(uceipci_debug)  
      printk(KERN_ERR "uceipci(%d):  invalid request (cmd: %d).\n",minor,usr_cmd); 
    return -EINVAL; 
  };

  if(status != 0) {
    printk(KERN_ERR "uceipci(%d):  failed to copy data to user space, status: %d\n", minor, status); 
    return status;
  }

  return 0;
}


static ssize_t uceipci_write(struct file* filp, const char* usr_buf, size_t count, loff_t* ppos) {
  int status=-1;
  unsigned int minor = iminor(filp->f_path.dentry->d_inode);
  struct pci_dev* pdev = ucei_pdev[minor];
  int usr_data=0;
  DEV_DATA* pData=NULL;
  pid_t cur_pid = ((struct task_struct*)current)->pid;
 #if !defined(NO_HW_INTERRUPTS) && defined(HW_INTERRUPTS_SIGNAL)
  ID_DATA* pId=(ID_DATA*)(filp->private_data);
 #endif

  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d): uceipci_write\n", minor);

  if((status = verify_id(filp, &cur_pid)) != 0) {
    printk(KERN_ERR "uceipci(%d):  PID(%d) not associated with device\n", minor, cur_pid);
    return status;
  }

  if((status = get_devData(minor, &pData)) != 0)
    return status;

  if((status = copy_from_user(&usr_data, usr_buf, sizeof(usr_data))) != 0) {
    printk(KERN_ERR "uceipci(%d):  failed copy_from_user, data (%d), count (%d), status (%d)\n",minor,usr_data,(int)count,status); 
    return status;
  }

  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d):  write - cmd(0x%X), data(0x%X)\n", minor, (int)count, usr_data);

  switch(count) {
  case 1:  // sets the sigpid (is -1 disable ID, if 0 set to current PID)
  #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceipci:  driver not compiled with hardware interrupts enabled.\n");
  #else
   #ifndef HW_INTERRUPTS_SIGNAL
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  driver not compiled with signal support for hardware interrupts.\n", minor);
   #else
    // disable the PID from receiving signals
    if(usr_data == -1) {
      pData->ids_mask_intrpt &= ~(1<<pId->id_indx);
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  PID(%d) unattached from signal(%d).\n", minor, pId->sigpid, pData->signal);
      pId->sigpid = 0;
      break;
    }
    if(pId->sigpid > 0) {
      if(uceipci_debug)
        printk(KERN_WARNING "uceipci(%d):  PID(%d) already assigned to signal(%d).\n", minor, pId->sigpid, pData->signal);
      break;
    }
    if(usr_data == 0) 
     pId->sigpid = cur_pid;
    else
     pId->sigpid = (pid_t)usr_data;
    pData->ids_mask_intrpt |= (1<<pId->id_indx);  // enable the PID to receive signals
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  PID(%d) attached for signal(%d).\n", minor, pId->sigpid, pData->signal);
   #endif
  #endif
    break;
  case 2: // sets the signal number (if -1 ignore, if 0 disable signals from in ISR)
  #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceipci:  driver not compiled with hardware interrupts enabled.\n");
  #else
   #ifndef HW_INTERRUPTS_SIGNAL
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  driver not compiled with signal support for hardware interrupts.\n", minor);
   #else
    if(usr_data == -1)
      break;
    if(usr_data == 0) {
      pData->status &= ~STATUS_HWINT_SIGNAL;
      pData->signal = 0;  // this will disable signals from being generated in the ISR
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  disabled signals from ISR\n", minor);
      break;
    }
    if(pData->status & STATUS_HWINT_SIGNAL) {
      if(uceipci_debug)
        printk(KERN_WARNING "uceipci(%d):  signal(%d) already assigned to PID(%d).\n", minor, pData->signal, pId->sigpid);
      break;
    }
    pData->status |= STATUS_HWINT_SIGNAL;
   #ifdef HW_INTERRUPTS_WAITQUEUE
    if(pData->status & STATUS_HWINT_WQ)
      pData->status &= ~STATUS_HWINT_WQ;  // disable wait queue
   #endif
    pData->signal = (unsigned int)usr_data;  // sets the signal for the ISR
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  signal(%d) attached to PID(%d).\n", minor, pData->signal, pId->sigpid);
   #endif
  #endif
    break;
  case 3: // sets the signal value (if -1 ignore, if 0 use the default setting in ISR)
  #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceipci:  driver not compiled with hardware interrupts enabled.\n");
  #else
   #ifndef HW_INTERRUPTS_SIGNAL
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  driver not compiled with signal support for hardware interrupts.\n", minor);
   #else
    if(usr_data == -1)
      break;
    if(usr_data == 0) {
      pId->sigval = -1;
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  using default sigval for PID(%d).\n", minor, pId->sigpid);
    }
    else {
      pId->sigval = usr_data;  // sets the signal value per device for the ISR
      if(uceipci_debug) 
        printk(KERN_INFO "uceipci(%d):  sigval(%d) set for PID(%d).\n", minor, pId->sigval, pId->sigpid);
    } 
   #endif
  #endif
    break;
  case 4:  // enables/disables hardware interrupts
   #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceipci:  driver not compiled with hardware interrupts enabled.\n");
   #else
    if((status = intrpt_cntrl(pData, usr_data)) != 0) {
      printk(KERN_ERR "uceipci(%d):  failed to control hardware interrupts, mode(%d), status(%d).\n", minor, usr_data, status);
      return status;
    }
   #endif
    break;
  case 5:  // wake wait queue
   #if !defined(NO_HW_INTERRUPTS) && defined(HW_INTERRUPTS_WAITQUEUE)
    if(((pData->status & STATUS_HWINT_WQ) == 0) || (pData->status & STATUS_HWINT_SIGNAL))
      return -1;
    if(usr_data >= MAX_CHANNELS)
      return -1;
    pData->cur_intrpt[usr_data]++; 
    wake_up_interruptible(&(pData->isr_wait_q[usr_data]));
    if(uceipci_debug >= 2)
      printk(KERN_INFO "uceipci(%d):  wake wait queue (%d) - count (%d)\n", pData->minor, usr_data, pData->cur_intrpt[usr_data]);
   #else
    return -EINVAL;
   #endif
    break; 
  default:
    printk(KERN_ERR "uceipci(%d):  invalid write index (%d)\n", minor, (int)count);
    return -EINVAL;
  }

  pci_set_drvdata(pdev, pData);

  return 0;
}	


static int uceipci_release(struct inode* inode, struct file* filp) {
  int status=0;
  unsigned int minor = iminor(inode);
  struct pci_dev* pdev = ucei_pdev[minor];
  DEV_DATA* pData=NULL;
  pid_t cur_pid = ((struct task_struct*)current)->pid;
  ID_DATA* pId = (ID_DATA*)filp->private_data;

  if(uceipci_debug)
    printk(KERN_INFO "uceipci(%d): uceipci_release\n", minor);

  if((status = verify_id(filp, &cur_pid)) != 0) {
    printk(KERN_WARNING "uceipci(%d):  PID(%d) not associated with device\n", minor, cur_pid);
    return status;
  }

  if((status = get_devData(minor, &pData)) != 0) {
    printk(KERN_ERR "uceipci(%d):  failed to get device data (%d)\n", minor, status);
    return status;
  }

  if(!(pData->status & STATUS_OPEN)) {
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  device already closed (PCI: bus %d, dev %d).\n",minor,pdev->bus->number,(pdev->devfn)>>3);
    return 0;
  }

  pData->ids_mask &= ~(1<<pId->id_indx);

  if(uceipci_debug) 
    printk(KERN_INFO "uceipci(%d):  unattached from ID(%d), PID(%d), ID count %d\n",minor, pId->id_indx, cur_pid, pData->id_cnt); 
  memset(pId, 0, sizeof(ID_DATA));  // flush ID information
  filp->private_data = NULL;

  // if last PID then close device
  if(pData->id_cnt-- == 1) {
   #ifndef NO_HW_INTERRUPTS
    if(pData->status & STATUS_INT_ENABLE) {
      if(uceipci_debug)
        printk(KERN_INFO "uceipci(%d):  hardware interrupts still enabled.\n", minor);
      intrpt_cntrl(pData, 0);
    }
   #endif
    pData->status &= ~STATUS_OPEN;
    if(uceipci_debug)
      printk(KERN_INFO "uceipci(%d):  device closed (PCI: bus %d, dev %d).\n",minor,pdev->bus->number,(pdev->devfn)>>3);
    pData->id_cnt = 0;
  }
  pci_set_drvdata(pdev, pData);

  return 0;
}


static int uceipci_mmap(struct file* filp, struct vm_area_struct* vma) {
   int bar_idx, status=0;
   unsigned long vma_size=0;
   unsigned long bar_size=0;
   unsigned long phys_addr=0;
   unsigned int minor = iminor(filp->f_path.dentry->d_inode);
   struct pci_dev* pdev = ucei_pdev[minor];
   DEV_DATA* pData=NULL;
   pid_t cur_pid=((struct task_struct*)current)->pid;
   ID_DATA* pId = (ID_DATA*)filp->private_data;

   if(uceipci_debug)
     printk(KERN_INFO "uceipci(%d): uceipci_mmap\n", minor);

   if((status = verify_id(filp, &cur_pid)) != 0) {
     printk(KERN_WARNING "uceipci(%d):  PID(%d) not associated with device\n", minor, cur_pid);
     return status;
   }

   // check if device has already been mapped for the ID instance
   if(pId->status & STATUS_MAP) {
     if(uceipci_debug)
       printk(KERN_WARNING "uceipci(%d):  device already mapped, PID(%d)\n", minor, cur_pid);
     return 0;
   }

   if((status = get_devData(minor, &pData)) != 0) {
     printk(KERN_ERR "uceipci(%d):  failed to get device data (%d)\n", minor, status);
     return status;
   }
   // determine BAR region to map for the ID instance
   for(bar_idx=0;bar_idx<6;bar_idx++) {
     if((pData->pci_bar_regions&(1<<bar_idx)) && !(pId->mapped_bar_regions&(1<<bar_idx))) 
       break;
   }
   if(bar_idx == 6) {
     printk(KERN_ERR "uceipci(%d):  failed to determine BAR region to map (%d - 0x%x).\n",minor, pData->pci_bar_regions, pId->mapped_bar_regions);
     return -EFAULT;
   }

   // map board memory to user (process) memory space. not available for no-MMU.
   vma_size = vma->vm_end - vma->vm_start;
   bar_size = pci_resource_len(pdev, bar_idx);
   if(bar_size < PAGE_SIZE)
     bar_size = PAGE_SIZE;
   if(vma_size != bar_size) {
     printk(KERN_ERR "uceipci(%d):  vma size mismatch for BAR%d - driver 0x%lx, vma 0x%lx, PID(%d)\n",minor,bar_idx,bar_size,vma_size,cur_pid);
     return -EFAULT;
   }
   phys_addr = (vma->vm_pgoff << PAGE_SHIFT) + pci_resource_start(pdev, bar_idx);
   
  #if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
   vma->vm_flags |= (VM_DONTEXPAND | VM_IO);
  #else
   vma->vm_flags |= (VM_RESERVED | VM_IO);
  #endif
   vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
   if((status = remap_pfn_range(vma, vma->vm_start, phys_addr>>PAGE_SHIFT, vma_size, vma->vm_page_prot)) != 0) {
     printk(KERN_ERR "uceipci(%d):  failed to map device - status: %d, PID(%d)\n",minor,status,cur_pid);
     return status;
   }

   pId->mapped_bar_regions |= (1<<bar_idx);
   // if all BAR regions are mapped for the PID set status to STATUS_MAP
   if(pData->pci_bar_regions == pId->mapped_bar_regions)
     pId->status |= STATUS_MAP;
   pci_set_drvdata(pdev, pData);

   if(uceipci_debug)
     printk(KERN_INFO "uceipci(%d):  mapped BAR%d, size: 0x%lx, PID(%d)\n",minor, bar_idx, vma_size, cur_pid);

   return 0;
}


static struct file_operations uceipci_fops = {
	.owner   =  THIS_MODULE,
	.mmap    =  uceipci_mmap,
	.read    =  uceipci_read,
	.write   =  uceipci_write,
	.open    =  uceipci_open,
	.release =  uceipci_release,
};


int register_uceipci(void) {
  int status=0;

  if(uceipci_debug)
    printk(KERN_INFO "uceipci: register_uceipci\n");
    
  // register char devices
  status = alloc_chrdev_region(&ucei_dev_t, 0, MAX_DEVICES, DRIVER_NAME);
  if((status != 0) || (ucei_dev_t == (dev_t) 0)) {
    printk(KERN_ERR "uceipci:  failed to alloc chrdev region, status - %d.\n", status);
    return status;
  }
  
  memset_io(&ucei_cdev,0,sizeof(struct cdev));	    
  cdev_init(&ucei_cdev, &uceipci_fops);

  ucei_cdev.owner = THIS_MODULE;
  kobject_set_name(&ucei_cdev.kobj, "uceipci");
  
  // this must be called after the driver is ready to work
  if((status = cdev_add(&ucei_cdev, ucei_dev_t, MAX_DEVICES)) != 0) {
    printk(KERN_ERR "uceipci:  failed to add char device, status: 0x%x.\n", status);
    kobject_put(&ucei_cdev.kobj);
    unregister_chrdev_region(ucei_dev_t, MAX_DEVICES);
  }
  else
    cdev_exists=1; 

  return status; 
}


void unregister_uceipci(void) {  
  if(uceipci_debug)
     printk(KERN_INFO "uceipci: unregister_uceipci\n");

  unregister_chrdev_region(ucei_dev_t, MAX_DEVICES);

  if(cdev_exists == 1)  
    cdev_del(&ucei_cdev);
  
  ucei_dev_t = 0;
  cdev_exists = 0; 
}


// begin of modules entry points
static int uceipci_probe(struct pci_dev* pdev, const struct pci_device_id* dev_id) {
  int i, status=0;
  DEV_DATA* pData=NULL;
  unsigned short wVal=0;

  if(uceipci_debug) 
    printk(KERN_INFO "uceipci: uceipci_probe\n");

  if(ucei_pdev_indx >= MAX_DEVICES) {
    printk(KERN_ERR "uceipci:  maximum devices supported reached - %d.\n", MAX_DEVICES);
    return -EBUSY;
  }

  if(pci_dev_get(pdev) == NULL) {
    printk(KERN_ERR "uceipci:  failed to get PCI device.\n");
    return -EFAULT;
  }
  if(uceipci_debug)  
    printk(KERN_INFO "uceipci:  detected device: 0x%X (bus: %d  dev: %d)\n", pdev->device, pdev->bus->number, pdev->devfn >> 3);

  if(pdev->device != 0x1008) {
    // request all boards's PCI BAR regions
    if((status = pci_request_regions(pdev, "uceipci")) != 0) {
      printk(KERN_ERR "uceipci:  unable to request BAR regions, status %d\n", status);
      pci_dev_put(pdev);
      return status;
    }
  }

  if((status = pci_enable_device(pdev)) != 0) {
    printk(KERN_ERR "uceipci:  unable to enable device 0x%X, status: %d.\n", pdev->device, status);
    if(pdev->device != 0x1008) 
      pci_release_regions(pdev);
    pci_dev_put(pdev);
    return status;
  }

  // map device data into kernel memory
  if((pData = (DEV_DATA*)kmalloc(sizeof(DEV_DATA), GFP_KERNEL))==NULL) {
    printk(KERN_ERR "uceipci:  failed to allocate kernel memory for device %d.\n", pdev->device);
    pci_disable_device(pdev);
    if(pdev->device != 0x1008) 
      pci_release_regions(pdev);
    pci_dev_put(pdev);
    return -ENOMEM;
  }
  // initialize device state
  memset_io(pData, 0, sizeof(DEV_DATA)); 
  strcpy(pData->boardname, "");
  pData->id = -1;
  pData->minor = ucei_pdev_indx;
 #ifndef NO_HW_INTERRUPTS
  pData->irq = -1;
 #endif

  // probe board's PCI BAR configuration registers
  for(i=0;i<6;i++) {
    // track PCI BAR memory mapped regions
    if((pci_resource_start(pdev, i) > 0) && ((pci_resource_flags(pdev, i) & 0x1) == 0)) {
      pData->pci_bar_regions |= (1<<i);
      pData->pci_bar_attr[i][0] = pci_resource_start(pdev, i);  // PCI address
      pData->pci_bar_attr[i][1] = pci_resource_len(pdev, i);    // PCI address size
    }
    if(uceipci_debug >= 2)
      printk(KERN_INFO "uceipci(%d):  BAR%d (addr: 0x%lx, size: 0x%lx, type: 0x%lx)\n", pData->minor, i, (unsigned long)pci_resource_start(pdev, i), (unsigned long)pci_resource_len(pdev, i), pci_resource_flags(pdev, i) & 0x1);
  }
  if(pData->pci_bar_regions == 0) {
    printk(KERN_ERR "uceipci:  failed to detect PCI BAR registers.\n");
    kfree(pData);
    pci_disable_device(pdev);
    if(pdev->device != 0x1008) 
      pci_release_regions(pdev);
    pci_dev_put(pdev);
    return -EFAULT;
  }

  // MEM enable
  pci_read_config_word(pdev, PCI_COMMAND, &wVal);
  wVal |= PCI_COMMAND_MEMORY;
  pci_write_config_word(pdev, PCI_COMMAND, wVal);

  // assume PLX at BAR0 when more then one PCI BAR memory address region exists
  i=0;
  if(pData->pci_bar_regions > 1) {
    pData->pci_bar_conf_membase = (unsigned long)ioremap_nocache(pData->pci_bar_attr[0][0], pData->pci_bar_attr[0][1]);
    if(pData->pci_bar_conf_membase == 0) {
      printk(KERN_ERR "uceipci(%d):  failed ioremap for BAR0.\n", pData->minor);
      kfree(pData);
      pci_disable_device(pdev);
      if(pdev->device != 0x1008) 
        pci_release_regions(pdev);
      pci_dev_put(pdev);
      return -ENOMEM;
    }
    if(uceipci_debug >= 1) 
      printk(KERN_INFO "uceipci(%d):  mapped BAR0 (mem: 0x%lx, size: 0x%lx, kernel: 0x%lx)\n", pData->minor, pData->pci_bar_attr[0][0], pData->pci_bar_attr[0][1], pData->pci_bar_conf_membase);
    for(i=1;i<6;i++) {
      if(pData->pci_bar_regions & (1<<i))
        break;
    } 
  }  
  // map PCI BAR memory address region to access the board's registers
  pData->pci_bar_laddr_membase = (unsigned long)ioremap_nocache(pData->pci_bar_attr[i][0], pData->pci_bar_attr[i][1]);
  if(pData->pci_bar_laddr_membase == 0) {
    printk(KERN_ERR "uceipci(%d):  failed ioremap for BAR%d.\n", pData->minor, i);
    iounmap((unsigned long*)pData->pci_bar_conf_membase); 
    kfree(pData);
    pci_disable_device(pdev);
    if(pdev->device != 0x1008) 
      pci_release_regions(pdev);
    pci_dev_put(pdev);
    return -ENOMEM;
  }
  if(uceipci_debug >= 1) 
    printk(KERN_INFO "uceipci(%d):  mapped BAR%d (mem: 0x%lx, size: 0x%lx, kernel: 0x%lx)\n", pData->minor, i, pData->pci_bar_attr[i][0], pData->pci_bar_attr[i][1], pData->pci_bar_laddr_membase);
  
  pci_set_drvdata(pdev, pData);
      
  switch(pdev->device) {
   case 0x1024:
   case 0x0040:
   case 0x430:
   case 0x430A:
   case 0x520:
   case 0x530:
   case 0x620:
   case 0x630:
   case 0x708:
   case 0x708A:
   case 0x820:
   case 0x821:
   case 0x830:
   case 0x830A:
   case 0x831:
   case 0x832:
   case 0x1004:
   case 0x1005:
   case 0x1006:
   case 0x1009:
   case 0x100A:
   case 0x100B:
   case 0x100C:
   case 0x100D:
     status = initialize_arinc(pdev);
     break;
   case 0x1003:
   case 0x1008:
   case 0x1553:
   case 0x1554:
   case 0x1555:
   case 0x1556:
   case 0x1557:
   case 0x1558:
   case 0x1559:
   case 0x155A:
   case 0x155B:
   case 0x155C:
   case 0x1530:
   case 0x1542:
   case 0x1544:
   case 0x1545:
     status = initialize_1553(pdev);
     break;
   default:
     printk(KERN_ERR "uceipci:  invalid device  - 0x%X\n", pdev->device);
     status = -EFAULT;
  };
  if(status != 0) {
    if(pData->pci_bar_laddr_membase > 0)  
      iounmap((unsigned long*)pData->pci_bar_laddr_membase);
    if(pData->pci_bar_conf_membase > 0)  
      iounmap((unsigned long*)pData->pci_bar_conf_membase); 
    kfree(pData);
    pci_disable_device(pdev);
    if(pdev->device != 0x1008) 
      pci_release_regions(pdev);
    pci_dev_put(pdev);
    return status;
  }

 #ifndef NO_HW_INTERRUPTS
  pData->irq = pdev->irq;
 #ifdef HW_INTERRUPTS_WAITQUEUE
  if(pData->status & STATUS_HWINT_WQ) {
    for(i=0;i<MAX_CHANNELS;i++)
      init_waitqueue_head(&(pData->isr_wait_q[i]));
  }
 #endif
 #endif
  pData->status |= STATUS_INIT;
  pci_set_drvdata(pdev, pData);

 #ifndef NO_SYSFS_SUPPORT
  // create MINOR attribute in the sysfs
  if((status = device_create_file(&pdev->dev, &dev_attr_minor)) != 0) 
    printk(KERN_WARNING "uceipci:  failed to set device attribute to sysfs, status: 0x%x.\n", status);
 #endif

  ucei_pdev[ucei_pdev_indx++] = pdev;

  return 0;
}


static void __exit uceipci_remove(struct pci_dev* pdev) {
  int i, status=0;
  DEV_DATA* pData=NULL;

  if(uceipci_debug) 
    printk(KERN_INFO "uceipci: uceipci_remove\n");

  if(pdev == NULL) {
    printk(KERN_WARNING "uceipci:  PCI device is non-existent.\n");	  
    return;
  }
  
  if((pData = (DEV_DATA*) pci_get_drvdata(pdev)) == NULL) {
    printk(KERN_ERR "uceipci:  failed to get device data (PCI: bus %d, dev %d).\n", pdev->bus->number,(pdev->devfn)>>3); 
    return;
  }
  
 #ifndef NO_HW_INTERRUPTS
  if(pData->status & STATUS_INT_ENABLE) 
    intrpt_cntrl(pData, 0);  // disable hardware interrupts
 #endif

  switch(pdev->device) {
   case 0x1024:
   case 0x0040:
   case 0x430:
   case 0x430A:
   case 0x520:
   case 0x530:
   case 0x620:
   case 0x630:
   case 0x708:
   case 0x708A:
   case 0x820:
   case 0x821:
   case 0x830:
   case 0x830A:
   case 0x831:
   case 0x832:
   case 0x1004:
   case 0x1005:
   case 0x1006:
   case 0x1009:
   case 0x100A:
   case 0x100B:
   case 0x100C:
   case 0x100D:
     status = uninitialize_arinc(pdev);
     break;
   case 0x1003:
   case 0x1008:
   case 0x1553:
   case 0x1554:
   case 0x1555:
   case 0x1556:
   case 0x1557:
   case 0x1558:
   case 0x1559:
   case 0x155A:
   case 0x155B:
   case 0x155C:
   case 0x1530:
   case 0x1542:
   case 0x1544:
   case 0x1545:
     status = uninitialize_1553(pdev);
     break;
   default:
     printk(KERN_ERR "uceipci:  invalid device  - 0x%X.\n", pdev->device);
     return;
  };
  if(status != 0)
    printk(KERN_ERR "uceipci(%d):  device failed to un-initialize, resources not released.\n", pData->minor); 

  if(pData->pci_bar_laddr_membase > 0)  
    iounmap((unsigned long*)pData->pci_bar_laddr_membase);
  if(pData->pci_bar_conf_membase > 0)  
    iounmap((unsigned long*)pData->pci_bar_conf_membase); 
   
  pci_set_drvdata(pdev, NULL);  // to flush data
  // pci_set_drvdata(pdev, pData);
  kfree(pData);  

 #ifndef NO_SYSFS_SUPPORT
  // remove attributes for sysfs 
  device_remove_file(&pdev->dev, &dev_attr_minor);  
 #endif
 
  if(pdev->device != 0x1008) 
    pci_release_regions(pdev);

  pci_disable_device(pdev);
  pci_dev_put(pdev);
  for(i=0;i<MAX_DEVICES;i++) {
    if(ucei_pdev[i] == pdev) {
      if(uceipci_debug) 
        printk(KERN_INFO "uceipci:  pdev 0x%lx (bus: %d  dev: %d)\n", (unsigned long)ucei_pdev[i], pdev->bus->number, pdev->devfn >> 3);
      ucei_pdev[i] = NULL;
      ucei_pdev_indx--;
      break;
    }  
  }
}

static int __init uceipci_init_module(void) {
  int status=0, kv1, kv2, kv3;
 #ifndef NO_HW_INTERRUPTS
  char buf[30]="";
 #endif

  printk(KERN_INFO "uceipci: loading module %s\n", uceipci_version); 

  if(uceipci_debug) 
    printk(KERN_INFO "uceipci: uceipci_init_module\n");
 
  // check the kernel version 
  kv1 = (LINUX_VERSION_CODE & 0xFF0000)>>16;
  kv2 = (LINUX_VERSION_CODE & 0xFF00)>>8;
  kv3 = LINUX_VERSION_CODE & 0xFF;
  if(kv1 == 3) {
    if(kv2 <= (sizeof(supported_kv3_minor)/4)) {
      if(kv3 <= supported_kv3_minor[kv2]) 
        status = 1;
    }
  }
  else if(kv1 == 4) {
    if(kv2 <= (sizeof(supported_kv4_minor)/4)) {
      if(kv3 <= supported_kv4_minor[kv2]) 
        status = 1;
    }
  }
  else if(kv1 == 5) {
    if(kv2 <= (sizeof(supported_kv5_minor)/4)) {
      if(kv3 <= supported_kv5_minor[kv2]) 
        status = 1;
    }
  }
  if(status != 1)  
    printk(KERN_WARNING "uceipci:  this driver has not been tested with this kernel version: %d.%d.%d\n", kv1, kv2, kv3);

  if(uceipci_debug) {
    if(uceipci_debug != 0)
      printk(KERN_INFO "uceipci:  debug level %d.\n", uceipci_debug);
    if(uceipci_major != 0)
      printk(KERN_INFO "uceipci:  major number %d.\n", uceipci_major);
   #ifndef NO_SYSFS_SUPPORT 
    printk(KERN_INFO "uceipci:  SYSFS support enabled.\n");
   #else
    printk(KERN_INFO "uceipci:  SYSFS support disabled.\n");
   #endif
   #ifndef NO_HW_INTERRUPTS
    #ifdef HW_INTERRUPTS_SIGNAL
     strcat(buf, "signal");
    #ifdef HW_INTERRUPTS_WAITQUEUE
     strcat(buf, ", ");
    #endif
    #endif
    #ifdef HW_INTERRUPTS_WAITQUEUE
     strcat(buf, "waitqueue");
    #endif
    #ifdef HW_MSI_INTERRUPTS
     strcat(buf, ", MSI");
    #endif
    printk(KERN_INFO "uceipci:  'hardware interrupts' (%s) support enabled.\n", buf);
   #else
    printk(KERN_INFO "uceipci:  'hardware interrupts' support disabled.\n");
   #endif
    // determine Endian
   #ifdef __powerpc__
    printk("uceipci:  Big Endian\n");
   #else
    printk("uceipci:  Little Endian\n");
   #endif
  }

  memset_io(ucei_pdev,0,sizeof(ucei_pdev));

  status = pci_register_driver(&uceipci_driver);
  if(status < 0) {
    printk(KERN_ERR "uceipci:  failed to register driver, status: 0x%x.\n", status);
    return status;
  }

 #ifndef NO_SYSFS_SUPPORT   
  // set attributes to the sysfs
  if((status = driver_create_file((struct device_driver*)&uceipci_driver.driver, &driver_attr_debug))!=0)
    printk(KERN_ERR "uceipci:  failed to set driver attribute to sysfs, status: 0x%x.\n", status);

  if((status = driver_create_file((struct device_driver*)&uceipci_driver.driver, &driver_attr_devnum))!=0)
    printk(KERN_ERR "uceipci:  failed to set driver attribute to sysfs, status: 0x%x.\n", status);
 #endif

  status = register_uceipci();

  return status;
}


static void __exit uceipci_exit_module(void) {
  if(uceipci_debug)
    printk(KERN_INFO "uceipci: uceipci_exit_module\n");

  unregister_uceipci(); 

 #ifndef NO_SYSFS_SUPPORT   
  // remove attributes from sysfs
  driver_remove_file((struct device_driver*)&uceipci_driver.driver, &driver_attr_debug);
  driver_remove_file((struct device_driver*)&uceipci_driver.driver, &driver_attr_devnum);  
 #endif

  pci_unregister_driver(&uceipci_driver);

  if(uceipci_debug)
    printk(KERN_INFO "uceipci: unloaded module\n"); 
}
