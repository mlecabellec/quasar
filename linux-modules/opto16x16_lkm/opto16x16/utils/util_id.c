// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_id.c $
// $Rev: 51410 $
// $Date: 2022-07-14 14:32:40 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	_id_device_pci
*
*	Purpose:
*
*		Identify the device using the PCI registers.
*
*	Arguments:
*
*		fd		The handle used to access the device.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

static int _id_device_pci(int fd)
{
	u32	didr;
	int	errs;
	u32	reg;

	gsc_label("Vendor ID");
	errs	= opto16x16_reg_read(fd, -1, 0, GSC_PCI_9056_VIDR, &reg);
	printf("0x%04lX      ", (long) reg);

	if (reg == 0x10B5)
	{
		printf("(PLX)\n");
	}
	else
	{
		errs	= 1;
		printf("(UNKNOWN) FAIL <---\n");
	}

	gsc_label("Device ID");
	errs	+= opto16x16_reg_read(fd, -1, 0, GSC_PCI_9056_DIDR, &didr);
	printf("0x%04lX      ", (long) didr);

	if (didr == 0x9056)
	{
		printf("(PCI9056)\n");
	}
	else
	{
		errs++;
		printf("(UNKNOWN) FAIL <---\n");
	}

	gsc_label("Sub Vendor ID");
	errs	+= opto16x16_reg_read(fd, -1, 0, GSC_PCI_9056_SVID, &reg);
	printf("0x%04lX      ", (long) reg);

	if (reg == 0x10B5)
	{
		printf("(PLX)\n");
	}
	else
	{
		errs++;
		printf("(UNKNOWN) FAIL <---\n");
	}

	gsc_label("Subsystem ID");
	errs	+= opto16x16_reg_read(fd, -1, 0, GSC_PCI_9056_SID, &reg);
	printf("0x%04lX      ", (long) reg);

	if (reg == 0x3460)
	{
		printf("(OPTO16X16)\n");
	}
	else
	{
		errs++;
		printf("(UNKNOWN) FAIL <---\n");
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	opto16x16_id_device
*
*	Purpose:
*
*		Identify the device.
*
*	Arguments:
*
*		fd		The handle to use to access the device.
*
*		index	The index of the device or -1 if we don't care.
*
*		verbose	Work verbosely?
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int opto16x16_id_device(int fd, int index, int verbose)
{
	s32			device_type;
	int			errs;
	const char*	psz		= "";

	if (verbose)
		gsc_label_index("Device", index);

	errs	= opto16x16_query(fd, -1, 0, OPTO16X16_QUERY_DEVICE_TYPE, &device_type);

	if (errs)
	{
		if (verbose)
			printf("FAIL <---  (query error)\n");
	}
	else
	{
		switch (device_type)
		{
			default:

				errs	= 1;

				if (verbose)
				{
					printf(	"FAIL <---  (unexpected device type: %ld)",
							(long) device_type);
				}

				break;

			case GSC_DEV_TYPE_OPTO16X16:	psz	= "OPTO16X16";	break;
		}

		if ((verbose) && (errs == 0))
			printf("%s\n", psz);

		gsc_label_level_inc();

		errs	+= _id_device_pci(fd);

		gsc_label_level_dec();
	}

	return(errs);
}



