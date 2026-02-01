/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_din.c                                              *
 *                                                                           *
 * Description:  Interactive Library Test                                    *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_din           ==> start display of digital inputs on           *
 *                                  device 0.                                *
 *   pcie6509_din -b <dn>   ==> start display of digital inputs on           *
 *                                  device <dn>. (where dn is device number) *
 *   pcie6509_din -p <mask> ==> start display of digital inputs on           *
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
    PCIE6509_GET_DIGITALINPUTPMASK=0,
    PCIE6509_READ,
};
#define DEF_PORT_MASK       PCIE6509_ALL_PORTS_MASK
#define    DEF_BOARD_NO     0
#define DEF_DELAY           1000
#define DEF_FILTER_OPTION   PCIE6509_FILTER_NONE
#define DEF_READ_MODE       PCIE6509_GET_DIGITALINPUTPMASK   
                                /* 0 = PCIE6509_Get_DigitalInputPmask()
                                 * 1 = PCIE6509_Read()
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
pcie6509_io_port_t  iop_jitter;
char                devname[15];
u_int               delay = DEF_DELAY; 
int                 loopback_mode=0;
int                 bnp=DEF_BOARD_NO;
double              ret_interval;
pcie6509_filter_option_t  filter_option=DEF_FILTER_OPTION;
int                 reset_board=0;
int                 read_mode=DEF_READ_MODE;

/* prototype */
void    usage(char * prog_p);
int     Display_Driver_Error(void *MyHandle);
int     Display_Library_Error(void *MyHandle);

void    pcie6509_get_info();
void    pcie6509_initialize_board();
void    pcie6509_read_operation();
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

    while ((option = getopt( argc, argv, "b:d:f:lm:p:R" )) != EOF) {
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

        case 'f': /* filter option */
            if(strcmp(optarg,"n")==0)
                filter_option=PCIE6509_FILTER_NONE;
            else if(strcmp(optarg,"s")==0)
                filter_option=PCIE6509_FILTER_SMALL;
            else if(strcmp(optarg,"m")==0)
                filter_option=PCIE6509_FILTER_MEDIUM;
            else if(strcmp(optarg,"l")==0)
                filter_option=PCIE6509_FILTER_LARGE;
            else {
                fprintf(stderr,
                    "%s: ERROR! Filter Option [n|s|m|l] not specified\n",
                    prog_p);
                exit(1);
            }
        break;


        case 'l': /* loopback mode */
            loopback_mode=1;
        break;

        case 'm': /* read mode */
            read_mode = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,
                    "%s: ERROR! Read Mode not specified\n",
                    prog_p);
                exit(1);
            }

            switch(read_mode) {
                case PCIE6509_GET_DIGITALINPUTPMASK:
                case PCIE6509_READ:
                break;
                default:
                    fprintf(stderr,
                        "%s: ERROR! Invalid Read Mode %d\n",
                        prog_p,read_mode);
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
     * This test should be used in conjunction with the 'pcie6509_dout' test.
     */

    if(loopback_mode) {
        iop.port_mask = PCIE6509_PORT_MASK_P6 | PCIE6509_PORT_MASK_P7 |
                        PCIE6509_PORT_MASK_P8 | PCIE6509_PORT_MASK_P9 |
                        PCIE6509_PORT_MASK_P10 | PCIE6509_PORT_MASK_P11;
    }

    MyHandle=NULL;

    /*** OPEN DEVICE ***/
    if((status=PCIE6509_Open(&MyHandle,bnp))) {
        fprintf(stderr,"pcie6509_Open Failed=%d\n",status);
        exit(1);
    }

    if(reset_board)
        pcie6509_reset_board();

    iop_jitter.port_mask = PCIE6509_ALL_PORTS_MASK;
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop_jitter.line_mask[port] = PCIE6509_ALL_LINES_MASK;
    }

    /* set filter option to remove jitter from open lines */
    if(PCIE6509_Set_FilterPmask(MyHandle, &iop_jitter, filter_option)) {
        Display_Library_Error(MyHandle);
        exit(1);
    }

    if((status=PCIE6509_Get_Info(MyHandle, &info))) {
        Display_Library_Error(MyHandle);
        exit(1);
    }

    /* set direction of all ports to input */
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop.line_mask[port] = 0x00;
    }
   
    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        fprintf(stderr,"PCIE6509_Set_PortDirectionPmask Failed\n");
        Display_Library_Error(MyHandle);
        exit(1);
    }

    init_window();

    move(0,0);     /* position to the top */

    pcie6509_get_info();

    pcie6509_read_operation();

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
    fprintf(stderr, "usage: %s -[bdflmpR]\n",prog_p);
    fprintf(stderr, "         -b <board>           (default = %d)\n",
            DEF_BOARD_NO);
    fprintf(stderr, "         -d <delay>           (default = %d microsecs)\n",
            DEF_DELAY);
    fprintf(stderr, "         -f [n|s|m|l]         (default = NONE)\n");
    fprintf(stderr, "            n=NONE            (no filter)\n");
    fprintf(stderr, "            s=SMALL           (less than 100ns)\n");
    fprintf(stderr, "            m=MEDIUM          (less than 6.4us)\n");
    fprintf(stderr, "            l=LARGE           (less than 2.54ms)\n");
    fprintf(stderr, "         -l                   (default = %s)\n",
            loopback_mode?"Loopback":"No-Loopback");
    fprintf(stderr, "         -m [0|1]             (read_mode default = 0)\n");
    fprintf(stderr, "            0=PCIE6509_GET_DIGITALINPUTPMASK\n");
    fprintf(stderr, "                              (PCIE6509_Get_DigitalInputPmask())\n");
    fprintf(stderr, "            1=PCIE6509_READ   (PCIE6509_Read())\n");
    fprintf(stderr, "         -p <port mask>       (default = 0x%03x)\n",
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
    char    str[17];
    char    foc[30];

    switch(filter_option) {
        case PCIE6509_FILTER_NONE:
            strcpy(foc,"NONE");
        break;
        case PCIE6509_FILTER_SMALL:
            strcpy(foc,"SMALL (less than 100ns)");
        break;
        case PCIE6509_FILTER_MEDIUM:
            strcpy(foc,"MEDIUM (less than 6.4us)");
        break;
        case PCIE6509_FILTER_LARGE:
            strcpy(foc,"LARGE (less than 2.54ms)");
        break;
    }

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
    printw(FMT " %s\n",     "Filter Option",foc);

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
    printw(FMT " %s\n","Read Mode",(read_mode==PCIE6509_GET_DIGITALINPUTPMASK)?
                                 "PCIE6509_Get_DigitalInputPmask()":
                                 "PCIE6509_Read()");
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
pcie6509_read_operation()
{
    pcie6509_io_port_t  rp;
    int                 bytes_read;
    int                 error;
    int                 port;
    int                 counter=0;
    int                 signal;
    int                 port_count;
    u_char              line_mask;
    double              ave_sum=0;
    
    min=100000;
    max=0;

    while(1) {
        move(28,0);
        printw("\n%08d - duration (usec): %f (min=%f, max=%f, ave=%f)\n",
                    counter++,delta,min,max,ave);
        bzero(&rp, sizeof(rp));
    
        rp.port_mask = iop.port_mask;
again:
        READ_TSC(start_time);
        
        if(read_mode == PCIE6509_GET_DIGITALINPUTPMASK) {
            if((status=PCIE6509_Get_DigitalInputPmask(MyHandle, &rp))) {
                printw("PCIE6509_Get_DigitalInputPmask Failed\n");
                Display_Library_Error(MyHandle);
                return;
            }
            bytes_read=0;
            for(port = 0, port_count=0; port < PCIE6509_MAX_PORTS; port++) {
                if((rp.port_mask & (1 << port))) {      /* if port is selected */
                    bytes_read++;
                }
            }
        } else {
            if((status=PCIE6509_Read(MyHandle, &rp, sizeof(rp), &bytes_read, 
                                                                &error))) {
                if(error == EBUSY)
                    goto again;
                printw("Read Failed: %s\n",strerror(error));
                Display_Library_Error(MyHandle);
                return;
            }
        }
        READ_TSC(end_time);
        DELTA_TIME(delta,end_time,start_time);
        if(counter != 1) {  /* skip first reading */
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
    
        printw("\n\nbytes read=%d (number of ports read)\n",bytes_read);
    
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

