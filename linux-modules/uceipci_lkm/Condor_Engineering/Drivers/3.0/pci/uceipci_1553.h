/*============================================================================*
 * FILE:                      U C E I P C I _ 1 5 5 3. H
 *============================================================================*
 *
 * COPYRIGHT (C) 2005 - 2019 BY
 *          ABACO SYSTEMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS.
 *
 *===========================================================================*
 * FUNCTION:   uceipci driver subroutines for MIL-STD-1553 devices
 *
 *             Supplements the universal PCI driver for Abaco Systems
 *             avionics MIL-STD-1553 devices.
 *
 *
 * ENTRY POINTS:
 *    initialize_1553       called by uceipci_probe 
 *
 *    uninitialize_1553     called by uceipci_remove
 * 
 *    intrpt_cntrl_1553     called by intrpt_cntrl
 *
 *    uceipci_1553_isr      interrupt service routine for UCA boards
 *
 *===========================================================================*/

/* $Revision:  1.08 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  10/11/2011   initial. bch
  10/09/2012   added RAR15-XMC-XT and RAR15-XMC. added support for UCA32
                firmware. bch
  01/14/2014   added R15-PMC. added "csc" to SYSFS. bch
  08/17/2015   removed IRQF_DISABLED for ISR. bch
  05/25/2016   added MSI support for PCIe boards. bch
  10/10/2016   added support for the R15-MPCIe. bch
  09/12/2017   added UCA32 support for RXMC-1553 and R15-AMC. bch
  09/26/2017   included "linux/sched/signal.h" for send_sig_info. bch
  11/18/2019   modified uceipci_1553_isr for kernel 4.20.x. bch
*/


#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0))
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#ifndef NO_HW_INTERRUPTS
 #include <linux/interrupt.h>
 // UCA32
 #define UCA32_HWR_CHAN_OFFSET   0x00004000
 #define UCA32_FW_VER_MSI_SUPPORT 0x0609
 // Multi-protocol ARINC
 #define MP_HWR_ARINC_OFFSET     0x00100000
 #define MP_ARINC_CHANNEL        (MAX_CHANNELS - 1)
 // UCA
 #define UCA_HWR_CHAN_OFFSET     0x00200000
 // CORE
 #define CORE_CHAN_REG_OFFSET    0x00020000
 static irqreturn_t uceipci_1553_isr(int irq, void* data);
#endif  
// offset to hardware register 
#define UCA32_HWR_OFFSET        0x00030000
#define UCA_HWR_OFFSET          0x00000800
#define CORE_GLOBAL_REG_OFFSET  0x00200000

#ifndef NO_SYSFS_SUPPORT  
 // setup the attributes for the sysfs
 bd_info_attr(mode, "%d\n")
 bd_info_attr(ch, "%d\n")
 bd_info_attr(irig, "%d\n")
 bd_info_attr(bus_type, "%d\n")
 bd_info_attr(csc, "0x%X\n")
 static struct attribute* bt1553_attrs[] = {
  &dev_attr_boardname.attr,
  &dev_attr_mode.attr,
  &dev_attr_ch.attr,
  &dev_attr_irig.attr,
  &dev_attr_bus_type.attr,
  &dev_attr_id.attr,
  &dev_attr_board_type.attr,
  &dev_attr_csc.attr,
  &dev_attr_status.attr,
  NULL,
 };
 static struct attribute_group cei_1553_attr_group = {
	.name = "board",
	.attrs = bt1553_attrs,
 };
#endif


static int initialize_1553(struct pci_dev* pdev) {
  int i, status=0;
  DEV_DATA* pData;
  unsigned short valW1=0; 
  unsigned short valW2=0; 

  if((pData = (DEV_DATA*) pci_get_drvdata(pdev)) == NULL) {
    printk(KERN_ERR "uceipci:  failed to get device data (PCI: bus %d, dev %d).\n", pdev->bus->number,(pdev->devfn)>>3); 
    return -EFAULT;
  }
  
  switch(pdev->device) {
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
    // read the CSC from the board's host interface register
    valW1 = readw((unsigned short*)pData->pci_bar_laddr_membase);
    if(uceipci_debug >= 1)
      printk(KERN_INFO "uceipci(%d):  CSC - 0x%X\n", pData->minor, valW1);
    break;
  case 0x1003:
    // configure PLX9056 Local Configuration Register with default values
    writel(0x100C767E, ((u32*)pData->pci_bar_conf_membase) + 27); // CNTRL register
    writel(0x42030343, ((u32*)pData->pci_bar_conf_membase) + 6);  // LBRD0 register
    // read Hardware Capabilities register
    valW1 = readw((u16*)(pData->pci_bar_laddr_membase + 0x200004));
    if(uceipci_debug >= 1)
      printk(KERN_INFO "uceipci(%d):  HCAP - 0x%X\n", pData->minor, valW1);
    // read Hardware ID register
    valW2 = readw((u16*)(pData->pci_bar_laddr_membase + 0x200000));
    if(uceipci_debug >= 1)
      printk(KERN_INFO "uceipci(%d):  HWID - 0x%X\n", pData->minor, valW2);
  };
  pData->csc = valW1;

  // read/set board configuration
  switch(pdev->device) {
  case 0x1553:
    if(pci_resource_len(pdev, 0) >= 0x2000000) {  // board is a PCI-1553, cPCI-1553, or PMC-1553 (BAR0-32MB)
      valW1 = valW1 >> 8;
      pData->board_type = (valW1 & 0xC0);
      if(valW1 | 0x3)  // CH 1 and CH 2 multiple mode
        pData->mode = 1;
      pData->ch++;
      if(valW1 & 0x10)  // CH 1 present
        pData->ch++;	
      if(valW1 & 0x20)  // CH 2 present 
        pData->ch++;	
      pData->id = 0x40;
      pData->irig = 0;
      pData->bus_type = 2;
      pData->status = 0;
      strcpy(pData->boardname, "PCI-1553, cPCI-1553 or PMC-1553");
     #ifndef NO_HW_INTERRUPTS
      for(i=0;i<pData->ch;i++)
        pData->hwr_chan_addr[i] = (pData->pci_bar_laddr_membase + 0x00400000 + (i*0x01000000));
     #endif
    }
    else if(pci_resource_len(pdev, 0) < 0x2000000) {
      pData->board_type = ((valW1 & 0x1F) >> 1);
      switch(pData->board_type) {
      case 1:  // QPMC-1553 (BAR0-8MB)
        pData->id = 0x110;
        strcpy(pData->boardname, "QPM(C)-1553");
        break;
      case 5:  // Q104-1553P (BAR0-8MB)
        pData->id = 0x180;
        strcpy(pData->boardname, "Q104-1553P");
        break;
      default:
        strcpy(pData->boardname, "unsupported");
        printk(KERN_ERR "uceipci(%d):  invalid board type - %d.\n", pData->minor, pData->board_type);
        break;
      };
      pData->ch = (valW1 & 0x3E0) >> 6;	
      pData->irig = (valW1 & 0x1000) >> 12;
      pData->mode = (valW1 & 0x800) >> 11;
      pData->uca32 = 0;
      pData->bus_type = 2;
      pData->status = 0;
     #ifndef NO_HW_INTERRUPTS
      for(i=0;i<pData->ch;i++) {
        if(pData->uca32) 
          pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
        else
          pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
      }
     #endif
    }
    break;
  case 0x1554:
    pData->board_type = ((valW1 & 0x1F) >> 1);
    switch(pData->board_type) {
    case 3:  // QPCI-1553 (BAR0-12b, BAR2-8MB)
      pData->id = 0x160;
      strcpy(pData->boardname, "QPCI-1553");
      break;
    default:
      strcpy(pData->boardname, "unsupported");
      printk(KERN_ERR "uceipci(%d):  invalid board type - %d.\n", pData->minor, pData->board_type);
      break;
    };
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
   #endif
    break;
  case 0x1555:
    pData->board_type = ((valW1 & 0x1F) >> 1);
    switch(pData->board_type) {
    case 7:  // QCP-1553 (BAR0-512b, BAR1-256b, BAR2-8MB)	      
      pData->id = 0x210;
      strcpy(pData->boardname, "QCP-1553");
      break;
    default:
      strcpy(pData->boardname, "unsupported");
      printk(KERN_ERR "uceipci(%d):  invalid board type - %d.\n", pData->minor, pData->board_type);
      break;
    };
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x1008:  // AMC-1553 (BAR2-8MB)
    pData->id = 0x110;
    strcpy(pData->boardname, "AMC-1553");
    pData->board_type = ((valW1 & 0x1F) >> 1);
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
   #endif
    break;
  case 0x1003:
    pData->id = pdev->device;
    strcpy(pData->boardname, "EPMC-1553");
    pData->board_type = pdev->device;
    pData->ch = 0;
    for(i=0;i<8;i++)
      if(valW1 & (0x1<<i)) pData->ch++;  // if((valW1 >> i) & 0x1) pData->ch++;
    pData->irig = ((valW1 >> 9) & 0x1);
    pData->mode = ((valW2 >> 12) & 0x1);
    pData->bus_type = 2;
    pData->status = 0;
    break;
  case 0x1556:
    pData->id = 0x220;
    strcpy(pData->boardname, "QPCX-1553");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x1557:
    pData->id = 0x230;
    strcpy(pData->boardname, "R15-EC");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x1558:
    pData->id = 0x240;
    strcpy(pData->boardname, "R15-AMC");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
     break;
  case 0x1559:
    pData->id = 0x260;
    strcpy(pData->boardname, "RXMC-1553");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x155A:
    pData->id = 0x250;
    strcpy(pData->boardname, "RPCIe-1553");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x155B:
    pData->id = 0x320;
    strcpy(pData->boardname, "R15-LPCIe");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x155C:
    pData->id = 0x300;
    strcpy(pData->boardname, "RXMC2-1553");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;	
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;	
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) 
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
      else
        pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA_HWR_OFFSET + (i*UCA_HWR_CHAN_OFFSET);
    }
   #endif
    break;
  case 0x1530:
    pData->id = 0x340;
    strcpy(pData->boardname, "RAR15-XMC");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    // 1553 channels
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
   #endif
    break;
  case 0x1542:
    pData->id = 0x360;
    strcpy(pData->boardname, "RAR15-XMC-XT");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6;
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    // 1553 channels
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
    // ARINC channel
    pData->hwr_chan_addr[MP_ARINC_CHANNEL] = pData->pci_bar_laddr_membase + MP_HWR_ARINC_OFFSET;
   #endif
    break;
  case 0x1544:
    pData->id = 0x380;
    strcpy(pData->boardname, "R15-PMC");
    pData->board_type = (valW1 >> 1) & 0x1F;
    pData->ch = (valW1 & 0x3E0) >> 6; 
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
   #endif
    break;
  case 0x1545:
    pData->id = 0x400;
    strcpy(pData->boardname, "R15-MPCIe");
    pData->board_type = ((valW1 & 0x3F) >> 1);
    pData->ch = (valW1 & 0x3E0) >> 6; 
    pData->irig = (valW1 & 0x1000) >> 12;
    pData->mode = (valW1 & 0x800) >> 11;  
    pData->uca32 = (valW1 & 0x4000) >> 14;
    pData->bus_type = 2;
    pData->status = 0;
   #ifndef NO_HW_INTERRUPTS  
    for(i=0;i<pData->ch;i++)
      pData->hwr_chan_addr[i] = pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET + (i*UCA32_HWR_CHAN_OFFSET) + 0x58;
   #endif
    break;
  default:
    strcpy(pData->boardname, "unsupported");
    printk(KERN_ERR "uceipci(%d):  invalid PCI device ID - 0x%x.\n", pData->minor, pdev->device);
    pci_set_drvdata(pdev, pData);
    return -EFAULT;
  };

  if((uceipci_debug >= 1) && (pdev->device != 0x1003)) {
    unsigned short ver_wcs_major=0, ver_wcs_minor=0, ver_lpu_major=0, ver_lpu_minor=0;
    unsigned int ver_wcs_build=0, ver_lpu_build=0, ver_fpga_build=0;
    unsigned char* hwr_addr=NULL;

    if(pData->uca32) {
      unsigned int val32=0;
      hwr_addr = (unsigned char*)pData->pci_bar_laddr_membase + UCA32_HWR_OFFSET;

      printk(KERN_INFO "uceipci(%d):  CSC(2) - 0x%04X\n", pData->minor, (readl((unsigned int*)pData->pci_bar_laddr_membase) >> 16) & 0xFFFF);
      if(uceipci_debug >= 2) {
        // if a RAR15-XMC or a RAR15-XMC-XT
        if((pData->id == 0x340) || (pData->id == 0x360))
          printk(KERN_INFO "uceipci(%d):  BSR - 0x%X\n", pData->minor, readl((unsigned int*)(pData->pci_bar_laddr_membase + 0x4)));
        for(i=0;i<pData->ch;i++)
          printk(KERN_INFO "uceipci(%d):  OPT(%d) - 0x%X\n", pData->minor, i, readl((unsigned int*)(pData->pci_bar_laddr_membase + 0x10 + (0x4+i))));
        printk(KERN_INFO "uceipci(%d):  SN - %d\n", pData->minor, readl((unsigned int*)(pData->pci_bar_laddr_membase + 0x30)));
      }
      val32 = readl(hwr_addr + 0x48);
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  WCS - 0x%X\n", pData->minor, val32);
      ver_wcs_major = (val32 & 0xFF00) >> 8 ;
      ver_wcs_minor = val32 & 0x00FF;
      ver_wcs_build = readl(hwr_addr + 0x4C);
      val32 = readl(hwr_addr + 0x40);
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  LPU - 0x%X\n", pData->minor, val32);
      ver_lpu_major = (val32 & 0xFF00) >> 8 ;
      ver_lpu_minor = val32 & 0x00FF;
      ver_lpu_build = readl(hwr_addr + 0x44);
      ver_fpga_build = readl((char*)pData->pci_bar_laddr_membase + 0x80);
      printk(KERN_INFO "uceipci(%d):  WCS v%X.%02Xb%X, LPU v%X.%02Xb%X, FPGA v%d\n", pData->minor, ver_wcs_major, ver_wcs_minor, ver_wcs_build, ver_lpu_major, ver_lpu_minor, ver_lpu_build, ver_fpga_build);

     #ifdef HW_MSI_INTERRUPTS
      // check if the board can support MSI
      if((pdev->device == 0x155A) && ((val32 & 0xFFFF) == UCA32_FW_VER_MSI_SUPPORT))
        pData->msi_mode = 1;
     #endif
    }
    else {
      unsigned int val16=0;
      hwr_addr=(unsigned char*)pData->pci_bar_laddr_membase + UCA_HWR_OFFSET;

      if(valW1 & 0x8000)
        printk(KERN_INFO "uceipci(%d):  ACR - 0x%04X\n", pData->minor, readw(((unsigned short*)pData->pci_bar_laddr_membase)+1) & 0x3FF);

      // WCS
      val16 = readw(hwr_addr + 0x30);
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  WCS - 0x%X\n", pData->minor, val16);
      ver_wcs_major = ver_lpu_major = 0x0F00;
      if(val16 < 0x0411)  // FW v4.11- 
        ver_wcs_build = (val16 & 0xF000) >> 12;
      ver_wcs_major = (val16 & ver_wcs_major) >> 8;
      ver_wcs_minor = val16 & 0x00FF;
      // LPU
      val16 = readw(hwr_addr + 0x12);
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  LPU - 0x%X\n", pData->minor, val16);
      ver_lpu_major = (val16 & ver_lpu_major) >> 8;
      ver_lpu_minor = val16 & 0x00FF;
      if(val16 >= 0x0411) 
        ver_lpu_build = readw(hwr_addr + 0x34);
      else
        ver_lpu_build = (val16 & 0xF000) >> 12;
      printk(KERN_INFO "uceipci(%d):  WCS v%X.%02Xb%X, LPU v%X.%02Xb%X\n", pData->minor, ver_wcs_major, ver_wcs_minor, ver_wcs_build, ver_lpu_major, ver_lpu_minor, ver_lpu_build);
    }
  } 

  pci_set_drvdata(pdev, pData);

 #ifdef HW_INTERRUPTS_WAITQUEUE
  // default, enable wait queue for all boards
  pData->status |= STATUS_HWINT_WQ;
 #endif

 #ifndef NO_SYSFS_SUPPORT  
  if((status=sysfs_create_group(&pdev->dev.kobj, &cei_1553_attr_group)) != 0)
    printk(KERN_ERR "uceipci(%d):  failed to create sysfs group, status %d\n", pData->minor, status);
 #endif

  return status;
}


static int uninitialize_1553(struct pci_dev* pdev) {
 #ifndef NO_SYSFS_SUPPORT  
  sysfs_remove_group(&pdev->dev.kobj, &cei_1553_attr_group);
 #endif

  return 0;
}

#ifndef NO_HW_INTERRUPTS
static int intrpt_cntrl_1553(DEV_DATA* pData, int mode) {
  int i, status=0;
  unsigned short wVal=0;
  int device = ucei_pdev[pData->minor]->device;

  if(device == 0x1542) {
    /* control the ARINC channel for hardware interrupts */
    writel(0x40000010, (unsigned int*)(pData->hwr_chan_addr[MP_ARINC_CHANNEL]));
    if(mode == 0)  /* disable */
      writel(0x40000020, (unsigned int*)(pData->hwr_chan_addr[MP_ARINC_CHANNEL]));
    else if(mode == 1)   /* enable */
      writel(0x80000020, (unsigned int*)(pData->hwr_chan_addr[MP_ARINC_CHANNEL]));
    else
      return -EINVAL;
  }
  
  switch(device) {
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
    // control each available 1553 channel for hardware interrupts
    for(i=0;i<pData->ch;i++) {
      if(pData->uca32) {
        // clear the interrupt, just in case
        writel(0x40000200, (unsigned int*)(pData->hwr_chan_addr[i]));
        if(mode == 0)  // disable
          writel(0x40004000, (unsigned int*)(pData->hwr_chan_addr[i]));
        else if(mode == 1)  // enable
          writel(0x80004000, (unsigned int*)(pData->hwr_chan_addr[i]));
      }
      else {
        wVal = readw((unsigned short*)pData->hwr_chan_addr[i]);
        writew(0, (unsigned short*)(pData->hwr_chan_addr[i] + 0x16));  // clear register
        if(mode == 0) // disable
          writew(wVal & ~0x4000, (unsigned short*)(pData->hwr_chan_addr[i]));
        else if(mode == 1)  // enable
          writew(wVal | 0x4000, (unsigned short*)(pData->hwr_chan_addr[i]));
      }
    }
    break;
  case 0x1003:
    // disable hardware interrupts on all channels (Interrupt Mask Register)
    writeb(0, (unsigned short*)(pData->pci_bar_laddr_membase + CORE_GLOBAL_REG_OFFSET + 0x14));
    // clear hardware interrupts on all channels
    for(i=0;i<pData->ch;i++) 
      writeb(0, (unsigned short*)(pData->pci_bar_laddr_membase + (CORE_CHAN_REG_OFFSET * i) + 0x4));
    break;
  default:
    return -EINVAL;
  };

  if(mode == 0) {
    free_irq(pData->irq, (void*)pData);  // returns when all executing interrupts are done
    pData->status &= ~STATUS_INT_ENABLE;
    if(uceipci_debug >= 2)
      printk(KERN_INFO "uceipci(%d):  unregistered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->irq, (unsigned long) pData);
   #ifdef HW_INTERRUPTS_WAITQUEUE
    if(pData->status & STATUS_HWINT_WQ) {
      // 1553 channels
      for(i=0;i<pData->ch;i++) {
        pData->cur_intrpt[i]++;
        wake_up_interruptible(&(pData->isr_wait_q[i]));  // wake the user process (1553) held in uceipci_read
      }
      if(pData->id == 0x360) {
        // ARINC channel
        pData->cur_intrpt[MP_ARINC_CHANNEL]++;
        wake_up_interruptible(&(pData->isr_wait_q[MP_ARINC_CHANNEL]));  // wake the user process (ARINC) held in uceipci_read
      }
    }
   #endif
   #ifdef HW_INTERRUPTS_SIGNAL
    if(pData->status & STATUS_HWINT_SIGNAL) {
      pData->status &= ~STATUS_HWINT_SIGNAL;
     #ifdef HW_INTERRUPTS_WAITQUEUE
      pData->status |= STATUS_HWINT_WQ;  // re-enable wait queue
     #endif
    }
   #endif
   #ifdef HW_MSI_INTERRUPTS
    if(pData->msi_mode == 1) {
      pci_clear_master(ucei_pdev[pData->minor]);
      // must be called after "free_irq" or will result in BUG_ON()
      pci_disable_msi(ucei_pdev[pData->minor]);
    }
   #endif
  }
  else if(mode == 1) {
   #ifdef HW_MSI_INTERRUPTS
    if(pData->msi_mode == 1) {
      if(pci_enable_msi(ucei_pdev[pData->minor]) != 0) {
        printk(KERN_ERR "uceipci(%d):  unable to enable MSI.\n", pData->minor);
        return -1;
      }
      // in the PCI config space for "COMMAND REGISTER" the "bus master" bit must be set when using MSI or DMA.
      pci_set_master(ucei_pdev[pData->minor]);

      // read the "Command" register (04h) in PCI config space.
      if((status = pci_read_config_word(ucei_pdev[pData->minor], (int)2, &wVal)) != 0)
        printk(KERN_ERR "uceipci(%d):  failed to read Command register in PCI config (word)\n", pData->minor);
      else {
        // check that the "Interrupt Disable" bit is set
        if((wVal & 0x0400) == 0) {
          printk("uceipci(%d):  Interrupt Disable not set\n", pData->minor);
          status = -1;
        }
        // check that the "Bus Master" bit is set
        if((wVal & 0x0002) == 0) {
          printk("uceipci(%d):  Bus Master not set\n", pData->minor);
          status = -1;
        }
      }
      if(status != 0) {
        pci_disable_msi(ucei_pdev[pData->minor]);
        pci_clear_master(ucei_pdev[pData->minor]);
        return -1;
      }

      // need to check for the assigned irq after enabling MSI
      pData->irq = ucei_pdev[pData->minor]->irq;

      printk(KERN_ERR "uceipci(%d):  enabled MSI.\n",pData->minor);
    }
    else
     printk(KERN_ERR "uceipci(%d):  board does not support MSI, using legacy interrupts.\n", pData->minor);
   #endif

   #ifdef HW_INTERRUPTS_WAITQUEUE
    if(pData->status & STATUS_HWINT_WQ) {
      // 1553 channels
      for(i=0;i<pData->ch;i++)
        pData->last_intrpt[i] = pData->cur_intrpt[i] = 0;
      if(pData->id == 0x360) {
        // ARINC channel
        pData->last_intrpt[MP_ARINC_CHANNEL] = pData->cur_intrpt[MP_ARINC_CHANNEL] = 0;
      }
    }
   #endif
    if((status = request_irq(pData->irq, uceipci_1553_isr, IRQF_SHARED, DRIVER_NAME, (void*)pData)) != 0) {
      printk(KERN_ERR "uceipci(%d):  unable to register interrupt handler IRQ (%d), status (%d).\n",pData->minor, pData->irq, status);
      return -1;
    }
    else {
      if(uceipci_debug >= 2)
        printk(KERN_INFO "uceipci(%d):  registered interrupt handler IRQ (%d), dev_id (0x%lx).\n",pData->minor, pData->irq, (unsigned long) pData);
    }
    pData->status |= STATUS_INT_ENABLE;
  }

  return 0;
}
#endif

#ifndef NO_HW_INTERRUPTS 
// Handle the hardware interrupts generated by the board
// Notes:
//  1. when an interrupt is detected on any channel, all IDs attached to the
//      hardware interrupt will receive a signal. So the user application will
//      need to determine which channel to check for
//  2. if CONFIG_DEBUG_SHIRQ defined and IRQF_SHARED flag is set, then the ISR
//      will be called (no PCI assertion) when requesting and freeing the 
//      interrupt line. 
static irqreturn_t uceipci_1553_isr(int irq, void* data) {
   int i, int_count=0;
   unsigned short cntrl_reg=0;
   unsigned int int_status_reg=0;
   DEV_DATA* pData=(DEV_DATA*)data;
  #ifdef HW_INTERRUPTS_SIGNAL
   int indx_mask, indx_id=0;
   ID_DATA* pId;
  #if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,20,0))
   struct kernel_siginfo siginfo;
  #else
   struct siginfo siginfo;
  #endif
   struct task_struct* ts;
  #endif

   if(pData == NULL) {
     if(uceipci_debug >= 2)
       printk(KERN_ERR "uceipci:  uceipci_1553_isr - no data pointer, IRQ(%d)\n", irq);
     return IRQ_NONE;
   }

   if(uceipci_debug >= 2)
     printk(KERN_INFO "uceipci(%d): uceipci_1553_isr\n", pData->minor);

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

  #ifdef HW_INTERRUPTS_SIGNAL
   siginfo.si_code = SI_QUEUE;
   siginfo.si_errno = 0;
   siginfo.si_signo = pData->signal;  // set by the user
  #endif
     
   if(pData->id == 0x1003) {  /* EPMC-1553 */
     // read the Interrupt Status Register (global register) instead of each channel register
     cntrl_reg = readw((unsigned short*)(pData->pci_bar_laddr_membase + CORE_GLOBAL_REG_OFFSET + 0x12));
     for(i=0;i<pData->ch;i++) {
       // check for interrupt per channel
       if((cntrl_reg >> i) & 0x1) {
         // clear the interrupt in the channel register
         writew(0, (unsigned short*)(pData->pci_bar_laddr_membase + (CORE_CHAN_REG_OFFSET * i) + 0x4));
         if(uceipci_debug >= 3)
           printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), chan (%d), cntrl_reg (0x%x).\n", pData->minor, pData->irq, i, cntrl_reg);
         int_count++;
        #ifdef HW_INTERRUPTS_SIGNAL
         // check if signals have been enabled
	       if((pData->status & STATUS_HWINT_SIGNAL) == 0)
           continue;
         indx_mask = 1;
         indx_id = 0;
         // checks for IDs marked to receive signal for hardware interrupts
         while(indx_mask <= pData->ids_mask_intrpt) {
           pId = &(pData->ids[indx_id]);
           if(pId->sigval == -1)  // send the devnum (device and channel)
             siginfo.si_int = (pData->minor*8) + i;
           else  // user provided
             siginfo.si_int = pId->sigval;
           // check if the ID is marked to receive signals and has a valid PID
           if((indx_mask & pData->ids_mask_intrpt) && (pId->sigpid > 1)) {
             ts = get_pid_task(find_vpid(pId->sigpid), PIDTYPE_PID);
             send_sig_info(pData->signal, &siginfo, ts);
             if(uceipci_debug >= 2)
               printk(KERN_INFO "uceipci(%d):  signal(%d) to PID(%d) with val(%d)\n", pData->minor, siginfo.si_signo, pId->sigpid, siginfo.si_int);
           }
           indx_mask = (1<<++indx_id);
         };
        #endif
       }
     }
   }
   else {
     if(pData->id == 0x360) {
       /* for the ARINC channel on the RAR15-XMC-XT */
       /* read the global "Interrupt Status" register */
       int_status_reg = readl((unsigned int*)(pData->pci_bar_laddr_membase + 0x84));
       if(int_status_reg & 0x100) {
         writel(0x40000010, (unsigned int*)(pData->hwr_chan_addr[MP_ARINC_CHANNEL]));
         int_count++;
         if(uceipci_debug >= 3)
           printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), chan (%d), isr_reg (0x%x).\n", pData->minor, pData->irq, MP_ARINC_CHANNEL, int_status_reg);
        #ifdef HW_INTERRUPTS_WAITQUEUE
         if(pData->status & STATUS_HWINT_WQ) {
           pData->cur_intrpt[MP_ARINC_CHANNEL]++;
           wake_up_interruptible(&(pData->isr_wait_q[MP_ARINC_CHANNEL]));  // wake any process/thread waiting to be notified that a hardware interrupt occured
           if(uceipci_debug >= 2)
             printk(KERN_INFO "uceipci(%d):  wake wait queue (%d) - count (%d)\n", pData->minor, MP_ARINC_CHANNEL, pData->cur_intrpt[MP_ARINC_CHANNEL]);
         }
        #endif
       }
     }
     /*  for the 1553 channels on UCA */
     for(i=0;i<pData->ch;i++) {
       if(pData->uca32) {
         // read the global "Interrupt Status" register
         int_status_reg = readl((unsigned int*)(pData->pci_bar_laddr_membase + 0x84));
         // check for interrupt on channel
         if(((int_status_reg >> i) & 0x1) == 0)
           continue;
         writel(0x40000200, (unsigned int*)(pData->hwr_chan_addr[i]));
         if(uceipci_debug >= 3)
           printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), chan (%d), isr_reg (0x%x).\n", pData->minor, pData->irq, i, int_status_reg);
       }
       else {
         // read the 1553_control_register on channel
         cntrl_reg = readw((unsigned short*)pData->hwr_chan_addr[i]);
         // check for interrupt
         if((cntrl_reg & 0x200) == 0)
           continue;
         // write a 0 to the write_interrupt_bit
         writew(0, (unsigned short*)(pData->hwr_chan_addr[i] + 0x16));
         if(uceipci_debug >= 3)
           printk(KERN_INFO "uceipci(%d):  valid interrupt - IRQ (%d), chan (%d), cntrl_reg (0x%x).\n", pData->minor, pData->irq, i, cntrl_reg);
       }
       int_count++;
      #ifdef HW_INTERRUPTS_WAITQUEUE
       if(pData->status & STATUS_HWINT_WQ) {
         pData->cur_intrpt[i]++;
         wake_up_interruptible(&(pData->isr_wait_q[i]));  // wake any process/thread waiting to be notified that a hardware interrupt occured
         if(uceipci_debug >= 2)
           printk(KERN_INFO "uceipci(%d):  wake wait queue (%d) - count (%d)\n", pData->minor, i, pData->cur_intrpt[i]);
         continue;
       }
      #endif
      #ifdef HW_INTERRUPTS_SIGNAL
       if((pData->status & STATUS_HWINT_SIGNAL) == 0)
         continue;
       indx_mask = 1;
       indx_id = 0;
       // checks for IDs marked to receive signal for hardware interrupts
       while(indx_mask <= pData->ids_mask_intrpt) {
         pId = &(pData->ids[indx_id]);
         if(pId->sigval == -1)
           siginfo.si_int = pData->minor;  // send the device number
         else
           siginfo.si_int = pId->sigval;  // user provided
         // check if the ID is marked to receive signals and has a valid PID
         if((indx_mask & pData->ids_mask_intrpt) && (pId->sigpid > 1)) {
           ts = get_pid_task(find_vpid(pId->sigpid), PIDTYPE_PID);
           if(!ts) {
             if(uceipci_debug >= 3)
               printk(KERN_ERR "uceipci(%d):  no PID(%d)\n", pData->minor, pId->sigpid);
             continue;
           }
           if(send_sig_info(pData->signal, &siginfo, ts) != 0) {
             if(uceipci_debug >= 2)
               printk(KERN_ERR "uceipci(%d):  failed to send signal(%d) to PID(%d) with val(%d)\n", pData->minor, siginfo.si_signo, pId->sigpid, siginfo.si_int);
           }
           else {
             if(uceipci_debug >= 2)
               printk(KERN_INFO "uceipci(%d):  signal(%d) to PID(%d) with val(%d)\n", pData->minor, siginfo.si_signo, pId->sigpid, siginfo.si_int);
           }
         }
         indx_mask = (1<<++indx_id);
       }
      #endif
     }
   }
 
   if(int_count == 0) {
     if(uceipci_debug >= 3) {
      #ifdef CONFIG_DEBUG_SHIRQ
       printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d), could be the kernel check for a valid ISR\n",pData->minor,irq,pData->irq);
      #else
       printk(KERN_ERR "uceipci(%d):  spurious interrupt - IRQ (%d,%d)\n",pData->minor,irq,pData->irq);
      #endif
     }
     return IRQ_NONE;
   }

   return IRQ_HANDLED;
}
#endif
