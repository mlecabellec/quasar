// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/ioctl.c $
// $Rev: 54971 $
// $Date: 2024-08-07 15:59:52 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



// macros *********************************************************************

#define	AUTOCAL_START			D13
#define	AUTOCAL_PASS			D14
#define	INIT_START				D15

#define	ICR_IRQ0_INIT_DONE		0x0000
#define	ICR_IRQ0_AUTOCAL_DONE	0x0001
#define	ICR_IRQ0_MASK			0x0007
#define	ICR_IRQ0_ACTIVE			0x0008
#define	ICR_IRQ1_ACTIVE			0x0080



//*****************************************************************************
static int _query(dev_data_t* dev, s32* arg)
{
	switch (arg[0])
	{
		default:							arg[0]	= AI64SSA_IOCTL_QUERY_ERROR;	break;
		case AI64SSA_QUERY_AUTOCAL_MS:		arg[0]	= dev->cache.autocal_ms;		break;
		case AI64SSA_QUERY_CHAN_RANGE:		arg[0]	= dev->cache.channel_range;		break;
		case AI64SSA_QUERY_CHANNEL_MAX:		arg[0]	= dev->cache.channels_max;		break;
		case AI64SSA_QUERY_CHANNEL_QTY:		arg[0]	= dev->cache.channel_qty;		break;
		case AI64SSA_QUERY_COUNT:			arg[0]	= AI64SSA_IOCTL_QUERY_LAST + 1;	break;
		case AI64SSA_QUERY_DATA_PACKING:	arg[0]	= dev->cache.data_packing;		break;
		case AI64SSA_QUERY_DEVICE_TYPE:		arg[0]	= dev->board_type;				break;
		case AI64SSA_QUERY_FIFO_SIZE:		arg[0]	= dev->cache.fifo_size;			break;
		case AI64SSA_QUERY_FSAMP_MAX:		arg[0]	= dev->cache.fsamp_max;			break;
		case AI64SSA_QUERY_FSAMP_MIN:		arg[0]	= dev->cache.fsamp_min;			break;
		case AI64SSA_QUERY_INIT_MS:			arg[0]	= dev->cache.initialize_ms;		break;
		case AI64SSA_QUERY_IRQ1_BUF_ERROR:	arg[0]	= dev->cache.irq1_buf_error;	break;
		case AI64SSA_QUERY_LOW_LATENCY:		arg[0]	= dev->cache.low_latency;		break;
		case AI64SSA_QUERY_MASTER_CLOCK:	arg[0]	= dev->cache.master_clock;		break;
		case AI64SSA_QUERY_NRATE_MAX:		arg[0]	= dev->cache.nrate_max;			break;
		case AI64SSA_QUERY_NRATE_MIN:		arg[0]	= dev->cache.nrate_min;			break;
		case AI64SSA_QUERY_RATE_GEN_QTY:	arg[0]	= dev->cache.rate_gen_qty;		break;
		case AI64SSA_QUERY_REG_ACAR:		arg[0]	= dev->cache.reg_acar;			break;
		case AI64SSA_QUERY_REG_LLCR:		arg[0]	= dev->cache.reg_llcr;			break;
	}

	return(0);
}



//*****************************************************************************
static int _init_start(dev_data_t* dev, void* arg)
{
	// Initiate initialization.
	os_reg_mem_tx_u32(NULL, dev->vaddr.gsc_bctlr_32, INIT_START);
	return(0);
}



//*****************************************************************************
int initialize_ioctl(dev_data_t* dev, void* arg)
{
	int			i;
	int			mask;
	long		ms			= dev->cache.initialize_ms + 5000;
	long		ms_total	= ms;
	u32			reg;
	int			ret			= 0;
	os_sem_t	sem;
	int			tmp;
	VADDR_T		va			= dev->vaddr.gsc_icr_32;
	int			value;
	gsc_wait_t	wt;

	if ((dev->irq.opened) && (gsc_global.driver_unloading == 0))
	{
		ms_total	*= 2;

		// Safely select the Initialize Done interrupt.
		mask	= ICR_IRQ0_MASK | ICR_IRQ0_ACTIVE | ICR_IRQ1_ACTIVE;
		value	= ICR_IRQ0_INIT_DONE | ICR_IRQ1_ACTIVE;
		os_reg_mem_mx_u32(dev, va, value, mask);

		// Wait for the local interrupt.
		os_sem_create(&sem);	// dummy, required for wait operations.
		memset(&wt, 0, sizeof(wt));
		wt.flags		= GSC_WAIT_FLAG_INTERNAL;
		wt.gsc			= AI64SSA_WAIT_GSC_INIT_DONE;
		wt.timeout_ms	= ms;
		ret				= gsc_wait_event(dev, &wt, _init_start, NULL, &sem);
		os_sem_destroy(&sem);

		if (wt.flags & GSC_WAIT_FLAG_TIMEOUT)
		{
			ret	= ret ? ret : -ETIMEDOUT;
			printf(	"%s: INITILIZE DONE IRQ TIMED OUT AFTER %ld ms.\n",
					dev->model,
					ms);
		}
	}
	else
	{
		_init_start(dev, NULL);
	}

	// Manually wait for completion as the IRQ wait may have ended early.
	va	= dev->vaddr.gsc_bctlr_32;
	tmp	= gsc_poll_u32(dev, ms, va, INIT_START, 0);

	if (tmp)
	{
		ret	= ret ? ret : tmp;
		printf(	"%s: INITIALIZATION DID NOT COMPLETE WITHIN %ld ms.\n",
				dev->model,
				ms_total);
	}

	// Manually check for completion as the IRQ wait may have ended early.
	reg	= os_reg_mem_rx_u32(dev, va);

	if (reg & INIT_START)
	{
		ret	= ret ? ret : -ETIMEDOUT;
		printf(	"%s: INITILIZE STILL ACTIVE AFTER %ld ms.\n",
				dev->model,
				ms_total);
	}

	// Initialize the software settings.

	for (i = 0; i < DEV_IO_STREAM_QTY; i++)
	{
		if (dev->io.io_streams[i])
		{
			if (dev->io.io_streams[i]->dev_io_sw_init)
			{
				(dev->io.io_streams[i]->dev_io_sw_init)(dev, dev->io.io_streams[i]);
			}
		}
	}

	return(ret);
}



//*****************************************************************************
static int _autocal_start(dev_data_t* dev, void* arg)
{
	// Initiate autocalibration.
	os_reg_mem_mx_u32(NULL, dev->vaddr.gsc_bctlr_32, AUTOCAL_START, AUTOCAL_START);
	return(0);
}



//*****************************************************************************
static int _autocal(dev_data_t* dev, void* arg)
{
	u32			mask;
	long		ms		= dev->cache.autocal_ms + 5000;
	u32			reg;
	int			ret;
	os_sem_t	sem;
	int			tmp;
	VADDR_T		va		= dev->vaddr.gsc_icr_32;
	u32			value;
	gsc_wait_t	wt;

	// Safely select the Autocalibration Done interrupt.
	mask	= ICR_IRQ0_MASK | ICR_IRQ0_ACTIVE | ICR_IRQ1_ACTIVE;
	value	= ICR_IRQ0_AUTOCAL_DONE | ICR_IRQ1_ACTIVE;
	os_reg_mem_mx_u32(dev, va, value, mask);

	// Wait for the local interrupt.
	os_sem_create(&sem);	// dummy, required for wait operations.
	memset(&wt, 0, sizeof(wt));
	wt.flags		= GSC_WAIT_FLAG_INTERNAL;
	wt.gsc			= AI64SSA_WAIT_GSC_AUTOCAL_DONE;
	wt.timeout_ms	= ms;
	ret				= gsc_wait_event(dev, &wt, _autocal_start, NULL, &sem);
	os_sem_destroy(&sem);

	if (wt.flags & GSC_WAIT_FLAG_TIMEOUT)
	{
		ret	= ret ? ret : -ETIMEDOUT;
		printf(	"%s: AUTOCALIBRATE DONE IRQ TIMED OUT AFTER %ld ms.\n",
				dev->model,
				ms);
	}

	// Manually wait for completion in case something terminates our wait early.
	va	= dev->vaddr.gsc_bctlr_32;
	tmp	= gsc_poll_u32(dev, ms, va, AUTOCAL_START, 0);

	if (tmp)
	{
		ms	*= 2;
		ret	= ret ? ret : tmp;
		printf(	"%s: AUTOCALIBRATION DID NOT COMPLETE WITHIN %ld ms.\n",
				dev->model,
				ms);
	}

	// Manually check for completion as the IRQ wait may have ended early.
	reg	= os_reg_mem_rx_u32(dev, va);

	if (reg & AUTOCAL_START)
	{
		ret	= ret ? ret : -ETIMEDOUT;
		printf(	"%s: AUTOCALIBRATION STILL ACTIVE AFTER %ld ms.\n",
				dev->model,
				ms);
	}

	// Final results.
	reg	= os_reg_mem_rx_u32(dev, va);

	if ((reg & AUTOCAL_PASS) == 0)
	{
		ret	= ret ? ret : -EIO;
		printf(	"%s: AUTOCALIBRATION FAILED (%ld ms).\n",
				dev->model,
				ms);
	}

	return(ret);
}



//*****************************************************************************
static int _autocal_status(dev_data_t* dev, s32* arg)
{
	u32	reg;

	reg	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);

	if (reg & AUTOCAL_START)
		arg[0]	= AI64SSA_AUTOCAL_STATUS_ACTIVE;
	else if (reg & AUTOCAL_PASS)
		arg[0]	= AI64SSA_AUTOCAL_STATUS_PASS;
	else
		arg[0]	= AI64SSA_AUTOCAL_STATUS_FAIL;

	return(0);
}



//*****************************************************************************
static int _ai_buf_clear(dev_data_t* dev, void* arg)
{
	#define	CLEAR	(dev->cache.pci9080 ? D16 : D18)	// IBCR
	#define	OVER	D17		// BCTLR
	#define	UNDER	D16		// BCTLR

	int		i;
	u32		reg;
	int		ret	= 0;
	VADDR_T	va		= dev->vaddr.gsc_ibcr_32;

	// Clear the buffer.
	os_reg_mem_mx_u32(dev, va, CLEAR, CLEAR);

	// Wait for the bit to clear.

	for (i = 0;; i++)
	{
		reg	= os_reg_mem_rx_u32(dev, va);

		if ((reg & CLEAR) == 0)
			break;

		if (i >= 250)
		{
			ret	= -EINVAL;
			printf(	"%s: The analog input buffer took too long to clear.\n",
					DEV_NAME);
			break;
		}
	}

	va		= dev->vaddr.gsc_bctlr_32;

	// Clear the Overflow status bit.
	os_reg_mem_mx_u32(dev, va, 0, OVER);

	// Clear the Underflow status bit.
	os_reg_mem_mx_u32(dev, va, 0, UNDER);

	return(ret);
}



//*****************************************************************************
static int _ai_buf_level(dev_data_t* dev, s32* arg)
{
	arg[0]	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bufsr_32);

	if (dev->cache.pci9080)
		arg[0]	&= 0x3FFFF;
	else
		arg[0]	&= 0x7FFFF;

	return(0);
}



//*****************************************************************************
static int _ai_buf_overflow(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AI_BUF_OVERFLOW_NO,
		AI64SSA_AI_BUF_OVERFLOW_YES,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 17, 17);
	return(ret);
}



//*****************************************************************************
static int _ai_buf_thr_lvl(dev_data_t* dev, s32* arg)
{
	u32	ibcr;
	int	ret	= 0;

	if (dev->cache.pci9080)
	{
		if (arg[0] == -1)
		{
			ibcr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_ibcr_32);
			arg[0]	= ibcr & 0xFFFF;

			if (ibcr & D19)
				arg[0]	*= 4;
		}
		else if (arg[0] < 0)
		{
			ret	= -EINVAL;
		}
		else if (arg[0] <= 0xFFFF)
		{
			os_reg_mem_mx_u32(
				dev,
				dev->vaddr.gsc_ibcr_32,
				arg[0],
				0xFFFF | D19);
		}
		else if (arg[0] <= 0x3FFFF)
		{
			os_reg_mem_mx_u32(
				dev,
				dev->vaddr.gsc_ibcr_32,
				((arg[0] + 3) / 4) | D19,
				0xFFFF | D19);
		}
		else
		{
			ret	= -EINVAL;
		}
	}
	else
	{
		ret	= gsc_s32_range_reg(dev, arg, 0, 0x3FFFF, dev->vaddr.gsc_ibcr_32, 17, 0);
	}

	return(ret);
}



//*****************************************************************************
static int _ai_buf_thr_sts(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AI_BUF_THR_STS_CLEAR,
		AI64SSA_AI_BUF_THR_STS_SET,
		-1	// terminate list
	};

	int	bit	= dev->cache.pci9080 ? 17 : 19;
	int	ret;

	arg[0]	= -1;
	ret		= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_ibcr_32, bit, bit);
	return(ret);
}



//*****************************************************************************
static int _ai_buf_underflow(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AI_BUF_UNDERFLOW_NO,
		AI64SSA_AI_BUF_UNDERFLOW_YES,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 16, 16);
	return(ret);
}



//*****************************************************************************
static int _ai_mode(dev_data_t* dev, s32* arg)
{
	u32	mask	= 0x307;
	u32	reg;
	int	ret		= 0;
	u32	v;

	if (arg[0] == -1)
	{
		reg	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);
		v	= reg & mask;

		switch (v)
		{
			default:

				ret	= -EINVAL;
				break;

			case AI64SSA_AI_MODE_SINGLE:
			case AI64SSA_AI_MODE_PS_DIFF:
			case AI64SSA_AI_MODE_DIFF:
			case AI64SSA_AI_MODE_ZERO:
			case AI64SSA_AI_MODE_VREF:

				arg[0]	= (s32) v;
				break;
		}
	}
	else
	{
		switch (arg[0])
		{
			default:

				ret	= -EINVAL;
				break;

			case AI64SSA_AI_MODE_SINGLE:
			case AI64SSA_AI_MODE_PS_DIFF:
			case AI64SSA_AI_MODE_DIFF:
			case AI64SSA_AI_MODE_ZERO:
			case AI64SSA_AI_MODE_VREF:

				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, (u32) arg[0], mask);
				break;
		}
	}

	return(ret);
}



//*****************************************************************************
static int _ai_range(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AI_RANGE_2_5V,
		AI64SSA_AI_RANGE_5V,
		AI64SSA_AI_RANGE_10V,
		AI64SSA_AI_RANGE_0_5V,
		AI64SSA_AI_RANGE_0_10V,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 5, 3);
	return(ret);
}



//*****************************************************************************
static int _aux_0_mode(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_MODE_DISABLE,
		AI64SSA_AUX_MODE_INPUT,
		AI64SSA_AUX_MODE_OUTPUT,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 1, 0);
	return(ret);
}



//*****************************************************************************
static int _aux_1_mode(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_MODE_DISABLE,
		AI64SSA_AUX_MODE_INPUT,
		AI64SSA_AUX_MODE_OUTPUT,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 3, 2);
	return(ret);
}



//*****************************************************************************
static int _aux_2_mode(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_MODE_DISABLE,
		AI64SSA_AUX_MODE_INPUT,
		AI64SSA_AUX_MODE_OUTPUT,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 5, 4);
	return(ret);
}



//*****************************************************************************
static int _aux_3_mode(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_MODE_DISABLE,
		AI64SSA_AUX_MODE_INPUT,
		AI64SSA_AUX_MODE_OUTPUT,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 7, 6);
	return(ret);
}



//*****************************************************************************
static int _aux_in_pol(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_IN_POL_LO_2_HI,
		AI64SSA_AUX_IN_POL_HI_2_LO,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 8, 8);
	return(ret);
}



//*****************************************************************************
static int _aux_noise(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_NOISE_LOW,
		AI64SSA_AUX_NOISE_HIGH,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 10, 10);
	return(ret);
}



//*****************************************************************************
static int _aux_out_pol(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_AUX_OUT_POL_HI_PULSE,
		AI64SSA_AUX_OUT_POL_LOW_PULSE,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_asiocr_32, 9, 9);
	return(ret);
}



//*****************************************************************************
static int _burst_size(dev_data_t* dev, void* arg)
{
	int	ret;

	ret	= gsc_s32_range_reg(dev, arg, 0, 0xFFFFF, dev->vaddr.gsc_bursr_32, 19, 0);
	return(ret);
}



//*****************************************************************************
static int _burst_src(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_BURST_SRC_DISABLE,
		AI64SSA_BURST_SRC_RBG,
		AI64SSA_BURST_SRC_EXT,
		AI64SSA_BURST_SRC_BCR,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_sscr_32, 9, 8);
	return(ret);
}



//*****************************************************************************
static int _burst_status(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_BURST_STATUS_IDLE,
		AI64SSA_BURST_STATUS_ACTIVE,
		-1	// terminate list
	};

	int		ret;
	s32*	val	= (s32*) arg;

	val[0]	= -1;
	ret		= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_sscr_32, 7, 7);
	return(ret);
}



//*****************************************************************************
static int _chan_active(dev_data_t* dev, void* arg)
{
	static s32	options[]	=
	{
		AI64SSA_CHAN_ACTIVE_0_63,
		AI64SSA_CHAN_ACTIVE_SINGLE,
		AI64SSA_CHAN_ACTIVE_0_1,
		AI64SSA_CHAN_ACTIVE_0_3,
		AI64SSA_CHAN_ACTIVE_0_7,
		AI64SSA_CHAN_ACTIVE_0_15,
		AI64SSA_CHAN_ACTIVE_0_31,
		AI64SSA_CHAN_ACTIVE_RANGE,
		-1	// terminate list
	};

	int	ret;

	if (dev->cache.channel_range)
		options[7]	= AI64SSA_CHAN_ACTIVE_RANGE;
	else
		options[7]	= AI64SSA_CHAN_ACTIVE_SINGLE;

	if (dev->cache.channel_qty >= 64)
		ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_sscr_32, 2, 0);
	else
		ret	= gsc_s32_list_reg(dev, arg, options + 1, dev->vaddr.gsc_sscr_32, 2, 0);

	return(ret);
}



//*****************************************************************************
static int _chan_first(dev_data_t* dev, s32* arg)
{
	s32	last;
	s32	max		= dev->cache.channel_qty - 1;
	u32	reg;
	int	ret		= 0;

	if (arg[0] == -1)
	{
		ret	= gsc_s32_range_reg(dev, arg, 0, max, dev->vaddr.gsc_acar_32, 7, 0);
	}
	else if ((arg[0] < 0) || (arg[0] > max))
	{
		ret	= -EINVAL;
	}
	else
	{
		reg		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_acar_32);
		last	= GSC_FIELD_DECODE(reg, 15, 8);

		if (last < arg[0])
			ret		= gsc_s32_range_reg(dev, arg, 0, max, dev->vaddr.gsc_acar_32, 15, 8);

		if (ret == 0)
			ret	= gsc_s32_range_reg(dev, arg, 0, max, dev->vaddr.gsc_acar_32, 7, 0);
	}

	return(ret);
}



//*****************************************************************************
static int _chan_last(dev_data_t* dev, s32* arg)
{
	u32	first;
	s32	max		= dev->cache.channel_qty - 1;
	u32	reg;
	int	ret		= 0;

	if (arg[0] == -1)
	{
		ret	= gsc_s32_range_reg(dev, arg, 0, max, dev->vaddr.gsc_acar_32, 15, 8);
	}
	else if ((arg[0] < 0) || (arg[0] > max))
	{
		ret	= -EINVAL;
	}
	else
	{
		reg		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_acar_32);
		first	= GSC_FIELD_DECODE(reg, 7, 0);

		if (first > arg[0])
		{
			first	= arg[0];
			ret		= gsc_s32_range_reg(dev, arg, 0, max, dev->vaddr.gsc_acar_32, 7, 0);
		}

		if (ret == 0)
			ret	= gsc_s32_range_reg(dev, arg, first, max, dev->vaddr.gsc_acar_32, 15, 8);
	}

	return(ret);
}



//*****************************************************************************
static int _chan_single(dev_data_t* dev, void* arg)
{
	u32	last;
	int	ret;

	last	= dev->cache.channel_qty - 1;
	ret		= gsc_s32_range_reg(dev, arg, 0, last, dev->vaddr.gsc_sscr_32, 17, 12);
	return(ret);
}



//*****************************************************************************
static int _data_format(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_DATA_FORMAT_2S_COMP,
		AI64SSA_DATA_FORMAT_OFF_BIN,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 6, 6);
	return(ret);
}



//*****************************************************************************
static int _data_packing(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_DATA_PACKING_DISABLE,
		AI64SSA_DATA_PACKING_ENABLE,
		-1	// terminate list
	};

	int	ret;

	if ((dev->cache.data_packing == 0) && (arg[0] == AI64SSA_DATA_PACKING_ENABLE))
		ret	= -EIO;
	else
		ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 18, 18);

	return(ret);
}



//*****************************************************************************
static int _ext_sync_enable(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_EXT_SYNC_ENABLE_NO,
		AI64SSA_EXT_SYNC_ENABLE_YES,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 7, 7);
	return(ret);
}



//*****************************************************************************
static int _input_sync(dev_data_t* dev, void* arg)
{
	unsigned long	ms_limit;
	int				ret;

	os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, D12, D12);

	// Wait for the operation to complete.

	if (dev->io.rx.timeout_s)
		ms_limit	= dev->io.rx.timeout_s * 1000;
	else
		ms_limit	= 1000;

	ret	= gsc_poll_u32(dev, ms_limit, dev->vaddr.gsc_bctlr_32, D12, 0);
	return(ret);
}



//*****************************************************************************
static int _irq0_sel(dev_data_t* dev, s32* arg)
{
	u32	reg;
	int	ret	= 0;
	u32	val;

	if (arg[0] == -1)
	{
		// Retrieve the current setting.
		reg		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_icr_32);
		arg[0]	= GSC_FIELD_DECODE(reg, 2, 0);
	}
	else
	{
		// Validate the option value passed in.

		switch (arg[0])
		{
			default:

				ret	= -EINVAL;
				break;

			case AI64SSA_IRQ0_INIT_DONE:
			case AI64SSA_IRQ0_AUTOCAL_DONE:
			case AI64SSA_IRQ0_SYNC_START:
			case AI64SSA_IRQ0_SYNC_DONE:
			case AI64SSA_IRQ0_BURST_START:
			case AI64SSA_IRQ0_BURST_DONE:

				// Clear the IRQ0 status bit and apply the new setting.
				val	= dev->cache.icr_anomaly ? (arg[0]) : (arg[0] | 0x88);
				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_icr_32, val, 0x8F);
				break;
		}
	}

	return(ret);
}



//*****************************************************************************
static int _irq1_sel(dev_data_t* dev, s32* arg)
{
	u32	reg;
	int	ret	= 0;
	u32	val;

	if (arg[0] == -1)
	{
		// Retrieve the current setting.
		reg		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_icr_32);
		arg[0]	= GSC_FIELD_DECODE(reg, 6, 4);
	}
	else
	{
		// Validate the option value passed in.

		switch (arg[0])
		{
			default:

				ret	= -EINVAL;
				break;

			case AI64SSA_IRQ1_NONE:
			case AI64SSA_IRQ1_IN_BUF_THR_L2H:
			case AI64SSA_IRQ1_IN_BUF_THR_H2L:
			case AI64SSA_IRQ1_BUF_ERROR:

				val	= dev->cache.icr_anomaly ? (arg[0] << 4) : ((arg[0] << 4) | 0x88);
				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_icr_32, val, 0xF8);
				break;
		}
	}

	return(ret);
}



//*****************************************************************************
static int _low_lat_doh(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_LOW_LAT_DOH_NO,
		AI64SSA_LOW_LAT_DOH_YES,
		-1	// terminate list
	};

	int	ret;

	if (dev->cache.low_latency == 0)
	{
		ret		= 0;
		arg[0]	= AI64SSA_LOW_LAT_DOH_NO;
	}
	else
	{
		arg[0]	= -1;
		ret		= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_bctlr_32, 10, 10);
	}

	return(ret);
}



//*****************************************************************************
static int _low_lat_read(dev_data_t* dev, ai64ssa_ll_t* arg)
{
	#define	LL_REG(i)	(((unsigned long) dev->vaddr.gsc_llhr00_16) + (4 * (i)))

	u32			bcr;
	int			begin;
	int			diff;
	int			end;
	unsigned	hold;
	int			i;
	u32			llcr;
	u32			reg;
	int			release;
	int			ret			= 0;

	for (;;)	// A convenience loop.
	{
		if (dev->cache.low_latency == 0)
		{
			// The Low Latency feature is unavailable.
			ret	= -EINVAL;
			break;
		}

		if (dev->cache.reg_llcr)
		{
			// Use the programmed HOLD and RELEASE channel numbers.
			llcr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_llcr_32);
			hold	= GSC_FIELD_DECODE(llcr, 5, 0);
			release	= GSC_FIELD_DECODE(llcr, 11, 6);
		}
		else
		{
			// Use the default HOLD and RELEASE channel numbers.
			hold	= 0;
			release	= dev->cache.channel_qty - 1;
		}

		// Initialize everything to zero.
		memset(arg, 0, sizeof(arg[0]));

		if ((hold >= dev->cache.channel_qty)	||
			(release >= dev->cache.channel_qty))
		{
			// Per the documentation, we do nothing if
			// HOLD or // RELEASE is invalid.
			break;
		}

		if (hold == release)
		{
			// Read the HOLD register only!
			begin	= 1;
			end		= 0;
		}
		else if (hold < release)
		{
			// Ignore channels outside the range HOLD to RELEASE.
			begin	= hold + 1;
			end		= release - 1;
		}
		else
		{
			// Don't ignore any range.
			begin	= 0;
			end		= dev->cache.channel_qty - 1;
		}

		// If using Full Differential, then ignore the odd numbered channels.
		bcr		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_ibcr_32);
		diff	= GSC_FIELD_DECODE(bcr, 9, 8);
		diff	= (diff == 2) ? 1 : 0;

		// Read the HOLD channel.
		reg				= os_reg_mem_rx_u32(dev, (VADDR_T) LL_REG(hold));
		arg->data[hold]	= (u16) (0xFFFF & reg);

		// Sequentially read the non-HOLD and non-RELEASE channels.

		for (i = begin; i <= end; i++)
		{
			if ((i == hold) || (i == release) || ((diff) && (i % 2)))
				continue;

			reg				= os_reg_mem_rx_u32(dev, (VADDR_T) LL_REG(i));
			arg->data[i]	= (u16) (0xFFFF & reg);
		}

		// Read the RELEASE channel.

		if (hold != release)
		{
			reg					= os_reg_mem_rx_u32(dev, (VADDR_T) LL_REG(release));
			arg->data[release]	= (u16) (0xFFFF & reg);
		}

		break;
	}

	return(ret);
}



//*****************************************************************************
static int _rag_enable(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_GEN_ENABLE_NO,
		AI64SSA_GEN_ENABLE_YES,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_ragr_32, 16, 16);
	return(ret);
}



//*****************************************************************************
static int _rag_nrate(dev_data_t* dev, s32* arg)
{
	int	ret;

	ret	= gsc_s32_range_reg(
			dev,
			arg,
			dev->cache.nrate_min,
			dev->cache.nrate_max,
			dev->vaddr.gsc_ragr_32,
			15,
			0);
	return(ret);
}



//*****************************************************************************
static int _rbg_clk_src(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_RBG_CLK_SRC_MASTER,
		AI64SSA_RBG_CLK_SRC_RAG,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_sscr_32, 10, 10);
	return(ret);
}



//*****************************************************************************
static int _rbg_enable(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_GEN_ENABLE_NO,
		AI64SSA_GEN_ENABLE_YES,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_rbgr_32, 16, 16);
	return(ret);
}



//*****************************************************************************
static int _rbg_nrate(dev_data_t* dev, void* arg)
{
	int	ret;

	ret	= gsc_s32_range_reg(
			dev,
			arg,
			dev->cache.nrate_min,
			dev->cache.nrate_max,
			dev->vaddr.gsc_rbgr_32,
			15,
			0);
	return(ret);
}



//*****************************************************************************
static int _rx_io_abort(dev_data_t* dev, s32* arg)
{
	arg[0]	= gsc_read_abort_active_xfer(dev, &dev->io.rx);
	return(0);
}



//*****************************************************************************
static int _rx_io_mode(dev_data_t* dev, s32* arg)
{
	static const s32	list[]	=
	{
		GSC_IO_MODE_PIO,
		GSC_IO_MODE_BMDMA,
		GSC_IO_MODE_DMDMA,
		-1
	};

	int	ret;

	ret	= gsc_s32_list_var(arg, list, &dev->io.rx.io_mode);
	return(ret);
}



//*****************************************************************************
static int _rx_io_overflow(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_IO_OVERFLOW_IGNORE,
		AI64SSA_IO_OVERFLOW_CHECK,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_var(arg, options, &dev->io.rx.overflow_check);
	return(ret);
}



//*****************************************************************************
static int _rx_io_timeout(dev_data_t* dev, s32* arg)
{
	int	ret;

	ret	= gsc_s32_range_var(
			arg,
			AI64SSA_IO_TIMEOUT_MIN,
			AI64SSA_IO_TIMEOUT_INFINITE,
			&dev->io.rx.timeout_s);
	return(ret);
}



//*****************************************************************************
static int _rx_io_underflow(dev_data_t* dev, s32* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_IO_UNDERFLOW_IGNORE,
		AI64SSA_IO_UNDERFLOW_CHECK,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_var(arg, options, &dev->io.rx.underflow_check);
	return(ret);
}



//*****************************************************************************
static int _samp_clk_src(dev_data_t* dev, void* arg)
{
	static const s32	options[]	=
	{
		AI64SSA_SAMP_CLK_SRC_RAG,
		AI64SSA_SAMP_CLK_SRC_RBG,
		AI64SSA_SAMP_CLK_SRC_EXT,
		AI64SSA_SAMP_CLK_SRC_BCR,
		-1	// terminate list
	};

	int	ret;

	ret	= gsc_s32_list_reg(dev, arg, options, dev->vaddr.gsc_sscr_32, 4, 3);
	return(ret);
}



//*****************************************************************************
static int _scan_marker(dev_data_t* dev, s32* arg)
{
	u32	bctlr;
	u32	bit;
	int	ret	= 0;

	if (dev->cache.pci9080)
	{
		switch (arg[0])
		{
			default:

				ret	= -EINVAL;
				break;

			case -1:

				bctlr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);
				bit		= bctlr & D18;
				arg[0]	= bit
						? AI64SSA_SCAN_MARKER_ENABLE
						: AI64SSA_SCAN_MARKER_DISABLE;
				break;

			case AI64SSA_SCAN_MARKER_DISABLE:

				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, 0, D18);
				break;

			case AI64SSA_SCAN_MARKER_ENABLE:

				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, D18, D18);
				break;
		}
	}
	else
	{
		switch (arg[0])
		{
			default:

				ret	= -EINVAL;
				break;

			case -1:

				bctlr	= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_bctlr_32);
				bit		= bctlr & D11;
				arg[0]	= bit
						? AI64SSA_SCAN_MARKER_DISABLE
						: AI64SSA_SCAN_MARKER_ENABLE;
				break;

			case AI64SSA_SCAN_MARKER_DISABLE:

				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, D11, D11);
				break;

			case AI64SSA_SCAN_MARKER_ENABLE:

				os_reg_mem_mx_u32(dev, dev->vaddr.gsc_bctlr_32, 0, D11);
				break;
		}
	}

	return(ret);
}



//*****************************************************************************
static int _scan_marker_get(dev_data_t* dev, u32* arg)
{
	u32	lwr;
	u32	upr;

	upr		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_smuwr_32);
	lwr		= os_reg_mem_rx_u32(dev, dev->vaddr.gsc_smlwr_32);
	arg[0]	= ((upr & 0xFFFF) << 16) | (lwr & 0xFFFF);
	return(0);
}



//*****************************************************************************
static int _scan_marker_set(dev_data_t* dev, u32* arg)
{
	u32	lwr;
	u32	upr;

	lwr	= arg[0] & 0xFFFF;
	upr	= arg[0] >> 16;
	os_reg_mem_mx_u32(dev, dev->vaddr.gsc_smuwr_32, upr, 0xFFFF);
	os_reg_mem_mx_u32(dev, dev->vaddr.gsc_smlwr_32, lwr, 0xFFFF);
	return(0);
}



//*****************************************************************************
static int _low_lat_hold_chan(dev_data_t* dev, s32* arg)
{
	int	ret;

	if (dev->cache.reg_llcr == 0)
		ret	= -EIO;
	else
		ret	= gsc_s32_range_reg(dev, arg, 0, dev->cache.channel_qty - 1, dev->vaddr.gsc_llcr_32, 5, 0);

	return(ret);
}



//*****************************************************************************
static int _low_lat_rel_chan(dev_data_t* dev, s32* arg)
{
	int	ret;

	if (dev->cache.reg_llcr == 0)
		ret	= -EIO;
	else
		ret	= gsc_s32_range_reg(dev, arg, 0, dev->cache.channel_qty - 1, dev->vaddr.gsc_llcr_32, 11, 6);

	return(ret);
}



// variables ******************************************************************

const gsc_ioctl_t	dev_ioctl_list[]	=
{
	{ AI64SSA_IOCTL_REG_READ,			(void*) gsc_reg_read_ioctl		},
	{ AI64SSA_IOCTL_REG_WRITE,			(void*) gsc_reg_write_ioctl		},
	{ AI64SSA_IOCTL_REG_MOD,			(void*) gsc_reg_mod_ioctl		},
	{ AI64SSA_IOCTL_QUERY,				(void*) _query					},
	{ AI64SSA_IOCTL_INITIALIZE,			(void*) initialize_ioctl		},
	{ AI64SSA_IOCTL_AUTOCAL,			(void*) _autocal				},
	{ AI64SSA_IOCTL_AUTOCAL_STATUS,		(void*) _autocal_status			},
	{ AI64SSA_IOCTL_AI_BUF_CLEAR,		(void*) _ai_buf_clear			},
	{ AI64SSA_IOCTL_AI_BUF_LEVEL,		(void*) _ai_buf_level			},
	{ AI64SSA_IOCTL_AI_BUF_OVERFLOW,	(void*) _ai_buf_overflow		},
	{ AI64SSA_IOCTL_AI_BUF_THR_LVL,		(void*) _ai_buf_thr_lvl			},
	{ AI64SSA_IOCTL_AI_BUF_THR_STS,		(void*) _ai_buf_thr_sts			},
	{ AI64SSA_IOCTL_AI_BUF_UNDERFLOW,	(void*) _ai_buf_underflow		},
	{ AI64SSA_IOCTL_AI_MODE,			(void*) _ai_mode				},
	{ AI64SSA_IOCTL_AI_RANGE,			(void*) _ai_range				},
	{ AI64SSA_IOCTL_AUX_0_MODE,			(void*) _aux_0_mode				},
	{ AI64SSA_IOCTL_AUX_1_MODE,			(void*) _aux_1_mode				},
	{ AI64SSA_IOCTL_AUX_2_MODE,			(void*) _aux_2_mode				},
	{ AI64SSA_IOCTL_AUX_3_MODE,			(void*) _aux_3_mode				},
	{ AI64SSA_IOCTL_AUX_IN_POL,			(void*) _aux_in_pol				},
	{ AI64SSA_IOCTL_AUX_NOISE,			(void*) _aux_noise				},
	{ AI64SSA_IOCTL_AUX_OUT_POL,		(void*) _aux_out_pol			},
	{ AI64SSA_IOCTL_BURST_SIZE,			(void*) _burst_size				},
	{ AI64SSA_IOCTL_BURST_SRC,			(void*) _burst_src				},
	{ AI64SSA_IOCTL_BURST_STATUS,		(void*) _burst_status			},
	{ AI64SSA_IOCTL_CHAN_ACTIVE,		(void*) _chan_active			},
	{ AI64SSA_IOCTL_CHAN_FIRST,			(void*) _chan_first				},
	{ AI64SSA_IOCTL_CHAN_LAST,			(void*) _chan_last				},
	{ AI64SSA_IOCTL_CHAN_SINGLE,		(void*) _chan_single			},
	{ AI64SSA_IOCTL_DATA_FORMAT,		(void*) _data_format			},
	{ AI64SSA_IOCTL_DATA_PACKING,		(void*) _data_packing			},
	{ AI64SSA_IOCTL_EXT_SYNC_ENABLE,	(void*) _ext_sync_enable		},
	{ AI64SSA_IOCTL_INPUT_SYNC,			(void*) _input_sync				},
	{ AI64SSA_IOCTL_IRQ0_SEL,			(void*) _irq0_sel				},
	{ AI64SSA_IOCTL_IRQ1_SEL,			(void*) _irq1_sel				},
	{ AI64SSA_IOCTL_LOW_LAT_DOH,		(void*) _low_lat_doh			},
	{ AI64SSA_IOCTL_LOW_LAT_READ,		(void*) _low_lat_read			},
	{ AI64SSA_IOCTL_RAG_ENABLE,			(void*) _rag_enable				},
	{ AI64SSA_IOCTL_RAG_NRATE,			(void*) _rag_nrate				},
	{ AI64SSA_IOCTL_RBG_CLK_SRC,		(void*) _rbg_clk_src			},
	{ AI64SSA_IOCTL_RBG_ENABLE,			(void*) _rbg_enable				},
	{ AI64SSA_IOCTL_RBG_NRATE,			(void*) _rbg_nrate				},
	{ AI64SSA_IOCTL_RX_IO_ABORT,		(void*) _rx_io_abort			},
	{ AI64SSA_IOCTL_RX_IO_MODE,			(void*) _rx_io_mode				},
	{ AI64SSA_IOCTL_RX_IO_OVERFLOW,		(void*) _rx_io_overflow			},
	{ AI64SSA_IOCTL_RX_IO_TIMEOUT,		(void*) _rx_io_timeout			},
	{ AI64SSA_IOCTL_RX_IO_UNDERFLOW,	(void*) _rx_io_underflow		},
	{ AI64SSA_IOCTL_SAMP_CLK_SRC,		(void*) _samp_clk_src			},
	{ AI64SSA_IOCTL_SCAN_MARKER,		(void*) _scan_marker			},
	{ AI64SSA_IOCTL_SCAN_MARKER_GET,	(void*) _scan_marker_get		},
	{ AI64SSA_IOCTL_SCAN_MARKER_SET,	(void*) _scan_marker_set		},
	{ AI64SSA_IOCTL_WAIT_CANCEL,		(void*) gsc_wait_cancel_ioctl	},
	{ AI64SSA_IOCTL_WAIT_EVENT,			(void*) gsc_wait_event_ioctl	},
	{ AI64SSA_IOCTL_WAIT_STATUS,		(void*) gsc_wait_status_ioctl	},
	{ AI64SSA_IOCTL_LOW_LAT_HOLD_CHAN,	(void*) _low_lat_hold_chan		},
	{ AI64SSA_IOCTL_LOW_LAT_REL_CHAN,	(void*) _low_lat_rel_chan		},
	{ -1, NULL }
};


