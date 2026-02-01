// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_irq.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
#ifndef	PRINTF_ISR
static void PRINTF_ISR(const char* format, ...)
{
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_irq_access_lock
*
*	Purpose:
*
*		Apply a locking mechanism to prevent simultaneous access to the
*		device's IRQ substructure.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_irq_access_lock(dev_data_t* dev)
{
	os_spinlock_lock(&dev->spinlock);
}



/******************************************************************************
*
*	Function:	gsc_irq_access_unlock
*
*	Purpose:
*
*		Remove the locking mechanism that prevented simultaneous access to the
*		device's IRQ substructure.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_irq_access_unlock(dev_data_t* dev)
{
	os_spinlock_unlock(&dev->spinlock);
}



/******************************************************************************
*
*	Function:	gsc_irq_close
*
*	Purpose:
*
*		Perform IRQ actions appropriate for closing a device.
*
*	Arguments:
*
*		dev		The device of interest.
*
*		index	The index of the device channel being referenced.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
void gsc_irq_close(dev_data_t* dev, int index)
{
	u32	map;

	if ((index >= 0) && (index < GSC_DEVS_PER_BOARD))
	{
		// Adjust the usage map.
		gsc_irq_access_lock(dev);
		map	= dev->irq.usage_map;
		map	&= ~ ((u32) 0x1 << index);
		dev->irq.usage_map	= map;

		if (map == 0)
		{
			// Reset the device interrupts.
			gsc_irq_reset_pci(dev, 0);

			if (dev->irq.opened)
			{
				// We're the last user so we now release the interrupt. We have
				// to release the lock before we release the interrupt, as the
				// release call may cause the ISR to be called.
				gsc_irq_access_unlock(dev);
				os_irq_close(dev);
				gsc_irq_access_lock(dev);
				dev->irq.opened	= 0;
			}
		}

		gsc_irq_access_unlock(dev);
	}
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_create
*
*	Purpose:
*
*		Perform a one time initialization of the structure.
*
*	Arguments:
*
*		dev		The device of interest.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_create(dev_data_t* dev)
{
	int	ret;

	memset(&dev->irq, 0, sizeof(dev->irq));
	os_sem_create(&dev->irq.sem);
	ret	= gsc_irq_create_pci(dev);
	gsc_irq_reset_pci(dev, 1);
	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_destroy
*
*	Purpose:
*
*		Perform a one time tear down of the structure.
*
*	Arguments:
*
*		dev		The device of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
void gsc_irq_destroy(dev_data_t* dev)
{
	gsc_irq_reset_pci(dev, 0);
	gsc_irq_destroy_pci(dev);
	os_sem_destroy(&dev->irq.sem);
	memset(&dev->irq, 0, sizeof(dev->irq));
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_open
*
*	Purpose:
*
*		Perform IRQ actions appropriate for opening a device.
*
*	Arguments:
*
*		dev		The device of interest.
*
*		index	The index of the device channel being referenced.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_open(dev_data_t* dev, int index)
{
	u32	map;
	int	ret	= 0;

	if ((index < 0) || (index >= GSC_DEVS_PER_BOARD))
	{
		ret	= -EINVAL;
		printf(	"%s: %d. %s: invalid device index: %d\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__,
				(int) index);
	}
	else
	{
		// Adjust the usage map.
		gsc_irq_access_lock(dev);
		map	= dev->irq.usage_map;
		dev->irq.usage_map	|= 0x1 << index;

		if (map == 0)
		{
			// We're the first user so we now acquire the interrupt. We have
			// to release the lock before we acquire the interrupt, as the
			// acquire call may cause the ISR to be called.
			gsc_irq_reset_pci(dev, 0);
			gsc_irq_access_unlock(dev);
			ret	= os_irq_open(dev);
			gsc_irq_access_lock(dev);

			if (ret == 0)
			{
				dev->irq.opened	= 1;
				gsc_irq_init_pci(dev, 0);
			}
			else
			{
				printf(	"%s: %d. %s: failed to acquire interrupt.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
			}
		}

		gsc_irq_access_unlock(dev);
	}

	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_isr_common
*
*	Purpose:
*
*		Detect and service interrupts.
*
*	Arguments:
*
*		dev_id	The structure for the device with the interrupt.
*
*		flags	See the GSC_IRQ_ISR_FLAG_xxx definitions in gsc_main.h.
*
*	Returned:
*
*		0		The interrupt was NOT ours.
*		1		The interrupt was ours.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_isr_common(void* dev_id, u32 flags)
{
	dev_data_t*		dev	= (void*) dev_id;
	int				is_ours;	// Is this one or our interrupts?

	if (flags & GSC_IRQ_ISR_FLAG_LOCK)
		gsc_irq_access_lock(dev);

	is_ours	= gsc_irq_isr_common_pci(dev, flags);

	if (flags & GSC_IRQ_ISR_FLAG_LOCK)
		gsc_irq_access_unlock(dev);

	return(is_ours);
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_local_disable
*
*	Purpose:
*
*		Disable local interrupts. This is for non-ISR use only.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_local_disable(dev_data_t* dev)
{
	int	ret;

	ret	= gsc_irq_local_disable_pci(dev);
	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_irq_local_enable
*
*	Purpose:
*
*		Enable local interrupts. This is for non-ISR use only.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_local_enable(dev_data_t* dev)
{
	int	ret;

	ret	= gsc_irq_local_enable_pci(dev);
	return(ret);
}
#endif


