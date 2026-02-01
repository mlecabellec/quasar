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
 * File:         setchans.c                                                  *
 *                                                                           *
 * Description:  Set output of multiple channels one at a time               *
 *                                                                           *
 * Date:        11/17/2010                                                   *
 * History:                                                                  *
 *                                                                           *
 *   6 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   5 04/20/10 D. Dubash                                                    *
 *              Support for GSC16AO12 board.                                 *
 *                                                                           *
 *   4 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   3 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   2  8/24/05 D. Dubash                                                    *
 *              Added device '-d' argument                                   *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gsc16ao_lib.h"
#include "gsc16ao_ioctl.h"

#define USAGE \
"Usage: %s [-d <dn>]\n" \
"Desc:  Sets output of all channels one at a time\n" \
"Where:\n" \
"<dn>\t- Gsc16ao device number 0-9 [default:0]\n" 

void
usage(char *progname)
{
  fprintf(stderr, USAGE, progname);
  exit(1);
}

extern board_info_t board_info;

int
main(int argc, char **argv)
{
  GSCDEV_HANDLE handle;
  int   device  = 0;
  float volts;
  int   i;
  int   hw_nchans;
  int   hw_ChanMask;

  while ((i = getopt(argc, argv, "d:")) != EOF) {
    switch (i) {
    case 'd':
      device = atoi(optarg);
      if (device < 0 || device > 9)
	usage(argv[0]);
      break;
      
    default:
      usage(argv[0]);
    }
  }
  
  if (optind != argc)
    usage(argv[0]);

  /*
   * Open and initialize device
   */
  handle = gsc16ao_init_board(device);
  if (!handle) {
    fprintf(stderr, "*** Cannot initialize GSC16AO board %d ***\n", device);
    exit(1);
  }
    
  Get_Board_Info(handle, 1);

  if(board_info.board_type != GSC_16AO_16)
    printf("\nAssume that the board '%s' supports a voltage range of +/-10 Volts\n",board_info.board_name);

  hw_nchans = board_info.max_channels; /* assume max channels */

  if(board_info.board_type == GSC_16AO_16) {
      switch((board_info.firmware_ops & (3 << 16)) >> 16) {
          case 0:
            hw_nchans      = board_info.max_channels;
            hw_ChanMask    = 0;        /* reserved */
          break;
          case 1:
            hw_nchans      = 8;        /* 8 channels */
            hw_ChanMask    = 0x00ff;   /* 8 channels */
          break;
          case 2:
            hw_nchans      = 12;       /* 12 channels */
            hw_ChanMask    = 0x0fff;   /* 12 channels */
          break;
          case 3:
            hw_nchans      = 16;       /* 16 channels */
            hw_ChanMask    = 0xffff;   /* 16 channels */
          break;
      }
  }

  /*
   * Set output of each channel from 10v to -10v linearly across all channels
   */
  printf("Setting voltages from -10v to +10v across all %d channels\n",
                    hw_nchans);
  for (i=0; i< hw_nchans; i++) {
    volts = (20.0 * (i/(float)(hw_nchans -1))) - 10.0;
	if(!(i%4))
		printf("\n");
	printf("Ch%-2d=%+6.2f   ",i,volts);
    gsc16ao_set_chan(handle, i, volts);
  }

  printf("\n");
  
  gsc16ao_close_board(handle);

  return(0);
}
