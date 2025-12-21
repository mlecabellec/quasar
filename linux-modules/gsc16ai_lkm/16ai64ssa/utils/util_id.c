// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_id.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



//*****************************************************************************
static int _id_device_pci(int fd)
{
	int	errs;
	u32	reg;

	gsc_label("Vendor ID");
	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9056_VIDR, &reg);
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

	gsc_label("Device ID");
	errs	+= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9056_DIDR, &reg);
	printf("0x%04lX      ", (long) reg);

	if (reg == 0x9056)
	{
		printf("(PCI9056)\n");
	}
	else if (reg == 0x9080)
	{
		printf("(PCI9080)\n");
	}
	else
	{
		errs++;
		printf("(UNKNOWN) FAIL <---\n");
	}

	gsc_label("Sub Vendor ID");
	errs	+= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9056_SVID, &reg);
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
	errs	+= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9056_SID, &reg);
	printf("0x%04lX      ", (long) reg);

	if (reg == 0x3101)
	{
		printf("(16AI64SSA/C)\n");
	}
	else if (reg == 0x2868)
	{
		printf("(16AI64SSA/C)\n");
	}
	else
	{
		errs++;
		printf("(UNKNOWN) FAIL <---\n");
	}

	return(errs);
}



//*****************************************************************************
int ai64ssa_id_device(int fd, int index, int verbose)
{
	s32	device_type;
	int	errs;

	if (verbose)
		gsc_label_index("Device", index);

	errs	= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_DEVICE_TYPE, &device_type);

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

			case GSC_DEV_TYPE_16AI64SSA:

				if (verbose)
					printf("16AI64SSA\n");

				break;

			case GSC_DEV_TYPE_16AI64SSC:

				if (verbose)
					printf("16AI64SSC\n");

				break;
		}

		gsc_label_level_inc();

		errs	+= _id_device_pci(fd);

		gsc_label_level_dec();
	}

	return(errs);
}


