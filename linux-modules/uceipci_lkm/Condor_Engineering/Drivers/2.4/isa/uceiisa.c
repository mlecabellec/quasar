/*===========================================================================*
 * FILE:                        UCEIISA . C
 *===========================================================================*
 *
 *      COPYRIGHT (C) 1999 - 2018 BY ABACO SYSTEMS, INC.
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
 * FUNCTION: Linux device driver for ISA boards
 *
 * DESCRIPTION: This file contains the source code for the Linux Q104-1553 ISA 
 * device driver.  this driver can be installed as a module or linked with the
 * kernel depending on how it is compiled.  The MODULE macro must be defined
 * during compilation for to get a Linux Module.
 *
 *  The uceiisa driver supports the following functions:
 *   lseek
 *   read
 *   write
 *   ioctl
 *   mmap
 *   open
 *   release
 *
 *===========================================================================*/

/* $Revision:  2.03 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
               Initial
  05/25/2006   added SET_SIGNAL and SET_SIGVAL to ioctls, replaced kill_proc
                with send_sig_info in cei_isr. bch
  04/10/2009   modified uceiisa_ioctl to set default values for the device's 
                pid and sigval. bch	
  12/06/2011   added SET_INTERRUPT_MODE to uceiisa_ioctl. bch		
*/


#include <linux/config.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/signal.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE < 0x20155
#include <linux/bios32.h>
#endif
#ifdef MODULE
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#else
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif

#if (LINUX_VERSION_CODE <= 0x20100)
#define ioremap(a,b) \
    (((a)<0x100000) ? (void *)((u_long)(a)) : vremap(a,b))
#define iounmap(v) \
    do { if ((u_long)(v) > 0x100000) vfree(v); } while (0)
#endif

#define MAX_BOARDS 4
#define uceiisa_MAGIC (0xBEEF)
// ioctls
#define SET_REGION           0x13c61
#define GET_REGION_MEM       0x13c62
#define GET_REGION_SIZE      0x13c63
#define GET_DEVICE_ID        0x13c64
#define GET_IRQ              0x13c65
#define SET_PID              0x13c66
#define SET_SIGNAL           0x13c67
#define SET_SIGVAL           0x13c68
#define SET_INTERRUPT_MODE   0x13c6c

static char *version = "uceiisa.c:v2.03 - (12/06/2011) Brian Hill(avionics.support@abaco.com)\n";
static const char invalid_magic[] = KERN_CRIT "uceiisa: invalid magic value\n";

#define VALIDATE_STATE(s)                     \
({                                            \
   if (!(s) || (s)->magic != uceiisa_MAGIC) { \
      printk(invalid_magic);                  \
      return -ENXIO;                          \
   }                                          \
})

/*
#define uceiisa_debug debug
#ifdef CEI_ISA_DEBUG
 static int uceiisa_debug=1;
#else
 static int uceiisa_debug=0;
#endif
*/
static int uceiisa_debug=0;

#define uceiisa_major major
#ifndef uceiisa_MAJOR
#define uceiisa_MAJOR 0 /* dynamic major by default */
#endif
int uceiisa_major=uceiisa_MAJOR;

#if defined(MODULE) && LINUX_VERSION_CODE > 0x20115
int IRQ[]={-1,-1,-1,-1};
int basemem[]={ -1, -1, -1, -1 };
MODULE_AUTHOR("Brian Hill <avionics.support@abaco.com>");
MODULE_DESCRIPTION("Abaco Systems Universal Linux kernel 2.4 ISA device driver.  Do not distribute the source code.");
MODULE_PARM(uceiisa_debug, "i");
MODULE_PARM(major, "i");
MODULE_PARM(basemem, "1-4i");
MODULE_PARM(IRQ, "1-4i");
#endif

typedef struct uceiisa_state {
   /* magic */
   unsigned int magic;

   /* we keep CEI-ISA cards in a linked list */
   struct uceiisa_state *next;

   int inuse;
   unsigned long card_membase;
   unsigned long phys_membase;
   unsigned long size;
   unsigned int minor;
   unsigned int irq;
} uceiisa_state;

typedef struct CEI_INT_DATA
{
    int int_board;
    int ch0;
    int ch1;
    int ch2;
    int ch3;
    int irq;
    int nchan;
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
}cei_int_data;

static cei_int_data int_data[MAX_BOARDS];
static uceiisa_state* devs = NULL;

static int scan_uceiisa(void);
static void cei_isr(int, void *, struct pt_regs *);

static loff_t uceiisa_lseek(struct file * file, loff_t offset, int orig)
{
   uceiisa_state *s = (uceiisa_state *)file->private_data;
   long newpos = 0;

   /* validate state */
   VALIDATE_STATE(s);

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_lseek\n");

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
   if (newpos < 0)
      return -EINVAL;

   /* update file pointer */
   file->f_pos = newpos;

   return newpos;
}

static ssize_t
uceiisa_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
   uceiisa_state *s = (uceiisa_state *)file->private_data;
   unsigned long p = *ppos;
   unsigned long end_mem;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_read\n");

   /* validate state */
   VALIDATE_STATE(s);

   /* get highest physical address */
   end_mem = __pa(high_memory);
   
   /* sanity check */
   if (p >= end_mem)
      return 0;
   if (count > end_mem - p)
      count = end_mem - p;
   
   /* copy from kernel to user space */
   if (copy_to_user(buf, (void*)(s->phys_membase + p), count))
      return -EFAULT;
   
   /* update counter and file pointer */
   *ppos += count;

   /* return number of bytes read */
   return count;
}

static ssize_t
uceiisa_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
   uceiisa_state *s = (uceiisa_state *)file->private_data;
   unsigned long p = *ppos;
   unsigned long end_mem;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_write\n");

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

static int
uceiisa_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
//   unsigned int junk;	

   uceiisa_state *s = (uceiisa_state *)file->private_data;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_ioctl\n");

   VALIDATE_STATE(s);

   switch(cmd)
   {
      case GET_REGION_SIZE:
         put_user(s->size,(unsigned long*)arg);
	 if(uceiisa_debug)
           printk(KERN_INFO "uceiisa: uceiisa_ioctl, GET_REGION_SIZE (0x%lX)\n", s->size);
         break;
      case GET_REGION_MEM:
         put_user(s->card_membase,(unsigned long*)arg);
	 if(uceiisa_debug)
           printk(KERN_INFO "uceiisa: uceiisa_ioctl, GET_REGION_MEM (0x%lX)\n", s->card_membase);
         break;
      case SET_REGION:
//	 get_user(junk,(unsigned int*)arg);
//	 if(uceiisa_debug)
//           printk(KERN_INFO "uceiisa: uceiisa_ioctl, SET_REGION (%d)\n",(unsigned)junk);
         break;
      case GET_IRQ:
	 put_user(IRQ[s->minor],(unsigned int*)arg);
	 if(uceiisa_debug)
            printk(KERN_INFO "uceiisa: uceiisa_ioctl, GET_IRQ (%d)\n", IRQ[s->minor]);
	 break;
      case SET_PID:
	 get_user(int_data[s->minor].pid,(unsigned int*)arg);
	 if(int_data[s->minor].pid <= 0)
           int_data[s->minor].pid = ((struct task_struct*)current)->pid;
	 if(uceiisa_debug)
            printk(KERN_INFO "uceiisa: uceiisa_ioctl, PCI_SET_PID (%d)\n", int_data[s->minor].pid);
	 break;
      case SET_SIGNAL:
	 get_user(int_data[s->minor].signal,(unsigned int*)arg);
	 if(uceiisa_debug)
            printk(KERN_INFO "uceiisa: uceiisa_ioctl, SET_SIGNAL (%d)\n",int_data[s->minor].signal);
	 break;
      case SET_SIGVAL:
         get_user(int_data[s->minor].sigval,(int*)arg);
         if(int_data[s->minor].sigval <= 0)
           int_data[s->minor].sigval = s->minor;
         if(uceiisa_debug)
           printk(KERN_INFO "uceiisa: uceiisa_ioctl, SET_SIGVAL (%d)\n",int_data[s->minor].sigval);
         break;
      case SET_INTERRUPT_MODE:
	 break;
      default:
         return -EINVAL;
   }
   return 0;
}

static void
uceiisa_vma_open(struct vm_area_struct * vma)
{
   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_vma_open\n");
   MOD_INC_USE_COUNT;
}

static void
uceiisa_vma_close(struct vm_area_struct * vma)
{
   if(uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_vma_close\n");
   MOD_DEC_USE_COUNT;
}

static struct vm_operations_struct uceiisa_vm_ops = {
   uceiisa_vma_open,
   uceiisa_vma_close,
};

static int 
uceiisa_mmap(struct file *file, struct vm_area_struct * vma)
{
   uceiisa_state *s = (uceiisa_state *)file->private_data;
   unsigned long size, offset;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_mmap\n");

   VALIDATE_STATE(s);

#if LINUX_VERSION_CODE >= 0x20324
   offset = s->card_membase + vma->vm_pgoff;
   size = vma->vm_end - vma->vm_start;
   if (vma->vm_pgoff + size > s->size)
#else
   offset = s->card_membase + vma->vm_offset;
   size = vma->vm_end - vma->vm_start;
   if (vma->vm_offset + size > s->size)
#endif
   {
     printk(KERN_INFO "uceiisa: offset error\n");
     return -EINVAL;
   }
   pgprot_val(vma->vm_page_prot) = pgprot_val(vma->vm_page_prot) | _PAGE_PCD;

   vma->vm_flags |= VM_IO;

   // if Red Hat Linux 9 or Red Hat Workstation 3.0  
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,4,20) || LINUX_VERSION_CODE == KERNEL_VERSION(2,4,21)   
   if (remap_page_range(vma, vma->vm_start, offset, size, vma->vm_page_prot))
#else
   if (remap_page_range(vma->vm_start, offset, size, vma->vm_page_prot))
#endif
   {
      printk(KERN_INFO "uceiisa: remap error\n");
      return -EAGAIN;
   }
   vma->vm_file = file;

   if (vma->vm_ops)
      return -EINVAL;

   vma->vm_ops = &uceiisa_vm_ops;
   MOD_INC_USE_COUNT;

   return 0;
}

static int
uceiisa_open(struct inode *inode, struct file *file)
{
   int minor = MINOR(inode->i_rdev);
   uceiisa_state *s = devs;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_open\n");

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
      printk(KERN_INFO "openning device in-use\n");
      /* return -EBUSY; */
   else
      s->inuse++;

   /* use file->private_data to point to the device data */
   file->private_data = s;

   /* increase reference count */
   MOD_INC_USE_COUNT;

   return 0;
}

static int
uceiisa_release(struct inode *inode, struct file *file)
{
   uceiisa_state *s = (uceiisa_state *)file->private_data;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: uceiisa_release\n");

   /* validate state */
   VALIDATE_STATE(s);

   /* no longer in use */
   s->inuse = 0;

   /* decrease reference count */
   MOD_DEC_USE_COUNT;

   return 0;
}

static struct file_operations uceiisa_fops = {
#if LINUX_VERSION_CODE >= 0x20400
   THIS_MODULE,
#endif
   uceiisa_lseek,     /* lseek   */
   uceiisa_read,      /* read    */
   uceiisa_write,     /* write   */
   NULL,              /* readdir */
   NULL,              /* poll    */
   uceiisa_ioctl,     /* ioctl   */
   uceiisa_mmap,      /* mmap    */
   uceiisa_open,      /* open    */
   NULL,              /* flush   */
   uceiisa_release,   /* release */
   NULL               /* fsync   */
};

static int register_uceiisa(void)
{
   int result;
   if (uceiisa_debug)
   {
      printk(KERN_INFO "uceiisa: register_uceiisa\n");
      printk(KERN_INFO "uceiisa: KERNEL VERSION %x\n",LINUX_VERSION_CODE);
   }

   /* register major and accept a dynamic number */
   result = register_chrdev(uceiisa_major, "uceiisa", &uceiisa_fops);
   if (result < 0) {
      printk(KERN_WARNING "uceiisa: can't get major %d\n", uceiisa_major);
      return result;
   }

   /* remember the major number */
   if (uceiisa_major == 0) uceiisa_major = result; /* dynamic */

   return result;
}

static void unregister_uceiisa(void)
{
   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: unregister_uceiisa\n");
   /* unregister the device */
   unregister_chrdev(uceiisa_major, "uceiisa");
}

int init_module(void)
{
   int result;

   printk(KERN_INFO "%s", version);
  
   if (uceiisa_debug) {
      printk(KERN_INFO "uceiisa: init_module\n");
   }

   if (basemem[0] == -1) {
      printk(KERN_ERR "uceiisa: basemem [1-4] are mandatory\n");
      return -EINVAL;
   }

   
   /* register the char device */
   result = register_uceiisa();
   if (result < 0) {
      return 0;
   }

   /* scan the ISA bus for the devices */
   result = scan_uceiisa();

   /* if unsuccesfull, unregister */
   if (result != 0) {
      unregister_uceiisa();
   }

   return result;
}

static int scan_uceiisa()
{
   uceiisa_state *s;
   int cards_found = 0;
   unsigned int value, tvalue, indx, i=0;
   int ret;
   int found = 0;

   unsigned int boardID[] = {0x0048,      // sf 1ch
                         0x0088,      // sf 2ch
                         0x0108,      // sf 4ch
                         0x0848,      // mf 1ch
                         0x0888,      // mf 2ch
                         0x0908};     // mf 4ch

   char *boardDesc[] = {"1-Channel Single Function",
                        "2-Channel Single Function",
                        "4-Channel Single Function",
                        "1-Channel Multi Function",
                        "2-Channel Multi Function",
                        "4-Channel Multi Function"};

   unsigned int iboardID[] = {0x5c00,      // sf 1ch
                          0x7c00,      // sf 2ch
                          0x5f00,      // mf 1ch
                          0x7f00};     // mf 2ch

   char *iboardDesc[] = {"1-Channel Single Function",
                         "2-Channel Single Function",
                         "1-Channel Multi Function",
                         "2-Channel Multi Function"};

   unsigned int boardID_x20[] = {0x00,       // CEI-220
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

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: scan_uceiisa\n");

   while (basemem[i] != -1) 
   {
      if(uceiisa_debug) {
        printk(KERN_INFO "uceiisa scan: addr = 0x%X\n",basemem[i]);
        printk(KERN_INFO "uceiisa scan: IRQ  = %d\n",IRQ[i]);
      }
      /* Check for one of the Board IDs above   */
      value = isa_readw(basemem[i]);
      if(uceiisa_debug)
        printk(KERN_INFO "ISA Host Interface = 0x%X\n",value);
      tvalue = value & 0x0fff; /*clear the IRIG bits*/
      for(indx=0;indx<6;indx++)
      {
         if(tvalue == boardID[indx])
	 {
            if(uceiisa_debug) {
              printk(KERN_INFO "index = %d\n",indx);
              /* allocate memory for device state */
              printk(KERN_INFO "Q104-1553 found. Board = %s\n",boardDesc[indx]);
            }
            int_data[cards_found].int_board=0x104;
            int_data[cards_found].nchan=tvalue&0x7c0>>6;
            found = 1;
         }
      }
      if(!found) /* if not found check for a ISA-1553*/
      {
         for(indx=0;indx<4;indx++)
         {
            if(value == iboardID[indx]) /* This is an ISA-1553 */
            {
                if(uceiisa_debug)
                  printk(KERN_INFO "index = %d\n",indx);
                /* allocate memory for device state */
                printk(KERN_INFO "ISA-1553 found. Board = %s\n",iboardDesc[indx]);
                int_data[cards_found].int_board=0x1;
                int_data[cards_found].nchan=tvalue&0x3000>>12;
                if(int_data[cards_found].nchan==3)
                  int_data[cards_found].nchan=2;
                found = 1;
             }
         }           
      }
      if(!found)
      {
         value = isa_readb(basemem[i] + 0x80A);
         for(indx=0;indx<8;indx++) 
         {
            if(value == boardID_x20[indx]) 
            {
               if(isa_readb(basemem[i] + 0x80C) == 0) 
               {
	          if(((isa_readb(basemem[i] + 0x80E)) & 0xF0) == 0) 
                  {
                     if(uceiisa_debug)
                        printk(KERN_INFO "CEI-x20 found.  Board = %s (0x%x)\n", boardDesc_x20[indx], value);
                     int_data[cards_found].int_board=0x0;
                     found=1;
                     break;
                  }
	       }
	    }
         }
      }
      if(found)
      {  
         if (!(s = kmalloc(sizeof(uceiisa_state), GFP_KERNEL))) {
             printk(KERN_WARNING "uceiisa: out of memory\n");
             return cards_found ? 0 : -ENODEV;
         }

         /* initialize device state */
         memset(s, 0, sizeof(uceiisa_state));
         s->magic = uceiisa_MAGIC;
         s->inuse = 0;

         /* The size of the card is not really PAGE_SIZE but smaller,
         * causing mmap() to fail. PAGE_SIZE is the minimum needed to
         * successfully execute mmap(). I assume CEI API is going
         * to wrap the system calls so it shouldn't be a problem since
         * they know about this limit.
         */
         s->size =0x4000;
         if (s->size < PAGE_SIZE) s->size = PAGE_SIZE;
         s->card_membase = basemem[i];
         s->minor = cards_found;
         s->irq = IRQ[i];

         /* map ISA card memory into kernel memory */
         s->phys_membase = (unsigned long)ioremap(s->card_membase, s->size);

         /* queue it for later freeing */
         s->next = devs;
         devs = s;
         /* add syslog entry with card info */
//         if (uceiisa_debug)
//            printk(KERN_INFO "uceiisa: found CEI-ISA at 0x%lx\n", s->card_membase);
            
         if(IRQ[i] != -1)
         {
            int_data[s->minor].irq = IRQ[i];
            ret = request_irq(IRQ[i],cei_isr,SA_INTERRUPT | SA_SHIRQ,"uceiisa",(void *)s->phys_membase);
            if (uceiisa_debug)
               printk(KERN_INFO "uceiisa:  interrupt request status = %d - IRQ = %d\n",ret,IRQ[i]);
         }
         cards_found++;
      }
      i++;
   }

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: found %i device(s)\n", cards_found);

   return cards_found ? 0 : -ENODEV;
}

#ifdef MODULE

void cleanup_module(void)
{
   /* No need to check MOD_IN_USE, as sys_delete_module() checks. */
   uceiisa_state *s;

   if (uceiisa_debug)
      printk(KERN_INFO "uceiisa: cleanup_module\n");

   /* traverse list and free memory */
   while ((s = devs)) {
      devs = devs->next;
      if (s->phys_membase) 
      {
         iounmap((void*)s->phys_membase);
	 if(s->irq != -1)
	 {
	    if (uceiisa_debug)
               printk(KERN_INFO "uceiisa: freeing IRQ %d\n",s->irq);
	    free_irq(s->irq,(void *)s->phys_membase);
	 }
      }

#if LINUX_VERSION_CODE >= 0x20400
      kfree(s);
#else
      kfree_s(s, sizeof(uceiisa_state));
#endif
   }

   /* unregister the char device */
   unregister_uceiisa();
}

#endif


static void cei_isr(int irq, void *data, struct pt_regs *regs)
{
   unsigned short control_data;
   unsigned long lpbase,HIR_Addr, HWR1_Addr, HWR2_Addr, HWR3_Addr, HWR4_Addr;
   unsigned int CHAN1_ISA;        /* Offset to channel 1 specific fncts  */
   unsigned int CHAN2_ISA;        /* Offset to channel 2 specific fncts  */
   unsigned int CHAN3_ISA;        /* Offset to channel 2 specific fncts  */
   unsigned int CHAN4_ISA;        /* Offset to channel 2 specific fncts  */
   unsigned int HW_REG_ISA;      
   int iminor, nchan;
   struct siginfo sig_info;

   for(iminor=0;iminor<2;iminor++)
   {
      if (int_data[iminor].irq == irq)
         break;
   }

   if( int_data[iminor].int_board == 0x104 )
   {
      CHAN1_ISA     =  0x00000000;   /* Offset to channel 1 specific fncts  */
      CHAN2_ISA     =  0x00001000;   /* Offset to channel 2 specific fncts  */
      CHAN3_ISA     =  0x00002000;   /* Offset to channel 2 specific fncts  */
      CHAN4_ISA     =  0x00003000;   /* Offset to channel 2 specific fncts  */
      HW_REG_ISA    =  0x00000200;   /* Offset to 1553 Control Register     */
   }
   else
   {
      CHAN1_ISA     =  0x00000000;   /* Offset to channel 1 specific fncts  */
      CHAN2_ISA     =  0x00002000;   /* Offset to channel 2 specific fncts  */
      CHAN3_ISA     =  0x00000000;   /* Offset to channel 2 specific fncts  */
      CHAN4_ISA     =  0x00000000;   /* Offset to channel 2 specific fncts  */
      HW_REG_ISA    =  0x00001000;   /* Offset to 1553 Control Register     */
   }

   /* check the device for interrupt */
   lpbase = (unsigned long)data;
   HIR_Addr  = lpbase + CHAN1_ISA;                // Host Interface Registers
   HWR1_Addr = lpbase + CHAN1_ISA + HW_REG_ISA;   // HW Registers
   HWR2_Addr = lpbase + CHAN2_ISA + HW_REG_ISA;   // HW Registers
   HWR3_Addr = lpbase + CHAN3_ISA + HW_REG_ISA;   // HW Registers
   HWR4_Addr = lpbase + CHAN4_ISA + HW_REG_ISA;   // HW Registers

   nchan = int_data[iminor].nchan; 
   /* Clear the interrupt */
   /* read the CSC control reg */
 
   if(nchan >= 1)
   {
      /* read the 1553_control_register on channel 1 */
      control_data = readw(HWR1_Addr);
      if(control_data & 0x200)
      {
         /* write a 0 to the write_interrupt_bit */
         writew(0x0000, HWR1_Addr+0x16);
         int_data[iminor].ch0 = 1;
         int_data[iminor].ch0_cnt++;
         if (uceiisa_debug)
            printk(KERN_INFO "uceiisa: interrupt channel 1 %d  %d\n", int_data[iminor].signal,int_data[iminor].pid);
        // kill_proc(int_data[iminor].pid,int_data[iminor].sigval,0);
         sig_info.si_signo = int_data[iminor].signal;
         sig_info.si_int = int_data[iminor].sigval;
         sig_info.si_code = SI_QUEUE;
         send_sig_info(int_data[iminor].signal, &sig_info, find_task_by_pid(int_data[iminor].pid));
      }
   }
   if(nchan >= 2)
   {
      /* read the 1553_control_register on channel 2 */
      control_data = readw(HWR2_Addr);
      if(control_data & 0x200)
      {
         /* printk(KERN_INFO "ceipci: cei_1553_int channel 1 interrupt found\n"); */
         /* write 0 to the write_interrupt_bit */
         writew(0x0000, HWR2_Addr+0x16);
         int_data[iminor].ch1=1;
         int_data[iminor].ch1_cnt++;
         if (uceiisa_debug)
           printk(KERN_INFO "uceiisa: interrupt channel 2 %d  %d\n", int_data[iminor].signal,int_data[iminor].pid);
	//  kill_proc(int_data[iminor].pid,signal_value[iminor][1],0);
         sig_info.si_signo = int_data[iminor].signal;
         sig_info.si_int = int_data[iminor].sigval;
         sig_info.si_code = SI_QUEUE;
         send_sig_info(int_data[iminor].signal, &sig_info, find_task_by_pid(int_data[iminor].pid));
      }
   }
   if(nchan >= 3)
   {
      /* read the 1553_control_register on channel 2 */
      control_data = readw(HWR3_Addr);
      if(control_data & 0x200)
      {
         /* printk(KERN_INFO "ceipci: cei_1553_int channel 1 interrupt found\n"); */
         /* write 0 to the write_interrupt_bit */
         writew(0x0000, HWR3_Addr+0x16);
         int_data[iminor].ch2=1;
         int_data[iminor].ch2_cnt++;
         if (uceiisa_debug)
           printk(KERN_INFO "uceiisa: interrupt channel 3 %d  %d\n", int_data[iminor].signal,int_data[iminor].pid);
        //  kill_proc(int_data[iminor].pid,signal_value[iminor][2],0);
         sig_info.si_signo = int_data[iminor].signal;
         sig_info.si_int = int_data[iminor].sigval;
         sig_info.si_code = SI_QUEUE;
         send_sig_info(int_data[iminor].signal, &sig_info, find_task_by_pid(int_data[iminor].pid));
      }
   }
   if(nchan >= 4)
   {
      /* read the 1553_control_register on channel 2 */
      control_data = readw(HWR4_Addr);
      if(control_data & 0x200)
      {
         /* printk(KERN_INFO "ceipci: cei_1553_int channel 1 interrupt found\n"); */
         /* write 0 to the write_interrupt_bit */
         writew(0x0000, HWR4_Addr+0x16);
         int_data[iminor].ch3=1;
         int_data[iminor].ch3_cnt++;
         if (uceiisa_debug)
           printk(KERN_INFO "uceiisa: interrupt channel 4 %d  %d\n", int_data[iminor].signal,int_data[iminor].pid);
        //  kill_proc(int_data[iminor].pid,signal_value[iminor][3],0);
         sig_info.si_signo = int_data[iminor].signal;
         sig_info.si_int = int_data[iminor].sigval;
         sig_info.si_code = SI_QUEUE;
         send_sig_info(int_data[iminor].signal, &sig_info, find_task_by_pid(int_data[iminor].pid));
      }
   }
   if((int_data[iminor].ch0==1) || (int_data[iminor].ch1==1) || (int_data[iminor].ch2==1) || (int_data[iminor].ch3==1))
   {
      int_data[iminor].int_occurred = 1;
   }
}
