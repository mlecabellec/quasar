/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_dout.c                                             *
 *                                                                           *
 * Description:  Interactive Library Test                                    *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_dout           ==> start writing to digital output ports       *
 *                                  device 0.                                *
 *   pcie6509_dout -b <dn>   ==> start writing to digital output ports       *
 *                                  device <dn>. (where dn is device number) *
 *   pcie6509_dout -p <mask> ==> start writing to digital butput ports       *
 *                                  device 0, port mask = 'mask'             *
 *                                                                           *
 * Date:        02/13/2014                                                   *
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
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include "pcie6509_user.h"
#include "pcie6509_lib.h"

enum {
    PCIE6509_SET_DIGITALOUTPUTPMASK=0,
    PCIE6509_WRITE,
};
#define DEF_PORT_MASK   PCIE6509_ALL_PORTS_MASK
#define	DEF_BOARD_NO    0
#define DEF_DELAY       500000

#define DEF_WRITE_MODE   PCIE6509_SET_DIGITALOUTPUTPMASK   
                         /* 0 = PCIE6509_Set_DigitalOutputPmask()
                          * 1 = PCIE6509_Write()
                          */

#define READ_TSC(Time) clock_gettime(CLOCK_REALTIME, &Time)

#define DELTA_TIME(Value,TimeB,TimeA) {                         \
    if(TimeB.tv_nsec > TimeA.tv_nsec) {                         \
        Value = (double)((TimeB.tv_nsec - TimeA.tv_nsec) +      \
            ((TimeB.tv_sec - TimeA.tv_sec) * 1000000000))/1000; \
    } else {                                                    \
        Value = (double)((1000000000 - TimeA.tv_nsec +          \
        TimeB.tv_nsec) +                                        \
        ((TimeB.tv_sec - TimeA.tv_sec -1) * 1000000000))/1000;  \
    }                                                           \
        Value = Value;                                          \
    }

// Global variables
struct  timespec    start_time, end_time;
double              delta, min, max, ave;
pcie6509_driver_info_t info;
void                *MyHandle;
int                 pcie6509_break_received;
int                 status;
pcie6509_io_port_t  iop;
char	            devname[15];
u_int               delay = DEF_DELAY;
int                 loopback_mode=0;
int 	            bnp=DEF_BOARD_NO;
int                 reset_board=0;
int                 write_mode=DEF_WRITE_MODE;

/* prototype */
void    usage(char * prog_p);
int     Display_Driver_Error(void *MyHandle);
int     Display_Library_Error(void *MyHandle);

void    pcie6509_get_info();
void    pcie6509_initialize_board();
void    pcie6509_write_operation();
void    pcie6509_reset_board();

void
init_window(void)
{
    initscr();
    cbreak();
    noecho();
    nonl();
    timeout(1);
    clear();
    refresh();
}

void
delete_window(void)
{
    echo();
    nl();
    endwin();
}

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    extern  char    *optarg;
    extern  int     optind, opterr;
	int             status, option;
    char            *pt;
    char            *prog_p = argv[0];
    int             port;

    iop.port_mask = DEF_PORT_MASK;

    if(!bnp)    /* if no device number entered, use default */
        bnp = DEF_BOARD_NO;

    while ((option = getopt( argc, argv, "b:d:lm:p:R" )) != EOF) {
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

        case 'l': /* loopback mode */
            loopback_mode=1;
        break;

        case 'm': /* write mode */
            write_mode = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,
                    "%s: ERROR! Write Mode not specified\n",
                    prog_p);
                exit(1);
            }

            switch(write_mode) {
                case PCIE6509_SET_DIGITALOUTPUTPMASK:
                case PCIE6509_WRITE:
                break;
                default:
                    fprintf(stderr,
                        "%s: ERROR! Invalid Write Mode %d\n",
                        prog_p,write_mode);
                    exit(1);
                break;
            }
        break;

        case 'p':  /* port mask */
            iop.port_mask = strtoul(optarg,&pt, 16);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! port mask not specified\n",
                    prog_p);
                exit(1);
            }

            if(iop.port_mask & ~PCIE6509_ALL_PORTS_MASK) {
                fprintf(stderr,
                    "%s: ERROR! Invalid port mask 0x%x: max=0x%03x\n",
                    prog_p, iop.port_mask,PCIE6509_ALL_PORTS_MASK);
                exit(1);
            }

        break;

        case 'R': /* reset board */
            reset_board=1;
        break;

        default:
            usage(prog_p);
        }
    }

    /* if spureous arguments, error out */
    if(optind != argc) {
        usage(prog_p);
    }

    /* If loopback mode has been selected, we assume that the user has
     * connected one of CCUR loopback cables. In that case, we have the 
     * following connections:
     *
     *    P0->P6      P1->P7      P2->P8   
     *    P3->P9      P4->P10     P5->P11  
     *
     * We will therefore set the output ports to P0..P5 and input 
     * to P6..P11. Additionally, the output test delay will be increased to
     * "see" the changes on the input test.
     *
     * This test should be used in conjunction with the 'pcie6509_din' test.
     */

    if(loopback_mode) {
        iop.port_mask = PCIE6509_PORT_MASK_P0 | PCIE6509_PORT_MASK_P1 |
                        PCIE6509_PORT_MASK_P2 | PCIE6509_PORT_MASK_P3 |
                        PCIE6509_PORT_MASK_P4 | PCIE6509_PORT_MASK_P5;

    }

    MyHandle=NULL;

    /*** OPEN DEVICE ***/
    if((status=PCIE6509_Open(&MyHandle,bnp))) {
        fprintf(stderr,"pcie6509_Open Failed=%d\n",status);
        exit(1);
    }

    if(reset_board)
        pcie6509_reset_board();

    if((status=PCIE6509_Get_Info(MyHandle, &info))) {
        Display_Library_Error(MyHandle);
        exit(1);
    }

    /* set direction of all ports to output */
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop.line_mask[port] = 0xFF;
    }
   
    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        fprintf(stderr,"PCIE6509_Set_PortDirectionPmask Failed\n");
        Display_Library_Error(MyHandle);
        exit(1);
    }

    init_window();

    move(0,0);     /* position to the top */

    pcie6509_get_info();

    pcie6509_write_operation();

    delete_window();

    if (MyHandle) {
        PCIE6509_Close(MyHandle);
    }

    exit(0);
}

/******************************************************************************
 *** Usage                                                                  ***
 ******************************************************************************/
void
usage( char *prog_p) {
    fprintf(stderr, "usage: %s -[bdlmpR]\n",prog_p);
    fprintf(stderr, "         -b <board>       (default = %d)\n",
            DEF_BOARD_NO);
    fprintf(stderr, "         -d <delay>       (default = %d)\n",
            DEF_DELAY);
    fprintf(stderr, "         -l               (default = %s)\n",
            loopback_mode?"Loopback":"No-Loopback");
    fprintf(stderr, "         -m [0|1]             (write_mode default = 0)\n");
    fprintf(stderr, "            0=PCIE6509_SET_DIGITALOUTPUTPMASK\n");
    fprintf(stderr, "                              (PCIE6509_Set_DigitalOutputPmask())\n");
    fprintf(stderr, "            1=PCIE6509_WRITE  (PCIE6509_Write())\n");
    fprintf(stderr, "         -p <port mask>   (default = 0x%03x)\n",
            DEF_PORT_MASK);
    fprintf(stderr, "         -R                   (reset board)\n");
    exit(1);
}

/***********************************************************************/
int
Display_Driver_Error(void *MyHandle)
{
    int status;
    pcie6509_user_error_t   driver_error;

    /*** GET DRIVER ERROR ***/
    if((status=PCIE6509_Get_Driver_Error(MyHandle, &driver_error))) {
        fprintf(stderr,"PCIE6509_Get_Driver_Error Failed=%d\n",status);
        return(status);
    }

    fprintf(stderr,"===============================================\n");
    fprintf(stderr,"driver error information:\n");
    fprintf(stderr,"   error:   %d\n",driver_error.error);
    fprintf(stderr,"    name:   %s\n",driver_error.name);
    fprintf(stderr,"    desc:   %s\n",driver_error.desc);
    fprintf(stderr,"===============================================\n");

    return(driver_error.error);
}
/***********************************************************************/

/***********************************************************************/
int
Display_Library_Error(void *MyHandle)
{
    int status;
    pcie6509_lib_error_t    lib_error;

    /*** GET LIBRARY ERROR ***/
    status=PCIE6509_Get_Lib_Error(MyHandle,&lib_error);
    fprintf(stderr,"===============================================\n");
    fprintf(stderr,"library error information:\n");
    if(lib_error.error != PCIE6509_LIB_NO_ERROR) {
        fprintf(stderr,"lib function:   %s(line=%d)\n",
                lib_error.function, lib_error.line_number);
    }
    fprintf(stderr,"       error:   %d\n",lib_error.error);
    fprintf(stderr,"        name:   %s\n",lib_error.name);
    fprintf(stderr,"        desc:   %s\n",lib_error.desc);
    fprintf(stderr,"===============================================\n");

    return(lib_error.error);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_info()
{
    #define FMT "%17s: "
    int     i;
    char    str[15];

    printw("\n");
    printw(FMT " %s\n",     "Version",    info.version);
    printw(FMT " %s\n",     "Build",      info.built);
    printw(FMT " %s\n",     "Module",     info.module_name);
    printw(FMT " %d (%s)\n","Board Type", info.board_type, 
                                                  info.board_desc);
    printw(FMT " %d\n",     "Bus",        info.bus);
    printw(FMT " %d\n",     "Slot",       info.slot);
    printw(FMT " %d\n",     "Func",       info.func);
    printw(FMT " 0x%x\n",   "Vendor ID",  info.vendor_id);
    printw(FMT " 0x%x\n",   "Device ID",  info.device_id);
    printw(FMT " N%x\n",    "Device",     info.device);
    printw(FMT " 0x%x\n",   "Board ID",   info.board_id);
    printw(FMT " 0x%x\n",   "Master Signature",   info.MasterSignature);
    printw(FMT " 0x%x\n",   "Slave Signature",   info.SlaveSignature);
    printw(FMT " %ld\n",    "Interrupt.count", info.interrupt.count);
    printw(FMT " 0x%04x\n",    "Interrupt.mask", info.interrupt.mask);
    printw(FMT " 0x%04x\n",    "Interrupt.status", info.interrupt.status);
    printw(FMT " 0x%04x\n",    "Interrupt.pending", info.interrupt.pending);

    /* now, display memory regions */
    for(i=0; i < PCIE6509_MAX_REGION; i++) {
        if(info.mem_region[i].size) {
            sprintf(str,"Region%2d",i);
            printw(FMT " Addr=%-10p  Size=%-5d (0x%x)\n",str,
                        (char *)(long)info.mem_region[i].physical_address,
                        info.mem_region[i].size,
                        info.mem_region[i].size);
        }
    }

    printw("\nUser options:\n");
    printw(FMT " %d\n",     "Board Number", bnp);
    printw(FMT " /dev/pcie6509%d\n",  "Device Name", bnp);
    printw(FMT " %d (us)\n","Delay", delay);
    printw(FMT " %s\n",     "loopback", (loopback_mode==0)?"no":"yes");
    printw(FMT " 0x%03x\n", "Port Mask",iop.port_mask);
    printw(FMT " %s\n","Write Mode",(write_mode==PCIE6509_SET_DIGITALOUTPUTPMASK)?
                                 "PCIE6509_Set_DigitalOutputPmask()":
                                 "PCIE6509_Write()");
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_initialize_board()
{
    if((status=PCIE6509_Initialize_Board(MyHandle))) {
        Display_Library_Error(MyHandle);
    }
}
/***********************************************************************/


/***********************************************************************/
void
pcie6509_write_operation()
{
    pcie6509_io_port_t   rp;
    int               bytes_written;
    int               error;
    int               port;
    int               counter=0;
    int               signal;
    int               port_count;   
    u_char            line_mask;
    double            ave_sum=0;

    min=100000;
    max=0;

    while(1) {
        move(27,0);
        printw("\n%08d - duration (usec): %f (min=%f, max=%f, ave=%f)\n",
                    counter,delta,min,max,ave);
        bzero(&rp, sizeof(rp));
    
        rp.port_mask = iop.port_mask;

        for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
            rp.line_mask[port] = ((port+0) << 5) | (counter % 0x20); 
                                /* place port number + counter in each port */
        }

        READ_TSC(start_time);
        
        if(write_mode == PCIE6509_SET_DIGITALOUTPUTPMASK) {
            if((status=PCIE6509_Set_DigitalOutputPmask(MyHandle, &rp))) {
                printw("PCIE6509_Set_DigitalOutputPmask Failed\n");
                Display_Library_Error(MyHandle);
                return;
            }
            bytes_written=0;
            for(port = 0, port_count=0; port < PCIE6509_MAX_PORTS; port++) {
                if((rp.port_mask & (1 << port))) {      /* if port is selected */
                    bytes_written++;
                }
            }
        } else {
            if((status=PCIE6509_Write(MyHandle, &rp, sizeof(rp), &bytes_written, 
                                                                &error))) {
                printw("Write Failed: %s\n",strerror(error));
                Display_Library_Error(MyHandle);
                return;
            }
        }
        READ_TSC(end_time);
        DELTA_TIME(delta,end_time,start_time);
        if(counter != 0) {  /* skip first writing */
            if(delta < min) min=delta;
            if(delta > max) max=delta;
            ave_sum += delta;
            ave = ave_sum/counter;
        }
    
        printw("\nSelected Ports Directions [0=in,1=out]:");
        for(port = 0, port_count=0; port < PCIE6509_MAX_PORTS; port++) {
            if((rp.port_mask & (1 << port))) {      /* if port is selected */
                if(!(port_count % 4))
                    printw("\n");
                port_count++;
                PCIE6509_Get_PortDirection(MyHandle,port,&line_mask);
                printw("  P%02d: 0x%02x",port,line_mask);
            }
        }

        printw("\n\nbytes written=%d (number of ports written)\n",bytes_written);

        printw("\nSelected Ports Signals [0=low,1=high]:");
    
        for(port = 0, port_count=0; port < PCIE6509_MAX_PORTS; port++) {
            if((rp.port_mask & (1 << port))) {      /* if port is selected */
                if(!(port_count % 4))
                    printw("\n");
                port_count++;
                printw("  P%02d: 0x%02x",port,rp.line_mask[port]);
            }
        }
        printw("\n\n");

        /****/

        printw("Port    7     6     5     4     3     2     1     0\n");
        printw("----   ---   ---   ---   ---   ---   ---   ---   ---");

        for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
            if((rp.port_mask & (1 << port))) {      /* if port is selected */
                printw("\nP%-2d",port);

                for(signal = 7; signal >=0; signal --)
                    printw("     %d",(rp.line_mask[port] & (1 << signal)) ? 1:0);
            }
        }

        move (0,0);
        refresh();

        usleep(delay);
        counter++;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_reset_board()
{
    if((status=PCIE6509_Reset_Board(MyHandle))) {
        Display_Library_Error(MyHandle);
    }
}
/***********************************************************************/

