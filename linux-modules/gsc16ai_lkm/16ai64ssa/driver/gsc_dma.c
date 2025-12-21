// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_dma.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// macros *********************************************************************

#ifndef	DEV_DMA_CHAN_SETUP
	#define	DEV_DMA_CHAN_SETUP(a,io,dma)		0
#endif



/******************************************************************************
*
*	Function:	_dma_chan_acquire
*
*	Purpose:
*
*		Get a DMA channel for the specified operation.
*
*	Arguments:
*
*		setup	Contains content needed for the DMA.
*
*	Returned:
*
*		NULL	We couldn't get the channel.
*		else	This is the pointer to the DMA channel to use.
*
******************************************************************************/

static gsc_dma_ch_t* _dma_chan_acquire(gsc_dma_setup_t* setup)
{
	gsc_dma_ch_t*	dma	= NULL;
	int				test;

	if (setup->io->dma_channel)
	{
		dma	= setup->io->dma_channel;
	}
	else
	{
		test	= os_sem_lock(&setup->dev->dma.sem);

		if (test == 0)
		{
			dma	= gsc_dma_acquire_pci(setup->dev, setup->ability);
			os_sem_unlock(&setup->dev->dma.sem);
		}
		else
		{
			printf(	"%s: %d. %s: failed to acquire lock.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
		}
	}

	return(dma);
}



/******************************************************************************
*
*	Function:	_dma_chan_release
*
*	Purpose:
*
*		Release the DMA channel currently assigned to the I/O stream.
*
*	Arguments:
*
*		setup	Contains content needed for the DMA.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was an error.
*
******************************************************************************/

static int _dma_chan_release(gsc_dma_setup_t* setup)
{
	int	i;
	int	ret;

	i	= os_sem_lock(&setup->dev->dma.sem);
	ret	= gsc_dma_release_pci(setup->dev, setup->dma);

	setup->io->dma_channel	= NULL;
	setup->dma->in_use		= 0;

	if (i == 0)
		os_sem_unlock(&setup->dev->dma.sem);

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_dma_close
*
*	Purpose:
*
*		Cleanup the DMA code for the device due to the device being closed.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*		index	The index of the device channel being referenced.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_dma_close(dev_data_t* dev, int index)
{
	u32	map;

	if ((index >= 0) && (index < GSC_DEVS_PER_BOARD))
	{
		// Adjust the usage map.
		gsc_irq_access_lock(dev);
		map	= dev->dma.usage_map;
		map	&= ~ ((u32) 0x1 << index);
		dev->dma.usage_map	= map;

		if (map == 0)
		{
			// We're the last user so we now release the DMA engines.
			gsc_dma_close_pci(dev);
		}

		gsc_irq_access_unlock(dev);
	}
}



/******************************************************************************
*
*	Function:	gsc_dma_create
*
*	Purpose:
*
*		Perform a one time initialization of the DMA portion of the given
*		structure.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*		ch0_flg	The DMA Channel 0 capability flags.
*
*		ch1_flg	The DMA Channel 1 capability flags.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_dma_create(dev_data_t* dev, u32 ch0_flg, u32 ch1_flg)
{
	int	ret;

	os_sem_create(&dev->dma.sem);
	os_sem_create(&dev->dma.wait.sem);
	os_sem_lock(&dev->dma.wait.sem);
	dev->dma.wait.count	= 0;

	ret	= gsc_dma_create_pci(dev, ch0_flg, ch1_flg);
	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_dma_destroy
*
*	Purpose:
*
*		Perform a one time clean up of the DMA portion of the given structure.
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

void gsc_dma_destroy(dev_data_t* dev)
{
	os_sem_destroy(&dev->dma.sem);
	os_sem_destroy(&dev->dma.wait.sem);
	memset(&dev->dma, 0, sizeof(dev->dma));
}



/******************************************************************************
*
*	Function:	gsc_dma_open
*
*	Purpose:
*
*		Perform the steps needed to prepare the DMA code for operation due to
*		the device being opened.
*
*	Arguments:
*
*		dev		The data for the device of interest.
*
*		index	The index of the device channel being referenced.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_dma_open(dev_data_t* dev, int index)
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
				index);
	}
	else
	{
		// Adjust the usage map.
		gsc_irq_access_lock(dev);
		map	= dev->dma.usage_map;
		dev->dma.usage_map	|= 0x1 << index;

		if (map == 0)
		{
			// We're the first user so we now setup the DMA engines.
			ret	= gsc_dma_open_pci(dev);
		}

		gsc_irq_access_unlock(dev);
	}

	return(ret);
}



//*****************************************************************************
static int _wait_for_dma_channel(dev_data_t* dev)
{
	int	ret;
	int	tmp;

	ret	= os_sem_lock(&dev->dma.sem);

	if (ret == 0)
	{
		dev->dma.wait.count++;
		os_sem_unlock(&dev->dma.sem);

		ret	= os_sem_lock(&dev->dma.wait.sem);

		if (ret)
		{
			// There was a problem.
			tmp	= os_sem_lock(&dev->dma.sem);
			dev->dma.wait.count--;

			if (tmp == 0)
				os_sem_unlock(&dev->dma.sem);
		}
	}

	return(ret);
}



//*****************************************************************************
static void _resume_awaiting_threads(dev_data_t* dev)
{
	int	count;
	int	ret;

	ret		= os_sem_lock(&dev->dma.sem);
	count	= dev->dma.wait.count;

	if (count)
		dev->dma.wait.count--;

	if (ret == 0)
		os_sem_unlock(&dev->dma.sem);

	if (count)
		os_sem_unlock(&dev->dma.wait.sem);
}



//*****************************************************************************
static long _compute_timeout(gsc_dma_setup_t* setup)
{
	os_time_tick_t		ticks;
	long				timeout;

	if (setup->io->non_blocking)
	{
		// Allow at least one second for the DMA.
		timeout	= 1000;
	}
	else if (setup->st_end)
	{
		timeout	= os_time_tick_timedout(setup->st_end);

		if (timeout)
		{
			// We've already timed out.
			timeout	= -ETIMEDOUT;
		}
		else
		{
			// This will almost always be the case.
			ticks	= os_time_tick_get();
			ticks	= setup->st_end - ticks;		// Works even during rollover period.
			timeout	= os_time_ticks_to_ms(ticks);

			if (timeout < 10)
				timeout	= 10;
		}
	}
	else
	{
		timeout	= 0;	// never timeout
	}

	return(timeout);
}



/******************************************************************************
*
*	Function:	gsc_dma_perform
*
*	Purpose:
*
*		Perform a DMA transfer per the given arguments.
*
*		NOTES:
*
*		With interrupts it is possible for the requested interrupt to occure
*		before the current task goes to sleep. This is possible, for example,
*		because the DMA transfer could complete before the task gets to sleep.
*		So, to make sure things work in these cases, we take manual steps to
*		insure that the current task can be awoken by the interrupt before the
*		task actually sleeps and before the interrupt is enabled.
*
*	Arguments:
*
*		setup	Contains content needed for the DMA.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

long gsc_dma_perform(gsc_dma_setup_t* setup)
{
	gsc_dma_ch_t*		dma;
	long				qty		= 0;
	int					ret;
	long				timeout	= 0;
	gsc_wait_t			wt;
	os_sem_t			sem;

	os_sem_create(&sem);	// dummy required for wait operations.

	for (;;)	// Use a loop for convenience.
	{
		dma	= _dma_chan_acquire(setup);

		if (dma == NULL)
		{
			qty	= _wait_for_dma_channel(setup->dev);

			if (qty)
				break;

			continue;
		}

		#ifdef DEV_DMA_CHAN_SETUP

		ret	= DEV_DMA_CHAN_SETUP(setup->alt, setup->io, dma);

		if (ret < 0)
			qty	= ret;

		#endif

		setup->io->dma_channel	= dma;
		dma->error				= 0;

		setup->dma		= dma;
		setup->error	= 0;
		ret				= gsc_dma_setup_pci(setup);

		if (ret < 0)
			qty	= ret;

		if (setup->io->timeout_s > 0)
		{
			//	Check for an I/O timeout.
			timeout	= _compute_timeout(setup);

			if ((timeout < 0) && (qty >= 0))
				qty	= timeout;
		}

		if (qty >= 0)
		{
			// Perform the DMA and wait for completion.
			memset(&wt, 0, sizeof(wt));
			wt.flags		= GSC_WAIT_FLAG_INTERNAL;
			wt.main			= dma->index ? GSC_WAIT_MAIN_DMA1 : GSC_WAIT_MAIN_DMA0;
			wt.timeout_ms	= (u32) timeout;
			gsc_wait_event(setup->alt, &wt, gsc_dma_start_pci, setup, &sem);

			// Clean up.
			ret	= gsc_dma_finish_pci(setup);

			if (ret < 0)
				qty	= ret;
			else if (setup->error < 0)
				qty	= setup->error;
			else
				qty	= setup->bytes;
		}

		if (dma->flags & GSC_DMA_SEL_DYNAMIC)
		{
			ret	= _dma_chan_release(setup);

			if (ret < 0)
				qty	= ret;
		}

		// Notify any threads waiting on an available DMA channel.
		_resume_awaiting_threads(setup->dev);
		break;
	}

	os_sem_destroy(&sem);

	return(qty);
}



/******************************************************************************
*
*	Function:	gsc_dma_abort
*
*	Purpose:
*
*		Abort the I/O channel's DMA transfer if it is in progress.
*
*	Arguments:
*
*		dev		The device data structure.
*
*		io		The I/O structure for the operation to access.
*
*	Returned:
*
*		1		An active DMA was aborted.
*		0		A DMA was not in progress.
*
******************************************************************************/

int gsc_dma_abort(dev_data_t* dev, dev_io_t* io)
{
	gsc_dma_ch_t*	dma;
	int				i;
	int				ret;

	i	= os_sem_lock(&dev->dma.sem);
	dma	= io->dma_channel;

	if (dma)
		ret	= gsc_dma_abort_pci(dev, dma);
	else
		ret	= 0;

	if (i == 0)
		os_sem_unlock(&dev->dma.sem);

	return(ret);
}


