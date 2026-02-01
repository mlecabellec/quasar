// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_common.h $
// $Rev: 54344 $
// $Date: 2024-04-29 08:38:44 -0500 (Mon, 29 Apr 2024) $

// OS & Device Independent: Device Driver: header file

#ifndef	__GSC_COMMON_H__
#define	__GSC_COMMON_H__

#include "os_common.h"



// macros *********************************************************************

//	Encoding and decoding of registers.
//	r = register value to decode
//	v = field value to encode
//	b = beginning bit (most significant bit position, 31 is the upper most bit).
//	e = ending bit (least significant bit position, 0 is the lower most bit).
#define	GSC_FIELD_DECODE(r,b,e)		(((r)>>(e)) & (0xFFFFFFFF >> (31-((b)-(e)))))
#define	GSC_FIELD_ENCODE(v,b,e)		(((v) << (e)) & ((0xFFFFFFFF << (e)) & (0xFFFFFFFF >> (31 - (b)))))



// Register definitions:
//	bits 0-2: Type: 0-5=BARx, 6=PCI, 7=ALT (device dependent)
//	bits 3-4: Size (bytes - 1)
//	bits 5-21: Offset (128K range)
//	bits 22-31: unused
// OTHER PRODUCTS USE THESE FIELDS AND THEREFORE MUST MATCH THIS LAYOUT.
// IF THIS LAYOUT IS CHANGED, THEN THOSE PRODUCT FILES MUST BE UPDATED.

#define	GSC_REG_TYPE_DECODE(r)			(GSC_FIELD_DECODE((r),   2, 0))
#define	GSC_REG_TYPE_ENCODE(t)			(GSC_FIELD_ENCODE((t),   2, 0))

#define	GSC_REG_SIZE_DECODE(r)			(GSC_FIELD_DECODE((r),   4, 3)+1)
#define	GSC_REG_SIZE_ENCODE(s)			(GSC_FIELD_ENCODE((s)-1, 4, 3))

#define	GSC_REG_OFFSET_DECODE(r)		(GSC_FIELD_DECODE((r),  21, 5))
#define	GSC_REG_OFFSET_ENCODE(o)		(GSC_FIELD_ENCODE((o),  21, 5))

#define	GSC_REG_ENCODE(t,s,o)			( GSC_REG_TYPE_ENCODE(t)	\
										| GSC_REG_SIZE_ENCODE(s)	\
										| GSC_REG_OFFSET_ENCODE(o))

#define	GSC_REG_TYPE(r)					GSC_REG_TYPE_DECODE((r))
#define	GSC_REG_SIZE(r)					GSC_REG_SIZE_DECODE((r))
#define	GSC_REG_OFFSET(r)				GSC_REG_OFFSET_DECODE((r))

#define	GSC_REG_TYPE_BAR0				0
#define	GSC_REG_TYPE_BAR1				1
#define	GSC_REG_TYPE_BAR2				2
#define	GSC_REG_TYPE_BAR3				3
#define	GSC_REG_TYPE_BAR4				4
#define	GSC_REG_TYPE_BAR5				5
#define	GSC_REG_TYPE_PCI				6
#define	GSC_REG_TYPE_ALT				7



// I/O Size macros
#define	GSC_IO_SIZE_QTY_MASK			GSC_FIELD_DECODE(0xFFFFFFFF,GSC_IO_SIZE_FLASG_SHIFT - 1, 0)
#define	GSC_IO_SIZE_FLASG_SHIFT			28


// I/O Timeout macros
#define	GSC_IO_TIMEOUT_MAX				3600	// 1 hour, in seconds
#define	GSC_IO_TIMEOUT_INFINITE			(GSC_IO_TIMEOUT_MAX + 1)



// Wait macros: gsc_wait_t.flags
#define	GSC_WAIT_FLAG_CANCEL			0x0001	// a wait was cancelled
#define	GSC_WAIT_FLAG_DONE				0x0002	// a desired event occurred
#define	GSC_WAIT_FLAG_TIMEOUT			0x0004	// the timeout period lapsed
#define	GSC_WAIT_FLAG_ALL				0x0007	// all flags
#define	GSC_WAIT_FLAG_INTERNAL			0x8000	// used internally

// Wait macros: gsc_wait_t.main
#define	GSC_WAIT_MAIN_PCI				0x0001	// main PCI interrupt
#define	GSC_WAIT_MAIN_DMA0				0x0002	// DMA0 done
#define	GSC_WAIT_MAIN_DMA1				0x0004	// DMA1 done
#define	GSC_WAIT_MAIN_GSC				0x0008	// firmware or similar interrupt
#define	GSC_WAIT_MAIN_OTHER				0x0010	// most likely a shared interrupt
#define	GSC_WAIT_MAIN_SPURIOUS			0x0020	// an unexpected interrupt
#define	GSC_WAIT_MAIN_UNKNOWN			0x0040	// device interrupt of unknown origin
#define	GSC_WAIT_MAIN_ALL				0x007F

// Wait macros: gsc_wait_t.io
// See the device API header file.

// This pertains to the code implemented for obtaining register access metrics.
#define	GSC_METRICS						1
// 1	Register I/O with gsc_reg_t.reg == 0 is supported as a special case.

// Wait macros: gsc_wait_t.timeout (in seconds)
#define	GSC_WAIT_TIMEOUT_MAX			(60L * 60L * 1000L)	// 1 hour



// data types *****************************************************************

typedef enum
{								// Vendor	Device	SubVen	SubDev	Additional
	GSC_DEV_TYPE_ADADIO,		// 0x10B5	0x9080	0x10B5	0x2370
	GSC_DEV_TYPE_6SDI,			// 0x10B5	0x9080	0x10B5	0x2408
	GSC_DEV_TYPE_16SDI,			// 0x10B5	0x9080	0x10B5	0x2371
	GSC_DEV_TYPE_16SDI_HS,		// 0x10B5	0x9080	0x10B5	0x2306	PMC: not distinguishable
								// 0x10B5	0x9080	0x10B5	0x2449	PCI: BRR.D8=0
	GSC_DEV_TYPE_16HSDI,		// 0x10B5	0x9080	0x10B5	0x2306	PMC: not distinguishable
								// 0x10B5	0x9080	0x10B5	0x2449	PCI: BRR.D8=1
	GSC_DEV_TYPE_24DSI12,		// 0x10B5	0x9080	0x10B5	0x3100	BrdCfg.D22=0
								// 0x10B5	0x9056	0x10B5	0x3540
	GSC_DEV_TYPE_24DSI32,		// 0x10B5	0x9080	0x10B5	0x2974
								// 0x10B5	0x9056	0x10B5	0x3547
	GSC_DEV_TYPE_24DSI6,		// 0x10B5	0x9080	0x10B5	0x3100	BrdCfg.D22=1
	GSC_DEV_TYPE_16AI32SSC,		// 0x10B5	0x9056	0x10B5	0x3101	@0x40 != BCR, @0x80 != BCR
	GSC_DEV_TYPE_18AI32SSC1M,	// 0x10B5	0x9056	0x10B5	0x3101	@0x40 != BCR, @0x80 == BCR, RAGR != 0x00010FA0
								// 0x10B5	0x9056	0x10B5	0x3431	This board has two id means.
	GSC_DEV_TYPE_16AO20,		// 0x10B5	0x9080	0x10B5	0x3102
	GSC_DEV_TYPE_16AISS16AO2,	// 0x10B5	0x9056	0x10B5	0x3243
	GSC_DEV_TYPE_16HSDI4AO4,	// 0x10B5	0x9056	0x10B5	0x3428
	GSC_DEV_TYPE_12AISS8AO4,	// 0x10B5	0x9080	0x10B5	0x2880
	GSC_DEV_TYPE_16AO16,		// 0x10B5	0x9056	0x10B5	0x3120	includes 16AO16FLV
	GSC_DEV_TYPE_16AIO,			// 0x10B5	0x9080	0x10B5	0x2402
	GSC_DEV_TYPE_12AIO,			// 0x10B5	0x9080	0x10B5	0x2409
	GSC_DEV_TYPE_16AO12,		// 0x10B5	0x9080	0x10B5	0x2405
	GSC_DEV_TYPE_SIO4,			// 0x10B5	0x9080	0x10B5	0x2401	FW Type = 0x00 or 0x01
								// 0x10B5	0x9056	0x10B5	0x3198
	GSC_DEV_TYPE_SIO4_SYNC,		// 0x10B5	0x9080	0x10B5	0x2401	FW Type = 0x04
								// 0x10B5	0x9056	0x10B5	0x3198
	GSC_DEV_TYPE_14HSAI4,		// 0x10B5	0x9056	0x10B5	0x3300
	GSC_DEV_TYPE_HPDI32,		// 0x10B5	0x9080	0x10B5	0x2400	32-bit, see note below
								// 0x10B5	0x9656	0x10B5	0x2705	64-bit
	// NOTE: 9080/2400 id values are also used by generic HPDI32 model boards.
	// However, COS boards have 0x10 in FRR.D16-D23, which is checked by both drivers.
	GSC_DEV_TYPE_OPTO16X16,		// 0x10B5	0x9056	0x10B5	0x3460
	GSC_DEV_TYPE_16AI32SSA,		// 0x10B5	0x9056	0x10B5	0x3101	@0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 1, RAGR != 0x00010FA0
	GSC_DEV_TYPE_24DSI16WRC,	// 0x10B5	0x9056	0x10B5	0x3466
	GSC_DEV_TYPE_16AIO168,		// 0x10B5	0x9080	0x10B5	0x2879
	GSC_DEV_TYPE_OPTO32,		// 0x10B5	0x906E	0x10B5	0x9080
								// 0x10B5	0x9056	0x10B5	0x3471
	GSC_DEV_TYPE_16AI64SSA,		// 0x10B5	0x9056	0x10B5	0x3101	@0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 0, Firmware == 0x0100-0x02xx
								// 0x10B5	0x9080	0x10B5	0x2868	same
	GSC_DEV_TYPE_24DSI20C500K,	// 0x10B5	0x9656	0x10B5	0x3490
	GSC_DEV_TYPE_18AISS6C,		// 0x10B5	0x9056	0x10B5	0x3467
	GSC_DEV_TYPE_16AICS32,		// 0x10B5	0x9080	0x10B5	0x3010
	GSC_DEV_TYPE_20AOF16C500KR,	// 0x10B5	0x9656	0x10B5	0x3491
	GSC_DEV_TYPE_16AI64SSC,		// 0x10B5	0x9056	0x10B5	0x3101	see below
								// 0x10B5	0x9080	0x10B5	0x2868	see below
								// --------------------- option #1	@0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 0, Firmware != 0x0100-0x02xx
								// --------------------- option #2	@0x40 == BCR, @0x80 == BCR, BCFGR.D15 == 1, RAGR == 0x00010FA0
								// --------------------- option #3	@0x40 != BCR, @0x80 == BCR, BCFGR.D15 == 1, RAGR == 0x00010FA0
	GSC_DEV_TYPE_PEX8111,		// 0x10B5	0x8111	0x0000	0x0000	PCI Express to PCI bus bridge
	GSC_DEV_TYPE_PEX8112,		// 0x10B5	0x8112	0x0000	0x0000	PCI Express to PCI bus bridge
	GSC_DEV_TYPE_16AISS8AO4,	// 0x10B5	0x9056	0x10B5	0x3172
	GSC_DEV_TYPE_18AO8,			// 0x10B5	0x9056	0x10B5	0x3357
	GSC_DEV_TYPE_16AO64C,		// 0x10B5	0x9056	0x10B5	0x3555
	GSC_DEV_TYPE_16AI64,		// 0x10B5	0x9080	0x10B5	0x2407
	GSC_DEV_TYPE_DIO24,			// 0x10B5	0x9080	0x10B5	0x2706
	GSC_DEV_TYPE_24DSI6C500K,	// 0x10B5	0x9056	0x10B5	0x3551
	GSC_DEV_TYPE_24DSI12WRCIEPE,// 0x10B5	0x9056	0x10B5	0x3573
	GSC_DEV_TYPE_16AO4MF,		// 0x10B5	0x9080	0x10B5	0x3064
	GSC_DEV_TYPE_DIO40,			// 0x10B5	0x9656	0x10B5	0x3191
	GSC_DEV_TYPE_DIO32,			// 0x10B5	0x9056	0x10B5	0x2706
	GSC_DEV_TYPE_12AI64,		// 0x10B5	0x9080	0x10B5	0x2406
	GSC_DEV_TYPE_18AI64SSC750K,	// 0x10B5	0x9056	0x10B5	0x3570
	GSC_DEV_TYPE_24DSI64C200K,	// 0x10B5	0x9056	0x10B5	0x3575
	GSC_DEV_TYPE_16AI32SSC1M,	// 0x10B5	0x9056	0x10B5	0x3592
	GSC_DEV_TYPE_16AISS2AO2A2M,	// 0x10B5	0x9056	0x10B5	0x3595
	GSC_DEV_TYPE_18AISS8AO8,	// 0x10B5	0x9056	0x10B5	0x3251
	GSC_DEV_TYPE_ADADIO2,		// 0x10B5	0x9080	0x10B5	0x2370	Actually uses 9056, firmwware is 4xxx
	GSC_DEV_TYPE_16AI64SSC5,	// 0x1C6E	0xA002	0x1C6E	0xA002
	GSC_DEV_TYPE_24DSI6LN4AO,	// 0x10B5	0x9056	0x10B5	0x3608
	GSC_DEV_TYPE_20AO8C500K,	// 0x10B5	0x9056	0x10B5	0x3574
	GSC_DEV_TYPE_SIO4_FASYNC,	// 0x10B5	0x9056	0x10B5	0x3198	FW Type = 0x08
	GSC_DEV_TYPE_TSI381,		// 0x10B5	0x10E3	0x0000	0x0000	same as PEX8111
	GSC_DEV_TYPE_HPDI32COS		// 0x10B5	0x9080	0x10B5	0x2400	see note below
	// NOTE: 9080/2400 id values are also used by generic HPDI32 model boards.
	// However, COS boards have 0x10 in FRR.D16-D23, which is checked by both drivers.

} gsc_dev_type_t;

typedef enum
{
	GSC_IO_MODE_PIO,		// Programmed I/O
	GSC_IO_MODE_BMDMA,		// Block Mode DMA
	GSC_IO_MODE_DMDMA		// Demand Mode DMA
} gsc_io_mode_t;

typedef struct
{
	u32	reg;	// range: any valid register definition
	u32	value;	// range: 0x0-0xFFFFFFFF
	u32	mask;	// range: 0x0-0xFFFFFFFF
} gsc_reg_t;

typedef enum
{
	GSC_VPD_TYPE_NAME,
	GSC_VPD_TYPE_MODEL_NUMBER,
	GSC_VPD_TYPE_SERIAL_NUMBER
} gsc_vpd_type_t;

typedef struct
{
	s32	type;
	s32	reserved;
	u8	data[128];
} gsc_vpd_t;

typedef struct
{
	u32	flags;		// done, timeout, cancel, ...
	u32	main;		// interrupts: PCI, DMA0, DMA1, Local, Other
	u32	gsc;		// firmware specific interrupts
	u32	alt;		// additional device interrupts, Ex. SIO4 Zilog interrupts
	u32	io;			// read and write call completion
	u32	timeout_ms;	// milliseconds
	u32	count;		// status: number of awaiting threads meeting any criteria
					// cancel: number of waits cancelled
} gsc_wait_t;



#endif

