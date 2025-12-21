/*============================================================================*
 * FILE:                  C E I _ P C C 1 5 5 3 _ C S . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 2004 - 2018 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:   BusTools API Linux Kernel Card Services Client Driver for
 *             Linux kernel 2.4.x.
 *
 *             This module performs the Kernel Card Services functions
 *             needed to support operation in a Linux environment.
 *
 *
 * EXTERNAL ENTRY POINTS:
 *    cei_open                opens a PCCARD-(D)1553
 *
 *    cei_close               closes a PCCARD-(D)1553
 *
 *    cei_ioctl               performs a variety of utility functions:
 *
 *    cei_mmap                map board physical memory to user space
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *
 * INTERNAL ROUTINES:
 *
 *===========================================================================*/

/* $Revision:  1.02 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  04/16/2004   initial release for PCCARD-(D)1553 - bch
  08/10/2005   added cei_mmap to map board - bch
  11/18/2008   modified cei_config - bch
*/


#include <linux/module.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>
#include <asm/uaccess.h>
//#include <pcmcia/mem_op.h>

/*====================================================================*/
//  module parameters
MODULE_AUTHOR("Brian Hill (avionics.support@abaco.com)");
MODULE_DESCRIPTION("Abaco Systems avionics Linux kernel 2.4 PCMCIA device driver.  Do not distribute the source code.");
MODULE_LICENSE("Prop");
static int cs_debug=0;  // sets the debug level 0-2
MODULE_PARM(cs_debug, "i");

// handles Card Services errors
static void cs_error(client_handle_t client_handle, int sub_func, int cs_result) {
    error_info_t error_info = {sub_func, cs_result};
    CardServices(ReportError, client_handle, &error_info);
}

/*====================================================================*/
// prototypes
static dev_link_t* cei_attach(void);
static int cei_config(dev_link_t* dev_link);
static void cei_release(u_long arg);
static int cei_event(event_t event, int priority, event_callback_args_t* args);
static void cei_detach(dev_link_t* dev_link);

static int cei_open(struct inode* inode, struct file* filp);
static int cei_close(struct inode* inode, struct file* filp);
static int cei_ioctl(struct inode* inode, struct file* filp, u_int cmd, u_long arg);
static int cei_mmap(struct file* filp, struct vm_area_struct* vma);
static struct file_operations cei_pcc1553_cs_fops = {
    open:    cei_open,
    release: cei_close,
    ioctl:   cei_ioctl,
    mmap:    cei_mmap,
};
// ioctls
#define SET_REGION           0x13c61
#define GET_REGION_MEM       0x13c62
#define GET_REGION_SIZE      0x13c63
#define SET_PID              0x13c66


/*====================================================================*/
// board specfic information
typedef struct cei_dev_t {
    unsigned long membase;
    unsigned long phybase; 
    unsigned long size;
    unsigned long pid;
    int minor;    
    int board_type;
} CEI_DEV;

// device specific information
typedef struct local_info_t {
    dev_link_t dev_link;  
    dev_node_t dev_node;
    CEI_DEV cei_dev;
} LOCAL_INFO;

#define MAX_BOARDS  2  // only supports two PCCARD-(D)1553 boards
static char *version="cei_pcc1553_cs:v1.02 - (11/18/2008) Brian Hill(avionics.support@abaco.com)";
static dev_info_t dev_info="cei_pcc1553_cs";
static LOCAL_INFO* local_info_table[MAX_BOARDS]={NULL,NULL};
static int major_dev=-1;

/*======================================================================
======================================================================*/
static dev_link_t* cei_attach(void) {
    int i, status=0;
    LOCAL_INFO* local_info=NULL;
    dev_link_t* dev_link=NULL;
    client_reg_t client_reg;
    
    if(cs_debug >= 2) 
      printk(KERN_INFO "cei_pcc1553_cs (cei_attach).\n");

    for(i = 0; i < MAX_BOARDS; i++) {
      if(local_info_table[i] == NULL)
        break;
    }      
    if(i == MAX_BOARDS) {
      printk(KERN_WARNING "cei_pcc1553_cs:  maximum devices (%d) reached for client driver.\n", MAX_BOARDS);
      return NULL;
    }
    
    // alloc and configure device struct
    if((local_info = kmalloc(sizeof(LOCAL_INFO), GFP_KERNEL)) == NULL) {
      printk(KERN_WARNING "cei_pcc1553_cs:  cannot alloc memory for local_info struct.\n");
      return NULL;
    }
    memset(local_info, 0, sizeof(LOCAL_INFO));    
    dev_link = &local_info->dev_link;
    dev_link->priv = local_info;
    local_info->dev_node.major = major_dev;
    local_info->dev_node.minor = i;
    local_info_table[i] = local_info;

    dev_link->release.function = &cei_release;
    dev_link->release.data = (u_long)dev_link;
    init_timer(&dev_link->release);
    
    // configure client
    memset(&client_reg, 0, sizeof(client_reg_t));
    client_reg.dev_info   = &dev_info;
    client_reg.Attributes = INFO_MEM_CLIENT; 
    client_reg.EventMask  = CS_EVENT_CARD_INSERTION | CS_EVENT_CARD_REMOVAL |
                            CS_EVENT_RESET_PHYSICAL | CS_EVENT_CARD_RESET   |
                            CS_EVENT_PM_SUSPEND     | CS_EVENT_PM_RESUME;
    client_reg.event_handler = &cei_event;
    client_reg.event_callback_args.client_data = dev_link;
    
    // register with Card Services
    if((status = CardServices(RegisterClient, &dev_link->handle, &client_reg)) != CS_SUCCESS) {
      cs_error(dev_link->handle, RegisterClient, status);
      printk(KERN_ERR "cei_pcc1553_cs:  failed to register client (0x%x).\n", status);
      return NULL;
    }

    if(cs_debug)
      printk(KERN_INFO "cei_pcc1553_cs:  successfully attached %s.\n", local_info->dev_node.dev_name);

    return dev_link;
}


/*======================================================================
======================================================================*/
static void cei_detach(dev_link_t* dev_link) {
    int status=0;
    int minor=-1;
    
    if(cs_debug >= 2)
      printk(KERN_INFO "cei_pcc1553_cs (cei_detach).\n");

    if(dev_link->priv == NULL)
      return;
       
    minor=dev_link->dev->minor;	
    del_timer(&dev_link->release);
      
    // check if device is configured, if so than need to "release" before "detach"
    if(dev_link->state & DEV_CONFIG) {
      printk(KERN_WARNING "cei_pcc1553_cs (cei_detach):  '%s' not released, releasing device...\n", dev_link->dev->dev_name);
      cei_release((unsigned long) dev_link);
    }

    if(dev_link->handle) {
      if((status = CardServices(DeregisterClient, dev_link->handle)) != CS_SUCCESS) {
        cs_error(dev_link->handle, DeregisterClient, status);
        dev_link->state |= DEV_STALE_LINK;
        return;
      }
    }
    dev_link->handle = 0;

    local_info_table[minor] = NULL;
    kfree(dev_link->priv);  // release device struct (local_info)
    dev_link->priv = NULL;
    dev_link->state &= ~DEV_PRESENT;

    if(cs_debug)
      printk(KERN_INFO "cei_pcc1553_cs (%d):  successfully detached %s.\n", minor, dev_link->dev->dev_name);
}


/*======================================================================
======================================================================*/
static int cei_config(dev_link_t* dev_link) {
    int status=0;
    int minor=-1;
    unsigned long valD=0;
    unsigned short hi_reg=0;
    win_req_t win_req; 
    LOCAL_INFO* local_info=NULL;

    if(cs_debug >= 2)
      printk(KERN_INFO "cei_pcc1553_cs (cei_config).\n");
   
    if((local_info = (LOCAL_INFO*)dev_link->priv) == NULL)
      return -1; 
   
    minor = local_info->dev_node.minor; 
    
    if(dev_link->state & DEV_CONFIG) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s already configured.\n", minor, local_info->dev_node.dev_name);
      return -1;  
    }
    
    // allocate region for device
    memset(&win_req, 0, sizeof(win_req));
    win_req.Attributes = WIN_DATA_WIDTH_16 | WIN_MEMORY_TYPE_CM | WIN_ENABLE | WIN_STRICT_ALIGN; 
    win_req.Size = 0x2000;
    dev_link->win = (window_handle_t) dev_link->handle;
    if((status = CardServices(RequestWindow, &dev_link->win, &win_req)) != CS_SUCCESS) {
      cs_error(dev_link->handle, RequestWindow, status);    
      return -1;
    }
    
    // map board memory to kernel space
    if((valD = (unsigned long) ioremap(win_req.Base, win_req.Size)) == 0) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  cannot map board memory.\n", minor);
      if((status = CardServices(ReleaseWindow, dev_link->win)) != CS_SUCCESS)  
        cs_error(dev_link->handle, ReleaseWindow, status);
      else
        dev_link->win = 0; 
      return -1;
    }
    
    local_info->cei_dev.membase = valD;
    local_info->cei_dev.phybase = (unsigned long) win_req.Base;
    local_info->cei_dev.size    = win_req.Size;
    // device is now configured
    dev_link->state &= ~DEV_CONFIG_PENDING;
    dev_link->state |= DEV_CONFIG;  
    
    hi_reg = readw((unsigned short*)(local_info->cei_dev.membase));
    if(cs_debug >= 2)
      printk(KERN_INFO "cei_pcc1553_cs (%d):  CSC register = 0x%x\n", minor, hi_reg);
    valD = ((hi_reg & 0x3E) >> 1);   // check "board_type" register for PCCARD-D1553
    if(valD != 6) 
      valD = ((hi_reg & 0xC000) >> 14);  // check "host bus type ID" ("bt" register) for PCCARD-1553
    local_info->cei_dev.board_type = valD;
    if((valD != 2) && (valD != 6)) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  invalid PCCARD-1553 board detected (%d).\n", minor, valD);
      sprintf(local_info->dev_node.dev_name, "<INVALID BOARD>");
      return -1;
    }
    sprintf(local_info->dev_node.dev_name, "pcc1553_%d", minor);
    local_info->cei_dev.board_type = valD;    
    local_info->cei_dev.minor = minor; 
    dev_link->dev = &local_info->dev_node;
    
    if(cs_debug) 
      printk(KERN_INFO "cei_pcc1553_cs (%d):  successfully configured %s.\n", minor, local_info->dev_node.dev_name);
    
    return 0;
}


/*======================================================================
======================================================================*/
static void cei_release(unsigned long arg) {
    int status=0;
    dev_link_t* dev_link=NULL;
    LOCAL_INFO* local_info=NULL;
    int minor=-1;
    unsigned short* membase=NULL;
    unsigned short valW=0;
    
    if(cs_debug >= 2)
      printk(KERN_INFO "cei_pcc1553_cs (cei_release).\n");

    if(arg <= 0) { 
      printk(KERN_ERR "cei_pcc1553_cs (%d):  NULL address for dev_link.\n", minor);
      return;
    }
    
    dev_link = (dev_link_t*) arg;
    local_info = dev_link->priv;
    if(local_info == NULL) {
      printk(KERN_ERR "cei_pcc1553_cs (%d):  invalid address for local_info (0x%x).\n", minor, (unsigned long)local_info);
      return;
    }
   
    minor=dev_link->dev->minor;
    
    if(!(dev_link->state & DEV_CONFIG)) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s not configured.\n", minor, dev_link->dev->dev_name);
      return;  
    }

    membase = (unsigned short*)(local_info->cei_dev.membase + 0x1000);  
    
    // if board is running stop BC, BM, or RT operations
    valW = readw(membase);
    if((valW & 0xE) && (valW != 0xFFFF)) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s running, stopping board operations.\n", minor, dev_link->dev->dev_name);
      writew((valW & ~0xE), membase);
    } 

    // check if device is open      
    if(dev_link->open == 1) {
      printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s released while open.\n", minor, dev_link->dev->dev_name);
      MOD_DEC_USE_COUNT;
    }

    // unmap board memory
    iounmap((char*)local_info->cei_dev.membase);
   
    // remove region allocation
    if(dev_link->win) {
      if((status = CardServices(ReleaseWindow, dev_link->win)) != CS_SUCCESS)  
        cs_error(dev_link->handle, ReleaseWindow, status);
      dev_link->win = 0;
    }

    dev_link->state &= ~DEV_CONFIG;

    // if previous "detach" attempt failed, try again   
    if(dev_link->state & DEV_STALE_LINK) 
      cei_detach(dev_link);    

    if(cs_debug)
      printk(KERN_NOTICE "cei_pcc1553_cs (%d):  successfully released %s.\n", minor, dev_link->dev->dev_name);
} 


/*======================================================================
======================================================================*/
static int cei_event(event_t event, int priority, event_callback_args_t* args) {
    int status=0;	
    dev_link_t* dev_link=NULL;

    if(cs_debug)
      printk(KERN_INFO "cei_pcc1553_cs (cei_event):  event(0x%x)\n", event);
   
    if((dev_link = args->client_data) == NULL) {
      printk(KERN_ERR "cei_pcc1553_cs:  NULL address for dev_link.\n");
      return;
    } 
    
    switch (event) {
    case CS_EVENT_CARD_INSERTION:   // 0x000004
        dev_link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
        status = cei_config(dev_link);
        break;
    case CS_EVENT_CARD_REMOVAL:     // 0x000008
    case CS_EVENT_RESET_REQUEST:    // 0x000100
    case CS_EVENT_RESET_PHYSICAL:   // 0x000200
    case CS_EVENT_EJECTION_REQUEST: // 0x010000
        if(dev_link->state & DEV_CONFIG) 
          cei_release((u_long) dev_link);
        break;
    case CS_EVENT_CARD_RESET:	    // 0x000400
        if(!(dev_link->state & DEV_CONFIG))
          status = cei_config(dev_link);
        break;
    case CS_EVENT_PM_SUSPEND:       // 0x002000
        // PCCARD-1553 does not have power management capability (APM or ACPI)
        printk(KERN_WARNING "cei_pcc1553_cs (%d):  cannot suspend.  %s does not support power management.\n", dev_link->dev->minor, dev_link->dev->dev_name);
        break;
    case CS_EVENT_PM_RESUME:        // 0x004000
        // PCCARD-1553 does not have power management capability (APM or ACPI)
        printk(KERN_WARNING "cei_pcc1553_cs (%d):  cannot resume.  %s does not support power management.\n", dev_link->dev->minor, dev_link->dev->dev_name);
        break;
    default:
        printk(KERN_WARNING "cei_pcc1553_cs (%d):  unhandled event (0x%x)\n", dev_link->dev->minor, event);
        return -1;
    }

    return status;
} 


/*======================================================================
======================================================================*/
static int cei_open(struct inode* inode, struct file* filp) {
   int i;
   int minor = MINOR(inode->i_rdev);
   LOCAL_INFO* local_info=NULL;
   dev_link_t* dev_link=NULL;
 
   if(cs_debug >= 2) 
     printk(KERN_INFO "cei_pcc1553_cs (cei_open).\n");

   for(i=0; i<MAX_BOARDS; i++) {
     if(local_info_table[i] != NULL) {
       local_info = local_info_table[i];
       dev_link = &(local_info->dev_link);
       if(local_info->dev_node.minor == minor) 
         break;
     }
   }
   if(i == MAX_BOARDS) {
      printk(KERN_WARNING "cei_pcc1553_cs:  no PCCARD-1553 devices attached.\n");
      return -ENODEV; 
   }

   if(!(dev_link->state & DEV_CONFIG)) {
     printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s not configured.\n", minor, dev_link->dev->dev_name);
     return -ENODEV;
   }
      
   if(dev_link->open == 1) {
     printk(KERN_WARNING "cei_pcc1553_cs (%d):  %s already open.\n", minor, dev_link->dev->dev_name);
     return -1;      
   }
   dev_link->open = 1;   
   filp->private_data = local_info;  // use file->private_data to access device data 
   MOD_INC_USE_COUNT;
   
   if(cs_debug >= 2) 
     printk(KERN_INFO "cei_pcc1553_cs (%d):  CSC register = 0x%x\n", minor, readw((unsigned short*)local_info->cei_dev.membase));

   if(cs_debug) 
     printk(KERN_INFO "cei_pcc1553_cs (%d):  successfully opened %s.\n", minor, dev_link->dev->dev_name);
      
   return 0;
} 


/*======================================================================
======================================================================*/
static int cei_close(struct inode* inode, struct file* filp) {
   LOCAL_INFO* local_info = (LOCAL_INFO*)filp->private_data;
   dev_link_t* dev_link=NULL;
   
   if(cs_debug >= 2)    
     printk(KERN_INFO "cei_pcc1553_cs (cei_close).\n");
   
   if(local_info == NULL)
     return -ENODEV;
   
   dev_link = &(local_info->dev_link);
   
   if(dev_link == NULL)
     return -ENODEV;
   if(dev_link->open == 0)
     return 0;

   dev_link->open = 0;  
   filp->private_data = NULL; 
   MOD_DEC_USE_COUNT;

   if(cs_debug) 
     printk(KERN_INFO "cei_pcc1553_cs (%d):  successfully closed %s\n", MINOR(inode->i_rdev), dev_link->dev->dev_name);
     
   return 0;
} 


/*======================================================================
======================================================================*/
static int cei_ioctl(struct inode* inode, struct file* filp, u_int cmd, u_long arg) {
   int minor=MINOR(inode->i_rdev);
   LOCAL_INFO* local_info = (LOCAL_INFO*)filp->private_data;
   CEI_DEV* cei_dev=NULL;
   
   if(cs_debug)  
     printk(KERN_INFO "cei_pcc1553_cs (cei_ioctl).\n");

   if(local_info == NULL) {
     printk(KERN_ERR "cei_pcc1553_cs (%d):  NULL address for local_info.\n", minor);
     return -ENODEV;
   }

   cei_dev = &(local_info->cei_dev);
   if(cei_dev == NULL) {
     printk(KERN_ERR "cei_pcc1553_cs (%d):  NULL address for cei_dev.\n", minor);
     return -ENODEV;
   } 
   
   switch(cmd)
   {
      case SET_REGION:
         break;	
      case GET_REGION_SIZE:
         put_user(cei_dev->size,(u_long*)arg);
         if(cs_debug >= 2) 
           printk("cei_pcc1553_cs (%d):  cmd(0x%x), SIZE (0x%lx).\n",minor,cmd,cei_dev->size);
         break;
      case GET_REGION_MEM:
         put_user(cei_dev->phybase,(u_long*)arg);
         if(cs_debug >= 2) 
           printk("cei_pcc1553_cs (%d):  cmd(0x%x), PHYBASE (0x%lx).\n",minor,cmd,cei_dev->phybase);
         break;
//      case SET_PID:
//         get_user(cei_dev->pid,(u_long*)arg);
//         if(cs_debug >= 2) 
//           printk("cei_pcc1553_cs (%d):  cmd(0x%x), PID (%ld).\n",minor,cmd,cei_dev->pid);
//         break;
      default:
         return -EINVAL;
   }

   return 0;
}


/*======================================================================
======================================================================*/
static int cei_mmap(struct file* filp, struct vm_area_struct* vma) {
   int status=0;
   int minor=-1;
   unsigned long vma_size=0;
   unsigned long brd_size=0;
   unsigned long phys_addr=0;
   LOCAL_INFO* local_info=NULL;
   CEI_DEV* cei_dev=NULL;

   if(cs_debug >= 2)
     printk(KERN_INFO "cei_pcc1553_cs (cei_mmap).\n");

   minor = minor(filp->f_dentry->d_inode->i_rdev);
   if((local_info = (LOCAL_INFO*)filp->private_data) == NULL) {
     printk(KERN_ERR "cei_pcc1553_cs (%d):  NULL address for local_info.\n", minor);
     return -ENODEV;
   }
    
   if((cei_dev = &(local_info->cei_dev)) == NULL) {
     printk(KERN_ERR "cei_pcc1553_cs (%d):  NULL address for cei_dev.\n", minor);
     return -ENODEV;
   } 

   vma_size = vma->vm_end - vma->vm_start;
   brd_size = cei_dev->size;
   if(vma_size != brd_size) { 
     printk(KERN_ERR "cei_pcc1553_cs (%d):  vma size mismatch - driver 0x%lx, vma 0x%lx.\n",minor,brd_size,vma_size);
     return -EFAULT;
   }  
  #if LINUX_VERSION_CODE >= 0x20324
   phys_addr = cei_dev->phybase + vma->vm_pgoff;
  #else
   phys_addr = cei_dev->phybase + vma->vm_offset;
  #endif    
   vma->vm_flags |= (VM_RESERVED | VM_IO); 
 
  // if Red Hat Linux 9 or Red Hat Workstation 3.0  
  #if LINUX_VERSION_CODE == KERNEL_VERSION(2,4,20) || LINUX_VERSION_CODE == KERNEL_VERSION(2,4,21)   
   if((status = remap_page_range(vma, vma->vm_start, phys_addr, vma_size, vma->vm_page_prot)) != 0) {
  #else
   if((status = remap_page_range(vma->vm_start, phys_addr, vma_size, vma->vm_page_prot)) !=0 ) {
  #endif
     printk(KERN_ERR "cei_pcc1553_cs(cei_mmap):  failed to map device - status: %d.\n",minor,status);
     return status;
   }

   if(cs_debug >= 2)
     printk(KERN_INFO "cei_pcc1553_cs (%d):  mmap phy_addr: 0x%lx, size: 0x%lx\n", minor, phys_addr, vma_size);

   if(cs_debug)
     printk(KERN_INFO "cei_pcc1553_cs (%d):  successfully mapped board.\n", minor);
      
   return 0;
}


/*======================================================================
======================================================================*/
static int init_cei_pcc1553_cs(void) {
    int status=-1;
    servinfo_t serv;
  
   if(cs_debug >= 2) 
     printk(KERN_INFO "cei_pcc1553_cs (init_cei_pcc1553_cs).\n");
   if(cs_debug)
     printk(KERN_INFO "cei_pcc1553_cs:  %s.  DEBUG (%d).\n", version, cs_debug);

    if((status = CardServices(GetCardServicesInfo, &serv)) == CS_SUCCESS) {
      if(cs_debug >= 2) 
        printk(KERN_INFO "cei_pcc1553_cs:  Card Services info:  sockets(%d), rev(0x%x), level(%d), vendor string (%s)\n", serv.Count, serv.Revision, serv.CSLevel, serv.VendorString);  
      // check if the version of Card Services is the same as the version of Card Services installed in the kernel
      if(serv.Revision != CS_RELEASE_CODE) {
        printk(KERN_WARNING "cei_pcc1553_cs:  Card Services rev (0x%x) does not match kernel Card Services rev (0x%x).\n", serv.Revision, CS_RELEASE_CODE);
        return -EINVAL;
      }
    }

    if((status = register_pccard_driver(&dev_info, &cei_attach, &cei_detach)) != CS_SUCCESS) {
      printk(KERN_WARNING "cei_pcc1553_cs:  unable to register device.  Out of memory.\n");
      return status;
    }

    // dynamically determine major number
    status = register_chrdev(0, dev_info, &cei_pcc1553_cs_fops);
    if(status == -EINVAL || status==0) {
      printk(KERN_WARNING "cei_pcc1553_cs:  invalid argument.\n");
      return -ENODEV;
    }
    else if(status == -EBUSY) {
      printk(KERN_WARNING "cei_pcc1553_cs:  device or resource busy.\n");
      return -ENODEV;
    }
    else
      major_dev = status;

    if(cs_debug)
      printk(KERN_INFO "cei_pcc1553_cs:  loading successful.\n");
       
    return 0;
}

/*======================================================================
======================================================================*/
static void exit_cei_pcc1553_cs(void) {
    int i;
    dev_link_t* dev_link=NULL;
   
    if(cs_debug >= 2)
      printk(KERN_INFO "cei_pcc1553_cs (exit_cei_pcc1553_cs).\n");
  
    // removes any PCCARD-1553 board still attached to Card Services
    for(i = 0; i < MAX_BOARDS; i++) {
      if(local_info_table[i] != NULL) {	    
        dev_link = &(local_info_table[i]->dev_link);
        printk(KERN_WARNING "cei_pcc1553_cs (%d):  device %s still attached.\n", dev_link->dev->minor, dev_link->dev->dev_name);
        cei_detach(dev_link);
      }      
    }

    unregister_chrdev(major_dev, dev_info);
    if(unregister_pccard_driver(&dev_info) != CS_SUCCESS) 
      printk(KERN_WARNING "cei_pcc1553_cs:  no such device %s.\n", dev_info);
    
    if(cs_debug)
      printk(KERN_INFO "cei_pcc1553_cs:  unloading successful.\n");
}


module_init(init_cei_pcc1553_cs);
module_exit(exit_cei_pcc1553_cs);
