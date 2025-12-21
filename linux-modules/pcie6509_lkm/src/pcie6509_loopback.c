/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_loopback.c                                         *
 *                                                                           *
 * Description:  Loopback test                                               *
 *                                                                           *
 * Syntax:                                                                   *
 *  pcie6509_loopback         ==> start display registers test on device 0   *
 *  pcie6509_loopback -b <dn> ==> start display registers test on device <dn>*
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

/*****************************************************************************
 * Loopback cable connection:                                                *
 *                                                                           *
 *          P0  <--->  P6                                                    *
 *          P1  <--->  P7                                                    *
 *          P2  <--->  P8                                                    *
 *          P3  <--->  P9                                                    *
 *          P4  <--->  P10                                                   *
 *          P5  <--->  P11                                                   *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *         <--------------- P I N S ------------------>                      *
 * Bucket  +--------------+--------------+--------------+                    *
 *   0     | 01,03,...,15 | 17,19,...,31 | 33,35,...,47 |                    *
 *         +--------------+--------------+--------------+                    *
 *   1     | 02,04,...,16 | 18,20,...,32 | 34,36,...,48 |                    *
 *         +--------------+--------------+--------------+                    *
 *   2     | 51,53,...,65 | 67,69,...,81 | 83,85,...,97 |                    *
 *         +--------------+--------------+--------------+                    *
 *   3     | 52,54,...,66 | 68,70,...,82 | 84,86,...,98 |                    *
 *         +--------------+--------------+--------------+                    *
 *                                                                           *
 *         <--------------- P O R T S ---------------->                      *
 * Bucket  +--------------+--------------+--------------+                    *
 *   0     |       2      |       1      |       0      |                    *
 *         +--------------+--------------+--------------+                    *
 *   1     |       5      |       4      |       3      |                    *
 *         +--------------+--------------+--------------+                    *
 *   2     |       8      |       7      |       6      |                    *
 *         +--------------+--------------+--------------+                    *
 *   3     |      11      |      10      |       9      |                    *
 *         +--------------+--------------+--------------+                    *
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


/* prototype */
void    usage(char * prog_p);
void    start_loop();
void    quit(int s);
void    write_pattern(int grp, u_int pattern, u_int *pattern_read);
void    read_pattern(int grp, u_int *pattern);
void    doit(int grp, int pattern, char *desc);
void    dump_mismatch();

int        fp;
int     *munmap_local_ptr, *munmap_shadow_ptr;
unsigned long mmap_local_size;
unsigned long mmap_shadow_size;

volatile short *read_port[PCIE6509_MAX_PORTS];
volatile int *write_port[PCIE6509_MAX_PORTS];

pcie6509_local_ctrl_data_t *local_ptr;
pcie6509_shadow_regs_t *shadow_ptr;

#define DEF_DEV_NAME    "/dev/" PCIE6509_DRIVER_NAME
#define DEF_BOARD_NO    0
#define DEF_LOOP_CNT    1000    /* 0=loop forever, else count */
#define DEF_DELAY       0       /* 0 second */
#define SUBGROUP        (PCIE6509_MAX_PORTS/6)                 /* 12/6 -> 2 */
#define PORTS_PER_GROUP (PCIE6509_MAX_PORTS/4)                 /* 12/4 -> 3 */
#define BUCKETS         (PCIE6509_MAX_PORTS/PORTS_PER_GROUP)   /* 12/3 -> 4 */

/* Global Definintions */
int     loop;
int     loop_cnt = DEF_LOOP_CNT;
long    delay = DEF_DELAY;
int     break_received=0;
char    devname[15];
int     Wsubd[SUBGROUP], Wsubd_Data[SUBGROUP];
int     Rsubd[SUBGROUP], Rsubd_Data[SUBGROUP];
int     Mismatch[BUCKETS];
int     Mismatch_History[BUCKETS];

char *pattern_desc[] = { "[A: Setting Single Line] ",
                         "[B: Setting All Lines]   ",
                         "[C: Clearing Single Line]",
                         "[D: Clearing All Lines]  ",
};
unsigned char ReadDIPort(pcie6509_local_ctrl_data_t *lp, int port);
void WriteDOPort(pcie6509_local_ctrl_data_t *lp, 
                 pcie6509_shadow_regs_t *sp, int port, unsigned char value);
void PortDirection(pcie6509_local_ctrl_data_t *lp, 
                   pcie6509_shadow_regs_t *sp, int port, int output);
void PortDirectionAll(pcie6509_local_ctrl_data_t *lp, 
                   pcie6509_shadow_regs_t *sp, int output);

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    extern  char    *optarg;
    extern  int     optind, opterr;
    int             status, option;
    int             bnp=DEF_BOARD_NO;
    char            *pt;
    char            *prog_p = argv[0];
    pcie6509_mmap_select_t mmap_select;
            
    signal(SIGINT, quit);

    while ((option = getopt( argc, argv, "b:d:l:" )) != EOF) {
        switch (option) {
        case 'b':  /* board */
            bnp = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! board number not specified\n",
                    prog_p);
                exit(1);
            }
            break;

        case 'd': /* delay - microseconds */
            delay = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,
                    "%s: ERROR! Delay not specified\n",
                    prog_p);
                exit(1);
            }
            break;

        case 'l':  /* Loop Count */
            loop_cnt = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! loop count not specified\n",
                    prog_p);
                exit(1);
            }
            break;

        default:
            usage(prog_p);
        }
    }

    /* if spureous arguments, error out */
    if(optind != argc) {
        usage(prog_p);
    }

    sprintf(devname,"%s%d",DEF_DEV_NAME,bnp);
    fprintf(stderr,"\nDevice Name: %s\n",devname);

    fp    = open(devname, O_RDWR);

    if (fp == -1) {
        fprintf(stderr,"open() failure on %s: %s\n",
                devname, strerror(errno));
        exit(1);
    }

    /* reset board */
    ioctl(fp, IOCTL_PCIE6509_RESET_BOARD, NULL);

    bzero(Mismatch_History, sizeof(Mismatch_History));

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
            goto abort;
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
            goto abort;
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

    /* Start the main loop */
    start_loop();

abort:

    /*** unmap LOCAL area ***/
    if(munmap_local_ptr != NULL) {
        status = munmap((void *)munmap_local_ptr, mmap_local_size);
        if(status == (long)MAP_FAILED)
            fprintf(stderr,"munmap() failure on %s, arg=%p: %s\n",devname,    
                            munmap_local_ptr, strerror(errno));
    }

    if(munmap_shadow_ptr != NULL) {
        status = munmap((void *)munmap_shadow_ptr, mmap_shadow_size);
        if(status == (long)MAP_FAILED)
            fprintf(stderr,"munmap() failure on %s, arg=%p: %s\n",devname,    
                            munmap_shadow_ptr, strerror(errno));
    }

    /*** close the device ***/
    status = close(fp);
    if(status == -1)
        fprintf(stderr,"close() failure on %s: %s\n",devname,strerror(errno));

    exit(0);
}

void
start_loop()
{
    int     port, group, line_mask;
    u_int   pattern=0;

    loop = loop_cnt;

again:
    loop--;
    if(loop & 1) {  /* swap roles */
        Wsubd[0]    = 0;    /* ports: 0..2 */
        Wsubd[1]    = 3;    /* ports: 3..5 */
        Rsubd[0]    = 6;    /* ports: 6..8 */
        Rsubd[1]    = 9;    /* ports: 9..11 */
    } else {
        Rsubd[0]    = 0;    /* ports: 0..2 */
        Rsubd[1]    = 3;    /* ports: 3..5 */
        Wsubd[0]    = 6;    /* ports: 6..8 */
        Wsubd[1]    = 9;    /* ports: 9..11 */
    }

    /* first set all ports for inputs */
    PortDirectionAll(local_ptr,shadow_ptr,0);
#if 0
int lines;
PortDirection(local_ptr,shadow_ptr,5,1);
fprintf(stderr,"Master.DioPortsDIODirection=0x%08x\n",shadow_ptr->Master.DioPortsDIODirection);
fprintf(stderr,"Slave.DioPortsDIODirection=0x%08x\n",shadow_ptr->Slave.DioPortsDIODirection);
fprintf(stderr,"Master.PFIDirection=0x%04x\n",shadow_ptr->Master.PFIDirection);
fprintf(stderr,"Slave.PFIDirection=0x%04x\n",shadow_ptr->Slave.PFIDirection);
for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
fprintf(stderr,"Master.PFIOutputSelectPort0[%d]=0x%02x\n",lines,shadow_ptr->Master.PFIOutputSelectPort0[lines]);
fprintf(stderr,"Master.PFIOutputSelectPort1[%d]=0x%02x\n",lines,shadow_ptr->Master.PFIOutputSelectPort1[lines]);
fprintf(stderr,"Slave.PFIOutputSelectPort0[%d] =0x%02x\n",lines,shadow_ptr->Slave.PFIOutputSelectPort0[lines]);
fprintf(stderr,"Slave.PFIOutputSelectPort1[%d] =0x%02x\n",lines,shadow_ptr->Slave.PFIOutputSelectPort1[lines]);
    }

exit(1);
#endif

    /* set Wsubd port directions as outputs */
    for(group=0; group < SUBGROUP;  group++) {
        for(port=Wsubd[group]; port < (Wsubd[group]+PORTS_PER_GROUP); port++) {
            PortDirection(local_ptr,shadow_ptr,port,1);
        }
    }

    /**************************************************************
     ****************    write pattern - CASE A    ****************
     **************************************************************/
    /* Setting single Line */

    /* write zero to all lines */
    for(group=0; group< SUBGROUP;  group++) {
        write_pattern(group,0,NULL);
    }

    for(group=0; group< SUBGROUP;  group++) {
        if(break_received) break;
            
        for(line_mask=1; line_mask <= 0x01000000; line_mask <<= 1) {
            if(break_received) break;
            pattern = (line_mask & 0x00ffffff);
            doit(group, pattern, pattern_desc[0]);
        }
    }
    /**************************************************************
     ****************    write pattern - CASE B    ****************
     **************************************************************/
    /* Setting all lines */

    /* write zero to all lines */
    for(group=0; group< SUBGROUP;  group++) {
        write_pattern(group,0,NULL);
    }

    for (group = 0; group < SUBGROUP; group++) {
        if(break_received) break;

        pattern = 0;
        for(line_mask=1; line_mask < 0x01000000; line_mask <<= 1) {
            pattern |= (line_mask & 0x00ffffff);
            doit(group, pattern, pattern_desc[1]);
        }
    }

    /**************************************************************
     ****************    write pattern - CASE C    ****************
     **************************************************************/
    /* Clearing single line */

    /* write ones to all lines */
    for(group=0; group< SUBGROUP;  group++) {
        write_pattern(group,0x00ffffff,NULL);
    }

    for (group = 0; group < SUBGROUP; group++) {
        if(break_received) break;
        for(line_mask=1; line_mask <= 0x01000000; line_mask <<= 1) {
            pattern = (0x00ffffff & ~line_mask);
            doit(group, pattern, pattern_desc[2]);
        }
    }

    /**************************************************************
     ****************    write pattern - CASE D    ****************
     **************************************************************/
    /* Clearing all lines */

    /* write ones to all lines */
    for(group=0; group< SUBGROUP;  group++) {
        write_pattern(group,0x00ffffff,NULL);
    }

    for (group = 0; group < SUBGROUP; group++) {
        if(break_received) break;
        for(line_mask=0x00ffffff; (line_mask & 0x1ffffff); line_mask <<= 1) {
            pattern = (0x00ffffff & line_mask);
            doit(group, pattern, pattern_desc[3]);
        }
    }

    /* if no break received, continue */
    if(break_received == 0) {
        if(loop_cnt == 0)
            goto again;

        if(loop != 0)
            goto again;
    }

    fprintf(stderr,"\n");

    dump_mismatch();

    return;
}

void
write_pattern(int grp, u_int pattern, u_int *pattern_read)
{
    int     port;
    int     end_port = Wsubd[grp]+PORTS_PER_GROUP;
    u_char  data;

    Wsubd_Data[grp] = pattern;

    for(port=Wsubd[grp]; port < end_port; port++) {
        data = (pattern & 0xff);
        WriteDOPort(local_ptr,shadow_ptr,port,data);
        pattern = (pattern >> 8);
    }

    if(pattern_read) {
        *pattern_read=0;
        for(port=(end_port-1); port >=Wsubd[grp]; port--) {
            *pattern_read <<= 8;
            data = ReadDIPort(local_ptr,port);
            *pattern_read |= data;
        }
    }

    return;
}

void
read_pattern(int grp, u_int *pattern)
{
    int     port;
    int     end_port = Rsubd[grp]+PORTS_PER_GROUP;
    u_char  data;

    *pattern=0;
    for(port=(end_port-1); port >=Rsubd[grp]; port--) {
        *pattern <<= 8;
        data = ReadDIPort(local_ptr,port);
        *pattern |= data;
    }

    Rsubd_Data[grp] = *pattern;
    return;
}

void
doit(int grp, int pattern, char *desc)
{
    u_int   pattern_read, looped_pattern_read;

    write_pattern(grp,pattern, &pattern_read);
    read_pattern(grp,&looped_pattern_read);

    Mismatch[Wsubd[grp]/PORTS_PER_GROUP] = (pattern ^ looped_pattern_read);
    Mismatch[Rsubd[grp]/PORTS_PER_GROUP] = (pattern ^ looped_pattern_read);

    Mismatch_History[Wsubd[grp]/PORTS_PER_GROUP] |= (pattern ^ looped_pattern_read);
    Mismatch_History[Rsubd[grp]/PORTS_PER_GROUP] |= (pattern ^ looped_pattern_read);

    /* loop count, write port, read port, wrote data, read data, looped back data */
    fprintf(stderr,"%5d: Wp=%d Rp=%d: Wd=%06x Rd=%06x Ld=%06x %s\r",
            loop_cnt - loop, Wsubd[grp], Rsubd[grp], pattern,
            pattern_read, looped_pattern_read,desc);
    if(pattern != pattern_read) {
        fprintf(stderr,"\n##### MISMATCH: wrote=0x%06x read=0x%06x\n",
                                                pattern,pattern_read);
    }
    if(pattern != looped_pattern_read) {
        fprintf(stderr,"\n##### MISMATCH: wrote=0x%06x looped=0x%06x\n",
                                                pattern,looped_pattern_read);
    }

    if(delay)
        usleep(delay);
}

void quit(int s)
{
    break_received++;
    fprintf(stderr,"\n");
    dump_mismatch();
    exit(1);
}

/******************************************************************************
 *         <--------------- P I N S ------------------>
 * Bucket  +--------------+--------------+--------------+
 *   0     | 01,03,...,15 | 17,19,...,31 | 33,35,...,47 |
 *         +--------------+--------------+--------------+
 *   1     | 02,04,...,16 | 18,20,...,32 | 34,36,...,48 |
 *         +--------------+--------------+--------------+
 *   2     | 51,53,...,65 | 67,69,...,81 | 83,85,...,97 |
 *         +--------------+--------------+--------------+
 *   3     | 52,54,...,66 | 68,70,...,82 | 84,86,...,98 |
 *         +--------------+--------------+--------------+
 *
 *         <--------------- P O R T S ---------------->
 * Bucket  +--------------+--------------+--------------+
 *   0     |       2      |       1      |       0      |
 *         +--------------+--------------+--------------+
 *   1     |       5      |       4      |       3      |
 *         +--------------+--------------+--------------+
 *   2     |       8      |       7      |       6      |
 *         +--------------+--------------+--------------+
 *   3     |      11      |      10      |       9      |
 *         +--------------+--------------+--------------+
 ******************************************************************************/

#define SUB0_PIN(bit) (47 - bit*2)
#define SUB1_PIN(bit) (48 - bit*2)
#define SUB2_PIN(bit) (97 - bit*2)
#define SUB3_PIN(bit) (98 - bit*2)

void
dump_mismatch()
{
    int bucket;
    int bit, pin;
    fprintf(stderr,"\n");

    for(bucket=0; bucket< BUCKETS;  bucket++) {
        fprintf(stderr,"port %02d-%02d: 0x%06x:%s",
            bucket*PORTS_PER_GROUP,
            bucket*PORTS_PER_GROUP+PORTS_PER_GROUP-1,
            Mismatch_History[bucket],
            Mismatch_History[bucket]?" Bad Pins: ":" *** GOOD ***");

        for(bit=23;bit >= 0; bit--) {
            if(Mismatch_History[bucket] & (1 << bit)) {
                pin = -1;
                switch(bucket) {
                    case 0:
                        pin = SUB0_PIN(bit);
                    break;
                    case 1:
                        pin = SUB1_PIN(bit);
                    break;
                    case 2:
                        pin = SUB2_PIN(bit);
                    break;
                    case 3:
                        pin = SUB3_PIN(bit);
                    break;
                }
                fprintf(stderr,"%d ",pin);
            }
        }

        fprintf(stderr,"\n");
    }

    fprintf(stderr,"\n");
}

void
usage( char *prog_p) {
    fprintf(stderr, "usage: %s -[bdl]\n",prog_p);
    fprintf(stderr, "         -b <board>       (default = %d)\n",
            DEF_BOARD_NO);
    fprintf(stderr, "         -d <usec delay>  (default = %d)\n",
            DEF_DELAY);
    fprintf(stderr, "         -l <loop count>  (default = %d) %s\n",
            DEF_LOOP_CNT, DEF_LOOP_CNT?"":"*FOREVER*");
    exit(1);
}


unsigned char 
ReadDIPort(pcie6509_local_ctrl_data_t *lp, int port) 
{
    u_char  port_shift;

    switch(port) {
        case 0 ... 3:
            port_shift = (port * 8); /* 0..3 -> 0..3 */
            return(lp->Master.DioPortsStaticDigitalInput >> port_shift);
        break;

        case 4 ... 5:
            port_shift = ((port - 4) * 8); /* 4..5 -> 0..1 */
            return(lp->Master.PFIStaticDigitalInput >> port_shift);
        break;

        case 6 ... 7:
            port_shift = ((port - 6) * 8); /* 6..7 -> 0..1 */
            return(lp->Slave.PFIStaticDigitalInput >> port_shift);
        break;

        case 8 ... 11:
            port_shift = ((port - 8) * 8); /* 8..11 -> 0..3 */
            return(lp->Slave.DioPortsStaticDigitalInput >> port_shift);
        break;

        default:
            fprintf(stderr,"Invalid Port %d\n",port);
        break;
    }
    return(0);
}

void
WriteDOPort(pcie6509_local_ctrl_data_t *lp, 
            pcie6509_shadow_regs_t *sp, int port, unsigned char value) 
{
    u_int   cur;
    u_char  port_shift;

    switch(port) {
        case 0 ... 3:
            port_shift = (port * 8); /* 0..3 -> 0..3 */
            cur = sp->Master.DioPortsStaticDigitalOutput;
            cur &= ~(0xFF << port_shift);
            cur |= (value << port_shift);
            lp->Master.DioPortsStaticDigitalOutput =
            sp->Master.DioPortsStaticDigitalOutput = cur;
        break;

        case 4 ... 5:
            port_shift = ((port - 4) * 8); /* 4..5 -> 0..1 */
            cur = sp->Master.PFIStaticDigitalOutput;
            cur &= ~(0xFF << port_shift);
            cur |= (value << port_shift);
            lp->Master.PFIStaticDigitalOutput =
            sp->Master.PFIStaticDigitalOutput = cur;
        break;

        case 6 ... 7:
            port_shift = ((port - 6) * 8); /* 6..7 -> 0..1 */
            cur = sp->Slave.PFIStaticDigitalOutput;
            cur &= ~(0xFF << port_shift);
            cur |= (value << port_shift);
            lp->Slave.PFIStaticDigitalOutput =
            sp->Slave.PFIStaticDigitalOutput = cur;
        break;

        case 8 ... 11:
            port_shift = ((port - 8) * 8); /* 8..11 -> 0..3 */
            cur = sp->Slave.DioPortsStaticDigitalOutput;
            cur &= ~(0xFF << port_shift);
            cur |= (value << port_shift);
            lp->Slave.DioPortsStaticDigitalOutput =
            sp->Slave.DioPortsStaticDigitalOutput = cur;
        break;

        default:
            fprintf(stderr,"Invalid Port %d\n",port);
        break;
    }
}

void
PortDirection(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int port, int output)
{
    u_char  value, line_value;
    int     lines;
    u_char  port_shift;

    value = (output)?0xFF:0x00;
    line_value = (output)?0x10:0x00;

    switch(port) {
        case 0 ... 3:
            port_shift = (port * 8); /* 0..3 -> 0..3 */
            sp->Master.DioPortsDIODirection &= ~(0xFF << port_shift);
            sp->Master.DioPortsDIODirection |= (value << port_shift);
            lp->Master.DioPortsDIODirection = sp->Master.DioPortsDIODirection;
        break;

        case 4 ... 5:
            port_shift = ((port - 4) * 8); /* 4..5 -> 0..1 */
            sp->Master.PFIDirection &= ~(0xFF << port_shift);
            sp->Master.PFIDirection |= (value << port_shift);
            lp->Master.PFIDirection = sp->Master.PFIDirection;
            for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
                if(port==4) {
                    lp->Master.PFIOutputSelectPort0[lines] = 
                    sp->Master.PFIOutputSelectPort0[lines] = line_value; /* port 4 */
                }
                else {
                    lp->Master.PFIOutputSelectPort1[lines] =
                    sp->Master.PFIOutputSelectPort1[lines] = line_value; /* port 5 */
                }
            }
        break;

        case 6 ... 7:
            port_shift = ((port - 6) * 8); /* 6..7 -> 0..1 */
            sp->Slave.PFIDirection &= ~(0xFF << port_shift);
            sp->Slave.PFIDirection |= (value << port_shift);
            lp->Slave.PFIDirection = sp->Slave.PFIDirection;
            for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
                if(port==6) {
                    lp->Slave.PFIOutputSelectPort0[lines] =
                    sp->Slave.PFIOutputSelectPort0[lines] = line_value; /* port 6 */
                }
                else {
                    lp->Slave.PFIOutputSelectPort1[lines] = 
                    sp->Slave.PFIOutputSelectPort1[lines] = line_value; /* port 7 */
                }
            }
        break;

        case 8 ... 11:
            port_shift = ((port - 8) * 8); /* 8..11 -> 0..3 */
            sp->Slave.DioPortsDIODirection &= ~(0xFF << port_shift);
            sp->Slave.DioPortsDIODirection |= (value << port_shift);
            lp->Slave.DioPortsDIODirection = sp->Slave.DioPortsDIODirection;
        break;

        default:
            fprintf(stderr,"Invalid Port %d\n",port);
        break;
    }
}

void
PortDirectionAll(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int output)
{
    int     lines;
    u_int   dio_out;
    u_char  pfi_out;
    if(output) {
        dio_out = 0xffffffff;
        pfi_out = 0x10;
    } else {
        dio_out = 0;
        pfi_out = 0;
    }

    lp->Master.DioPortsDIODirection = 
    sp->Master.DioPortsDIODirection = dio_out;     /* port 0..3 */
    lp->Slave.DioPortsDIODirection =
    sp->Slave.DioPortsDIODirection = dio_out;      /* port 8..11 */
    lp->Master.PFIDirection =
    sp->Master.PFIDirection = dio_out;             /* port 4..5 */
    lp->Slave.PFIDirection =
    sp->Slave.PFIDirection = dio_out;              /* port 6..7 */

    for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
        lp->Master.PFIOutputSelectPort0[lines] =
        sp->Master.PFIOutputSelectPort0[lines] = pfi_out; /* set port 4 as output */
        lp->Master.PFIOutputSelectPort1[lines] = 
        sp->Master.PFIOutputSelectPort1[lines] = pfi_out; /* set port 5 as output */
        lp->Slave.PFIOutputSelectPort0[lines]  = 
        sp->Slave.PFIOutputSelectPort0[lines]  = pfi_out; /* set port 6 as output */
        lp->Slave.PFIOutputSelectPort1[lines]  = 
        sp->Slave.PFIOutputSelectPort1[lines]  = pfi_out; /* set port 7 as output */
    }
}
