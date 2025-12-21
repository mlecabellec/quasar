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
 * Description:  Set single channel example                                  *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_set [-d <dn>] [-c <chan>] <volts>                               *
 *   Where:                                                                  *
 *      <dn>     - gsc16ao device number 0-9 [default:0]                     * 
 *      <chan>   - gsc16ao12 channel number 0-11 [default:0]                 * 
 *               - gsc16ao16 channel number 0-15 [default:0]                 * 
 *      <volts>  - Output voltage in the range [-10.0,10.0]                  * 
 *                                                                           *
 * Date:        11/17/2010                                                   *
 * History:                                                                  *
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
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gsc16ao_ioctl.h"
#include "gsc16ao_lib.h"

#define USAGE \
"Usage: %s [-d <dn>] [-c <chan>] [-v <volts>]\n" \
"Desc:  Sets voltage of single GSC16AO12 channel\n" \
"Where:\n" \
"<dn>   - Gsc16ao device number 0-9 [default:0]\n" \
"<chan> - Gsc16ao12 channel number 0-11 [default:0]\n" \
"       - Gsc16ao16 channel number 0-15 [default:0]\n" \
"<volts>\t- Output voltage in the range [-10.0,10.0]\n"

extern board_info_t board_info;
int  hw_nchans;

void
usage(char *progname)
{
  fprintf(stderr, USAGE, progname);
  exit(1);
}


int
main(int argc, char **argv)
{
  GSCDEV_HANDLE handle;
  int   device  = 0;
  int   channel = 0;
  float volts   = 5.0;
  int   i;

  while ((i = getopt(argc, argv, "d:c:v:")) != EOF) {
    switch (i) {
    case 'd':
      device = atoi(optarg);
      if (device < 0 || device > 9)
	usage(argv[0]);
      break;
      
    case 'c':
      channel = atoi(optarg);
      if (channel < 0 || channel > 16)
	usage(argv[0]);
      break;

    case 'v':
      volts = atof(optarg);
      if (volts < -10.0 || volts > 10.0)
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

  hw_nchans = board_info.max_channels; /* assume max channels */
  if(board_info.board_type == GSC_16AO_16) {
      char *nchans;
        
      switch((board_info.firmware_ops & (3 << 16)) >> 16) {
          case 0:
            nchans = "(reserved)";
            hw_nchans = board_info.max_channels; 
          break;
          case 1:
            nchans = "8";
            hw_nchans = 8; 
          break;
          case 2:
            nchans = "12";
            hw_nchans = 12; 
          break;
          case 3:
            nchans = "16";
            hw_nchans = 16; 
          break;
      }
  }

  if(channel >= hw_nchans) {
	fprintf(stderr,
	"\nERROR!!! Channel Range is 0..%d (Max output chans=%d)\n\n",
					(hw_nchans-1),hw_nchans);
	usage(argv[0]);
  }

  if(board_info.board_type != GSC_16AO_16)
    printf("\nAssume that the board '%s' supports a voltage range of +/-10 Volts\n",board_info.board_name);

  printf("Setting output voltage on channel %d to %f volts\n",channel,volts);
  /*
   * Set output voltage of the requested channel
   */
  gsc16ao_set_chan(handle, channel, volts);

  gsc16ao_close_board(handle);
  exit(0);
}
