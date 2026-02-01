/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_dio.c                                              *
 *                                                                           *
 * Description:  Interactive Library Test                                    *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_dio           ==> start display of digital inputs on           *
 *                                  device 0.                                *
 *   pcie6509_dio -b <dn>   ==> start display of digital inputs on           *
 *                                  device <dn>. (where dn is device number) *
 *   pcie6509_dio -p <mask> ==> start display of digital inputs on           *
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
#include "pcie6509_user.h"
#include "pcie6509_lib.h"

#define DEF_PORT_MASK   PCIE6509_ALL_PORTS_MASK
#define DEF_LINE_MASK   PCIE6509_ALL_LINES_MASK
#define	DEF_BOARD_NO    0

// Global variables
pcie6509_driver_info_t info;
void                *MyHandle;
int                 pcie6509_break_received;
int                 status;
u_short             line_mask=DEF_LINE_MASK;
pcie6509_io_port_t  iop;
char	            devname[15];
int 	            bnp=DEF_BOARD_NO;

/* prototype */
void    usage(char * prog_p);
int     Display_Library_Error(void *MyHandle);

void    pcie6509_get_info();

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
    int             direction=PCIE6509_PORT_INPUT;
    u_char          all_direction;
    int             port;
    int             bytes_returned;
    int             error;
    int             signal;
    char            string_format[10];
    int             myport=0, myline=0;
    pcie6509_io_port_t rp;

    iop.port_mask = DEF_PORT_MASK;

    string_format[0] = 0;

    if(!bnp)    /* if no device number entered, use default */
        bnp = DEF_BOARD_NO;

    while ((option = getopt( argc, argv, "b:il:op:P:" )) != EOF) {
        switch (option) {
        case 'b':  /* board */
            bnp = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! board number not specified\n",
                    prog_p);
                exit(1);
            }
        break;

        case 'i':  /* input */
            direction = PCIE6509_PORT_INPUT;
        break;

        case 'l':  /* line mask */
            line_mask = strtoul(optarg,&pt, 16);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! line mask not specified\n",
                    prog_p);
                exit(1);
            }

            if(line_mask & ~PCIE6509_ALL_LINES_MASK) {
                fprintf(stderr,
                    "%s: ERROR! Invalid line mask 0x%x: max=0x%03x\n",
                    prog_p, iop.port_mask,PCIE6509_ALL_LINES_MASK);
                exit(1);
            }

        break;

        case 'o':  /* output */
            direction = PCIE6509_PORT_OUTPUT;
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

        case 'P':  /* port string -Pnn.m format (nn=port 0..11,m=line 0..7) */
            strcpy(string_format,optarg);

        break;

        default:
            usage(prog_p);
        }
    }

    /* if spureous arguments, error out */
    if(optind != argc) {
        usage(prog_p);
    }

    if(string_format[0]) {
        myport = strtol(string_format,&pt,10);
        if(string_format == pt) {
            fprintf(stderr,"%s: ERROR! port number not specified\n",
                prog_p);
            exit(1);
        }
        if((myport < 0) || (myport > 11)) {
            fprintf(stderr,"%s: ERROR! port number not in range 0..11\n",
                prog_p);
            exit(1);
        }
        iop.port_mask = 0;
        PCIE6509_PORT_SET(iop.port_mask,myport);

        if(pt[0] != '.') {
            fprintf(stderr,"%s: ERROR! 'P' format must be -Pnn.m[sr]\n",
                prog_p);
            fprintf(stderr,"    Where: nn=port 0..11\n");
            fprintf(stderr,"         :  m=line 0..7\n");
            fprintf(stderr,"         :  s=set line\n");
            fprintf(stderr,"         :  r=reset line\n");
            exit(1);
        }
        pt++;

        myline = strtol(pt,NULL,10);
        if((myline < 0) || (myline > 7)) {
            fprintf(stderr,"%s: ERROR! line number not in range 0..7\n",
                prog_p);
            exit(1);
        }

        if(direction == PCIE6509_PORT_OUTPUT) {
            pt++;
            line_mask = 0;
            if(*pt == 's')
                PCIE6509_LINE_SET(line_mask,myline);
            else if (*pt != 'r')  {
                fprintf(stderr,"%s: ERROR! 'P' format must be -Pnn.m[sr]\n",
                    prog_p);
                fprintf(stderr,"    Where: nn=port 0..11\n");
                fprintf(stderr,"         :  m=line 0..7\n");
                fprintf(stderr,"         :  s=set line\n");
                fprintf(stderr,"         :  r=reset line\n");
                exit(1);
            }
         }
    }
 
    MyHandle=NULL;

    /*** OPEN DEVICE ***/
    if((status=PCIE6509_Open(&MyHandle,bnp))) {
        fprintf(stderr,"pcie6509_Open Failed=%d\n",status);
        exit(1);
    }

    all_direction = (direction==PCIE6509_PORT_OUTPUT)?0xFF:0x00;

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop.line_mask[port] = all_direction;
    }

    /* set direction of all ports */
    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        fprintf(stderr,"PCIE6509_Set_PortDirectionPmask Failed\n");
        Display_Library_Error(MyHandle);
        exit(1);
    }

    switch(direction) {
        case PCIE6509_PORT_INPUT:
            bzero(&rp,sizeof(rp));
            rp.port_mask = iop.port_mask;

again:
            if((status=PCIE6509_Read(MyHandle, &rp, sizeof(rp), &bytes_returned,
                                                                &error))) {
                if(error == EBUSY)
                    goto again;
                fprintf(stderr,"Read Failed: %s\n",strerror(error));
                Display_Library_Error(MyHandle);
                exit(1);
            }

            if(string_format[0]) {
                fprintf(stderr,"P%d.%d = %d\n",
                        myport,myline,PCIE6509_LINE_GET(rp.line_mask[myport],myline));
                break;
            }

            fprintf(stderr,"Ports Read.....\n");
            fprintf(stderr,"Port    7     6     5     4     3     2     1     0\n");
            fprintf(stderr,"----   ---   ---   ---   ---   ---   ---   ---   ---");

            for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
                if((rp.port_mask & (1 << port))) {      /* if port is selected */
                fprintf(stderr,"\nP%-2d",port);

                    for(signal = 7; signal >=0; signal --)
                        fprintf(stderr,"     %d",
                                        (rp.line_mask[port] & (1 << signal)) ? 1:0);
                }
            }
            fprintf(stderr,"\n");

        break;

        case PCIE6509_PORT_OUTPUT:
            bzero(&rp,sizeof(rp));
            rp.port_mask = iop.port_mask;
            for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
                if(PCIE6509_PORT_GET(iop.port_mask,port)) {
                    rp.line_mask[port] = line_mask;
                }
            }

            if((status=PCIE6509_Write(MyHandle, &rp, sizeof(rp), &bytes_returned,
                                                                &error))) {
                fprintf(stderr,"Write Failed: %s\n",strerror(error));
                Display_Library_Error(MyHandle);
                exit(1);
            }

            if(string_format[0]) {
                fprintf(stderr,"P%d.%d = %d\n",
                        myport,myline,PCIE6509_LINE_GET(rp.line_mask[myport],myline));
                break;
            }

            fprintf(stderr,"Ports Written.....\n");
            fprintf(stderr,"Port    7     6     5     4     3     2     1     0\n");
            fprintf(stderr,"----   ---   ---   ---   ---   ---   ---   ---   ---");

            for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
                if((rp.port_mask & (1 << port))) {      /* if port is selected */
                fprintf(stderr,"\nP%-2d",port);

                    for(signal = 7; signal >=0; signal --)
                        fprintf(stderr,"     %d",
                                        (rp.line_mask[port] & (1 << signal)) ? 1:0);
                }
            }
            fprintf(stderr,"\n");

        break;
    }

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
    fprintf(stderr, "usage: %s -[bilopP]\n",prog_p);
    fprintf(stderr, "         -b <board>       (default = %d)\n",
            DEF_BOARD_NO);
    fprintf(stderr, "         -i               (set to input direction)\n");
    fprintf(stderr, "         -l <line mask>   (default = 0x%02x)\n",
            DEF_LINE_MASK);
    fprintf(stderr, "         -o               (set to output direction)\n");
    fprintf(stderr, "         -p <port mask>   (default = 0x%03x)\n",
            DEF_PORT_MASK);
    fprintf(stderr, "         -P <nn.m>        (input format: '-Pnn.m')\n");
    fprintf(stderr, "             nn=port 0..11\n");
    fprintf(stderr, "             m =line 0..7\n");
    fprintf(stderr, "         -P <nn.m[sr]     (output format: '-Pnn.m[sr]')\n");
    fprintf(stderr, "             nn=port 0..11\n");
    fprintf(stderr, "             m =line 0..7\n");
    fprintf(stderr, "             s =set line\n");
    fprintf(stderr, "             r =resetset line\n");
    fprintf(stderr, " e.g. -o -P6.3s = set line 3 of port 6\n");
    fprintf(stderr, "      -i -P0.3  = read line 3 of port 0\n");
    fprintf(stderr, "      -o -p0x910 -l0x22 = lines: set 1,5 reset 0,2-4,6-7\n");
    fprintf(stderr, "                          of ports 4,8,11\n");
    fprintf(stderr, "      -i -p0x424 -l0xff = lines: reads 0-7\n");
    fprintf(stderr, "                          of ports 2,5,10\n");
    exit(1);
}

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
