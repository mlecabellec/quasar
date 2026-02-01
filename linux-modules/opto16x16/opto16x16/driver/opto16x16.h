// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/opto16x16.h $
// $Rev: 51408 $
// $Date: 2022-07-14 14:22:37 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Device Driver: header file

#ifndef	__OPTO16X16_H__
#define	__OPTO16X16_H__

#include "gsc_common.h"
#include "gsc_pci9056.h"



// macros *********************************************************************

#define	OPTO16X16_BASE_NAME					"opto16x16"
#define	OPTO16X16_BASE_NAME_LC				OPTO16X16_BASE_NAME
#define	OPTO16X16_BASE_NAME_UC				"OPTO16X16"

// IOCTL command codes
#define	OPTO16X16_IOCTL_REG_READ			OS_IOCTL_RW( 0, 12, gsc_reg_t)
#define	OPTO16X16_IOCTL_REG_WRITE			OS_IOCTL_W ( 1, 12, gsc_reg_t)
#define	OPTO16X16_IOCTL_REG_MOD				OS_IOCTL_W ( 2, 12, gsc_reg_t)
#define	OPTO16X16_IOCTL_QUERY				OS_IOCTL_RW( 3,  4, s32)
#define	OPTO16X16_IOCTL_INITIALIZE			OS_IOCTL   ( 4)
#define	OPTO16X16_IOCTL_CLOCK_DIVIDER		OS_IOCTL_RW( 5,  4, s32)
#define	OPTO16X16_IOCTL_COS_POLARITY		OS_IOCTL_RW( 6,  4, s32)
#define	OPTO16X16_IOCTL_DEBOUNCE_MS			OS_IOCTL_RW( 7,  4, s32)
#define	OPTO16X16_IOCTL_DEBOUNCE_US			OS_IOCTL_RW( 8,  4, s32)
#define	OPTO16X16_IOCTL_IRQ_ENABLE			OS_IOCTL_RW( 9,  4, s32)
#define	OPTO16X16_IOCTL_LED					OS_IOCTL_RW(10,  4, s32)
#define	OPTO16X16_IOCTL_RX_DATA				OS_IOCTL_R (11,  4, s32)
#define	OPTO16X16_IOCTL_RX_EVENT_COUNTER	OS_IOCTL_RW(12,  4, s32)
#define	OPTO16X16_IOCTL_TX_DATA				OS_IOCTL_RW(13,  4, s32)
#define	OPTO16X16_IOCTL_WAIT_EVENT			OS_IOCTL_RW(14, 28, gsc_wait_t)
#define	OPTO16X16_IOCTL_WAIT_CANCEL			OS_IOCTL_RW(15, 28, gsc_wait_t)
#define	OPTO16X16_IOCTL_WAIT_STATUS			OS_IOCTL_RW(16, 28, gsc_wait_t)

//*****************************************************************************
// OPTO16X16_IOCTL_REG_READ
// OPTO16X16_IOCTL_REG_WRITE
// OPTO16X16_IOCTL_REG_MOD
//
#define	OPTO16X16_REG_ENCODE(s,o)			GSC_REG_ENCODE(GSC_REG_TYPE_BAR2,(s),(o))
// Parameter:	gsc_reg_t*
#define	OPTO16X16_GSC_BCSR					OPTO16X16_REG_ENCODE(1,0x00)// Board Control/Status Register
#define	OPTO16X16_GSC_RDR					OPTO16X16_REG_ENCODE(2,0x04)// Receive Data Register
#define	OPTO16X16_GSC_COSR					OPTO16X16_REG_ENCODE(2,0x08)// Change of State Register
#define	OPTO16X16_GSC_RECR					OPTO16X16_REG_ENCODE(2,0x0C)// Receive Event Counter Register
#define	OPTO16X16_GSC_CIER					OPTO16X16_REG_ENCODE(2,0x10)// COS Interrupt Enable Register
#define	OPTO16X16_GSC_CPR					OPTO16X16_REG_ENCODE(2,0x14)// COS Polarity Register
#define	OPTO16X16_GSC_CDR					OPTO16X16_REG_ENCODE(4,0x18)// Clock Division Register
#define	OPTO16X16_GSC_ODR					OPTO16X16_REG_ENCODE(2,0x1C)// Output Data Register

//*****************************************************************************
// OPTO16X16_IOCTL_QUERY
//
//	Parameter:	s32
//		Pass in a value from the list below.
//		The value returned is the answer to the query.

typedef enum
{
	OPTO16X16_QUERY_COUNT,			//				How many query options are supported?
	OPTO16X16_QUERY_DEVICE_TYPE,	// PCI ID regs	Value from gsc_dev_type_t

	OPTO16X16_IOCTL_QUERY_LAST

} opto16x16_query_t;

#define	OPTO16X16_IOCTL_QUERY_ERROR			(-1)

//*****************************************************************************
// OPTO16X16_IOCTL_INITIALIZE				manual operation
//
//	Parameter:	None

//*****************************************************************************
// OPTO16X16_IOCTL_CLOCK_DIVIDER			CDR D0-D24
//
//	Parameter:	s32*
//		Pass in any value in the range from 0 to 0xFFFFFF, or
//		-1 to read the current setting.

//*****************************************************************************
// OPTO16X16_IOCTL_COS_POLARITY				CPR D0-D15
//
//	Parameter:	s32*
//		Pass in any combination of the below options, or
//		-1 to read the current setting.
#define	OPTO16X16_COS_00					0x00000001
#define	OPTO16X16_COS_01					0x00000002
#define	OPTO16X16_COS_02					0x00000004
#define	OPTO16X16_COS_03					0x00000008
#define	OPTO16X16_COS_04					0x00000010
#define	OPTO16X16_COS_05					0x00000020
#define	OPTO16X16_COS_06					0x00000040
#define	OPTO16X16_COS_07					0x00000080
#define	OPTO16X16_COS_08					0x00000100
#define	OPTO16X16_COS_09					0x00000200
#define	OPTO16X16_COS_10					0x00000400
#define	OPTO16X16_COS_11					0x00000800
#define	OPTO16X16_COS_12					0x00001000
#define	OPTO16X16_COS_13					0x00002000
#define	OPTO16X16_COS_14					0x00004000
#define	OPTO16X16_COS_15					0x00008000

//*****************************************************************************
// OPTO16X16_IOCTL_DEBOUNCE_MS				CDR D0-D24
//
//	Parameter:	s32*
//		Pass in any value in the range from 0 to 4,000, or
//		-1 to read the current setting.
#define	OPTO16X16_DEBOUNCE_MS_MAX			2000

//*****************************************************************************
// OPTO16X16_IOCTL_DEBOUNCE_US				CDR D0-D24
//
//	Parameter:	s32*
//		Pass in any value in the range from 0 to 4,000,000, or
//		-1 to read the current setting.
#define	OPTO16X16_DEBOUNCE_US_MAX			2000000L

//*****************************************************************************
// OPTO16X16_IOCTL_IRQ_ENABLE				CIER D0-D15, BCR D6, BSR D6
//
//	Parameter:	s32*
//		Pass in any combination of the below options, or
//		-1 to read the current setting. The value -1 cal be passed in for the
//		SEL service to determine which options are selected.
#define	OPTO16X16_IRQ_COS_00				0x00000001
#define	OPTO16X16_IRQ_COS_01				0x00000002
#define	OPTO16X16_IRQ_COS_02				0x00000004
#define	OPTO16X16_IRQ_COS_03				0x00000008
#define	OPTO16X16_IRQ_COS_04				0x00000010
#define	OPTO16X16_IRQ_COS_05				0x00000020
#define	OPTO16X16_IRQ_COS_06				0x00000040
#define	OPTO16X16_IRQ_COS_07				0x00000080
#define	OPTO16X16_IRQ_COS_08				0x00000100
#define	OPTO16X16_IRQ_COS_09				0x00000200
#define	OPTO16X16_IRQ_COS_10				0x00000400
#define	OPTO16X16_IRQ_COS_11				0x00000800
#define	OPTO16X16_IRQ_COS_12				0x00001000
#define	OPTO16X16_IRQ_COS_13				0x00002000
#define	OPTO16X16_IRQ_COS_14				0x00004000
#define	OPTO16X16_IRQ_COS_15				0x00008000
#define	OPTO16X16_IRQ_EVENT_COUNT			0x00010000

//*****************************************************************************
// OPTO16X16_IOCTL_LED						BCR/BSR D7
//
//	Parameter:	s32*
//		Pass in any of the below options, or
//		-1 to read the current setting.
#define	OPTO16X16_LED_OFF					1
#define	OPTO16X16_LED_ON					0

//*****************************************************************************
// OPTO16X16_IOCTL_RX_DATA					RDR D0-D15
//
//	Parameter:	s32*
//		The returned value is from 0x0000 to 0xFFFF.

//*****************************************************************************
// OPTO16X16_IOCTL_RX_EVENT_COUNTER			RECR D0-D15
//
//	Parameter:	s32*
//		Pass in any value in the range from 0 to 0xFFFF, or
//		-1 to read the current setting.

//*****************************************************************************
// OPTO16X16_IOCTL_TX_DATA					ODR D0-D15
//
//	Parameter:	s32*
//		Pass in any value in the range from 0 to 0xFFFF, or
//		-1 to read the current setting.

//*****************************************************************************
// OPTO16X16_IOCTL_WAIT_EVENT		all fields must be valid
// OPTO16X16_IOCTL_WAIT_CANCEL		fields need not be valid
// OPTO16X16_IOCTL_WAIT_STATUS		fields need not be valid
//
//	Parameter:	gsc_wait_t*
// gsc_wait_t.flags - see gsc_common.h
// gsc_wait_t.main - see gsc_common.h
// gsc_wait_t.gsc
#define	OPTO16X16_WAIT_GSC_COS_00			OPTO16X16_IRQ_COS_00
#define	OPTO16X16_WAIT_GSC_COS_01			OPTO16X16_IRQ_COS_01
#define	OPTO16X16_WAIT_GSC_COS_02			OPTO16X16_IRQ_COS_02
#define	OPTO16X16_WAIT_GSC_COS_03			OPTO16X16_IRQ_COS_03
#define	OPTO16X16_WAIT_GSC_COS_04			OPTO16X16_IRQ_COS_04
#define	OPTO16X16_WAIT_GSC_COS_05			OPTO16X16_IRQ_COS_05
#define	OPTO16X16_WAIT_GSC_COS_06			OPTO16X16_IRQ_COS_06
#define	OPTO16X16_WAIT_GSC_COS_07			OPTO16X16_IRQ_COS_07
#define	OPTO16X16_WAIT_GSC_COS_08			OPTO16X16_IRQ_COS_08
#define	OPTO16X16_WAIT_GSC_COS_09			OPTO16X16_IRQ_COS_09
#define	OPTO16X16_WAIT_GSC_COS_10			OPTO16X16_IRQ_COS_10
#define	OPTO16X16_WAIT_GSC_COS_11			OPTO16X16_IRQ_COS_11
#define	OPTO16X16_WAIT_GSC_COS_12			OPTO16X16_IRQ_COS_12
#define	OPTO16X16_WAIT_GSC_COS_13			OPTO16X16_IRQ_COS_13
#define	OPTO16X16_WAIT_GSC_COS_14			OPTO16X16_IRQ_COS_14
#define	OPTO16X16_WAIT_GSC_COS_15			OPTO16X16_IRQ_COS_15
#define	OPTO16X16_WAIT_GSC_EVENT_COUNT		OPTO16X16_IRQ_EVENT_COUNT
#define	OPTO16X16_WAIT_GSC_ALL				0x1FFFF
// gsc_wait_t.alt flags
#define	OPTO16X16_WAIT_ALT_ALL				0
// gsc_wait_t.io - see gsc_common.h
#define	OPTO16X16_WAIT_IO_ALL				0

#endif
