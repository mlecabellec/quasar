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
 * File:         gsc16ao_out.c                                               *
 *                                                                           *
 * Description:  Analog Output Test                                          *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_out           ==> start interactive analog output test on       *
 *                             on device 0.                                  *
 *   gsc16ao_out <dn>      ==> start interactive analog output test on       *
 *                             on device <dn>. (where dn range is 0 - 9)     *
 *                                                                           *
 * Date:        03/02/2011                                                   *
 * History:                                                                  *
 *                                                                           *
 *   8 03/02/11 D. Dubash                                                    *
 *              Add Auto Calibration option.                                 *
 *                                                                           *
 *   7 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   6 04/20/10 D. Dubash                                                    *
 *              Support for GSC16AO12 board.                                 *
 *                                                                           *
 *   5 02/27/09 D. Dubash                                                    *
 *              Get rid of warnings                                          *
 *                                                                           *
 *   4 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   3 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   2 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Adapted from lcaio_analog_out.c                              *
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define  LINUX_KLOOGE

#include "gsc16ao_ioctl.h"

#define MAX_BUFFER_SIZE  (128 * 1024)
unsigned int WriteBuffer[MAX_BUFFER_SIZE];

#define	MODE_WRITE_PROGIO	1
#define	MODE_WRITE_DMA		2
#define	MODE_MMAP_DATA_PIO	3
#define	MODE_MMAP_ALL_PIO	4

#define	WAVE_SINE		1
#define	WAVE_TRIANGLE		2
#define	WAVE_SQUARE		3
#define	WAVE_STEP		4
#define	WAVE_SIN_STEP_TR_SQ	5
#define	WAVE_CONSTANT		6

#define	DEF_MODE		MODE_WRITE_DMA
#define DEF_NUM_SAMPLES		598		/* Default Number of Samples */
#define	DEF_CHANNEL_MASK	0x1		/* Chan 0 enabled by default */

static int assume_msg=1;

typedef unsigned int U_Int; 

char			buf[200];
char			devname[]="/dev/gsc16ao0";
int			c;
int			fp, status;
int			gsc16ao_break_received;

gsc16ao_gscregs	    *gscptr;
long			    dmaenable;
int			        mode = DEF_MODE;
int                 sample_freq;
int	                num_of_samples = DEF_NUM_SAMPLES;
int                 max_chan_mask;
int	                chan_mask = DEF_CHANNEL_MASK;
unsigned long       buffer_size, buf_size_reg;
int                 *munmap_gscptr;
int                 hw_nchans;
int                 autocal = 0;    /* default is no calibration */

int buffer_sizes_ao12[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072 };
int buffer_sizes_ao16[] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144 };
int			current_wave = 0; 
float			const_volts = 0.0;

int direct_reg_access;

void	quit(int s);
unsigned int volts_to_data(float volts);
int gsc16ao_get_float(float	value, char *desc, float *new, float low, float high);
void BadArg(char *arg);
void run_mode_of_operation_menu();
void Initialize_Board();
void Get_Board_Info();
void Reset_Settings();
void Reset_Board();
int run_wave_generation_menu();
int gsc16ao_get(u_int value, char *desc, int *new, u_int low, u_int high);
int num_active_channels(int chan_mask);
int gsc16ao_get_hex(U_Int value, char *desc, int *new, U_Int low, U_Int high);
int buffer_size_menu();
int main_menu();

board_info_t    board_info;

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    char str[60];
	if(argc == 2) {
		if(argv[1][0] < '0' || argv[1][0] > '9')
			BadArg(argv[1]);
		if(strlen(argv[1]) > 1)
			BadArg(argv[1]);
		devname[12]= argv[1][0];
	} 

	signal(SIGINT, quit);
	
    Get_Board_Info();

	Initialize_Board();

again:
	main_menu();
	
	while((c = fgetc(stdin))) {
		/*** If interrupted, issue a dummy fgetc to clear the error ***/
		if(gsc16ao_break_received)
			fgetc(stdin);

		/*** Issue a dummy fgetc to get rid of any following new line ***/
		if(c != '\n')
			fgetc(stdin);

        gsc16ao_break_received = 0;

		switch(c) {
			case 'A':	/* Auto Calibration */
                /* perform autocal after the voltage range has changed */
                printf("Auto Calibration Initiated....please wait (3-5 seconds)\n");
                ioctl(fp, (unsigned long)IOCTL_GSC16AO_AUTO_CAL, NULL);
                autocal++;
			break;
			case 'b':	/* Set buffer size */
				buffer_size_menu();
			break;

			case 'c':	/* Set Channel Mask */
                sprintf(str,"Channel Mask [0x1-0x%x]\t",
                                max_chan_mask);

				if(gsc16ao_get_hex(chan_mask, str, &chan_mask,0x1,
                                        max_chan_mask))
					break;
				
				if (num_of_samples * num_active_channels(chan_mask) > buffer_size) {
					fprintf(stderr,
						"WARNING: Buffer size %ld not large enough for %d samples times %d active channels\n",
						buffer_size, num_of_samples, num_active_channels(chan_mask));
					fprintf(stderr,
						"         Increase buffer size or reduce samples or active channels!!!\n");
				}
			break;

			case 'f':	/* Set Sampling Frequency */
                sprintf(str,"Sample Freq [%d-%d]\t",
                            board_info.min_sample_freq,
                            board_info.max_sample_freq);
				if(gsc16ao_get(sample_freq,str,
							&sample_freq,board_info.min_sample_freq,
                            board_info.max_sample_freq));
				  break;
				
				fprintf(stderr,"Sample Frequency = %d\n",sample_freq);

				if (direct_reg_access) {
				  gscptr->sample_rate = board_info.master_clock/sample_freq;
				}
				else {
				  unsigned long parm = board_info.master_clock/sample_freq;
				  status = ioctl(fp, IOCTL_GSC16AO_PROGRAM_RATE_GEN, &parm);
				  if(status) {
				    fprintf(stderr,"ioctl(IOCTL_GSC16AO_PROGRAM_RATE_GEN) failed\n");
				    exit(1);
				  }
				}
			break;

			case 'm':	/* Mode of operation */
				run_mode_of_operation_menu();
			break;

			case 'n':	/* Number of samples in buffer */
				if(gsc16ao_get(num_of_samples,
					       "Number Of Samples [1-131072]\t",
					       &num_of_samples,1,131072))
				        break;
				
				fprintf(stderr,"Number of Samples = %d\n", num_of_samples);

				if (num_of_samples * num_active_channels(chan_mask) > buffer_size) {
					fprintf(stderr,
						"WARNING: Buffer size %ld not large enough for %d samples times %d active channels\n",
						buffer_size, num_of_samples, num_active_channels(chan_mask));
					fprintf(stderr,
						"         Increase buffer size or reduce samples or active channels!!!\n");
				}
			break;

			case 'w':	/* Wave Generation Menu */
				run_wave_generation_menu();
			break;

			case 'R':	/* Reset Board */
				fprintf(stderr,"\nResetting Board\n");
				Reset_Board();
			break;

			case 'q':	/* Quit */
				goto    getout;
			break;

			case '\n':
				goto	again;
			break;

			default:
				fprintf(stderr,"ERROR!!! Invalid Input <%c>\n",c);
			break;
		}

		main_menu();
	}

	if(gsc16ao_break_received)
		goto again;

getout:
	if(fp) {
		Reset_Board();
		/*** unmap GSC area ***/
		munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
	}

exit(0);
}

/*** Reset Board Routine ***/
void
Reset_Board()
{
  unsigned long parm;
  
  /*** Get the board into a known state ***/
  status = ioctl(fp, IOCTL_GSC16AO_INIT_BOARD, NULL);
  if(status) {
    fprintf(stderr,"ioctl(IOCTL_GSC16AO_INIT_BOARD) failed: [%s]\n",
	    strerror(errno));
    exit(1);
  }

  /* Pickup default buffer size from board/driver */
  buf_size_reg = gscptr->buffer_ops & 0xf;

  if(board_info.board_type == GSC_16AO_16) {
    buffer_size = buffer_sizes_ao16[buf_size_reg];
  } else {
    buffer_size = buffer_sizes_ao12[buf_size_reg];
  }
  
  /*** Set DMA mode ***/
  if (mode == MODE_WRITE_DMA)
    dmaenable = 1;		/* Enable DMA */
  else
    dmaenable = 0;
  
  status = ioctl(fp, IOCTL_GSC16AO_SET_WRITE_MODE, &dmaenable);
  if(status) {
    fprintf(stderr,"ioctl(IOCTL_GSC16AO_SET_WRITE_MODE) failed\n");
    exit(1);
  }

  /*** Set DMA mode ***/
  if (mode == MODE_MMAP_ALL_PIO)
    direct_reg_access = 1;		/* Use direct register access */
  else
    direct_reg_access = 0;		/* Use driver ioctls */

  
  /*
   * Setup board for generating periodic functions.
   */
  if (direct_reg_access) {
    /*** Select simultaneous clocking mode, continuous mode (looping), offset binary format ***/
    gscptr->board_control = 0x0090;

    /*** Select internal clock src ***/
    gscptr->buffer_ops &= ~0x0030; /* Disable clocking, Set internal clk source */

    /*** Set internal clock rate - 400KHz (MAX) sample rate ***/
    gscptr->sample_rate = board_info.master_clock/sample_freq;
  }
  else {
    /*** Select offset binary format ***/
    parm = GSC16AO_OFFSET_BINARY;
    status = ioctl(fp, IOCTL_GSC16AO_SELECT_DATA_FORMAT, &parm);
    if(status) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_DATA_FORMAT) failed\n");
      exit(1);
    }
    
    /*** Set sampling mode to Continous (looping) ***/
    parm = GSC16AO_CONT_MODE;
    status = ioctl(fp, IOCTL_GSC16AO_SELECT_SAMPLING_MODE, &parm);
    if(status) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_SAMPLING_MODE) failed\n");
      exit(1);
    }
    
    /*** Set clocking mode to simultaneous ***/
    parm = SIMULTANEOUS;
    status = ioctl(fp, IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE, &parm);
    if(status) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE) failed\n");
      exit(1);
    }
    
    /*** Set clock src to internal clock ***/
    parm = INTERNAL;
    status = ioctl(fp, IOCTL_GSC16AO_SELECT_CLK_SOURCE, &parm);
    if(status) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_CLK_SOURCE) failed\n");
      exit(1);
    }
    
    /*** Program selected clock rate ***/
    parm = board_info.master_clock/sample_freq;
    status = ioctl(fp, IOCTL_GSC16AO_PROGRAM_RATE_GEN, &parm);
    if(status) {
      fprintf(stderr,"ioctl(IOCTL_GSC16AO_PROGRAM_RATE_GEN) failed\n");
      exit(1);
    }
  }

    if(board_info.board_type == GSC_16AO_16) {
        parm = GSC16AO16_RANGE_10; /* +/- 10 Volts */

        status = ioctl(fp, IOCTL_GSC16AO_SELECT_OUTPUT_RANGE, &parm);
        if(status) {
          fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_OUTPUT_RANGE) failed\n");
          exit(1);
        }
    } else {
        if(assume_msg) {
        	printf("\nAssume that the board '%s' supports a voltage range of +/-10 Volts\n",board_info.board_name);
                assume_msg=0;
        }
    }
	  
    gscptr->channel_select = chan_mask;
}

/*** Reset Setings ***/
void
Reset_Settings()
{
  /* reset program settings to defaults */
  mode = DEF_MODE;
  sample_freq = board_info.max_sample_freq;   /* set default sample frequency */
  num_of_samples = DEF_NUM_SAMPLES;
  chan_mask = DEF_CHANNEL_MASK;
  current_wave = 0;
  Reset_Board();
}

void 
Get_Board_Info(int disp)
{
    fp  = open(devname, O_RDWR);
    if (fp == -1) {
      printf(   "open() failure on %s: [%s]\n",devname, strerror(errno));
      exit(1);
    }

    if(ioctl(fp, IOCTL_GSC16AO_GET_BOARD_INFO,&board_info)) {
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

/*** Initialize Board Routine ***/
void
Initialize_Board()
{
    unsigned long int offset;

    sample_freq = board_info.max_sample_freq;   /* set default sample frequency */

    /*** Map GSC GSC16AO CONTROL AND STATUS REGISTERS ***/
    gscptr = (gsc16ao_gscregs *) mmap((caddr_t)0 ,GSC16AO_GSC_REGS_MMAP_SIZE, 
			      PROT_READ|PROT_WRITE, MAP_SHARED, fp, GSC16AO_GSC_REGS_MMAP_OFFSET);
    
    munmap_gscptr = (int *)gscptr;
    if(gscptr == MAP_FAILED) {
      printf("GSC mmap() failure on %s: [%s]\n",devname,strerror(errno));
      exit(1);
    }

    offset = GSC16AO_GSC_REGS_MMAP_OFFSET;
    if(ioctl(fp, IOCTL_GSC16AO_GET_OFFSET,&offset)) {
        fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_OFFSET) Failed: %s\n",
            strerror(errno));
        exit(1);
    }

    gscptr = (gsc16ao_gscregs *)((char *)gscptr + offset);
    
    fprintf(stderr,"\ngscptr = %p\n",gscptr);
    
    Reset_Settings();
}

/****************************************************************************
 * Print Main Menu                                                          *
 ****************************************************************************/
int
main_menu()
{
	char	chn[20], sf[20], ns[20], bs[20];
	if(chan_mask==max_chan_mask)
		strcpy(chn,"<<all>>   ");
	else
		sprintf(chn,"<<0x%04x>>",chan_mask);
	sprintf(sf,"<<%d>>",sample_freq);
	sprintf(ns,"<<%d>>",num_of_samples);
	sprintf(bs,"<<%ld>>",buffer_size);

	fprintf(stderr,"  +===================================================+\n");
	fprintf(stderr,"  |         ************ MAIN MENU ***********        |\n");
	fprintf(stderr,"  |                                                   |\n");
	fprintf(stderr,"  | 'A'  - Auto Calibrate Board                       |\n");
	fprintf(stderr,"  | 'b'  - Set buffer size                 %-10s |\n",bs);
	fprintf(stderr,"  | 'c'  - Set channel mask                %s |\n",chn);
	fprintf(stderr,"  | 'f'  - Set sampling frequency (HZ)     %-10s |\n",sf);
	fprintf(stderr,"  | 'm'  - ===> Mode of Operation MENU                |\n");
	fprintf(stderr,"  | 'n'  - Set Number of Samples in Buffer %-10s |\n",ns);
	fprintf(stderr,"  | 'w'  - ===> Wave Generation MENU                  |\n");
	fprintf(stderr,"  | 'R'  - Reset Board                                |\n");
	fprintf(stderr,"  | 'q'  - quit the test                              |\n");
	fprintf(stderr,"  +===================================================+\n");
	fprintf(stderr,"  Select Option --->");
    
	return(0);
}

/****************************************************************************
 * Print mode menu                                                          *
 ****************************************************************************/
int
mode_menu()
{
	fprintf(stderr,"\t+===============================================================+\n");
	fprintf(stderr,"\t|               ***** MODE OF OPERATION MENU *****              |\n");
	fprintf(stderr,"\t|                                                               |\n");
	fprintf(stderr,"\t| 'p'  - write() [ Programmed I/O ]               %s   |\n",
			mode==MODE_WRITE_PROGIO?"<<current>>":"           ");
	fprintf(stderr,"\t| 'd'  - write() [ DMA ]                          %s   |\n",
			mode==MODE_WRITE_DMA?"<<current>>":"           ");
	fprintf(stderr,"\t| 'm'  - mmap()  [ User-mode PIO data transfers ] %s   |\n",
			mode==MODE_MMAP_DATA_PIO?"<<current>>":"           ");
	fprintf(stderr,"\t| 'a'  - mmap()  [ User-mode access for all I/O ] %s   |\n",
			mode==MODE_MMAP_ALL_PIO?"<<current>>":"           ");
	fprintf(stderr,"\t| 'q'  - ===> MAIN MENU                                         |\n");
	fprintf(stderr,"\t+===============================================================+\n");
	fprintf(stderr,"\tSelect Option --->");
    
	return(0);
}

/****************************************************************************
 * Print Wave Menu                                                          *
 ****************************************************************************/
int
wave_menu()
{
	fprintf(stderr,"\t+=====================================================+\n");
	fprintf(stderr,"\t|         ****** WAVE GENERATION MENU ******          |\n");
	fprintf(stderr,"\t|                                                     |\n");
	fprintf(stderr,"\t| 'c'  - User Supplied Constant          %s  |\n",
		current_wave==WAVE_CONSTANT?"<<current>>":"           ");
	fprintf(stderr,"\t| 's'  - Sine Wave                       %s  |\n",
		current_wave==WAVE_SINE?"<<current>>":"           ");
	fprintf(stderr,"\t| 't'  - Triangle Wave                   %s  |\n",
		current_wave==WAVE_TRIANGLE?"<<current>>":"           ");
	fprintf(stderr,"\t| 'x'  - Square Wave                     %s  |\n",
		current_wave==WAVE_SQUARE?"<<current>>":"           ");
	fprintf(stderr,"\t| 'y'  - Step Wave                       %s  |\n",
		current_wave==WAVE_STEP?"<<current>>":"           ");
	fprintf(stderr,"\t| 'z'  - Sine, Step, Triangle and Square %s  |\n",
		current_wave==WAVE_SIN_STEP_TR_SQ?"<<current>>":"           ");
	fprintf(stderr,"\t| 'q'  - ===> MAIN MENU                               |\n");
	fprintf(stderr,"\t+=====================================================+\n");
	fprintf(stderr,"\tSelect Option --->");
    
	return(0);
}

/****************************************************************************
 * Print buffer size menu                                                   *
 ****************************************************************************/
int
bufsz_menu()
{
	fprintf(stderr,"\t+=================================================================+\n");
	fprintf(stderr,"\t|                ***** BUFFER SIZE MENU *****                     |\n");
	fprintf(stderr,"\t|                                                                 |\n");

    if(board_info.board_type == GSC_16AO_16) {
    	fprintf(stderr,"\t| '0' -    8 samples %s   '8' - 2048 samples %s |\n",
    		buffer_size == 4 ? "<<current>>":"           ",
    		buffer_size == 1024 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '1' -   16 samples %s   '9' - 4096 samples %s |\n",
    		buffer_size == 8 ? "<<current>>":"           ",
    		buffer_size == 2048 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '2' -   32 samples %s   'a' - 8192 samples %s |\n",
    		buffer_size == 16 ? "<<current>>":"           ",
    		buffer_size == 4096 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '3' -   64 samples %s   'b' -  16K samples %s |\n",
    		buffer_size == 32 ? "<<current>>":"           ",
    		buffer_size == 8192 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '4' -  128 samples %s   'c' -  32k samples %s |\n",
    		buffer_size == 64 ? "<<current>>":"           ",
    		buffer_size == 16384 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '5' -  256 samples %s   'd' -  64k samples %s |\n",
    		buffer_size == 128 ? "<<current>>":"           ",
    		buffer_size == 32768 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '6' -  512 samples %s   'e' - 128k samples %s |\n",
    		buffer_size == 256 ? "<<current>>":"           ",
    		buffer_size == 65536 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '7' - 1024 samples %s   'f' - 256k samples %s |\n",
    		buffer_size == 512 ? "<<current>>":"           ",
    		buffer_size == 131072 ? "<<current>>":"           ");

    } else {
 
    	fprintf(stderr,"\t| '0' -    4 samples %s   '8' - 1024 samples %s |\n",
    		buffer_size == 4 ? "<<current>>":"           ",
    		buffer_size == 1024 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '1' -    8 samples %s   '9' - 2048 samples %s |\n",
    		buffer_size == 8 ? "<<current>>":"           ",
    		buffer_size == 2048 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '2' -   16 samples %s   'a' - 4096 samples %s |\n",
    		buffer_size == 16 ? "<<current>>":"           ",
    		buffer_size == 4096 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '3' -   32 samples %s   'b' - 8192 samples %s |\n",
    		buffer_size == 32 ? "<<current>>":"           ",
    		buffer_size == 8192 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '4' -   64 samples %s   'c' -  16k samples %s |\n",
    		buffer_size == 64 ? "<<current>>":"           ",
    		buffer_size == 16384 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '5' -  128 samples %s   'd' -  32k samples %s |\n",
    		buffer_size == 128 ? "<<current>>":"           ",
    		buffer_size == 32768 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '6' -  256 samples %s   'e' -  64k samples %s |\n",
    		buffer_size == 256 ? "<<current>>":"           ",
    		buffer_size == 65536 ? "<<current>>":"           ");
    	fprintf(stderr,"\t| '7' -  512 samples %s   'f' - 128k samples %s |\n",
    		buffer_size == 512 ? "<<current>>":"           ",
    		buffer_size == 131072 ? "<<current>>":"           ");
    }

	fprintf(stderr,"\t| 'q' - ===> MAIN MENU                                            |\n");
	fprintf(stderr,"\t+=================================================================+\n");
	fprintf(stderr,"\tSelect Option --->");
    
	return(0);
}

/****************************************************************************
 * break interrupt received                                                 *
 ****************************************************************************/
void
quit(int s)
{
    gsc16ao_break_received = 1;
    fprintf(stderr,"\nTERMINATE RECEIVED!!! Shutdown/quit Test? (q,y/n)->");
    c = fgetc(stdin);
    if((c == 'y') || (c == 'q')) {
        if(fp) {
	  Reset_Board();
	  /*** unmap GSC area ***/
	  munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
	  
	  close(fp);
        }
        exit(1);
    }
    else
        signal(SIGINT, quit);

    fprintf(stderr,"                            Hit <cr> to continue ->");
}

/****************************************************************************
 * buffer size menu                                                         *
 ****************************************************************************/
int
buffer_size_menu()
{

 bufsize_again:
            bufsz_menu();
	
	while((c = fgetc(stdin))) {
		/*** If interrupted, issue a dummy fgetc to clear the error ***/
		if(gsc16ao_break_received)
			fgetc(stdin);

		/*** Issue a dummy fgetc to get rid of any following new line ***/
		if(c != '\n')
			fgetc(stdin);

		gsc16ao_break_received = 0;

		switch (c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  if (c >= '0' && c <= '9')
		    buf_size_reg = c - '0';
		  else
		    buf_size_reg = c - 'a' + 10;

          if(board_info.board_type == GSC_16AO_16) {
              buffer_size = buffer_sizes_ao16[buf_size_reg];
          } else {
              buffer_size = buffer_sizes_ao12[buf_size_reg];
          }
		  
		  fprintf(stderr,"Buffer size set to %ld samples\n", buffer_size);
		  break;
		  
		case 'q':	/* Quit */
		  return(0);
		  break;
		  
		case '\n':
		  goto	bufsize_again;
		  break;
		  
		default:
		  fprintf(stderr,"ERROR!!! Invalid Input <%c>\n",c);
		  break;
		}
		
		bufsz_menu();
	}
	
	if(gsc16ao_break_received)
		goto bufsize_again;

	return(0);
}

/****************************************************************************
 * Run Mode of Operation Menu                                               *
 ****************************************************************************/
void
run_mode_of_operation_menu()
{
  int done = 0;
  int prevmode = mode;
  
  do {
    /* Display menu */
    mode_menu();
    
    c = fgetc(stdin);
    
    /*** If interrupted, issue a dummy fgetc to clear the error ***/
    if (gsc16ao_break_received)
      fgetc(stdin);
      
    /*** Issue a dummy fgetc to get rid of any following new line ***/
    if (c != '\n')
      fgetc(stdin);
    
    gsc16ao_break_received = 0;
      
    switch(c) {
    case 'p':	/* write() [ Programmed I/O ] */
      mode = MODE_WRITE_PROGIO;
      fprintf(stderr,"Mode set to write() Programmed I/O\n");
      break;
      
    case 'd':	/* write() [ DMA ] */
      mode = MODE_WRITE_DMA;
      fprintf(stderr,"Mode set to write() DMA\n");
      break;
      
    case 'm':	/* mmap() [ User-mode access I/O] */
      mode = MODE_MMAP_DATA_PIO;
      fprintf(stderr,"Mode set to mmap() user-mode programmed I/O for data transfers\n");
      break;
      
    case 'a':	/* mmap() [ User-mode access for all I/O] */
      mode = MODE_MMAP_ALL_PIO;
      fprintf(stderr,"Mode set to mmap() user-mode access for all I/O\n");
      break;
      
    case '\n': /* Print menu again */
      break;
      
    case 'q':	/* Quit */
    case EOF:
      done = 1;
      break;
      
    default:
      fprintf(stderr,"ERROR!!! Invalid Input <%c>\n",c);
      break;
    } /* switch */
  } while (!done);

  if (mode != prevmode)
    Reset_Board();
}

int
num_active_channels(int chan_mask)
{
  int i;
  int n = 0;
  
  chan_mask &= max_chan_mask;
  
  for (i=0 ; i<hw_nchans; i++) {
    if (chan_mask & (1<<i))
      n ++;
  }
  
  return n;
}

void
usec_delay(unsigned long usec)
{
  struct timespec tv;
  tv.tv_sec = 0;
  tv.tv_nsec = usec * 1000;
  nanosleep(&tv,NULL);
}   

/****************************************************************************
 * Generate Wave                                                            *
 ****************************************************************************/
void
generate_wave(int num_samples)
{
	int i;
	CHAN_SELECT channels;
	unsigned long parm;

	if (num_samples == 0) {
		fprintf(stderr,
		"ERROR!!!: generate_wave(): Insufficient number of samples provided\n");
		return;
	}

	WriteBuffer[num_samples-1] |= 0x10000; /* Set End of Frame flag */
	
#ifdef DEBUG
	for(i=0; i < num_samples; i++) {
	  if(!(i%8))
	    fprintf(stderr,"\n0x%04x ",i*32);
	  fprintf(stderr,"%08x ",WriteBuffer[i]);
	}
	fprintf(stderr,"\n");
#endif /* end DEBUG */

	/*
	 * Prepare to load buffer
	 */
	if (direct_reg_access) {
	  /*** Disable Output Clock ***/
	  gscptr->buffer_ops &= ~0x0020;

	  /*** Clear buffer ***/
	  gscptr->buffer_ops |= 0x0800; /* trigger buffer clear */
	  do { usec_delay(100); } while (gscptr->buffer_ops & 0x0800); /* wait for it to finish */

	  /*** Open buffer and (re)set buffer size ***/
	  gscptr->buffer_ops = (gscptr->buffer_ops & ~0x010f) | (buf_size_reg & 0xf);
	}
	else {
	  /*** Disable clocking ***/
	  status = ioctl(fp, IOCTL_GSC16AO_DISABLE_CLK, NULL);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_DISABLE_CLK) failed\n");
	    exit(1);
	  }
	
	  /*** Clear existing buffer ***/
	  status = ioctl(fp, IOCTL_GSC16AO_CLEAR_BUFFER, NULL);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_CLEAR_BUFFER) failed: [%s]\n",
		    strerror(errno));
	    exit(1);
	  }

	  /*** Enable selected buffer size ***/
	  status = ioctl(fp, IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE, &buf_size_reg);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE) failed: [%s]\n",
		    strerror(errno));
	    exit(1);
	  }
	
	  /*** Open the buffer up while we load it (prevents waiting for LOAD_REQ) ***/
	  parm = GSC16AO_OPEN_BUF;
	  status = ioctl(fp, IOCTL_GSC16AO_SELECT_BUF_CONFIG, &parm);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_BUF_CONFIG) failed\n");
	    exit(1);
	  }
	}

	/*
	 * Load the buffer using selected access mode
	 */
	switch(mode) {
		case MODE_WRITE_DMA:	/* write() DMA */
		case MODE_WRITE_PROGIO:	/* write() Programmed I/O */
			status = write(fp, WriteBuffer, num_samples*4);
            if(status == -1) {
                fprintf(stderr,"Write Failed: %d [%s]\n",errno,strerror(errno));
            } else  {
				if(status != num_samples*4) {
				  fprintf(stderr,"Write Failed, rv=%d, expected %d\n", status, num_samples*4);
				  
				  parm = 0;
				  status = ioctl(fp, IOCTL_GSC16AO_GET_DEVICE_ERROR, &parm);
				  if (status) {
				    fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_DEVICE_ERROR) failed\n");
				    exit(1);
				  }
				  fprintf(stderr,"               extended device error code: %ld\n",
	                     parm);
				}			  
            }

		break;
		case MODE_MMAP_DATA_PIO:	/* mmap() Programmed I/O */
		case MODE_MMAP_ALL_PIO:		/* mmap() All I/O */
			for(i=0; i < num_samples; i++)
				gscptr->output_data_buffer = WriteBuffer[i];
		break;
	}

	/*
	 * Close buffer, enable channels and start output
	 */
	if (direct_reg_access) {
      /* for some reason, if autocal is enabled and we circulate a constant
       * wave, the zero offset is several milliseconds from zero for
       * many channels. It almost seems that setting board for circular 
       * mode causes the original autocal to lose its information. For now,
       * there is really no need to circulate a constant wave.
       */
      if(!(autocal && (current_wave == WAVE_CONSTANT))) {
	     /*** Close the buffer so it will circulate instead of shifting out ***/
	     gscptr->buffer_ops |= 0x0100;
      }

	  /*** Select active channels ***/
	  gscptr->channel_select = chan_mask;
	  
	  /*** (Re)enable output clocking ***/
	  gscptr->buffer_ops |= 0x0020;
	}
	else {
      /* for some reason, if autocal is enabled and we circulate a constant
       * wave, the zero offset is several milliseconds from zero for
       * many channels. It almost seems that setting board for circular 
       * mode causes the original autocal to lose its information. For now,
       * there is really no need to circulate a constant wave.
       */
      if(!(autocal && (current_wave == WAVE_CONSTANT))) {
	     /*** Close the buffer so it will circulate instead of shifting out ***/
	     parm = GSC16AO_CIRCULAR_BUF;
	     status = ioctl(fp, IOCTL_GSC16AO_SELECT_BUF_CONFIG, &parm);
	     if(status) {
	       fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_BUF_CONFIG) failed\n");
	       exit(1);
	     }
      }
	  
	  /*** Select active channels ***/
	  channels.ulChannels = chan_mask;
	  channels.ulNumChannels = num_active_channels(chan_mask);
	  status = ioctl(fp,IOCTL_GSC16AO_SELECT_ACTIVE_CHAN, &channels);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_SELECT_ACTIVE_CHAN) failed: [%s]\n",
		    strerror(errno));
	    exit(1);
	  }
	  
	  /*** (Re)enable output clocking ***/
	  status = ioctl(fp, IOCTL_GSC16AO_ENABLE_CLK, NULL);
	  if(status) {
	    fprintf(stderr,"ioctl(IOCTL_GSC16AO_ENABLE_CLK) failed\n");
	    exit(1);
	  }
	}
}

int
create_sine_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int i, j, n, volts;
	double x;
	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! SINE: Minimum of 2 samples required\n");
		return(0);
	}
	j = 0;
	x = 0.0;
	for(i=0; i < num_samples ;i++) {
		volts = 0xffff & (int)(sin(x) * 32767.0);
		if(i <= num_samples/2) {
			volts += 0x8000;
            if(volts > 0xffff)
                volts = 0xffff;
        }
		else
			volts -= 0x8000;

		x += (2.0 * M_PI) / (double)num_samples;
		
		for (n=0; n<chan_cnt; n++)
		  wbufptr[j++] = volts;
	}
	fprintf(stderr,"Sine Wave [%d] samples - frequency = %0.1f Hz\n",j,
                        ((double)sample_freq/(double)j) *
                        (double)num_active_channels(chan_mask));
	return(j);
}

int
create_triangle_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n, up, down;
	double	incr;
	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! TRIANGLE: Minimum of 2 samples required\n");
		return(0);
	}
	up = (num_samples/2 + 1);
	down = (num_samples/2 - 1);
	j = 0;
	incr = 0xffff/(up-1);	
	for(i=0; i < up; i++)	/* 300 points i.e. 0 - 299 */
		for (n=0; n<chan_cnt; n++)
			wbufptr[j++] = (int) i*incr;
	
	for(i=down; i > 0; i--)	/* 298 points i.e. 298 - 1 */
		for (n=0; n<chan_cnt; n++)
			wbufptr[j++] = (int) i*incr;
	
	fprintf(stderr,"Triangle Wave [%d] samples - frequency = %0.1f Hz\n",j,
                        ((double)sample_freq/(double)j) *
                        (double)num_active_channels(chan_mask));
	return(j);
}

int
create_square_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n;
	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! SQUARE: Minimum of 2 samples required\n");
		return(0);
	}
	j = 0;
	for(i=0; i < num_samples/2; i++)
		for (n=0; n<chan_cnt; n++)
		  	wbufptr[j++] = 0xffff;
	for(i=0; i < num_samples/2; i++)
		for (n=0; n<chan_cnt; n++)
			wbufptr[j++] = 0;
	fprintf(stderr,"Square Wave [%d] samples - frequency = %0.1f Hz\n",j,
                        ((double)sample_freq/(double)j) *
                        (double)num_active_channels(chan_mask));
	return(j);
}

int
create_step_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n, step;
	double	incr;
	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! STEP: Minimum of 2 samples required\n");
		return(0);
	}
	j = 0;
	incr = 0;
	if(num_samples < 6)
		step = 1;
	else {
		step = num_samples/6;
		num_samples = step*6;
	}
	
	for(i=0; i < num_samples; i++) {
		if(!(i%step) && i)
			incr += 0xffff/((num_samples/step)-1);

		for (n=0; n<chan_cnt; n++)
			  wbufptr[j++] = incr;
	}
	fprintf(stderr,"Step Wave [%d] samples - frequency = %0.1f Hz\n",j,
                        ((double)sample_freq/(double)j) *
                        (double)num_active_channels(chan_mask));
	return(j);
}

/****************************************************************************
 * Run Wave Generation Menu                                                 *
 ****************************************************************************/
int
run_wave_generation_menu()
{
	unsigned int *bpt;
	int	       ns, i;
	int            chan_cnt;
	unsigned int   data;
again:
	wave_menu();
	
	while((c = fgetc(stdin))) {
		/*** If interrupted, issue a dummy fgetc to clear the error ***/
		if(gsc16ao_break_received)
			fgetc(stdin);

		/*** Issue a dummy fgetc to get rid of any following new line ***/
		if(c != '\n')
			fgetc(stdin);

		gsc16ao_break_received = 0;

		chan_cnt = num_active_channels(chan_mask);
		if (chan_cnt == 0) {
			fprintf(stderr, "ERROR!!!: No channels enabled???\n");
			return(-1);
		}

		if (num_of_samples * chan_cnt > buffer_size) {
		  	fprintf(stderr,
				"ERROR!!! Number of samples (%d) times active channels (%d) > buffer_size (%ld)\n",
				num_of_samples, chan_cnt, buffer_size);
			return(-1);
		}

		switch(c) {
			case 'c':	/* CONSTANT Wave */
				current_wave = WAVE_CONSTANT;
				if(gsc16ao_get_float(const_volts, "Constant Volts [+/-10]\t",
											&const_volts,-10.0,+10.0))
					break;

				data = volts_to_data(const_volts);
				for (i=0; i<chan_cnt; i++)
				  WriteBuffer[i] = data;
				
				generate_wave(chan_cnt);
			break;

			case 's':	/* SINE wave */
				current_wave = WAVE_SINE;
				generate_wave(create_sine_wave(WriteBuffer,num_of_samples,chan_cnt));
			break;

			case 't':	/* Triangle wave */
				current_wave = WAVE_TRIANGLE;
				generate_wave(create_triangle_wave(WriteBuffer,num_of_samples,chan_cnt));
			break;

			case 'x':	/* Square wave */
				current_wave = WAVE_SQUARE;
				generate_wave(create_square_wave(WriteBuffer,num_of_samples,chan_cnt));
			break;

			case 'y':	/* Step wave */
				current_wave = WAVE_STEP;
				generate_wave(create_step_wave(WriteBuffer,num_of_samples,chan_cnt));
			break;

			case 'z':	/* Sine, Step, Triangle & Square wave */
				if(num_of_samples < 8) {
					fprintf(stderr,
						"ERROR!!! Minimum 8 samples required for this wave\n");
					break;
				}
				current_wave = WAVE_SIN_STEP_TR_SQ;
				bpt = WriteBuffer;
				ns = create_sine_wave(bpt,num_of_samples/4,chan_cnt);	/* sine */
				bpt += ns;
				ns = create_step_wave(bpt,num_of_samples/4,chan_cnt);	/* step */
				bpt += ns;
				ns = create_triangle_wave(bpt,num_of_samples/4,chan_cnt);/* triangle */
				bpt += ns;
				ns = create_square_wave(bpt,num_of_samples/4,chan_cnt);	/* square */
				bpt += ns;
				generate_wave(bpt - WriteBuffer);
			break;

			case 'q':	/* Quit */
				return(0);
			break;

			case '\n':
				goto	again;
			break;

			default:
				fprintf(stderr,"ERROR!!! Invalid Input <%c>\n",c);
			break;
		}

		wave_menu();
	}

	if(gsc16ao_break_received)
		goto again;

return(0);
}

/*****************
 * Volts to Data *
 *****************/
unsigned int
volts_to_data(float volts)
{
	/***** Only Offset Binary - BiPolar mode supported *****/
	return((u_int)((volts + 10.0) * (0x10000-1) ) / (2.0 * 10.0));
}

/******************************************************************************
 *** Get Input From User Routine                                            ***
 ******************************************************************************/
int
gsc16ao_get(u_int value, char *desc, int *new, u_int low, u_int high)
{
	int		i,j;
	char	c[20];

	again:

	sprintf(c,"0x%x(%d)",value,value);
	fprintf(stderr,"\t%s\t%18s -->",desc, c);
#ifdef  LINUX_KLOOGE
	/*** ALL THIS KLOOGE CODE TO OVERCOME BUGS IN LINUX fgets/fgetc ***/
	i=0;
	while((j = fgetc(stdin))) {
		buf[i++] = (unsigned char)j;
		if((j==EOF) || (j=='\n') || !j)
			break;
	}
#else /* else !LINUX_KLOOGE */
	fgets(buf, sizeof(buf), stdin);
#endif /* end LINUX_KLOOGE */

	/*** If interrupted, issue a dummy fgetc to clear the error ***/
	if(gsc16ao_break_received) {
		gsc16ao_break_received = 0;
		fgetc(stdin);
		return(1);
	}

	if((buf[0] == 'q') && (buf[1] == '\n'))
		return(1);

	if(buf[0] == '\n') {
		if((value < low) || (value > high)) {
			fprintf(stderr,
					"ERROR!!! Value %d not in range %d to %d (0x%x-0x%x)\n", 
												value,low,high,low,high);
			goto again;
		}
		*new = value;
		return(0);
	}

	if((buf[0] != '-') && (buf[0] != '+')&&((buf[0] < '0') || (buf[0] > '9'))) {
		fprintf(stderr,"Invalid Value %s\n",buf);
		goto again;
	}

	i = strtoul(buf, NULL, 0);
	if((i < low) || (i > high)) {
		fprintf(stderr,"ERROR!!! Value %d not in range %d to %d (0x%x-0x%x)\n",
												i,low,high,low,high);
		goto again;
	}
	*new = i;
	return(0);
}

/******************************************************************************
 *** Get Float From User Routine                                            ***
 ******************************************************************************/
int
gsc16ao_get_float(float	value, char *desc, float *new, float low, float high)
{
	int		ret, i, j;
	float	f;
	char	c[20];

	again:

	sprintf(c,"%f",value);
	fprintf(stderr,"\t%s\t%18s -->",desc, c);
#ifdef LINUX_KLOOGE
	/*** ALL THIS KLOOGE CODE TO OVERCOME BUGS IN LINUX fgets/fgetc ***/
	i=0;
	while((j = fgetc(stdin))) {
		buf[i++] = (unsigned char)j;
		if((j==EOF) || (j=='\n') || !j)
			break;
	}
#else /* else !LINUX_KLOOGE */
	fgets(buf, sizeof(buf), stdin);
#endif /* end LINUX_KLOOGE */

	/*** If interrupted, issue a dummy fgetc to clear the error ***/
	if(gsc16ao_break_received) {
		gsc16ao_break_received = 0;
		fgetc(stdin);
		return(1);
	}

	if((buf[0] == 'q') && (buf[1] == '\n'))
		return(1);
	if(buf[0] == '\n') {
		if((value < low) || (value > high)) {
			fprintf(stderr,
					"ERROR!!! Value %f not in range %f to %f\n",value,low,high);
			goto again;
		}
		*new = value;
		return(0);
	}

	if((buf[0] != '.') && (buf[0] != '-') && (buf[0] != '+')
                      && ((buf[0] < '0') || (buf[0] > '9')) ) {
		fprintf(stderr,"Invalid Value %s\n",buf);
		goto again;
	}

	ret= sscanf(buf ,"%f",&f);

	if(!ret || ret == EOF) {
		fprintf(stderr,"ERROR!!! Invalid Float %s\n",buf);
		goto again;
	}

	if((f < low) || (f > high)) {
		fprintf(stderr,"ERROR!!! Value %f not in range %f to %f\n",f,low,high);
 		goto again;
	}
	*new = f;
	return(0);
}

/******************************************************************************
 *** Get Hex From User Routine                                              ***
 ******************************************************************************/
int
gsc16ao_get_hex(U_Int value, char *desc, int *new, U_Int low, U_Int high)
{
	int		ret, i, j;
	U_Int	val;
	char   *bp;
	char	c[20];

	again:

	sprintf(c,"0x%x",value);
	fprintf(stderr,"\t%s\t%18s -->",desc, c);
#ifdef LINUX_KLOOGE
	/*** ALL THIS KLOOGE CODE TO OVERCOME BUGS IN LINUX fgets/fgetc ***/
	i=0;
	while((j = fgetc(stdin))) {
		buf[i++] = (unsigned char)j;
		if((j==EOF) || (j=='\n') || !j)
			break;
	}
#else /* else !LINUX_KLOOGE */
	fgets(buf, sizeof(buf), stdin);
#endif /* end LINUX_KLOOGE */

	/*** If interrupted, issue a dummy fgetc to clear the error ***/
	if(gsc16ao_break_received) {
		gsc16ao_break_received = 0;
		fgetc(stdin);
		return(1);
	}

	if((buf[0] == 'q') && (buf[1] == '\n'))
		return(1);
	
	if(buf[0] == '\n') {
		if((value < low) || (value > high)) {
			fprintf(stderr,
					"ERROR!!! Value 0x%x not in range 0x%x to 0x%x\n",value,low,high);
			goto again;
		}
		*new = value;
		return(0);
	}

	if((buf[0] == '0') && (buf[1] == 'x'))
	  bp = &buf[2];
	else
	  bp = buf;
	
	ret = sscanf(bp ,"%x", &val);

	if(!ret || ret == EOF) {
		fprintf(stderr,"ERROR!!! Invalid hex integer %s\n",buf);
		goto again;
	}

	if((val < low) || (val > high)) {
		fprintf(stderr,"ERROR!!! Value 0x%x not in range 0x%x to 0x%x\n",val,low,high);
 		goto again;
	}
	*new = val;
	return(0);
}

/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
	fprintf(stderr,"\n*** Invalid Argument [%s] ***\n",arg);
	fprintf(stderr,"Usage: gsc16ao_out <device_num 0-9>\n");
	exit(1);
}

