// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_reg_read.c $
// $Rev: 51410 $
// $Date: 2022-07-14 14:32:40 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_reg_read
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_REG_READ service.
*
*	Arguments:
*
*		fd		Use this handle to access the device.
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely?
*
*		reg		The register to access.
*
*		get		The value read goes here.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int opto16x16_reg_read(int fd, int index, int verbose, u32 reg, u32* get)
{
	gsc_reg_t	arg;
	int			errs;
	int			ret;
	const char*	name;

	if (verbose)
		gsc_label_index("Register Read", index);

	arg.reg		= reg;
	arg.value	= 0xDEADBEEF;
	arg.mask	= 0;	// unused here
	ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, &arg);
	errs		= ret ? 1 : 0;

	if (verbose)
	{
		name	= opto16x16_reg_get_name(reg);
		printf(	"%s  (val 0x%08lX, %s)\n",
				errs ? "FAIL <---" : "PASS",
				(unsigned long) arg.value,
				name ? name : "UNKNOWN");
	}

	if (get)
		get[0]	= arg.value;

	return(errs);
}


