// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 *                            == DISCLAIMER ==                               *
 *                                                                           *
 *    The source code enclosed has been included as an aid in the            *
 *    development of your application, and while believed to be accurate and *
 *    fully functional code, is in NO WAY to be held to the standard of      *
 *    normal supported and maintained source code that has been stringently  *
 *    tested and debugged for the purposes currently offered.                *
 *    The attached source code is offered "AS IS" and as such will not be    *
 *    supported by its author or any other employee of Concurrent Computer   *
 *    Corporation.                                                           *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *  Copyright (C) 2002 and beyond Concurrent Computer Corporation            *
 *  All rights reserved                                                      *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_reg.c                                               *
 *                                                                           *
 * Description:  Display Registers Test                                      *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_reg          ==> start display registers test on device 0       *
 *   gsc16ao_reg <dn>     ==> start display registers test on device <dn>.   *
 *                            (where dn range is 0 - 9)                      *
 *                                                                           *
 * Date:        06/25/2007                                                   *
 * History:                                                                  *
 *                                                                           *
 *   4 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   3 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   2 12/07/05 D. Dubash                                                    *
 *              Cleanup warnings. Added EEPROM REV register display          *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Adapted from lcaio_reg.c                                     *
 *                                                                           *
 *****************************************************************************/
/*
 * Headers
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#include "gsc16ao_ioctl.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

typedef struct _what {
	int	 index;
	char	*name;
} what;


what pci_config_reg[]= {
	{ GSC16AO_PCI_IDR,			"GSC16AO_PCI_IDR               " },
	{ GSC16AO_PCI_CR_SR,			"GSC16AO_PCI_CR_SR             " },
	{ GSC16AO_PCI_REV_CCR,			"GSC16AO_PCI_REV_CCR           " },
	{ GSC16AO_PCI_CLSR_LTR_HTR_BISTR,		"GSC16AO_PCI_CLSR_LTR_HTR_BISTR" },
	{ GSC16AO_PCI_BAR0,			"GSC16AO_PCI_BAR0              " },
	{ GSC16AO_PCI_BAR1,			"GSC16AO_PCI_BAR1              " },
	{ GSC16AO_PCI_BAR2,			"GSC16AO_PCI_BAR2              " },
	{ GSC16AO_PCI_BAR3,			"GSC16AO_PCI_BAR3              " },
	{ GSC16AO_PCI_BAR4,			"GSC16AO_PCI_BAR4              " },
	{ GSC16AO_PCI_BAR5,			"GSC16AO_PCI_BAR5              " },
	{ GSC16AO_PCI_CIS,			"GSC16AO_PCI_CIS               " },
	{ GSC16AO_PCI_SVID_SID,			"GSC16AO_PCI_SVID_SID          " },
	{ GSC16AO_PCI_ERBAR,			"GSC16AO_PCI_ERBAR             " },
	{ GSC16AO_PCI_ILR_IPR_MGR_MLR,		"GSC16AO_PCI_ILR_IPR_MGR_MLR   " },
	{ 0,	0, }
};

what	gsc_reg[]= {
	{ GSC16AO_GSC_BCR,   "GSC16AO_GSC_BCR   " },
	{ GSC16AO_GSC_CSR,   "GSC16AO_GSC_CSR   " },
	{ GSC16AO_GSC_SRR,   "GSC16AO_GSC_SRR   " },
	{ GSC16AO_GSC_BOR,   "GSC16AO_GSC_BOR   " },
	{ GSC16AO_GSC_REV,   "GSC16AO_GSC_REV   " },
	{ GSC16AO_GSC_ODBR,  "GSC16AO_GSC_ODBR  " },
	{ GSC16AO_GSC_ACLK,  "GSC16AO_GSC_ACLK  " },
	{ 0,	0, }
};

what	plx_local_config_reg[]= {
	{ GSC16AO_PLX_LASORR,"GSC16AO_PLX_LASORR" },
	{ GSC16AO_PLX_LAS0BA,"GSC16AO_PLX_LAS0BA" },
	{ GSC16AO_PLX_MARBR, "GSC16AO_PLX_MARBR " },
	{ GSC16AO_PLX_BIGEND,"GSC16AO_PLX_BIGEND" },
	{ GSC16AO_PLX_EROMRR,"GSC16AO_PLX_EROMRR" },
	{ GSC16AO_PLX_EROMBA,"GSC16AO_PLX_EROMBA" },
	{ GSC16AO_PLX_LBRD0, "GSC16AO_PLX_LBRD0 " },
	{ GSC16AO_PLX_DMRR,  "GSC16AO_PLX_DMRR  " },
	{ GSC16AO_PLX_DMLBAM,"GSC16AO_PLX_DMLBAM" },
	{ GSC16AO_PLX_DMLBAI,"GSC16AO_PLX_DMLBAI" },
	{ GSC16AO_PLX_DMPBAM,"GSC16AO_PLX_DMPBAM" },
	{ GSC16AO_PLX_DMCFGA,"GSC16AO_PLX_DMCFGA" },
	{ GSC16AO_PLX_LAS1RR,"GSC16AO_PLX_LAS1RR" },
	{ GSC16AO_PLX_LAS1BA,"GSC16AO_PLX_LAS1BA" },
	{ GSC16AO_PLX_LBRD1, "GSC16AO_PLX_LBRD1 " },
	{ 0,	0, }
};

what	plx_runtime_reg[]= {
	{ GSC16AO_PLX_MBOX0,    "GSC16AO_PLX_MBOX0    " },
	{ GSC16AO_PLX_MBOX1,    "GSC16AO_PLX_MBOX1    " },
	{ GSC16AO_PLX_MBOX2,    "GSC16AO_PLX_MBOX2    " },
	{ GSC16AO_PLX_MBOX3,    "GSC16AO_PLX_MBOX3    " },
	{ GSC16AO_PLX_MBOX4,    "GSC16AO_PLX_MBOX4    " },
	{ GSC16AO_PLX_MBOX5,    "GSC16AO_PLX_MBOX5    " },
	{ GSC16AO_PLX_MBOX6,    "GSC16AO_PLX_MBOX6    " },
	{ GSC16AO_PLX_MBOX7,    "GSC16AO_PLX_MBOX7    " },
	{ GSC16AO_PLX_P2LDBELL, "GSC16AO_PLX_P2LDBELL " },
	{ GSC16AO_PLX_L2PDBELL, "GSC16AO_PLX_L2PDBELL " },
	{ GSC16AO_PLX_INTCSR,   "GSC16AO_PLX_INTCSR   " },
	{ GSC16AO_PLX_CNTRL,    "GSC16AO_PLX_CNTRL    " },
	{ GSC16AO_PLX_PCIHIDR,  "GSC16AO_PLX_PCIHIDR  " },
	{ GSC16AO_PLX_PCIHREV,  "GSC16AO_PLX_PCIHREV  " },
	{ GSC16AO_PLX_MBOX0_ALT,"GSC16AO_PLX_MBOX0_ALT" },
	{ GSC16AO_PLX_MBOX1_ALT,"GSC16AO_PLX_MBOX1_ALT" },
	{ 0,	0, }
};

what	plx_dma_reg[]= {
	{ GSC16AO_PLX_DMAMODE0, "GSC16AO_PLX_DMAMODE0 " },
	{ GSC16AO_PLX_DMAPADR0, "GSC16AO_PLX_DMAPADR0 " },
	{ GSC16AO_PLX_DMALADR0, "GSC16AO_PLX_DMALADR0 " },
	{ GSC16AO_PLX_DMASIZ0,  "GSC16AO_PLX_DMASIZ0  " },
	{ GSC16AO_PLX_DMADPR0,  "GSC16AO_PLX_DMADPR0  " },
	{ GSC16AO_PLX_DMAMODE1, "GSC16AO_PLX_DMAMODE1 " },
	{ GSC16AO_PLX_DMAPADR1, "GSC16AO_PLX_DMAPADR1 " },
	{ GSC16AO_PLX_DMALADR1, "GSC16AO_PLX_DMALADR1 " },
	{ GSC16AO_PLX_DMASIZ1,  "GSC16AO_PLX_DMASIZ1  " },
	{ GSC16AO_PLX_DMADPR1,  "GSC16AO_PLX_DMADPR1  " },
	{ GSC16AO_PLX_DMACSR01, "GSC16AO_PLX_DMACSR01 " },
	{ GSC16AO_PLX_DMAARB,   "GSC16AO_PLX_DMAARB   " },
	{ GSC16AO_PLX_DMATHR,   "GSC16AO_PLX_DMATHR   " }, 
	{ 0,	0, }
};

what	plx_msg_queue_reg[]= {
	{ GSC16AO_PLX_OPLFIS,   "GSC16AO_PLX_OPLFIS   " },
	{ GSC16AO_PLX_OPLFIM,   "GSC16AO_PLX_OPLFIM   " },
	{ GSC16AO_PLX_IQP,      "GSC16AO_PLX_IQP      " },
	{ GSC16AO_PLX_OQP,      "GSC16AO_PLX_OQP      " },
	{ GSC16AO_PLX_MQCR,     "GSC16AO_PLX_MQCR     " },
	{ GSC16AO_PLX_QBAR,     "GSC16AO_PLX_QBAR     " },
	{ GSC16AO_PLX_IFHPR,    "GSC16AO_PLX_IFHPR    " },
	{ GSC16AO_PLX_IFTPR,    "GSC16AO_PLX_IFTPR    " },
	{ GSC16AO_PLX_IPHPR,    "GSC16AO_PLX_IPHPR    " },
	{ GSC16AO_PLX_IPTPR,    "GSC16AO_PLX_IPTPR    " },
	{ GSC16AO_PLX_OFHPR,    "GSC16AO_PLX_OFHPR    " },
	{ GSC16AO_PLX_OFTPR,    "GSC16AO_PLX_OFTPR    " },
	{ GSC16AO_PLX_OPHPR,    "GSC16AO_PLX_OPHPR    " },
	{ GSC16AO_PLX_OPTPR,    "GSC16AO_PLX_OPTPR    " },
	{ 0,	0, }
};

char devname[]="/dev/gsc16ao0";
int	 fp;
volatile int  *gscptr, *plxptr, page_size;
int  *munmap_gscptr, *munmap_plxptr; 

struct timespec  stp, etp;
double empty_deltat, deltat;

void BadArg(char *arg);
void disp_regs(volatile int *ptr, char *desc, what *what);
void disp_pci_config(char *desc, what *what, int argc);

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    unsigned long int offset;

	if(argc == 2) {
		if(argv[1][0] < '0' || argv[1][0] > '9')
			BadArg(argv[1]);
		if(strlen(argv[1]) > 1)
			BadArg(argv[1]);
		devname[12]= argv[1][0];
	} 

	fp	= open(devname, O_RDWR);
	
	if(fp < 0) {
		fprintf(stderr,"open failed: %s\n",strerror(errno));
	 	exit(1);
	}
	if (fp == -1) {
		printf(	"open() failure on %s, errno = %d\n",
				devname, errno);
		perror("Open failed");
		exit(1);
	}

	/*** Map GSC GSC16AO CONTROL AND STATUS REGISTERS ***/
	gscptr = munmap_gscptr = (int *) mmap((caddr_t)0 ,
			      GSC16AO_GSC_REGS_MMAP_SIZE, 
			      PROT_READ|PROT_WRITE,
			      MAP_SHARED,
			      fp,
			      GSC16AO_GSC_REGS_MMAP_OFFSET);

	if(gscptr == MAP_FAILED) {
		printf("mmap() failure on %s, %s [errno = %d]\n",devname,strerror(errno),errno);
		exit(1);
	}

    offset = GSC16AO_GSC_REGS_MMAP_OFFSET;
    if(ioctl(fp, IOCTL_GSC16AO_GET_OFFSET,&offset)) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_OFFSET) Failed: %s\n",
            strerror(errno));
        exit(1);
    }

    gscptr = (int *)((char *)gscptr + offset);

	/*** MAP PLX ADDRESS ***/
	plxptr = (int *) mmap((caddr_t)0,
			      GSC16AO_PLX_REGS_MMAP_SIZE, 
			      PROT_READ|PROT_WRITE,
			      MAP_SHARED,
			      fp,
			      GSC16AO_PLX_REGS_MMAP_OFFSET);

	if(plxptr == MAP_FAILED) {
		printf("mmap() failure on %s, %s [errno = %d]\n",devname,strerror(errno),errno);
		exit(1);
	}

    offset = GSC16AO_PLX_REGS_MMAP_OFFSET;
    if(ioctl(fp, IOCTL_GSC16AO_GET_OFFSET,&offset)) {
        fprintf(stderr,"ioctl(GSC16AO_PLX_REGS_MMAP_OFFSET) Failed: %s\n",
            strerror(errno));
        exit(1);
    }

    plxptr = (int *)((char *)plxptr + offset);

	disp_pci_config("PCI Configuration",pci_config_reg,argc);

	disp_regs(gscptr,"GSC16AO",gsc_reg);

	disp_regs(plxptr,"PLX Local Configuration",plx_local_config_reg);
	disp_regs(plxptr,"PLX Run Time",plx_runtime_reg);
	disp_regs(plxptr,"PLX DMA",plx_dma_reg);
	disp_regs(plxptr,"PLX Message Queue",plx_msg_queue_reg);

	/*** unmap GSC area ***/
	munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);

	/*** unmap PLX area ***/
	munmap((void *)munmap_plxptr, GSC16AO_PLX_REGS_MMAP_SIZE);

	exit(0);
}

void
disp_regs(volatile int *ptr, char *desc, what *what)
{
	int	i = 0;

	fprintf(stderr,"\n==== %s Registers ====\n",desc);
	while(what[i].name) {
		fprintf(stderr,"%s\t= 0x%08x\t@%p\n",what[i].name,
			ptr[what[i].index],
			ptr+what[i].index); 
		i++;
	}
}

void
disp_pci_config(char *desc, what *what, int argc)
{
        REGISTER_PARAMS reg;
	int	i = 0;

	reg.regset   = GSC16AO_PCI_REGISTER;
	
	fprintf(stderr,"\n==== %s Registers ====\n",desc);
	while(what[i].name) {
		reg.regnum = what[i].index;
		reg.regval = 0xDEADBEEF;
		ioctl(fp, IOCTL_GSC16AO_READ_REGISTER,(unsigned long)&reg);
		fprintf(stderr,"%s\t= 0x%08lx\t@0x%08x\n",
			what[i].name,
			reg.regval,
			what[i].index); 
		i++;
	}
}

/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
	fprintf(stderr,"\n*** Invalid Argument [%s] ***\n",arg);
	fprintf(stderr,"Usage: gsc16ao_reg <device_num 0-9>\n");
	exit(1);
}

