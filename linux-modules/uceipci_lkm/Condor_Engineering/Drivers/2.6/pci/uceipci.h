/*============================================================================*
 * FILE:                      U C E I P C I . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 2006 - 2018 BY ABACO SYSTEMS, INC.
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

/* $Revision:  1.17 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  03/30/2005   Initial. bch
  07/15/2005   change MODULE_LICENSE to allow for GPL calls. bch
  03/27/2007   modified to support kernel changes in 2.6.19. bch
  05/30/2007   added ID_DATA to support multiple processes. modified DEV_DATA
                for waiting on hardware interrupts. bch
  01/23/2008   modified to support kernel changes in 2.6.23. bch
  02/27/2009   modified struct DEV_DATA by adding pci_bar_conf_membase and
               renaming kern_membase to pci_bar_laddr_membase. bch
  10/20/2009   modified struct DEV_DATA by adding bus_type. bch
  11/10/2010   removed __init from uceipci_probe. bch
  10/11/2011   modified DEV_DATA. bch 
  08/07/2012   modified DEV_DATA. bch 
  01/14/2014   added "csc" to DEV_DATA. bch
*/


#include <linux/version.h>
#include <linux/module.h>  // standard
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
 #include <linux/config.h>  // standard
#endif
#include <linux/init.h>    // for macros (__init, __exit, module_init, module_exit)
#include <linux/pci.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
 #include <linux/fs.h>  // removed from mm.h in kernel 2.6.23
#endif
#include <linux/device.h>
#include <asm/atomic.h>

#define DRIVER_NAME "uceipci"

static char* uceipci_version="v1.25 - (01/15/2015) Brian Hill(avionics.support@abaco.com)";
static int uceipci_debug=0;  // debug level 0 (no debug output to kernel message log)
static int uceipci_major=0;  // dynamic major number assignment (default)

MODULE_AUTHOR("Brian Hill <avionics.support@abaco.com>");
MODULE_DESCRIPTION("Abaco Systems avionics universal Linux kernel 2.6 PCI device driver v1.25.  Do not distribute the source code.");
#if((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)) && !defined(NO_SYSFS_SUPPORT)) || \
   ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)) && defined(HW_INTERRUPTS_SIGNAL))
  // change of SYSFS calls from EXPORT_SYMBOL to EXPORT_SYMBOL_GPL starting with 2.6.10
  // change find_vpid from EXPORT_SYMBOL to EXPORT_SYMBOL_GPL starting with 2.6.31
  MODULE_LICENSE("GPL");
#else
  MODULE_LICENSE("Proprietary");  // Source code supplied
#endif
module_param(uceipci_debug, int, 0644);
MODULE_PARM_DESC(uceipci_debug, "  Set the debugging level for the driver (0-3).");

#ifdef UCEIPCI_MAJOR
// if the need for a specific major number, check the "/usr/src/linux/Documentation/devices.txt" for already assigned major numbers
 module_param(uceipci_major, int, 0644); 
 MODULE_PARM_DESC(uceipci_major, "  The major number to assign to the driver instead of the operating system.");
#endif
 
static struct pci_device_id pci_id_table[] = {
  {  // Condor Engineering boards
    .vendor    = 0x13c6,
    .device    = PCI_ANY_ID,
    .subvendor = PCI_ANY_ID,
    .subdevice = PCI_ANY_ID,
  },
  {  // Acromag APC8620 carrier
    .vendor    = 0x10B5,
    .device    = 0x1024,
    .subvendor = PCI_ANY_ID,
    .subdevice = PCI_ANY_ID,
  },
  {  // SBS PCI-40A carrier
    .vendor    = 0x124B,
    .device    = 0x0040,
    .subvendor = PCI_ANY_ID,
    .subdevice = PCI_ANY_ID,
  },	  
  { }
};
MODULE_DEVICE_TABLE(pci, pci_id_table);

static int uceipci_probe(struct pci_dev* pdev, const struct pci_device_id* dev_id);
static void __exit uceipci_remove(struct pci_dev* pdev);
static struct pci_driver uceipci_driver = {
//	.owner   =  THIS_MODULE,
        .name     = DRIVER_NAME,
        .id_table = pci_id_table,
        .probe    = uceipci_probe,
        .remove   = uceipci_remove,
};
static int __init uceipci_init_module(void);
static void __exit uceipci_exit_module(void);
module_init(uceipci_init_module);
module_exit(uceipci_exit_module);


// device specific
#define MAX_DEVICES    8
#define MAX_CHANNELS   9
#define MAX_INSTANCES  20
#define STATUS_INIT          0x1
#define STATUS_OPEN          0x2
#define STATUS_MAP           0x4
#define STATUS_INT_ENABLE    0x8
#define STATUS_THREAD        0x10
#define STATUS_HWINT_SIGNAL  0x20
#define STATUS_HWINT_WQ      0x40

typedef struct{ 
  u32 id_indx;
  pid_t pid;  // process id
  pid_t tid;  // thread id
  int mapped_bar_regions;
 #ifndef NO_HW_INTERRUPTS
  #ifdef HW_INTERRUPTS_SIGNAL
  pid_t sigpid;
  int sigval;
  #endif
 #endif
  int status;
}ID_DATA;

typedef struct{
  int minor;
  unsigned long pci_bar_attr[6][3];  // PCI address, PCI address size, PCI type
  int pci_bar_regions;
  unsigned long pci_bar_conf_membase;
  unsigned long pci_bar_laddr_membase;
  int id_cnt;
  int ids_mask;
  ID_DATA ids[MAX_INSTANCES];
 #ifndef NO_HW_INTERRUPTS
  int irq;
  unsigned long hwr_chan_addr[MAX_CHANNELS];
  #ifdef HW_INTERRUPTS_SIGNAL
   int signal;
   int ids_mask_intrpt;
  #endif
  #ifdef HW_INTERRUPTS_WAITQUEUE
   int cur_intrpt[MAX_CHANNELS];
   int last_intrpt[MAX_CHANNELS];
   wait_queue_head_t isr_wait_q[MAX_CHANNELS];
  #endif
 #endif
  char boardname[50];
  int id;
  int board_type;
  int bus_type;
  int ch;
  int irig;
  int mode; 
  int uca32;
  int csc;
  int status;
}DEV_DATA;

#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl(DEV_DATA* pData, int mode);
#endif

static struct cdev ucei_cdev;
static dev_t ucei_dev_t=0;
static int cdev_exists=0; 
static struct pci_dev* ucei_pdev[MAX_DEVICES];
static int ucei_pdev_indx=0;

#ifndef NO_SYSFS_SUPPORT
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13))
 #define show_bd_info(field, format_string)                         \
  static ssize_t show_info_##field(struct device* dev, char* buf) {  \
   struct pci_dev* pdev = to_pci_dev(dev);                          \
   DEV_DATA* data = (DEV_DATA*) pci_get_drvdata(pdev);              \
   return sprintf(buf, format_string, data->field);                 \
  }
#else
 #define show_bd_info(field, format_string)                         \
  static ssize_t show_info_##field(struct device* dev, struct device_attribute* attr, char* buf) {  \
   struct pci_dev* pdev = to_pci_dev(dev);                          \
   DEV_DATA* data = (DEV_DATA*) pci_get_drvdata(pdev);              \
   return sprintf(buf, format_string, data->field);                 \
  }
#endif

 #define bd_info_attr(field, format_string)                     \
  show_bd_info(field, format_string)                             \
  static DEVICE_ATTR(field, S_IRUGO, show_info_##field, NULL);

 bd_info_attr(minor, "%d\n")
 bd_info_attr(status, "%d\n")
 bd_info_attr(boardname, "%s\n")
 bd_info_attr(board_type, "%d\n")
 bd_info_attr(id, "0x%X\n")
#endif
