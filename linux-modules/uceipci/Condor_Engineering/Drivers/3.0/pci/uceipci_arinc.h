/*============================================================================*
 * FILE:                      U C E I P C I _ A R I N C . H
 *============================================================================*
 *
 * COPYRIGHT (C) 2006 - 2016 BY
 *          ABACO SYSTEMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO
 *          SYSTEMS.
 *
 *===========================================================================*
 * FUNCTION:   uceipci driver subroutines for ARINC devices
 *
 *             Supplements the universal PCI driver for Abaco Systems avionics
 *             CEI-x20, CEI-x30, CEI-500, P-708, P-SER, P-DIS devices.
 *
 *
 * ENTRY POINTS:
 *    initialize_arinc       called by uceipci_probe 
 *
 *    uninitialize_arinc     called by uceipci_remove
 *
 *    intrpt_cntrl_arinc     called by intrpt_cntrl
 *
 *    uceipci_x20_isr        interrupt service routine for CEI-x20 boards
 *
 *    uceipci_pser_isr       interrupt service routine for the P-Ser 
 *
 *    uceipci_x30_isr        interrupt service routine for CEI-x30 boards 
 *
 *===========================================================================*/

/* $Revision:  1.06 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/11/2011   initial. bch
  04/24/2013   added support for CEI-830X820. bch
  11/19/2013   added support for CEI-830A. bch
  05/08/2014   added support for the RP-708. bch
  01/15/2015   added support for the RAR-XMC. bch
  08/17/2015   removed IRQF_DISABLED for ISR. bch
  10/10/2016   added support for the RAR-MPCIe. bch
*/


// function prototypes
static int initialize_arinc(struct pci_dev* pdev);
static int uninitialize_arinc(struct pci_dev* pdev);

#ifndef NO_SYSFS_SUPPORT  	
// setup the attributes for the sysfs
 static struct attribute* btARINC_attrs[] = {
	&dev_attr_boardname.attr,
	&dev_attr_board_type.attr,
	&dev_attr_status.attr,
	&dev_attr_id.attr,
	NULL,
 };
 static struct attribute_group cei_arinc_attr_group = {
	.name = "board",
	.attrs = btARINC_attrs,
 };
#endif

#ifndef NO_HW_INTERRUPTS
 // byte offset for hardware interrupt register
 #define CEIX20_ICR_OFFSET  0x400000
 #define PSER_CSR_OFFSET    0
 #define CEIX30_CSR_OFFSET  0
 static irqreturn_t uceipci_x20_isr(int irq, void* data);
 static irqreturn_t uceipci_pser_isr(int irq, void* data);
 static irqreturn_t uceipci_x30_isr(int irq, void* data);
#endif

static int initialize_arinc(struct pci_dev* pdev) {
  DEV_DATA* pData=NULL;
  unsigned short devId=pdev->device;
  u32 valD=0;
 #ifndef NO_SYSFS_SUPPORT
  int status=0;
 #endif

  if((pData = (DEV_DATA*) pci_get_drvdata(pdev)) == NULL) {
    printk(KERN_ERR "uceipci:  failed to get device data (PCI: bus %d, dev %d).\n", pdev->bus->number,(pdev->devfn)>>3); 
    return -EFAULT;
  }

  // config based on board 
  switch(pdev->device) {
  case 0x1024:
    strcpy(pData->boardname, "CEI-500 (Acromag APC8620)");
    devId = 0x500;
    break;
  case 0x0040:
    strcpy(pData->boardname, "CEI-500 (SBS PCI-40A)");
    devId = 0x500;
    break;
  case 0x430:
    strcpy(pData->boardname, "CEI-430");
    break;
  case 0x430A:
    strcpy(pData->boardname, "CEI-430A");
    break;
  case 0x520:
    strcpy(pData->boardname, "CEI-520");
    break;
  case 0x530:
    strcpy(pData->boardname, "RCEI-530");
    break;
  case 0x620:
    strcpy(pData->boardname, "CEI-620");
    break;
  case 0x630:
    strcpy(pData->boardname, "RAR-CPCI");
    break;
  case 0x708:
    strcpy(pData->boardname, "P-708");
    break;
  case 0x708A:
    strcpy(pData->boardname, "RP-708");
    break;
  case 0x820:
    strcpy(pData->boardname, "CEI-820");
    break;
  case 0x821:
    strcpy(pData->boardname, "CEI-821");
    break;
  case 0x830:
    strcpy(pData->boardname, "CEI-830");
    break;
  case 0x831:
    strcpy(pData->boardname, "RCEI-830RX");
    break;
  case 0x832:
    strcpy(pData->boardname, "CEI-830X820");
    break;
  case 0x1004:
    strcpy(pData->boardname, "P-SER");
    // disable hardware interrupts (PLX-9056: INTCSR (PCI:68h)
    //  PCI Interrupt Enable and Local Interrupt Input Enable register)
    valD = readl(((u32*)pData->pci_bar_conf_membase) + 26) & ~0x00000900;
    writel(valD, ((u32*)pData->pci_bar_conf_membase) + 26);
    break;
  case 0x1005:
    strcpy(pData->boardname, "P-MIO");
    break;
  case 0x1006:
    strcpy(pData->boardname, "P-DIS");
    break;
  case 0x1009:
    strcpy(pData->boardname, "AMC-A30");
    break;
  case 0x100A:
    strcpy(pData->boardname, "RAR-EC");
    break;
  case 0x100B:
    strcpy(pData->boardname, "RAR-PCIE");
    break;
  case 0x830A:
    strcpy(pData->boardname, "CEI-830A");
    break;
  case 0x100C:
    strcpy(pData->boardname, "RAR-XMC");
    break;
  case 0x100D:
    strcpy(pData->boardname, "RAR-MPCIe");
    break;
  default:
    strcpy(pData->boardname, "unsupported");
    printk(KERN_ERR "uceipci:  invalid PCI device ID - 0%x.\n", pdev->device);
    pci_set_drvdata(pdev, pData);
    return -EFAULT;
  }

 #ifndef NO_HW_INTERRUPTS
 #ifdef HW_INTERRUPTS_WAITQUEUE
  switch(pdev->device) {
  case 0x820:
  case 0x1004:  
  case 0x430:
  case 0x430A:
  case 0x530:
  case 0x630:
  case 0x830:
  case 0x831:
  case 0x832:
  case 0x100A:
  case 0x100B:
  case 0x830A:
  case 0x100C:
  case 0x100D:
    // enable wait queue
    pData->status |= STATUS_HWINT_WQ;
  };
 #endif
 #endif

  pData->id = devId;
  pData->board_type = 0;

  pci_set_drvdata(pdev, pData);
  
 #ifndef NO_SYSFS_SUPPORT
  if((status=sysfs_create_group(&pdev->dev.kobj, &cei_arinc_attr_group)) != 0)
    printk(KERN_ERR "uceipci(%d):  failed to create sysfs group, status %d\n", pData->minor, status);
 #endif
  
  return 0;
}


static int uninitialize_arinc(struct pci_dev* pdev) {
 #ifndef NO_SYSFS_SUPPORT
  sysfs_remove_group(&pdev->dev.kobj, &cei_arinc_attr_group);
 #endif

  return 0;
}

#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl_arinc(DEV_DATA* pData, int mode) {
  int status=0;

  if(mode == 0) {
    free_irq(pData->irq, (void*)pData);  // returns when all executing interrupts are done
    pData->status &= ~STATUS_INT_ENABLE;
    if(uceipci_debug >= 2)
      printk(KERN_INFO "uceipci(%d):  unregistered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->irq, (unsigned long) pData);
   #ifdef HW_INTERRUPTS_WAITQUEUE
    if(pData->status & STATUS_HWINT_WQ) {
      pData->cur_intrpt[0]++; 
      wake_up_interruptible(&(pData->isr_wait_q[0]));  // wake the user process held in uceipci_read
    }
   #endif
  }
  else if(mode == 1) {
   #ifdef HW_INTERRUPTS_WAITQUEUE
    if(pData->status & STATUS_HWINT_WQ)
      pData->last_intrpt[0] = pData->cur_intrpt[0] = 0;
   #endif
    switch(pData->id) {
    case 0x820:
      pData->hwr_chan_addr[0] = pData->pci_bar_laddr_membase + CEIX20_ICR_OFFSET;
      status = request_irq(pData->irq, uceipci_x20_isr, IRQF_SHARED, DRIVER_NAME, (void*)pData);
      break;
    case 0x1004:  // P-SER
      pData->hwr_chan_addr[0] = pData->pci_bar_laddr_membase + PSER_CSR_OFFSET;
      status = request_irq(pData->irq, uceipci_pser_isr, IRQF_SHARED, DRIVER_NAME, (void*)pData);
      break;
    case 0x430:
    case 0x430A:
    case 0x530:
    case 0x630:
    case 0x830:
    case 0x831:
    case 0x832:
    case 0x100A:
    case 0x100B:
    case 0x830A:
    case 0x100C:
    case 0x100D:
      pData->hwr_chan_addr[0] = pData->pci_bar_laddr_membase + CEIX30_CSR_OFFSET;
      status = request_irq(pData->irq, uceipci_x30_isr, IRQF_SHARED, DRIVER_NAME, (void*)pData);
      break;
    default:
      printk(KERN_INFO "uceipci(%d):  hardware interrupt support not available on %s (0x%x)\n", pData->minor, pData->boardname, pData->id);
      return -1;
    };

    if(status != 0) {
      printk(KERN_WARNING "uceipci(%d):  unable to register interrupt handler IRQ (%d), status (%d).\n",pData->minor, pData->irq, status);
      return status;
    }
    else {
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  registered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->irq, (unsigned long) pData);
    }
    pData->status |= STATUS_INT_ENABLE;
  }
 
  return 0;
}


// Handles the hardware interrupts generated by the CEI-820 (only)
// Notes:
//  1. if using kernel 2.6.21 or later with CONFIG_DEBUG_SHIRQ defined, and
//      IRQF_SHARED flag is set, then the ISR will be call (no PCI assertion)
//      when requesting and freeing the interrupt line. 
static irqreturn_t uceipci_x20_isr(int irq, void* data) {
   DEV_DATA* pData = (DEV_DATA*) data;
   unsigned short icr_reg=0;

   if(pData == NULL) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci:  uceipci_x20_isr - no data pointer, IRQ(%d)\n", irq);
     return IRQ_NONE;
   }

   if(uceipci_debug >= 2)
     printk(KERN_INFO "uceipci(%d): uceipci_x20_isr\n", pData->minor);

   // return if hardware interrupts are not enabled
   if(!(pData->status & STATUS_INT_ENABLE)) {
    #ifdef CONFIG_DEBUG_SHIRQ
     if(uceipci_debug >= 2)
       printk(KERN_INFO "uceipci(%d):  ISR called before interrupts enabled - IRQ (%d), could be the kernel check for a valid ISR\n", pData->minor, irq);
     return IRQ_HANDLED;
    #endif
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  received an interrupt before interrupts enabled - IRQ (%d), possible shared IRQ\n", pData->minor, irq);
     return IRQ_NONE;
   }   
   
   if(pData->irq != irq) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  IRQ mismatch (%d, %d)\n", pData->minor, pData->irq, irq);
     return IRQ_NONE;
   }

   // read the ICR
   icr_reg = readw((unsigned short*)(pData->hwr_chan_addr[0]));
   // check for interrupt
   if(icr_reg & 0x1) {
     // clear interrupt register
     writew(icr_reg & ~0x1, (unsigned short*)(pData->hwr_chan_addr[0]));
     if(uceipci_debug >= 3)
       printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), cntrl_reg (0x%x).\n", pData->minor, pData->irq, icr_reg);
    #ifdef HW_INTERRUPTS_WAITQUEUE
     if(pData->status & STATUS_HWINT_WQ) {
       pData->cur_intrpt[0]++;
       wake_up_interruptible(&(pData->isr_wait_q[0]));  // wake any thread waiting to notify a hardware interrupt occured
       if(uceipci_debug >= 2)
         printk(KERN_INFO "uceipci(%d):  wake wait queue (0) - count (%d)\n", pData->minor, pData->cur_intrpt[0]);
     }
    #endif
     return IRQ_HANDLED;
   }
 
   if(uceipci_debug >= 3) {
    #ifdef CONFIG_DEBUG_SHIRQ
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d), could be the kernel check for a valid ISR\n",pData->minor,irq,pData->irq);
    #else
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d)\n",pData->minor,irq,pData->irq);
    #endif
   }
  
   return IRQ_NONE;
}


// Handles the hardware interrupts generated by the P-Ser board
// Notes:
//  1. if CONFIG_DEBUG_SHIRQ defined and IRQF_SHARED flag is set, then the ISR
//     will be call (no PCI assertion) when requesting and freeing the interrupt line. 
static irqreturn_t uceipci_pser_isr(int irq, void* data) {
   DEV_DATA* pData = (DEV_DATA*) data;
   unsigned int csr_reg=0;

   if(pData == NULL) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci:  uceipci_pser_isr - no data pointer, IRQ(%d)\n", irq);
     return IRQ_NONE;
   }

   if(uceipci_debug >= 2)
     printk(KERN_INFO "uceipci(%d): uceipci_pser_isr\n", pData->minor);

   // return if hardware interrupts are not enabled
   if(!(pData->status & STATUS_INT_ENABLE)) {
    #ifdef CONFIG_DEBUG_SHIRQ
     if(uceipci_debug >= 2)
       printk(KERN_INFO "uceipci(%d):  ISR called before interrupts enabled - IRQ (%d), could be the kernel check for a valid ISR\n", pData->minor, irq);
     return IRQ_HANDLED;
    #endif
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  received an interrupt before interrupts enabled - IRQ (%d), possible shared IRQ\n", pData->minor, irq);
     return IRQ_NONE;
   }

   if(pData->irq != irq) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  IRQ mismatch (%d, %d)\n", pData->minor, pData->irq, irq);
     return IRQ_NONE;
   }

   // read the ICR
   csr_reg = readl((unsigned int*)(pData->hwr_chan_addr[0]));
   // check for interrupt
   if(csr_reg & 0x10) {
     // clear interrupt register
     writel(csr_reg | 0x10, (unsigned int*)(pData->hwr_chan_addr[0]));
     if(uceipci_debug >= 3) {
       csr_reg = readl((unsigned int*)(pData->hwr_chan_addr[0]));
       printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), csr (0x%x).\n", pData->minor, pData->irq, csr_reg);
     }
    #ifdef HW_INTERRUPTS_WAITQUEUE
     if(pData->status & STATUS_HWINT_WQ) {
       pData->cur_intrpt[0]++;
       wake_up_interruptible(&(pData->isr_wait_q[0]));  // wake any process/thread waiting to be notified that a hardware interrupt occured
       if(uceipci_debug >= 2)
         printk(KERN_INFO "uceipci(%d):  wake wait queue (0) - count (%d)\n", pData->minor, pData->cur_intrpt[0]);
     }
    #endif
     return IRQ_HANDLED;
   }

   if(uceipci_debug >= 3) {
    #ifdef CONFIG_DEBUG_SHIRQ
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d), could be the kernel check for a valid ISR\n",pData->minor,irq,pData->irq);
    #else
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d)\n",pData->minor,irq,pData->irq);
    #endif
   }

   return IRQ_NONE;
}


// Handles the hardware interrupts generated by the P-Ser board
// Notes:
//  1. if CONFIG_DEBUG_SHIRQ defined and IRQF_SHARED flag is set, then the ISR
//     will be call (no PCI assertion) when requesting and freeing the interrupt line.
static irqreturn_t uceipci_x30_isr(int irq, void* data) {
   DEV_DATA* pData = (DEV_DATA*) data;
   unsigned int csr_reg=0;

   if(pData == NULL) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci:  uceipci_x30_isr - no data pointer, IRQ(%d)\n", irq);
     return IRQ_NONE;
   }

   if(uceipci_debug >= 2)
     printk(KERN_INFO "uceipci(%d): uceipci_x30_isr\n", pData->minor);

   // return if hardware interrupts are not enabled
   if(!(pData->status & STATUS_INT_ENABLE)) {
    #ifdef CONFIG_DEBUG_SHIRQ
     if(uceipci_debug >= 2)
       printk(KERN_INFO "uceipci(%d):  ISR called before interrupts enabled - IRQ (%d), could be the kernel check for a valid ISR\n", pData->minor, irq);
     return IRQ_HANDLED;
    #endif
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  received an interrupt before interrupts enabled - IRQ (%d), possible shared IRQ\n", pData->minor, irq);
     return IRQ_NONE;
   }

   if(pData->irq != irq) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci(%d):  IRQ mismatch (%d, %d)\n", pData->minor, pData->irq, irq);
     return IRQ_NONE;
   }

   // read the ICR
   csr_reg = readl((unsigned int*)(pData->hwr_chan_addr[0]));
   // check for interrupt
   if(csr_reg & 0x10) {
     // clear interrupt register
     if((csr_reg & 0xFF00) >= 0x4D00)  // FW v4.13+ CMD usage
       writel(0x40000010, (unsigned int*)(pData->hwr_chan_addr[0]));
     else
       writel(csr_reg | 0x10, (unsigned int*)(pData->hwr_chan_addr[0]));
     if(uceipci_debug >= 3) {
       csr_reg = readl((unsigned int*)(pData->hwr_chan_addr[0]));
       printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), csr (0x%x).\n", pData->minor, pData->irq, csr_reg);
     }
    #ifdef HW_INTERRUPTS_WAITQUEUE
     if(pData->status & STATUS_HWINT_WQ) {
       pData->cur_intrpt[0]++;
       wake_up_interruptible(&(pData->isr_wait_q[0]));  // wake any process/thread waiting to be notified that a hardware interrupt occured
       if(uceipci_debug >= 2)
         printk(KERN_INFO "uceipci(%d):  wake wait queue (0) - count (%d)\n", pData->minor, pData->cur_intrpt[0]);
     }
    #endif
     return IRQ_HANDLED;
   }

   if(uceipci_debug >= 3) {
    #ifdef CONFIG_DEBUG_SHIRQ
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d), could be the kernel check for a valid ISR\n",pData->minor,irq,pData->irq);
    #else
     printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d)\n",pData->minor,irq,pData->irq);
    #endif
   }

   return IRQ_NONE;
}
#endif
