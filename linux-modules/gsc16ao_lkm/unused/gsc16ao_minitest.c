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
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 * File:         minitest.c                                                  *
 *                                                                           *
 * Description:  GSC16AO Mini Test                                           *
 *                                                                           *
 * Syntax:                                                                   *
 *   minitest      ==> start interactive analog output test on device 0.     *
 *   minitest <dn> ==> start interactive analog output test on device <dn>   *
 *                      (where dn range is 0 - 9)                            *
 *                                                                           *
 * Date:        11/17/2010                                                   *
 * History:                                                                  *
 *                                                                           *
 *   5 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   4 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   3 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   2  8/20/03 G. Barton                                                    *
 *              Add WAIT_FOR_INTERRUPT ioctl support                         *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "gsc16ao_ioctl.h"

void BadArg(char *arg);
void Get_Board_Info(int disp);

char driver_name[]="/dev/gsc16ao0";
board_info_t    board_info;
int             hw_nchans;

#define WORDS_PER_WRITE    4096
#define NUMBER_OF_WRITES   8

/* text names to describe each type of board supported. */
char *boards_supported[] = {
	"GSC16AO16",
	"GSC16AO12",
	"GSC16AO2",
	"NULL" 
};

/* number of channels for each board supported. */

int channels[] = {
	16,
	12,
	12
};

int fd;

int
main(int argc, char *argv[])
{
  struct register_params rw_q;
  unsigned int buffer[32768];
  int res, i;
  long int board_type;
  unsigned long param;
  unsigned long ulVal;

  if(argc == 2) {
    if(argv[1][0] < '0' || argv[1][0] > '9')
      BadArg(argv[1]);
    if(strlen(argv[1]) > 1)
      BadArg(argv[1]);
    driver_name[12]= argv[1][0];
  } 

  printf("about to open %s\n",driver_name);

  fd = open(driver_name, O_RDWR);
  if (fd < 0) {
    printf("%s: can not open device %s\n", argv[0], driver_name);
    return (1);
  }

  Get_Board_Info(1);

  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_GET_DEVICE_TYPE, &board_type);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_GET_DEVICE_TYPE failed\n", argv[0]);
	  return (1);
  }
  printf("open OK - board type: %s channels: %d\n", boards_supported[board_type],hw_nchans);

  rw_q.regset = GSC16AO_GSC_REGISTER;
  rw_q.regnum = GSC16AO_GSC_BCR;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_READ_REGISTER, &rw_q);
  if (res < 0) {
    printf("%s: ioctl IOCTL_GSC16AO_READ_REGISTER failed\n", argv[0]);
    return (1);
  }
	printf("before reset: BCR is 0x%lx\n", rw_q.regval);
	res = ioctl(fd, IOCTL_GSC16AO_INIT_BOARD, NULL);
	if (res < 0) {
		printf("%s: ioctl IOCTL_GSC16AO_INIT_BOARD failed\n", argv[0]);
		return (1);
	}
	printf("board reset OK\n");

  rw_q.regset = GSC16AO_GSC_REGISTER;
  rw_q.regnum = GSC16AO_GSC_BCR;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_READ_REGISTER, &rw_q);
  if (res < 0) {
    printf("%s: ioctl IOCTL_GSC16AO_READ_REGISTER failed\n", argv[0]);
    return (1);
  }
  printf("after reset: BCR is 0x%lx\n", rw_q.regval);

  printf("Auto Calibration Initiated....please wait (3-5 seconds)\n");
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_AUTO_CAL, NULL);
  if (res < 0) {
    printf("%s: ioctl IOCTL_GSC16AO_AUTO_CAL failed\n", argv[0]);
    return (1);
  }
  printf("Auto Calibration OK\n");

  /*
  *	set clock for 1 kHz.
  */
  param = board_info.master_clock/1000;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_PROGRAM_RATE_GEN, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_PROGRAM_RATE_GEN failed\n", argv[0]);
	  return (1);
  }

  /*
  *	two's comp data format.
  */
  param = GSC16AO_TWOS_COMP;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_SELECT_DATA_FORMAT, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_SELECT_DATA_FORMAT failed\n", argv[0]);
	  return (1);
  }

  /*
   *	simultaneous clocking.
   */
  param = SIMULTANEOUS;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE failed\n", argv[0]);
	  return (1);
  }

  /*
   *	continuous output.
   */
  param = GSC16AO_CONT_MODE;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_SELECT_SAMPLING_MODE, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_SELECT_SAMPLING_MODE failed\n", argv[0]);
	  return (1);
  }
  /*
  *	internal clock.
  */
  param = INTERNAL;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_SELECT_CLK_SOURCE, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_SELECT_CLK_SOURCE failed\n", argv[0]);
	  return (1);
  }

  /*
   *	create a buffer of data.
   */
  for (i=0;i<WORDS_PER_WRITE;i++)
  {
	  buffer[i]=i;
  }

  buffer[WORDS_PER_WRITE-1]|=0x10000; // mark end of buffer.

  /*
   * write a buffer with PIO.  The driver 'knows' to wait for
   * the buffer to be ready.
   */
  printf("before PIO write\n");
  for (i=0;i<NUMBER_OF_WRITES;i++)
  {
	  res = write(fd, buffer, WORDS_PER_WRITE*4);
	  if (res < 0){
		  printf("read error -> after write...res = %d [%s]\n",res,
                strerror(errno));
		  return (1);
	  }
  }

  printf("after PIO write..enabling clock...\n");

  /*
  *	enable clock.
  */
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_ENABLE_CLK, NULL);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_ENABLE_CLK failed\n", argv[0]);
	  return (1);
  }

  printf("waiting for output to drain...");

  /*
   * Wait for output buffer to drain
   */
  ulVal = GSC16AO_BCR_IRQ_OUT_BUFFER_EMPTY;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_WAIT_FOR_INTERRUPT, &ulVal);
  if (res < 0) {
	  printf("ERROR\n");
	  printf("%s: ioctl IOCTL_GSC16AO_WAIT_FOR_INTERRUPT failed\n", argv[0]);
	  return (1);
  }
  
  printf("done\n");
  
  /*
   *	disable clock.
   */
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_DISABLE_CLK);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_DISABLE_CLK failed\n", argv[0]);
	  return (1);
  }

  /*
  * write a buffer with DMA.  The driver 'knows' to wait for 
  * the buffer ready.
  */
  param = GSC16AO_DMA_MODE;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_SET_WRITE_MODE, &param);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_SET_WRITE_MODE failed\n", argv[0]);
	  return (1);
  }

  printf("before DMA write\n");
  for (i=0;i<NUMBER_OF_WRITES;i++)
  {
	  res = write(fd, buffer, WORDS_PER_WRITE*4);
	  if (res < 0){
		  printf("read error -> after write...res = %d [%s]\n",res,
                strerror(errno));
		  return (1);
	  }
  }
  printf("after DMA write..enabling clock...\n");

  /*
   *	enable clock.
   */
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_ENABLE_CLK);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_ENABLE_CLK failed\n", argv[0]);
	  return (1);
  }

  printf("waiting for output to drain...");

  /*
   * Wait for output buffer to drain
   */
  ulVal = GSC16AO_BCR_IRQ_OUT_BUFFER_EMPTY;
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_WAIT_FOR_INTERRUPT, &ulVal);
  if (res < 0) {
          printf("ERROR\n");
          printf("%s: ioctl IOCTL_GSC16AO_WAIT_FOR_INTERRUPT failed\n", argv[0]);
	  return (1);
  }
  
  printf("done\n");
  
  /*
   *	disable clock.
   */
  res = ioctl(fd, (unsigned long)IOCTL_GSC16AO_DISABLE_CLK);
  if (res < 0) {
	  printf("%s: ioctl IOCTL_GSC16AO_DISABLE_CLK failed\n", argv[0]);
	  return (1);
  }

  printf("Minitest completed\n");
  close(fd);
  return 0;
}

/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
	fprintf(stderr,"\n*** Invalid Argument [%s] ***\n",arg);
	fprintf(stderr,"Usage: gsc16ao_minitest <device_num 0-9>\n");
	exit(1);
}

void 
Get_Board_Info(int disp)
{
    int max_chan_mask;
    if(ioctl(fd, IOCTL_GSC16AO_GET_BOARD_INFO,&board_info)) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_BOARD_INFO) Failed: %s\n",
            strerror(errno));
        exit(1);
    }
   

	if(disp) {
    	fprintf(stderr,"\n================ board info ================\n");
    	fprintf(stderr,"   %-25s :%d\n","max_channels",board_info.max_channels);
    	fprintf(stderr,"   %-25s :%d (%.3f)\n","master_clock", 
                    board_info.master_clock, board_info.dbl_master_clock);
    	fprintf(stderr,"   %-25s :%d (%.3f)\n","min_sample_freq", 
                    board_info.min_sample_freq, board_info.dbl_min_sample_freq);
    	fprintf(stderr,"   %-25s :%d (%.3f)\n","max_sample_freq", 
                    board_info.max_sample_freq, board_info.dbl_max_sample_freq);
	}

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

		if(disp) {
        	fprintf(stderr,"   %-25s :0x%08x\n","firmware",
					board_info.firmware_ops);
        	fprintf(stderr,"   %-25s :0x%x\n","  revision", 
					board_info.firmware_ops & 0xfff);
        	fprintf(stderr,"   %-25s :%s\n",  "  output channels", 
					nchans);
        	fprintf(stderr,"   %-25s :%s\n",  "  filter frequency", 
					board_info.filter);
        	fprintf(stderr,"   %-25s :%s\n",  "  wire", 
	           (board_info.differential) ?  "Differential":"Single-Ended");
		}
    }
    max_chan_mask = (1 << hw_nchans) - 1;

	if(disp) {
    	fprintf(stderr,"   %-25s :%d\n","board_type", board_info.board_type);
    	fprintf(stderr,"   %-25s :%s\n","board_name", board_info.board_name);
    	fprintf(stderr,"============================================\n");
	}
}

