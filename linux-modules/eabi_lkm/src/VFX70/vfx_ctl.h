

/*
{+D}
    SYSTEM:             Software for pmcvfx Digital I/O Boards

    FILENAME:           pmcvfx.h

    MODULE NAME:        pmcvfx.h

    VERSION:            A

    CREATION DATE:      02/05/09

    CODED BY:           FM

    ABSTRACT:           This "inlcude" file contains all the major data
                        structures and memory map used in the software

    CALLING SEQUENCE:

    MODULE TYPE:        include file

    I/O RESOURCES:

    SYSTEM RESOURCES:

    MODULES CALLED:

    REVISIONS:

  DATE       BY     PURPOSE
----------  ----  -------------------------------------------------------------
29/03/2013   YG   Definition de noms symboliques pour les cmd ioctl Acromag
 2/04/2013   YG   Adaptation prototype de DirectConfig() au code remis en forme
 4/04/2013   YG   Suppression des prototypes inutilises dans le driver et
                  changement du nom du fichier.
                  Inclusion d'un certain nombre d'elements extraits de
                  pmcvxmulticommon.h.



{-D}
*/


#ifndef VFX_CTL_H_
#define VFX_CTL_H_


/*
    DEFINITIONS:
*/


#define MAX_PMCS 4			/* maximum number of PMC boards */

#define VENDOR_ID (word)0x16D5	/* Acromag's vendor ID for all PMC bus products */

#define PMC_VX_BAR0_MAP_SIZE		4096		/* size of BAR0 */
#define PMC_VX_BAR1_MAP_SIZE		0x400000	/* size of BAR1 */
#define PMC_VX_BAR2_MAP_SIZE		0x400000	/* size of BAR2 */



typedef int BOOL;
typedef unsigned char byte;     /* custom made byte data type */
typedef unsigned short word;    /* custom made word data type */
typedef int PSTATUS;            /* return value from the Pmcvxmulticommon functions. */

#define TRUE	1			/* Boolean value for true */
#define FALSE	0			/* Boolean value for false */


/* //////////////////////////////////////////////////////////////////////// */
/* Select CPU type that corresponds to your hardware.                       */
/* Default is A32 - Commented out '#define BUILDING_FOR_A64 0' for A32 CPU. */
/* Uncomment '#define BUILDING_FOR_A64 0 ' to build for A64 CPU.            */
/* //////////////////////////////////////////////////////////////////////// */
/*#define BUILDING_FOR_A64 0*/



/* //////////////////////////////////////////////////////////////////////// */
/* Select board, array type, and device name by selecting the defines below */
/* that correspond to your hardware.      Default is PmcVFX70               */
/* Any change here requires the complete rebuild of the demo application as */
/* well as the kernel code.                                                 */
/* //////////////////////////////////////////////////////////////////////// */


#define PMCVFXBOARD (word)0x5605					/* PmcVFX70 device ID */

#ifndef DEVICE_NAME
#define DEVICE_NAME	"apmcvfx70_"					/* the name of the device */
#endif


#define PARTITION_0_START_SECTOR	0			/* partition 0 holds Xilinx bit data */
#define PARTITION_0_LAST_SECTOR		127
#define PARTITION_0_START_ADDR		0x00000000	/* start address of FLASH partition 0*/

#define PARTITION_1_START_SECTOR	128			/* partition 1 holds object code for the PPC */
#define PARTITION_1_LAST_SECTOR		255
#define PARTITION_1_START_ADDR		0x01000000	/* start address of FLASH partition 1 */



/* The user may redefine the size of this DMA buffer */
/* Both devXXXX kernel code and the application code must be recompiled */
#define TOBUFFER_SIZE 2048			/* DMA buffer */

#define MAX_MEMORY_PAGES  ( TOBUFFER_SIZE >> 12 ) + 2



typedef enum
{
	Success = 0,
	ParameterOutOfRange = 1,	/* Parameter in error */
	InvalidPointer = 2,			/* Flag NULL pointers */
	DataDirection = 3,			/* data direction error */
	TimeOut = 4					/* time-out error */
} PMCSTAT;

/*
    Parameter mask bit positions for AMX-DXX and AMX-EDK
*/

#define SEL_MODEL      1        /* selelct model */
#define INT_STATUS     2        /* interrupt status registers */
#define INT_ENAB       4        /* interrupt enable registers */
#define INT_POLARITY   8        /* interrupt polarity registers */
#define INT_TYPE     0x10       /* interrupt type registers */
#define DIG_DIR      0x20       /* direction */
#define RESET        0x40       /* reset user FPGA */
#define DIFF_DIR     0x80       /* direction */
#define RESET_LX30   0x100      /* reset bus FPGA */


/*
    Parameter mask bit positions for AXM-A30
*/

#define ADC_D_CONV			0	/* disable conversions */
#define ADC_E_CONV			1	/* enable conversions */

#define ADC_SB_SELECT		0	/* straight binary */
#define ADC_TC_SELECT		1	/* two's compliment */

#define ADC_DC_DISABLE		0	/* disable duty cycle stabilizer */
#define ADC_DC_ENABLE		1	/* enable duty cycle stabilizer */

#define ADC_FIFO_CLEAR		1	/* clear ADC FIFO */
#define ADC_FIFO_FULL		0x8000	/* ADC FIFO full */
#define ADC_FIFO_NOT_EMPTY	0x2000	/* ADC FIFO not empty */
#define ADC_FIFO_INT_PEND0	0x100	/* ADC FIFO interrupt pending */
#define ADC_FIFO_INT_PEND1	0x200	/* ADC FIFO interrupt pending */

#define ADC_INT_DIS			0	/* disable interrupt */
#define ADC_INT_EN			1	/* interrupt */

#define ADC_STOP_AT_THRESHOLD	1	/* disable after threshold samples */

#define ADC_DISABLE		0		/* disable */

#define ADC_BUF_SIZE 8192


/*
    Defined below is the memory map template for the pmcvfx Board.
    This structure provides access to the various registers on the board.
*/

struct mapvfx                  /* Memory map of the I/O board */
{
    /* Flash Configuration Control */
    word ConfigStatusRegister;
    word unusedw1;				/* unused */
    word ConfigControlRegister;
    word unusedw2;				/* unused */
    word ConfigDataRegister;
    word unusedw3;				/* unused */
    word FlashStatus1Register;
    word unusedw4;				/* unused */
    word FlashStatus2Register;
    word unusedw5;				/* unused */
    word FlashRead;
    word unusedw6;				/* unused */
    word FlashReset;
    word unusedw7;				/* unused */
    word FlashStartWrite;
    word unusedw8;				/* unused */
    word FlashEraseSector;
    word unusedw9;				/* unused */
    word FlashEraseChip;
    word unusedw10;				/* unused */
    word FlashDataRegister;
    word unusedw11;				/* unused */
    word FlashAddress7_0;
    word unusedw12;				/* unused */
    word FlashAddress15_8;
    word unusedw13;				/* unused */
    word FlashAddress23_16;
    word unusedw14;				/* unused */
    word FlashAddress31_24;
    word unusedw15;				/* unused */
	uint32_t SysMonStatusControl;
	uint32_t SysMonAddrReg;
    word unusedw16[0x3FDE];		/* unused */

    /* PMC Base Section */
    word sts_reg;				/* Status/Clear Register */
    word ResetRegister;			/* Reset Register */
    /* AMX-DXX and AMX-EDK Section*/
    uint32_t Diff_io_reg;			/* Differential input/output register */
    uint32_t Diff_dir_reg;		/* Differential direction Register */
    uint32_t Dig_io_reg;			/* 15-0 Digital input/output register */
    uint32_t Dig_dir_reg;			/* 15-0 Digital direction Register */
    uint32_t en_reg;				/* Interrupt Enable Register */
    uint32_t type_reg;			/* Interrupt Type Select Register */
    uint32_t pol_reg;				/* Interrupt Input Polarity Register */
    uint32_t rsvd[3];				/* reserved space */
    /* PMC Base Section */
    uint32_t RearRead;			/* Rear connector read register */
    uint32_t RearWrite;			/* Rear connector write register */
    uint32_t DMA_ctrl;			/* DMA control register */
    uint32_t FPGA_SRAM_31_0;		/* FPGA Port SRAM Register Data Lines 31 to 0 */
    uint32_t FPGA_SRAM_63_32;		/* FPGA Port SRAM Register Data Lines 63 to 32 */
    uint32_t FPGA_SRAM_Control;	/* SRAM control register */
    uint32_t FPGA_SRAM_Address;	/* SRAM address register */
    uint32_t DMA_Threshold0;		/* threshold register */
    uint32_t DMA_Threshold1;		/* threshold register */
    uint32_t Address_Reset0;		/* address reset register */
    uint32_t Address_Reset1;		/* address reset register */
    uint32_t rsvd1;
    uint32_t PPCMEM_Control;		/* PPCMEM control register */
    uint32_t PPCMEM_Address;		/* PPCMEM address register */
    uint32_t PPCMEM_Read;			/* PPCMEM read registers */
    uint32_t PPCMEM_Write;		/* PPCMEM write registers */
    uint32_t PPCMEM_Mask;			/* PPCMEM mask register */
    uint32_t rsvd2[6];
    uint32_t SysMonStatusControl2;
    uint32_t SysMonAddrReg2;
    byte rsvd3[0x70];			/* reserved space */

	/* AXM-A30 Section */
	word AXM_Control;			/* AXM-A30 Control Register */
	word AXM_GPIO;				/* General purpose I/O bit */
	word ADC_ControlStatus[2];	/* ADC Control Status Registers */
	word ADC_ConversionThreshold[2];	/* ADC Conversion Threshold Registers */
	word ADC_FIFORead[2];		/* ADC Read Register */
    word Clock_StatusControl;	/* Clock Conditioner Control/Status Register */
	word AXM_unused1;
    uint32_t Clock_Conditioner[8];/* Clock Conditioner Registers */
	word ADC_DP_SRAM;			/* ADC DP SRAM overide */
	word AXM_unused2;
	byte unusedb1[0xF7EC8];		/* end of boundry */
    byte LocalSRAM[0xFFFFF];	/* 1Meg SRAM */
};

/*
    Defined below is the structure which is used to hold the
    board's status information.
*/

struct sblkvfx
{
    uint32_t Diff_direction;		/* direction register */
    uint32_t Dig_direction;		/* direction register */
    uint32_t enable;		        /* interrupt enable register */
    uint32_t type;				/* interrupt type select register */
    uint32_t int_status;			/* interrupt status register */
    uint32_t mez_status;			/* mezzaine status register */
    uint32_t polarity;			/* interrupt input polarity register */
    uint32_t PciBusFPGAAdrData[10];	/* PCI bus FPGA address & data order:0,1,2,20 thru 26 */
    uint32_t ReprogFPGAAdrData[10];	/* Reprogrammable FPGA address & data order:0,1,2,20 thru 26 */
};


/*
    Defined below is the structure which is used to hold the
    board's configuration information.
*/

struct cblkvfx
{
    struct sblkvfx *sblk_ptr;   /* pointer to status block structure */
    struct mapvfx *brd_ptr;     /* base address of the I/O board */
	char *dpr_ptr;				/* DPR base address */
    int nHandle;                /* handle to an open carrier board */
    BOOL bPmc;                  /* flag indicating board is open */
    BOOL bInitialized;          /* flag indicating ready to talk to board */	
    BOOL bIntAttached;          /* flag indicating ISR attached */	
    word param;                 /* parameter mask for configuring board */
    uint32_t Diff_direction;		/* direction register */
    uint32_t Dig_direction;		/* direction register */
    uint32_t enable;				/* interrupt enable registers */
    uint32_t type;				/* interrupt type select registers */
    uint32_t int_status;			/* interrupt status registers */
    uint32_t polarity;			/* interrupt input polarity registers */

	/* AXM-A30 */
    uint32_t ClockConditioner[8];	/* Clock Conditioner values */
    word convert_mask;          /* mask of channels to convert */
	word gpio;                  /* general purpose I/O bit control */
	word dp_SRAM_Overide;		/* DP-SRAM overide */
	word clk_source;            /* ADC clock source */
	word interrupt_enable[2];   /* interrupt */
    word data_format[2];        /* SB or BTC */
    word conversion_threshold[2];/* threshold value */ 
    word fifo_clear[2];         /* FIFO_CLEAR */
    word stabilizer[2];         /* stabilizer */	
    word stop_at_threshold[2];  /* stop at threshold value  */	
    word *cor_buf[2];           /* corrected buffer start */
    word *raw_buf[2];           /* raw buffer start */
    word minus_cal_buf[2];      /* - calibration count */
    word plus_cal_buf[2];       /* + calibration count */
};



/*
    Defined below is the Interrupt Handler Data Structure.
    The interrupt handler is provided with a pointer that points
    to this structure.  From this the handler has a link back to
    its related process and common data area.  The usage of the members
    is not absolute.  Their typical uses are described below.
*/
struct handler_data
    {
    int h_pid;          /* Handler related process i.d. number.
                           Needed to know if handler is going to wake up,
                           put to sleep, or suspend a process.*/
    char *hd_ptr;       /* Handler related data pointer.
                           Needed to know if handler is going to access
                           a process' data area. */
    };

    
/*
   Structure used by VFX_LECVERSION ioctl function
*/
struct vfxver {
    char  nom[101];       /* Nom du pilote */
    char  version[101];   /* Version du pilote */
    char  date[101];      /* Date du pilote */

    int vfxver;           /* Numero de version du pilote */
    int vfxrev;           /* Numero de revision du pilote */
};





/* DMA Attributes */

#define LOCAL_TO_PCI				(0xF << 4)		/* DMA direction */
#define PCI_TO_LOCAL				(0xE << 4)
#define HighPriority				(1 << 1)		/* DMA channel priority */
#define XFR64					(1 << 2)		/* Transfer size bits */
#define ScatterGather				(1 << 3)		/* DMA channel priority */

/* Scatter/gather */

#define DMA_CHANNEL_DESCRIPTOR_END	(1 << 0)	/* last descriptor mask */


/* DMA register bits */

#define BoardInterruptPending		(1 << 0)
#define DMACh0InterruptPending		(1 << 1)
#define DMACh1InterruptPending		(1 << 2)
#define FGPAInterruptPending		(1 << 3)

#define DMACh0InterruptEnable		(1 << 0)
#define DMACh1InterruptEnable		(1 << 1)
#define FPGAInterruptEnable			(1 << 2)
#define BoardInterruptEnable		(1 << 31)

#define DMACh0TransferComplete		(1 << 0)
#define DMACh1TransferComplete		(1 << 1)
#define DMACh0InterruptAbort		(1 << 8)
#define DMACh1InterruptAbort		(1 << 9)
#define DMACh0StateEncoding			(7 << 16)
#define DMACh1StateEncoding			(7 << 20

#define DMACmdPriorityHigh			(1 << 1)
#define DMATransferSize64			(1 << 2)
#define DMAScatterGatherEnabled		(1 << 3)
#define DMAMemoryWriteBurst			(0xF << 4)




/*
    declare interrupt handlers
*/

int isr_pmcvfx(int);	/*  Interrupt Handler for Pmcvfx */
                        /* Return 1 if interrupt can't be handled, */
                        /* otherwise 0.                             */

/* Commandes ioctl Acromag */

#define VFX_GET_PCIBAR2    3
#define VFX_FLUSH_CACHE    4
#define VFX_GET_PCIBAR0    5
#define VFX_GET_IRQ        6
#define VFX_GET_PCIBAR1    7
#define VFX_DMA_V2P        8
#define VFX_FREE_DMA_BUF   9
#define VFX_LECVERSION    10



#endif    /* VFX_CTL_H_ */



