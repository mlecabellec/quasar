// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_reg.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



// macros *********************************************************************

#define	_GSC_REG(a)				"GSC " #a, OPTO16X16_GSC_##a, 0, 0
#define	WIDTH					27



//*****************************************************************************
static int _bcsr_detail(int fd, int supported, u32 value, int width)
{
	static const char* const	disable[]	= { "Disable",	"Enable"	};
	static const char* const	idle[]		= { "Idle",		"Active"	};
	static const char* const	on[]		= { "On",		"Off"		};

	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 7, 7, 1, on,		"LED"						);
	gsc_reg_field_show(width, WIDTH, value, 6, 6, 1, disable,	"Event Overflow IRQ"		);
	gsc_reg_field_show(width, WIDTH, value, 5, 5, 1, NULL,		"Slow Debounce Clock"		);
	gsc_reg_field_show(width, WIDTH, value, 4, 4, 1, idle,		"Master IRQ Status"			);
	gsc_reg_field_show(width, WIDTH, value, 3, 3, 1, idle,		"Event Overflow IRQ Status"	);
	gsc_reg_field_show(width, WIDTH, value, 2, 2, 1, NULL,		"Reserved"					);
	gsc_reg_field_show(width, WIDTH, value, 1, 1, 1, idle,		"COS 8-15 IRQ Status"		);
	gsc_reg_field_show(width, WIDTH, value, 0, 0, 1, idle,		"COS 0-7 IRQ Status"		);

	return(0);
}



//*****************************************************************************
static int _cosr_detail(int fd, int supported, u32 value, int width)
{
	static const char* const	idle[]	= { "Idle",	"Active"	};

	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 15, 15, 1, idle,	"COS 15"	);
	gsc_reg_field_show(width, WIDTH, value, 14, 14, 1, idle,	"COS 14"	);
	gsc_reg_field_show(width, WIDTH, value, 13, 13, 1, idle,	"COS 13"	);
	gsc_reg_field_show(width, WIDTH, value, 12, 12, 1, idle,	"COS 12"	);
	gsc_reg_field_show(width, WIDTH, value, 11, 11, 1, idle,	"COS 11"	);
	gsc_reg_field_show(width, WIDTH, value, 10, 10, 1, idle,	"COS 10"	);
	gsc_reg_field_show(width, WIDTH, value,  9,  9, 1, idle,	"COS 9"		);
	gsc_reg_field_show(width, WIDTH, value,  8,  8, 1, idle,	"COS 8"		);
	gsc_reg_field_show(width, WIDTH, value,  7,  7, 1, idle,	"COS 7"		);
	gsc_reg_field_show(width, WIDTH, value,  6,  6, 1, idle,	"COS 6"		);
	gsc_reg_field_show(width, WIDTH, value,  5,  5, 1, idle,	"COS 5"		);
	gsc_reg_field_show(width, WIDTH, value,  4,  4, 1, idle,	"COS 4"		);
	gsc_reg_field_show(width, WIDTH, value,  3,  3, 1, idle,	"COS 3"		);
	gsc_reg_field_show(width, WIDTH, value,  2,  2, 1, idle,	"COS 2"		);
	gsc_reg_field_show(width, WIDTH, value,  1,  1, 1, idle,	"COS 1"		);
	gsc_reg_field_show(width, WIDTH, value,  0,  0, 1, idle,	"COS 0"		);

	return(0);
}



//*****************************************************************************
static int _recr_detail(int fd, int supported, u32 value, int width)
{
	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 15, 0, 1, NULL,	"Count"	);

	return(0);
}



//*****************************************************************************
static int _cier_detail(int fd, int supported, u32 value, int width)
{
	static const char* const	disabled[]	= { "Disabled",	"Enabled"	};

	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 15, 15, 1, disabled,	"COS 15"	);
	gsc_reg_field_show(width, WIDTH, value, 14, 14, 1, disabled,	"COS 14"	);
	gsc_reg_field_show(width, WIDTH, value, 13, 13, 1, disabled,	"COS 13"	);
	gsc_reg_field_show(width, WIDTH, value, 12, 12, 1, disabled,	"COS 12"	);
	gsc_reg_field_show(width, WIDTH, value, 11, 11, 1, disabled,	"COS 11"	);
	gsc_reg_field_show(width, WIDTH, value, 10, 10, 1, disabled,	"COS 10"	);
	gsc_reg_field_show(width, WIDTH, value,  9,  9, 1, disabled,	"COS 9"		);
	gsc_reg_field_show(width, WIDTH, value,  8,  8, 1, disabled,	"COS 8"		);
	gsc_reg_field_show(width, WIDTH, value,  7,  7, 1, disabled,	"COS 7"		);
	gsc_reg_field_show(width, WIDTH, value,  6,  6, 1, disabled,	"COS 6"		);
	gsc_reg_field_show(width, WIDTH, value,  5,  5, 1, disabled,	"COS 5"		);
	gsc_reg_field_show(width, WIDTH, value,  4,  4, 1, disabled,	"COS 4"		);
	gsc_reg_field_show(width, WIDTH, value,  3,  3, 1, disabled,	"COS 3"		);
	gsc_reg_field_show(width, WIDTH, value,  2,  2, 1, disabled,	"COS 2"		);
	gsc_reg_field_show(width, WIDTH, value,  1,  1, 1, disabled,	"COS 1"		);
	gsc_reg_field_show(width, WIDTH, value,  0,  0, 1, disabled,	"COS 0"		);

	return(0);
}



//*****************************************************************************
static int _cpr_detail(int fd, int supported, u32 value, int width)
{
	static const char* const	h2l[]	= { "High-to-Low",	"Low-to-High"	};

	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 15, 15, 1, h2l,	"COS 15"	);
	gsc_reg_field_show(width, WIDTH, value, 14, 14, 1, h2l,	"COS 14"	);
	gsc_reg_field_show(width, WIDTH, value, 13, 13, 1, h2l,	"COS 13"	);
	gsc_reg_field_show(width, WIDTH, value, 12, 12, 1, h2l,	"COS 12"	);
	gsc_reg_field_show(width, WIDTH, value, 11, 11, 1, h2l,	"COS 11"	);
	gsc_reg_field_show(width, WIDTH, value, 10, 10, 1, h2l,	"COS 10"	);
	gsc_reg_field_show(width, WIDTH, value,  9,  9, 1, h2l,	"COS 9"		);
	gsc_reg_field_show(width, WIDTH, value,  8,  8, 1, h2l,	"COS 8"		);
	gsc_reg_field_show(width, WIDTH, value,  7,  7, 1, h2l,	"COS 7"		);
	gsc_reg_field_show(width, WIDTH, value,  6,  6, 1, h2l,	"COS 6"		);
	gsc_reg_field_show(width, WIDTH, value,  5,  5, 1, h2l,	"COS 5"		);
	gsc_reg_field_show(width, WIDTH, value,  4,  4, 1, h2l,	"COS 4"		);
	gsc_reg_field_show(width, WIDTH, value,  3,  3, 1, h2l,	"COS 3"		);
	gsc_reg_field_show(width, WIDTH, value,  2,  2, 1, h2l,	"COS 2"		);
	gsc_reg_field_show(width, WIDTH, value,  1,  1, 1, h2l,	"COS 1"		);
	gsc_reg_field_show(width, WIDTH, value,  0,  0, 1, h2l,	"COS 0"		);

	return(0);
}



//*****************************************************************************
static int _cdr_detail(int fd, int supported, u32 value, int width)
{
	u32	div;

	width	+= 10;

	gsc_reg_field_show(width, WIDTH, value, 23, 0, 0, NULL,	"Divider"	);
	div	= value & 0xFFFFFF;
	printf("%ld (/%ld)\n", (long) div, (long) ((div <= 1) ? 2 : div));

	return(0);
}



// variables ******************************************************************

static gsc_reg_def_t	_gsc[]	=
{
	{ _GSC_REG(BCSR),	0,	_bcsr_detail,	"Board Control/Status Register"		},
	{ _GSC_REG(RDR),	0,	NULL,			"Receive Data Register"				},
	{ _GSC_REG(COSR),	0,	_cosr_detail,	"Change of State Register"			},
	{ _GSC_REG(RECR),	0,	_recr_detail,	"Receive Event Counter Register"	},
	{ _GSC_REG(CIER),	0,	_cier_detail,	"COS Interrupt EnableRegister"		},
	{ _GSC_REG(CPR),	0,	_cpr_detail,	"COS Polarity Register"				},
	{ _GSC_REG(CDR),	0,	_cdr_detail,	"Clock Divider Register"			},
	{ _GSC_REG(ODR),	0,	NULL,			"Output Data Register"				},
	{ NULL, 0, 0, 0, 0,		NULL	}
};



//*****************************************************************************
static const gsc_reg_def_t* _find_reg(u32 reg, const gsc_reg_def_t* list)
{
	const gsc_reg_def_t*	def	= NULL;
	int						i;

	for (i = 0; list[i].name; i++)
	{
		if (reg == list[i].reg)
		{
			def	= &list[i];
			break;
		}
	}

	return(def);
}



/******************************************************************************
*
*	Function:	opto16x16_reg_get_def_id
*
*	Purpose:
*
*		Retrieve the register definition structure given the register id.
*
*	Arguments:
*
*		reg		The id of the register to access.
*
*	Returned:
*
*		NULL	The register id wasn't found.
*		else	A pointer to the register definition.
*
******************************************************************************/

const gsc_reg_def_t* opto16x16_reg_get_def_id(u32 reg)
{
	const gsc_reg_def_t*	def;

	def	= _find_reg(reg, _gsc);
	return(def);
}



/******************************************************************************
*
*	Function:	opto16x16_reg_get_def_index
*
*	Purpose:
*
*		Retrieve the register definition structure based on an index.
*
*	Arguments:
*
*		index	The index of the register to access.
*
*	Returned:
*
*		NULL	The index doesn't correspond to a known register.
*		else	A pointer to the register definition.
*
******************************************************************************/

const gsc_reg_def_t* opto16x16_reg_get_def_index(int index)
{
	const gsc_reg_def_t*	def;

	if (index < 0)
		def	= NULL;
	else if (index >= (SIZEOF_ARRAY(_gsc) - 1))
		def	= NULL;
	else
		def	= &_gsc[index];

	return(def);
}



/******************************************************************************
*
*	Function:	opto16x16_reg_get_desc
*
*	Purpose:
*
*		Retrieve the description of the specified register.
*
*	Arguments:
*
*		reg		The register whose description is desired.
*
*	Returned:
*
*		!NULL	The register's name.
*
******************************************************************************/

const char* opto16x16_reg_get_desc(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				desc;

	def	= _find_reg(reg, _gsc);

	if (def)
		desc	= def->desc;
	else
		desc	= "UNKNOWN";

	return(desc);
}



/******************************************************************************
*
*	Function:	opto16x16_reg_get_name
*
*	Purpose:
*
*		Retrieve the name of the specified register.
*
*	Arguments:
*
*		reg		The register whose name is desired.
*
*	Returned:
*
*		!NULL	The register's name.
*
******************************************************************************/

const char* opto16x16_reg_get_name(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				name;

	def	= _find_reg(reg, _gsc);

	if (def)
		name	= def->name;
	else
		name	= "UNKNOWN";

	return(name);
}



/******************************************************************************
*
*	Function:	opto16x16_reg_list
*
*	Purpose:
*
*		List the GSC registers and their values.
*
*	Arguments:
*
*		fd		The handle to access the device.
*
*		detail	List the register details?
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int opto16x16_reg_list(int fd, int detail)
{
	int	errs;

	errs	= gsc_reg_list(fd, _gsc, detail, opto16x16_reg_read);
	return(errs);
}



