// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/reg_write.c $
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
		u32	mask;
	} list[]	=
	{
		// reg					value		mask
		{ OPTO16X16_GSC_CPR,	0x0000AA55,	0x0000FFFF	},
		{ OPTO16X16_GSC_CPR,	0x000055AA,	0x0000FFFF	},
		{ OPTO16X16_GSC_CPR,	0x00000000,	0x0000FFFF	},

		{ OPTO16X16_GSC_CDR,	0x00AA55AA,	0x00FFFFFF	},
		{ OPTO16X16_GSC_CDR,	0x0055AA55,	0x00FFFFFF	},
		{ OPTO16X16_GSC_CDR,	0x00000000,	0x00FFFFFF	},

		{ OPTO16X16_GSC_ODR,	0x0000AA55,	0x0000FFFF	},
		{ OPTO16X16_GSC_ODR,	0x000055AA,	0x0000FFFF	},
		{ OPTO16X16_GSC_ODR,	0x00000000,	0x0000FFFF	},
	};

	int			errs	= 0;
	int			i;
	gsc_reg_t	parm;
	int			ret;

	for (i = 0; i < SIZEOF_ARRAY(list); i++)
	{
		parm.reg	= list[i].reg;
		parm.value	= list[i].value;
		parm.mask	= 0xBEEFDEAD;
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_WRITE, (void*) &parm);

		if (ret < 0)
		{
			errs	= 1;
			printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
					__LINE__,
					i,
					ret);
			break;
		}

		if (parm.mask != 0xBEEFDEAD)
		{
			errs	= 1;
			printf("FAIL <---  (%d. i %d, mask changed)\n", __LINE__, i);
			break;
		}

		// Now read the register and verify the bits of interest.
		ret	= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, (void*) &parm);

		if (ret < 0)
		{
			errs	= 1;
			printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
					__LINE__,
					i,
					ret);
			break;
		}

		if ((list[i].value & list[i].mask) != (parm.value & list[i].mask))
		{
			errs	= 1;
			printf(	"FAIL <---  (%d. i %d, mask 0x%lX, got 0x%lX, expected 0x%lX)\n",
					__LINE__,
					i,
					(long) list[i].mask,
					(long) parm.value,
					(long) list[i].value);
			break;
		}
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	reg_write_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_REG_WRITE.
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

int reg_write_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_REG_WRITE");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


