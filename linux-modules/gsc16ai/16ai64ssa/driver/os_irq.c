// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_irq.c $
// $Rev: 53838 $
// $Date: 2023-11-15 13:51:51 -0600 (Wed, 15 Nov 2023) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



// macros *********************************************************************

#ifndef DEV_IRQ_SHOW
#define	DEV_IRQ_SHOW	0
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int os_irq_create(dev_data_t* dev)
{
	// The ISR is installed during the open call, so nothing is done here.
	dev->irq.os.created	= 1;

#if DEV_IRQ_SHOW
	printf("%s: IRQ: irq# %d\n", DEV_NAME, (int) dev->pci.pd->irq);
#endif

	return(0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
void os_irq_destroy(dev_data_t* dev)
{
	// The ISR is removed during the close call, so nothing is done here.

	if (dev->irq.os.created)
		dev->irq.os.created	= 0;
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int os_irq_open(dev_data_t* dev)
{
	int	ret	= 0;

	for (;;)	// A convenience loop.
	{
		if (dev->irq.os.opened)
			break;

#if (OS_SUPPORTS_MSI) && (DEV_SUPPORTS_MSI)

		dev->irq.os.msi_active	= 0;
		ret	= OS_MSI_ENABLE(dev);

		if (ret == 0)
		{
			if (dev->pci.pd->irq == 0)
			{
				ret	= EINVAL;
				printf(	"%s: %d. %s: invalid MSI IRQ: 0\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}

			ret	= IRQ_REQUEST_MSI(dev, os_irq_isr);

			if (ret)
			{
				printf(	"%s: %d. %s: RQ_REQUEST_MSI() failed: MSI IRQ %d, error %d\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__,
						(int) dev->pci.pd->irq,
						ret);
				break;
			}

			// Success.
			dev->irq.os.msi_active	= 1;
			dev->irq.os.opened		= 1;
			break;
		}

		// The MSI request failed. Try INTx interrupts.
#endif

		if (dev->pci.pd->irq == 0)
		{
			ret	= EINVAL;
			printf(	"%s: %d. %s: invalid IRQ: 0\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		ret	= IRQ_REQUEST(dev, os_irq_isr);

		if (ret)
		{
			printf(	"%s: %d. %s: IRQ_REQUEST() failed: IRQ %d, error %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) dev->pci.pd->irq,
					ret);
			break;
		}

		// Success.
		dev->irq.os.opened	= 1;
		break;
	}

	return(ret);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
void os_irq_close(dev_data_t* dev)
{
	if (dev->irq.os.opened)
	{
		free_irq(dev->pci.pd->irq, dev);

#if (OS_SUPPORTS_MSI) && (DEV_SUPPORTS_MSI)

		if (dev->irq.os.msi_active)
		{
			dev->irq.os.msi_active	= 0;
			OS_MSI_DISABLE(dev);
		}

#endif

		dev->irq.os.opened	= 0;
	}
}
#endif


