// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/reg_mod.c $
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
		u32	mask;
	} list[]	=
	{
		// reg					mask
		{ OPTO16X16_GSC_CPR,	0x0000FFFF	},
		{ OPTO16X16_GSC_CDR,	0x00FFFFFF	},
		{ OPTO16X16_GSC_ODR,	0x0000FFFF	},
	};

	u32			bit;
	int			errs	= 0;
	int			i;
	gsc_reg_t	parm;
	int			ret;
	u32			value;

	for (i = 0; (errs == 0) && (i < SIZEOF_ARRAY(list)); i++)
	{

		for (bit = 0x1; bit; bit <<= 1)
		{
			if ((list[i].mask & bit) == 0)
				continue;

			// Read the register value so we can restore it later.
			parm.reg	= list[i].reg;
			ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, (void*) &parm);
			value		= parm.value;

			if (ret < 0)
			{
				errs	= 1;
				printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
						__LINE__,
						i,
						ret);
				break;
			}

			// Set the bit low.
			parm.value	= 0;
			parm.mask	= bit;
			ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_MOD, (void*) &parm);

			if (ret < 0)
			{
				errs	= 1;
				printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
						__LINE__,
						i,
						ret);
				break;
			}

			// Now verify that the bit is low.
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

			if (parm.value & bit)
			{
				errs	= 1;
				printf(	"FAIL <---  (%d. i %d, bit not low, mask 0x%lX, got 0x%lX)\n",
						__LINE__,
						i,
						(long) bit,
						(long) parm.value);
				break;
			}

			// Now restore the register to its previous value.
			parm.value	= value;
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

			// Set the bit high.
			parm.value	= bit;
			parm.mask	= bit;
			ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_MOD, (void*) &parm);

			if (ret < 0)
			{
				errs	= 1;
				printf(	"FAIL <---  (%d. i %d, opto16x16_ioctl() failure, error %d)\n",
						__LINE__,
						i,
						ret);
				break;
			}

			// Now verify that the bit is high.
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

			if ((parm.value & bit) == 0)
			{
				errs	= 1;
				printf(	"FAIL <---  (%d. i %d, bit not high, mask 0x%lX, got 0x%lX)\n",
						__LINE__,
						i,
						(long) bit,
						(long) parm.value);
				break;
			}

			// Now restore the register to its previous value.
			parm.value	= value;
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
		}
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	reg_mod_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_REG_MOD.
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

int reg_mod_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_REG_MOD");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


