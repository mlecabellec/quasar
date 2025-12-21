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
 * File:         gsc16ao_tst.c                                               *
 *                                                                           *
 * Description:  Interactive GSC16AO Driver Test                             *
 *                                                                           *
 * Syntax:                                                                   *
 *   gsc16ao_tst      ==> start interactive analog output test on device 0.  *
 *   gsc16ao_tst <dn> ==> start interactive analog output test on device <dn>*
 *                      (where dn range is 0 - 9)                            *
 *                                                                           *
 * Date:        11/17/2010                                                   *
 * History:                                                                  *
 *                                                                           *
 *   5 11/17/10 D. Dubash                                                    *
 *              Use hw_nchans instead of max_channels.                       *
 *                                                                           *
 *   4 02/27/09 D. Dubash                                                    *
 *              Get rid of warnings                                          *
 *                                                                           *
 *   3 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   2 11/22/05 D. Dubash                                                    *
 *              Cleanup warnings.                                            *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Adapapted from lcaio_test application                        *
 *                                                                           *
 *****************************************************************************/

/*
 * File : gsc16ao_tst.c
 *
 * Description : Test application for Linux Host driver for General Standards
 *               Corp 16AO12 analog output card.
 *
 * Comments : This application can be executed to test all the IOCTLS and also
 *            driver write functions.
 *
 * Revision History :
 *
 * 5-13-2003: Adapted for lcaio test application
 */

/*
 * Headers
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>


#include "gsc16ao_ioctl.h"

#define MIN_NRATE (board_info.master_clock/board_info.max_sample_freq)
#define MAX_NRATE (board_info.master_clock/board_info.min_sample_freq)

#define MIN_FGEN (board_info.min_sample_freq/1000.0) /* 0.45777 - 16AO12 */
#define MAX_FGEN (board_info.max_sample_freq/1000.0) /* 400.000 - 15AO12 */

#define GEN_MULT   0.063875
#define GEN_OFFSET 511.0

#define ROUND(fValue) (((fValue - floor(fValue)) >= 0.5) ? ceil(fValue) : floor(fValue))

#define ROUND_TO_unsigned_long(fValue) ((unsigned long) ((long) ROUND(fValue)))

/* Converts a clock frequency in KHz to a Nrate value */
#define Fgen_To_Nrate(Fgen) ((Fgen < MIN_FGEN) ? MAX_NRATE : ((Fgen > MAX_FGEN) ? MIN_NRATE : ROUND_TO_unsigned_long(board_info.master_clock/Fgen)))

#define ERROR_SUCCESS      0
#define INVALID_INT_VALUE -1

#define WRITE_BUFFER_SIZE 0x100000

unsigned int WriteBuffer[WRITE_BUFFER_SIZE];

/* Pattern Types */

#define FIXED_PATTERN        1
#define	INCREMENTING_PATTERN 2
#define INVERTING_PATTERN    3
#define WALKING_ONES         4
#define WALKING_ZEROS        5
#define SPECIFIC_PATTERN     6

/* Local Register Types */

#define DMA_REGISTERS          0
#define LOCAL_CONFIG_REGISTERS 1
#define RUN_TIME_REGISTERS     2
#define MESSAGE_REGISTERS      3

/*
 * Global data
 */
int  fd;
unsigned int ulDataMask = 0xFFFF;
unsigned char bExitIntThread;

unsigned int WriteNotify = 0;
unsigned int WriteNotifyDone = 0;
unsigned char flush[100];
unsigned int signal_received;
unsigned long boardType = 0xffffffff;
board_info_t    board_info;
int             hw_nchans;

/*
 * Constants
 */
const char *pszRegisterNames[] = {
    "Board Control Register                  ",
    "Channel Selection Register              ",
    "Sample Rate Register                    ",
    "Buffer Operations Register              ",
    "Firmware and Options Register           ",
    "Autocal Values (maintenance ref. only)  ",
    "Output Data Buffer Register             ",
    "Adjustable Clock Control Register       ",
};

const char *pszPCIConfigRegisterNames[] = {
	"Device/Vendor ID Register               ",
	"Status/Command Register                 ",
	"Class Code/Revision ID Register         ",
	"BIST/Header Type/Lat Cache Size Register",
	"PCI Base Address 0 Register             ",
	"PCI Base Address 1 Register             ",
	"PCI Base Address 2 Register             ",
	"PCI Base Address 3 Register             ",
	"PCI Base Address 4 Register             ",
	(char *) NULL,
	"Cardbus CIS Pointer Register            ",
	"Subsystem ID/Vendor ID Register         ",
	"PCI Base Address Local ROM Register     ",
	"PCI Capability List offset              ",
	(char *) NULL,
	"Lat Gnt/Int Pin Line Register           "
};

unsigned int bufsizes_ao12[] = { 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072 };

unsigned int bufsizes_ao16[] = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144 };

typedef struct RegStruct
{
	const char *pszRegisterName;
	int         iRegOffset;
} REG_STRUCT, *PREG_STRUCT;

const REG_STRUCT DMA_Regs[] = {
	{"DMA_CH_0_MODE",           32},
	{"DMA_CH_0_PCI_ADDR",       33},
	{"DMA_CH_0_LOCAL_ADDR",     34},
	{"DMA_CH_0_TRANS_BYTE_CNT", 35},
	{"DMA_CH_0_DESC_PTR",       36},
	{"DMA_CH_1_MODE",           37},
	{"DMA_CH_1_PCI_ADDR",       38},
	{"DMA_CH_1_LOCAL_ADDR",     39},
	{"DMA_CH_1_TRANS_BYTE_CNT", 40},
	{"DMA_CH_1_DESC_PTR",       41},
	{"DMA_CMD_STATUS",          42},
	{"DMA_MODE_ARB_REG",        43},
	{"DMA_THRESHOLD_REG",       44},
	{(char *) NULL,              0}
};

const REG_STRUCT Local_Config_Regs[] = {
	{"PCI_TO_LOC_ADDR_0_RNG",        0},
	{"LOC_BASE_ADDR_REMAP_0",        1},
	{"MODE_ARBITRATION",             2},
	{"BIG_LITTLE_ENDIAN_DESC",       3},
	{"PCI_TO_LOC_ROM_RNG",           4},
	{"LOC_BASE_ADDR_REMAP_EXP_ROM",  5},
	{"BUS_REG_DESC_0_FOR_PCI_LOC",   6},
	{"DIR_MASTER_TO_PCI_RNG",        7},
	{"LOC_ADDR_FOR_DIR_MASTER_MEM",  8},
	{"LOC_ADDR_FOR_DIR_MASTER_IO",   9},
	{"PCI_ADDR_REMAP_DIR_MASTER",   10},
	{"PCI_CFG_ADDR_DIR_MASTER_IO",  11},
	{"PCI_TO_LOC_ADDR_1_RNG",       92},
	{"LOC_BASE_ADDR_REMAP_1",       93},
	{"BUS_REG_DESC_1_FOR_PCI_LOC",  94},
	{(char *) NULL,                  0}
};

const REG_STRUCT Run_Time_Regs[] = {
	{"MAILBOX_REGISTER_0",       16},
	{"MAILBOX_REGISTER_1",       17},
	{"MAILBOX_REGISTER_2",       18},
	{"MAILBOX_REGISTER_3",       19},
	{"MAILBOX_REGISTER_4",       20},
	{"MAILBOX_REGISTER_5",       21},
	{"MAILBOX_REGISTER_6",       22},
	{"MAILBOX_REGISTER_7",       23},
	{"PCI_TO_LOC_DOORBELL",      24},
	{"LOC_TO_PCI_DOORBELL",      25},
	{"INT_CTRL_STATUS",          26},
	{"PROM_CTRL_CMD_CODES_CTRL", 27},
	{"DEVICE_ID_VENDOR_ID",      28},
	{"REVISION_ID",              29},
	{"MAILBOX_REG_0",            30},
	{"MAILBOX_REG_1",            31},
	{(char *) NULL,               0}
};

const REG_STRUCT Message_Regs[] = {
	{"OUT_POST_Q_INT_STATUS", 12},
	{"OUT_POST_Q_INT_MASK",   13},
	{"IN_Q_PORT",             16},
	{"OUT_Q_PORT",            17},
	{"MSG_UNIT_CONFIG",       48},
	{"Q_BASE_ADDR",           49},
	{"IN_FREE_HEAD_PTR",      50},
	{"IN_FREE_TAIL_PTR",      51},
	{"IN_POST_HEAD_PTR",      52},
	{"IN_POST_TAIL_PTR",      53},
	{"OUT_FREE_HEAD_PTR",     54},
	{"OUT_FREE_TAIL_PTR",     55},
	{"OUT_POST_HEAD_PTR",     56},
	{"OUT_POST_TAIL_PTR",     57},
	{"Q_STATUS_CTRL_REG",     58},
	{(char *) NULL,            0}
};

const REG_STRUCT * const LocalRegs[4] = {
	DMA_Regs,
	Local_Config_Regs,
	Run_Time_Regs,
	Message_Regs
};

const char *pszDeviceError[10] = {
	"GSC16AO Success",
	"GSC16AO Invalid Parameter",
	"GSC16AO Invalid Buffer Size",
	"GSC16AO PIO Timeout",
	"GSC16AO DMA Timeout",
	"GSC16AO Ioctl Timeout",
	"GSC16AO Operation Cancelled",
	"GSC16AO Resource Allocation Error",
	"GSC16AO Invalid Request"
        "GSC16AO Autocalibration Failure"
};

char		devname[]="/dev/gsc16ao0";

struct sigaction io_act;
void signal_initialize();
void sigio_handler (int );
void print_main_menu(int fd);
void BadArg(char *arg);
void Get_Board_Info(int disp);

int GetLastError();

static  unsigned long eGetGSC16AOConfigRegister()
{

	unsigned int iCh;
	unsigned int iRegister;

	do
	{
		printf("\n");
		for (iRegister = GSC16AO_PCI_IDR;
		     iRegister <= GSC16AO_PCI_ILR_IPR_MGR_MLR;
		     iRegister++)
		{
			if (pszPCIConfigRegisterNames[iRegister])
			{
			   printf("%c = %s\n", (char) (((int) 'A') + iRegister),
			   pszPCIConfigRegisterNames[iRegister]);
			}
		}
		printf("\nEnter Selection: ");
                /* getchar(); */
		iCh = getchar();
		getchar(); /* dummy read for 'nl' */
		printf("\n");

		if (islower(iCh))
		{
			iRegister = iCh - ((int) 'a');
		}
		else
		{
			iRegister = iCh - ((int) 'A');
		}

                if ((iRegister > GSC16AO_PCI_ILR_IPR_MGR_MLR) || (! pszPCIConfigRegisterNames[iRegister]))
		{
			printf("Invalid Selection!\n");
		}
	} while ((iRegister > GSC16AO_PCI_ILR_IPR_MGR_MLR) || (! pszPCIConfigRegisterNames[iRegister]));

	return (( unsigned long) iRegister);

}  /* eGetGSC16AOConfigRegister */




static  unsigned long eGetGSC16AOLocalRegister ( unsigned long *reg,
                                              unsigned long *localreg_type)
{

	int               iCh;
	int               iRegister;
	int               iRegisterType;
	unsigned int              uiNumRegs;
	REG_STRUCT const *pRegs;

	do
	{
		printf("\n");
		printf("A = DMA Registers\n");
		printf("B = Local Configuration Registers\n");
		printf("C = Run Time Registers\n");
		printf("D = Message Queue Registers\n");
		printf("\nEnter Register Type: ");

                /* getchar(); */
		iCh = getchar();
		getchar(); /* dummy read for 'nl' */
		printf("\n");
		if (islower(iCh))
		{
			iRegisterType = iCh - ((int) 'a');
		}
		else
		{
			iRegisterType = iCh - ((int) 'A');
		}

		if ((iRegisterType < DMA_REGISTERS) ||
			(iRegisterType > MESSAGE_REGISTERS))
		{
			printf("Invalid Selection!\n");
		}
	} while ((iRegisterType < DMA_REGISTERS) || (iRegisterType > MESSAGE_REGISTERS));

	pRegs = LocalRegs[iRegisterType];

	do
	{
		printf("\n");
		uiNumRegs = 0;
		for (iRegister=0;pRegs[iRegister].pszRegisterName;iRegister++)
		{
			uiNumRegs++;
			printf("%c = %s\n", (char) (((int) 'A') + iRegister),
				   pRegs[iRegister].pszRegisterName);
		}
		printf("\nEnter Selection: ");
                /* getchar(); */
		iCh = getchar();
		getchar(); /* dummy read for 'nl' */
		printf("\n");
		if (islower(iCh))
		{
			iRegister = iCh - ((int) 'a');
		}
		else
		{
			iRegister = iCh - ((int) 'A');
		}

		if ((iRegister < 0) || (iRegister >= ((int) uiNumRegs)))
		{
			printf("Invalid Selection!\n");
		}
	} while ((iRegister < 0) || (iRegister > ((int) uiNumRegs)));

        /* *********** Start of Modification : 21-12-01 **********/

        *reg = iRegister;
        *localreg_type = iRegisterType;

        /* *********** End of Modification : 21-12-01 ************/

	return ((unsigned long)pRegs[iRegister].iRegOffset|GSC16AO_PLX_REGISTER);

}  /* eGetGSC16AOLocalRegister */


static  unsigned long eGetGSC16AORegister()
{

	unsigned int iCh;
	unsigned int iRegister;

	do
	{
		printf("\n");
		for (iRegister = GSC16AO_GSC_BCR; iRegister <= GSC16AO_GSC_ACLK; iRegister++)
		{
			printf("%c = %s\n", (char) (((int) 'A') + iRegister),
				   pszRegisterNames[iRegister]);
		}
		printf("\nEnter Selection: ");
                /* getchar(); */
		iCh = getchar();
		getchar(); /* dummy read for 'nl' */
		printf("\n");
		if (islower(iCh))
		{
		  iRegister = iCh - ((int) 'a');
		}
		else
		{
			iRegister = iCh - ((int) 'A');
		}

		if (iRegister > GSC16AO_GSC_ACLK)
		{
			printf("Invalid Selection!\n");
		}
	} while (iRegister > GSC16AO_GSC_ACLK);

	return (( unsigned long) iRegister);

}  /* eGetGSC16AORegister */


static  unsigned long ulGetRegisterValue()
{

	 unsigned long ulValue;

	while(1)
	{
		printf("Enter Register Value (hex): ");
		if(scanf("%lx", &ulValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
		}
		else
			break;
	} 

	getchar(); /* flush the new-line character after scanf */

	return (ulValue);

}  /* ulGetRegisterValue */


static float GetFloat(char *pszPrompt, float fMin, float fMax)
{
    float fValue;

	do
	{
		/* Prompt user for a floating point value. */
		printf(pszPrompt);

		/* Read the value. */
		if(scanf("%f", &fValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
			continue;
		}

		/* Check Value */
		if ((fValue < fMin) || (fValue > fMax))
		{
			printf("Invalid Entry!\n\n");
		}
	} while ((fValue < fMin) || (fValue > fMax));

	getchar(); /* flush the new-line character after scanf */

    /* Return the value. */
    return(fValue);

}  /* GetFloat */


static  unsigned long GetHexLong(char *pszPrompt)
{

     unsigned long ulValue;

	while(1)
	{
    	/* Prompt user for a hex long value. */
    	printf(pszPrompt);

    	/* Read the value. */
		if(scanf("%lx", &ulValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
		}
		else
			break;
	}

	getchar(); /* flush the new-line character after scanf */

    /* Return the value. */
    return(ulValue);

}  /* GetHexLong */


unsigned short GetHexShort(char *pszPrompt)
{

	unsigned short uhValue;

    /* Read the value from the user. */
	while(1)
	{
    	/* Prompt the user for a hex short value. */
    	printf(pszPrompt);

    	/* Read the value. */
		if(scanf("%hx", &uhValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
		}
		else
			break;
	}

	getchar(); /* flush the new-line character after scanf */

    /* Return the value. */
    return(uhValue);

}  /* GetHexShort */


static unsigned int GetInt(char *pszPrompt, unsigned int uiLowerLimit, unsigned int uiUpperLimit)
{

    unsigned int uiValue;

    /* Loop through asking the user for an integer until the user enters a
       value within the given range. */
    do
    {
		/* Prompt for an integer. */
		printf(pszPrompt);

		/* Read the integer. */
		if(scanf("%d", &uiValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
		}
    } while((uiValue < uiLowerLimit) || (uiValue > uiUpperLimit));

	getchar(); /* flush the new-line character after scanf */

    /* Return the value entered. */
    return(uiValue);

}  /* GetInt */


static long GetLong(char *pszPrompt)
{

    long lValue;

	/* Read the long. */
	while(1)
	{
		/* Prompt for an integer. */
		printf(pszPrompt);

    	/* Read the value. */
		if(scanf("%ld", &lValue) == 0) {
			scanf("%s",(char *)&flush);
			fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
		}
		else
			break;
	}

	getchar(); /* flush the new-line character after scanf */

    /* Return the value entered. */
    return(lValue);

}  /* GetLong */


static unsigned char YorN(char *pszPrompt)
{
	char iCh;
	char Ch1;
        unsigned char ret;

	printf(pszPrompt);

	Ch1 = getchar();
	iCh = getchar();

	printf("\n");

        if (( iCh == 'Y' )  || (iCh == 'y')) ret = 1;
        if (( iCh == 'N' )  || (iCh == 'n')) ret = 0;

        if ( ret != 0 || ret != 1 )
          {
             if (( Ch1 == 'Y' )  || ( Ch1 == 'y')) ret = 1;
             if (( Ch1 == 'N' )  || ( Ch1 == 'n')) ret = 0;
          }

        return ret;

}  /* YorN */


/*
 * This helper routine converts a system-service error
 * code into a text message and prints it on StdOutput
 */
static void ErrorMessage(char *lpOrigin, unsigned int dwMessageId)
{

	char   *pszMsgPtr;

	if ((dwMessageId >= GSC16AO_SUCCESS) &&
		(dwMessageId <= GSC16AO_ERR_AUTOCAL_FAILED))
	{
		pszMsgPtr = (char *) pszDeviceError[dwMessageId-GSC16AO_SUCCESS];
	}
#ifdef DEBUG
        printf ("msg id = %x\n",dwMessageId);
#endif /* end DEBUG */

	if (pszMsgPtr)
	{
		printf("Error: %s -- %s\n", lpOrigin, pszMsgPtr);
	}
	else
	{
		printf("FormatMessage error: %d\n", GetLastError());
	}

}  // ErrorMessage

int GSC16AO_close(int fp)
{
	int status;

	status	= close(fp);
	if (status < 0)
	  printf("close() failure, errno = %d\n", errno);
	
	return(status);
}

int GSC16AO_open()
{
	int fp;

	fp = open(devname, O_RDWR);

	if (fp < 0) {
	  printf("open() failure on %s: [%s]\n",devname, strerror(errno));
	  exit(1);
	}
	
	return(fp);
}

int GSC16AO_write(int fp, const void *buf, size_t count)
{
	int	status;

	status	= write(fp, buf, count);

	if (status == -1)
		printf("write() failure, errno = %d [%s]\n", errno,strerror(errno));
	else if (status != count)
		printf("write() %d of %d.\n", status, (int) count);

	return(status);
}


int GSC16AO_ioctl_no_command(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_NO_COMMAND, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_NO_COMMAND) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_init_board(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_INIT_BOARD, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_INIT_BOARD) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_read_register(int fp, unsigned short reg_set, unsigned short reg_no, unsigned long *pValue)
{
	struct register_params parm;
	int status;

	parm.regset	= reg_set;
	parm.regnum	= reg_no;
	parm.regval	= 0xDEADBEEF;
	
	status	= ioctl(fp, IOCTL_GSC16AO_READ_REGISTER, (unsigned long)&parm);
	if (status < 0) {
	  printf("ioctl(IOCTL_GSC16AO_READ_REGISTER) failure, errno = %d\n", errno);
	  return (GetLastError());
	}

	*pValue = parm.regval;
	return(GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_read_gsc_register(int fp, unsigned short reg_no, unsigned long* pValue)
{
  return GSC16AO_ioctl_read_register(fp, GSC16AO_GSC_REGISTER, reg_no, pValue);
}

int GSC16AO_ioctl_read_pci_register(int	fp, unsigned short reg_no, unsigned long* pValue)
{
  return GSC16AO_ioctl_read_register(fp, GSC16AO_PCI_REGISTER, reg_no, pValue);
}

int GSC16AO_ioctl_read_plx_register(int	fp, unsigned short reg_no, unsigned long* pValue)
{
  return GSC16AO_ioctl_read_register(fp, GSC16AO_PLX_REGISTER, reg_no, pValue);
}

int GSC16AO_ioctl_write_register(int fp, unsigned short	reg_set, unsigned short	reg_no, unsigned long value)
{
	struct register_params parm;
	int status;

	parm.regset = reg_set;
	parm.regnum = reg_no;
	parm.regval = value;

	status	= ioctl(fp, IOCTL_GSC16AO_WRITE_REGISTER, (unsigned long)&parm);
	if (status < 0) {
	  printf("ioctl(IOCTL_GSC16AO_WRITE_REGISTER) failure, errno = %d\n", errno);
	  return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_write_gsc_register(int fp, unsigned short reg_no, unsigned long value)
{
  return GSC16AO_ioctl_write_register(fp, GSC16AO_GSC_REGISTER, reg_no, value);
}

int GSC16AO_ioctl_write_pci_register(int fp, unsigned short reg_no, unsigned long value)
{
  return GSC16AO_ioctl_write_register(fp, GSC16AO_PCI_REGISTER, reg_no, value);
}

int GSC16AO_ioctl_write_plx_register(int fp, unsigned short reg_no, unsigned long value)
{
  return GSC16AO_ioctl_write_register(fp, GSC16AO_PLX_REGISTER, reg_no, value);
}

int GSC16AO_ioctl_set_debug_flags(int fp, unsigned long classes, unsigned long level)
{
	struct gsc_debug_flags parm;
	int status;

	parm.db_classes = classes;
	parm.db_level   = level;

	status	= ioctl(fp, IOCTL_GSC16AO_SET_DEBUG_FLAGS, (unsigned long)&parm);
	if (status < 0) {
	  printf("ioctl(IOCTL_GSC16AO_SET_DEBUG_FLAGS) failure, errno = %d\n", errno);
	  return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_device_error(int fp, unsigned long *pError)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_DEVICE_ERROR, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_DEVICE_ERROR) failure, errno = %d\n", errno);
    return(-1);
  }

  if (pError)
    *pError = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_set_write_mode(int fp, unsigned long dmaEnable)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SET_WRITE_MODE, &dmaEnable);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SET_WRITE_MODE) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_auto_cal(int fp)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_AUTO_CAL, NULL);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_AUTO_CAL) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_program_rate_gen(int fp, float freq)
{
	int	status;
        unsigned long Nrate;

        Nrate = Fgen_To_Nrate (freq);

	status	= ioctl(fp, IOCTL_GSC16AO_PROGRAM_RATE_GEN, &Nrate);
	if (status > 0) {
		printf("ioctl(IOCTL_GSC16AO_PROGRAM_RATE_GEN) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_active_chan(int fp, unsigned long chanmask)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_ACTIVE_CHAN, &chanmask);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_ACTIVE_CHAN) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_set_out_buffer_size(int fp, unsigned long bufsize)
{
  int status;
  unsigned long parm;
  int nentries;

  if(board_info.board_type == GSC_16AO_16) {
     nentries  = sizeof(bufsizes_ao16)/sizeof(unsigned int);
     for (parm=0; parm < nentries; parm++) {
       if (bufsize == bufsizes_ao16[parm])
         break;
     }
  } else {
     nentries  = sizeof(bufsizes_ao12)/sizeof(unsigned int);
     for (parm=0; parm < nentries; parm++) {
       if (bufsize == bufsizes_ao12[parm])
         break;
     }
  }
  
  if (parm >= nentries) {
    printf("GSC16AO_ioctl_set_out_buffer_size: invalid buf size %ld\n", 
            bufsize);
    return(-1);
  }

  status = ioctl(fp, IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_buf_status(int fp, unsigned long *pStatus)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_BUF_STATUS, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_BUF_STATUS) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pStatus)
    *pStatus = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_enable_clk(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_ENABLE_CLK, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_ENABLE_CLK) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_disable_clk(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_DISABLE_CLK, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_DISABLE_CLK) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_calib_status(int fp, unsigned long *pStatus)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_CALIB_STATUS, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_CALIB_STATUS) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pStatus)
    *pStatus = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_data_format(int fp, unsigned long format)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_DATA_FORMAT, &format);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_DATA_FORMAT) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_sampling_mode(int fp, unsigned long mode)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_SAMPLING_MODE, &mode);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_SAMPLING_MODE) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_bursting_status(int fp, unsigned long *pStatus)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_BURSTING_STATUS, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_BURSTING_STATUS) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pStatus)
    *pStatus = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_burst_trigger(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_BURST_TRIGGER, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_BURST_TRIGGER) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_enable_remote_gnd_sense(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_disable_remote_gnd_sense(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_out_clking_mode(int fp, unsigned long mode)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE, &mode);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_clk_source(int fp, unsigned long source)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_CLK_SOURCE, &source);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_CLK_SOURCE) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_clk_status(int fp, unsigned long *pStatus)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_CLK_STATUS, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_CLK_STATUS) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pStatus)
    *pStatus = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_single_output_clk_evt(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVT, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVT) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_select_buf_config(int fp, unsigned long bufcfg)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SELECT_BUF_CONFIG, &bufcfg);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SELECT_BUF_CONFIG) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_load_access_req(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_LOAD_ACCESS_REQ, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_LOAD_ACCESS_REQ) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_cir_buf_status(int fp, unsigned long *pStatus)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_CIR_BUF_STATUS, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_CIR_BUF_STATUS) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pStatus)
    *pStatus = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_clear_buffer(int fp)
{
	int	status;

	status	= ioctl(fp, IOCTL_GSC16AO_CLEAR_BUFFER, NULL);
	if (status < 0) {
		printf("ioctl(IOCTL_GSC16AO_CLEAR_BUFFER) failure, errno = %d\n", errno);
                return (GetLastError());
	}

	return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_get_device_type(int fp, unsigned long *pType)
{
  unsigned long parm = -1;
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_GET_DEVICE_TYPE, &parm);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_GET_DEVICE_TYPE) failure, errno = %d\n", errno);
    return(GetLastError());
  }

  if (pType)
    *pType = parm;
  
  return (GSC16AO_SUCCESS);
}

int GSC16AO_ioctl_set_timeout(int fp, unsigned long timeout)
{
  int status;

  status = ioctl(fp, IOCTL_GSC16AO_SET_TIMEOUT, &timeout);
  if (status < 0) {
    printf("ioctl(IOCTL_GSC16AO_SET_TIMEOUT) failure, errno = %d\n", errno);
    return(GetLastError());
  }
  
  return (GSC16AO_SUCCESS);
}

int GetLastError()
{
  unsigned int ErrorCode;
  unsigned long DeviceError;
  ErrorCode = GSC16AO_ioctl_get_device_error(fd, &DeviceError);

  if ( ErrorCode > 0 ) return DeviceError;
  else return ErrorCode;
}

static void WriteFIFO()
{
  int         iPattType;
  int         iFirstChannel;
  int         iLastChannel;
  int         no_of_loops;
  unsigned long       Loop, i;
  unsigned long       Value;
  unsigned long	    WordsToWrite;
  unsigned long       bGroup;
  unsigned long       bBurst;
  ssize_t	    BytesWritten = 0;
  char        msg[50];
  
  
  do {
    printf ("\nEnter Number of Words to Write ( hex : 1 - 0x9FFF ): ");
    if(scanf ("%lx", &WordsToWrite) == 0) {
      scanf("%s",(char *)&flush);
      fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
      continue;
    }
    
    if ( WordsToWrite <= 0 || WordsToWrite > (WRITE_BUFFER_SIZE-1) )
      printf ("Error : Invalid no. of words to write\n");
    
  }  while ( WordsToWrite > WRITE_BUFFER_SIZE-1
	     || WordsToWrite  <= 0 );

  getchar(); /* flush the new-line character after scanf */

  do {
    printf("\n1 = Fixed Pattern\n");
    printf("2 = Incrementing Pattern\n");
    printf("3 = Inverting Pattern\n");
    printf("4 = Walking Ones\n");
    printf("5 = Walking Zeros\n");
    printf("6 = Specific Pattern\n");
    
    printf("\nEnter Pattern Type : ");
    if(scanf ("%d", &iPattType) == 0) {
      scanf("%s",(char *)&flush);
      fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
      continue;
    }
    
  } while ((iPattType < FIXED_PATTERN) || (iPattType > SPECIFIC_PATTERN));
  
  getchar(); /* flush the new-line character after scanf */

  if ( (iPattType == FIXED_PATTERN) ||  (iPattType == INCREMENTING_PATTERN) ||
       (iPattType == INVERTING_PATTERN) ) {
    
    while(1) {
      printf ("\nEnter Value to Write (in Hex) : ");
      /* Read the value. */
      if(scanf("%lx", &Value) == 0) {
	scanf("%s",(char *)&flush);
	fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
      }
      else
	break;
    }
    
    getchar(); /* flush the new-line character after scanf */
    
    Value = Value & ulDataMask;
    if (iPattType == FIXED_PATTERN) {
      
      for (i=0;i<(int) WordsToWrite;i++)  {
	WriteBuffer[i] = Value;
      }
    }
  }
  else {
    if (iPattType == SPECIFIC_PATTERN) {
      for (i=0;i<(int) WordsToWrite;i++) {
	WriteBuffer[i] = i & ulDataMask;
      }
    }
    else {
      Value = 1;
    }
  }

  sprintf(msg,"\nEnter First Channel ( 0-%d ): ",(hw_nchans-1));
  while(1) {
    printf (msg);
    /* Read the value. */
    if(scanf ("%d", &iFirstChannel) == 0) {
      scanf("%s",(char *)&flush);
      fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
    }
    else
      break;
  }

  getchar(); /* flush the new-line character after scanf */

  sprintf(msg,"\nEnter Last Channel ( 0-%d ): ",(hw_nchans-1));
  while(1) {
    printf (msg);
    /* Read the value. */
    if(scanf ("%d", &iLastChannel) == 0) {
      scanf("%s",(char *)&flush);
      fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
    }
    else
      break;
  }

  getchar(); /* flush the new-line character after scanf */

  bGroup = YorN("\nEnter Group Output (Y/N): ");

  if (bGroup == 0) {
    bBurst = YorN("\nEnter Burst Output (Y/N): ");
  }
  else {
    bBurst = 0;
  }

  for ( i = 0; i < (int) WordsToWrite; i++ ) {
    switch (iPattType) {
    case INCREMENTING_PATTERN:
      WriteBuffer[i] = Value;
      if (Value == ulDataMask) {
	Value = 0;
      }
      else {
	Value++;
      }
      break;

    case INVERTING_PATTERN:
      WriteBuffer[i] = Value;
      Value = (~ Value) & ulDataMask;
      break;
      
    case WALKING_ZEROS:
      WriteBuffer[i] = ~Value & ulDataMask;
      Value = Value << 1;
      if (! (Value & ulDataMask)) {
	Value = 1;
      }
      break;

    case WALKING_ONES:
      WriteBuffer[i] = Value;
      Value = Value << 1;
      if (! (Value & ulDataMask)) {
	Value = 1;
      }
      break;
      
    default:
      break;
    }
  }

#ifdef DEBUG
  printf ("gsc16ao_tst.c : write buffer :: \n");
  
  for ( i = 0; i < WordsToWrite; i++) {
    printf(" %08lx ", WriteBuffer[i]);
  }
  printf("\n");
#endif /* end DEBUG */
  
  while(1) {
    printf ("Enter the number of loops for write (dec) : ");
    /* Read the value. */
    if(scanf ("%d", &no_of_loops) == 0) {
      scanf("%s",(char *)&flush);
      fprintf(stderr,"\n*** Invalid Entry [%s] ***\n",flush);
    }
    else
      break;
  }
  
  getchar(); /* flush the new-line character after scanf */
  
  for ( Loop = 0; Loop < no_of_loops; Loop++) {
    BytesWritten = write(fd, WriteBuffer,  WordsToWrite*4);
    if (BytesWritten < 0) {
      printf("\nWrite failed\n");
      return;
    }
    else {
      if (BytesWritten != (4*WordsToWrite)) {
	printf("\nLoop %ld : Only Wrote %ld Words ( %lX )\n", Loop,
	       (unsigned long)BytesWritten/4, (unsigned long)BytesWritten/4);
      }
      else
	printf ("\nLoop %ld : Wrote %ld words (%lX) successfully\n",
		Loop, (unsigned long)BytesWritten/4, (unsigned long)BytesWritten/4);
    }
    
    if (( WriteNotifyDone == 0 ) && ( WriteNotify == 1 ))
      sleep (1);
    
    WriteNotifyDone = 0;
  }
}

void
signal_initialize()
{
   int flags;

#ifdef DEBUG
   printf ("gsc16ao_tst.c :: <signal_initialize> function entry ...\n");
#endif /* end DEBUG */

   if (fcntl (fd, F_SETOWN, getpid()) != 0)
    {
        printf ("Error : Process ID = %d \n", getpid());
    }

    /* Register signal */

    flags = fcntl (fd, F_GETFL);
    if ( flags == -1)
      {
        printf ("Error : flags\n");
      }
    if ( fcntl(fd, F_SETFL, flags | FASYNC) == -1 )
      {
        printf ("Error : flags .....\n");
       }

  io_act.sa_handler = sigio_handler;
  sigemptyset (&io_act.sa_mask);
  io_act.sa_flags = 0;
  if ( sigaction ( SIGIO, &io_act, NULL ) != 0 )
    {
      printf ("Error : sigaction failed\n");
      return;
    }
}

void
sigio_handler (int sig)
{
  signal_received  = 1;
  printf ("\ngsc16ao_tst.c :: <SIGIO_HANDLER> function entry ...\n");
  return;
}

/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
	fprintf(stderr,"\n*** Invalid Argument [%s] ***\n",arg);
	fprintf(stderr,"Usage: gsc16ao_tst <device_num 0-9>\n");
	exit(1);
}

/*********** Main Menu Commands *****************/
/* Initialization */
#define GSC16AO_CMD_NO_COMMAND		      0
#define GSC16AO_CMD_INIT_BOARD                1
#define GSC16AO_CMD_AUTO_CAL                  2
/* Setup */
#define GSC16AO_CMD_SET_FORMAT                3  /* 2's comp vs binary offset */ 
#define GSC16AO_CMD_SET_SAMPLING_MODE         4  /* continuous vs burst */
#define GSC16AO_CMD_SET_CLOCKING_MODE         5  /* sequential vs simultaneous */
#define GSC16AO_CMD_SET_CLK_SOURCE            6  /* Set clock source and rate for internal */
/* Buffers */
#define GSC16AO_CMD_SELECT_BUF_CONFIG         7  /* open vs closed (cirular) */
#define GSC16AO_CMD_SET_OUT_BUFFER_SIZE       8  /* Set active part of buffer */
#define GSC16AO_CMD_SET_WRITE_MODE            9  /* Set PIO/DMA mode */
#define GSC16AO_CMD_WRITE_BUFFER              10 /* Write data to the buffer */
#define GSC16AO_CMD_CLEAR_BUFFER              11 /* Reset buffer */
/* Clocking */
#define GSC16AO_CMD_SET_CLK_ENABLE            12 /* Enable/disable clocking */
#define GSC16AO_CMD_SINGLE_OUTPUT_CLK_EVT     13 /* Software generated clock event */
#define GSC16AO_CMD_BURST_TRIGGER             14 /* Trigger a burst */
/* Status */
#define GSC16AO_CMD_GET_STATUS                15
#define GSC16AO_CMD_GET_CALIB_STATUS          16
/* Misc */
#define GSC16AO_CMD_READ_REGISTER             17
#define GSC16AO_CMD_WRITE_REGISTER            18
#define GSC16AO_CMD_SET_TIMEOUT               19
#define GSC16AO_CMD_SET_DEBUG_FLAGS           20
#define GSC16AO_CMD_REMOTE_GND_SENSE          21
/* XXX - Interrupt related ioctls do not appear to do anything */
#define GSC16AO_CMD_INT_SOURCE                22
#define GSC16AO_CMD_REG_FOR_INT_NOTIFY        23

#define GSC16AO_CMD_EXIT                      24  /*** MUST BE LAST ENTRY ***/

void
print_main_menu(int fd)
{
	printf("\n");
	printf("     GSC16AO%s Linux Driver Menu\n", boardType == GSC_16AO_12 ? "12" : "2" );
	printf("\n");

	printf(" 0 = No Command               12 = Enable/Disable Clocking\n");
	printf(" 1 = Initialize               13 = Software Clock Event\n");
	printf(" 2 = Auto Calibrate           14 = Burst Trigger\n");
	printf(" 3 = Set Format               15 = Get Device Status\n");
	printf(" 4 = Set Sampling Mode        16 = Get Calibration Status\n");
	printf(" 5 = Set Clocking Mode        17 = Read Register\n");
	printf(" 6 = Set Clock Source & Freq  18 = Write Register\n");
	printf(" 7 = Set Buffer Config        19 = Set Timeout\n");
	printf(" 8 = Set Buffer Size          20 = Set Debug Flags\n");
	printf(" 9 = Set DMA Enable           21 = Enable/Disable Remote GND Sense\n");
	printf("10 = Write Data to Buffer     22 = Set Interrupt Source\n");
	printf("11 = Clear Buffer             23 = Register For Int Notify\n");
	printf("\n");
}

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
  int				    iSelection;
  unsigned int                        uiGeneratorFunc;
  float                      	    Fgen;
  unsigned int			    dwErrorCode;
  unsigned long			    eGSC16AORegister;
  unsigned int			    i;
  unsigned long                       regs;
  unsigned long                       type;
  unsigned long                       ulRegType;
  unsigned long                       ReadType;
  unsigned long                       ulTimeout;
  unsigned long                       ulDeviceError;
  unsigned long                       ulDataFormat;
  unsigned long                       ulRegisterValue;
  unsigned long			      eSequential;
  unsigned long                       eContinuous;
  unsigned long                       eCircular;
  unsigned long                     debugClasses;
  unsigned long                     debugLevel;
  unsigned long                     ulStatus;
  unsigned long                     bEnable;
  char 			*cp;
  
  if(argc == 2) {
    if(argv[1][0] < '0' || argv[1][0] > '9')
      BadArg(argv[1]);
    if(strlen(argv[1]) > 1)
      BadArg(argv[1]);
    devname[12]= argv[1][0];
  } 
  
  fd = GSC16AO_open();
  Get_Board_Info(1);

  signal_initialize();
  
  dwErrorCode = GSC16AO_ioctl_get_device_type(fd, &boardType);
  if (dwErrorCode != GSC16AO_SUCCESS) {
    ErrorMessage("GSC16AO_ioctl_get_device_type: %d", dwErrorCode);
  }

  switch (boardType) {
  case GSC_16AO_16:
  case GSC_16AO_12:
  case GSC_16AO_2:
    break;
    
  default:
    printf("Invalid/unknown board type detected 0x%lx\n",boardType);
    printf("\n");
    GSC16AO_close(fd);
    exit(ERROR_SUCCESS);
  }
  
  bExitIntThread = 0;
  
  print_main_menu(fd);
  
  do {
    fprintf(stderr,"Enter Selection ('h'=display menu, 'q'=quit)-> ");
    
    /* set flag in case we get a timeout or other signal */
    signal_received =0;
    
    fgets((char *)flush, sizeof(flush), stdin);

    if(signal_received ) /* if signal handler entered, redo request */
      continue;

    cp = (char *)flush;
    while((*cp == ' '))cp++; /* skip leading blanks */
    i = strlen(cp) - 1; /* point 2 end of string and remove blanks*/
    while((cp[i] == '\n') || (cp[i] == ' ')) cp[i--] = 0;
    
    if(cp[0] == '\0') /* if no entry....repeat selection */
      continue;
    
    if(!strcmp(cp,"h")) {	/* if help, display table */
      print_main_menu(fd);
      continue;
    }

    if(!strcmp(cp,"q")) {	/* if quit, exit test */
      iSelection = GSC16AO_CMD_EXIT;
    }
    else {
      iSelection = atoi(cp);	/* convert to integer */
      
      if((iSelection < 0) || (iSelection > (GSC16AO_CMD_EXIT-1))) {
	fprintf(stderr,"Invalid Selection [%d]\n",iSelection);
	iSelection = -1; /* invalidate selection */
	continue;
      }
    }

    switch (iSelection)	{
    case GSC16AO_CMD_NO_COMMAND:
      dwErrorCode = GSC16AO_ioctl_no_command(fd);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_NoCommand", dwErrorCode);
      }
      else 
	printf ("\nNo command ... Successful\n");

      break;
      
    case GSC16AO_CMD_AUTO_CAL:
      printf ("\nBoard Autocalibration in Progress ...\n");
      dwErrorCode = GSC16AO_ioctl_auto_cal(fd);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_AutoCalibration", dwErrorCode);
      }
      else
	printf ("\nBoard Autocalibration ... Done\n");
      break;

    case GSC16AO_CMD_INIT_BOARD:
      printf ("\nBoard Initialization in Progress ...\n");
      
      dwErrorCode = GSC16AO_ioctl_init_board(fd);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_Initialize", dwErrorCode);
      }
      else
	printf ("\nBoard Initialization ... Done\n");
      
      break;
		
    case GSC16AO_CMD_SET_CLOCKING_MODE:
      eSequential = GetInt("\n\n"
			   "0 = Channel Sequential\n"
			   "1 = Simultaneous\n\n"
			   "Enter Clocking Mode (0-1): ",
			   SEQUENTIAL,
			   SIMULTANEOUS);
      
      dwErrorCode = GSC16AO_ioctl_select_out_clking_mode(fd, eSequential);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetOutputClkingMode", dwErrorCode);
      }
      else
	printf ("\nClocking mode set to %s\n",
		eSequential == SEQUENTIAL ? "Sequential" : "Simultaneous");
	
      break;

    case GSC16AO_CMD_SET_SAMPLING_MODE:
      eContinuous = GetInt("\n\n"
			   "0 = Continuous Sampling\n"
			   "1 = Triggered Burst Mode\n\n"
			   "Enter Sampling Mode (0-1): ",
			   GSC16AO_CONT_MODE,
			   GSC16AO_BURST_MODE);

      dwErrorCode = GSC16AO_ioctl_select_sampling_mode(fd, eContinuous);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetSampleMode", dwErrorCode);
      }
      else
	printf ("\nSampling mode set to %s\n",
		eContinuous == GSC16AO_CONT_MODE ? "Continuous" : "Triggered Burst");
      break;

    case GSC16AO_CMD_SELECT_BUF_CONFIG:
      eCircular = GetInt("\n\n"
			 "0 = Open buffer\n"
			 "1 = Closed (circular) buffer\n\n"
			 "Enter Buffer config (0-1): ",
			 GSC16AO_OPEN_BUF,
			 1);

      dwErrorCode = GSC16AO_ioctl_select_buf_config(fd, eCircular == GSC16AO_OPEN_BUF ? GSC16AO_OPEN_BUF : GSC16AO_CIRCULAR_BUF);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetBufConfig", dwErrorCode);
      }
      else
	printf ("\nBuffer config is set to %s\n",
		eCircular == GSC16AO_OPEN_BUF ? "Open (draining)" : "Closed (circular)");
      break;
      
    case GSC16AO_CMD_SET_FORMAT:
      ulDataFormat = GetInt("\n\n"
			    "0 = Two's Complement\n"
			    "1 = Offset Binary\n\n"
			    "Enter Data Format (0-1): ",
			    GSC16AO_TWOS_COMP, GSC16AO_OFFSET_BINARY);
      
      dwErrorCode = GSC16AO_ioctl_select_data_format(fd, ulDataFormat);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetDataFormat", dwErrorCode);
      }
      else
	printf ("\nData format set to %s\n",
		ulDataFormat == GSC16AO_TWOS_COMP ? "Two's complement" : "Offset/binary");
      break;

    case GSC16AO_CMD_CLEAR_BUFFER:
      printf ("\nClearing Board Output Buffer in Progress ...\n");
      dwErrorCode = GSC16AO_ioctl_clear_buffer(fd);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_ClearOutputBuffer", dwErrorCode);
      }
      else
	printf ("\nClearing Board Output Buffer ... Done\n");

      break;

    case GSC16AO_CMD_SET_CLK_SOURCE:
      uiGeneratorFunc = GetInt("\n\n"
			       "0 = Set Internal Clock\n"
			       "1 = Set External Clock\n"
			       "Enter Clock Function (0-1): ",
			       INTERNAL, EXTERNAL);

      switch (uiGeneratorFunc) {
      case EXTERNAL:
	dwErrorCode = GSC16AO_ioctl_select_clk_source(fd,EXTERNAL);
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_SetClockSource", dwErrorCode);
	}
	else
	  printf ("\nExternal clock source selected\n");
	break;

      case INTERNAL:
	Fgen = GetFloat("Enter Internal Clock Frequency (0.45777-400.000 kHz): ",
			(float) MIN_FGEN, (float) MAX_FGEN);

	dwErrorCode = GSC16AO_ioctl_program_rate_gen(fd,Fgen);
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_SetClockRate", dwErrorCode);
	}
	else
	  printf ("\nClock frequency set to %5.2f kHz\n", Fgen);

	dwErrorCode = GSC16AO_ioctl_select_clk_source(fd,INTERNAL);
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_SetClockSource", dwErrorCode);
	}
	else
	  printf ("\nInternal clock generator selected\n");

	break;
      } /* switch clk source */
      break;

    case GSC16AO_CMD_READ_REGISTER:
      ulRegType = GetInt("\n\n"
			 "0 = GSC16AO Register\n"
			 "1 = PCI Configuration Register\n"
			 "2 = Local (PLX) Register\n\n"
			 "Enter Register Type to Read (0-2): ",
			 GSC16AO_GSC_REGISTER, GSC16AO_PLX_REGISTER);

      switch (ulRegType) {
      default:
      case GSC16AO_GSC_REGISTER:
	ReadType = GetInt("\n\n"
			  "0 = Read individual GSC Registers\n"
			  "1 = Read all GSC Registers\n\n"
			  "Enter Display Type (0-1): ",
			  0, 1);

	if ( ReadType == 0 ) {
	  eGSC16AORegister  = eGetGSC16AORegister();
	  dwErrorCode = GSC16AO_ioctl_read_gsc_register(fd,
							eGSC16AORegister,
							&ulRegisterValue);

	  if (dwErrorCode != GSC16AO_SUCCESS) {
	    ErrorMessage("GSC16AOReadRegister",dwErrorCode);
	  }
	  else {
	    printf("\n");
	    printf("%s = %08lx",
		   pszRegisterNames[eGSC16AORegister],
		   ulRegisterValue);
	  }
	}
	else if ( ReadType == 1 ) {
	  for (eGSC16AORegister=GSC16AO_GSC_BCR;
	       eGSC16AORegister <= GSC16AO_GSC_ACLK;
	       eGSC16AORegister++) {
			      
	    if (pszRegisterNames[eGSC16AORegister] == NULL)
	      continue;
			      
	    dwErrorCode = GSC16AO_ioctl_read_gsc_register(fd,
							  eGSC16AORegister,
							  &ulRegisterValue);
	    if (dwErrorCode != GSC16AO_SUCCESS) {
	      ErrorMessage("GSC16AOReadRegister", dwErrorCode);
	    }
	    else {
	      printf("\n%s = %08lx",
		     pszRegisterNames[eGSC16AORegister],
		     ulRegisterValue);
	    }
	  }
	}
	break;

      case GSC16AO_PCI_REGISTER:
	eGSC16AORegister  = eGetGSC16AOConfigRegister();
	dwErrorCode = GSC16AO_ioctl_read_pci_register(fd,
						      eGSC16AORegister,
						      &ulRegisterValue);

	if (dwErrorCode == GSC16AO_SUCCESS) {
	  printf("\n%s = %08lx",
		 pszPCIConfigRegisterNames[eGSC16AORegister],
		 ulRegisterValue);
	}
	else {
	  ErrorMessage("GSC16AOReadPCIConfigRegisters", dwErrorCode);
	}
	break;

      case GSC16AO_PLX_REGISTER:
	eGSC16AORegister  = eGetGSC16AOLocalRegister(&regs, &type);

	dwErrorCode = GSC16AO_ioctl_read_plx_register(fd,
						      eGSC16AORegister,
						      &ulRegisterValue);
			
	if (dwErrorCode == GSC16AO_SUCCESS) {
	  printf("\n%s = %08lx\n",
		 LocalRegs[type][regs].pszRegisterName,
		 ulRegisterValue);
	}
	
	else {
	  ErrorMessage("GSC16AOReadLocalConfigRegisters", dwErrorCode);
	}
	break;
      }
      printf("\n");
      break;

    case GSC16AO_CMD_WRITE_REGISTER:
      ulRegType = GetInt("\n\n"
			 "0 = GSC16AO Register\n"
			 "1 = PCI Configuration Register\n"
			 "2 = Local (PLX) Register\n\n"
			 "Enter Register Type to Write (0-2): ",
			 GSC16AO_GSC_REGISTER, GSC16AO_PLX_REGISTER);

      switch (ulRegType) {
      default:
      case GSC16AO_GSC_REGISTER:
	eGSC16AORegister  = eGetGSC16AORegister();
	ulRegisterValue = ulGetRegisterValue();
	dwErrorCode = GSC16AO_ioctl_write_gsc_register(fd, eGSC16AORegister, ulRegisterValue);
	if (dwErrorCode != GSC16AO_SUCCESS){
	  ErrorMessage("GSC16AOWriteRegister", dwErrorCode);
	}
	else
	    printf("\n%08lx --> %s\n",
		   ulRegisterValue,
		   pszRegisterNames[eGSC16AORegister]);
	break;
		      
      case GSC16AO_PCI_REGISTER:
	eGSC16AORegister  = eGetGSC16AOConfigRegister();
	ulRegisterValue = ulGetRegisterValue();
	dwErrorCode = GSC16AO_ioctl_write_pci_register(fd, eGSC16AORegister, ulRegisterValue);
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AOWritePCIConfigRegister", dwErrorCode);
	}
	else
	  printf("\n%08lx --> %s\n",
		 ulRegisterValue,
		 pszPCIConfigRegisterNames[eGSC16AORegister]);
	  
	break;

      case GSC16AO_PLX_REGISTER:
	eGSC16AORegister  = eGetGSC16AOLocalRegister(&regs, &type);
	ulRegisterValue = ulGetRegisterValue();
	dwErrorCode = GSC16AO_ioctl_write_plx_register(fd,eGSC16AORegister, ulRegisterValue);
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_WriteLocalConfigRegister", dwErrorCode);
	}
	else
	  printf("\n%08lx --> %s\n",
		 ulRegisterValue,
		 LocalRegs[type][regs].pszRegisterName);
	break;
      }
      break;

    case GSC16AO_CMD_SET_TIMEOUT:
      ulTimeout = GetLong("\nEnter Timeout (sec) (0 = NO Timeout): ");
      dwErrorCode = GSC16AO_ioctl_set_timeout(fd,(unsigned long)ulTimeout);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetTimeout", dwErrorCode);
      }
      else
	printf("\nTimeout set to %ld secs\n", ulTimeout);
      
      break;

    case GSC16AO_CMD_GET_STATUS:
      dwErrorCode = GSC16AO_ioctl_get_device_error(fd, &ulDeviceError);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetDeviceError", dwErrorCode);
      }
      else {
	printf("\nDevice Error = %s\n", pszDeviceError[ulDeviceError-GSC16AO_SUCCESS]);
      }

      dwErrorCode = GSC16AO_ioctl_get_buf_status(fd, &ulStatus);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetBufStatus", dwErrorCode);
      }
      else
	printf("Buffer status = 0x%lx\n", ulStatus);

      dwErrorCode = GSC16AO_ioctl_get_bursting_status(fd, &ulStatus);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetBurstingStatus", dwErrorCode);
      }
      else
	printf("Bursting status = 0x%lx\n", ulStatus);

      dwErrorCode = GSC16AO_ioctl_get_clk_status(fd, &ulStatus);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetClkStatus", dwErrorCode);
      }
      else
	printf("Clock status = 0x%lx\n", ulStatus);

      dwErrorCode = GSC16AO_ioctl_get_cir_buf_status(fd, &ulStatus);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetCirBufStatus", dwErrorCode);
      }
      else
	printf("Circular buffer status = 0x%lx\n", ulStatus);

      break;

    case GSC16AO_CMD_GET_CALIB_STATUS:
      dwErrorCode = GSC16AO_ioctl_get_calib_status(fd, &ulStatus);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_GetDeviceError", dwErrorCode);
      }
      else
	printf("\nCalibration status = 0x%lx\n", ulStatus);

      break;

    case GSC16AO_CMD_WRITE_BUFFER:
      WriteFIFO();
      break;

    case GSC16AO_CMD_SET_WRITE_MODE:
      bEnable = YorN("\nEnable DMA (Y/N)?: ");
      dwErrorCode = GSC16AO_ioctl_set_write_mode(fd, bEnable ? GSC16AO_DMA_MODE : GSC16AO_SCAN_MODE);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetWriteMode", dwErrorCode);
      }
      else
	printf("DMA transfers are %s\n", bEnable ? "enabled" : "disabled");
	
      break;

    case GSC16AO_CMD_SET_OUT_BUFFER_SIZE:
      if(board_info.board_type == GSC_16AO_16) {
          i = GetInt("\n\n"
    		 "0 = 8 samples    8  = 2048 samples\n"
    		 "1 = 16 samples   9  = 4096 samples\n"
    		 "2 = 32 samples   10 = 8192 samples\n"
    		 "3 = 64 samples   11 = 16K samples\n"
    		 "4 = 128 samples  12 = 32k samples\n"
    		 "5 = 256 samples  13 = 64k samples\n"
    		 "6 = 512 samples  14 = 128k samples\n"
    		 "7 = 1024 samples 15 = 256k samples\n\n"
    		 "Choose buffer size (0-15): ",
    		 0, 15);
    
          dwErrorCode = GSC16AO_ioctl_set_out_buffer_size(fd, bufsizes_ao16[i]);
          if (dwErrorCode != GSC16AO_SUCCESS) {
    	     ErrorMessage("GSC16AO_SetOutBufferSize", dwErrorCode);
          }
          else {
    	     printf("\nActive buffer size set to %d samples\n", 
						bufsizes_ao16[i]);
          }
      } else {
          i = GetInt("\n\n"
    		 "0 = 4 samples    8  = 1024 samples\n"
    		 "1 = 8 samples    9  = 2048 samples\n"
    		 "2 = 16 samples   10 = 4096 samples\n"
    		 "3 = 32 samples   11 = 8192 samples\n"
    		 "4 = 64 samples   12 = 16k samples\n"
    		 "5 = 128 samples  13 = 32k samples\n"
    		 "6 = 256 samples  14 = 64k samples\n"
    		 "7 = 512 samples  15 = 128k samples\n\n"
    		 "Choose buffer size (0-15): ",
    		 0, 15);
    
          dwErrorCode = GSC16AO_ioctl_set_out_buffer_size(fd, bufsizes_ao12[i]);
          if (dwErrorCode != GSC16AO_SUCCESS) {
    	     ErrorMessage("GSC16AO_SetOutBufferSize", dwErrorCode);
          }
          else {
    	     printf("\nActive buffer size set to %d samples\n", 
							bufsizes_ao12[i]);
          }
      }
      
      break;
	       
    case GSC16AO_CMD_SET_CLK_ENABLE:
      bEnable = YorN("\nEnable Clocking (Y/N)?: ");
      if (bEnable)
	dwErrorCode = GSC16AO_ioctl_enable_clk(fd);
      else
	dwErrorCode = GSC16AO_ioctl_disable_clk(fd);
      
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetClkEnable", dwErrorCode);
      }
      else
	printf("Clocking is %s\n", bEnable ? "enabled" : "disabled");

      break;
      
    case GSC16AO_CMD_SINGLE_OUTPUT_CLK_EVT:
      bEnable = YorN("\nGenerate software clock pulse (Y/N)?: ");
      if (bEnable) {
	dwErrorCode = GSC16AO_ioctl_single_output_clk_evt(fd);
      
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_OutptClkEvent", dwErrorCode);
	}
	else
	  printf("\nSoftware clock generated\n");
      }
      break;
	       
    case GSC16AO_CMD_BURST_TRIGGER:
      bEnable = YorN("\nGenerate burst trigger (Y/N)?: ");
      if (bEnable) {
	dwErrorCode = GSC16AO_ioctl_burst_trigger(fd);
      
	if (dwErrorCode != GSC16AO_SUCCESS) {
	  ErrorMessage("GSC16AO_BurstTrigger", dwErrorCode);
	}
	else
	  printf("\nBurst triggered\n");
      }
      break;
	       
    case GSC16AO_CMD_SET_DEBUG_FLAGS:

      debugLevel = GetInt("\n\n"
			  "0 = Critical Errors Only\n"
			  "1 = Errors\n"
			  "2 = Warnings\n"
			  "3 = Notices\n"
			  "4 = Info\n"
			  "5 = Trace Level 1\n"
			  "6 = Trace Level 2\n"
			  "7 = Trace Level 3\n"
			  "8 = Trace Level 4\n"
			  "9 = Trace Function Entry/Exit\n\n"
			  "Enter debug level (0-9): ",
			  0, 9);
	
      debugClasses = GetHexLong("\n\n"
			  "0x01 = Driver Initialization\n"
			  "0x02 = Open/Close\n"
			  "0x04 = Read/Write\n"
			  "0x08 = Ioctl\n"
			  "0x10 = Intr Handler\n"
			  "0x20 = Mmap\n"
			  "0x40 = GSC Reg Access\n"
			  "0x80 = Misc\n\n"
			  "Enter debug class enable mask (0-0xff): ");

      dwErrorCode = GSC16AO_ioctl_set_debug_flags(fd, debugClasses, debugLevel);
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_SetDebugFlags", dwErrorCode);
      }
      else
	printf("\nNew driver debug level %ld, classes 0x%02lx\n", debugLevel, debugClasses);

      break;
	       
    case GSC16AO_CMD_REMOTE_GND_SENSE:
      bEnable = YorN("\nEnable Remote Ground Sense (Y/N)?: ");
      if (bEnable)
	dwErrorCode = GSC16AO_ioctl_enable_remote_gnd_sense(fd);
      else
	dwErrorCode = GSC16AO_ioctl_disable_remote_gnd_sense(fd);
      
      if (dwErrorCode != GSC16AO_SUCCESS) {
	ErrorMessage("GSC16AO_RemoteGndSense", dwErrorCode);
      }
      else
	printf("\nRemote ground sense is %s\n", bEnable ? "enabled" : "disabled");

      break;
      
    case GSC16AO_CMD_INT_SOURCE:
    case GSC16AO_CMD_REG_FOR_INT_NOTIFY:
      printf("\nNot Implemented\n");
      break;
      
    case GSC16AO_CMD_EXIT:
      break;

    default:
      printf("Invalid Selection [%d]\n",iSelection);
      break;
    }
    
    printf("\n");
  } while (iSelection != GSC16AO_CMD_EXIT) ;

  bExitIntThread = 1;
  
  /*
   * All done with this; get rid of it
   */
  GSC16AO_close(fd);
  exit(ERROR_SUCCESS);
  
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

