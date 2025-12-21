// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_reg_mod.c $
// $Rev: 44253 $
// $Date: 2018-12-10 12:58:41 -0600 (Mon, 10 Dec 2018) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_reg_mod
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_REG_MOD service.
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
*		reg		This is the register to access.
*
*		set		This is the value to apply via the mask.
*
*		mask	These are the bits to modify.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int opto16x16_reg_mod(int fd, int index, int verbose, u32 reg, u32 set, u32 mask)
{
	gsc_reg_t	arg;
	int			errs;
	int			ret;
	const char*	name;

	if (verbose)
		gsc_label_index("Register Mod", index);

	arg.reg		= reg;
	arg.value	= set;
	arg.mask	= mask;
	ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_MOD, &arg);
	errs		= ret ? 1 : 0;

	if (verbose)
	{
		name	= opto16x16_reg_get_name(reg);
		printf(	"%s  (val 0x%08lX, mask 0x%08lX, %s)\n",
				errs ? "FAIL <---" : "PASS",
				(unsigned long) set,
				(unsigned long) mask,
				name ? name : "UNKNOWN");
	}

	return(errs);
}


