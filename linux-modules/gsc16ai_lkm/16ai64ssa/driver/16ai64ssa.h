// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/16ai64ssa.h $
// $Rev: 53568 $
// $Date: 2023-08-07 16:29:27 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Device Driver: header file

#ifndef	__16AI64SSA_H__
#define	__16AI64SSA_H__

#include "gsc_common.h"
#include "gsc_pci9056.h"
#include "gsc_pci9080.h"



// macros *********************************************************************

#define	AI64SSA_BASE_NAME					"16ai64ssa"

// IOCTL command codes
#define	AI64SSA_IOCTL_REG_READ				OS_IOCTL_RW( 0,  12, gsc_reg_t)
#define	AI64SSA_IOCTL_REG_WRITE				OS_IOCTL_W ( 1,  12, gsc_reg_t)
#define	AI64SSA_IOCTL_REG_MOD				OS_IOCTL_W ( 2,  12, gsc_reg_t)
#define	AI64SSA_IOCTL_QUERY					OS_IOCTL_RW( 3,	  4, s32)
#define	AI64SSA_IOCTL_INITIALIZE			OS_IOCTL   ( 4)
#define	AI64SSA_IOCTL_AUTOCAL				OS_IOCTL   ( 5)
#define	AI64SSA_IOCTL_AUTOCAL_STATUS		OS_IOCTL_R ( 6,	  4, s32)
#define	AI64SSA_IOCTL_AI_BUF_CLEAR			OS_IOCTL   ( 7)
#define	AI64SSA_IOCTL_AI_BUF_LEVEL			OS_IOCTL_R ( 8,	  4, s32)
#define	AI64SSA_IOCTL_AI_BUF_OVERFLOW		OS_IOCTL_RW( 9,	  4, s32)
#define	AI64SSA_IOCTL_AI_BUF_THR_LVL		OS_IOCTL_RW(10,	  4, s32)
#define	AI64SSA_IOCTL_AI_BUF_THR_STS		OS_IOCTL_R (11,	  4, s32)
#define	AI64SSA_IOCTL_AI_BUF_UNDERFLOW		OS_IOCTL_RW(12,	  4, s32)
#define	AI64SSA_IOCTL_AI_MODE				OS_IOCTL_RW(13,	  4, s32)
#define	AI64SSA_IOCTL_AI_RANGE				OS_IOCTL_RW(14,	  4, s32)
#define	AI64SSA_IOCTL_AUX_0_MODE			OS_IOCTL_RW(15,	  4, s32)
#define	AI64SSA_IOCTL_AUX_1_MODE			OS_IOCTL_RW(16,	  4, s32)
#define	AI64SSA_IOCTL_AUX_2_MODE			OS_IOCTL_RW(17,	  4, s32)
#define	AI64SSA_IOCTL_AUX_3_MODE			OS_IOCTL_RW(18,	  4, s32)
#define	AI64SSA_IOCTL_AUX_IN_POL			OS_IOCTL_RW(19,	  4, s32)
#define	AI64SSA_IOCTL_AUX_NOISE				OS_IOCTL_RW(20,	  4, s32)
#define	AI64SSA_IOCTL_AUX_OUT_POL			OS_IOCTL_RW(21,	  4, s32)
#define	AI64SSA_IOCTL_BURST_SIZE			OS_IOCTL_RW(22,	  4, s32)
#define	AI64SSA_IOCTL_BURST_SRC				OS_IOCTL_RW(23,	  4, s32)
#define	AI64SSA_IOCTL_BURST_STATUS			OS_IOCTL_R (24,	  4, s32)
#define	AI64SSA_IOCTL_CHAN_ACTIVE			OS_IOCTL_RW(25,	  4, s32)
#define	AI64SSA_IOCTL_CHAN_FIRST			OS_IOCTL_RW(26,	  4, s32)
#define	AI64SSA_IOCTL_CHAN_LAST				OS_IOCTL_RW(27,	  4, s32)
#define	AI64SSA_IOCTL_CHAN_SINGLE			OS_IOCTL_RW(28,	  4, s32)
#define	AI64SSA_IOCTL_DATA_FORMAT			OS_IOCTL_RW(29,	  4, s32)
#define	AI64SSA_IOCTL_DATA_PACKING			OS_IOCTL_RW(30,	  4, s32)
#define	AI64SSA_IOCTL_EXT_SYNC_ENABLE		OS_IOCTL_RW(31,	  4, s32)
#define	AI64SSA_IOCTL_INPUT_SYNC			OS_IOCTL   (32)
#define	AI64SSA_IOCTL_IRQ0_SEL				OS_IOCTL_RW(33,	  4, s32)
#define	AI64SSA_IOCTL_IRQ1_SEL				OS_IOCTL_RW(34,	  4, s32)
#define	AI64SSA_IOCTL_LOW_LAT_DOH			OS_IOCTL_R (35,	  4, s32)
#define	AI64SSA_IOCTL_LOW_LAT_READ			OS_IOCTL_R (36, 128, ai64ssa_ll_t)
#define	AI64SSA_IOCTL_RAG_ENABLE			OS_IOCTL_RW(37,	  4, s32)
#define	AI64SSA_IOCTL_RAG_NRATE				OS_IOCTL_RW(38,	  4, s32)
#define	AI64SSA_IOCTL_RBG_CLK_SRC			OS_IOCTL_RW(39,	  4, s32)
#define	AI64SSA_IOCTL_RBG_ENABLE			OS_IOCTL_RW(40,	  4, s32)
#define	AI64SSA_IOCTL_RBG_NRATE				OS_IOCTL_RW(41,	  4, s32)
#define	AI64SSA_IOCTL_RX_IO_ABORT			OS_IOCTL_R (42,	  4, s32)
#define	AI64SSA_IOCTL_RX_IO_MODE			OS_IOCTL_RW(43,	  4, s32)
#define	AI64SSA_IOCTL_RX_IO_OVERFLOW		OS_IOCTL_RW(44,	  4, s32)
#define	AI64SSA_IOCTL_RX_IO_TIMEOUT			OS_IOCTL_RW(45,	  4, s32)
#define	AI64SSA_IOCTL_RX_IO_UNDERFLOW		OS_IOCTL_RW(46,	  4, s32)
#define	AI64SSA_IOCTL_SAMP_CLK_SRC			OS_IOCTL_RW(47,	  4, s32)
#define	AI64SSA_IOCTL_SCAN_MARKER			OS_IOCTL_RW(48,	  4, s32)
#define	AI64SSA_IOCTL_SCAN_MARKER_GET		OS_IOCTL_R (49,	  4, u32)
#define	AI64SSA_IOCTL_SCAN_MARKER_SET		OS_IOCTL_W (50,	  4, u32)
#define	AI64SSA_IOCTL_WAIT_CANCEL			OS_IOCTL_RW(51,  28, gsc_wait_t)
#define	AI64SSA_IOCTL_WAIT_EVENT			OS_IOCTL_RW(52,  28, gsc_wait_t)
#define	AI64SSA_IOCTL_WAIT_STATUS			OS_IOCTL_RW(53,  28, gsc_wait_t)
#define	AI64SSA_IOCTL_LOW_LAT_HOLD_CHAN		OS_IOCTL_RW(54,	  4, s32)
#define	AI64SSA_IOCTL_LOW_LAT_REL_CHAN		OS_IOCTL_RW(55,	  4, s32)

//*****************************************************************************
// AI64SSA_IOCTL_REG_READ
// AI64SSA_IOCTL_REG_WRITE
// AI64SSA_IOCTL_REG_MOD
//
#define	AI64SSA_REG_ENCODE(s,o)				GSC_REG_ENCODE(GSC_REG_TYPE_BAR2,(s),(o))
// Parameter:	gsc_reg_t*
#define	AI64SSA_GSC_BCTLR					AI64SSA_REG_ENCODE(4,0x000)// Board Control Register
#define	AI64SSA_GSC_ICR						AI64SSA_REG_ENCODE(4,0x004)// Interrupt Control Register
#define	AI64SSA_GSC_IDBR					AI64SSA_REG_ENCODE(4,0x008)// Input Data Buffer Register
#define	AI64SSA_GSC_IBCR					AI64SSA_REG_ENCODE(4,0x00C)// Input Buffer Control Register
#define	AI64SSA_GSC_RAGR					AI64SSA_REG_ENCODE(4,0x010)// Rate-A Generator Register
#define	AI64SSA_GSC_RBGR					AI64SSA_REG_ENCODE(4,0x014)// Rate-B Generator Register
#define	AI64SSA_GSC_BUFSR					AI64SSA_REG_ENCODE(4,0x018)// Buffer Size Register
#define	AI64SSA_GSC_BURSR					AI64SSA_REG_ENCODE(4,0x01C)// Burst Size Register
#define	AI64SSA_GSC_SSCR					AI64SSA_REG_ENCODE(4,0x020)// Scan & Sync Control Register
#define	AI64SSA_GSC_ACAR					AI64SSA_REG_ENCODE(4,0x024)// Active Channel Assignment Register
#define	AI64SSA_GSC_BCFGR					AI64SSA_REG_ENCODE(4,0x028)// Board Configuration Register
#define	AI64SSA_GSC_AVR						AI64SSA_REG_ENCODE(4,0x02C)// Autocal Values Register
#define	AI64SSA_GSC_ARWR					AI64SSA_REG_ENCODE(4,0x030)// Auxiliary R/W Register
#define	AI64SSA_GSC_ASIOCR					AI64SSA_REG_ENCODE(4,0x034)// Auxiliary Sync I/O Control Register
#define	AI64SSA_GSC_SMUWR					AI64SSA_REG_ENCODE(4,0x038)// Scan Marker Upper Word Register
#define	AI64SSA_GSC_SMLWR					AI64SSA_REG_ENCODE(4,0x03C)// Scan Marker Lower Word Register
#define	AI64SSA_GSC_LLCR					AI64SSA_REG_ENCODE(4,0x040)// Low Latency Control Register

#define	AI64SSA_GSC_LLHR00					AI64SSA_REG_ENCODE(2,0x100)// Low Latency Holding Register 00
#define	AI64SSA_GSC_LLHR01					AI64SSA_REG_ENCODE(2,0x104)// Low Latency Holding Register 01
#define	AI64SSA_GSC_LLHR02					AI64SSA_REG_ENCODE(2,0x108)// Low Latency Holding Register 02
#define	AI64SSA_GSC_LLHR03					AI64SSA_REG_ENCODE(2,0x10C)// Low Latency Holding Register 03
#define	AI64SSA_GSC_LLHR04					AI64SSA_REG_ENCODE(2,0x110)// Low Latency Holding Register 04
#define	AI64SSA_GSC_LLHR05					AI64SSA_REG_ENCODE(2,0x114)// Low Latency Holding Register 05
#define	AI64SSA_GSC_LLHR06					AI64SSA_REG_ENCODE(2,0x118)// Low Latency Holding Register 06
#define	AI64SSA_GSC_LLHR07					AI64SSA_REG_ENCODE(2,0x11C)// Low Latency Holding Register 07
#define	AI64SSA_GSC_LLHR08					AI64SSA_REG_ENCODE(2,0x120)// Low Latency Holding Register 08
#define	AI64SSA_GSC_LLHR09					AI64SSA_REG_ENCODE(2,0x124)// Low Latency Holding Register 09
#define	AI64SSA_GSC_LLHR10					AI64SSA_REG_ENCODE(2,0x128)// Low Latency Holding Register 10
#define	AI64SSA_GSC_LLHR11					AI64SSA_REG_ENCODE(2,0x12C)// Low Latency Holding Register 11
#define	AI64SSA_GSC_LLHR12					AI64SSA_REG_ENCODE(2,0x130)// Low Latency Holding Register 12
#define	AI64SSA_GSC_LLHR13					AI64SSA_REG_ENCODE(2,0x134)// Low Latency Holding Register 13
#define	AI64SSA_GSC_LLHR14					AI64SSA_REG_ENCODE(2,0x138)// Low Latency Holding Register 14
#define	AI64SSA_GSC_LLHR15					AI64SSA_REG_ENCODE(2,0x13C)// Low Latency Holding Register 15
#define	AI64SSA_GSC_LLHR16					AI64SSA_REG_ENCODE(2,0x140)// Low Latency Holding Register 16
#define	AI64SSA_GSC_LLHR17					AI64SSA_REG_ENCODE(2,0x144)// Low Latency Holding Register 17
#define	AI64SSA_GSC_LLHR18					AI64SSA_REG_ENCODE(2,0x148)// Low Latency Holding Register 18
#define	AI64SSA_GSC_LLHR19					AI64SSA_REG_ENCODE(2,0x14C)// Low Latency Holding Register 19
#define	AI64SSA_GSC_LLHR20					AI64SSA_REG_ENCODE(2,0x150)// Low Latency Holding Register 20
#define	AI64SSA_GSC_LLHR21					AI64SSA_REG_ENCODE(2,0x154)// Low Latency Holding Register 21
#define	AI64SSA_GSC_LLHR22					AI64SSA_REG_ENCODE(2,0x158)// Low Latency Holding Register 22
#define	AI64SSA_GSC_LLHR23					AI64SSA_REG_ENCODE(2,0x15C)// Low Latency Holding Register 23
#define	AI64SSA_GSC_LLHR24					AI64SSA_REG_ENCODE(2,0x160)// Low Latency Holding Register 24
#define	AI64SSA_GSC_LLHR25					AI64SSA_REG_ENCODE(2,0x164)// Low Latency Holding Register 25
#define	AI64SSA_GSC_LLHR26					AI64SSA_REG_ENCODE(2,0x168)// Low Latency Holding Register 26
#define	AI64SSA_GSC_LLHR27					AI64SSA_REG_ENCODE(2,0x16C)// Low Latency Holding Register 27
#define	AI64SSA_GSC_LLHR28					AI64SSA_REG_ENCODE(2,0x170)// Low Latency Holding Register 28
#define	AI64SSA_GSC_LLHR29					AI64SSA_REG_ENCODE(2,0x174)// Low Latency Holding Register 29
#define	AI64SSA_GSC_LLHR30					AI64SSA_REG_ENCODE(2,0x178)// Low Latency Holding Register 30
#define	AI64SSA_GSC_LLHR31					AI64SSA_REG_ENCODE(2,0x17C)// Low Latency Holding Register 31
#define	AI64SSA_GSC_LLHR32					AI64SSA_REG_ENCODE(2,0x180)// Low Latency Holding Register 32
#define	AI64SSA_GSC_LLHR33					AI64SSA_REG_ENCODE(2,0x184)// Low Latency Holding Register 33
#define	AI64SSA_GSC_LLHR34					AI64SSA_REG_ENCODE(2,0x188)// Low Latency Holding Register 34
#define	AI64SSA_GSC_LLHR35					AI64SSA_REG_ENCODE(2,0x18C)// Low Latency Holding Register 35
#define	AI64SSA_GSC_LLHR36					AI64SSA_REG_ENCODE(2,0x190)// Low Latency Holding Register 36
#define	AI64SSA_GSC_LLHR37					AI64SSA_REG_ENCODE(2,0x194)// Low Latency Holding Register 37
#define	AI64SSA_GSC_LLHR38					AI64SSA_REG_ENCODE(2,0x198)// Low Latency Holding Register 38
#define	AI64SSA_GSC_LLHR39					AI64SSA_REG_ENCODE(2,0x19C)// Low Latency Holding Register 39
#define	AI64SSA_GSC_LLHR40					AI64SSA_REG_ENCODE(2,0x1A0)// Low Latency Holding Register 40
#define	AI64SSA_GSC_LLHR41					AI64SSA_REG_ENCODE(2,0x1A4)// Low Latency Holding Register 41
#define	AI64SSA_GSC_LLHR42					AI64SSA_REG_ENCODE(2,0x1A8)// Low Latency Holding Register 42
#define	AI64SSA_GSC_LLHR43					AI64SSA_REG_ENCODE(2,0x1AC)// Low Latency Holding Register 43
#define	AI64SSA_GSC_LLHR44					AI64SSA_REG_ENCODE(2,0x1B0)// Low Latency Holding Register 44
#define	AI64SSA_GSC_LLHR45					AI64SSA_REG_ENCODE(2,0x1B4)// Low Latency Holding Register 45
#define	AI64SSA_GSC_LLHR46					AI64SSA_REG_ENCODE(2,0x1B8)// Low Latency Holding Register 46
#define	AI64SSA_GSC_LLHR47					AI64SSA_REG_ENCODE(2,0x1BC)// Low Latency Holding Register 47
#define	AI64SSA_GSC_LLHR48					AI64SSA_REG_ENCODE(2,0x1C0)// Low Latency Holding Register 48
#define	AI64SSA_GSC_LLHR49					AI64SSA_REG_ENCODE(2,0x1C4)// Low Latency Holding Register 49
#define	AI64SSA_GSC_LLHR50					AI64SSA_REG_ENCODE(2,0x1C8)// Low Latency Holding Register 50
#define	AI64SSA_GSC_LLHR51					AI64SSA_REG_ENCODE(2,0x1CC)// Low Latency Holding Register 51
#define	AI64SSA_GSC_LLHR52					AI64SSA_REG_ENCODE(2,0x1D0)// Low Latency Holding Register 52
#define	AI64SSA_GSC_LLHR53					AI64SSA_REG_ENCODE(2,0x1D4)// Low Latency Holding Register 53
#define	AI64SSA_GSC_LLHR54					AI64SSA_REG_ENCODE(2,0x1D8)// Low Latency Holding Register 54
#define	AI64SSA_GSC_LLHR55					AI64SSA_REG_ENCODE(2,0x1DC)// Low Latency Holding Register 55
#define	AI64SSA_GSC_LLHR56					AI64SSA_REG_ENCODE(2,0x1E0)// Low Latency Holding Register 56
#define	AI64SSA_GSC_LLHR57					AI64SSA_REG_ENCODE(2,0x1E4)// Low Latency Holding Register 57
#define	AI64SSA_GSC_LLHR58					AI64SSA_REG_ENCODE(2,0x1E8)// Low Latency Holding Register 58
#define	AI64SSA_GSC_LLHR59					AI64SSA_REG_ENCODE(2,0x1EC)// Low Latency Holding Register 59
#define	AI64SSA_GSC_LLHR60					AI64SSA_REG_ENCODE(2,0x1F0)// Low Latency Holding Register 60
#define	AI64SSA_GSC_LLHR61					AI64SSA_REG_ENCODE(2,0x1F4)// Low Latency Holding Register 61
#define	AI64SSA_GSC_LLHR62					AI64SSA_REG_ENCODE(2,0x1F8)// Low Latency Holding Register 62
#define	AI64SSA_GSC_LLHR63					AI64SSA_REG_ENCODE(2,0x1FC)// Low Latency Holding Register 63

#define	AI64SSA_GSC_ACVR					AI64SSA_GSC_AVR	// retained for compatibility

//*****************************************************************************
// AI64SSA_IOCTL_QUERY
//
//	Parameter:	s32
//		Pass in a value from the list below.
//		The value returned is the answer to the query.

typedef enum
{
	AI64SSA_QUERY_AUTOCAL_MS,		// Max autocalibration period in ms.
	AI64SSA_QUERY_CHANNEL_MAX,		// Maximum number of channels supported.
	AI64SSA_QUERY_CHANNEL_QTY,		// The number of A/D channels.
	AI64SSA_QUERY_COUNT,			// The number of query options.
	AI64SSA_QUERY_DATA_PACKING,		// Is Data Packing supported?
	AI64SSA_QUERY_DEVICE_TYPE,		// Value from gsc_dev_type_t
	AI64SSA_QUERY_FIFO_SIZE,		// FIFO depth in 32-bit samples
	AI64SSA_QUERY_FSAMP_MAX,		// The maximum sample rate per channel.
	AI64SSA_QUERY_FSAMP_MIN,		// The minimum sample rate per channel.
	AI64SSA_QUERY_INIT_MS,			// Max initialize period in ms.
	AI64SSA_QUERY_LOW_LATENCY,		// Is Low Latency supported?
	AI64SSA_QUERY_MASTER_CLOCK,		// Master clock frequency
	AI64SSA_QUERY_NRATE_MAX,		// Maximum rate generator Nrate value.
	AI64SSA_QUERY_NRATE_MIN,		// Maximum rate generator Nrate value.
	AI64SSA_QUERY_RATE_GEN_QTY,		// Number of Rate Generatorts.
	AI64SSA_QUERY_REG_LLCR,			// Low Latency Control Reg. supported?
	AI64SSA_QUERY_CHAN_RANGE,		// Is the range option available?
	AI64SSA_QUERY_REG_ACAR,			// Active Channel Assignment Reg. supported?
	AI64SSA_QUERY_IRQ1_BUF_ERROR,	// Is this interrupt supported?
	AI64SSA_IOCTL_QUERY_LAST		// MUST BE LAST!
} ai64ssa_query_t;

#define	AI64SSA_IOCTL_QUERY_ERROR			(-1)

// Retained for backwards compatibility:
#define	AI64SSA_QUERY_AUTO_CAL_MS	AI64SSA_QUERY_AUTOCAL_MS

//*****************************************************************************
// AI64SSA_IOCTL_INITIALIZE					BCTLR D15
//
//	Parameter:	None

//*****************************************************************************
// AI64SSA_IOCTL_AUTOCAL					BCTLR D13
//
//	Parameter:	None

// Retained for backwards compatibility:
#define	AI64SSA_IOCTL_AUTO_CALIBRATE		AI64SSA_IOCTL_AUTOCAL

//*****************************************************************************
// AI64SSA_IOCTL_AUTOCAL_STATUS				BCTLR D13, D14
//
//	Parameter:	s32*
//		The value returned is one of the following.
#define	AI64SSA_AUTOCAL_STATUS_ACTIVE		0
#define	AI64SSA_AUTOCAL_STATUS_FAIL			1
#define	AI64SSA_AUTOCAL_STATUS_PASS			2

// Retained for backwards compatibiliyu:
#define	AI64SSA_IOCTL_AUTO_CAL_STS			AI64SSA_IOCTL_AUTOCAL_STATUS
#define	AI64SSA_AUTO_CAL_STS_ACTIVE			AI64SSA_AUTOCAL_STATUS_ACTIVE
#define	AI64SSA_AUTO_CAL_STS_FAIL			AI64SSA_AUTOCAL_STATUS_FAIL
#define	AI64SSA_AUTO_CAL_STS_PASS			AI64SSA_AUTOCAL_STATUS_PASS

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_CLEAR				9080 IBCR D16, else IBCR D18
//
//	Parameter:	None.

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_LEVEL				9080 BUFSR D0-D17, else BUFSR D0-D18
//
//	Parameter:	s32*
//		The value returned is the current input buffer fill level.

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_OVERFLOW			BCTLR D17
//
//	Parameter:	s32*
//		Pass in any valid option below, or
//		-1 to read the current setting.
#define	AI64SSA_AI_BUF_OVERFLOW_CLEAR		0
#define	AI64SSA_AI_BUF_OVERFLOW_CHECK		(-1)

// For queries the following values are returned.
#define	AI64SSA_AI_BUF_OVERFLOW_NO			0
#define	AI64SSA_AI_BUF_OVERFLOW_YES			1

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_THR_LVL				9080 IBCR D0-D15, D19, else IBCR D0-D17
//
//	Parameter:	s32*
//		Pass in any valid value from 0x0 to 0xFFFF/0x3FFFF, or
//		-1 to read the current setting.

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_THR_STS				9080 IBCR D17, else IBCR D19
//
//	Parameter:	s32*
//		The value returned is one of the following.
#define	AI64SSA_AI_BUF_THR_STS_CLEAR		0
#define	AI64SSA_AI_BUF_THR_STS_SET			1

//*****************************************************************************
// AI64SSA_IOCTL_AI_BUF_UNDERFLOW			BCTLR D16
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AI_BUF_UNDERFLOW_CLEAR		0
#define	AI64SSA_AI_BUF_UNDERFLOW_CHECK		(-1)

// For queries the following values are returned.
#define	AI64SSA_AI_BUF_UNDERFLOW_NO			0
#define	AI64SSA_AI_BUF_UNDERFLOW_YES		1

//*****************************************************************************
// AI64SSA_IOCTL_AI_MODE					BCTLR D0-D2,D8-D9
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AI_MODE_SINGLE				0x000	// Single Ended
#define	AI64SSA_AI_MODE_PS_DIFF				0x100	// Pseudo Differential
#define	AI64SSA_AI_MODE_DIFF				0x200	// Full Differential
#define	AI64SSA_AI_MODE_ZERO				0x002	// Zero test
#define	AI64SSA_AI_MODE_VREF				0x003	// Vref test

//*****************************************************************************
// AI64SSA_IOCTL_AI_RANGE					BCTLR D3-D5
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AI_RANGE_2_5V				0x0	// +- 2.5 volts
#define	AI64SSA_AI_RANGE_5V					0x2	// +- 5 volts
#define	AI64SSA_AI_RANGE_10V				0x4	// +- 10 volts
#define	AI64SSA_AI_RANGE_0_5V				0x1	// 0 to +5 volts
#define	AI64SSA_AI_RANGE_0_10V				0x3	// 0 to +10 volts

//*****************************************************************************
// AI64SSA_IOCTL_AUX_0_MODE					ASIOCR D0-D1
// AI64SSA_IOCTL_AUX_1_MODE					ASIOCR D2-D3
// AI64SSA_IOCTL_AUX_2_MODE					ASIOCR D4-D5
// AI64SSA_IOCTL_AUX_3_MODE					ASIOCR D6-D7
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AUX_MODE_DISABLE			0
#define	AI64SSA_AUX_MODE_INPUT				1
#define	AI64SSA_AUX_MODE_OUTPUT				2

//*****************************************************************************
// AI64SSA_IOCTL_AUX_IN_POL					ASIOCR D8
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AUX_IN_POL_LO_2_HI			0
#define	AI64SSA_AUX_IN_POL_HI_2_LO			1

//*****************************************************************************
// AI64SSA_IOCTL_AUX_NOISE					ASIOCR D10
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AUX_NOISE_LOW				0
#define	AI64SSA_AUX_NOISE_HIGH				1

//*****************************************************************************
// AI64SSA_IOCTL_AUX_OUT_POL				ASIOCR D9
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_AUX_OUT_POL_HI_PULSE		0
#define	AI64SSA_AUX_OUT_POL_LOW_PULSE		1

//*****************************************************************************
// AI64SSA_IOCTL_BURST_STATUS				SSCR D7
//
//	Parameter:	s32*
//		The value returned is one of the following.
#define	AI64SSA_BURST_STATUS_IDLE			0
#define	AI64SSA_BURST_STATUS_ACTIVE			1

//*****************************************************************************
// AI64SSA_IOCTL_BURST_SIZE					BURSR D0-D19
//
//	Parameter:	s32*
//		Pass in any value between the below inclusive limits, or
//		-1 to read the current setting.
//		At this time the valid range is 0-0xFFFFF.

//*****************************************************************************
// AI64SSA_IOCTL_BURST_SRC					SSCR D8-D9
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_BURST_SRC_DISABLE			0
#define	AI64SSA_BURST_SRC_RBG				1
#define	AI64SSA_BURST_SRC_EXT				2
#define	AI64SSA_BURST_SRC_BCR				3

//*****************************************************************************
// AI64SSA_IOCTL_CHAN_ACTIVE				SSCR D0-D2
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_CHAN_ACTIVE_SINGLE			0
#define	AI64SSA_CHAN_ACTIVE_0_1				1
#define	AI64SSA_CHAN_ACTIVE_0_3				2
#define	AI64SSA_CHAN_ACTIVE_0_7				3
#define	AI64SSA_CHAN_ACTIVE_0_15			4
#define	AI64SSA_CHAN_ACTIVE_0_31			5
#define	AI64SSA_CHAN_ACTIVE_0_63			6
#define	AI64SSA_CHAN_ACTIVE_RANGE			7	// not always available

//*****************************************************************************
// AI64SSA_IOCTL_CHAN_FIRST					ACAR D0-D7
//
//	Parameter:	s32*
//		Pass in any valid value from zero to the index of the last channel, or
//		-1 to read the current setting. If the argument is greater than the
//		"last" setting, then the "last" setting is silently adjusted to evual
//		the value specified.

//*****************************************************************************
// AI64SSA_IOCTL_CHAN_LAST					ACAR D8-D15
//
//	Parameter:	s32*
//		Pass in any valid value from zero to the index of the last channel, or
//		-1 to read the current setting. If the argument is less than the
//		"first" setting, then the "first" setting is silently adjusted to evual
//		the value specified.

//*****************************************************************************
// AI64SSA_IOCTL_CHAN_SINGLE				SSCR D12-D17
//
//	Parameter:	s32*
//		Pass in any valid value from zero to the index of the last channel, or
//		-1 to read the current setting.

//*****************************************************************************
// AI64SSA_IOCTL_DATA_FORMAT				BCTLR D6
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_DATA_FORMAT_2S_COMP			0	// Twos Compliment
#define	AI64SSA_DATA_FORMAT_OFF_BIN			1	// Offset Binary

//*****************************************************************************
// AI64SSA_IOCTL_DATA_PACKING				BCTLR D18
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_DATA_PACKING_DISABLE		0
#define	AI64SSA_DATA_PACKING_ENABLE			1

//*****************************************************************************
// AI64SSA_IOCTL_EXT_SYNC_ENABLE			BCTLR D7
//
//	Parameter:	s32*
//		Pass in any valid option below, or
//		-1 to read the current setting.
#define	AI64SSA_EXT_SYNC_ENABLE_NO			0
#define	AI64SSA_EXT_SYNC_ENABLE_YES			1

//*****************************************************************************
// AI64SSA_IOCTL_INPUT_SYNC					BCTLR D12
//
//	Parameter:	None.

//*****************************************************************************
// AI64SSA_IOCTL_IRQ0_SEL					ICR D0-D2
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_IRQ0_INIT_DONE				0
#define	AI64SSA_IRQ0_AUTOCAL_DONE			1
#define	AI64SSA_IRQ0_SYNC_START				2
#define	AI64SSA_IRQ0_SYNC_DONE				3
#define	AI64SSA_IRQ0_BURST_START			4
#define	AI64SSA_IRQ0_BURST_DONE				5

// Retained for backwards compatibility:
#define	AI64SSA_IRQ0_AUTO_CAL_DONE			AI64SSA_IRQ0_AUTOCAL_DONE

//*****************************************************************************
// AI64SSA_IOCTL_IRQ1_SEL					ICR D4-D6
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_IRQ1_NONE					0
#define	AI64SSA_IRQ1_IN_BUF_THR_L2H			1
#define	AI64SSA_IRQ1_IN_BUF_THR_H2L			2
#define	AI64SSA_IRQ1_BUF_ERROR				3	// Not available on PMC-models

//*****************************************************************************
// AI64SSA_IOCTL_LOW_LAT_DOH				9080 N/A, else BCTLR D10
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_LOW_LAT_DOH_NO				0	// Data On Hold: No
#define	AI64SSA_LOW_LAT_DOH_YES				1	// Data On Hold: Yes

//*****************************************************************************
// AI64SSA_IOCTL_LOW_LAT_READ				LLHR00-LLHR63
//
//	Parameter:	ai64ssa_ll_t*

typedef struct
{
	u16	data[64];
} ai64ssa_ll_t;

//*****************************************************************************
// AI64SSA_IOCTL_RAG_ENABLE					RAGR D16
// AI64SSA_IOCTL_RBG_ENABLE					RBGR D16
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_GEN_ENABLE_NO				1
#define	AI64SSA_GEN_ENABLE_YES				0

//*****************************************************************************
// AI64SSA_IOCTL_RAG_NRATE					RAGR D0-D15
// AI64SSA_IOCTL_RBG_NRATE					RBGR D0-D15
//	Parameter:	s32*
//		Pass in any value between the lower limit and 0xFFFF, or -1 to read the
//		current setting. The lower limit is dependent on the Master Clock
//		frequency, which puts the lower limit from 150 to 259.

//*****************************************************************************
// AI64SSA_IOCTL_RBG_CLK_SRC				SSCR D10
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_RBG_CLK_SRC_MASTER			0
#define	AI64SSA_RBG_CLK_SRC_RAG				1

//*****************************************************************************
// AI64SSA_IOCTL_RX_IO_ABORT				This is a software feature.
//
//	Parameter:	s32*
//		The returned value is one of the below options.
#define	AI64SSA_IO_ABORT_NO					0
#define	AI64SSA_IO_ABORT_YES				1

//*****************************************************************************
// AI64SSA_IOCTL_RX_IO_MODE					This is a software setting.
//
//	Parameter:	s32*
//		Pass in any of the gsc_io_mode_t options, or
//		-1 to read the current setting.
#define	AI64SSA_IO_MODE_DEFAULT				GSC_IO_MODE_PIO
// GSC_IO_MODE_PIO
// GSC_IO_MODE_BMDMA
// GSC_IO_MODE_DMDMA

//*****************************************************************************
// AI64SSA_IOCTL_RX_IO_OVERFLOW				This is a software setting.
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_IO_OVERFLOW_DEFAULT			AI64SSA_IO_OVERFLOW_CHECK
#define	AI64SSA_IO_OVERFLOW_IGNORE			0
#define	AI64SSA_IO_OVERFLOW_CHECK			1

//*****************************************************************************
// AI64SSA_IOCTL_RX_IO_TIMEOUT (in seconds)	This is a software setting.
//
//	Parameter:	s32*
//		Pass in any value from the minimim to the maximim, the infinite option,
//		or -1 to read the current setting. The value -1 is returned if the
//		feature is not supported, but this should never happen.
#define	AI64SSA_IO_TIMEOUT_DEFAULT			10
#define	AI64SSA_IO_TIMEOUT_NO_SLEEP			0
#define	AI64SSA_IO_TIMEOUT_MIN				0
#define	AI64SSA_IO_TIMEOUT_MAX				GSC_IO_TIMEOUT_MAX
#define	AI64SSA_IO_TIMEOUT_INFINITE			GSC_IO_TIMEOUT_INFINITE

//*****************************************************************************
// AI64SSA_IOCTL_RX_IO_UNDERFLOW			This is a software setting.
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_IO_UNDERFLOW_DEFAULT		AI64SSA_IO_UNDERFLOW_CHECK
#define	AI64SSA_IO_UNDERFLOW_IGNORE			0
#define	AI64SSA_IO_UNDERFLOW_CHECK			1

//*****************************************************************************
// AI64SSA_IOCTL_SAMP_CLK_SRC				SSCR D3-D4
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_SAMP_CLK_SRC_RAG			0
#define	AI64SSA_SAMP_CLK_SRC_RBG			1
#define	AI64SSA_SAMP_CLK_SRC_EXT			2
#define	AI64SSA_SAMP_CLK_SRC_BCR			3

//*****************************************************************************
// AI64SSA_IOCTL_SCAN_MARKER				9080 BCTLR D18, else BCTLR  D11
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	AI64SSA_SCAN_MARKER_DISABLE			0
#define	AI64SSA_SCAN_MARKER_ENABLE			1

//*****************************************************************************
// AI64SSA_IOCTL_SCAN_MARKER_GET			SMUWR D0-D15, SMLWR D0-D15
//
//	Parameter:	u32*
//		The value returned is in the range 0 to 0xFFFFFFFF.

//*****************************************************************************
// AI64SSA_IOCTL_SCAN_MARKER_SET			SMUWR D0-D15, SMLWR D0-D15
//
//	Parameter:	u32*
//		Pass in any value in the range 0 to 0xFFFFFFFF.

//*****************************************************************************
// AI64SSA_IOCTL_WAIT_CANCEL				fields need not be valid
// AI64SSA_IOCTL_WAIT_EVENT					all fields must be valid
// AI64SSA_IOCTL_WAIT_STATUS				fields need not be valid
//
//	Parameter:	gsc_wait_t*
// gsc_wait_t.flags - see gsc_common.h
// gsc_wait_t.main - see gsc_common.h
// gsc_wait_t.gsc
#define	AI64SSA_WAIT_GSC_INIT_DONE			0x0001
#define	AI64SSA_WAIT_GSC_AUTOCAL_DONE		0x0002
#define	AI64SSA_WAIT_GSC_SYNC_START			0x0004
#define	AI64SSA_WAIT_GSC_SYNC_DONE			0x0008
#define	AI64SSA_WAIT_GSC_BURST_START		0x0010
#define	AI64SSA_WAIT_GSC_BURST_DONE			0x0020
#define	AI64SSA_WAIT_GSC_IN_BUF_THR_L2H		0x0040
#define	AI64SSA_WAIT_GSC_IN_BUF_THR_H2L		0x0080
#define	AI64SSA_WAIT_GSC_BUF_ERROR			0x0100
#define	AI64SSA_WAIT_GSC_ALL				0x01FF
// gsc_wait_t.alt flags
#define	AI64SSA_WAIT_ALT_ALL				0x0000
// gsc_wait_t.io
#define	AI64SSA_WAIT_IO_RX_ABORT			0x0001
#define	AI64SSA_WAIT_IO_RX_DONE				0x0002
#define	AI64SSA_WAIT_IO_RX_ERROR			0x0004
#define	AI64SSA_WAIT_IO_RX_TIMEOUT			0x0008

#define	AI64SSA_WAIT_IO_ALL					0x000F

// Retained for backwards compatibility:
#define	AI64SSA_WAIT_GSC_AUTO_CAL_DONE		AI64SSA_WAIT_GSC_AUTOCAL_DONE

//*****************************************************************************
// AI64SSA_IOCTL_LOW_LAT_HOLD_CHAN			LLCR D0-D6
// AI64SSA_IOCTL_LOW_LAT_REL_CHAN			LLCR D7-D11
//
//	Parameter:	u32*
//		Pass in any value in the range 0 to 0xFC for 64 channel devices and
//		any value in the range 0 to 0x7C for 32 channel devices.



#endif
