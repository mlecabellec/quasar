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
 * File:         gsc16ao_wave.c                                              *
 *                                                                           *
 * Description:  Analog Output Test                                          *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_wave          ==> start non-interactive analog output test on   *
 *                             on device 0.                                  *
 *   gsc16ao_wave <dn>     ==> start non-interactive analog output test on   *
 *                             on device <dn>. (where dn range is 0 - 9)     *
 *                                                                           *
 * Date:        02/23/2011                                                   *
 * History:                                                                  *
 *                                                                           *
 *   7 02/23/11 D. Dubash                                                    *
 *              Add Auto Calibration option.                                 *
 *                                                                           *
 *   6 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   5 04/20/10 D. Dubash                                                    *
 *              Support for GSC16AO12 board.                                 *
 *                                                                           *
 *   4 01/31/08 D. Dubash                                                    *
 *              change constant 'c' base from 0 to 16                        *
 *                                                                           *
 *   3 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   2 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   1 11/22/05 D. Dubash                                                    *
 *              Created a command driver wave generator application          *
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
#include <unistd.h>
#include <string.h>

#define  LINUX_KLOOGE

#include "gsc16ao_ioctl.h"

#define MAX_BUFFER_SIZE  (131072 * GSC16AO_MAX_CHANNELS)
unsigned int WriteBuffer[MAX_BUFFER_SIZE];

#define BOARD_RESOLUTION    16          /* 16 bits */
#define RESOLUTION          (double)(unsigned int)((1 << BOARD_RESOLUTION))

#define RMS                 1.414

#define	MODE_WRITE_PROGIO	1
#define	MODE_WRITE_DMA		2
#define	MODE_MMAP_DATA_PIO	3
#define	MODE_MMAP_ALL_PIO	4

#define	WAVE_SINE		    1
#define	WAVE_TRIANGLE		2
#define	WAVE_SQUARE		    3
#define	WAVE_STEP		    4
#define	WAVE_SIN_TR_SQ_STEP	5
#define	WAVE_CONSTANT		6

#define DEVICE_NAME                 "gsc16ao"
#define DEF_BOARD_NO                0
#define	DEF_MODE		            MODE_WRITE_DMA
#define DEF_NUM_SAMPLES		        800		/* Default Number of Samples */
#define	DEF_CHANNEL_MASK	        0x1		/* Chan 0 enabled by default */
#define DEF_WAVE_SELECTION          WAVE_SINE
#define DEF_AMPLITUDE               5.0     /* volts */
#define DEF_CONST_VOLTAGE           5.0     /* volts */
#define DEF_BUFFER_SIZE_SELECTION   'f'     /* 131072 */
#define DEF_WAVE_FREQ               0       /* 0=do not compute wave freq */
#define DEF_VOLTAGE_RANGE           10.0    /* voltage range */
#define DEF_FORMAT                  GSC16AO_OFFSET_BINARY

char			buf[200];
char			devname[30];
int			    c;
int			    fp, status;

gsc16ao_gscregs *gscptr;
unsigned long    buffer_size, buf_size_reg;

int buffer_sizes_ao12[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072 };
int buffer_sizes_ao16[] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144 };

/*** global variables ***/
int			current_wave;
double      amplitude;
double      const_voltage;
double      wave_freq;
int         direct_reg_access;
int         board;
int         hw_nchans;
int			mode;
int			sample_freq;
int			num_of_samples;
int         max_chan_mask;
int			chan_mask;
int         *munmap_gscptr;
int         format;
int         autocal;

board_info_t    board_info;

/* prototype */
static ushort _VoltsToData(double volts, int Format, double ConfigVoltageRange);
int  buffer_size_selection(char select);
int  do_wave_selection(char select);
int  run_wave_generation_menu();
int  run_mode_of_operation_menu(char select);
void print_wave_info(char *str, int n_samples);
void Usage(char *prog_p);
void Abort();
int  create_step_wave(unsigned int *wbufptr, int num_samples, int chan_cnt);
int  create_sine_wave(unsigned int *wbufptr, int num_samples, int chan_cnt);
int  create_triangle_wave(unsigned int *wbufptr, int num_samples,int chan_cnt);
int  create_triangle_wave_2comp(unsigned int *wbufptr, int num_samples,int chan_cnt);
int  create_square_wave(unsigned int *wbufptr, int num_samples, int chan_cnt);
void Initialize_Board();
int  board_opened=0;
void Get_Board_Info(int disp);
void Reset_Board();
int  num_active_channels(int chan_mask);
    
/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    extern  char    *optarg;
    extern  int     optind, opterr;
    char    *prog_p = argv[0];
    char    *pt;
    int     option;

	int     chan_cnt;
    int     reset_board=0;

    board           = DEF_BOARD_NO;
    fp              = 0;
    current_wave    = DEF_WAVE_SELECTION;
    amplitude       = DEF_AMPLITUDE;
    const_voltage   = DEF_CONST_VOLTAGE;
    mode            = DEF_MODE;
    sample_freq     = 0;
    num_of_samples  = DEF_NUM_SAMPLES;
    chan_mask       = DEF_CHANNEL_MASK;
    wave_freq       = DEF_WAVE_FREQ;
    format          = DEF_FORMAT; 
    autocal         = 0;
    buffer_size_selection(DEF_BUFFER_SIZE_SELECTION);

    while ((option = getopt( argc, argv, "2a:Ab:B:c:f:F:m:n:Rv:w:" )) != EOF) {
        switch (option) {

        case '2': /* twos complement */
            format = GSC16AO_TWOS_COMP;
        break;

        case 'a':   /* wave amplitude */
            amplitude = strtod(optarg, &pt);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! amplitude not specified\n",
                    prog_p);
                Usage(prog_p);
            }

            /* if volts specified in RMS */
            if((*pt == 'r') || (*pt == 'R')) {
                amplitude = amplitude * RMS;
            }

            if((amplitude < 0.0) || (amplitude > +10.0)) {
                fprintf(stderr,"%s: ERROR! Invalid amplitude %f (%f RMS)\n",
                    prog_p, amplitude, amplitude/RMS);
                Usage(prog_p);
            }
        break;

        case 'A':   /* enable auto-cal */
            autocal++;
        break;

        case 'b':  /* board */      
            board = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! board number not specified\n",
                    prog_p);
                Usage(prog_p);
            }
            if((board < 0) || (board > 9)) {
                fprintf(stderr,"%s: ERROR! Invalid board number %d\n",
                    prog_p, board);
                Usage(prog_p);
            }
    		
            Get_Board_Info(0); 
        break;

        case 'B':  /* buffer selection '0' - 'f' */
            if(optarg[1] != 0) {
                fprintf(stderr,"ERROR! Invalid buffer size selection '%s'\n",optarg);
                Usage(prog_p);
            }

            if(buffer_size_selection(optarg[0])) {
                Usage(prog_p);
            }
        break;

        case 'c':   /* channel mask selection */
            chan_mask = strtoul(optarg,&pt, 16);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! channel mask not specified\n",
                    prog_p);
                Usage(prog_p);
            }
            if(chan_mask <= 0) {
                fprintf(stderr,"%s: ERROR! Invalid channel mask 0x%x\n",
                    prog_p, chan_mask);
                Usage(prog_p);
            }
        break;

        case 'f':   /* sample frequency */
            sample_freq = strtod(optarg, &pt);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! sample frequency not specified\n",
                    prog_p);
                Usage(prog_p);
            }
        break;

        case 'F':  /* Wave Frequency */
            wave_freq = strtod(optarg, &pt);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! wave frequency not specified\n",
                    prog_p);
                Usage(prog_p);
            }

            if((wave_freq < 0.1)) {
                fprintf(stderr,"%s: ERROR! Invalid wave frequency %f\n",
                    prog_p, wave_freq);
                Usage(prog_p);
            }
        break;

        case 'm':  /* mode 'p', 'd', 'm' or 'a' */
            if(optarg[1] != 0) {
                fprintf(stderr,"ERROR! Invalid buffer size selection '%s'\n",optarg);
                Usage(prog_p);
            }

            if(run_mode_of_operation_menu(optarg[0])) {
                Usage(prog_p);
            }
        break;

        case 'n':   /* number of samples */
            num_of_samples = strtoul(optarg,&pt, 0);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! number of samples not specified\n",
                    prog_p);
                Usage(prog_p);
            }
            if((num_of_samples < 1) || (num_of_samples > 131072)) {
                fprintf(stderr,"%s: ERROR! Invalid number of samples %d\n",
                    prog_p, num_of_samples);
                Usage(prog_p);
            }
        break;


        /* reset board, all other options are ignored */
        case 'R':
            reset_board++;
        break;

        case 'v':   /* constant wave voltage */
            const_voltage = strtod(optarg, &pt);
            if(optarg == pt) {
                fprintf(stderr,"%s: ERROR! constant wave voltage not specified\n",
                    prog_p);
                Usage(prog_p);
            }

            /* if volts specified in RMS */
            if((*pt == 'r') || (*pt == 'R')) {
                const_voltage = const_voltage * 1.414;
            }

            if((const_voltage < -10.0) || (const_voltage > +10.0)) {
                fprintf(stderr,"%s: ERROR! Invalid constant wave voltage %f "
                                "(%f RMS)\n",
                    prog_p, const_voltage, const_voltage/RMS);
                Usage(prog_p);
            }
        break;

        case 'w':   /* wave selection 'c', 's', 't', 'x', 'y', 'z' */
            if(optarg[1] != 0) {
                fprintf(stderr,"ERROR! Invalid buffer size selection '%s'\n",optarg);
                Usage(prog_p);
            }

            if(do_wave_selection(optarg[0]))
                Usage(prog_p);
        break;

        default:
            Usage(prog_p);
        break;

        }
    }

    Get_Board_Info(1);

    /* if spureous arguments, error out */
    if(optind != argc) {
        Usage(prog_p);
    }

    if(chan_mask > max_chan_mask) {
	fprintf(stderr,"\n%s: ERROR! Invalid channel mask 0x%x\n\n",
                    prog_p, chan_mask);
	Usage(prog_p);
    }

    if(sample_freq == 0)
            sample_freq = board_info.max_sample_freq; /* set default sample frequency */

    if((sample_freq < board_info.min_sample_freq) || 
                        (sample_freq > board_info.max_sample_freq)) {
        fprintf(stderr,"%s: ERROR! Invalid sample frequency %d\n",
            prog_p, sample_freq);
        Usage(prog_p);
    }

    if((wave_freq > (board_info.max_sample_freq/3))) {
        fprintf(stderr,"%s: ERROR! Invalid wave frequency %f\n",
            prog_p, wave_freq);
        Usage(prog_p);
    }

    /* first see if a wave frequency is specified. If so, then use it to
     * compute the number of samples and frequency.
     */
    if(wave_freq != 0.0) {

        if(wave_freq < 10) {
            sample_freq = 1000;
        } else {
            sample_freq = board_info.max_sample_freq;
        }

        num_of_samples = sample_freq/wave_freq;
        sample_freq = num_of_samples * wave_freq;
    }

    /* more validation */
    if (num_of_samples * num_active_channels(chan_mask) > buffer_size) {
        fprintf(stderr, "WARNING: Buffer size %ld not large enough for %d samples times %d active channels\n",
						buffer_size, num_of_samples, num_active_channels(chan_mask));
        fprintf(stderr, "         Increase buffer size or reduce samples or active channels!!!\n");
        Usage(prog_p);
    }

    chan_cnt = num_active_channels(chan_mask);
    if (chan_cnt == 0) {
            fprintf(stderr, "ERROR!!!: No channels enabled???\n");
            Usage(prog_p);
    }

    if((num_of_samples < 8) && (current_wave == WAVE_SIN_TR_SQ_STEP)) {
        fprintf(stderr, "ERROR!!! Minimum 8 samples required for Sine_Tr_Squ_Step waveave\n");
        Usage(prog_p);
    }

	Initialize_Board();

    /* if board reset requested, do it and exit */
    if(reset_board) {
        fprintf(stderr,"*** Resetting board ***\n");
        Reset_Board();
        exit(0);
    }

    /* set the sample frequency */
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

    /* go generate requested wave */
    run_wave_generation_menu();

    exit(0);
}

/*** Reset Board Routine ***/
void
Reset_Board()
{
  unsigned long parm;
  unsigned long dmaenable;
  
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

  /* set buffer size */
  gscptr->buffer_ops = buf_size_reg;
  
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
    parm = format;
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
        printf("\nAssume that the board '%s' supports a voltage range of +/-10 Volts\n",board_info.board_name);
    }

    if(autocal) {
        /* perform autocal after teh voltage range has changed */
        printf("Auto Calibration Initiated....please wait (3-5 seconds)\n");
        ioctl(fp, (unsigned long)IOCTL_GSC16AO_AUTO_CAL, NULL);
    }
}

void 
Get_Board_Info(int disp)
{
    if(board_opened) {
       close(fp);
       board_opened=0;
    }

    sprintf(devname,"/dev/%s%d",DEVICE_NAME, board);

    /*** Open the device ***/
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
	board_opened = 1;
}

/*** Initialize Board Routine ***/
void
Initialize_Board()
{
    unsigned long int offset;
    
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
    
    Reset_Board();
}

/****************************************************************************
 * buffer size menu                                                         *
 ****************************************************************************/
int
buffer_size_selection(char select)
{
		switch (select) {
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
		  if (select >= '0' && select <= '9')
		    buf_size_reg = select - '0';
		  else
		    buf_size_reg = select - 'a' + 10;

            if(board_info.board_type == GSC_16AO_16) {
                buffer_size = buffer_sizes_ao16[buf_size_reg];
            } else {
                buffer_size = buffer_sizes_ao12[buf_size_reg];
            }

		  break;

        default:
            fprintf(stderr,"Invalid Buffer Size Selection '%c'\n",select);
            return (1);
          break;
        }
       
    return(0);
}

/****************************************************************************
 * Run Mode of Operation Menu                                               *
 ****************************************************************************/
int
run_mode_of_operation_menu(char select)
{
    switch(select) {
    case 'p':	/* write() [ Programmed I/O ] */
      mode = MODE_WRITE_PROGIO;
      break;
      
    case 'd':	/* write() [ DMA ] */
      mode = MODE_WRITE_DMA;
      break;
      
    case 'm':	/* mmap() [ User-mode access I/O] */
      mode = MODE_MMAP_DATA_PIO;
      break;
      
    case 'a':	/* mmap() [ User-mode access for all I/O] */
      mode = MODE_MMAP_ALL_PIO;
      break;
      
    default:
      fprintf(stderr,"ERROR!!! Invalid Mode <%c>\n",select);
      return(1);
      break;
    } /* switch */

    return(0);
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
            } else {
				if(status != num_samples*4) {
				  fprintf(stderr,"Write Failed, rv=%d, expected %d\n", status, num_samples*4);
				  
				  parm = 0;
				  status = ioctl(fp, IOCTL_GSC16AO_GET_DEVICE_ERROR, &parm);
				  if (status) {
				    fprintf(stderr,"ioctl(IOCTL_GSC16AO_GET_DEVICE_ERROR) failed\n");
				    exit(1);
				  }
				  fprintf(stderr,"               extended device error code: %ld\n", parm);
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

#define MAXSHORT 0xffff

short testsig_check_overrange(int tmp)
{
    if(abs(tmp) > MAXSHORT) {
        if(tmp < 0) 
            tmp = -(MAXSHORT);
        else
            tmp = MAXSHORT;
    }
    return (short) tmp;
}

int
create_sine_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int i, j, n, volts;
	double  x;
    unsigned int  amp;
    double  rad_arg;
    int     buf_index=0;
    int     tmps;

                
    amp = _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE) - _VoltsToData(0.0, format, DEF_VOLTAGE_RANGE);

	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! SINE: Minimum of 2 samples required\n");
		return(0);
	}

    if(format == GSC16AO_TWOS_COMP) {
        j = 0;
        while(buf_index < num_samples) {
            rad_arg = (2 * M_PI) * ((double) buf_index/(double)num_samples);
            tmps = (int) ((double)amp * (double)sin(rad_arg));
    		for (n=0; n<chan_cnt; n++)
                wbufptr[j++] = testsig_check_overrange(tmps);
            buf_index++;
        }
        print_wave_info("Sine",j);
        return(j);
    } else {
    	j = 0;
    	x = 0.0;
    	for(i=0; i < num_samples ;i++) {
    		volts = 0xffff & (int)(sin(x) * amp);
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
        print_wave_info("Sine",j);
    	return(j);
    }
}

int
create_triangle_wave_2comp(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
    double      ramp_delta, capture_interval, sig_max, sig_min, cur_val;
    int         num_samples_mod4, buf_index;
    int         amp, j, n;

    buf_index = 0;
    amp = _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE) - _VoltsToData(0.0, format, DEF_VOLTAGE_RANGE);

    ramp_delta = (4.0 * amp) / (double) num_samples;
    capture_interval = 0.1 * ramp_delta;

    sig_max = amp;
    sig_min = -amp;

    cur_val = 0;
    j = 0;
    for(n=0; n < chan_cnt; n++)
        wbufptr[j++] = testsig_check_overrange(cur_val);
    num_samples_mod4 = num_samples % 4;

    while ((cur_val + ramp_delta) <= (sig_max + capture_interval)) {
        cur_val += ramp_delta;
        for(n=0; n < chan_cnt; n++)
            wbufptr[j++] = testsig_check_overrange((int)cur_val);
        buf_index++;
    }

    if (num_samples_mod4 != 0) {
        cur_val = sig_max - ((cur_val + ramp_delta) - sig_max);
        for(n=0; n < chan_cnt; n++)
            wbufptr[j++] = testsig_check_overrange((int)cur_val);
        buf_index++;
    }
    while ((cur_val - ramp_delta) >= (sig_min - capture_interval)) {
        cur_val -= ramp_delta;
        for(n=0; n < chan_cnt; n++)
            wbufptr[j++] = testsig_check_overrange((int)cur_val);
        buf_index++;
    }
    if (num_samples_mod4 != 0) {
        cur_val = sig_min + (sig_min - (cur_val - ramp_delta));
        for(n=0; n < chan_cnt; n++)
            wbufptr[j++] = testsig_check_overrange((int)cur_val);
        buf_index++;
    }
    while (buf_index < num_samples - 1) {
        cur_val += ramp_delta;
        for(n=0; n < chan_cnt; n++)
            wbufptr[j++] = testsig_check_overrange((int)cur_val);
        buf_index++;
    }

    print_wave_info("Triangle",j);

    return(j);
}

int
create_triangle_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n, up, down;
	double	incr;
    unsigned    int amp;
    int         hold, max_offset, bias;
                
    max_offset = _VoltsToData(10.0, format, DEF_VOLTAGE_RANGE);
    bias = max_offset - _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE);
    amp = _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE) - _VoltsToData(0.0, format, DEF_VOLTAGE_RANGE);

	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! TRIANGLE: Minimum of 2 samples required\n");
		return(0);
	}
	up = (num_samples/2 + 1);
	down = (num_samples/2 - 1);
	j = 0;
	incr = (amp * 2)/(up-1);	
	for(i=0; i < up; i++)	/* 300 points i.e. 0 - 299 */
		for (n=0; n<chan_cnt; n++) {
            hold = (int) (i*incr);
            if(hold > max_offset)
			    wbufptr[j++] = max_offset;
            else
			    wbufptr[j++] = hold;
        }
	
	for(i=down; i > 0; i--)	/* 298 points i.e. 298 - 1 */
		for (n=0; n<chan_cnt; n++)
			wbufptr[j++] = (int) i*incr;

    /* add bias to center the triangle */
    for(i=0; i < j; i++) {
			wbufptr[i] += bias;
    }
	
    print_wave_info("Triangle",j);

	return(j);
}

int
create_square_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n;
    unsigned    int high, low;
                
    high = _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE);
    low = _VoltsToData(-amplitude, format, DEF_VOLTAGE_RANGE);

	if(num_samples < 2) {
		fprintf(stderr,"ERROR!!! SQUARE: Minimum of 2 samples required\n");
		return(0);
	}
	j = 0;
	for(i=0; i < num_samples/2; i++)
		for (n=0; n<chan_cnt; n++)
		  	wbufptr[j++] = high;
	for(i=0; i < num_samples/2; i++)
		for (n=0; n<chan_cnt; n++)
			wbufptr[j++] = low;

    print_wave_info("Square",j);

	return(j);
}

int
create_step_wave(unsigned int *wbufptr, int num_samples, int chan_cnt)
{
	int	i, j, n, step;
	double	incr;
    unsigned    int amp;
    int         max_offset, bias;
    int         two_comp_adjust;
                
    max_offset = _VoltsToData(10.0, format, DEF_VOLTAGE_RANGE);
    bias = max_offset - _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE);
    amp = _VoltsToData(amplitude, format, DEF_VOLTAGE_RANGE) - _VoltsToData(0.0, format, DEF_VOLTAGE_RANGE);

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
			incr += (amp * 2)/((num_samples/step)-1);

		for (n=0; n<chan_cnt; n++)
			  wbufptr[j++] = incr;
	}

    if(format == GSC16AO_TWOS_COMP)
        two_comp_adjust = 0x8000;
    else
        two_comp_adjust = 0x0000;

    /* add bias to center the step */
    for(i=0; i < j; i++) {
			wbufptr[i] += two_comp_adjust | bias;
    }
	
    print_wave_info("Step",j);

	return(j);
}

int
do_wave_selection(char select)
{
		switch(select) {
			case 'c':	/* CONSTANT Wave */
				current_wave = WAVE_CONSTANT;
			break;

			case 's':	/* SINE wave */
				current_wave = WAVE_SINE;
			break;

			case 't':	/* Triangle wave */
				current_wave = WAVE_TRIANGLE;
			break;

			case 'x':	/* Square wave */
				current_wave = WAVE_SQUARE;
			break;

			case 'y':	/* Step wave */
				current_wave = WAVE_STEP;
			break;

			case 'z':	/* Sine, Triangle, Square & Step wave */
				current_wave = WAVE_SIN_TR_SQ_STEP;
			break;

			default:
				fprintf(stderr,"ERROR!!! Invalid Wave Selection <%c>\n",select);
                return(1);
			break;
		}
        return(0);
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
	unsigned   data;
	
    chan_cnt = num_active_channels(chan_mask);

    switch(current_wave) {
        case WAVE_CONSTANT:
                data = _VoltsToData(const_voltage, format, DEF_VOLTAGE_RANGE);
				for (i=0; i<chan_cnt; i++)
				    WriteBuffer[i] = data;
				
	            fprintf(stderr,"Constant Voltage: %.2f volts\n",const_voltage);
				generate_wave(chan_cnt);
        break;

        case WAVE_SINE:
				generate_wave(create_sine_wave(WriteBuffer,num_of_samples,chan_cnt));
        break;

        case WAVE_TRIANGLE:
                if(format == GSC16AO_TWOS_COMP)
				    generate_wave(create_triangle_wave_2comp(WriteBuffer,num_of_samples,chan_cnt));
                else
				    generate_wave(create_triangle_wave(WriteBuffer,num_of_samples,chan_cnt));
        break;

        case WAVE_SQUARE:
				generate_wave(create_square_wave(WriteBuffer,num_of_samples,chan_cnt));
        break;

        case WAVE_STEP:
				generate_wave(create_step_wave(WriteBuffer,num_of_samples,chan_cnt));
        break;

        case WAVE_SIN_TR_SQ_STEP:
				current_wave = WAVE_SIN_TR_SQ_STEP;
				bpt = WriteBuffer;
				ns = create_sine_wave(bpt,num_of_samples/4,chan_cnt);	/* sine */
				bpt += ns;
                if(format == GSC16AO_TWOS_COMP)
				    ns = create_triangle_wave_2comp(bpt,num_of_samples/4,chan_cnt);
                else
				    ns = create_triangle_wave(bpt,num_of_samples/4,chan_cnt);
				bpt += ns;
				ns = create_square_wave(bpt,num_of_samples/4,chan_cnt);	/* square */
				bpt += ns;
				ns = create_step_wave(bpt,num_of_samples/4,chan_cnt);	/* step */
				bpt += ns;
				generate_wave(bpt - WriteBuffer);
        break;
    }

    return(0);
}

/**************************************************
 * _VoltsToData()
 **************************************************/
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

void
Abort()
{
	if(fp) {
		Reset_Board();
		/*** unmap GSC area ***/
		munmap((void *)munmap_gscptr, GSC16AO_GSC_REGS_MMAP_SIZE);
        close(fp);
	}
    exit(1);
}

void
print_wave_info(char *str, int n_samples)
{
	fprintf(stderr,"  %s Wave: %.2f volts (%.2f RMS) (p-p=%.2f volts)\n"
                   "  Wave Frequency: %0.1f Hz - %d samples\n", 
                        str, amplitude, amplitude/RMS, amplitude*2, 
                        (((double)sample_freq/(double)n_samples) *
                        (double)num_active_channels(chan_mask)), n_samples);
}

void
Usage(char *prog_p)
{
    if(board_opened == 0)
        Get_Board_Info(0);

    fprintf(stderr, "Usage: %s -[2abBcfFmnvw]\n", prog_p);
    
    fprintf(stderr,"      -2 TWOs Complement\n");
    fprintf(stderr,"      -a <wave amplitude><r> [0 - 10.0] (volts)\n");
    fprintf(stderr,"      -A                     Enable Auto Calibration\n");
    fprintf(stderr,"      -b <board number>      [0 - 9]\n");
    fprintf(stderr,"      -B <buffer select>     ['0' - 'f']\n");
    fprintf(stderr,"      -c <channel mask>      [0x001 - 0x%x]\n",(max_chan_mask));
    fprintf(stderr,"      -f <sample freq>       [%d - %d]\n",
                                        board_info.min_sample_freq,
                                        board_info.max_sample_freq);
    fprintf(stderr,"      -F <Wave freq>         [0.1 - %.0f]\n",
                                        (double)board_info.max_sample_freq/3);
    fprintf(stderr,"      -m <run mode>          ['p','d','m','a']\n");
    fprintf(stderr,"      -n <num. samples>      [1 - 131072]\n");
    fprintf(stderr,"      -R                     Reset Board and abort\n");
    fprintf(stderr,"      -v <const. volts>      [-10.0 - +10.0] (volts)\n");
    fprintf(stderr,"      -w <wave select>       ['c','s','t','x','y','z']\n");

    fprintf(stderr,"\n");
    fprintf(stderr,"    amplitude: append volts with 'r' or 'R' for RMS volts\n");
    fprintf(stderr,"buffer select: 0=4    1=8    2=16   3=32   4=64    5=128   6=256   7=512\n");
    fprintf(stderr,"               8=1024 9=2048 a=4096 b=8192 c=16384 d=32768 e=65536 f=131072\n");
    fprintf(stderr,"               *** DOUBLE BUFFER SIZE FOR GSC16AO16 BOARDS ***\n\n");
    fprintf(stderr,"    wave freq: create a wave matching this frequency. Sample frequency\n"
                   "               and number of samples are ignored\n");
    fprintf(stderr,"     run mode: p=program i/o, d=dma, m=mmap data, a=mmap all\n");
    fprintf(stderr," const. volts: append volts with 'r' or 'R' for RMS volts\n");
    fprintf(stderr,"  wave select: c=constant, s=sine, t=triangle, x=square, y=step\n");
    fprintf(stderr,"               z=sine,triangle,square,step\n");

    exit(1);
}
