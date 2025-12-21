// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_pci.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

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
		printf(	"%s: %d. %s: invalid pointer reference.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
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


