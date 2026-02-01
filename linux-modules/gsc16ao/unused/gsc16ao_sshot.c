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
 * File:         gsc16ao_setchan.c                                           *
 *                                                                           *
 * Description:  Write all 16 channels and time results                      *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_sshot [-d <dn>]                                                 *
 *   Where:                                                                  *
 *      <dn>     - gsc16ao board number 0-9 [default:0]                      * 
 *                                                                           *
 * Date:        07/25/2012                                                   *
 * History:                                                                  *
 *                                                                           *
 *   6 07/25/12 D. Dubash                                                    *
 *              Fixed Read_TSC. Added mfence.                                *
 *                                                                           *
 *   5 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   4 04/20/10 D. Dubash                                                    *
 *              Support for GSC16AO12 board.                                 *
 *                                                                           *
 *   3 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   2 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   1  8/24/05 D. Dubash                                                    *
 *              Created                                                      *
 *                                                                           *
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "gsc16ao_ioctl.h"
#include "gsc16ao_lib.h"
#include "gsc16ao_regs.h"

extern board_info_t board_info;

#define USAGE \
"\nUsage: %s [-d <dn> -n<nchans>]\n" \
"Desc:  Write to 'n' channels and time results\n" \
"Where:\n" \
"<dn>     - Gsc16ao board number 0-9 [default:0]\n" \
"<nchans> - Gsc16ao12 number of channels 1-12 [default:12]\n" \
"         - Gsc16ao16 number of channels 1-16 [default:16]\n"

#define LOOP_CNT 5000000
double Init_TSC();
double Ticks;


/* support for 64 bit architecture */
#define Read_TSC(val) do { \
         unsigned int __a,__d; \
         asm volatile("mfence"); \
         asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
         (val) = ((unsigned long long)__a) | (((unsigned long long)__d)<<32); \
} while(0)

struct stats {
    float fire;
    float wait;
    float total;
};

struct stats cur;
struct stats min;
struct stats max;
struct stats sum;
int    hw_nchans;

unsigned short chan_mask[]={0x0001, 0x0003, 0x0007, 0x000f,
                            0x001f, 0x003f, 0x007f, 0x00ff,
                            0x01ff, 0x03ff, 0x07ff, 0x0fff,
                            0x1fff, 0x3fff, 0x7fff, 0xffff
};

void gsc16ao_ret_gscptr(GSCDEV_HANDLE devhndl, gsc16ao_gscregs **ptr);
void gsc16ao_ret_fd(GSCDEV_HANDLE devhndl, int *fd);

void usage(char *progname)
{
    fprintf(stderr, USAGE, progname);
    exit(1);
}

int main(int argc, char **argv)
{
    GSCDEV_HANDLE handle;
    unsigned long parm;
    int fd;
    int status;
    int board = 0;
    int nchans = 0;
    int i, loop;
    static long long r_tp_a;
    static long long r_tp_b;
    static long long r_tp_c;
    gsc16ao_gscregs *gscptr = NULL;

    min.fire = min.wait = min.total = 99999999;
    max.fire = max.wait = max.total = 0;
    sum.fire = sum.wait = sum.total = 0;

    while ((i = getopt(argc, argv, "d:n:")) != EOF) {
        switch (i) {
        case 'd':
            board = atoi(optarg);
            if (board < 0 || board > 9)
                usage(argv[0]);
            break;

        case 'n':
            nchans = atoi(optarg);
            if (nchans < 1 || nchans > GSC16AO_MAX_CHANNELS)
	            usage(argv[0]);
            break;

        default:
            usage(argv[0]);
        }
    }

    if (optind != argc)
        usage(argv[0]);

    Init_TSC(); /* get processor speed */

    /*
     * Open and initialize board
     */
    handle = gsc16ao_init_board(board);
    if (!handle) {
        fprintf(stderr, "*** Cannot initialize GSC16AO board %d ***\n",
                board);
        exit(1);
    }

    Get_Board_Info(handle, 1);

    hw_nchans = board_info.max_channels; /* assume max channels */
    if(board_info.board_type == GSC_16AO_16) {
        switch((board_info.firmware_ops & (3 << 16)) >> 16) {
            case 0:
              hw_nchans = board_info.max_channels; 
            break;
            case 1:
              hw_nchans = 8; 
            break;
            case 2:
              hw_nchans = 12; 
            break;
            case 3:
              hw_nchans = 16; 
            break;
        }
    }


    if(nchans > hw_nchans) {
        fprintf(stderr,
        "\nERROR!!! Channel Range is 0..%d (Max output chans=%d)\n\n",
                                        (hw_nchans-1),hw_nchans);
        usage(argv[0]);
    }

    if(nchans == 0) nchans = hw_nchans;

    gsc16ao_ret_gscptr(handle, &gscptr); /* get mapped register pointer */

    fprintf(stderr,"Device=%d, Number_of_Channels=%d\n",board,nchans);

    gscptr->channel_select = chan_mask[nchans-1]; /* select all 12 channels */
    gscptr->board_control |= BCR_SIMULTANEOUS_OUTPUTS; /* simultaneous sampling of channels */

    gsc16ao_ret_fd(handle, &fd);

    if(board_info.board_type == GSC_16AO_16) {
        parm = GSC16AO16_RANGE_10; /* +/- 10 Volts */

        status = ioctl(fd, IOCTL_GSC16AO_SELECT_OUTPUT_RANGE, &parm);
        if(status) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_OUTPUT_RANGE) failed\n");
        exit(1);
        }
    }

    for(loop=0; loop < LOOP_CNT; loop++) {

        Read_TSC(r_tp_a);   /* get ticks */

        /* write a value to all 12 channels */
        /* EOF flag is ignored in open-buffer mode */
        for(i=0; i < nchans; i++) {
            gscptr->output_data_buffer=(i*0x1000);
        }

        Read_TSC(r_tp_b);   /* get ticks */
      
        while(!(gscptr->buffer_ops & GSC16AO_OUTPUT_EMPTY));
    
        Read_TSC(r_tp_c);   /* get ticks */

        cur.fire = (float)(r_tp_b - r_tp_a)/Ticks;
        cur.wait = (float)(r_tp_c - r_tp_b)/Ticks;
        cur.total = (float)(r_tp_c - r_tp_a)/Ticks;

        sum.fire += cur.fire;
        sum.wait += cur.wait;
        sum.total += cur.total;
    
        if(cur.fire < min.fire) min.fire = cur.fire;
        if(cur.fire > max.fire) max.fire = cur.fire;

        if(cur.wait < min.wait) min.wait = cur.wait;
        if(cur.wait > max.wait) max.wait = cur.wait;

        if(cur.total < min.total) min.total = cur.total;
        if(cur.total > max.total) max.total = cur.total;

        if(!((loop+1) % 1000)) {
        fprintf(stderr,"### %6d: f=%.2f (%.2f/%.2f/%.2f) w=%.2f (%.2f/%.2f/%.2f) [t=%.2f (%.2f/%.2f/%.2f)]     \r",loop+1,
            cur.fire, min.fire, max.fire, sum.fire/(loop+1),
            cur.wait, min.wait, max.wait, sum.wait/(loop+1),
            cur.total, min.total,max.total, sum.total/(loop+1));
        }
    }

    fprintf(stderr,"\n");

    gsc16ao_close_board(handle);
    exit(0);
}

double Init_TSC()
{
    FILE *fp;
    char s[80];

    if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
        return (0.0);
    }

    Ticks = 0.0;
    while (fgets(s, sizeof(s), fp) != NULL) {
        if (strncmp(s, "cpu MHz", 7) == 0) {
            Ticks = atof(s + 11);
            break;
        }
    }

    (void) fclose(fp);

    return (Ticks);
}
