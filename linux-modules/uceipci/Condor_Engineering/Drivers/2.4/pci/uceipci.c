/*============================================================================*
 * FILE:                     U C E I P C I . C
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
 *
 *===========================================================================*
 *
 * FUNCTION:   Linux PCI Device Driver.
 *
 *             This module provides the interface between BusTools and Abaco
 *             Systems's avionics Linux PCI products.
 *
 *             This module performs the actual I/O and memory-mapping functions
 *             needed to support operation in a Linux environment.
 *
 *             Since this file is used by multiple products use care when
 *             performing modifications.
 *
 * EXTERNAL ENTRY POINTS:
 *    open                    Opens a 1553 or ARINC device.
 *
 *    close                   Closes a 1553 or ARINC device
 *
 *    mmap                    maps device memory into user space.
 *
 *    lseek                   position the file pointer to specified offset
 *
 *    read                    reads data from the device at the file pointer
 *                            location.
 *
 *    write                   write data to the device at the file poitner 
 *                            location.
 *
 *    ioctl                   performs a variety of utility functions:
 *
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  2.36 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/13/2003   Add support for the QPCI-1553. bch
  02/23/2005   Changed name to "uceipci". bch
  08/11/2005   Modified scan_uceipci to determine vectors instead of IRQ on
                SMP systems. bch
  01/19/2006   Modified for 64-bit support. bch
  05/18/2006   added SET_SIGNAL and SET_SIGVAL to ioctls, replaced kill_proc
                with send_sig_info cei_isr. bch
  03/27/2007   added support for the QPCX-1553. bch
  07/09/2007   added support for the CEI-430 and the CEI-530. modified
                cleanup_module. bch
  08/06/2007   added support for the RCEI-830RX and EPMC-1553. bch
  12/12/2007   modified cei_1553_int to return if pid not set. replace module
                parameter "debug" with "uceipci_debug". bch
  01/17/2008   added support for the RAR-CPCI, P-708, P-SER, P-MIO, and P-DIS.
                renamed RCEI-830RX to R830RX. bch
  11/18/2008   removed support to kernels prior to 2.4. modified uceipci_ioctl
                by replacing "pcibios_read_config_" with "pci_read_config_".
                modified IRQ determination in scan_uceipci. bch
  04/10/2009   modified uceipci_ioctl to set default values for the device's 
                pid and sigval. bch	
  02/01/2010   modified uceipci_mmap to support PowerPC, modified uceipci_ioctl
                by using a 32-bit value for put_user and get_user. bch
  10/11/2011   added support for CEI-430A. added "cei_x30_int" and wait queue
                functionality for hardware interrupt support on CEI-x30
	       	boards. modified cei_1553_int to lock/unlock for
	       	find_task_by_pin. modified struct cei_int_data. added 64-bit
                kernel support. bch
  04/26/2013   added support for CEI-830X820. bch
  06/20/2013   added support for UCA32. modified cei_1553_int. bch
  11/19/2013   added support for CEI-830A. bch
  08/19/2015   changed 0x80A to 0x830A in CONDOR_DEVICE_ID for the CEI-830A. bch
*/


#include <linux/version.h>
#include <linux/config.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/signal.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#ifdef MODULE
 #ifdef MODVERSIONS
  #include <linux/modversions.h>
 #endif
 #include <linux/module.h>
#else
 #define MOD_INC_USE_COUNT
 #define MOD_DEC_USE_COUNT
#endif

#define MAX_BOARDS 8

#define UCEIPCI_MAGIC ((PCI_VENDOR_ID_CONDOR<<16)| 0x1234)

// ioctls
#define SET_REGION           0x13c61
#define GET_REGION_MEM       0x13c62
#define GET_REGION_SIZE      0x13c63
#define GET_DEVICE_ID        0x13c64
#define GET_IRQ              0x13c65
#define SET_PID              0x13c66
#define SET_SIGNAL           0x13c67
#define SET_SIGVAL           0x13c68
#define GET_PCIREGION_BYTE   0x13c69
#define GET_PCIREGION_WORD   0x13c6a
#define GET_PCIREGION_DWORD  0x13c6b
#define SET_INTERRUPT_MODE   0x13c6c
#define SET_WAIT_QUEUE       0x13c6d


#ifdef NEW_CONDOR_PCI
 #define CONDOR_DEVICE "uceipci"
 #define PCI_VENDOR_ID_CONDOR  0x13c6
 unsigned CONDOR_DEVICE_ID[]={0x1553,0x1553,0x1553,0x1554,0x1555,0x1556,0x1003,0x430,0x430A,0x520,0x530,0x620,0x820,0x821,0x830,0x831,0x630,0x832,0x830A,0x708,0x1004,0x1005,0x1006};
 int number_of_boards=sizeof(CONDOR_DEVICE_ID)/4;
 char CONDOR_DEVICE_NAME[][20]={"pci1553","q1041553p","qpmc1553","qpci1553","qcp1553","qpcx1553","epmc1553","cei430","cei430A","cei520","rcei530","cei620","cei820","cei821","cei830","r830rx","rar-cpci","cei830x820","cei830A","p-708","p-ser","p-mio","p-dis"};
 int mbase[]={0x0,0x0,0x0,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2,0x2};
 unsigned int board_mask[]={0xc000,0x3e,0x3e,0x3e,0x3e,0x3e,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 int board_value[]={0x0,0xa,0x2,0x6,0xe,0x6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
 int int_support[]={1,1,1,1,1,1,1,1,1,0,1,0,0,0,1,1,1,1,1,0,0,0,0};
 static void cei_1553_int(int, void *, struct pt_regs *);
 static void cei_x30_int(int, void *, struct pt_regs *);
#endif

static char *version="uceipci.c:v2.35 - (11/19/2013) Brian Hill(avionics.support@abaco.com)\n";

static const char invalid_magic[]= KERN_CRIT "uceipci: invalid magic value\n";

#define VALIDATE_STATE(s)                     \
({                                            \
   if (!(s) || (s)->magic != UCEIPCI_MAGIC) {  \
      printk(invalid_magic);                  \
      return -ENXIO;                          \
   }                                          \
})

static int uceipci_debug=0;

#define uceipci_major major
#ifndef UCEIPCI_MAJOR
 #define UCEIPCI_MAJOR 0 /* dynamic major by default */
#endif
int uceipci_major=UCEIPCI_MAJOR;

#if defined(MODULE)
 MODULE_AUTHOR("Brian Hill <avionics.support@abaco.com>");
 MODULE_DESCRIPTION("Abaco Systems avionics universal Linux kernel 2.4 PCI device driver v2.35.  Do not distribute the source code.");
 MODULE_PARM(uceipci_debug, "i");
 MODULE_PARM(major, "i");
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
  MODULE_LICENSE("Proprietary Abaco Systems. Source code supplied. Send bug reports to avionics.support@abaco.com>");
 #endif
#endif

static u32 addresses[] = {
   PCI_BASE_ADDRESS_0,
   PCI_BASE_ADDRESS_1,
   PCI_BASE_ADDRESS_2,
   PCI_BASE_ADDRESS_3,
   PCI_BASE_ADDRESS_4,
   PCI_BASE_ADDRESS_5
};

typedef struct pciregion_state {
   unsigned long membase;
   unsigned long size;
   int type;
} pciregion_state;

typedef struct uceipci_state {
  unsigned int magic;
  struct uceipci_state *next;
  int inuse;
  unsigned long card_membase;
  unsigned long phys_membase;
  unsigned long size;
  unsigned int num_regs;
  unsigned int minor;
  unsigned int device_id;
  char cei_irq;
  unsigned char pci_bus;
  unsigned char pci_device_fn;
  pciregion_state regions[6];
} uceipci_state;

typedef struct CEI_INT_DATA {
  int int_board;
  int irq;
  unsigned int ch0_cnt;
  unsigned int ch1_cnt;
  unsigned int ch2_cnt;
  unsigned int ch3_cnt;
  int count;
  volatile int int_occurred;
  unsigned int pid;
  unsigned int signal;
  int sigval;
  unsigned int minor;
  int notifier_mode;
  wait_queue_head_t isr_wait_q;
  unsigned int cur_intrpt;
  unsigned int last_intrpt;
} cei_int_data;
 
static cei_int_data int_data[MAX_BOARDS];
static int int_count=0;
static int icount=0;
int pci_region=0;
static uceipci_state *devs=NULL;

#if defined(CONFIG_X86_64) || defined(CONFIG_PPC64)
extern int sys_ioctl(unsigned int, unsigned int, unsigned long);
extern int register_ioctl32_conversion(unsigned int cmd, int (*handler)(unsigned int, unsigned int, unsigned long,struct file *));
extern int unregister_ioctl32_conversion(unsigned int cmd);
static int register_ioctl32_uceipci(void);
static int unregister_ioctl32_uceipci(void);
#endif

static int scan_uceipci(void);

static loff_t uceipci_lseek(struct file * file, loff_t offset, int orig) {
   uceipci_state *s = (uceipci_state *)file->private_data;
   long newpos = 0;

   VALIDATE_STATE(s);

   if(uceipci_debug)
     printk(KERN_INFO "uceipci: uceipci_lseek\n");

   switch(orig) {
   case 0: /* SEEK_SET */
     newpos = offset;
     break;
   case 1: /* SEEK_CUR */
     newpos = file->f_pos + offset;
     break;
   default:
     /* can't happen */
    return -EINVAL;
   }
   if(newpos < 0)
     return -EINVAL;

   /* update file pointer */
   file->f_pos = newpos;

   return newpos;
}

static ssize_t uceipci_read(struct file *file, char *buf, size_t count, loff_t *ppos) {
   uceipci_state *s = (uceipci_state *)file->private_data;
   unsigned long p = *ppos;
   unsigned long end_mem;

   if(uceipci_debug)
     printk(KERN_INFO "uceipci: uceipci_read\n");

   VALIDATE_STATE(s);

   /* get highest physical address */
   end_mem = __pa(high_memory);

   /* sanity check */
   if(p >= end_mem)
     return 0;
   if(count > end_mem - p)
     count = end_mem - p;

   /* copy from kernel to user space */
   if(copy_to_user(buf, (void*)(s->phys_membase + p), count))
     return -EFAULT;

   /* update counter and file pointer */
   *ppos += count;

   /* return number of bytes read */
   return count;
}

static ssize_t uceipci_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
   uceipci_state *s = (uceipci_state *)file->private_data;
   unsigned long p = *ppos;
   unsigned long end_mem;

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_write\n");

   /* validate state */
   VALIDATE_STATE(s);

   /* get highest physical address */
   end_mem = __pa(high_memory);

   /* sanity check */
   if (p >= end_mem)
      return 0;
   if (count > end_mem - p)
      count = end_mem - p;

   /* copy from user to kernel space */
   if (copy_from_user((void*)(s->phys_membase + p), buf, count))
      return -EFAULT;

   /* update file pointer */
   *ppos += count;

   /* return number of bytes written */
   return count;
}

static int uceipci_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {
   unsigned long offset;
   int status, valued;
   short valuew;
   char valueb;
   uceipci_state* s=(uceipci_state*)file->private_data;

   VALIDATE_STATE(s);

   switch(cmd) {
   case GET_REGION_SIZE:
     put_user(s->regions[pci_region].size,(unsigned int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_REGION_SIZE (0x%lx)\n",s->regions[pci_region].size);
     break;
   case GET_REGION_MEM:
     put_user(s->regions[pci_region].membase,(unsigned int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_REGION_MEM (0x%lx)\n",s->regions[pci_region].membase);
     break;
   case GET_DEVICE_ID:
     put_user(s->device_id,(int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_DEVICE_ID (0x%x)\n",s->device_id);
     break;
   case GET_IRQ:
     put_user(s->cei_irq,(int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_IRQ (%d)\n",s->cei_irq);
     break;
   case SET_REGION:
     get_user(pci_region,(int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, SET_REGION (%u)\n",pci_region);
     break;
   case SET_PID:
     get_user(int_data[s->minor].pid,(unsigned int*)arg);
     if(int_data[s->minor].pid <= 0)
       int_data[s->minor].pid = ((struct task_struct*)current)->pid;
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, SET_PID (%d)\n",int_data[s->minor].pid);
     break;
   case GET_PCIREGION_BYTE:
     get_user(offset,(unsigned int*)arg);
     pci_read_config_byte(pci_find_slot(s->pci_bus, s->pci_device_fn), offset, &valueb);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_PCIREGION_BYTE (0x%lx:0x%x)\n",offset,valueb);
     put_user(valueb, (unsigned int*)arg);
     break;
   case GET_PCIREGION_WORD:
     get_user(offset,(unsigned int*)arg);
     pci_read_config_word(pci_find_slot(s->pci_bus, s->pci_device_fn), offset*2, &valuew);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_PCIREGION_WORD (0x%lx:0x%x)\n",offset,valuew);
     put_user(valuew, (unsigned int*)arg);
     break;
   case GET_PCIREGION_DWORD:
     get_user(offset,(unsigned int*)arg);
     pci_read_config_dword(pci_find_slot(s->pci_bus, s->pci_device_fn), offset*4, &valued);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, GET_PCIREGION_DWORD (0x%lx:0x%x)\n",offset,valued);
     put_user(valued, (unsigned int*)arg);
     break;
   case SET_SIGNAL:
     get_user(int_data[s->minor].signal,(unsigned int*)arg);
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, SET_SIGNAL (%d)\n",int_data[s->minor].signal);
     break;
   case SET_SIGVAL:
     get_user(int_data[s->minor].sigval,(int*)arg);
     if(int_data[s->minor].sigval <= 0)
       int_data[s->minor].sigval = s->minor;
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, SET_SIGVAL (%d)\n",int_data[s->minor].sigval);
     break;
   case SET_INTERRUPT_MODE:
     get_user(valued,(int*)arg);
     if(uceipci_debug) 
       printk(KERN_INFO "uceipci: uceipci_ioctl, SET_INTERRUPT_MODE (%d)\n", valued);
     if(valued == 1) {
       if(int_data[s->minor].notifier_mode == 1) {
         int_data[s->minor].cur_intrpt = 0;
         int_data[s->minor].last_intrpt = 0;
       }
     }
     else if(valued == 0) { 
       if(int_data[s->minor].notifier_mode == 1) {
         if(uceipci_debug) 
           printk(KERN_INFO "uceipci: uceipci_ioctl, HWINT count - %d\n", int_data[s->minor].cur_intrpt);
         int_data[s->minor].cur_intrpt = -1;
         wake_up_interruptible(&(int_data[s->minor].isr_wait_q));
       }
       else {
         if(uceipci_debug) 
           printk(KERN_INFO "uceipci: uceipci_ioctl, HWINT count - ch0: %d, ch1: %d, ch2: %d, ch3:%d\n", int_data[s->minor].ch0_cnt, int_data[s->minor].ch1_cnt, int_data[s->minor].ch2_cnt, int_data[s->minor].ch3_cnt);
       }   
     }
     break;
   case SET_WAIT_QUEUE:
     if(int_data[s->minor].last_intrpt == int_data[s->minor].cur_intrpt) {
       status = wait_event_interruptible(int_data[s->minor].isr_wait_q, (int_data[s->minor].last_intrpt != int_data[s->minor].cur_intrpt));
       if(status != 0) {
         if(uceipci_debug) 
           printk(KERN_ERR "uceipci: uceipci_ioctl, SET_WAIT_QUEUE - interrupted by signal (status: %d)\n", status);
         return status;
       }
       if(uceipci_debug) 
         printk(KERN_INFO "uceipci: uceipci_ioctl, SET_WAIT_QUEUE (%d/%d)\n", int_data[s->minor].last_intrpt, int_data[s->minor].cur_intrpt);
       int_data[s->minor].last_intrpt = int_data[s->minor].cur_intrpt;
     }
     break;
   default:
     if(uceipci_debug)
       printk(KERN_INFO "uceipci: uceipci_ioctl, invalid (cmd: 0x%x)\n",cmd);
     return -EINVAL;
   };

   return 0;
}

static void uceipci_vma_open(struct vm_area_struct * vma) {
   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_vma_open\n");
   MOD_INC_USE_COUNT;
}

static void uceipci_vma_close(struct vm_area_struct * vma) {
   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_vma_close\n");
   MOD_DEC_USE_COUNT;
}

static struct vm_operations_struct uceipci_vm_ops = {
   uceipci_vma_open,
   uceipci_vma_close,
};

static int uceipci_mmap(struct file *file, struct vm_area_struct * vma) {
   uceipci_state *s = (uceipci_state *)file->private_data;
   unsigned long size, offset;
   int pci_region_idx=pci_region;

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_mmap\n");

   VALIDATE_STATE(s);
   offset = s->regions[pci_region_idx].membase + vma->vm_pgoff;
   size = vma->vm_end - vma->vm_start;
   if (vma->vm_pgoff + size > s->regions[pci_region_idx].size) {
     printk(KERN_INFO "uceipci: offset error\n");
     return -EINVAL;
   }
   vma->vm_flags |= VM_IO;
   // disable caching
  #if defined(__powerpc__)
   pgprot_val(vma->vm_page_prot) = (pgprot_val(vma->vm_page_prot) | _PAGE_NO_CACHE | _PAGE_GUARDED);
  #else
   pgprot_val(vma->vm_page_prot) = (pgprot_val(vma->vm_page_prot) | _PAGE_PCD | _PAGE_PWT);
  #endif

  // if Red Hat Linux 9 or Red Hat Workstation 3.0  
  #if LINUX_VERSION_CODE == KERNEL_VERSION(2,4,20) || LINUX_VERSION_CODE == KERNEL_VERSION(2,4,21)
   if (remap_page_range(vma, vma->vm_start, offset, size, vma->vm_page_prot))
  #else
   if (remap_page_range(vma->vm_start, offset, size, vma->vm_page_prot))
  #endif
   {
      printk(KERN_INFO "uceipci: remap error\n");
      return -EAGAIN;
   }
   vma->vm_file = file;

   if (vma->vm_ops)
   {
      printk(KERN_INFO "uceipci: vm_ops error\n");
      return -EINVAL;
   }
   
   vma->vm_ops = &uceipci_vm_ops;
   MOD_INC_USE_COUNT;

   return 0;
}

static int uceipci_open(struct inode *inode, struct file *file) {
   int minor = MINOR(inode->i_rdev);
   uceipci_state *s = devs;

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_openned ok\n");

   /* search for the state vector */
   while (s && s->minor != minor)
      s = s->next;

   /* did we find one? */
   if (!s)
      return -ENODEV;

   /* validate state */
   VALIDATE_STATE(s);

   /* return EBUSY, if it's already been opened */
   if (s->inuse)
      printk(KERN_INFO "Opening device in use\n");
   else
      s->inuse++;

   /* use file->private_data to point to the device data */
   file->private_data = s;

   /* increase reference count */
   MOD_INC_USE_COUNT;

   return 0;
}

static int uceipci_release(struct inode *inode, struct file *file) {
   uceipci_state *s = (uceipci_state *)file->private_data;

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: uceipci_release\n");

   /* validate state */
   VALIDATE_STATE(s);

   /* no longer in use */
   s->inuse = 0;

   /* decrease reference count */
   MOD_DEC_USE_COUNT;

   return 0;
}

static struct file_operations uceipci_fops = {
   THIS_MODULE,
   uceipci_lseek,     /* lseek   */
   uceipci_read,      /* read    */
   uceipci_write,     /* write   */
   NULL,             /* readdir */
   NULL,             /* poll    */
   uceipci_ioctl,     /* ioctl   */
   uceipci_mmap,      /* mmap    */
   uceipci_open,      /* open    */
   NULL,             /* flush   */
   uceipci_release,   /* release */
   NULL              /* fsync   */
};

static int register_uceipci(void) {
   int result;
   if (uceipci_debug)
   {
      printk(KERN_INFO "uceipci: register_uceipci\n");
      printk(KERN_INFO "uceipci: KERNEL VERSION %x\n",LINUX_VERSION_CODE);
   }

   /* register major and accept a dynamic number */
   result = register_chrdev(uceipci_major, CONDOR_DEVICE, &uceipci_fops);
   if (result < 0) {
      printk(KERN_WARNING "uceipci: can't get major %d\n", uceipci_major);
      return result;
   }

   /* remember the major number */
   if(uceipci_major == 0)
     uceipci_major = result; /* dynamic */

  #if defined(CONFIG_X86_64) || defined(CONFIG_PPC64)
   result = register_ioctl32_uceipci();
  #endif

   return result;
}

static void unregister_uceipci(void) {
   if (uceipci_debug)
      printk(KERN_INFO "uceipci: unregister_uceipci\n");

  #if defined(CONFIG_X86_64) || defined(CONFIG_PPC64)
   unregister_ioctl32_uceipci();
  #endif

   /* unregister the device */
   unregister_chrdev(uceipci_major, CONDOR_DEVICE);
}

int init_module(void) {
   int result;

   printk(KERN_INFO "%s", version);

   if (uceipci_debug) 
      printk(KERN_INFO "uceipci: init_module\n");

   /* register the char device */
   result = register_uceipci();
   if (result < 0) {
      return 0;
   }

   /* scan the PCI bus for the devices */
   result = scan_uceipci();
   /* if unsuccesfull, unregister */
   if (result != 0) {
      unregister_uceipci();
   }
   return result;
}

static int scan_uceipci() {
   uceipci_state *s;
   int i, dindx, cards_found = 0;
   int d_dindx=0;
   int ret=0;
   unsigned value=0;
   unsigned long mem_base;
   u32 hmem;
   unsigned short dev_id[MAX_BOARDS]; // supports 8 devices
   int dev_val=-1;
   
   if (uceipci_debug)
      printk(KERN_INFO "uceipci: scan_uceipci\n");

   memset(dev_id,0,sizeof(dev_id));
   
  #if defined(CONFIG_PCI) || (defined(MODULE) && !defined(NO_PCI))
   if(pcibios_present()) {
     static int pci_index = 0;
     unsigned char pci_bus, pci_device_fn;

     for(dindx=0;dindx<number_of_boards;dindx++) {
       for(pci_index=0 ;pci_index < 0xff; pci_index++) {
        #if LINUX_VERSION_CODE < 0x20402
         struct pci_dev* pdev = NULL;
        #endif
         /* probe PCI bus for our device */
         if (pcibios_find_device(PCI_VENDOR_ID_CONDOR, CONDOR_DEVICE_ID[dindx], pci_index, &pci_bus, &pci_device_fn)) 
           break;
         if(uceipci_debug)
           printk(KERN_INFO "detected device id 0x%x (bus 0x%x, slot 0x%x)\n", CONDOR_DEVICE_ID[dindx],pci_bus,PCI_SLOT(pci_device_fn));
         d_dindx = dindx;
         if(board_value[dindx]>=0) {
          #if LINUX_VERSION_CODE >= 0x20402
           pcibios_read_config_dword(pci_bus, pci_device_fn, addresses[mbase[dindx]], &hmem);
          #else
           pdev = pci_find_slot(pci_bus, pci_device_fn);
           hmem = pdev->base_address[mbase[dindx]];
          #endif
           mem_base = (unsigned long)ioremap(hmem, 2);
           value = readw(mem_base);
           if(dindx<3) { // figure which 0x1553 device this is PCI-1553 Q104-1553P or QPMC-1553
             if((value & 0xf) == 0)// this is a PCI-1553
               d_dindx = 0;
             else if ((value & 0x3e) == board_value[1]) // This is a Q104-1553P
               d_dindx = 1;
             else if ((value & 0x3e) == board_value[2]) // This is a QPMC-1553 
               d_dindx = 2;
             if(dindx != d_dindx)
               continue;
           }

           dev_val = ((pci_bus << 8) | pci_device_fn);  // create unique identifier 
           for(i=0;i<MAX_BOARDS;i++) {
             if(dev_id[i] == 0) {
               dev_id[i] = dev_val;	  
               break;
             }
             if(dev_id[i] == dev_val) {
               dev_val = -1;
               iounmap((char*)mem_base);
               continue;	  
             }	  
           }
           if(i == MAX_BOARDS) {
             printk("reached maximum board support\n");
             dindx = number_of_boards;
             iounmap((char*)mem_base);
             continue;
           }

           if(uceipci_debug) {
             printk(KERN_INFO "checking for a %s (0x%x)\n", CONDOR_DEVICE_NAME[d_dindx],CONDOR_DEVICE_ID[d_dindx]); 
             printk(KERN_INFO "host I/F = %x value = %x for index %d\n", value,(value & board_mask[d_dindx]),d_dindx);
             if((value & board_mask[d_dindx]) != board_value[d_dindx]) {
               printk(KERN_INFO "This is NOT the right board...\n");
               continue;
             }

             printk(KERN_INFO "This IS the right board...\n");
           }
         }

         /* allocate memory for device state */
         if(!(s = kmalloc(sizeof(uceipci_state), GFP_KERNEL))) {
           printk(KERN_WARNING "uceipci: out of memory\n");
           break;
         }

         /* initialize device state */
         memset(s, 0, sizeof(uceipci_state));
         s->magic = UCEIPCI_MAGIC;
         s->num_regs=0;
         s->inuse = 0;
         s->pci_device_fn = pci_device_fn;
         s->pci_bus = pci_bus;
         s->device_id = CONDOR_DEVICE_ID[d_dindx];

         for(i=0; i<6; i++) {
           u32 mem, mask;
           unsigned long flags;
           char* type;

           /* read address */
          #if LINUX_VERSION_CODE >= 0x20402
           pcibios_read_config_dword(pci_bus, pci_device_fn, addresses[i], &mem);
          #else
           pdev = pci_find_slot(pci_bus, pci_device_fn);
           mem = pdev->base_address[i];
          #endif
           if(mem==0)
             continue;
           /* determine region size */
           save_flags(flags);
           cli();
           pcibios_write_config_dword(pci_bus, pci_device_fn, addresses[i], ~0);
           pcibios_read_config_dword(pci_bus, pci_device_fn, addresses[i], &mask);
           pcibios_write_config_dword(pci_bus, pci_device_fn, addresses[i], mem);
           restore_flags(flags);
           if(uceipci_debug) 
             printk(KERN_INFO "  region %i: mask 0x%08x, now at 0x%08x\n", i, (unsigned int)mask, (unsigned int)mem);
           if(!mask) {
             if(uceipci_debug) 
               printk(KERN_INFO "  region %i: not existent\n", i);
             continue;		  
           }

           /* extract type and size */
           if(mask & PCI_BASE_ADDRESS_SPACE) {
             type = "I/O";
             mask &= PCI_BASE_ADDRESS_IO_MASK;
           } 
           else {
             type = "mem";
             mask &= PCI_BASE_ADDRESS_MEM_MASK;
             /* store region info */
             s->regions[i].membase = mem;
             s->regions[i].size = ~mask+1;
             if((s->regions[i].size > 0) && (s->regions[i].size < 0x1000))
               s->regions[i].size = 0x1000;
             if(s->regions[i].size > 0) 
               s->num_regs++;
           }

           if(uceipci_debug) 
             printk(KERN_INFO "  region %i: type %s, size 0x%x\n", i, type, ~mask+1);
         }

         /* PCI card uses region 0 or 2 so use index */
         s->card_membase = s->regions[mbase[d_dindx]].membase;
         s->size = s->regions[mbase[d_dindx]].size;
         s->minor = cards_found;

         if((value & 0xf) == 0)
           int_data[s->minor].int_board = 0;
         else
           int_data[s->minor].int_board = 1; 

         /* map PCI card memory into kernel memory */
         s->phys_membase = (unsigned long)ioremap(s->card_membase, s->size);

         if(int_support[d_dindx]) {
           struct pci_dev* pdev = pci_find_slot(pci_bus, pci_device_fn);
           if(pdev->irq == 0) {
             s->cei_irq = -1;
             if(uceipci_debug)
               printk(KERN_INFO "  failed to read IRQ\n");
           }
 	         else {
             int_data[s->minor].irq = s->cei_irq = pdev->irq;
             if(uceipci_debug)
               printk(KERN_INFO "  interrupt assigned = %d\n",s->cei_irq);
             switch(s->device_id) {
             case 0x430:
             case 0x430A:
             case 0x530:
             case 0x630:
             case 0x830:
             case 0x831:
             case 0x832:
               int_data[s->minor].notifier_mode = 1;
               init_waitqueue_head(&(int_data[s->minor].isr_wait_q));
               ret = request_irq(s->cei_irq,cei_x30_int,SA_INTERRUPT|SA_SHIRQ,CONDOR_DEVICE,(void*)s);
               break;
             default:
               ret = request_irq(s->cei_irq,cei_1553_int,SA_INTERRUPT|SA_SHIRQ,CONDOR_DEVICE,(void*)s);
             };
             if(uceipci_debug)
               printk(KERN_INFO "  interrupt request status = %d\n",ret);
             /* set up the interrupt handler to run */
             if(ret)
               printk(KERN_INFO "scan_uceipci: unable to get assigned int (IRQ: %i)\n",s->cei_irq);
             else {
               icount=0;
               int_count=0;
               int_data[s->minor].ch0_cnt=0;
               int_data[s->minor].ch1_cnt=0;
               int_data[s->minor].ch2_cnt=0;
               int_data[s->minor].ch3_cnt=0;
               // if EPMC-1553 
               if(s->device_id == 0x1003) {
                 // map BAR0 region - PLX9056
                 mem_base = (unsigned long)ioremap(s->regions[0].membase, s->regions[0].size);
                 // configure PLX9056 Local Configuration Register with default values
                 writel(0x100C767E, ((u32*)mem_base) + 27); // CNTRL register
                 writel(0x42030343, ((u32*)mem_base) + 6);  // LBRD0 register
                 iounmap((u32*)mem_base);
               }
             }
           }
         }

         /* queue it for later freeing */
         s->next = devs;
         devs = s;
         /* enable the card */
         pcibios_write_config_word(pci_bus, pci_device_fn, PCI_COMMAND, PCI_COMMAND_MEMORY);
         cards_found++;
       }
       if(uceipci_debug)
         printk(KERN_INFO "Done with %s \n",CONDOR_DEVICE_NAME[dindx]);
     }
   }
  #endif /* NO_PCI */

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: found %i device(s)\n", cards_found);

   return cards_found ? 0 : -ENODEV;
}

void cleanup_module(void) {
   /* No need to check MOD_IN_USE, as sys_delete_module() checks. */
   uceipci_state *s;

   if (uceipci_debug)
      printk(KERN_INFO "uceipci: cleanup_module\n");

   /* traverse list and free memory */
   while ((s = devs)) {
      devs = devs->next;
      if(s->phys_membase) {
        if(s->cei_irq != -1)
          free_irq(s->cei_irq ,(void *)s);//->phys_membase);
        iounmap((void*)s->phys_membase);
      }
      kfree(s);
   }

   /* unregister the char device */
   unregister_uceipci();
}

#if defined(CONFIG_X86_64) || defined(CONFIG_PPC64)
static int register_ioctl32_uceipci(void) {
  int status=0;
  status |= register_ioctl32_conversion(SET_REGION, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_REGION_MEM, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_REGION_SIZE, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_DEVICE_ID, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_IRQ, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(SET_PID, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(SET_SIGNAL, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(SET_SIGVAL, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_PCIREGION_BYTE, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_PCIREGION_WORD, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(GET_PCIREGION_DWORD, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(SET_INTERRUPT_MODE, (void*)sys_ioctl); 
  status |= register_ioctl32_conversion(SET_WAIT_QUEUE, (void*)sys_ioctl); 
  return status;
}

static int unregister_ioctl32_uceipci(void) {
  int status=0;
  status |= unregister_ioctl32_conversion(SET_REGION); 
  status |= unregister_ioctl32_conversion(GET_REGION_MEM); 
  status |= unregister_ioctl32_conversion(GET_REGION_SIZE); 
  status |= unregister_ioctl32_conversion(GET_DEVICE_ID); 
  status |= unregister_ioctl32_conversion(GET_IRQ); 
  status |= unregister_ioctl32_conversion(SET_PID); 
  status |= unregister_ioctl32_conversion(SET_SIGNAL); 
  status |= unregister_ioctl32_conversion(SET_SIGVAL); 
  status |= unregister_ioctl32_conversion(GET_PCIREGION_BYTE); 
  status |= unregister_ioctl32_conversion(GET_PCIREGION_WORD); 
  status |= unregister_ioctl32_conversion(GET_PCIREGION_DWORD); 
  status |= unregister_ioctl32_conversion(SET_INTERRUPT_MODE); 
  status |= unregister_ioctl32_conversion(SET_WAIT_QUEUE); 
  return status;
}
#endif

static void cei_1553_int(int irq, void *data, struct pt_regs *regs) {
   unsigned int reg_data=0, uca=0;
   unsigned long reg_addr=0;
   int iminor, nchan, index;
   struct siginfo sig_info;
   uceipci_state* pState = (uceipci_state*)data;
   struct task_struct *p=NULL;

   iminor = pState->minor;
   int_data[iminor].int_occurred = 0;

   if(int_data[iminor].pid != 0) { 
     read_lock(&tasklist_lock);
     p = find_task_by_pid(int_data[iminor].pid);
     read_unlock(&tasklist_lock);
     sig_info.si_signo = int_data[iminor].signal;
     sig_info.si_code = SI_QUEUE;
     sig_info.si_int = int_data[iminor].sigval;
   }

   // check the EPMC-1553 for interrupts
   if(pState->device_id == 0x1003) {
     // read the Interrupt Status Register (global register) instead of each channel register
     reg_data = readw(pState->phys_membase + 0x200012);
     for(index=0;index<8;index++) {
       // check for interrupt per channel
       if((reg_data >> index) & 0x1) {
       // clear the interrupt in the channel register
         writew(0, pState->phys_membase + (index * 0x20000) + 0x4);
         int_data[iminor].int_occurred++;
         if(p) {
           sig_info.si_int = iminor + index;  // send the devnum
           send_sig_info(int_data[iminor].signal, &sig_info, p);
	 }
       }
     }
     return;
   }

   reg_data = readw(pState->phys_membase);
   if(reg_data & 0x4000)
     uca = 2;  // UCA32
   else
     uca = int_data[iminor].int_board;
   nchan = (reg_data >> 6) & (uca?0x7:0x3);

   for(index=0;index<nchan;index++) {
     if(uca == 2) {
       /* read the global "Interrupt Status" register */
       reg_data = readl(pState->phys_membase + 0x84);
       /* check for interrupt on channel */
       if(((reg_data >> index) & 0x1) == 0)
         continue;
       writel(0x40000200, pState->phys_membase + (index * 0x4000) + 0x30058);
     }
     else {
       if(uca == 0)
         reg_addr = pState->phys_membase + (index * 0x1000000) + 0x400000;
       else
         reg_addr = pState->phys_membase + (index * 0x200000) + 0x800;
       /* read the 1553_control_register on channel */
       reg_data = readw(reg_addr);
       /*// check for interrupt */
       if((reg_data & 0x200) == 0)
         continue;
       writew(0, reg_addr + 0x16);
     }
     if(index == 0) int_data[iminor].ch0_cnt++;
     else if(index == 1) int_data[iminor].ch1_cnt++;
     else if(index == 2) int_data[iminor].ch2_cnt++;
     else if(index == 3) int_data[iminor].ch3_cnt++;
     int_data[iminor].int_occurred++;
     if(p)
       send_sig_info(int_data[iminor].signal, &sig_info, p);
   }
}

static void cei_x30_int(int irq, void* data, struct pt_regs* regs) {
   unsigned int icr_reg;
   int iminor;
   uceipci_state* pState = (uceipci_state*)data;

   for(iminor=0;iminor<MAX_BOARDS;iminor++) {
     if(int_data[iminor].irq == irq)
       break;
   }
   if(iminor == MAX_BOARDS)
     return;

   // read the ICR (32-bit)
   icr_reg = readl(pState->phys_membase);
   // check for interrupt
   if(icr_reg & 0x10) {
     // clear interrupt register
     if((icr_reg & 0xFF00) >= 0x4D00)  // FW v4.13+ CMD usage
       writel(0x40000010, pState->phys_membase);
     else
       writel(icr_reg | 0x10, pState->phys_membase);
     int_data[iminor].cur_intrpt++;
     wake_up_interruptible(&(int_data[iminor].isr_wait_q));
   }   
}

