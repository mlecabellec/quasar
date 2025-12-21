// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/pci_plx/gsc_util_reg_pex8112.c $
// $Rev: 50955 $
// $Date: 2022-04-06 06:42:06 -0500 (Wed, 06 Apr 2022) $

// PEX8112: OS & Device Independent: Utility: source file

#include "main.h"



// macros *********************************************************************

#define	_PCI_REG(a)	"PCI " #a, GSC_PCI_8112_##a, 0, 0, 0, NULL
#define	_PLX_REG(a)	"PLX " #a, GSC_PLX_8112_##a, 0, 0, 0, NULL



// variables ******************************************************************

static gsc_reg_def_t	_pci[]	=
{
	{ _PCI_REG(VIDR),		"Vendor ID Register"								},
	{ _PCI_REG(DIDR),		"Device ID Register"								},
	{ _PCI_REG(CR),			"Command Register"									},
	{ _PCI_REG(SR),			"Status Register"									},
	{ _PCI_REG(REV),		"Device Revision ID Register"						},
	{ _PCI_REG(CCR),		"Class Code Register"								},
	{ _PCI_REG(CLSR),		"Cache Line Size Register"							},
	{ _PCI_REG(BLTR),		"Bus Latency Timer Register"						},
	{ _PCI_REG(HTR),		"Header Type Register"								},
	{ _PCI_REG(BISTR),		"Built-In Self-Test Register"						},
	{ _PCI_REG(BAR0),		"Base Address 0 Register"							},
	{ _PCI_REG(BAR1),		"Base Address 1 Register"							},
	{ _PCI_REG(PRIBNR),		"Primary Bus Number Register"						},
	{ _PCI_REG(SECBNR),		"Secondary Bus Number Register"						},
	{ _PCI_REG(SUBBNR),		"Subordinate Bus Number Register"					},
	{ _PCI_REG(SECLTR),		"Secondary Latency Timer Register"					},
	{ _PCI_REG(IOBR),		"I/O Base Register"									},
	{ _PCI_REG(IOLR),		"I/O Limit Register"								},
	{ _PCI_REG(SECSR),		"Secondary Status Register"							},
	{ _PCI_REG(MBR),		"Memory Base Register"								},
	{ _PCI_REG(MLR),		"Memory Limit Register"								},
	{ _PCI_REG(PMBR),		"Prefetchable Memory Base Register"					},
	{ _PCI_REG(PMLR),		"Prefetchable Memory Limit Register"				},
	{ _PCI_REG(PMBUR),		"Prefetchable Memory Base Upper 32-bits Register"	},
	{ _PCI_REG(PMLUR),		"Prefetchable Memory Limit Upper 32-bits Register"	},
	{ _PCI_REG(IOBUR),		"I/O Base Upper 16-bits Register"					},
	{ _PCI_REG(IOLUR),		"I/O Limit Upper 16-bits Register"					},
	{ _PCI_REG(CPR),		"Capability Pointer Register"						},
	{ _PCI_REG(BAERR),		"Base Address for Expansion ROM Register"			},
	{ _PCI_REG(ILR),		"Interrupt Line Register"							},
	{ _PCI_REG(IPR),		"Interrupt Pin Register"							},
	{ _PCI_REG(BCR),		"Bridge Control Register"							},

	{ _PCI_REG(PMCIDR),		"Power Management Capability ID Register"			},
	{ _PCI_REG(PMNCPR),		"Power Management Next Capability Pointer Register"	},
	{ _PCI_REG(PMCR),		"Power Management Capability Register"				},
	{ _PCI_REG(PMCSR),		"Power Management Control/Status Register"			},
	{ _PCI_REG(PMBSR),		"Power Management Bridge Support Register"			},
	{ _PCI_REG(PMDR),		"Power Management Data Register"					},
	{ _PCI_REG(DSCR),		"Device-Specific Control Register"					},
	{ _PCI_REG(MCIDR),		"MSI Capability ID Register"						},
	{ _PCI_REG(MNCPR),		"MSI Next Capability Pointer Register"				},
	{ _PCI_REG(MCR),		"MSI Control Register"								},
	{ _PCI_REG(MAR),		"MSI Address Register"								},
	{ _PCI_REG(MUAR),		"MSI Upper Address Register"						},
	{ _PCI_REG(MDR),		"MSI Data Register"									},
	{ _PCI_REG(PECIDR),		"PCI Express Capability ID Register"				},
	{ _PCI_REG(PENCPR),		"PCI Express Next Capability Pointer Register"		},
	{ _PCI_REG(PECR),		"PCI Express Capability Register"					},
	{ _PCI_REG(DCR),		"Device Capability Register"						},
	{ _PCI_REG(PEDCR),		"PCI Express Device Control Register"				},
	{ _PCI_REG(PEDSR),		"PCI Express Device Status Register"				},
	{ _PCI_REG(LCAPR),		"Link Capability Register"							},
	{ _PCI_REG(LCTLR),		"Link Control Register"								},
	{ _PCI_REG(LSTSR),		"Link Status Register"								},
	{ _PCI_REG(SCAPR),		"Slot Capability Register"							},
	{ _PCI_REG(SCTLR),		"Slot Control Register"								},
	{ _PCI_REG(SSTSR),		"Slot Status Register"								},
	{ _PCI_REG(MCRIR),		"Main Control Register Index Register"				},
	{ _PCI_REG(MCRDR),		"Main Control Register Data Register"				},

	{ _PCI_REG(PBECHR),		"Power Budget Enhanced Capability Header Register"	},
	{ _PCI_REG(PBDSR),		"Power Budget Data Select Register"					},
	{ _PCI_REG(PBDR),		"Power Budget Data Register"						},
	{ _PCI_REG(PBCR),		"Power Budget Capability Register"					},
	{ _PCI_REG(SNPEECIDR),	"Serial Number Enhanced Capability Header Register"	},
	{ _PCI_REG(SNLR),		"Serial Number Low Register"						},
	{ _PCI_REG(SNHR),		"Serial Number Hi Register"							},

	{ NULL	}
};

static gsc_reg_def_t	_plx[]	=
{
	{ _PLX_REG(DIR),		"Device Initialization Register"				},
	{ _PLX_REG(SECR),		"Serial EEPROM Control Register"				},
	{ _PLX_REG(SECFR),		"Serial EEPROM Clock Frequency Register"		},
	{ _PLX_REG(PCR),		"PCI Control Register"							},
	{ _PLX_REG(PEIRER),		"PCI Express Interrupt Request Enable Register"	},
	{ _PLX_REG(IRER),		"Interrupt Request Enable Register"				},
	{ _PLX_REG(IRSR),		"Interrupt Request Status Register"				},
	{ _PLX_REG(PR),			"Power Register"								},
	{ _PLX_REG(GPIOCR),		"General Purpose I/O Control Register"			},
	{ _PLX_REG(GPIOSR),		"General Purpose I/O Status Register"			},
	{ _PLX_REG(M0R),		"Mailbox 0 Register"							},
	{ _PLX_REG(M1R),		"Mailbox 1 Register"							},
	{ _PLX_REG(M2R),		"Mailbox 2 Register"							},
	{ _PLX_REG(M3R),		"Mailbox 3 Register"							},
	{ _PLX_REG(CSRR),		"Chip Silicon Revision Register"				},
	{ _PLX_REG(DCR),		"Diagnostics Control Register"					},
	{ _PLX_REG(TCC0R),		"TLP Controller Configuration 0 Register"		},
	{ _PLX_REG(TCC1R),		"TLP Controller Configuration 1 Register"		},
	{ _PLX_REG(TCC2R),		"TLP Controller Configuration 2 Register"		},
	{ _PLX_REG(TCTR),		"TLP Controller Tag Register"					},
	{ _PLX_REG(TCTL0R),		"TLP Controller Time Limit 0 Register"			},
	{ _PLX_REG(TCTL1R),		"TLP Controller Time Limit 1 Register"			},
	{ _PLX_REG(CTR),		"CSR Timer Register"							},
	{ _PLX_REG(ECAR),		"Enhanced Configuration Address Register"		},

	{ NULL	}
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


//*****************************************************************************
int gsc_reg_pex8112_list_pci(int fd, int (func)(int fd, int index, int verbose, u32 reg, u32* value))
{
	int	errs;

	gsc_label("PEX8112 PCI Registers");
	printf("\n");

	errs	= gsc_reg_list(fd, _pci, 0, func);

	return(errs);
}



//*****************************************************************************
int gsc_reg_pex8112_list_plx(int fd, int (func)(int fd, int index, int verbose, u32 reg, u32* value))
{
	int	errs;

	gsc_label("PEX8112 PLX Registers");
	printf("\n");

	errs	= gsc_reg_list(fd, _plx, 0, func);

	return(errs);
}



//*****************************************************************************
const char* gsc_reg_pex8112_get_desc(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				desc;

	def	= _find_reg(reg, _pci);

	if (def == NULL)
		def	= _find_reg(reg, _plx);

	if (def)
		desc	= def->desc;
	else
		desc	= "UNKNOWN";

	return(desc);
}



//*****************************************************************************
const char* gsc_reg_pex8112_get_name(u32 reg)
{
	const gsc_reg_def_t*	def;
	const char*				name;

	def	= _find_reg(reg, _pci);

	if (def == NULL)
		def	= _find_reg(reg, _plx);

	if (def)
		name	= def->name;
	else
		name	= "UNKNOWN";

	return(name);
}



//*****************************************************************************
const gsc_reg_def_t* gsc_reg_pex8112_get_def_id(int reg)
{
	const gsc_reg_def_t*	def;

	def	= _find_reg(reg, _pci);

	if (def == NULL)
		def	= _find_reg(reg, _plx);

	return(def);
}


