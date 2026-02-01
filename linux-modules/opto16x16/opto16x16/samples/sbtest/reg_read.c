// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/reg_read.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _service_test(int fd)
{
	return(0);
}



//*****************************************************************************
static int _function_test(int fd)
{
	static const struct
	{
		u32	reg;
		u32	value;
	} list[]	=
	{
		// reg					value
		{ OPTO16X16_GSC_BCSR,	0		},
		{ OPTO16X16_GSC_RDR,	0		},
		{ OPTO16X16_GSC_COSR,	0		},
		{ OPTO16X16_GSC_RECR,	0		},
		{ OPTO16X16_GSC_CIER,	0		},
		{ OPTO16X16_GSC_CPR,	0		},
		{ OPTO16X16_GSC_CDR,	0		},
		{ OPTO16X16_GSC_ODR,	0		},

		{ GSC_PCI_9056_VIDR,	0x10B5	},
		{ GSC_PCI_9056_DIDR,	0x9056	},
		{ GSC_PCI_9056_SVID,	0x10B5	},
		{ GSC_PCI_9056_SID,		0		},

		{ GSC_PLX_9056_VIDR,	0x10B5	},
		{ GSC_PLX_9056_DIDR,	0x9056	}
	};

	int			errs	= 0;
	int			i;
	gsc_reg_t	parm;
	int			ret;

	for (i = 0; i < SIZEOF_ARRAY(list); i++)
	{
		parm.reg	= list[i].reg;
		parm.value	= 0xDEADBEEF;
		parm.mask	= 0xBEEFDEAD;
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, (void*) &parm);

		if (ret < 0)
		{
			errs	= 1;
			printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
					__LINE__,
					i,
					ret);
			break;
		}

		if (parm.value == 0xDEADBEEF)
		{
			errs	= 1;
			printf("FAIL <---  (%d. i %d, value not changed)\n", __LINE__, i);
			break;
		}

		if (parm.mask != 0xBEEFDEAD)
		{
			errs	= 1;
			printf("FAIL <---  (%d. i %d, mask changed)\n", __LINE__, i);
			break;
		}

		if ((list[i].value) && (parm.value != list[i].value))
		{
			errs	= 1;
			printf(	"FAIL <---  (%d. i %d, got 0x%lX, expected 0x%lX)\n",
					__LINE__,
					i,
					(long) parm.value,
					(long) list[i].value);
			break;
		}
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	reg_read_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_REG_READ.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int reg_read_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_REG_READ");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


