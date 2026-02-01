// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_reg.c $
// $Rev: 53557 $
// $Date: 2023-08-07 14:30:23 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Utilities: source file

#include "main.h"



// macros *********************************************************************

#define	_GSC_REG(a)				"GSC " #a, AI64SSA_GSC_##a, 0, 0
#define	SHOW_FIELD(b,e,eol,l,d)	gsc_reg_field_show(width+10,21, value,(b),(e),(eol),(l),(d))



//*****************************************************************************
static void _bctlr_9080_decode(int fd, u32 value, int width)
{
	static const char*	aim[]	=
	{
		"System",
		"Reserved",
		"Zero Test",
		"+Vref Test",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};

	static const char*	range[]	=
	{
		"+-2.5 Volts",
		"+-5 Volts",
		"+-10 Volts",
		"+-10 Volts"
	};

	static const char*	single[]	=
	{
		"Single Ended",
		"Pseudo-Differential",
		"Full Differential",
		"Reserved"
	};

	static const char*	disabled[]	= { "Disabled",			"Enabled"		};
	static const char*	enabled[]	= { "Enabled",			"Disabled"		};
	static const char*	fail[]		= { "Fail",				"Pass"			};
	static const char*	idle[]		= { "Idle",				"Active"		};
	static const char*	intern[]	= { "Internal",			"External"		};
	static const char*	no[]		= { "No",				"Yes"			};
	static const char*	twos[]		= { "Twos Compliment",	"Offset Binary"	};
	static const char*	unipolar[]	= { "No, Bipolar",		"Yes, Unipolar"	};

	SHOW_FIELD( 2,  0, 1, aim,		"Input Mode"		);
	SHOW_FIELD( 3,  3, 1, unipolar,	"Unipolar Input"	);
	SHOW_FIELD( 5,  4, 1, range,	"Range"				);
	SHOW_FIELD( 6,  6, 1, twos,		"Data Format"		);
	SHOW_FIELD( 7,  7, 1, intern,	"Sync Mode"			);
	SHOW_FIELD( 9,  8, 1, single,	"Differential Mode"	);
	SHOW_FIELD(11, 10, 1, NULL,		"Reserved"			);
	SHOW_FIELD(12, 12, 1, idle,		"Input Sync"		);
	SHOW_FIELD(13, 13, 1, idle,		"Autocalibrate"		);
	SHOW_FIELD(14, 14, 1, fail,		"Autocal Status"	);
	SHOW_FIELD(15, 15, 1, idle,		"Initialize"		);
	SHOW_FIELD(16, 16, 1, no,		"Buffer Underflow"	);
	SHOW_FIELD(17, 17, 1, no,		"Buffer Overflow"	);
	SHOW_FIELD(18, 18, 1, disabled,	"Scan Marker"		);
	SHOW_FIELD(19, 19, 1, enabled,	"DMDMA"				);
	SHOW_FIELD(31, 20, 1, NULL,		"Reserved"			);
}



//*****************************************************************************
static void _bctlr_9056_decode(int fd, u32 value, int width)
{
	static const char*	aim[]	=
	{
		"System",
		"Reserved",
		"Zero Test",
		"+Vref Test",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};

	static const char*	range[]	=
	{
		"+-2.5 Volts",
		"+-5 Volts",
		"+-10 Volts",
		"+-10 Volts"
	};

	static const char*	single[]	=
	{
		"Single Ended",
		"Pseudo-Differential",
		"Full Differential",
		"Reserved"
	};

	static const char*	disabled[]	= { "Disabled",			"Enabled"		};
	static const char*	enabled[]	= { "Enabled",			"Disabled"		};
	static const char*	fail[]		= { "Fail",				"Pass"			};
	static const char*	idle[]		= { "Idle",				"Active"		};
	static const char*	intern[]	= { "Internal",			"External"		};
	static const char*	no[]		= { "No",				"Yes"			};
	static const char*	thresh[]	= { "Threshold Based",	"Autonomous"	};
	static const char*	twos[]		= { "Twos Compliment",	"Offset Binary"	};
	static const char*	unipolar[]	= { "No, Bipolar",		"Yes, Unipolar"	};

	SHOW_FIELD( 2,  0, 1, aim,		"Input Mode"		);
	SHOW_FIELD( 3,  3, 1, unipolar,	"Unipolar Input"	);
	SHOW_FIELD( 5,  4, 1, range,	"Range"				);
	SHOW_FIELD( 6,  6, 1, twos,		"Data Format"		);
	SHOW_FIELD( 7,  7, 1, intern,	"Sync Mode"			);
	SHOW_FIELD( 9,  8, 1, single,	"Differential Mode"	);
	SHOW_FIELD(10, 10, 1, no,		"Data On Hold"		);
	SHOW_FIELD(11, 11, 1, enabled,	"Scan Marker"		);
	SHOW_FIELD(12, 12, 1, idle,		"Input Sync"		);
	SHOW_FIELD(13, 13, 1, idle,		"Autocalibrate"		);
	SHOW_FIELD(14, 14, 1, fail,		"Autocal Status"	);
	SHOW_FIELD(15, 15, 1, idle,		"Initialize"		);
	SHOW_FIELD(16, 16, 1, no,		"Buffer Underflow"	);
	SHOW_FIELD(17, 17, 1, no,		"Buffer Overflow"	);
	SHOW_FIELD(18, 18, 1, disabled,	"Data Packing"		);
	SHOW_FIELD(19, 19, 1, thresh,	"DMDMA Mode"		);
	SHOW_FIELD(31, 20, 1, NULL,		"Reserved"			);
}



//*****************************************************************************
static int _bctlr_decode(int fd, int supported, u32 value, int width)
{
	u32	did;
	int	errs;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &did);

	if (did == 0x9080)
		_bctlr_9080_decode(fd, value, width);
	else
		_bctlr_9056_decode(fd, value, width);

	return(errs);
}



//*****************************************************************************
static int _icr_decode(int fd, int supported, u32 value, int width)
{
	static const char*	irq0[]	=
	{
		"Initialization Complete",
		"Autocalibration Completed",
		"Input Sample Initiated (Sync)",
		"Input Sample Completed (data ready)",
		"Trigger Burst Initiated (Burst Busy -> High",
		"Trigger Burst Completed (Burst Busy -> Low",
		"Reserved",
		"Reserved"
	};

	static const char*	irq1_a[]	=
	{
		"Idle/None",
		"Input Buffer Threshold Low->High",
		"Input Buffer Threshold High->Low",
		"Input Buffer Error",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};

	static const char*	irq1_b[]	=
	{
		"Idle/None",
		"Input Buffer Threshold Low->High",
		"Input Buffer Threshold High->Low",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"
	};

	static const char*	negated[]	= { "Negated",	"Asserted"	};

	int				errs;
	s32				irq;
	const char**	irq1;

	errs	= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_IRQ1_BUF_ERROR, &irq);
	irq1	= irq ? irq1_b : irq1_a;

	SHOW_FIELD( 2,  0, 1, irq0,		"IRQ0 Select"	);
	SHOW_FIELD( 3,  3, 1, negated,	"IRQ0 Request"	);
	SHOW_FIELD( 6,  4, 1, irq1,		"IRQ1 Select"	);
	SHOW_FIELD( 7,  7, 1, negated,	"IRQ1 Request"	);
	SHOW_FIELD(31,  8, 1, NULL,		"Reserved"		);
	return(errs);
}



//*****************************************************************************
static int _idbr_decode(int fd, int supported, u32 value, int width)
{
	static const char*	tag[]	= { "Not First",	"First"	};

	SHOW_FIELD( 15,  0, 1, NULL,	"Data"			);
	SHOW_FIELD( 16, 16, 1, tag,		"Channel Tag"	);
	SHOW_FIELD( 31, 17, 1, NULL,	"Reserved"		);
	return(0);
}



//*****************************************************************************
static void _ibcr_9080_decode(int fd, u32 value, int width)
{
	static const char*	_1x[]		= { "1x",		"4x"		};
	static const char*	clear[]		= { "Clear",	"Set"		};
	static const char*	enable[]	= { "Enabled",	"Disabled"	};
	static const char*	idle[]		= { "Idle",		"Active"	};

	SHOW_FIELD( 15,  0, 1, NULL,	"Threshold"			);
	SHOW_FIELD( 16, 16, 1, idle,	"Clear Buffer"		);
	SHOW_FIELD( 17, 17, 1, clear,	"Threshold Flag"	);
	SHOW_FIELD( 18, 18, 1, enable,	"Buffer Disable"	);
	SHOW_FIELD( 19, 19, 1, _1x,		"Threshold 4x"		);
	SHOW_FIELD( 31, 20, 1, NULL,	"Reserved"			);
}



//*****************************************************************************
static void _ibcr_9056_decode(int fd, u32 value, int width)
{
	static const char*	clear[]	= { "Clear",	"Set"		};
	static const char*	idle[]	= { "Idle",		"Active"	};

	SHOW_FIELD( 17,  0, 1, NULL,	"Threshold"			);
	SHOW_FIELD( 18, 18, 1, idle,	"Clear Buffer"		);
	SHOW_FIELD( 19, 19, 1, clear,	"Threshold Flag"	);
	SHOW_FIELD( 31, 20, 1, NULL,	"Reserved"			);
}



//*****************************************************************************
static int _ibcr_decode(int fd, int supported, u32 value, int width)
{
	int	errs;
	u32	did;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &did);

	if (did == 0x9080)
		_ibcr_9080_decode(fd, value, width);
	else
		_ibcr_9056_decode(fd, value, width);

	return(errs);
}



//*****************************************************************************
static int _rgr_decode(int fd, int supported, u32 value, int width)
{
	static const char*	enabled[]	= { "Enabled",	"Disabled"	};

	SHOW_FIELD( 15,  0, 1, NULL,	"Nrate"		);
	SHOW_FIELD( 16, 16, 1, enabled,	"Disable"	);
	SHOW_FIELD( 31, 17, 1, NULL,	"Reserved"	);
	return(0);
}



//*****************************************************************************
static int _bufsr_9080_decode(int fd, u32 value, int width)
{
	SHOW_FIELD( 17,  0, 1, NULL,	"Filled Locations"	);
	SHOW_FIELD( 31, 18, 1, NULL,	"Reserved"			);
	return(0);
}



//*****************************************************************************
static int _bufsr_9056_decode(int fd, u32 value, int width)
{
	SHOW_FIELD( 18,  0, 1, NULL,	"Filled Locations"	);
	SHOW_FIELD( 31, 19, 1, NULL,	"Reserved"			);
	return(0);
}



//*****************************************************************************
static int _bufsr_decode(int fd, int supported, u32 value, int width)
{
	u32	did;
	int	errs;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &did);

	if (did == 0x9080)
		_bufsr_9080_decode(fd, value, width);
	else
		_bufsr_9056_decode(fd, value, width);

	return(errs);
}



//*****************************************************************************
static int _bursr_decode(int fd, int supported, u32 value, int width)
{
	SHOW_FIELD( 19,  0, 1, NULL,	"Burst Size"	);
	SHOW_FIELD( 31, 20, 1, NULL,	"Reserved"		);
	return(0);
}



//*****************************************************************************
static int _sscr_decode(int fd, int supported, u32 value, int width)
{
	static const char*	burst[]	=
	{
		"Disabled",
		"Rate-B Generator",
		"Ext Sync Input (target Mode)",
		"Software (BCTLR Input Sync Bit)"
	};

	static const char*	clk_src[]	=
	{
		"Rate-A Generator",
		"Rate-B Generator",
		"External Clock (clock Target mode)",
		"Software (BCTLR Input Sync Bit)"
	};

	static const char*	channels[]	=
	{
		"Single Channel Mode",
		"2 Channels (0-1)",
		"4 Channels (0-3)",
		"8 Channels (0-7)",
		"16 Channels (0-15)",
		"32 Channels (0-31)",
		"64 Channels (0-63)",
		"Channel Range (x-y)"
	};

	static const char*	b_src[]		= { "Master Clock",	"Rate-A Generator"	};
	static const char*	idle[]		= { "Idle",			"Active"			};

	SHOW_FIELD( 2,  0, 1, channels,	"Active Channel"		);
	SHOW_FIELD( 4,  3, 1, clk_src,	"Sample Clock Source"	);
	SHOW_FIELD( 6,  5, 1, NULL,		"Reserved"				);
	SHOW_FIELD( 7,  7, 1, idle,		"Bursting Status"		);
	SHOW_FIELD( 9,  8, 1, burst,	"Bursting Source"		);
	SHOW_FIELD(10, 10, 1, b_src,	"Rate-B Gen. Source"	);
	SHOW_FIELD(11, 11, 1, NULL,		"Reserved"				);
	SHOW_FIELD(17, 12, 1, NULL,		"Single Channel"		);
	SHOW_FIELD(31, 18, 1, NULL,		"Reserved"				);
	return(0);
}



//*****************************************************************************
static int _acar_decode(int fd, int supported, u32 value, int width)
{
	SHOW_FIELD( 7,  0, 1, NULL,	"First Channel"	);
	SHOW_FIELD(15,  8, 1, NULL,	"Last Channel"	);
	SHOW_FIELD(31, 16, 1, NULL,	"Reserved"		);
	return(0);
}



//*****************************************************************************
static int _bcfgr_9080_decode(int fd, u32 value, int width)
{
	static const char*	_64k[]		= { "64K",				"256K"			};
	static const char*	channels[]	= { "64 Channels",		"32 Channels"	};

	int	errs;
	u32	pci9080;
	u32	v;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &pci9080);

	SHOW_FIELD( 11,  0, 0, NULL,	"Firmware Rev"	);
	v	= value & 0xFFF;
	printf("%03lX\n", (long) v);

	SHOW_FIELD(15, 12, 1, NULL,		"Reserved"		);
	SHOW_FIELD(16, 16, 1, channels,	"Channels"		);
	SHOW_FIELD(17, 17, 1, NULL,		"Reserved"		);
	SHOW_FIELD(18, 18, 1, _64k,		"FIFO Size"		);
	SHOW_FIELD(31, 19, 1, NULL,		"Reserved"		);
	return(errs);
}



//*****************************************************************************
static int _bcfgr_9056_decode(int fd, u32 value, int width)
{
	static const char*	master[]	=
	{
		"50.000 MHz",
		"45.000 MHz",
		"49.152 MHz",
		"51.840 MHz"
	};

	static const char*	channels[]	= { "64 Channels",		"32 Channels"	};
	static const char*	not[]		= { "Not Supported",	"Supported"		};

	int	errs;
	u32	pci9080;
	u32	v;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &pci9080);

	SHOW_FIELD( 11,  0, 0, NULL,	"Firmware Rev"	);
	v	= value & 0xFFF;
	printf("%03lX\n", (long) v);

	SHOW_FIELD(14, 12, 1, NULL,		"Reserved"		);
	SHOW_FIELD(15, 15, 1, not,		"Low Latency"	);
	SHOW_FIELD(16, 16, 1, channels,	"Channels"		);
	SHOW_FIELD(18, 17, 1, master,	"Master Clock"	);
	SHOW_FIELD(31, 19, 1, NULL,		"Reserved"		);
	return(errs);
}



//*****************************************************************************
static int _bcfgr_decode(int fd, int supported, u32 value, int width)
{
	u32	did;
	int	errs;

	errs	= ai64ssa_reg_read(fd, -1, 0, GSC_PCI_9080_DIDR, &did);

	if (did == 0x9080)
		errs	+= _bcfgr_9080_decode(fd, value, width);
	else
		errs	+= _bcfgr_9056_decode(fd, value, width);

	return(errs);
}



//*****************************************************************************
static int _asiocr_decode(int fd, int supported, u32 value, int width)
{
	static const char*	aux[]	=
	{
		"Inactive",
		"Active Input (Low-to-High Edge)",
		"Active Output (Positive Pulse)",
		"Reserved"
	};

	static const char*	noise[]	= { "100ns-135ns",		"1.5us-2.0us"		};
	static const char*	high[]	= { "High Pulses",		"Low Pulses"		};
	static const char*	low[]	= { "Low-to-High Edge",	"High-to-Low Edge"	};

	SHOW_FIELD( 1,  0, 1, aux,		"Aux 0 Mode"		);
	SHOW_FIELD( 3,  2, 1, aux,		"Aux 1 Mode"		);
	SHOW_FIELD( 5,  4, 1, aux,		"Aux 2 Mode"		);
	SHOW_FIELD( 7,  6, 1, aux,		"Aux 3 Mode"		);
	SHOW_FIELD( 8,  8, 1, low,		"Input Polarity"	);
	SHOW_FIELD( 9,  9, 1, high,		"Output Polarity"	);
	SHOW_FIELD(10, 10, 1, noise,	"Noise Supression"	);
	SHOW_FIELD(31, 11, 1, NULL,		"Reserved"			);
	return(0);
}



//*****************************************************************************
static int _smwr_decode(int fd, int supported, u32 value, int width)
{
	SHOW_FIELD( 15,  0, 1, NULL,	"Marker"	);
	SHOW_FIELD( 31, 16, 1, NULL,	"Reserved"	);
	return(0);
}



//*****************************************************************************
static int _llhr_decode(int fd, int supported, u32 value, int width)
{
	SHOW_FIELD( 15,  0, 1, NULL,	"Value"	);
	return(0);
}



//*****************************************************************************
static int _llcr_decode(int fd, int supported, u32 value, int width)
{
	s32	llcr;
	int	ret		= 0;

	if (supported)
	{
		ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_REG_LLCR, &llcr);
		ret	= llcr;
	}
	else
	{
		SHOW_FIELD( 5,  0, 1, NULL,	"HOLD Channel"		);
		SHOW_FIELD(11,  6, 1, NULL,	"RELEASE Channel"	);
		SHOW_FIELD(31, 12, 1, NULL,	"Reserved"			);
	}

	return(ret);
}



// variables ******************************************************************

static gsc_reg_def_t	_gsc[]	=
{
	{ _GSC_REG(BCTLR),		0,	_bctlr_decode,	"Board Control Register"				},
	{ _GSC_REG(ICR),		0,	_icr_decode,	"Interrupt Control Register"			},
	{ _GSC_REG(IDBR),		0,	_idbr_decode,	"Input Data Buffer Register"			},
	{ _GSC_REG(IBCR),		0,	_ibcr_decode,	"Input Buffer Control Register"			},
	{ _GSC_REG(RAGR),		0,	_rgr_decode,	"Rate-A Generator Register"				},
	{ _GSC_REG(RBGR),		0,	_rgr_decode,	"Rate-B Generator Register"				},
	{ _GSC_REG(BUFSR),		0,	_bufsr_decode,	"Buffer Size Register"					},
	{ _GSC_REG(BURSR),		0,	_bursr_decode,	"Burst Size Register"					},
	{ _GSC_REG(SSCR),		0,	_sscr_decode,	"Scan & Sync Control Register"			},
	{ _GSC_REG(ACAR),		0,	_acar_decode,	"Active Channel Assignment Register"	},
	{ _GSC_REG(BCFGR),		0,	_bcfgr_decode,	"Board Configuration Register"			},
	{ _GSC_REG(AVR),		0,	NULL,			"Autocal Values Register"				},
	{ _GSC_REG(ARWR),		0,	NULL,			"Auxiliary R/W Register"				},
	{ _GSC_REG(ASIOCR),		0,	_asiocr_decode,	"Auxiliary Sync I/O Control Register"	},
	{ _GSC_REG(SMUWR),		0,	_smwr_decode,	"Scan Marker Upper Word Register"		},
	{ _GSC_REG(SMLWR),		0,	_smwr_decode,	"Scan Marker Lower Word Register"		},
	{ _GSC_REG(LLCR),		1,	_llcr_decode,	"Low Latency Control Register"			},

	{ NULL, 0, 0, 0, 0,		NULL	}
};

static gsc_reg_def_t	_gsc_ll[]	=
{
	{ _GSC_REG(LLHR00),		0,	_llhr_decode,	"Low Latency Holding Register 00"	},
	{ _GSC_REG(LLHR01),		0,	_llhr_decode,	"Low Latency Holding Register 01"	},
	{ _GSC_REG(LLHR02),		0,	_llhr_decode,	"Low Latency Holding Register 02"	},
	{ _GSC_REG(LLHR03),		0,	_llhr_decode,	"Low Latency Holding Register 03"	},
	{ _GSC_REG(LLHR04),		0,	_llhr_decode,	"Low Latency Holding Register 04"	},
	{ _GSC_REG(LLHR05),		0,	_llhr_decode,	"Low Latency Holding Register 05"	},
	{ _GSC_REG(LLHR06),		0,	_llhr_decode,	"Low Latency Holding Register 06"	},
	{ _GSC_REG(LLHR07),		0,	_llhr_decode,	"Low Latency Holding Register 07"	},
	{ _GSC_REG(LLHR08),		0,	_llhr_decode,	"Low Latency Holding Register 08"	},
	{ _GSC_REG(LLHR09),		0,	_llhr_decode,	"Low Latency Holding Register 09"	},
	{ _GSC_REG(LLHR10),		0,	_llhr_decode,	"Low Latency Holding Register 10"	},
	{ _GSC_REG(LLHR11),		0,	_llhr_decode,	"Low Latency Holding Register 11"	},
	{ _GSC_REG(LLHR12),		0,	_llhr_decode,	"Low Latency Holding Register 12"	},
	{ _GSC_REG(LLHR13),		0,	_llhr_decode,	"Low Latency Holding Register 13"	},
	{ _GSC_REG(LLHR14),		0,	_llhr_decode,	"Low Latency Holding Register 14"	},
	{ _GSC_REG(LLHR15),		0,	_llhr_decode,	"Low Latency Holding Register 15"	},
	{ _GSC_REG(LLHR16),		0,	_llhr_decode,	"Low Latency Holding Register 16"	},
	{ _GSC_REG(LLHR17),		0,	_llhr_decode,	"Low Latency Holding Register 17"	},
	{ _GSC_REG(LLHR18),		0,	_llhr_decode,	"Low Latency Holding Register 18"	},
	{ _GSC_REG(LLHR19),		0,	_llhr_decode,	"Low Latency Holding Register 19"	},
	{ _GSC_REG(LLHR20),		0,	_llhr_decode,	"Low Latency Holding Register 20"	},
	{ _GSC_REG(LLHR21),		0,	_llhr_decode,	"Low Latency Holding Register 21"	},
	{ _GSC_REG(LLHR22),		0,	_llhr_decode,	"Low Latency Holding Register 22"	},
	{ _GSC_REG(LLHR23),		0,	_llhr_decode,	"Low Latency Holding Register 23"	},
	{ _GSC_REG(LLHR24),		0,	_llhr_decode,	"Low Latency Holding Register 24"	},
	{ _GSC_REG(LLHR25),		0,	_llhr_decode,	"Low Latency Holding Register 25"	},
	{ _GSC_REG(LLHR26),		0,	_llhr_decode,	"Low Latency Holding Register 26"	},
	{ _GSC_REG(LLHR27),		0,	_llhr_decode,	"Low Latency Holding Register 27"	},
	{ _GSC_REG(LLHR28),		0,	_llhr_decode,	"Low Latency Holding Register 28"	},
	{ _GSC_REG(LLHR29),		0,	_llhr_decode,	"Low Latency Holding Register 29"	},
	{ _GSC_REG(LLHR30),		0,	_llhr_decode,	"Low Latency Holding Register 30"	},
	{ _GSC_REG(LLHR31),		0,	_llhr_decode,	"Low Latency Holding Register 31"	},
	{ _GSC_REG(LLHR32),		0,	_llhr_decode,	"Low Latency Holding Register 32"	},
	{ _GSC_REG(LLHR33),		0,	_llhr_decode,	"Low Latency Holding Register 33"	},
	{ _GSC_REG(LLHR34),		0,	_llhr_decode,	"Low Latency Holding Register 34"	},
	{ _GSC_REG(LLHR35),		0,	_llhr_decode,	"Low Latency Holding Register 35"	},
	{ _GSC_REG(LLHR36),		0,	_llhr_decode,	"Low Latency Holding Register 36"	},
	{ _GSC_REG(LLHR37),		0,	_llhr_decode,	"Low Latency Holding Register 37"	},
	{ _GSC_REG(LLHR38),		0,	_llhr_decode,	"Low Latency Holding Register 38"	},
	{ _GSC_REG(LLHR39),		0,	_llhr_decode,	"Low Latency Holding Register 39"	},
	{ _GSC_REG(LLHR40),		0,	_llhr_decode,	"Low Latency Holding Register 40"	},
	{ _GSC_REG(LLHR41),		0,	_llhr_decode,	"Low Latency Holding Register 41"	},
	{ _GSC_REG(LLHR42),		0,	_llhr_decode,	"Low Latency Holding Register 42"	},
	{ _GSC_REG(LLHR43),		0,	_llhr_decode,	"Low Latency Holding Register 43"	},
	{ _GSC_REG(LLHR44),		0,	_llhr_decode,	"Low Latency Holding Register 44"	},
	{ _GSC_REG(LLHR45),		0,	_llhr_decode,	"Low Latency Holding Register 45"	},
	{ _GSC_REG(LLHR46),		0,	_llhr_decode,	"Low Latency Holding Register 46"	},
	{ _GSC_REG(LLHR47),		0,	_llhr_decode,	"Low Latency Holding Register 47"	},
	{ _GSC_REG(LLHR48),		0,	_llhr_decode,	"Low Latency Holding Register 48"	},
	{ _GSC_REG(LLHR49),		0,	_llhr_decode,	"Low Latency Holding Register 49"	},
	{ _GSC_REG(LLHR50),		0,	_llhr_decode,	"Low Latency Holding Register 50"	},
	{ _GSC_REG(LLHR51),		0,	_llhr_decode,	"Low Latency Holding Register 51"	},
	{ _GSC_REG(LLHR52),		0,	_llhr_decode,	"Low Latency Holding Register 52"	},
	{ _GSC_REG(LLHR53),		0,	_llhr_decode,	"Low Latency Holding Register 53"	},
	{ _GSC_REG(LLHR54),		0,	_llhr_decode,	"Low Latency Holding Register 54"	},
	{ _GSC_REG(LLHR55),		0,	_llhr_decode,	"Low Latency Holding Register 55"	},
	{ _GSC_REG(LLHR56),		0,	_llhr_decode,	"Low Latency Holding Register 56"	},
	{ _GSC_REG(LLHR57),		0,	_llhr_decode,	"Low Latency Holding Register 57"	},
	{ _GSC_REG(LLHR58),		0,	_llhr_decode,	"Low Latency Holding Register 58"	},
	{ _GSC_REG(LLHR59),		0,	_llhr_decode,	"Low Latency Holding Register 59"	},
	{ _GSC_REG(LLHR60),		0,	_llhr_decode,	"Low Latency Holding Register 60"	},
	{ _GSC_REG(LLHR61),		0,	_llhr_decode,	"Low Latency Holding Register 61"	},
	{ _GSC_REG(LLHR62),		0,	_llhr_decode,	"Low Latency Holding Register 62"	},
	{ _GSC_REG(LLHR63),		0,	_llhr_decode,	"Low Latency Holding Register 63"	},

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
*	Function:	ai64ssa_reg_get_def_id
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

const gsc_reg_def_t* ai64ssa_reg_get_def_id(u32 reg)
{
	const gsc_reg_def_t*	def;

	def	= _find_reg(reg, _gsc);

	if (def == NULL)
		def	= _find_reg(reg, _gsc_ll);

	return(def);
}



/******************************************************************************
*
*	Function:	ai64ssa_reg_get_def_index
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

const gsc_reg_def_t* ai64ssa_reg_get_def_index(int index)
{
	const gsc_reg_def_t*	def;

	if (index < 0)
		def	= NULL;
	else if (index >= (SIZEOF_ARRAY(_gsc) + SIZEOF_ARRAY(_gsc_ll) - 2))
		def	= NULL;
	else if (index < (SIZEOF_ARRAY(_gsc) - 1))
		def	= &_gsc[index];
	else
		def	= &_gsc_ll[index - (SIZEOF_ARRAY(_gsc) - 1)];

	return(def);
}



/******************************************************************************
*
*	Function:	ai64ssa_reg_get_desc
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

const char* ai64ssa_reg_get_desc(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				desc;

	def	= _find_reg(reg, _gsc);

	if (def == NULL)
		def	= _find_reg(reg, _gsc_ll);

	if (def)
		desc	= def->desc;
	else
		desc	= "UNKNOWN";

	return(desc);
}



/******************************************************************************
*
*	Function:	ai64ssa_reg_get_name
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

const char* ai64ssa_reg_get_name(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				name;

	def	= _find_reg(reg, _gsc);

	if (def == NULL)
		def	= _find_reg(reg, _gsc_ll);

	if (def)
		name	= def->name;
	else
		name	= "UNKNOWN";

	return(name);
}



/******************************************************************************
*
*	Function:	ai64ssa_reg_list
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

int ai64ssa_reg_list(int fd, int detail)
{
	int	errs	= 0;
	s32	ll;

	errs	+= ai64ssa_query(fd, -1, 0, AI64SSA_QUERY_LOW_LATENCY, &ll);
	errs	+= gsc_reg_list(fd, _gsc, detail, ai64ssa_reg_read);

	if (ll)
	{
		printf("\n");
		errs	+= gsc_reg_list(fd, _gsc_ll, detail, ai64ssa_reg_read);
	}

	return(errs);
}



