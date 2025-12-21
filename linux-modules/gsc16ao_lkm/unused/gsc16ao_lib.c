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
 * File:         gsc16ao_lib.c                                               *
 *                                                                           *
 * Description:  Functions to open and set output of single channel          *
 *                                                                           *
 * Date:        11/17/2010                                                   *
 * History:                                                                  *
 *                                                                           *
 *   7 11/17/10 D. Dubash                                                    *
 *              Fixed gsc16ao_set_chan() problems.                           *
 *                                                                           *
 *   6 04/20/10 D. Dubash                                                    *
 *              Support for GSC16AO12 board.                                 *
 *                                                                           *
 *   5 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   4 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   3 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   2  8/23/05 D. Dubash                                                    *
 *              Added new function gsc16ao_ret_gscptr()                      *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *****************************************************************************/

/*
 * Headers
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gsc16ao_ioctl.h"

#include "gsc16ao_lib.h"
static unsigned int volts_to_data(float volts);
static ushort _VoltsToData(double volts, int Format,double ConfigVoltageRange);
#define BOARD_RESOLUTION    16          /* 16 bits */
#define RESOLUTION          (double)(unsigned int)((1 << BOARD_RESOLUTION))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct _gscdevice {
  int      fd;
  gsc16ao_gscregs *gscptr;
} gscdevice;

int  *munmap_gscptr;

board_info_t board_info;
int  hw_nchans;

void 
Get_Board_Info(GSCDEV_HANDLE devhndl, int disp)
{
    int max_chan_mask;

    gscdevice *device = (gscdevice *)devhndl;

    if(ioctl(device->fd, IOCTL_GSC16AO_GET_BOARD_INFO,&board_info)) {
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

#if 1
static unsigned int
volts_to_data(float volts)
{
    return(_VoltsToData(volts, GSC16AO_OFFSET_BINARY, 10.0));
}

static ushort
_VoltsToData(double volts, int Format, double ConfigVoltageRange)
{
    /*** Only BiPolar mode supported ***/
    switch(Format) {
        case GSC16AO_OFFSET_BINARY:
            if(volts == ConfigVoltageRange) return(0xffff);
            return((u_short)(((volts + ConfigVoltageRange) * RESOLUTION) / 
                                                (2.0 * ConfigVoltageRange)));
        break;
        case GSC16AO_TWOS_COMP:
            if(volts == (ConfigVoltageRange)) return(0x7fff);
            return((u_short)((volts * RESOLUTION) / (2.0 * ConfigVoltageRange)));
        break;
    }

    return(0);
}
#else
static unsigned int
volts_to_data(float volts)
{
	/***** Only Offset Binary - BiPolar mode supported *****/
	return((u_int)((volts + 10.0) * (0x10000-1) ) / (2.0 * 10.0));
}
#endif

static int
InitializeBoard(gscdevice *device)
{
  int status;
  unsigned long parm;
  
  /*** Get the board into a known state ***/
  status = ioctl(device->fd, IOCTL_GSC16AO_INIT_BOARD, NULL);
  if (status) {
    fprintf(stderr,"ioctl(IOCTL_GSC16AO_INIT_BOARD) failed: [%s]\n", strerror(errno));
    return(FALSE);
  }

  /*** Set internal clock rate - 400KHz (MAX) sample rate ***/
  device->gscptr->sample_rate = 75;

  /*** Clear simultaneous bit in BCR to enable sequential output ***/
  device->gscptr->board_control &= ~0x0080;

    if(ioctl(device->fd, IOCTL_GSC16AO_GET_BOARD_INFO,&board_info)) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_BOARD_INFO) Failed: %s\n",
            strerror(errno));
        exit(1);
    }

    if(board_info.board_type == GSC_16AO_16) {
        parm = GSC16AO16_RANGE_10; /* +/- 10 Volts */

        status = ioctl(device->fd, IOCTL_GSC16AO_SELECT_OUTPUT_RANGE, &parm);
        if(status) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_OUTPUT_RANGE) failed\n");
        exit(1);
        }
    }
  
  /*** Select internal clock src ***/
  device->gscptr->buffer_ops |=  0x0020; /* enable clocking */

  /*
   * At this point, all we have to do get output is to set the channel
   * selection mask and write the data to the output buffer
   */
  return(TRUE);
}

/*
 * PUBLIC FUNCTIONS
 */

GSCDEV_HANDLE
gsc16ao_init_board(int devno)
{
  char devname[20];
  int fd;
  gsc16ao_gscregs *gscptr = NULL;
  gscdevice *device = NULL;
  unsigned long int offset;

  if (devno < 0 || devno > 9)
    return (NULL);
  
  /*** Open the device ***/
  sprintf(devname, "/dev/gsc16ao%d", devno);
  fd = open(devname, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "open() failure on %s: [%s]\n",devname, strerror(errno));
    return(NULL);
  }
    
  /*** Map GSC GSC16AO CONTROL AND STATUS REGISTERS ***/
  gscptr = (gsc16ao_gscregs *) mmap((caddr_t)0 ,GSC16AO_GSC_REGS_MMAP_SIZE, 
			    PROT_READ|PROT_WRITE, MAP_SHARED, fd, GSC16AO_GSC_REGS_MMAP_OFFSET);
  
  munmap_gscptr = (int *)gscptr;
  if (gscptr == MAP_FAILED) {
    fprintf(stderr, "GSC mmap() failure on %s: [%s]\n",devname,strerror(errno));
    close(fd);
    return(NULL);
  }

  offset = GSC16AO_GSC_REGS_MMAP_OFFSET;
  if(ioctl(fd, IOCTL_GSC16AO_GET_OFFSET,&offset)) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_OFFSET) Failed: %s\n",
          strerror(errno));
      exit(1);
  }

  gscptr = (gsc16ao_gscregs *)((char *)gscptr + offset);

  device = (gscdevice *)malloc(sizeof(gscdevice));
  if (!device) {
    fprintf(stderr, "alloc failed() for %s: [%s]\n", devname, strerror(errno));
    munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
    close(fd);
    return(NULL);
  }

  device->gscptr = gscptr;
  device->fd = fd;
  
  if (!InitializeBoard(device)) {
    fprintf(stderr, "InitializeBoard() failed for %s: [%s]\n", devname, strerror(errno));
    gsc16ao_close_board((GSCDEV_HANDLE)device);
    return(NULL);
  }

  return((GSCDEV_HANDLE)device);
}

static int last_channel = -1;

void
gsc16ao_set_chan(GSCDEV_HANDLE devhndl, int channel, float volts)
{
  gscdevice *device;
  
  if (!devhndl)
    return;

  if (channel < 0 || channel > hw_nchans)
    return;

  if (volts < -10.0 || volts > 10.0)
    return;

  device = (gscdevice *)devhndl;

  /* Program hardware */
  if (channel != last_channel)
    device->gscptr->channel_select = 1 << channel;
  
  device->gscptr->output_data_buffer  = (0x10000 | volts_to_data(volts));

  /* Once the date is written to the output buffer, the user must wait
   * for the buffer to become empty before issuing another request. This
   * is necessary, especially if we are going to change the channel
   * selection while a buffer is not empty.
   */
  usleep(10);
  while(!(device->gscptr->buffer_ops & GSC16AO_OUTPUT_EMPTY));
  
  last_channel=channel;
}

void
gsc16ao_ret_gscptr(GSCDEV_HANDLE devhndl, gsc16ao_gscregs **ptr)
{
  gscdevice *device;
  
  if (!devhndl)
    return;

  device = (gscdevice *)devhndl;
  *ptr = device->gscptr;
}

void
gsc16ao_ret_fd(GSCDEV_HANDLE devhndl, int *fd)
{
  gscdevice *device;
  
  if (!devhndl)
    return;

  device = (gscdevice *)devhndl;
  *fd = device->fd;
}

GSCDEV_HANDLE
gsc16ao_close_board(GSCDEV_HANDLE devhndl)
{
  gscdevice *device = (gscdevice *)devhndl;

  if (!device)
    return(0);

  munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
  close(device->fd);
  free(device);
  return(0);
}

