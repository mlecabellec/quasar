/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_reg.c                                              *
 *                                                                           *
 * Description:  Display Registers Test                                      *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_reg          ==> start display registers test on device 0      *
 *   pcie6509_reg <dn>     ==> start display registers test on device <dn>.  *
 *                             (where dn is device number)                   *
 *                                                                           *
 * Date:         02/13/2014                                                  *
 * History:                                                                  *
 *                                                                           *
 *  1  02/13/14 D. Dubash                                                    *
 *              Initial release                                              *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 * Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 * All rights reserved                                                       *
 *                                                                           *
 *****************************************************************************/

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
 *****************************************************************************/

/*
 * Headers
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include "pcie6509_user.h"
#include "pcie6509_snap.h"

#define PRINT_VAL(IteM) {           \
    fprintf(stderr,"\t%-40s=0x%08x\t@0x%08lx\n",#IteM, ptr->IteM,\
    ((long)&ptr->IteM - (long)ptr)); \
}

#define PRINT_VAL_SHADOW(IteM) {           \
    fprintf(stderr,"\t%-38s=0x%08x\t@0x%08lx\n",#IteM, ptr->IteM,\
    ((long)&local_ptr->IteM - (long)local_ptr)); \
}

#define PRINT_IO_PORT(Port,IteM) {           \
    fprintf(stderr,"\t%02d:%-27s=0x%08x\t@0x%08lx\n",Port,#IteM, \
                       ptr->io_port[Port].IteM,\
                        ((long)&ptr->io_port[Port].IteM - (long)ptr)); \
}

int     fp;
int     *munmap_local_ptr, *munmap_shadow_ptr;
unsigned long mmap_local_size;
unsigned long mmap_shadow_size;

pcie6509_local_ctrl_data_t *local_ptr;
pcie6509_shadow_regs_t *shadow_ptr;

#define DEF_DEV_NAME   "/dev/" PCIE6509_DRIVER_NAME
#define DEF_BOARD_NO   "0"

/* Global Definintions */
char        devname[15];

/* prototype */
void BadArg(char *arg);
void disp_local_registers(pcie6509_local_ctrl_data_t *local_ptr);
void disp_local_shadow_registers(pcie6509_shadow_regs_t *shadow_ptr);

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    int status;
    pcie6509_mmap_select_t mmap_select;
    char    *bnp=0;
    char    *endptr;

    if(argc > 2)    /* more than one argument, error out */
        BadArg(0);

    if(argc == 2) {    /* only one argument specified */
        bnp=argv[1];
        strtoul(bnp, &endptr, 10);
        if(!endptr || *endptr != 0) 
            BadArg(argv[1]);
    }
    
    if(!bnp)    /* if no device number entered, use default */
        bnp = DEF_BOARD_NO;

    sprintf(devname,"%s%s",DEF_DEV_NAME,bnp);
    fprintf(stderr,"\nDevice Name: %s\n",devname);

    fp    = open(devname, O_RDWR);

    if (fp == -1) {
        fprintf(stderr,"open() failure on %s: %s\n",
                devname, strerror(errno));
        exit(1);
    }

    /*** Select Local Registers ***/
    /*** This IOCTL must be called before mmap() ***/
    mmap_select.select = PCIE6509_SELECT_LOCAL_MMAP;
    mmap_select.offset=0;
    mmap_select.size=0;

    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT,(void *)&mmap_select);

    if(status) {
        /* if ENOMEM, no local region present, skip it */
        if(errno == ENOMEM) {
            local_ptr = NULL;
            munmap_local_ptr = NULL;
            fprintf(stderr,"Local Region Not Present\n");
            goto skip_local_region;
        }
        fprintf(stderr,"ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: %s\n",
                        strerror(errno));
        exit(1);
    }

    mmap_local_size = mmap_select.size; /* save for unmap */

    /*** Map LOCAL PCIE6509 CONTROL AND STATUS REGISTERS ***/
    munmap_local_ptr = (int *) mmap((caddr_t)0 ,
                        mmap_local_size, 
                        (PROT_READ|PROT_WRITE), MAP_SHARED, fp, 0);
    local_ptr = (pcie6509_local_ctrl_data_t *)munmap_local_ptr;

    if(local_ptr == MAP_FAILED) {
        fprintf(stderr,"mmap() failure on %s: %s\n",devname,strerror(errno));
        exit(1);
    }

    local_ptr = (pcie6509_local_ctrl_data_t *)((char *)local_ptr + 
                                                    mmap_select.offset);
    fprintf(stderr,"LOCAL Register %p Offset=0x%lx\n",local_ptr,
                                                    mmap_select.offset);

#if 0
    /* DO NOT USE mmap_local_size AS YOU COULD WOULD CRASH SYSTEM */
    PCIE6509_Snap(stderr, "#### LOCAL REGS ####", (char *)local_ptr, 
                256 /*mmap_local_size*/, 0, "+LCL+ ", 'i');
    fprintf(stderr,"\n");
#endif

skip_local_region:

    /*** Select Shadow Registers ***/
    /*** This IOCTL must be called before mmap() ***/
    mmap_select.select = PCIE6509_SELECT_SHADOW_REG_MMAP;
    mmap_select.offset=0;
    mmap_select.size=0;

    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT,(void *)&mmap_select);

    if(status) {
        /* if ENOMEM, no shadow memory present, skip it */
        if(errno == ENOMEM) {
            shadow_ptr = NULL;
            munmap_shadow_ptr = NULL;
            fprintf(stderr,"Shadow Memory Not Present\n");
            goto skip_shadow_region;
        }
        fprintf(stderr,"ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: %s\n",
                        strerror(errno));
        exit(1);
    }

    mmap_shadow_size = mmap_select.size; /* save for unmap */

    /*** Map LOCAL PCIE6509 CONTROL AND STATUS REGISTERS ***/
    munmap_shadow_ptr = (int *) mmap((caddr_t)0 ,
                        mmap_shadow_size, 
                        (PROT_READ|PROT_WRITE), MAP_SHARED, fp, 0);
    shadow_ptr = (pcie6509_shadow_regs_t *)munmap_shadow_ptr;

    if(shadow_ptr == MAP_FAILED) {
        fprintf(stderr,"mmap() failure on %s: %s\n",devname,strerror(errno));
        exit(1);
    }

    shadow_ptr = (pcie6509_shadow_regs_t *)((char *)shadow_ptr + 
                                                    mmap_select.offset);
    fprintf(stderr,"Shadow Register %p Offset=0x%lx\n",shadow_ptr,
                                                    mmap_select.offset);

skip_shadow_region:
    /* Display register information */
    disp_local_registers((pcie6509_local_ctrl_data_t *)local_ptr);
    disp_local_shadow_registers((pcie6509_shadow_regs_t *)shadow_ptr);

    /*** unmap LOCAL area ***/
    if(munmap_local_ptr != NULL) {
        status = munmap((void *)munmap_local_ptr, mmap_local_size);
        if(status == (long)MAP_FAILED)
            fprintf(stderr,"munmap() failure on %s, arg=%p: %s\n",devname,    
                            munmap_local_ptr, strerror(errno));
    }

    /*** close the device ***/
    status = close(fp);
    if(status == -1)
        fprintf(stderr,"close() failure on %s: %s\n",devname,strerror(errno));

    exit(0);
}


/******************************************************************************
 *** Display LOCAL Registers                                                ***
 ******************************************************************************/
void
disp_local_registers(pcie6509_local_ctrl_data_t *ptr)
{
    if(ptr == NULL)
        return;

    fprintf(stderr,"\n======= LOCAL REGISTERS =========\n");
    PRINT_VAL(CHInCh.Identification);
    PRINT_VAL(CHInCh.InterruptMask);
    PRINT_VAL(CHInCh.InterruptStatus);
    PRINT_VAL(CHInCh.VolatileInterruptStatus);
    PRINT_VAL(CHInCh.Scrap);
    PRINT_VAL(CHInCh.PCISubsystemID);

    fprintf(stderr,"----------------------------------------------\n");
    PRINT_VAL(Master.CSScratchPad);
    PRINT_VAL(Master.CSSignature);
    PRINT_VAL(Master.CSTimeSincePowerUp);
    PRINT_VAL(Master.CSWatchdogStatus);
    PRINT_VAL(Master.CSGlobalInterruptStatus);
    PRINT_VAL(Master.CSDIInterruptStatus);
    PRINT_VAL(Master.CSWatchdogTimerInterruptStatus);
    PRINT_VAL(Master.PFIStaticDigitalInput);
    PRINT_VAL(Master.DioPortsStaticDigitalInput);
    PRINT_VAL(Master.CSDIChangeDetectionStatus);
    PRINT_VAL(Master.PFIChangeDetectionLatched);
    PRINT_VAL(Master.CSIntForwardingControlStatus);
    PRINT_VAL(Master.CSIntForwardingDestination);

    fprintf(stderr,"----------------------------------------------\n");
    PRINT_VAL(Slave.CSScratchPad);
    PRINT_VAL(Slave.CSSignature);
    PRINT_VAL(Slave.CSTimeSincePowerUp);
    PRINT_VAL(Slave.CSWatchdogStatus);
    PRINT_VAL(Slave.CSGlobalInterruptStatus);
    PRINT_VAL(Slave.CSDIInterruptStatus);
    PRINT_VAL(Slave.CSWatchdogTimerInterruptStatus);
    PRINT_VAL(Slave.PFIStaticDigitalInput);
    PRINT_VAL(Slave.DioPortsStaticDigitalInput);
    PRINT_VAL(Slave.CSDIChangeDetectionStatus);
    PRINT_VAL(Slave.PFIChangeDetectionLatched);
    PRINT_VAL(Slave.CSIntForwardingControlStatus);
    PRINT_VAL(Slave.CSIntForwardingDestination);
    fprintf(stderr,"----------------------------------------------\n");
}

/******************************************************************************
 *** Display LOCAL SHADOW Registers                                         ***
 ******************************************************************************/
void
disp_local_shadow_registers(pcie6509_shadow_regs_t *ptr)
{
    if(ptr == NULL)
        return;

    fprintf(stderr,"\n======= BOARD MASTER [SHADOW] WRITE REGISTERS =========\n");
    PRINT_VAL_SHADOW(Master.CSJointReset);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimeout);
    PRINT_VAL_SHADOW(Master.CSWatchdogConfiguration);
    PRINT_VAL_SHADOW(Master.CSWatchdogControl);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimerInterrupt1);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimerInterrupt2);
    PRINT_VAL_SHADOW(Master.CSGlobalInterruptEnable);
    PRINT_VAL_SHADOW(Master.PFIDirection);
    PRINT_VAL_SHADOW(Master.CSRTSITrigDirection);

    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[0]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[1]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[2]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[3]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[4]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[5]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[6]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[7]);
    
    PRINT_VAL_SHADOW(Master.PFIFilterPort0Low);
    PRINT_VAL_SHADOW(Master.PFIFilterPort0High);
    PRINT_VAL_SHADOW(Master.PFIFilterPort1Low);
    PRINT_VAL_SHADOW(Master.PFIFilterPort1High);
    
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[0]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[1]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[2]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[3]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[4]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[5]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[6]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[7]);
    
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[0]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[1]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[2]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[3]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[4]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[5]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[6]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[7]);
    
    PRINT_VAL_SHADOW(Master.PFIStaticDigitalOutput);
    PRINT_VAL_SHADOW(Master.PFIWDTSafeState);
    PRINT_VAL_SHADOW(Master.PFIWDTModeSelect);
    PRINT_VAL_SHADOW(Master.DioPortsStaticDigitalOutput);
    PRINT_VAL_SHADOW(Master.DioPortsDIODirection);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTSafeState);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTModeSelectP01);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTModeSelectP23);
    PRINT_VAL_SHADOW(Master.DioPortsDIChangeIrqRE);
    PRINT_VAL_SHADOW(Master.DioPortsDIChangeIrqFE);
    PRINT_VAL_SHADOW(Master.PFIChangeIrq);
    PRINT_VAL_SHADOW(Master.DioPortsDIFilterP01);
    PRINT_VAL_SHADOW(Master.DioPortsDIFilterP23);
    PRINT_VAL_SHADOW(Master.CSChangeDetectionIrq);

    fprintf(stderr,"\n======= BOARD SLAVE [SHADOW] WRITE REGISTERS =========\n");
    PRINT_VAL_SHADOW(Slave.CSJointReset);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimeout);
    PRINT_VAL_SHADOW(Slave.CSWatchdogConfiguration);
    PRINT_VAL_SHADOW(Slave.CSWatchdogControl);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimerInterrupt1);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimerInterrupt2);
    PRINT_VAL_SHADOW(Slave.CSGlobalInterruptEnable);
    PRINT_VAL_SHADOW(Slave.PFIDirection);
    PRINT_VAL_SHADOW(Slave.CSRTSITrigDirection);

    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[0]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[1]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[2]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[3]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[4]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[5]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[6]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIFilterPort0Low);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort0High);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort1Low);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort1High);
    
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[0]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[1]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[2]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[3]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[4]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[5]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[6]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[0]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[1]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[2]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[3]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[4]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[5]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[6]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIStaticDigitalOutput);
    PRINT_VAL_SHADOW(Slave.PFIWDTSafeState);
    PRINT_VAL_SHADOW(Slave.PFIWDTModeSelect);
    PRINT_VAL_SHADOW(Slave.DioPortsStaticDigitalOutput);
    PRINT_VAL_SHADOW(Slave.DioPortsDIODirection);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTSafeState);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTModeSelectP01);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTModeSelectP23);
    PRINT_VAL_SHADOW(Slave.DioPortsDIChangeIrqRE);
    PRINT_VAL_SHADOW(Slave.DioPortsDIChangeIrqFE);
    PRINT_VAL_SHADOW(Slave.PFIChangeIrq);
    PRINT_VAL_SHADOW(Slave.DioPortsDIFilterP01);
    PRINT_VAL_SHADOW(Slave.DioPortsDIFilterP23);
    PRINT_VAL_SHADOW(Slave.CSChangeDetectionIrq);
}

/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
    if(arg)
        fprintf(stderr,"\n*** Invalid argument [%s] ***\n",arg);
    else
        fprintf(stderr,"\n*** Only one argument must be specified ***\n");

    fprintf(stderr,"Usage: pcie6509_reg <device number>\n");
    exit(1);
}
