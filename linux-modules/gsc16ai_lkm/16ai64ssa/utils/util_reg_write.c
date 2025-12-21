// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_reg_write.c $
// $Rev: 51330 $
// $Date: 2022-07-11 17:27:44 -0500 (Mon, 11 Jul 2022) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_reg_write
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_REG_WRITE service.
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
*		set		This is the value to apply.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int ai64ssa_reg_write(int fd, int index, int verbose, u32 reg, u32 set)
{
	gsc_reg_t	arg;
	int			errs;
	int			ret;
	const char*	name;

	if (verbose)
		gsc_label_index("Register Write", index);

	arg.reg		= reg;
	arg.value	= set;
	arg.mask	= 0;	// unused here
	ret			= ai64ssa_ioctl(fd, AI64SSA_IOCTL_REG_WRITE, &arg);
	errs		= ret ? 1 : 0;

	if (verbose)
	{
		name	= ai64ssa_reg_get_name(reg);
		printf(	"%s  (val 0x%08lX, %s)\n",
				errs ? "FAIL <---" : "PASS",
				(unsigned long) arg.value,
				name ? name : "UNKNOWN");
	}

	return(errs);
}


