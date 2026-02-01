// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_pci.c $
// $Rev: 42879 $
// $Date: 2018-05-29 11:45:41 -0500 (Tue, 29 May 2018) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
int os_pci_dev_enable(os_pci_t* pci)
{
	int	ret;

	if ((pci) && (pci->pd) && (pci->enabled == 0))
	{
		ret	= PCI_ENABLE_DEVICE(pci->pd);

		if (ret == 0)
			pci->enabled	= 1;
	}
	else
	{
		ret	= -EINVAL;
	}

	return(ret);
}



//*****************************************************************************
void os_pci_dev_disable(os_pci_t* pci)
{
	if ((pci) && (pci->pd) && (pci->enabled))
	{
		PCI_DISABLE_DEVICE(pci->pd);
		pci->enabled	= 0;
	}
}



//*****************************************************************************
void os_pci_master_clear(os_pci_t* pci)
{
	if ((pci) && (pci->pd) && (pci->enabled))
	{
		PCI_CLEAR_MASTER(pci->pd);
	}
}



//*****************************************************************************
int os_pci_master_set(os_pci_t* pci)
{
	if ((pci) && (pci->pd) && (pci->enabled))
	{
		pci_set_master(pci->pd);
	}

	return(0);
}


