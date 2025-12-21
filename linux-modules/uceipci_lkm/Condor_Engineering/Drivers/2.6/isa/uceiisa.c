/*============================================================================*
 * FILE:                    U C E I I S A . C (hybrid)
 *============================================================================*
 *
 *      COPYRIGHT (C) 2005 - 2011 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:   Abaco Systems Universal ISA device driver (hybrid)
 *
 *             This module provides the interface between Abaco Systems
 *             avionic ISA products and the Linux kernel 2.6.x. It performs
 *             the necessary functions (registering, memory mapping,..) needed
 *             to operate in a LINUX operating system. This module is utilized
 *             by multiple Abaco Systems avionic products, use care when
 *             performing modifications.
 * 
 *
 * EXTERNAL ENTRY POINTS:
 *    uceiisa_open            open a device
 *
 *    uceiisa_close           close a device
 *
 *    uceiisa_mmap            map the device's memory into user space
 *
 *    uceiisa_ioctl           performs a variety of utility functions
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *    uceiisa_init_module     initializes the module, calls register_uceiisa
 *                            and scan_uceiisa
 *
 *    uceiisa_exit_module     uninitializes the module, calls
 *                            unregister_uceiisa
 *
 * INTERNAL ROUTINES: 
 *    register_uceiisa        registers the module with the kernel
 *
 *    unregister_uceiisa      unregisters the module from the kernel
 *
 *    scan_uceiisa            probes the ISA bus looking for supported 
 *                            ISA devices.
 *
 *
 *===========================================================================*/

/* $Revision:  1.10 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/10/2004   Initial.  Support for Bustools/1553 and CEI-x20 ISA devices. bch
  09/03/2005   added uceiisa_read and uceiisa_write, modified the ISR. bch
  05/25/2006   replaced kill_proc with send_sig_info in uceiisa_1553_isr, added
                signal and sigval to uceiisa_write. bch
  09/18/2006   added isa_readb and isa_readw macros for kernels 2.6.17+. bch
  03/27/2007   modified to support kernel changes in 2.6.19. bch
  06/07/2007   added intrpt_cntrl. modified uceiisa_write and uceiisa_1553_isr. bch
  12/10/2007   modified to support kernel changes in 2.6.22. bch
  11/18/2008   modified uceiisa_1553_isr to support kernel changes in 2.6.26. bch
  04/10/2009   modified uceiisa_write to set the default value for pid. bch
  04/13/2011   modified uceiisa_1553_isr to support kernel changes in 2.6.31. bch
  11/03/2011   modified uceiisa_1553_isr to support kernel changes in 2.6.39. bch
*/

#include "uceiisa.h"

// ISA helper functions removed from kernel
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
 #ifndef isa_readb
  #define isa_readb(val)  readb(__ISA_IO_base + val)
 #endif
 #ifndef isa_readw
  #define isa_readw(val)  readw(__ISA_IO_base + val)
 #endif
#endif

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
static long uceiisa_unlocked_ioctl(struct file* filp, unsigned int cmd, unsigned long arg) {
   unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
#else
static int uceiisa_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg) {
   unsigned int minor = MINOR(inode->i_rdev);
#endif
   int status=-1;
   DEV_DATA* pData=NULL;

   if((status = get_devData(filp, &pData)) != 0)
     return status;

   switch(cmd){
   case SET_REGION:
     break;
   case GET_REGION_SIZE:
     status = copy_to_user((unsigned long*)arg, &pData->size, 4);
     if(uceiisa_debug)
       printk(KERN_INFO "uceiisa(%d): uceiisa_ioctl, GET_REGION_SIZE (0x%lx)\n", minor, pData->size);
     break;
   case GET_REGION_MEM:
     status = copy_to_user((unsigned long*)arg, &pData->phys_membase, 4);
     if(uceiisa_debug)
       printk(KERN_INFO "uceiisa(%d): uceiisa_ioctl, GET_REGION_MEM (0x%lx)\n", minor, pData->phys_membase);
     break;
   default:
     return -EINVAL;
   }

   if(status) {
     printk(KERN_WARNING "uceiisa(%d):  failed ioctl (0x%x) with arg %lu.\n",minor,cmd,arg);
     return -EFAULT;
   }
   else
     return 0; 
}

static ssize_t uceiisa_read(struct file* filp, char __user* usr_buf, size_t count, loff_t* ppos) {
  int status=-1;
  u8 usr_cmd=0;
  u8 b_val=0;
  u16 w_val=0;
  u32 d_val=0;
  u32 offset=0;
  unsigned long buf[2]={0,0};
  unsigned int minor=MINOR(filp->f_dentry->d_inode->i_rdev);
  DEV_DATA* pData=NULL;

  if(uceiisa_debug)
    printk(KERN_INFO "uceiisa(%d): uceiisa_read ", minor);

  if((status = get_devData(filp, &pData)) != 0)
    return status;
    
  if((status = copy_from_user(&offset, usr_buf, count)) != 0) {
    printk(KERN_ERR "\nuceiisa(%d):  failed copy_from_user, data (0x%x), status (%d)\n", minor, offset, status); 
    return status;
  }
  usr_cmd = (u8)(offset>>30);
  offset &= ~(0x3<<30);
  if(uceiisa_debug)
    printk("(%d, 0x%x, %d)\n", usr_cmd, offset, (int)count);

  // flush out user buffer
  if((status = copy_to_user(usr_buf, &d_val, 4)) != 0) {
    if(uceiisa_debug)  
      printk(KERN_ERR "uceiisa(%d):  failed to flush user buffer, status (%d)\n", minor, status); 
  } 

  switch(usr_cmd) {
  // read the board's memory
  case 0x0:	  
    switch(count) {
    case 1:
      b_val = readb((char*)pData->kern_membase + offset); 
      if((status = copy_to_user(usr_buf, &b_val, 1)) == 0) {
        if(uceiisa_debug)  
          printk(KERN_INFO "uceiisa(%d):  read byte from board, offset - 0x%x, value - 0x%x\n",minor,offset,b_val);
      } 
      break;
    case 2:
      w_val = readw((unsigned short*)pData->kern_membase + offset); 
      if((status = copy_to_user(usr_buf, &w_val, 2)) == 0) {
        if(uceiisa_debug)  
          printk(KERN_INFO "uceiisa(%d):  read word from board, offset - 0x%x, value - 0x%x\n",minor,offset,w_val);
      }
      break;
    case 4:
      d_val = readl((unsigned long*)pData->kern_membase + offset); 
      if((status = copy_to_user(usr_buf, &d_val, 4)) == 0) {
        if(uceiisa_debug)  
         printk(KERN_INFO "uceiisa(%d):  read dword from board, offset - 0x%x, value - 0x%x\n",minor,offset,d_val);
      }
      break;
    default:
      if(uceiisa_debug)  
        printk(KERN_INFO "uceiisa(%d):  invalid request (cmd: %d, count: %d).\n",minor,usr_cmd,(int)count); 
      return -EINVAL; 
    };
    break;
  case 0x1:
  // device attributes
  case 0x2:
    switch(offset) {
    case 0x1:  // gets the ISA region info
      w_val = sizeof(pData->phys_membase) + sizeof(pData->size);
      if(count < w_val) {
        if(uceiisa_debug)
          printk(KERN_WARNING "uceiisa(%d):  user buffer (0x%x) too small for driver data (0x%x).\n",minor, (int)count, w_val); 
        status = -EINVAL;
      }  
      else
        buf[0] = pData->phys_membase;
        buf[1] = pData->size;
        status = copy_to_user(usr_buf, buf, count);
      break;
    case 0x2:  // gets the IRQ
     #ifdef NO_HW_INTERRUPTS
      printk(KERN_WARNING "uceiisa(%d):  driver not compiled with hardware interrupts enabled.\n", minor);
     #else
      if(count < sizeof(pData->int_data.irq)) {
        if(uceiisa_debug)
          printk(KERN_WARNING "uceiisa(%d):  user buffer size (%d) too small for driver data (%d).\n",minor, (int)count, (int)sizeof(pData->int_data.irq)); 
	status = -EINVAL;
      }
      else
        status = copy_to_user(usr_buf, &pData->int_data.irq, count);
     #endif
      break;
    default:
      if(uceiisa_debug)  
        printk(KERN_INFO "uceiisa(%d):  invalid request (cmd: %d, sub-cmd: %d).\n",minor,usr_cmd,offset); 
      return -EINVAL; 
    };
    break;
  default:
    if(uceiisa_debug)  
      printk(KERN_INFO "uceiisa(%d):  invalid request (cmd: %d).\n",minor,usr_cmd); 
    return -EINVAL; 
  };

  if(status != 0) {
    printk(KERN_ERR "uceiisa(%d):  failed to copy data to user space, status: %d\n", minor, status); 
    return status;
  }

  return 0;
}


static ssize_t uceiisa_write(struct file* filp, const char* usr_buf, size_t count, loff_t* ppos) {
  int status=-1;
  int usr_data=0;
  unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
  DEV_DATA* pData=NULL;
  pid_t cur_pid = ((struct task_struct*)current)->pid;

  if(uceiisa_debug) 
    printk(KERN_INFO "uceiisa(%d): uceiisa_write ", minor);

  if((status = get_devData(filp, &pData)) != 0)
    return status;

  if((status = copy_from_user(&usr_data, usr_buf, 4)) != 0) {
    printk(KERN_ERR "\nuceiisa(%d):  failed to copy data from user buffer, data (0x%x), count (%d), status (%d)\n", minor, usr_data, (int)count, status);
    return status;
  }

  if(uceiisa_debug)
    printk("(%d, %d)\n", (int)count, usr_data);

  switch(count) {
  case 1:
   #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceiisa(%d):  driver not compiled with hardware interrupts enabled.\n", minor);
   #else
    #ifdef HW_INTERRUPTS_SIGNAL
    // disable the PID from receiving signals
    if(usr_data == -1) 
      pData->int_data.sigpid = 0;
    else if(usr_data == 0) 
     pData->int_data.sigpid = cur_pid;
    else
     pData->int_data.sigpid = (pid_t)usr_data;
    #else
    printk(KERN_WARNING "uceiisa:  driver not compiled with signal support for hardware interrupts.\n");
    #endif
   #endif
    break;
  case 2:  // sets the signal number (if -1 ignore, if 0 disable signals from in ISR)
   #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceiisa(%d):  driver not compiled with hardware interrupts enabled.\n", minor);
   #else
    #ifdef HW_INTERRUPTS_SIGNAL
    if(usr_data == -1)
      break;
    if(usr_data == 0) {
      pData->int_data.signal = 0;  // this will disable signals from being generated in the ISR
      if(uceiisa_debug)
        printk(KERN_INFO "uceiisa(%d):  disabled signals from ISR\n", minor);
    }
    else {
      pData->int_data.signal = (unsigned int)usr_data;  // sets the signal value per device for the ISR
      if(uceiisa_debug)
        printk(KERN_INFO "uceiisa(%d):  signal (%d) attached to PID (0x%X).\n", minor, pData->int_data.signal, pData->int_data.sigpid);
    }
    #else
    printk(KERN_WARNING "uceiisa:  driver not compiled with signal support for hardware interrupts.\n");
    #endif
   #endif
    break;
  case 3: // sets the signal value (if -1 ignore, if 0 use the default setting in ISR)
   #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceiisa(%d):  driver not compiled with hardware interrupts enabled.\n", minor);
   #else
    #ifdef HW_INTERRUPTS_SIGNAL
    if(usr_data == -1)
      break;
    if(usr_data == 0) {
      pData->int_data.sigval = -1;
      if(uceiisa_debug) 
        printk(KERN_INFO "uceiisa(%d):  using default sigval for PID (0x%X).\n", minor, pData->int_data.sigpid);
    }
    else {
      pData->int_data.sigval = (int)usr_data;  // sets the signal value per device for the ISR
      if(uceiisa_debug) 
        printk(KERN_INFO "uceiisa(%d):  sigval (0x%X) set for PID (0x%X).\n", minor, pData->int_data.sigval, pData->int_data.sigpid);
    }
    #else
    printk(KERN_WARNING "uceiisa:  driver not compiled with signal support for hardware interrupts.\n");
    #endif
   #endif
    break;
  case 4:  // enables/disables hardware interrupts
   #ifdef NO_HW_INTERRUPTS
    printk(KERN_WARNING "uceiisa(%d):  driver not compiled with hardware interrupts enabled.\n", minor);
   #else
    if((status = intrpt_cntrl(pData, usr_data)) != 0) {
      printk(KERN_ERR "uceiisa(%d):  failed to control hardware interrupts, mode(%d), status(%d).\n", minor, usr_data, status);
      return status;
   }
   #endif
    break; 
  default:
    printk(KERN_WARNING "uceiisa(%d):  invalid write index (%d)\n", minor, (int)count);    
    return -EINVAL;
  }

  return 0;
}


static int uceiisa_mmap(struct file* filp, struct vm_area_struct* vma) {
   int status=-1;
   unsigned long vma_size=0;
   unsigned long phys_addr=0;
   unsigned int minor=MINOR(filp->f_dentry->d_inode->i_rdev);
   pid_t cur_pid=((struct task_struct*)get_current())->pid;
   DEV_DATA* pData=NULL;
 
   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa(%d): uceiisa_mmap\n", minor);

   if((status = get_devData(filp, &pData)) != 0)
    return status;

   if(pData->status & STATUS_MAP) {
     if(uceiisa_debug)
       printk(KERN_WARNING "uceiisa(%d):  device already mapped, PID (0x%X)\n", minor, cur_pid);
     return 0;
   }

   vma_size = (vma->vm_end - vma->vm_start);
   if(vma_size > pData->size) {
      printk(KERN_WARNING "uceiisa(%d): offset error\n", minor);
      return -EINVAL;
   }
   phys_addr = pData->phys_membase + vma->vm_pgoff;
   vma->vm_flags |= (VM_RESERVED | VM_IO);
  #if(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9))
   vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
   if((status = remap_pfn_range(vma, vma->vm_start, phys_addr>>PAGE_SHIFT, vma_size, vma->vm_page_prot)) != 0) {
  #else
   if((status = remap_page_range(vma, vma->vm_start, phys_addr, vma_size, vma->vm_page_prot)) != 0) {
  #endif
     printk(KERN_ERR "uceiisa(%d):  failed to map device - status: %d.\n",minor,status);
     return status;
   }

   pData->status |= STATUS_MAP;

   return 0;
}


static int uceiisa_open(struct inode* inode, struct file* filp) {
   int status=-1;
   unsigned int minor=MINOR(inode->i_rdev);
   struct module* owner=filp->f_op->owner;
   DEV_DATA* pData=NULL;

   if(uceiisa_debug)
     printk(KERN_INFO "uceiisa(%d): uceiisa_open\n",minor);

   if((status = get_devData(filp, &pData)) != 0)
     return status;

   if((pData->status & STATUS_INIT) == 0) {
     if(uceiisa_debug)
       printk(KERN_WARNING "uceiisa(%d):  device not initialized (0x%x)\n", minor, pData->status);
     return 0;
   }

   if(pData->status & STATUS_OPEN) {
      printk(KERN_WARNING "uceiisa(%d):  device already open.\n", minor);
      return -EBUSY;
   }

   if(try_module_get(owner) == 0) {
     if(uceiisa_debug) 
       printk(KERN_ERR "uceiisa(%d):  failed to get module %s.\n", minor, module_name(owner));
     return -ENODEV;
   }

   if((pData->status & STATUS_OPEN) == 0) {
     pData->status |= STATUS_OPEN;
     if(uceiisa_debug)
       printk(KERN_INFO "uceiisa(%d):  device opened\n",minor); 
   }

   filp->private_data = pData;  // use to point to the device data

   return 0;
}


static int uceiisa_release(struct inode *inode, struct file* filp) {
   int status=-1;
   unsigned int minor=MINOR(inode->i_rdev);
   struct module* owner=filp->f_op->owner;
   DEV_DATA* pData=NULL;

   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa(%d): uceiisa_release\n",minor);

   if((status = get_devData(filp, &pData)) != 0)
     return status;

   pData->status &= ~STATUS_OPEN;
   pData->status &= ~STATUS_MAP;
   module_put(owner);

   return 0;
}


static int register_uceiisa(void) {
   int status=-1;

   if(uceiisa_debug) {
      printk(KERN_INFO "uceiisa: register_uceiisa\n");
      printk(KERN_INFO "uceiisa: kernel version 0x%X\n", LINUX_VERSION_CODE);
   }

   // register major and accept a dynamic number
   if((status = register_chrdev(uceiisa_major, CEI_ISA_MODULE, &uceiisa_fops)) < 0) {
     printk(KERN_WARNING "uceiisa: can't get major %d\n", uceiisa_major);
     return status;
   }
   if(uceiisa_major == 0)
     uceiisa_major = status;  // remember the major number

   return 0;
}


static void unregister_uceiisa(void) {
   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa: unregister_uceiisa\n");

   // unregister the device
   unregister_chrdev(uceiisa_major, CEI_ISA_MODULE);
}


static int scan_uceiisa(void){
   unsigned int value, indx, res_indx;
   unsigned long mem_size;
  #ifndef NO_HW_INTERRUPTS
   int req_irq;
  #endif
   int dev_indx, boardType;
   DEV_DATA* pData=NULL;
   unsigned boardID_1553[] = {0x0048,      // sf 1ch (Q104)
                         0x0088,      // sf 2ch (Q104)
                         0x0108,      // sf 4ch (Q104)
                         0x0848,      // mf 1ch (Q104)
                         0x0888,      // mf 2ch (Q104)
                         0x0908,      // mf 4ch (Q104)
                         0x5c00,      // sf 1ch (ISA)
                         0x7c00,      // sf 2ch (ISA)
                         0x5f00,      // mf 1ch (ISA)
                         0x7f00};     // mf 2ch (ISA)
   char *boardDesc_1553[] = {"1-Channel Single Function",
                             "2-Channel Single Function",
                             "4-Channel Single Function",
                             "1-Channel Multi Function",
                             "2-Channel Multi Function",
                             "4-Channel Multi Function",
                             "1-Channel Single Function",
                             "2-Channel Single Function",
                             "1-Channel Multi Function",
                             "2-Channel Multi Function"};
   unsigned boardID_x20[] = {0x00,       // CEI-220
                             0x10,       // CEI-420
                             0x20,       // CEI-220 with ch 5=561 6-wire
                             0x30,       // CEI-420-70J with -717 reduced
			     0x40,       // CEI-420 with HBP xmit/Holt
			     0x50,       // CEI-420A 12 MHz, 16 in disc
			     0x60,       // CEI-420A with 16 MHz clock
			     0x70};      // CEI-420A xxJ 16 MHz clock
   char *boardDesc_x20[] = {"CEI-220",
                            "CEI-420",
                            "CEI-220 6-Wire",
                            "CEI-420-70J",
                            "CEI-420-XXJ",
                            "CEI-420A-12",
                            "CEI-420A",
                            "CEI-420A-XXJ"};
   mem_size=boardType=dev_indx=res_indx=0;
   
   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa:  scan_uceiisa\n");

   while(basemem[res_indx] != -1) {
      // Check for a Q104-1553 Board ID
      value = (isa_readw(basemem[res_indx])) & 0x0FFF;
      for(indx=0;indx<6;indx++) {
        if(value == boardID_1553[indx]) {
    	  boardType = 0x1553;
          mem_size = 0x4000;
          if(uceiisa_debug)
            printk(KERN_INFO "uceiisa:  found a Q104-1553 - %s (0x%x)\n",boardDesc_1553[indx], value);
	      break;
        }
      }
      // Check for a ISA-1553 Board ID
      if(boardType == 0) {
        value = (isa_readw(basemem[res_indx])) & 0xFF00;
	for(indx=6;indx<10;indx++) {
          if(value == boardID_1553[indx]) {
            boardType = 0x1553;
            mem_size = 0x4000;
            if(uceiisa_debug)
              printk(KERN_INFO "uceiisa:  found a ISA-1553 - %s (0x%x)\n",boardDesc_1553[indx], value);
	        break;
	      }
        }
      }	

      // Check for a CEI-x20 Board ID
      //  - configuration register 1 : board identifier
      //  - configuration register 2 : value has to be 0x0
      //  - configuration register 3 : value of bits 4-7 has to be 0x0
      if(boardType == 0) {
        value = isa_readb(basemem[res_indx] + 0x80A);
        for(indx=0;indx<8;indx++) {
          if(value == boardID_x20[indx]) {
            if(isa_readb(basemem[res_indx] + 0x80C) == 0) {
	      if(((isa_readb(basemem[res_indx] + 0x80E)) & 0xF0) == 0) {
                boardType = 0x20;
                mem_size = 0x80f;
                if(uceiisa_debug)
                  printk(KERN_INFO "uceiisa:  found a CEI-x20 ISA board = %s (0x%x)\n", boardDesc_x20[indx], value);
                break;
              }
	    }
	  }
	}
      }

      if(boardType == 0)
        return dev_indx ? 0 : -ENODEV;

      // allocate memory for device
      if((pData = (DEV_DATA*)kmalloc(sizeof(DEV_DATA), GFP_KERNEL)) == NULL) {
        printk(KERN_ERR "uceiisa:  failed to allocate kernel memory.\n");
        return -ENOMEM;
      }
      memset_io(pData, 0, sizeof(DEV_DATA));

      pData->minor = dev_indx++;
      pData->phys_membase = basemem[res_indx];
      if(mem_size < PAGE_SIZE)
        pData->size = PAGE_SIZE;
      else
        pData->size = mem_size;
      pData->device = boardType;

      // map ISA card memory into kernel memory
      if((pData->kern_membase = (unsigned long)ioremap(pData->phys_membase, pData->size)) == 0) {
        printk(KERN_ERR "uceiisa(%d):  failed ioremap for membase 0x%lx\n", pData->minor, pData->phys_membase);
        return -ENOMEM;
      }
      if(uceiisa_debug)
        printk(KERN_INFO "uceiisa(%d):  phys_membase 0x%lx, kern_membase 0x%lx\n", pData->minor, pData->phys_membase, pData->kern_membase);

     #ifndef NO_HW_INTERRUPTS
      pData->int_data.ch = (value >> 6) & 0x7;
      if((boardType == 0x1553) && (irq[res_indx] != -1)) {
        req_irq = irq[res_indx]; 
        if(req_irq < 3 || req_irq > 15 || req_irq == 8 || req_irq == 13) {
          printk(KERN_ERR "uceiisa(%d):  IRQ %d not supported.\n", pData->minor, req_irq);
          req_irq = -1;
        }
	if(indx <= 5) {
          if(req_irq == 4 || req_irq == 6) {
            printk(KERN_ERR "uceiisa(%d):  IRQ %d not supported for Q104-1553.\n", pData->minor, req_irq);
            req_irq = -1;
	      }
        }
        pData->int_data.irq = req_irq;
        for(indx=0;indx<pData->int_data.ch;indx++) 
          pData->int_data.hwr_chan_addr[indx] = (pData->kern_membase + (indx*HWR_CHAN_OFFSET) + HWR_OFFSET);
      }
     #endif
      pData->status |= STATUS_INIT;
      pdev_data[pdev_indx++] = pData;
      res_indx++;
   } 

   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa(%d):  found %i device(s)\n", pData->minor, dev_indx);

   return dev_indx ? 0 : -ENODEV;
}


static int __init uceiisa_init_module(void) {
   int status=-1;
  #ifndef NO_HW_INTERRUPTS
   char buf[10]="";
  #endif

   printk(KERN_INFO "uceiisa:  %s", version);
   if(uceiisa_debug) 
     printk(KERN_INFO "uceiisa: init_module\n");

   if(basemem[0] == -1) {
     printk(KERN_ERR "uceiisa: basemem [1-4] are mandatory\n");
     return -EINVAL;
   }

   // register the char device 
   if((status = register_uceiisa()) != 0) 
     return status;
   
   // scan the ISA addresses for the devices, if unsuccesfull unregister
   if((status = scan_uceiisa()) != 0)
     unregister_uceiisa();

  #ifndef NO_HW_INTERRUPTS
   #ifdef HW_INTERRUPTS_SIGNAL
   strcat(buf, "signal");
   #endif
   if(uceiisa_debug) 
     printk(KERN_INFO "uceiisa:  'hardware interrupts' (%s) support enabled.\n", buf);
  #else
    if(uceiisa_debug) 
    printk(KERN_INFO "uceiisa:  'hardware interrupts' support disabled.\n");
  #endif

   return status;
}


static void __exit uceiisa_exit_module(void) {
   int i;
   DEV_DATA* pData=NULL;

   if(uceiisa_debug)
     printk(KERN_INFO "uceiisa: uceiisa_exit_module\n");

   // free memory 
   for(i=0;i<pdev_indx;i++) {
     if((pData = pdev_data[i]) == NULL)
       continue;
     if(uceiisa_debug)
       printk(KERN_INFO "uceiisa(%d):  release device\n", pData->minor);
    #ifndef NO_HW_INTERRUPTS
     intrpt_cntrl(pData, 0);
    #endif
     if(pData->kern_membase)
       iounmap((void*)pData->kern_membase);
     kfree(pData);
     pdev_data[i] = NULL;
   }

   // unregister the char device
   unregister_uceiisa();
}


#ifndef NO_HW_INTERRUPTS
// handles the hardware interrupts generated by the board
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
static irqreturn_t uceiisa_1553_isr(int irq, void *data) {
#else
static irqreturn_t uceiisa_1553_isr(int irq, void *data, struct pt_regs *regs) {
#endif
   int i, int_count=0;
   unsigned short cntrl_reg=0;
   DEV_DATA* pData=(DEV_DATA*)data;
  #ifdef HW_INTERRUPTS_SIGNAL
   struct siginfo siginfo;
   struct task_struct* ts;
  #endif

   if(pData == NULL) {
     if(uceiisa_debug > 1)
       printk(KERN_ERR "uceiisa:  uceiisa_1553_isr - no data pointer, IRQ(%d)\n", irq);
     return IRQ_NONE;
   }

   if(uceiisa_debug > 1)
     printk(KERN_INFO "uceiisa(%d): uceiisa_1553_isr\n", pData->minor);

   // return if hardware interrupts are not enabled
   if(!(pData->status & STATUS_INT_ENABLE)) {
     if(uceiisa_debug > 1)
       printk(KERN_ERR "uceiisa(%d):  received non-asserted interrupt, IRQ (%d) shared\n", pData->minor, irq);
     return IRQ_NONE;
   }

   if(pData->int_data.irq != irq) {
     if(uceiisa_debug > 1)
       printk(KERN_ERR "uceiisa(%d):  IRQ mismatch (%d, %d)\n", pData->minor, pData->int_data.irq, irq);
     return IRQ_NONE;
   }

  #ifdef HW_INTERRUPTS_SIGNAL
   siginfo.si_code = SI_QUEUE;
   siginfo.si_errno = 0;
   siginfo.si_signo = pData->int_data.signal;  // set by the user
  #endif

   for(i=0;i<pData->int_data.ch;i++) {
     // read the 1553_control_register on channel
     cntrl_reg = readw((unsigned short*)pData->int_data.hwr_chan_addr[i]);
     // check for interrupt
     if(cntrl_reg & 0x200) {
       // write a 0 to the write_interrupt_bit
       writew(0, (unsigned short*)(pData->int_data.hwr_chan_addr[i] + 0x16));
       if(uceiisa_debug > 2)
         printk(KERN_INFO "uceiisa(%d):  valid interrupt - IRQ (%d), chan (%d), cntrl_reg (0x%x).\n", pData->minor, pData->int_data.irq, i, cntrl_reg);
       int_count++;
      #ifdef HW_INTERRUPTS_SIGNAL
       // check if signals have been enabled
       if(pData->int_data.signal <= 0)
         continue;
       if(pData->int_data.sigpid <= 1) {
         pData->int_data.signal = 0;  // disable signal
         if(uceiisa_debug > 1)
           printk(KERN_WARNING "uceiisa(%d):  invalid PID (0x%X), disabling signals from ISR\n",pData->minor,pData->int_data.sigpid);
         continue;
       }
       if(pData->int_data.sigval == -1)
         siginfo.si_int = pData->minor;  // send the device number
       else
         siginfo.si_int = pData->int_data.sigval;  // user provided
      #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
       ts = get_pid_task(find_vpid(pData->int_data.sigpid), PIDTYPE_PID);
      #else
       rcu_read_lock();
      #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
       ts = pid_task(find_vpid(pData->int_data.sigpid), PIDTYPE_PID);
      #else
       ts = find_task_by_pid(pData->int_data.sigpid); 
      #endif
       rcu_read_unlock();
      #endif
       if(!ts)
         continue; // -ESRCH;
       send_sig_info(pData->int_data.signal, &siginfo, ts);
       if(uceiisa_debug > 1)
         printk(KERN_INFO "uceiisa(%d):  signal(%d) to PID(0x%X) with val(0x%X)\n", pData->minor, siginfo.si_signo, pData->int_data.sigpid, siginfo.si_int);
      #endif
     }
   }

   if(int_count == 0) {
     if(uceiisa_debug > 2)
       printk(KERN_WARNING "uceiisa(%d):  spurious interrupt - IRQ (%d,%d).\n",pData->minor,irq,pData->int_data.irq);
     return IRQ_NONE;
   }

   return IRQ_HANDLED;
}
#endif


// interal function
static int get_devData(struct file* filp, DEV_DATA** pData) {
  unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
    
  if(minor >= pdev_indx) {
    printk(KERN_ERR "uceiisa(%d):  no device for minor (%d).\n", minor, pdev_indx);
    return -ENODEV;
  }
  if((*pData = (DEV_DATA*)(filp->private_data)) == NULL) {
    if((*pData = pdev_data[minor]) == NULL) {  // for uceiisa_open
      printk(KERN_ERR "uceiisa(%d):  no device data.\n", minor);
      return -ENODEV;
    }
  }
  if((*pData)->minor != minor) {
    printk(KERN_ERR "uceiisa(%d):  minor number mismatch (%d).\n", minor, (*pData)->minor);
    return -ENODEV;
  }

  return 0;
}

#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl(DEV_DATA* pData, int mode) {
  int i, status;
  unsigned short wVal=0;

  if(uceiisa_debug >= 1)
    printk(KERN_INFO "uceiisa(%d):  hardware interrupt mode (%d)\n", pData->minor, mode);

  if(pData->device != 0x1553) {
    if(uceiisa_debug >= 1)
      printk(KERN_ERR "uceiisa(%d): hardware interrupts not supported on device (0x%X)\n", pData->minor, pData->device);
    return -EINVAL;
  }

  if(pData->int_data.irq  == -1) {
    printk(KERN_ERR "uceiisa(%d): non-supported IRQ assigned\n", pData->minor);
    return -EINVAL;
  }

  switch(mode) {
  case 0:  // check if hardware interrupts disabled
    if(!(pData->status & STATUS_INT_ENABLE)) {
      if(uceiisa_debug)
        printk(KERN_INFO "uceiisa(%d):  hardware interrupts already disabled\n", pData->minor);
      return 0;
    }
    break;
  case 1:  // check if hardware interrupts enabled
    if(pData->status & STATUS_INT_ENABLE) {
      if(uceiisa_debug)
        printk(KERN_INFO "uceiisa(%d):  hardware interrupts already enabled\n", pData->minor);
      return 0;
    }
    break;
  default:
    printk(KERN_WARNING "uceiisa(%d): invalid hardware interrupt mode (%d)\n", pData->minor, mode);
    return -EINVAL;
  };

  // control each available channel for hardware interrupts
  for(i=0;i<pData->int_data.ch;i++) {
     wVal = readw((unsigned short*)pData->int_data.hwr_chan_addr[i]);
    if(mode == 0) // disable
      writew(wVal & ~0x4000, (unsigned short*)(pData->int_data.hwr_chan_addr[i]));  // disable
    else if(mode == 1)  // enable
      writew(wVal | 0x4000, (unsigned short*)(pData->int_data.hwr_chan_addr[i]));
    writew(0, (unsigned short*)(pData->int_data.hwr_chan_addr[i] + 0x16));  // clear register
  }

  if(mode == 0) {
    free_irq(pData->int_data.irq, (void*)pData); 
    if(uceiisa_debug > 1)
      printk(KERN_INFO "uceiisa(%d):  unregistered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->int_data.irq, (unsigned long) pData);
  }
  else if(mode == 1) {
   #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
    if((status = request_irq(pData->int_data.irq, uceiisa_1553_isr, IRQF_DISABLED|IRQF_SHARED, CEI_ISA_MODULE, (void*)pData)) != 0) {
   #else
    if((status = request_irq(pData->int_data.irq, uceiisa_1553_isr, SA_INTERRUPT|SA_SHIRQ, CEI_ISA_MODULE, (void*)pData)) != 0) {
   #endif
      printk(KERN_ERR "uceiisa(%d):  unable to register interrupt handler IRQ (%d), status (%d).\n",pData->minor, pData->int_data.irq, status);
      return -1;
    }
    else {
      if(uceiisa_debug > 1)
        printk(KERN_INFO "uceiisa(%d):  registered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->int_data.irq, (unsigned long) pData);
    }
  }

  switch(mode) {
  case 0:
    pData->status &= ~STATUS_INT_ENABLE;
    if(uceiisa_debug)
      printk(KERN_INFO "uceiisa(%d):  disabled hardware interrupts\n", pData->minor);
    break;
  case 1:
    pData->status |= STATUS_INT_ENABLE;
    if(uceiisa_debug)
      printk(KERN_INFO "uceiisa(%d):  enabled hardware interrupts\n", pData->minor);
    break;
  default:
    if(uceiisa_debug)
      printk(KERN_WARNING "uceiisa(%d):  invalid mode (%d) for hardware interrupt\n", pData->minor, mode);
    return -EINVAL;
  };

  return 0;
}
#endif

