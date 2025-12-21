// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_reg_mod.c $
// $Rev: 42829 $
// $Date: 2018-05-17 17:02:12 -0500 (Thu, 17 May 2018) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_reg_mod
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_REG_MOD service.
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

int ai64ssa_reg_mod(int fd, int index, int verbose, u32 reg, u32 set, u32 mask)
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
	ret			= ai64ssa_ioctl(fd, AI64SSA_IOCTL_REG_MOD, &arg);
	errs		= ret ? 1 : 0;

	if (verbose)
	{
		name	= ai64ssa_reg_get_name(reg);
		printf(	"%s  (val 0x%08lX, mask 0x%08lX, %s)\n",
				errs ? "FAIL <---" : "PASS",
				(unsigned long) set,
				(unsigned long) mask,
				name ? name : "UNKNOWN");
	}

	return(errs);
}


