/*============================================================================*
 * FILE:                     B U I L D _ C O N F . C
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
 *===========================================================================*/

/* $Revision:  1.27 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  04/07/2005   Initial.
  09/05/2005   added build_conf_pcmcia, and removed use of /dev/mem in
                build_conf_isa. bch
  09/12/2005   added support for CEI-x30. bch
  02/20/2006   added support for CEI-500. bch
  09/13/2006   added support for AMC-1553 and AMC-A30. bch
  03/27/2007   added support for EPMC-1553 and QPCX-1553. bch
  06/07/2007   modified build_conf_pci to correctly identify devices. bch
  07/09/2007   added support for CEI-530. bch
  08/17/2007   added support for RCEI-830RX and EPMC-1553. bch
  12/12/2007   added support for the R15-EC. added Big Endian support. bch
  01/23/2008   added support for the RAR-CPCI, RAR-EC, P-708, P-SER, P-DIS,
                and P-MIO. bch
  11/18/2008   added support for P-708, removed swap32 macros. bch
  07/21/2009   added support for RXMC-1553. bch
  10/20/2009   added support for RPCIe-1553. bch
  02/12/2010   modified build_conf_pci to support 32bit in 64bit system. bch 
  04/14/2011   added support for RXMC2-1553 and R15-LPCIe. bch
  10/11/2011   added support for Linux kernel 3.x, added support for
                CEI-430A. bch
  10/15/2012   added support for RAR15-XMC, RAR15-XMC-XT and UCA32 firmware. bch
  04/24/2013   added support for CEI-830X820. bch
  11/19/2013   added support for CEI-830A. bch
  01/14/2014   added support for R15-PMC. added "csc" register. bch
  05/08/2014   added support for RP-708. bch
  05/14/2014   added support for R15-USB. bch
  09/29/2014   added support for RAR-USB. bch
  01/15/2015   added support for RAR-XMC. bch
  10/10/2016   added support for R15-MPCIe and RAR-MPCIe. bch
  08/18/2017   changed default for R15-USB to 2 channels. bch
  11/27/2018   modified argument usage. bch
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/version.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#ifdef INCLUDE_USB_SUPPORT
#include <libusb-1.0/libusb.h>
#endif


// NOTE:  set R15USB_CHANNEL_COUNT_DEFAULT to "2" for a two channel board and
//        set to "1" for a one channel board.
#define R15USB_CHANNEL_COUNT_DEFAULT  2

// NOTE:  set R15USB_MODE_DEFAULT to "1" for a multi-function board and set to
//        "0" for a single function board.
#define R15USB_MODE_DEFAULT           1


// ioctls
#define SET_REGION           0x13c61
#define GET_REGION_MEM       0x13c62
#define GET_REGION_SIZE      0x13c63
#define GET_DEVICE_ID        0x13c64

#ifdef SWAP
 #define swap16(val16) (((val16 & 0x00FF) << 8) | ((val16 & 0xFF00) >> 8))
#else
 #define swap16(val16) val16
#endif

static unsigned short QdevID[] = {0x110,0x120,0x220,0x170,0x180,0x200,0x210,0x110,0x230,0x240,0x260,0x250,0x160,0x300,0x320,0x3000,0x340,0x360,0x380,0x400};
char *qdevName[] = {"QPM(C)-1553", "IP-D1553", "QPCX-1553", "Q104-1553", "Q104-1553P", "PCCARD-D1553", "QCP-1553", "AMC-1553", "R15-EC", "R15-AMC", "RXMC-1553", "RPCIe-1553", "QPCI-1553", "RXMC2-1553", "R15-LPCIe", "R15-USB", "RAR15-XMC", "RAR15-XMC-XT", "R15-PMC","R15-MPCIe"};
unsigned short board_config[] = { // These are for the Q104-1553
                           0x4800,   // sf 1ch
                           0x8800,   // sf 2ch
                           0x0801,   // sf 4ch
                           0x4808,   // mf 1ch
                           0x8808,   // mf 2ch
                           0x0809,   // mf 4ch
                           0x4810,   // sf 1ch
                           0x8810,   // sf 2ch
                           0x0811,   // sf 4ch
                           0x4818,   // mf 1ch
                           0x8818,   // mf 2ch
                           0x0819,   // mf 4ch
                           0x0048,   // sf 1ch
                           0x0088,   // sf 2ch
                           0x0108,   // sf 4ch
                           0x0848,   // mf 1ch
                           0x0888,   // mf 2ch
                           0x0908,   // mf 4ch
                           0x1048,   // sf 1ch
                           0x1088,   // sf 2ch
                           0x1108,   // sf 4ch
                           0x1848,   // mf 1ch
                           0x1888,   // mf 2ch
                           0x1908};  // mf 4ch

unsigned devid_x20[] = {0x220,
                        0x420,
                        0x220,
                        0x420,
                        0x420,
                        0x420,
                        0x420,
                        0x420};

unsigned boardID_x20[] = {0x00,       // CEI-220
                          0x10,       // CEI-420
                          0x20,       // CEI-220 with ch 5=561 6-wire
                          0x30,       // CEI-420-70J with -717 reduced
                          0x40,       // CEI-420 with HBP xmit/Holt
                          0x50,       // CEI-420A 12 MHz, 16 in disc
                          0x60,       // CEI-420A with 16 MHz clock
               	          0x70};      // CEI-420A xxJ 16 MHz clock

//static unsigned short devID[] = {0x40,0x80,0xa0,0x100,0x430,0x430A,0x520,0x530,0x620,0x820,0x821,0x830,0x1009,0x831,0x630,0x100A};
static unsigned short devID[] = {0x40,0x80,0xa0,0x100};

FILE* fp_conf;
int dev_cnt=0;
int debug=0;  // set to 1 for debug output

// function prototypes
void write_to_conf(unsigned int* dev_info);
int build_conf_pci(void);
int build_conf_usb(void);
int build_conf_pcmcia(void);
int build_conf_isa(void);

int main(int argc, char** argv) {
   int build_type=0;
   char home[80]={""};
   char buf[80]={""};
   time_t tt;
   char *result = NULL;
   
   if(argc > 1) {
     if(strcmp("usb", argv[1]) == 0)
      build_type = 1;
     if(strcmp("pcmcia", argv[1]) == 0)
      build_type = 2;
     if(strcmp("isa", argv[1]) == 0)
      build_type = 3;
   }
  
   if(getcwd(buf, sizeof(buf)) == NULL) {
     printf("Failed to get directory path.\n");
     return -1;
   }
   result = strtok(buf, "/");
   while((result != NULL) && (strcmp(result, "Condor_Engineering")!=0)) {
     sprintf(buf,"/%s",result);
     strcat(home,buf);
     result = strtok(NULL, "/");
   }
   sprintf(buf,"%s%s",home,"/Condor_Engineering/ceidev.conf");

   if((fp_conf = fopen(buf,"w")) == NULL) {
     printf("Can't open ceidev.conf\n");
     return -1;
   }

   sprintf(buf,"%s","# Configuration File for Condor Engineering Boards\n");
   fwrite(buf,strlen(buf),1,fp_conf);
   time(&tt);
   sprintf(buf,"# Created on %s\n", ctime(&tt));
   fwrite(buf,strlen(buf),1,fp_conf);
   fwrite("#\n",2,1,fp_conf);

   switch(build_type) {
   case 0:
     if(build_conf_pci() != 0)
       printf("Failed to build PCI section of conf file.\n");
     break;
   case 1:
    #ifdef INCLUDE_USB_SUPPORT
     if(build_conf_usb() != 0)
       printf("Failed to build USB section of conf file.\n");
    #else
       printf("Not configured with USB support.\n");
    #endif
     break;
   case 2:
     if(build_conf_pcmcia() != 0) 
       printf("Failed to build PCMCIA section of conf file.\n");
     break;
   case 3:
     if(build_conf_isa() != 0) 
       printf("Failed to build ISA section of conf file.\n");
     break;
   };
   
   printf("Total Condor Device Installed = %d\n",dev_cnt);

   sprintf(buf,"devnum=%d\n",dev_cnt);
   fwrite(buf,strlen(buf),1,fp_conf);
   fclose(fp_conf);

   return 0;
}


int build_conf_pci(void) {
   int i, fd=0, btype=0, uca32=0, minor_cnt=0, region=0;
   char dev_path[80], buf[80];
   char* lpbase;
   unsigned int device=0, bar_region_size=0;
   unsigned int data=0;
   unsigned short host_interface=0, nchan=0, dev_id=0, mode=0, irig=0;
   unsigned int dev_info[8];  // bus, dev, minor, channel, id, mode, irig, csc
  #if ((defined KERNEL_26) || (defined KERNEL_30))
  #ifdef _32BIT
   // need if running a 32-bit app on a 64-bit OS
   unsigned long long pci_bar_regions[6][3];
  #else
   unsigned long pci_bar_regions[6][3];
  #endif
  #endif
   
   // probes for a maximum of 8 PCI devices
   for(minor_cnt=0;minor_cnt<8;minor_cnt++) {
      sprintf(dev_path,"/dev/uceipci_%d",minor_cnt);
      if((fd = open(dev_path,O_RDWR)) == -1) {
        if(debug) {
          if(minor_cnt == 0)
            printf("PCI device driver not loaded.  Ignore if no Condor PCI devices installed.\n");  		
          else
            printf("No more Condor Engineering PCI devices (total: %d)\n", minor_cnt);
        }
        return 0;
      }

      if(debug)
        printf("Found Condor Engineering PCI Device -- Device Number = %d  Minor = %d\n",dev_cnt,minor_cnt);

     #if ((defined KERNEL_26) || (defined KERNEL_30)) 
      device = ((0x1 << 30) + 1);  // read "Device ID" from board's PCI config
      if((read(fd,&device,2)) != -1) {
        if(debug)
          printf("Device %d is 0x%x\n",minor_cnt,device);
      }
      else {
        printf("Failed read for device %d.\n", minor_cnt);
        continue;
        return -1;
      }
     #else
      if(ioctl(fd,GET_DEVICE_ID,&device)== 0) {
        if(debug)
          printf("Device %d is 0x%x\n",minor_cnt,(unsigned short)device);
      }
      else {
        printf("Failed ioctl for device %d.\n", minor_cnt);
        continue;
      }
     #endif

      dev_id = device;

      switch(device) {
      //  CEI-500 carriers (IPAV)
      case 0x0040: // SBS PCI-40A
      case 0x1024: // Acromag APC8620
        printf("Installing CEI-500 (0x%x) as device %d\n", device, dev_cnt);
        sprintf(buf,"# Installing CEI-500 (0x%x) as device %d\n", device,dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        dev_id = 0x500;
        break;	
      // P family
      case 0x708:
        printf("Installing P-708 as device %d\n", dev_cnt);
        sprintf(buf,"# Installing P-708 as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x708A:
        printf("Installing RP-708 as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RP-708 as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x1004:
        printf("Installing P-SER as device %d\n", dev_cnt);
        sprintf(buf,"# Installing P-SER as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x1005:
        printf("Installing P-MIO as device %d\n", dev_cnt);
        sprintf(buf,"# Installing P-MIO as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x1006:
        printf("Installing P-DIS as device %d\n", dev_cnt);
        sprintf(buf,"# Installing P-DIS as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      //  CEI-x30 device
      case 0x630:
        printf("Installing RAR-CPCI as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RAR-CPCI as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x831:
        printf("Installing R830RX as device %d\n", dev_cnt);
        sprintf(buf,"# Installing R830RX as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x832:
        printf("Installing CEI-830X820 as device %d\n", dev_cnt);
        sprintf(buf,"# Installing CEI-830X820 as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x1009:
        printf("Installing AMC-A30 as device %d\n", dev_cnt);
        sprintf(buf,"# Installing AMC-A30 as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
//        dev_id = 0xA30;
        break;
      case 0x100A:
        printf("Installing RAR-EC as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RAR-EC as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x100B:
        printf("Installing RAR-PCIe as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RAR-PCIe as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x100C:
        printf("Installing RAR-XMC as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RAR-XMC as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x100D:
        printf("Installing RAR-MPCIe as device %d\n", dev_cnt);
        sprintf(buf,"# Installing RAR-MPCIe as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x430:
      case 0x430A:
      case 0x530:
      case 0x830:
      case 0x830A:
      //  CEI-x20 device
      case 0x520:
      case 0x620:
      case 0x820:
        printf("Installing CEI-%x as device %d\n",(unsigned short)device,dev_cnt);
        sprintf(buf,"# Installing CEI-%x as device %d\n",(unsigned short)device,dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      case 0x821:
        printf("Installing CEI-820TX as device %d\n", dev_cnt);
        sprintf(buf,"# Installing CEI-820TX as device %d\n", dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);
        break;
      //  MIL-STD-1553 device
      case 0x1008:  // AMC-1553
      case 0x1553:  // PCI-1553, cPCI-1553, PMC-1553, QPMC-1553, QPM-1553, and Q104-1553P
      case 0x1554:  // QPCI-1553
      case 0x1555:  // QCP-1553
      case 0x1556:  // QPCX-1553
      case 0x1557:  // R15-EC
      case 0x1559:  // RXMC-1553
      case 0x155A:  // RPCIe-1553		
      case 0x155B:  // R15-LPCIe
      case 0x155C:  // RXMC2-1553
      case 0x1530:  // RAR15-XMC
      case 0x1542:  // RAR15-XMC-XT
      case 0x1544:  // R15-PMC
      case 0x1545:  // R15-MPCIe
        if((device == 0x1554) || (device == 0x1555) || (device == 0x1556)) 
          region = 2;
        else
          region = 0;
       #if ((defined KERNEL_26) || (defined KERNEL_30))
        pci_bar_regions[0][0] = ((0x2 << 30) + 1);  // read device attribute
        if(read(fd, &pci_bar_regions, sizeof(pci_bar_regions)) != 0) {
          printf("Failed to read driver, errno: %d\n", errno);
          close(fd);
          continue;
        }
        // need to set the mmap index in the driver to the necessary BAR region      
        for(i=0;i<region;i++) {
          if(pci_bar_regions[i][0] > 0) {
            if((lpbase=(char*)mmap(NULL,pci_bar_regions[i][1],PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))!=(char*)-1)
             munmap(lpbase,pci_bar_regions[i][1]);
          }
        }
        bar_region_size = pci_bar_regions[region][1];
       #else
        ioctl(fd,SET_REGION,&region);
        ioctl(fd,GET_REGION_SIZE,&bar_region_size);
       #endif

        if(bar_region_size == 0) {
          close(fd);
          printf("Invalid memory size 0x%X for BAR region %d\n", bar_region_size, region);
          return -1;
        }

        if(debug)	
          printf("BAR region %d, size 0x%X\n", region, bar_region_size);

        lpbase = (char*)mmap(NULL,bar_region_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        if(lpbase == (char*)-1) {
          close(fd);
          printf("Failed to map PCI device.\n");
          return -1;
      	}	
        host_interface = swap16(*((unsigned short*)lpbase));
        munmap(lpbase,bar_region_size);
	
        if(debug)	
          printf("PCI host interface for device %d = 0x%x\n",dev_cnt,host_interface);

        if((device == 0x1553) && (bar_region_size >= 0x2000000)) {
          printf("Installing PCI-1553, cPCI-1553 or PMC-1553 Board as device %d\n",dev_cnt);
          sprintf(buf,"# Installing PCI-1553 or PMC-1553 as device %d\n",dev_cnt);
          fwrite(buf,strlen(buf),1,fp_conf);
          btype = (host_interface & 0xc000)>>14;
          nchan = (host_interface & 0x3000)>>12;
          if(nchan > 1)
            nchan = 2;
          dev_id = devID[btype];
          if(host_interface & 0x300)
            mode = 1;
          else
            mode = 0;
          irig = 0;
	        if(debug)
            printf("btype = %d nchan = %d\n",btype,nchan);
        }
        else {
          if(device == 0x1554)
            btype = 12;
          else
            btype = ((host_interface & 0x3e) >> 1) - 1;
          uca32 = (host_interface & 0x4000) >> 14;
          printf("Installing %s as device %d\n",qdevName[btype],dev_cnt);
          sprintf(buf,"# Installing %s as device %d\n",qdevName[btype],dev_cnt);
          fwrite(buf,strlen(buf),1,fp_conf);
          if(uca32)
            nchan = (host_interface & 0x3E0) >> 6;
          else
            nchan = (host_interface & 0x7c0) >> 6;
          dev_id = QdevID[btype];
          if(host_interface & 0x800)
            mode = 1;
          else
            mode = 0;
          if(host_interface & 0x1000)
            irig = 1;
          else
            irig = 0;
        }
        break;
      //  CORE device
      case 0x1003:
       #if ((defined KERNEL_26) || (defined KERNEL_30))
        data = 0x200000;  // read HWID register
        if(read(fd, &data, 2) != 0) {
          printf("Failed to read driver, errno: %d\n", errno);
          close(fd);
          return -1;
        }
        if(debug)	
          printf("HWID register for device %d = 0x%x\n",dev_cnt,data);
        mode = (data >> 12) & 0x1;  // multi-function RT
        data = 0x200004;  // read HCAP register
        if(read(fd, &data, 2) != 0) {
          printf("Failed to read driver, errno: %d\n", errno);
          close(fd);
          return -1;
        }
       #else
        region = 2;
        ioctl(fd,SET_REGION,&region);
        ioctl(fd,GET_REGION_SIZE,&bar_region_size);
        if(bar_region_size == 0) {
          close(fd);
          printf("Invalid memory size 0x%X for BAR region %d\n", bar_region_size, region);
          return -1;
        }
        if(debug)	
          printf("BAR region %d, size 0x%X\n", region, bar_region_size);
        lpbase = (char*)mmap(NULL,bar_region_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        if(lpbase == (char*)-1) {
          close(fd);
          printf("Failed to map EPMC-1553 device %d.\n", dev_cnt);
          return -1;
        }	
        data = *((unsigned short*)(lpbase+0x200000));
        if(debug)	
          printf("HWID register for device %d = 0x%x\n",dev_cnt,data);
        mode = (data >> 12) & 0x1;  // multi-function RT
        data = *((unsigned short*)(lpbase+0x200004));
        munmap(lpbase,bar_region_size);
       #endif
        if(debug)	
          printf("HCAP register for device %d = 0x%x\n",dev_cnt,data);
        for(i=0;i<8;i++) 
          if(data & (0x1<<i)) nchan++;
        irig = (data >> 9) & 0x1;
        printf("Installing EPMC-1553 as device %d\n",dev_cnt);
        sprintf(buf,"# Installing EPMC-1553 as device %d\n",dev_cnt);
        fwrite(buf,strlen(buf),1,fp_conf);	
        dev_id = device;
        break;
      default:
        printf("Invalid Condor_Engineering device (0%x).\n", device); 
      };

      close(fd);
  
      dev_info[0] = 2;
      dev_info[1] = dev_cnt++;
      dev_info[2] = minor_cnt;
      dev_info[3] = nchan;
      dev_info[4] = dev_id;
      dev_info[5] = mode;
      dev_info[6] = irig;
      dev_info[7] = host_interface;
      write_to_conf(dev_info);
   }  // end minors 

   return 0;
}


int build_conf_isa(void) {
   int fd;
   char buf[80]="";
   char* lpbase;
   int minor_cnt=0, base_addr=0;
   unsigned short host_interface=0, nchan=0, dev_id=0, irig=0, mode=0;
   int jj;
   unsigned int dev_info[8];  // bus, dev, minor, channel, id, mode, irig, csc

   // probes for a maximum of 4 ISA devices
   for(minor_cnt=0;minor_cnt<4;minor_cnt++) {
     sprintf(buf, "/dev/uceiisa_%d", minor_cnt);
     if((fd = open(buf,O_RDWR)) == -1) {
       if(debug) {
         if(minor_cnt == 0)
           printf("ISA device driver not loaded.  Ignore if no Condor ISA devices installed.\n");  		
         else
           printf("No more Condor Engineering ISA devices (total: %d)\n", minor_cnt);
       }
       return 0;
     }

     lpbase = (char*)mmap(NULL,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
     if(lpbase == (char*)-1) {
       printf("Failed to map ISA device\n");
       close(fd);
       return -1;
     }

     ioctl(fd,GET_REGION_MEM,(int*)&base_addr);

     host_interface = swap16(*((unsigned short *)lpbase));
     if(debug)
       printf("ISA Host interface at address (0x%X): 0x%X\n", base_addr, host_interface);
     for(jj=0;jj<24;jj++) {
       if(host_interface == board_config[jj])
         break;
     }

     if(jj<24) { // Q104-1553
       if(debug)	     
         printf("Q104-1553 found at base address 0x%X\n",base_addr);
       printf("Installing Q104-1553 as device %d\n",dev_cnt);
       sprintf(buf,"# Installing Q104-1553 as device %d\n",dev_cnt);
       fwrite(buf,strlen(buf),1,fp_conf);
       if(jj < 12 )  // this is a big endian machine so flip the host_interface
         host_interface = ((host_interface & 0xff00)>>8) | ((host_interface & 0x00ff)<<8);
       nchan = (host_interface & 0x7c0)>>6;
       dev_id = 0x170;
       if(host_interface & 0x800)
         mode = 1;
       else
         mode = 0;
       if(host_interface & 0x1000)
         irig = 1;
       else
         irig = 0;
     }
     else {       
       if((host_interface & 0xdc0f)==0x5c00) {  //  ISA-1553
         if(debug)	     
           printf("ISA-1553 found at base address 0x%X\n",base_addr);
         printf("Installing ISA-1553 as device %d\n",dev_cnt);
         sprintf(buf,"# Installing ISA-1553 as device %d\n",dev_cnt);
         fwrite(buf,strlen(buf),1,fp_conf);
         nchan = (host_interface & 0x3000)>>12;
         if(nchan > 1)
           nchan = 2;
         dev_id = 0x80;
         if(host_interface & 0x300)
           mode = 1;
         else
           mode = 0;
         irig = 0;
       }
       else { //  CEI-220/CEI-440
         host_interface = swap16(*(lpbase+0x80a));
         for(jj=0;jj<8;jj++) {
           if((host_interface==boardID_x20[jj]) && (*(lpbase+0x80c)==0) && ((*(lpbase+0x80e)&0xF0)==0)) 
             break;
         }
         if(jj==8) {
           printf("No Condor Engineering ISA Board Detected at base address 0x%X\n",base_addr);
           close(fd);
           return -1;
         }

         if(debug)	     
           printf("CEI-%x found at base address 0x%X\n",devid_x20[jj],base_addr);
         printf("Installing CEI-%x as device %d\n",devid_x20[jj],dev_cnt);
         sprintf(buf,"# Installing CEI-%x as device %d\n",devid_x20[jj],dev_cnt);
         fwrite(buf,strlen(buf),1,fp_conf);
         dev_id = devid_x20[jj];
         nchan = 0; 
         mode = 0;
         irig = 0;
       }
     }
    
     munmap(lpbase,0x1000);
      
     dev_info[0] = 1;
     dev_info[1] = dev_cnt++;
     dev_info[2] = minor_cnt;
     dev_info[3] = nchan;
     dev_info[4] = dev_id;
     dev_info[5] = mode;
     dev_info[6] = irig;
     dev_info[7] = host_interface;
     write_to_conf(dev_info);         
   } 

   close(fd);
	
   return 0;
}


int build_conf_pcmcia(void) {
   int fd, minor_cnt=0;
   unsigned int valD=0;
   char dev_path[80], buf[80], devName[15];
   char* lpbase;
   unsigned int region_size=0;
   unsigned short host_interface=0, nchan=0, dev_id=0, irig, mode=0;
   unsigned int dev_info[7];  // bus, dev, minor, channel, id, mode, irig
     
   // probes for a maximum of 2 PCMCIA devices
   for(minor_cnt=0;minor_cnt<2;minor_cnt++) {
      sprintf(dev_path,"/dev/pcc1553_%d",minor_cnt);
      if((fd = open(dev_path,O_RDWR)) == -1) {
        if(debug) {
          if(minor_cnt == 0)
            printf("PCMCIA client driver not loaded.  Ignore if no Condor PCMCIA devices installed.\n");  		
          else
            printf("No more Condor Engineering PCMCIA devices found (total: %d)\n", minor_cnt);
	      }
        return 0;
      }
      if(debug)
        printf("Found Condor Engineering PCMCIA Device -- Device Number = %d  Minor = %d\n",dev_cnt,minor_cnt);

      ioctl(fd,GET_REGION_SIZE,&region_size);
      
      lpbase = (char*)mmap(NULL,region_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
      if(lpbase == (char*)-1) {
        printf("Failed to map PCMCIA device.\n");
        close(fd);      
        return -1;
      } 	
      host_interface = swap16(*((unsigned short*)lpbase));	
      munmap(lpbase,region_size);
      close(fd);      

      if(host_interface == 0xffff) {
        printf("Invalid host interface value: 0x%x\n", host_interface);	      
        continue;
      }
      
      if(debug)	
        printf("PCMCIA host interface for device %d = 0x%x\n",dev_cnt,host_interface);

      if((valD = (host_interface & 0xE000) >> 13) != 4)  // PCCARD-1553
        valD = (host_interface & 0x003E) >> 1;  // PCCARD-D1553

      if(valD == 4) {
        dev_id = 0xA0;
        strcpy(devName, "PCCARD-1553");
        irig = 1;
        if(host_interface & 0x1000) { // CH 1 present
          nchan = 1;	
          if(host_interface & 0x100)  // CH 1 multiple mode
            mode = 1;
        }
      }
      else if (valD == 6) {
        dev_id = 0x200;
        strcpy(devName, "PCCARD-D1553");
        irig = (host_interface & 0x1000) >> 12;
        nchan = (host_interface & 0x3E0) >> 6;       
        mode = (host_interface & 0x800) >> 11;
      }
      else {
        printf("Invalid board type: %d.\n", (int)valD);
        continue;
      }
      
      printf("Installing %s as device %d\n", devName, dev_cnt);
      sprintf(buf,"# Installing %s as device %d\n", devName, dev_cnt);
      fwrite(buf, strlen(buf), 1, fp_conf);
      dev_info[0] = 4;
      dev_info[1] = dev_cnt++;
      dev_info[2] = minor_cnt;
      dev_info[3] = nchan;
      dev_info[4] = dev_id;
      dev_info[5] = mode;
      dev_info[6] = irig;
      dev_info[7] = host_interface;
      write_to_conf(dev_info);
   }  // end minors 

   return 0;
}


#ifdef INCLUDE_USB_SUPPORT
// probe all USB devices for R15-USB and RAR-USB
int build_conf_usb(void) {
  int i, cnt=0;
  libusb_device **devs;
  struct libusb_device_descriptor dev_desc;
  unsigned int dev_info[7];
  char buf[100];

  if(libusb_init(NULL) != LIBUSB_SUCCESS)
    return -1;

  if((cnt = libusb_get_device_list(NULL, &devs)) < 0) {
    printf("No USB devices found.\n");
    libusb_exit(NULL);
    return -1;
  }

  for(i=0;i<cnt;i++) {
    memset(&dev_desc, 0, sizeof(dev_desc));
    if(libusb_get_device_descriptor(devs[i], &dev_desc) != LIBUSB_SUCCESS)
      continue;
    if(dev_desc.idVendor != 0x21A8)
      continue;

    memset(&buf, 0, sizeof(buf));
    memset(&dev_info, 0, sizeof(dev_info));
    if((dev_desc.idProduct == 0x7502) || (dev_desc.idProduct == 0x7503)) {
      dev_info[1] = dev_cnt;
      dev_info[3] = R15USB_CHANNEL_COUNT_DEFAULT;
      dev_info[4] = 0x3000;
      dev_info[5] = R15USB_MODE_DEFAULT;
      sprintf(buf,"%s","# configured for a R15-USB");
      if(dev_info[5] == 1)
        sprintf(buf, "%s %s", buf, "multi-function");
      else
        sprintf(buf, "%s %s", buf, "single-function");
      if(dev_info[3] == 2)
        sprintf(buf, "%s %s", buf, "two channel\n");
      else
        sprintf(buf, "%s %s", buf, "one channel\n");
      printf("Installing %s as device %d\n", "R15-USB", dev_cnt);
    }
    else if((dev_desc.idProduct == 0x7504) || (dev_desc.idProduct == 0x7505)) {
      dev_info[1] = dev_cnt;
      dev_info[4] = 0x7505;
      sprintf(buf,"%s","# configured for a RAR-USB\n");
      printf("Installing %s as device %d\n", "RAR-USB", dev_cnt);
    }
    else
      continue;

    dev_cnt++;
    fwrite(buf, strlen(buf), 1, fp_conf);
    dev_info[0] = 5;
    dev_info[2] = (libusb_get_bus_number(devs[i]) << 16) | libusb_get_device_address(devs[i]); //  use the USB bus number and USB device number in place of the minor number
    write_to_conf(dev_info);
  }

  libusb_free_device_list(devs, 1);
  libusb_exit(NULL);

  return 0;
}
#endif

// bus, dev, minor, channel, id, mode, irig
void write_to_conf(unsigned int* dev_info) {
   char buf[80];

   sprintf(buf,"id%d=%d\n",dev_info[1],dev_info[4]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"ch%d=%d\n",dev_info[1],dev_info[3]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"func%d=%d\n",dev_info[1],dev_info[5]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"bus%d=%d\n",dev_info[1],dev_info[0]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"minor%d=%d\n",dev_info[1],dev_info[2]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"irig%d=%d\n",dev_info[1],dev_info[6]);
   fwrite(buf,strlen(buf),1,fp_conf);
   sprintf(buf,"csc%d=%d\n",dev_info[1],dev_info[7]);
   fwrite(buf,strlen(buf),1,fp_conf);
   if(dev_info[0] == 1)
      sprintf(buf,"drv%d=uceiisa_%d\n",dev_info[1],dev_info[2]);
   else if(dev_info[0] == 2)
      sprintf(buf,"drv%d=uceipci_%d\n",dev_info[1],dev_info[2]);
   else if(dev_info[0] == 4)
      sprintf(buf,"drv%d=pcc1553_%d\n",dev_info[1],dev_info[2]);
   else if(dev_info[0] == 5)
      sprintf(buf,"drv%d=libusb\n",dev_info[1]);
   fwrite(buf,strlen(buf),1,fp_conf);
   fwrite("#\n",2,1,fp_conf);
}

