/*============================================================================*
 * FILE:                      U C E I I S A . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 2005 - 2018 BY ABACO SYSTEMS, INC.
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
 *===========================================================================*/

/* $Revision:  1.06 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/30/2005   Initial.  support for Bustools/1553 and CEI-x20 ISA devices. bch
  03/13/2007   modified to support kernel changes in 2.6.19. bch
  06/07/2007   modified the DEV_DATA and INT_DATA structures. bch
  04/13/2011   modified file_operations for changes in kernel 2.6.36 and
                MODULE_LICENSE. bch
*/

#include <linux/version.h>
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
 #include <linux/config.h>
#endif
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#ifndef NO_HW_INTERRUPTS
 #include <asm/irq.h>
 #include <linux/interrupt.h>
 #include <linux/signal.h>
#endif

#include <linux/delay.h>	

#define CEI_ISA_MODULE "uceiisa"
#define MAX_BTA        4

#define UCEIISA_DEBUG

// ioctl commands 
#define SET_REGION      0x13c61
#define GET_REGION_MEM  0x13c62
#define GET_REGION_SIZE 0x13c63
#define GET_INTERRUPT   0x13c64
#define SET_PID         0x13c67

#define STATUS_INIT        0x1
#define STATUS_OPEN        0x2
#define STATUS_MAP         0x4
#define STATUS_INT_ENABLE  0x8

#ifndef NO_HW_INTERRUPTS
 typedef struct {
   int irq;
   int ch;
   pid_t sigpid;
   unsigned int signal;
   int sigval;
   unsigned long hwr_chan_addr[4];
 }INT_DATA;
#endif

typedef struct {
  unsigned int minor;
  unsigned long phys_membase;
  unsigned long kern_membase;
  unsigned long size;
  int status;
  int device;
 #ifndef NO_HW_INTERRUPTS
  INT_DATA int_data;
 #endif
}DEV_DATA;

static char *version="v1.10 - (11/03/2011) Brian Hill(avionics.support@abaco.com)\n";
static int uceiisa_debug=0;
static int uceiisa_major=0;
static int basemem[MAX_BTA]={-1,-1,-1,-1};
static DEV_DATA* pdev_data[MAX_BTA]={NULL,NULL,NULL,NULL};
static int pdev_indx=0;

static int irq[MAX_BTA]={-1,-1,-1,-1};
#ifndef NO_HW_INTERRUPTS
// the base Real-Time signal used in the ISR to notify the API that an interrupt occured on a specific channel
 //#define HW_INT_BASE_SIGVAL 40//SIGRTMIN
 // Q104-1553
 #define HWR_OFFSET       0x00000200
 #define HWR_CHAN_OFFSET  0x00001000
#endif

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
static long uceiisa_unlocked_ioctl(struct file* filp, unsigned int cmd, unsigned long arg);
#else
static int uceiisa_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg);
#endif
static int uceiisa_mmap(struct file* filp, struct vm_area_struct* vma);
static int uceiisa_open(struct inode* inode, struct file* filp);	
static int uceiisa_release(struct inode* inode, struct file* filp);
static ssize_t uceiisa_read(struct file* filp, char __user* usr_buf, size_t count, loff_t* ppos);
static ssize_t uceiisa_write(struct file* filp, const char* usr_buf, size_t count, loff_t* ppos);
static struct file_operations uceiisa_fops = {
  .owner   =  THIS_MODULE,
 #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
  .unlocked_ioctl = uceiisa_unlocked_ioctl,
 #else
  .ioctl   =  uceiisa_ioctl,
 #endif
  .mmap    =  uceiisa_mmap,
  .open    =  uceiisa_open,
  .read    =  uceiisa_read,
  .write   =  uceiisa_write,
  .release =  uceiisa_release,
};
#ifndef NO_HW_INTERRUPTS
 // interrupt service routine
 #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
  static irqreturn_t uceiisa_1553_isr(int irq, void *data);
 #else
  static irqreturn_t uceiisa_1553_isr(int irq, void *data, struct pt_regs *regs);
 #endif
#endif
// internal functions
static int register_uceiisa(void);
static void unregister_uceiisa(void);
static int scan_uceiisa(void);
static int get_devData(struct file* filp, DEV_DATA** pdata);
#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl(DEV_DATA* pData, int mode);
#endif
static int __init uceiisa_init_module(void);
static void __exit uceiisa_exit_module(void);
module_init(uceiisa_init_module);
module_exit(uceiisa_exit_module);


MODULE_AUTHOR("Brian Hill <avionics.support@abaco.com>");
MODULE_DESCRIPTION("Abaco Systems avionics universal Linux kernel 2.6 ISA device driver v1.10.  Do not distribute the source code.");
#if((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)) && defined(HW_INTERRUPTS_SIGNAL))
 // change find_vpid from EXPORT_SYMBOL to EXPORT_SYMBOL_GPL starting with 2.6.31
 MODULE_LICENSE("GPL");
#else
 MODULE_LICENSE("Proprietary");  // Source code supplied
#endif
#ifdef UCEIISA_DEBUG
 module_param(uceiisa_debug, int, 0644);
 MODULE_PARM_DESC(uceiisa_debug, "  Set the debugging level for the driver.");
#endif
#ifdef UCEIISA_MAJOR
// if the need for a specific major number, check the
// "/usr/src/linux/Documentation/devices.txt" for already assigned major numbers
 module_param(uceiisa_major, int, 0644); 
 MODULE_PARM_DESC(uceiisa_major, "  The major number to assign to the driver instead of the operating system.");
#endif

static int basemem_num=0;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11))
 module_param_array(basemem, int, &basemem_num, 0644); 
#else
 module_param_array(basemem, int, basemem_num, 0644); 
#endif 
MODULE_PARM_DESC(basemem, "  The memory address for the board.");

static int irq_num=0;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11))
module_param_array(irq, int, &irq_num, 0644); 
#else
module_param_array(irq, int, irq_num, 0644); 
#endif
MODULE_PARM_DESC(irq, "  The IRQ for the board.");
