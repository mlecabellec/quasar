// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_wait.c $
// $Rev: 50967 $
// $Date: 2022-04-25 08:41:30 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// macros *********************************************************************

// gsc_wait_t.io flags

#ifdef DEV_SUPPORTS_WAIT

	#if defined(DEV_SUPPORTS_READ)
		#if !defined(DEV_WAIT_IO_ALL)
			#error "ERROR: DEV_WAIT_IO_ALL not defined."
		#endif
	#endif

	#if defined(DEV_SUPPORTS_WRITE)
		#if !defined(DEV_WAIT_IO_ALL)
			#error "ERROR: DEV_WAIT_IO_ALL not defined."
		#endif
	#endif

	#ifndef	DEV_WAIT_IO_ALL
		#define	DEV_WAIT_IO_ALL		0
	#endif

#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
static void _wait_list_node_add(GSC_ALT_STRUCT_T* alt, gsc_wait_node_t* node)
{
	dev_data_t*	dev	= GSC_ALT_DEV_GET(alt);

	gsc_irq_access_lock(dev);
	node->next		= alt->wait_list;
	alt->wait_list	= node;
	gsc_irq_access_unlock(dev);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
static int _wait_list_node_remove(GSC_ALT_STRUCT_T* alt, gsc_wait_node_t* node, int lock)
{
	dev_data_t*			dev		= GSC_ALT_DEV_GET(alt);
	int					found	= 0;
	gsc_wait_node_t*	list;

	// It is presumed here that access is already locked.

	if (lock)
		gsc_irq_access_lock(dev);

	if (alt->wait_list == node)
	{
		found			= 1;
		alt->wait_list	= node->next;
		node->next		= NULL;
	}
	else
	{
		for (list = alt->wait_list; list;)
		{
			if (list->next == node)
			{
				list->next	= node->next;
				node->next	= NULL;
				found		= 1;
				break;
			}
			else
			{
				list	= list->next;
			}
		}
	}

	if (lock)
		gsc_irq_access_unlock(dev);

	return(found);
}
#endif



//*****************************************************************************
// Not all OS's can take the address of an item on the stack during an ISR.
// This also applies to passing in a structure by value. Because of these
// limits the "wait" criteria is passed in as individual fields rather than a
// pointer. In this case the culprit is INTime for Windows. Don 5/3/2014
#ifdef DEV_SUPPORTS_WAIT
static u32 _wait_resume(
	GSC_ALT_STRUCT_T*	alt,
	u32					new_flags,
	u32					wt_flags,
	u32					wt_main,
	u32					wt_gsc,
	u32					wt_alt,
	u32					wt_io,
	int					lock)
{
	dev_data_t*			dev	= GSC_ALT_DEV_GET(alt);
	u32					count	= 0;
	gsc_wait_node_t*	list;
	gsc_wait_node_t*	next;
	u32					test;

	if (lock)
		gsc_irq_access_lock(dev);

	for (list = alt->wait_list; list; list = next)
	{
		next	= list->next;

		if ((list->wait->flags & GSC_WAIT_FLAG_INTERNAL) !=
			(wt_flags & GSC_WAIT_FLAG_INTERNAL))
		{
			continue;
		}

		test	= list->wait->main & wt_main;

		if (test)
		{
			count++;
			list->wait->flags	= new_flags;
			list->wait->main	= test;
			list->wait->gsc		= 0;
			list->wait->alt		= 0;
			list->wait->io		= 0;
			_wait_list_node_remove(alt, list, 0);
			os_event_resume(&list->evnt);
			continue;
		}

		test	= list->wait->gsc & wt_gsc;

		if (test)
		{
			count++;
			list->wait->flags	= new_flags;
			list->wait->main	= 0;
			list->wait->gsc		= test;
			list->wait->alt		= 0;
			list->wait->io		= 0;
			_wait_list_node_remove(alt, list, 0);
			os_event_resume(&list->evnt);
			continue;
		}

		test	= list->wait->alt & wt_alt;

		if (test)
		{
			count++;
			list->wait->flags	= new_flags;
			list->wait->main	= 0;
			list->wait->gsc		= 0;
			list->wait->alt		= test;
			list->wait->io		= 0;
			_wait_list_node_remove(alt, list, 0);
			os_event_resume(&list->evnt);
			continue;
		}

		test	= list->wait->io & wt_io;

		if (test)
		{
			count++;
			list->wait->flags	= new_flags;
			list->wait->main	= 0;
			list->wait->gsc		= 0;
			list->wait->alt		= 0;
			list->wait->io		= test;
			_wait_list_node_remove(alt, list, 0);
			os_event_resume(&list->evnt);
			continue;
		}
	}

	if (lock)
		gsc_irq_access_unlock(dev);

	return(count);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_resume_io(GSC_ALT_STRUCT_T* alt, u32 io)
{
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	io,
		/* lock			*/	1);

	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	0,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	io,
		/* lock			*/	1);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_resume_irq_alt(GSC_ALT_STRUCT_T* alt_t, u32 alt)
{
	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt_t,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	alt,
		/* wt_io		*/	0,
		/* lock			*/	0);

	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt_t,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	0,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	alt,
		/* wt_io		*/	0,
		/* lock			*/	0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_resume_irq_gsc(GSC_ALT_STRUCT_T* alt, u32 gsc)
{
	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	0,
		/* wt_gsc		*/	gsc,
		/* wt_alt		*/	0,
		/* wt_io		*/	0,
		/* lock			*/	0);

	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	0,
		/* wt_main		*/	0,
		/* wt_gsc		*/	gsc,
		/* wt_alt		*/	0,
		/* wt_io		*/	0,
		/* lock			*/	0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_resume_irq_io(GSC_ALT_STRUCT_T* alt, u32 io)
{
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	io,
		/* lock			*/	0);

	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	0,
		/* wt_main		*/	0,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	io,
		/* lock			*/	0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_resume_irq_main(GSC_ALT_STRUCT_T* alt, u32 main)
{
	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	main,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	0,
		/* lock			*/	0);

	// Access should already be locked.
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_DONE,
		/* wt_flags		*/	0,
		/* wt_main		*/	main,
		/* wt_gsc		*/	0,
		/* wt_alt		*/	0,
		/* wt_io		*/	0,
		/* lock			*/	0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
#if (GSC_DEVS_PER_BOARD > 1)
void gsc_wait_resume_irq_main_multi(dev_data_t* dev, u32 flags)
{
	int	i;

	for (i = 0; i < GSC_DEVS_PER_BOARD; i++)
		gsc_wait_resume_irq_main(&dev->channel[i], flags);
}
#endif
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
#if (GSC_DEVS_PER_BOARD > 1)
void gsc_wait_resume_irq_main_dma_multi(dev_data_t* dev, gsc_dma_ch_t* dma, u32 flags)
{
	GSC_ALT_STRUCT_T*	alt		= NULL;
	int					i;
	int					j;
	u32					type	= GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS;

	for (i = 0; i < GSC_DEVS_PER_BOARD; i++)
	{
		alt	= &dev->channel[i];

		for (j = 0; j < DEV_IO_STREAM_QTY; j++)
		{
			if (alt->io.io_streams[j])
			{
				if (alt->io.io_streams[j]->dma_channel == dma)
				{
					type	= flags;
					i		= GSC_DEVS_PER_BOARD;
					break;
				}
			}
		}
	}

	gsc_wait_resume_irq_main(alt, type);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_wait_event
*
*	Purpose:
*
*		Implement a generic Wait Event service.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		wait	The wait structure to utilize.
*
*		setup	The function to call before sleeping. NULL is OK.
*
*		arg		The arbitrary argument to pass to the above function.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_WAIT
int gsc_wait_event(
	GSC_ALT_STRUCT_T*	alt,
	gsc_wait_t*			wait,
	int					(*setup)(GSC_ALT_STRUCT_T* alt, void* arg),
	void*				arg,
	os_sem_t*			sem)
{
	int				i;
	gsc_wait_node_t	node;
	int				ret		= 0;
	os_time_tick_t	timeout;	// in system ticks
	os_time_t		tt_end;

	for (;;)
	{
		if ((alt == NULL) || (wait == NULL))
		{
			ret	= -EINVAL;
			break;
		}

		if (wait->timeout_ms)
			timeout	= os_time_ms_to_ticks(wait->timeout_ms);
		else
			timeout	= 0;

		// Initialize the wait node.
		node.next		= NULL;
		node.wait		= wait;
		os_time_get(&node.tt_start);
		os_event_create(&node.evnt);

		_wait_list_node_add(alt, &node);

		if (setup)
			ret	= (setup)(alt, arg);

		if (ret == 0)
		{
			os_sem_unlock(sem);
			os_event_wait(&node.evnt, timeout);
			os_sem_lock(sem);
		}

		os_event_destroy(&node.evnt);

		i	= _wait_list_node_remove(alt, &node, 1);

		if (i)
		{
			// The node was still on the list, which means the wait request timed out.
			wait->flags	= GSC_WAIT_FLAG_TIMEOUT;
			wait->main	= 0;
			wait->gsc	= 0;
			wait->alt	= 0;
			wait->io	= 0;
		}

		// Compute the amount of time the thread waited.
		os_time_get(&tt_end);

		if (wait->timeout_ms)
			wait->timeout_ms	= (u32) os_time_delta_ms(&node.tt_start, &tt_end);

		break;
	}

	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_wait_event_ioctl
*
*	Purpose:
*
*		Implement the generic portion of the Wait Event IOCTL service.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		arg		The argument required for the service.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_WAIT
int gsc_wait_event_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg)
{
	int	ret		= 0;

	for (;;)	// A convenience loop.
	{
		if (arg->flags)
		{
			ret	= -EINVAL;
			break;
		}

		if (arg->main & (u32) ~GSC_WAIT_MAIN_ALL)
		{
			ret	= -EINVAL;
			break;
		}

		if (arg->gsc & (u32) ~DEV_WAIT_GSC_ALL)
		{
			ret	= -EINVAL;
			break;
		}

		if (arg->alt & (u32) ~DEV_WAIT_ALT_ALL)
		{
			ret	= -EINVAL;
			break;
		}

		if (arg->io & (u32) ~DEV_WAIT_IO_ALL)
		{
			ret	= -EINVAL;
			break;
		}

		if (arg->count)
		{
			ret	= -EINVAL;
			break;
		}

		if ((arg->timeout_ms > GSC_WAIT_TIMEOUT_MAX)	||
			(arg->timeout_ms < 0))
		{
			ret	= -EINVAL;
			break;
		}

		if ((arg->main) || (arg->gsc) || (arg->alt) || (arg->io))
		{
			ret	= gsc_wait_event(alt, arg, NULL, NULL, &alt->sem);
		}
		else
		{
			ret	= -EINVAL;
		}

		break;
	}

	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_wait_cancel_ioctl
*
*	Purpose:
*
*		Implement the generic portion of the Wait Cancel IOCTL service.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		arg		The argument required for the service.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_WAIT
int gsc_wait_cancel_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg)
{
	arg->count	= _wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_CANCEL,
		/* wt_flags		*/	arg->flags & ~GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	arg->main,
		/* wt_gsc		*/	arg->gsc,
		/* wt_alt		*/	arg->alt,
		/* wt_io		*/	arg->io,
		/* lock			*/	1);

	return(0);
}
#endif



/******************************************************************************
*
*	Function:	gsc_wait_status_ioctl
*
*	Purpose:
*
*		Count the number of waiting threads which mwaiting any of the given
*		criereia.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		arg		The argument required for the service.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_WAIT
int gsc_wait_status_ioctl(GSC_ALT_STRUCT_T* alt, gsc_wait_t* arg)
{
	dev_data_t*			dev		= GSC_ALT_DEV_GET(alt);
	gsc_wait_node_t*	list;

	arg->count	= 0;
	gsc_irq_access_lock(dev);

	for (list = alt->wait_list; list; list = list->next)
	{
		if (list->wait->flags & GSC_WAIT_FLAG_INTERNAL)
			continue;

		if ((list->wait->main		& arg->main)	||
			(list->wait->gsc		& arg->gsc)		||
			(list->wait->alt		& arg->alt)		||
			(list->wait->io			& arg->io))
		{
			arg->count++;
		}
	}

	gsc_irq_access_unlock(dev);

	return(0);
}
#endif



/******************************************************************************
*
*	Function:	gsc_wait_close
*
*	Purpose:
*
*		Make sure all wait nodes are released.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*	Returned:
*
*		None.
*
******************************************************************************/

#ifdef DEV_SUPPORTS_WAIT
void gsc_wait_close(GSC_ALT_STRUCT_T* alt)
{
	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_CANCEL,
		/* wt_flags		*/	GSC_WAIT_FLAG_INTERNAL,
		/* wt_main		*/	0xFFFFFFFF,
		/* wt_gsc		*/	0xFFFFFFFF,
		/* wt_alt		*/	0xFFFFFFFF,
		/* wt_io		*/	0xFFFFFFFF,
		/* lock			*/	1);

	_wait_resume(
		/* alt			*/	alt,
		/* new_flags	*/	GSC_WAIT_FLAG_CANCEL,
		/* wt_flags		*/	0,
		/* wt_main		*/	0xFFFFFFFF,
		/* wt_gsc		*/	0xFFFFFFFF,
		/* wt_alt		*/	0xFFFFFFFF,
		/* wt_io		*/	0xFFFFFFFF,
		/* lock			*/	1);
}
#endif



