// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/service.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



/******************************************************************************
*
*	Function:	_ioctl_get
*
*	Purpose:
*
*		Issue the given IOCTL call to retrieve the current setting, then
*		verify that the setting is as expected.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		cmd		The IOCTL service to request.
*
*		arg		The argument to pass to the service.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _ioctl_get(int fd, int index, unsigned long cmd, s32 arg)
{
	int	errs	= 0;
	s32	get;
	int	ret;

	for (;;)	// A convenience loop.
	{
		// Read the current setting.
		get	= -1;
		ret	= opto16x16_ioctl(fd, cmd, &get);

		if (ret < 0)
		{
			// There was a problem.
			errs	= 1;
			printf("FAIL <--- (%d. ", __LINE__);

			if (index >= 0)
				printf("index %d, ", index);

			printf("opto16x16_ioctl() error %d)\n", ret);
			break;
		}

		if (arg != get)
		{
			// We didn't get the same setting back.
			errs	= 1;
			printf("FAIL <--- (%d. ", __LINE__);

			if (index >= 0)
				printf("index %d, ", index);

			printf(	"got %ld (0x%lX), expected %ld (0x%lX))\n",
					(long) get,
					(long) get,
					(long) arg,
					(long) arg);
			break;
		}

		// All went well.
		errs	= 0;
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_ioctl_set
*
*	Purpose:
*
*		Issue the given IOCTL call.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		cmd		The IOCTL service to request.
*
*		arg		The argument to pass to the service.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _ioctl_set(int fd, int index, unsigned long cmd, s32 arg)
{
	int	errs	= 0;
	int	ret;

	for (;;)	// A convenience loop.
	{
		// Perform the operation.
		ret	= opto16x16_ioctl(fd, cmd, &arg);

		if (ret == 0)
		{
			// All went well.
			errs	= 0;
			break;
		}

		// There was an error.
		errs	= 1;
		printf("FAIL <--- (");

		if (index >= 0)
			printf("index %d, ", index);

		printf("opto16x16_ioctl(), error %d)\n", ret);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_reg_mod
*
*	Purpose:
*
*		Perform a read-modify-write on the specified register.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		reg		The register to access. If -1 then this is ignored.
*
*		mask	The set of register bits of interest.
*
*		value	The value to apply/check for the bits of interest.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _reg_mod(int fd, int index, u32 reg, u32 mask, u32 value)
{
	int			errs	= 0;
	gsc_reg_t	parm;
	int			ret;

	for (;;)	// A convenience loop.
	{
		if (reg == -1)
		{
			// We're supposed to ignore this register.
			errs	= 0;
			break;
		}

		// Perform the operation.
		parm.reg	= reg;
		parm.value	= value;;
		parm.mask	= mask;
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_MOD, &parm);

		if (ret == 0)
		{
			// All went well.
			errs	= 0;
			break;
		}

		// There was an error.
		errs	= 1;
		printf("FAIL <--- (");

		if (index >= 0)
			printf("index %d, ", index);

		printf("OPTO16X16_IOCTL_REG_MOD, error %d)\n", ret);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_reg_read
*
*	Purpose:
*
*		Read a register, but do nothing with the value.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		reg		The register to access. If -1 then this is ignored.
*
*		arg		This is the number of of reads to perform. This is presumed
*				to me one or more.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _reg_read(int fd, int index, u32 reg, s32 arg)
{
	int			errs	= 0;
	gsc_reg_t	parm;
	int			ret;

	for (;;)	// A convenience loop.
	{
		if (reg == -1)
		{
			// We're supposed to ignore this register.
			break;
		}

		arg	= (arg < 1) ? 1 : arg;

		for (; arg; arg--)
		{
			// Read the register.
			parm.reg	= reg;
			parm.value	= 0;;
			parm.mask	= 0;
			ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, &parm);

			if (ret < 0)
			{
				// There was a problem.
				errs	= 1;
				printf("FAIL <--- (%d. ", __LINE__);

				if (index >= 0)
					printf("index %d, ", index);

				printf("OPTO16X16_IOCTL_REG_READ, error %d)\n", ret);
				break;
			}
		}

		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_reg_show
*
*	Purpose:
*
*		Read a register and display its content.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		reg		The register to access. If -1 then this is ignored.
*
*		ul		A pointer to a string to display, if non-NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _reg_show(int fd, int index, u32 reg, unsigned long ul)
{
	int			errs	= 0;
	gsc_reg_t	parm;
	int			ret;

	for (;;)	// A convenience loop.
	{
		if (reg == -1)
		{
			// We're supposed to ignore this register.
			errs	= 0;
			break;
		}

		// Read the register.
		parm.reg	= reg;
		parm.value	= 0;;
		parm.mask	= 0;
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, &parm);

		if (ret < 0)
		{
			// There was a problem.
			errs	= 1;
			printf("FAIL <--- (%d. ", __LINE__);

			if (index >= 0)
				printf("index %d, ", index);

			printf("OPTO16X16_IOCTL_REG_READ, error %d)\n", ret);
			break;
		}
		else
		{
			errs	= 0;
			printf("(0x%lX) ", (long) parm.value);
		}

		break;
	}

	fflush(stdout);
	return(errs);
}



/******************************************************************************
*
*	Function:	_reg_test
*
*	Purpose:
*
*		Verify that the specified register bits have the specified values.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		reg		The register to access. If -1 then this is ignored.
*
*		mask	The set of register bits of interest.
*
*		value	The value to apply/check for the bits of interest.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _reg_test(int fd, int index, u32 reg, u32 mask, u32 value)
{
	int			errs	= 0;
	gsc_reg_t	parm;
	int			ret;

	for (;;)	// A convenience loop.
	{
		if (reg == -1)
		{
			// We're supposed to ignore this register.
			errs	= 0;
			break;
		}

		if (value & ~mask)
		{
			// Excess value bits are set.
			errs	= 1;
			printf("FAIL <--- (%d. ", __LINE__);

			if (index >= 0)
				printf("index %d, ", index);

			printf(	"INTERNAL ERROR, value 0x%lX, mask 0x%lX)\n",
					(long) value,
					(long) mask);
			break;
		}

		// Read the register.
		parm.reg	= reg;
		parm.value	= 0;;
		parm.mask	= 0;
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, &parm);

		if (ret < 0)
		{
			// There was a problem.
			errs	= 1;
			printf("FAIL <--- (%d. ", __LINE__);

			if (index >= 0)
				printf("index %d, ", index);

			printf("OPTO16X16_IOCTL_REG_READ, error %d)\n", ret);
			break;
		}

		if ((parm.value & mask) == (value & mask))
		{
			// The register bits of interest are as expected.
			errs	= 0;
			break;
		}

		// The register bits of interest are NOT as expected.
		errs	= 1;
		printf("FAIL <--- (%d. ", __LINE__);

		if (index >= 0)
			printf("index %d, ", index);

		printf(	"expected 0x%lX, got 0x%lX (0x%08lX))\n",
				(long) value,
				(long) parm.value & mask,
				(long) parm.value);

		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_reg_write
*
*	Purpose:
*
*		Perform a write to the specified register.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	The table index to report, or negactive to ignore.
*
*		reg		The register to access. If -1 then this is ignored.
*
*		arg		This is the number of times to perform the write. This is
*				presumed to be at least one.
*
*		value	The value to write.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _reg_write(int fd, int index, u32 reg, s32 arg, u32 value)
{
	int			errs	= 0;
	gsc_reg_t	parm;
	int			ret;

	for (;;)	// A convenience loop.
	{
		if (reg == -1)
		{
			// We're supposed to ignore this register.
			break;
		}

		arg	= (arg < 1) ? 1 : arg;

		for (; arg > 0; arg--)
		{
			// Perform the operation.
			parm.reg	= reg;
			parm.value	= value;;
			parm.mask	= 0;
			ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_WRITE, &parm);

			if (ret < 0)
			{
				errs++;
				printf("FAIL <--- (");

				if (index >= 0)
					printf("index %d, ", index);

				printf("OPTO16X16_IOCTL_REG_WRITE, error %d)\n", ret);
				break;
			}
		}

		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_service_perform
*
*	Purpose:
*
*		Perform the specified services on the given structure.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	A table index to report. Ignore this if negative.
*
*		service	The service to perform.
*
*		item	The structure to process.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _service_perform(
	int						fd,
	int						index,
	service_t				service,
	const service_data_t*	item)
{
	int	errs	= 0;

	switch (service)
	{
		default:
		case SERVICE_END_LIST:
		case SERVICE_NORMAL:

			errs	= 1;
			printf(	"FAIL <---  (%d. INTERNAL ERROR: ", __LINE__);

					if (index >= 0)
						printf("index %d, ", index);

			printf("service %d)\n", (int) service);
			break;

		case SERVICE_IOCTL_GET:

			errs	= _ioctl_get(fd, index, item->cmd, item->arg);
			break;

		case SERVICE_NONE:

			errs	= 0;
			break;

		case SERVICE_IOCTL_SET:

			errs	= _ioctl_set(fd, index, item->cmd, item->arg);
			break;

		case SERVICE_REG_MOD:

			errs	= _reg_mod(fd, index, item->reg, item->mask, item->value);
			break;

		case SERVICE_REG_READ:

			errs	= _reg_read(fd, index, item->reg, item->arg);
			break;

		case SERVICE_REG_SHOW:

			errs	= _reg_show(fd, index, item->reg, item->arg);
			break;

		case SERVICE_REG_TEST:

			errs	= _reg_test(fd, index, item->reg, item->mask, item->value);
			break;

		case SERVICE_REG_WRITE:

			errs	= _reg_write(fd, index, item->reg, item->arg, item->value);
			break;

		case SERVICE_SLEEP:

			errs	= 0;
			os_sleep_ms(item->arg * 1000);
			break;

		case SERVICE_SLEEP_MS:

			errs	= 0;
			os_sleep_ms(item->arg);
			break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_service_item
*
*	Purpose:
*
*		Perform the specified services on the given structure.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	A table index to report. Ignore this if negative.
*
*		item	The structure to process.
*
*		srv1	The first primary service to perform.
*
*		srv2	The second primary service to perform.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _service_item(
	int						fd,
	int						index,
	const service_data_t*	item,
	service_t				srv1,
	service_t				srv2)
{
	int	errs	= 0;

	if (item->service == SERVICE_NORMAL)
	{
		errs	= _service_perform(fd, index, srv1, item);

		if (errs == 0)
			errs	= _service_perform(fd, index, srv2, item);
	}
	else
	{
		errs	= _service_perform(fd, index, item->service, item);
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_service_list
*
*	Purpose:
*
*		Go through the list and, for each element, perform the specified
*		services.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		index	A table index to report. Ignore this if negative.
*
*		list	The list to go through.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

static int _service_list(
	int						fd,
	const service_data_t*	list,
	service_t				srv1,
	service_t				srv2)
{
	int	errs	= 0;
	int	i;

	for (i = 0; (errs == 0) && (list[i].service != SERVICE_END_LIST); i++)
		errs	+= _service_item(fd, i, &list[i], srv1, srv2);

	return(errs);
}



/******************************************************************************
*
*	Function:	service_ioctl_reg_get_list
*
*	Purpose:
*
*		Go through the list and, for each element, perform the specified
*		services.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		list	The list to go through.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int service_ioctl_reg_get_list(int fd, const service_data_t* list)
{
	int	errs;

	errs	= _service_list(fd, list, SERVICE_REG_MOD, SERVICE_IOCTL_GET);
	return(errs);
}



/******************************************************************************
*
*	Function:	service_ioctl_set_list
*
*	Purpose:
*
*		Go through the list and, for each element, perform the specified
*		services.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		list	The list to go through.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int service_ioctl_set_list(int fd, const service_data_t* list)
{
	int	errs;

	errs	= _service_list(fd, list, SERVICE_IOCTL_SET, SERVICE_NONE);
	return(errs);
}



/******************************************************************************
*
*	Function:	service_ioctl_set_get_list
*
*	Purpose:
*
*		Go through the list and, for each element, perform the specified
*		services.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		list	The list to go through.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int service_ioctl_set_get_list(int fd, const service_data_t* list)
{
	int	errs;

	errs	= _service_list(fd, list, SERVICE_IOCTL_SET, SERVICE_IOCTL_GET);
	return(errs);
}



/******************************************************************************
*
*	Function:	service_ioctl_set_reg_list
*
*	Purpose:
*
*		Go through the list and, for each element, perform the specified
*		services.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*		list	The list to go through.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int service_ioctl_set_reg_list(int fd, const service_data_t* list)
{
	int	errs;

	errs	= _service_list(fd, list, SERVICE_IOCTL_SET, SERVICE_REG_TEST);
	return(errs);
}


