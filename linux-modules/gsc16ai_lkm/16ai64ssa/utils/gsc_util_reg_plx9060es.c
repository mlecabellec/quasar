// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/pci_plx/gsc_util_reg_plx9060es.c $
// $Rev: 50955 $
// $Date: 2022-04-06 06:42:06 -0500 (Wed, 06 Apr 2022) $

// PCI9060ES: OS & Device Independent: Utility: source file

#include "main.h"



// macros *********************************************************************

#define	_PCI_REG(a)	"PCI " #a, GSC_PCI_9060ES_##a, 0, 0, 0
#define	_PLX_REG(a)	"PLX " #a, GSC_PLX_9060ES_##a, 0, 0, 0



// variables ******************************************************************

static gsc_reg_def_t	_pci[]	=
{
	{ _PCI_REG(BAR0),		NULL	},
	{ _PCI_REG(BAR1),		NULL	},
	{ _PCI_REG(BAR2),		NULL	},
	{ _PCI_REG(BAR3),		NULL	},
	{ _PCI_REG(BAR4),		NULL	},
	{ _PCI_REG(BAR5),		NULL	},
	{ _PCI_REG(BISTR),		NULL	},
	{ _PCI_REG(CCR),		NULL	},
	{ _PCI_REG(CLSR),		NULL	},
	{ _PCI_REG(CR),			NULL	},
	{ _PCI_REG(DIDR),		NULL	},
	{ _PCI_REG(ERBAR),		NULL	},
	{ _PCI_REG(HTR),		NULL	},
	{ _PCI_REG(ILR),		NULL	},
	{ _PCI_REG(IPR),		NULL	},
	{ _PCI_REG(LTR),		NULL	},
	{ _PCI_REG(MGR),		NULL	},
	{ _PCI_REG(MLR),		NULL	},
	{ _PCI_REG(REV),		NULL	},
	{ _PCI_REG(SID),		NULL	},
	{ _PCI_REG(SR),			NULL	},
	{ _PCI_REG(SVID),		NULL	},
	{ _PCI_REG(VIDR),		NULL	},
	{ NULL	}
};

static gsc_reg_def_t	_plx[]	=
{
	{ _PLX_REG(BIGEND),		NULL	},
	{ _PLX_REG(CNTRL),		NULL	},
	{ _PLX_REG(DIDR),		NULL	},
	{ _PLX_REG(DMCFGA),		NULL	},
	{ _PLX_REG(DMLBAI),		NULL	},
	{ _PLX_REG(DMLBAM),		NULL	},
	{ _PLX_REG(DMPBAM),		NULL	},
	{ _PLX_REG(DMRR),		NULL	},
	{ _PLX_REG(EROMBA),		NULL	},
	{ _PLX_REG(EROMRR),		NULL	},
	{ _PLX_REG(INTCSR),		NULL	},
	{ _PLX_REG(L2PDBELL),	NULL	},
	{ _PLX_REG(LARBR),		NULL	},
	{ _PLX_REG(LAS0BA),		NULL	},
	{ _PLX_REG(LAS0RR),		NULL	},
	{ _PLX_REG(LBRD0),		NULL	},
	{ _PLX_REG(MBOX0),		NULL	},
	{ _PLX_REG(MBOX1),		NULL	},
	{ _PLX_REG(MBOX2),		NULL	},
	{ _PLX_REG(MBOX3),		NULL	},
	{ _PLX_REG(P2LDBELL),	NULL	},
	{ _PLX_REG(VIDR),		NULL	},
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
int gsc_reg_plx9060es_list_pci(int fd, int (func)(int fd, int index, int verbose, u32 reg, u32* value))
{
	int	errs;

	gsc_label("PCI9060ES PCI Registers");
	printf("\n");

	errs	= gsc_reg_list(fd, _pci, 0, func);

	return(errs);
}



//*****************************************************************************
int gsc_reg_plx9060es_list_plx(int fd, int (func)(int fd, int index, int verbose, u32 reg, u32* value))
{
	int	errs;

	gsc_label("PCI9060ES PLX Registers");
	printf("\n");

	errs	= gsc_reg_list(fd, _plx, 0, func);

	return(errs);
}



//*****************************************************************************
const char* gsc_reg_plx9060es_get_desc(u32 reg)
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
const char* gsc_reg_plx9060es_get_name(u32 reg)
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
const gsc_reg_def_t* gsc_reg_plx9060es_get_def_id(int reg)
{
	const gsc_reg_def_t*	def;

	def	= _find_reg(reg, _pci);

	if (def == NULL)
		def	= _find_reg(reg, _plx);

	return(def);
}


