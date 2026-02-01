// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/ioctl.c $
// $Rev: 51408 $
// $Date: 2022-07-14 14:22:37 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Device Driver: source file

#include "main.h"



//*****************************************************************************
static int _query(dev_data_t* dev, s32* arg)
{
	switch (arg[0])
	{
		default:							arg[0]	= OPTO16X16_IOCTL_QUERY_ERROR;	break;
		case OPTO16X16_QUERY_COUNT:			arg[0]	= OPTO16X16_IOCTL_QUERY_LAST;	break;
		case OPTO16X16_QUERY_DEVICE_TYPE:	arg[0]	= dev->board_type;				break;
	}

	return(0);
}



//*****************************************************************************
int initialize_ioctl(dev_data_t* dev, void* arg)
{
	int	ret	= 0;

	os_reg_mem_tx_u8(dev, dev->vaddr.gsc_bcsr_8, 0x0B);		// Disable all interrupts.
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cier_16, 0);		// Disable all interrupts.
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cosr_16, 0xFFFF);	// Clear all interrupts.
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_recr_16, 0);
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cpr_16, 0);
	os_reg_mem_tx_u32(dev, dev->vaddr.gsc_cdr_32, 0);
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_odr_16, 0);
	return(ret);
}



//*****************************************************************************
static int _clock_divider(dev_data_t* dev, s32* arg)
{
	u8	bcsr;
	u16	cier;
	int	i;
	int	ret		= 0;

	if (arg[0] == -1)
	{
		ret	= gsc_s32_range_reg(dev, arg, 0, 0xFFFFFF, dev->vaddr.gsc_cdr_32, 23, 0);
	}
	else if ((arg[0] & 0xFFFFFF) == arg[0])
	{
		// Gain exclusive access to interrupts.
		i	= gsc_irq_local_disable(dev);

		// Disable interrupts.
		bcsr	= os_reg_mem_mx_u8(dev, dev->vaddr.gsc_bcsr_8, 0, 0x7F);
		cier	= os_reg_mem_mx_u16(dev, dev->vaddr.gsc_cier_16, 0, 0xFFFF);

		// Change the divider setting.
		os_reg_mem_mx_u32(dev, dev->vaddr.gsc_cdr_32, arg[0], 0xFFFFFF);

		// Restore interrupt selections.
		os_reg_mem_tx_u8(dev, dev->vaddr.gsc_bcsr_8, bcsr & 0xC0);
		os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cier_16, cier);

		// Release exclusive access to interrupts.

		if (i == 0)
			gsc_irq_local_enable(dev);
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
static int _cos_polarity(dev_data_t* dev, s32* arg)
{
	int	ret;

	ret	= gsc_s32_range_reg(dev, arg, 0, 0xFFFF, dev->vaddr.gsc_cpr_16, 15, 0);
	return(ret);
}



//*****************************************************************************
static int _debounce_ms(dev_data_t* dev, s32* arg)
{
	int	ret;
	s32	v;

	if (arg[0] == -1)
	{
		ret	= gsc_s32_range_reg(dev, arg, 0, 0xFFFFFF, dev->vaddr.gsc_cdr_32, 23, 0);

		if (ret == 0)
		{
			arg[0]	*= 2;
			arg[0]	+= 2;
			arg[0]	*= 50;
			arg[0]	*= 3;
			arg[0]	+= 500000;
			arg[0]	/= 1000000L;
		}
	}
	else if ((arg[0] < 0) || (arg[0] > OPTO16X16_DEBOUNCE_MS_MAX))
	{
		ret	= -EINVAL;
	}
	else
	{
		// ((((arg[0] * 1,000,000) / 3) / 50) - 2) / 2
		v	= arg[0];
		v	*= 1000000L;
		v	/= 3;
		v	/= 50;
		v	-= 2;
		v	/= 2;
		v	= (v < 0) ? 0 : v;
		ret	= gsc_s32_range_reg(dev, &v, 0, 0xFFFFFF, dev->vaddr.gsc_cdr_32, 23, 0);
	}

	return(ret);
}



//*****************************************************************************
static int _debounce_us(dev_data_t* dev, s32* arg)
{
	int	ret;
	s32	v;

	if (arg[0] == -1)
	{
		ret		= gsc_s32_range_reg(dev, arg, 0, 0xFFFFFF, dev->vaddr.gsc_cdr_32, 23, 0);

		if (ret == 0)
		{
			arg[0]	*= 2;
			arg[0]	+= 2;
			arg[0]	*= 50;
			arg[0]	*= 3;
			arg[0]	+= 500;
			arg[0]	/= 1000L;
		}
	}
	else if ((arg[0] < 0) || (arg[0] > OPTO16X16_DEBOUNCE_US_MAX))
	{
		ret	= -EINVAL;
	}
	else
	{
		// ((((arg[0] * 1,000) / 3) / 50) - 2) / 2
		v	= arg[0];
		v	*= 1000L;
		v	/= 3;
		v	/= 50;
		v	-= 2;
		v	/= 2;
		v	= (v < 0) ? 0 : v;
		ret	= gsc_s32_range_reg(dev, &v, 0, 0xFFFFFF, dev->vaddr.gsc_cdr_32, 23, 0);
	}

	return(ret);
}



//*****************************************************************************
static int _irq_enable(dev_data_t* dev, s32* arg)
{
	u32	bcsr;
	u32	cier;
	int	i;
	int	ret	= 0;

	i	= gsc_irq_local_disable(dev);

	if (arg[0] == -1)
	{
		// Retrieve the current setting.
		cier	= os_reg_mem_rx_u16(dev, dev->vaddr.gsc_cier_16);
		bcsr	= os_reg_mem_rx_u8(dev, dev->vaddr.gsc_bcsr_8);
		arg[0]	= (cier & 0xFFFF) | ((bcsr & 0x40) ? 0x10000 : 0x00000);
	}
	else if ((arg[0] & 0x1FFFF) == arg[0])
	{
		// Apply/retrieve the setting.
		cier	= arg[0] & 0xFFFF;
		os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cier_16, cier);

		bcsr	= (arg[0] & 0x10000) ? 0x40 : 0x00;
		os_reg_mem_mx_u8(dev, dev->vaddr.gsc_bcsr_8, bcsr, 0x7F);
	}
	else
	{
		ret	= -EINVAL;
	}

	if (i == 0)
		i	= gsc_irq_local_enable(dev);

	ret	= ret ? ret : i;
	return(ret);
}



//*****************************************************************************
static int _led(dev_data_t* dev, s32* arg)
{
	u8	bcsr;
	int	ret	= 0;

	if (arg[0] == -1)
	{
		bcsr	= os_reg_mem_rx_u8(dev, dev->vaddr.gsc_bcsr_8);
		arg[0]	= (bcsr & 0x80) >> 7;
	}
	else if ((arg[0] == OPTO16X16_LED_ON) || (arg[0] == OPTO16X16_LED_OFF))
	{
		// Apply the new setting.
		bcsr	= arg[0] << 7;
		os_reg_mem_mx_u8(dev, dev->vaddr.gsc_bcsr_8, bcsr, 0xBF);
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
static int _rx_data(dev_data_t* dev, s32* arg)
{
	arg[0]	= os_reg_mem_rx_u16(dev, dev->vaddr.gsc_rdr_16) & 0xFFFF;
	return(0);
}



//*****************************************************************************
static int _rx_event_counter(dev_data_t* dev, s32* arg)
{
	int	ret;

	ret		= gsc_s32_range_reg(dev, arg, 0, 0xFFFF, dev->vaddr.gsc_recr_16, 15, 0);
	return(ret);
}



//*****************************************************************************
static int _tx_data(dev_data_t* dev, s32* arg)
{
	int	ret;

	ret	= gsc_s32_range_reg(dev, arg, 0, 0xFFFF, dev->vaddr.gsc_odr_16, 15, 0);
	return(ret);
}



// variables ******************************************************************

const gsc_ioctl_t	dev_ioctl_list[]	=
{
	{ OPTO16X16_IOCTL_REG_READ,			(void*) gsc_reg_read_ioctl		},
	{ OPTO16X16_IOCTL_REG_WRITE,		(void*) gsc_reg_write_ioctl		},
	{ OPTO16X16_IOCTL_REG_MOD,			(void*) gsc_reg_mod_ioctl		},
	{ OPTO16X16_IOCTL_QUERY,			(void*) _query					},
	{ OPTO16X16_IOCTL_INITIALIZE,		(void*) initialize_ioctl		},
	{ OPTO16X16_IOCTL_CLOCK_DIVIDER,	(void*) _clock_divider			},
	{ OPTO16X16_IOCTL_COS_POLARITY,		(void*) _cos_polarity			},
	{ OPTO16X16_IOCTL_DEBOUNCE_MS,		(void*) _debounce_ms			},
	{ OPTO16X16_IOCTL_DEBOUNCE_US,		(void*) _debounce_us			},
	{ OPTO16X16_IOCTL_IRQ_ENABLE,		(void*) _irq_enable				},
	{ OPTO16X16_IOCTL_LED,				(void*) _led					},
	{ OPTO16X16_IOCTL_RX_DATA,			(void*) _rx_data				},
	{ OPTO16X16_IOCTL_RX_EVENT_COUNTER,	(void*) _rx_event_counter		},
	{ OPTO16X16_IOCTL_TX_DATA,			(void*) _tx_data				},
	{ OPTO16X16_IOCTL_WAIT_EVENT,		(void*) gsc_wait_event_ioctl	},
	{ OPTO16X16_IOCTL_WAIT_CANCEL,		(void*) gsc_wait_cancel_ioctl	},
	{ OPTO16X16_IOCTL_WAIT_STATUS,		(void*) gsc_wait_status_ioctl	},
	{ -1, NULL }
};


