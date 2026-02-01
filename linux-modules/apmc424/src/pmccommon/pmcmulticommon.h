
/*
{+D}
    SYSTEM:	    Pentium

    FILENAME:	    PmcMultiCommon.h

    MODULE NAME:    Functions common to the Pentium example software.

    VERSION:	    C

    CREATION DATE:  06/29/06

    DESIGNED BY:    FJM

    CODED BY:	    FJM

    ABSTRACT:       This file contains the definitions, structures
                    and prototypes for PMC Multi Common.

    CALLING
	SEQUENCE:

    MODULE TYPE:

    I/O RESOURCES:

    SYSTEM
	RESOURCES:

    MODULES
	CALLED:

    REVISIONS:

  DATE	  BY	    PURPOSE
-------  ----	------------------------------------------------
01/11/07 FJM   Add support x86_64
04/01/09 FJM   Add blocking_start_convert for ain boards

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

	This file contains the definitions, structures and prototypes for PMC Multi Common.
*/

#define VENDOR_ID (word)0x16D5		/* Acromag's vendor ID for all PCI bus products */
#define MAX_PMCS 4			/* maximum number of PMC boards */


#ifndef BUILDING_FOR_KERNEL
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* Required for FC4 */
#include <stdlib.h>     /* malloc */
#include <string.h>     /* memset */
#endif /* BUILDING_FOR_KERNEL */



typedef unsigned short UWORD;
typedef unsigned char BYTE;
typedef int BOOL;
typedef uint32_t ULONG;
typedef unsigned char byte;		/* custom made byte data type */
typedef unsigned short word;		/* custom made word data type */
typedef short WORD;



typedef int PSTATUS;			/* custom made PSTATUS data type, used as a
                                           return value from the PmcCommon functions. */
#define TRUE	1			/* Boolean value for true */
#define FALSE	0			/* Boolean value for false */



#define APMC_INT_RELEASE	0x8000	/* ORed with interrupt register to release a interrupt */
#define APMC_INT_ENABLE		0x0001	/* interrupt enable */
#define APMC_INT_PENDING	0x0002	/* interrupt pending bit */



/*
	PSTATUS return values
	Errors will have most significant bit set and are preceded with an E_.
	Success values will be succeeded with an S_.
*/

#define ERROR			0x8000	/* general */
#define E_OUT_OF_MEMORY 	0x8001	/* Out of memory status value */
#define E_OUT_OF_PMCS   	0x8002	/* All Pmc spots have been taken */
#define E_INVALID_HANDLE	0x8003	/* no Pmc exists for this handle */
#define E_NOT_INITIALIZED	0x8006	/* Pmc not initialized */
#define E_NOT_IMPLEMENTED       0x8007	/* Function is not implemented */
#define E_NO_INTERRUPTS 	0x8008	/* unable to handle interrupts */
#define S_OK                    0x0000	/* Everything worked successfully */


/*
	Pmc data structure
*/

typedef struct
{
	int nHandle;			/* handle from addpmc() */
	int nPmcDeviceHandle;		/* handle from kernel open() */
	long lBaseAddress;		/* pointer to base address of Pmc board */
	int nInteruptID;		/* ID of interrupt handler */
	int nIntLevel;			/* Interrupt level of Pmc */
	char devname[64];		/* device name */
	BOOL bInitialized;		/* intialized flag */
	BOOL bIntEnabled;		/* interrupts enabled flag */
}PMCDATA_STRUCT;

typedef struct
{
	word InterruptRegister;		/* Interrupt Pending/control Register */
}PMC_BOARD_MEMORY_MAP;

/*
	Function Prototypes
*/

PSTATUS GetPmcAddress(int nHandle, long* pAddress);
PSTATUS SetPmcAddress(int nHandle, long lAddress);
PSTATUS EnableInterrupts(int nHandle);
PSTATUS DisableInterrupts(int nHandle);
PSTATUS InitPmcLib(void);
PSTATUS PmcOpen(int nDevInstance, int* pHandle, char* devname);
PSTATUS PmcClose(int nHandle);
PSTATUS PmcInitialize(int nHandle);

/*  Functions used by above functions */
void AddPmc(PMCDATA_STRUCT* pPmc);
void DeletePmc(int nHandle);
PMCDATA_STRUCT* GetPmc(int nHandle);
byte input_byte(int nHandle, byte*);		/* function to read an input byte */
word input_word(int nHandle, word*);		/* function to read an input word */
void output_byte(int nHandle, byte*, byte);	/* function to output a byte */
void output_word(int nHandle, word*, word);	/* function to output a word */
long input_long(int nHandle, long*);		/* function to read an input long */
void output_long(int nHandle, long*, long);	/* function to output a long */
void blocking_start_convert(int nHandle, word *p, word v);


