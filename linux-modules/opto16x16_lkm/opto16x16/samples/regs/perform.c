// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/perform.c $
// $Rev: 53723 $
// $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static void _dump_gsc(int fd)
{
	printf("GSC Registers:\n");
	opto16x16_reg_list(fd, 0);
}



//*****************************************************************************
static void _dump_gsc_detail(int fd)
{
	printf("GSC Registers:\n");
	opto16x16_reg_list(fd, 1);
}



//*****************************************************************************
static void _dump_pci(int fd)
{
	printf("PCI Registers:\n");
	gsc_reg_plx9056_list_pci(fd, opto16x16_reg_read);
}



//*****************************************************************************
static void _dump_plx(int fd)
{
	printf("PLX Registers:\n");
	gsc_reg_plx9056_list_plx(fd, opto16x16_reg_read);
}



//*****************************************************************************
static void _id_device(int fd)
{
	opto16x16_id_device(fd, -1, 1);
}



//*****************************************************************************
static void _id_driver(int fd)
{
	os_id_driver(opto16x16_open, opto16x16_read, opto16x16_close);
}



//*****************************************************************************
static void _id_host(int fd)
{
	os_id_host();
}



//*****************************************************************************
static void _dump_bar2(int fd)
{
	int			i;
	gsc_reg_t	reg;
	int			ret;

	printf("\n");
	printf("BAR2 Dump  (first 32 words)\n");
	printf("Offset  Value\n");
	printf("======  ==========\n");

	for (i = 0; i < 128; i += 4)
	{
		printf("0x%02lX  ", (long) i);
		reg.reg		= OPTO16X16_REG_ENCODE(4, i);
		reg.value	= 0xDEADBEEF;
		reg.mask	= 0;	// unused here
		ret			= opto16x16_ioctl(fd, OPTO16X16_IOCTL_REG_READ, &reg);
		printf("  0x%08lX", (long) reg.value);

		if (ret)
		{
			printf("  FAIL <----  (opto16x16_ioctl() returned %d)\n", ret);
			break;
		}

		printf("\n");
	}
}



//*****************************************************************************
static void _dump_everything(int fd)
{
	_id_host(fd);
	printf("\n");

	_id_driver(fd);
	printf("\n");

	_id_device(fd);
	printf("\n");

	_dump_pci(fd);
	printf("\n");

	_dump_plx(fd);
	printf("\n");

	_dump_gsc_detail(fd);
	printf("\n");

	_dump_bar2(fd);
	printf("\n");
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	static const menu_item_t	list[]	=
	{
		// name								func
		{ "Dump Everything",				_dump_everything	},
		{ "Host OS Identification",			_id_host			},
		{ "Driver Identification",			_id_driver			},
		{ "Device Identification",			_id_device			},
		{ "PCI Register Dump",				_dump_pci			},
		{ "PLX Register Dump",				_dump_plx			},
		{ "GSC Register Dump",				_dump_gsc			},
		{ "GSC Register Detail Dump",		_dump_gsc_detail	},
		{ "BAR2 Dump",						_dump_bar2			},
		{ "Edit GSC Register By Name",		reg_mod_by_name		},
		{ "Edit GSC Register By Offset",	reg_mod_by_offset	},
		{ NULL,								NULL				}
	};

	static const menu_t	menu	=
	{
		/* title	*/	"Main Menu",
		/* list		*/	list
	};

	printf("\n\n");
	printf("Register Access Application\n");
	menu_call(args->fd, &menu);
	return(0);
}


