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
 *  Copyright (C) 2003 and beyond Concurrent Computer Corporation            *
 *  All rights reserved                                                      *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_pbit.c                                              *
 *                                                                           *
 * Description:  Analog Output Test                                          *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_pbit          ==> start non-interactive analog output test on   *
 *                             on device 0.                                  *
 *   gsc16ao_pbit <dn>     ==> start non-interactive analog output test on   *
 *                             on device <dn>. (where dn range is 0 - 9)     *
 *                                                                           *
 * Date:        11/16/2006                                                   *
 * History:                                                                  *
 *                                                                           *
 *   2 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   1 11/18/05 D. Dubash                                                    *
 *              Created an PBIT test.                                        *
 *                                                                           *
 *****************************************************************************/

/*
 * Headers
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>

#include "gsc16ao_ioctl.h"

#define DEVICE_NAME                 "gsc16ao"
#define DEF_BOARD_NO                0

#define PATTERN                     0x1234

/*** global variables ***/
char			buf[200];
char			devname[30];
char            *testname;
int			    c;
int			    fp, status;
int             board;
gsc16ao_gscregs	*gscptr;
int             *munmap_gscptr;

enum {
    PASSED=0,
    FAILED
};

void    Abort(int reason);
void    Usage();
void    Initialize_Board();

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    extern  char    *optarg;
    extern  int     optind, opterr;
    char            *pt;
    int             option;
    int             read_pattern;

    testname        = argv[0];

    board           = DEF_BOARD_NO;
    fp              = 0;

    while ((option = getopt( argc, argv, "b:" )) != EOF) {
        switch (option) {

        case 'b':  /* board */      
            board = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! board number not specified\n",
                    testname);
                Usage(testname);
            }
            if((board < 0) || (board > 9)) {
                fprintf(stderr,"%s: ERROR! Invalid board number %d\n",
                    testname, board);
                Usage(testname);
            }
        break;

        default:
            Usage(testname);
        break;

        }
    }

    /* if spureous arguments, error out */
    if(optind != argc) {
        Usage(testname);
    }

    sprintf(devname,"/dev/%s%d",DEVICE_NAME, board);

	Initialize_Board();

    gscptr->sample_rate = PATTERN;          /* write pattern */
    read_pattern = gscptr->sample_rate;     /* read pattern */
    if(read_pattern != PATTERN) {
        fprintf(stderr,"Data Mismatch: expected 0x%x received=0x%x\n",
                    PATTERN, read_pattern);
        Abort(FAILED);
    }

    Abort(PASSED);
    return(0);
}


/*** Initialize Board Routine ***/
void
Initialize_Board()
{
    unsigned long int offset;

    /*** Open the device ***/
    fp	= open(devname, O_RDWR);
    if (fp == -1) {
      printf(	"open() failure on %s: [%s]\n",devname, strerror(errno));
      Abort(FAILED);
    }
    
    /*** Map GSC GSC16AO CONTROL AND STATUS REGISTERS ***/
    gscptr = (gsc16ao_gscregs *) mmap((caddr_t)0 ,GSC16AO_GSC_REGS_MMAP_SIZE, 
			                           PROT_READ|PROT_WRITE, MAP_SHARED, 
                                        fp, GSC16AO_GSC_REGS_MMAP_OFFSET);
    
    munmap_gscptr = (int *)gscptr;
    if(gscptr == MAP_FAILED) {
      printf("GSC mmap() failure on %s: [%s]\n",devname,strerror(errno));
      Abort(FAILED);
    }

    offset = GSC16AO_GSC_REGS_MMAP_OFFSET;
    if(ioctl(fp, IOCTL_GSC16AO_GET_OFFSET,&offset)) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_OFFSET) Failed: %s\n",
            strerror(errno));
        exit(1);
    }

    gscptr = (gsc16ao_gscregs *)((char *)gscptr + offset);
}

int
Reset_Board()
{
    int status;

    /*** Get the board into a known state ***/
    status = ioctl(fp, IOCTL_GSC16AO_INIT_BOARD, NULL);
    if(status) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_INIT_BOARD) failed: [%s]\n",
	        strerror(errno));
    }

    return(status);
}

void
Abort(int reason)
{
    char    info[50];

    sprintf(info,"%s: %s (board=%d)",basename(testname), 
                    basename(devname), board);
	if(fp > 0) {
        if(Reset_Board())
            reason = FAILED;
		/*** unmap GSC area ***/
        if(munmap_gscptr)
		    munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
        close(fp);
	}
    if(reason == PASSED)
        fprintf(stderr,"%s: ==== PBIT TEST PASSED ====\n",info);
    else
        fprintf(stderr,"%s: #### ERROR!!! PBIT TEST FAILED ####\n",info);
      
    exit(reason);
}

void
Usage()
{
    fprintf(stderr, "Usage: %s -[b]\n", testname);
    
    fprintf(stderr,"      -b <board number>    [0 - 9]\n");

    Abort(FAILED);
}
