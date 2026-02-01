// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/gsc_util_buf_man.c $
// $Rev: 52357 $
// $Date: 2023-01-12 09:58:19 -0600 (Thu, 12 Jan 2023) $

// OS & Device Independent: Utility: source file

#include "main.h"

/******************************************************************************
*
*	Implement a buffer manager for circular buffer or ping-pong style
*	buffer implementations.
*
******************************************************************************/



// macros *********************************************************************

#ifndef	TRUE
	#define	TRUE				(1 == 1)
#endif

#ifndef	FALSE
	#define	FALSE				(1 == 0)
#endif



// data types *****************************************************************

typedef struct _local_buf_t
{
	// This structure keeps track of an individual memory buffer.

	gsc_buf_man_t			bm;
	int						active;	// Have we started using it yet?
	unsigned long			count;	// Count when buffer was given for use.
	unsigned char			index;
	long					pushed;	// What was this buffer put on the list.
	struct _local_buf_t*	next;
} local_buf_t;

typedef struct
{
	// This structure keeps track of a list of memory buffers.

	const char*		name;
	const char*		desc;
	os_sem_t		sem;		// This reflects buffers being in the list.
								// "Lock" is called to wait for a list entry.
								// "Unlock" is called after adding a list entry.
	local_buf_t*	head;
	local_buf_t*	tail;
	int				waiting;	// Number of tasks waiting for a buffer.

	struct
	{
		// This substructure records statistics on how long threads wait for
		// a buffer and how long buffers remain on the list.

		struct
		{
			unsigned long	sum;	// Toal ms that all buffers were on list.
			long			qty;	// Count of buffers on list.
		} present;	// List elements present on this list.

		struct
		{
			unsigned long	sum;
			long			qty;
		} waiting;	// Number of threads waiting for a list element.

	} stats;

} local_list_t;

typedef struct
{
	// This structure keeps track of all memory buffers.

	os_sem_t		sem;	// Must have this to modify a list.
	int				init;	// Have we been initialized?
	int				busy;	// Are we busy with important work right now?
	unsigned long	ms_begin;
	unsigned long	ms_end;
	unsigned char	qty;	// Number of buffer's we've got.
	local_list_t	data;	// These buffers contain data.
	local_list_t	empty;	// These buffers are empty.
	local_list_t	rx;		// These buffers were empty and are in processing.
	int				stop;	// Stop processing?
	local_list_t	tx;		// These buffers contained data and are in processing.
} local_master_t;



// variables ******************************************************************

static	local_master_t	_master;



/******************************************************************************
*
*	Function:	_buf_alloc
*
*	Purpose:
*
*		Allocate and clear a memory buffer.
*
*	Arguments:
*
*		size	This is the size of the buffer to allocate.
*
*	Returned:
*
*		NULL	The request failed.
*		else	A pointer to the requested memory.
*
******************************************************************************/

static void* _buf_alloc(size_t size)
{
	void*	vp;

	vp	= malloc(size);

	if (vp)
	{
		memset(vp, 0, size);
	}
	else
	{
		printf(	"FAIL <---  (_buf_alloc(): malloc(): %lu Byte%ss)\n",
				(unsigned long) size,
				(size == 1) ? "" : "s");
	}

	return(vp);
}



/******************************************************************************
*
*	Function:	_buf_free
*
*	Purpose:
*
*		Free a memory buffer.
*
*	Arguments:
*
*		vp		This is the buffer to free. This may be NULL.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _buf_free(void* vp)
{
	if (vp)
		free(vp);
}



/******************************************************************************
*
*	Function:	_buf_to_lb
*
*	Purpose:
*
*		Locate the given buffer within the given list.
*
*	Arguments:
*
*		vp		The buffer to look for.
*
*		ll		The list to scan.
*
*	Returned:
*
*		NULL	The buffer wasn't found.
*		else	The local structure where it was found.
*
******************************************************************************/

static local_buf_t* _buf_to_lb(const void* const vp, const local_list_t* const ll)
{
	const local_buf_t*	lb;

	for (lb = ll->head; lb; lb = lb->next)
	{
		if (lb->bm.buffer == vp)
			break;
	}

	return((void*) lb);
}



/******************************************************************************
*
*	Function:	_lb_alloc
*
*	Purpose:
*
*		Allocate and clear a local_buf_t structure.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		NULL	The request failed.
*		else	A pointer to the requested memory.
*
******************************************************************************/

static local_buf_t* _lb_alloc(void)
{
	size_t			bytes	= sizeof(local_buf_t);
	local_buf_t*	lb;

	lb	= malloc(bytes);

	if (lb)
	{
		memset(lb, 0, sizeof(local_buf_t));
	}
	else
	{
		printf(	"FAIL <---  (_lb_alloc(): malloc(): %ld Byte%s)\n",
				(long) bytes,
				(bytes == 1) ? "" : "s");
	}

	return(lb);
}



/******************************************************************************
*
*	Function:	_lb_free
*
*	Purpose:
*
*		Free a cleared local_buf_t structure.
*
*	Arguments:
*
*		lb		This is the structure to be freed. This may be NULL.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _lb_free(local_buf_t* lb)
{
	if (lb)
	{
		memset(lb, 0, sizeof(local_buf_t));
		free(lb);
	}
}



/******************************************************************************
*
*	Function:	_lb_from_bm
*
*	Purpose:
*
*		Locate the given structure and return its corresponding structures.
*
*	Arguments:
*
*		bm		The structure to look for.
*
*		ll		The list it is in goes here. This is NULL if the it isn't
*				found.
*
*	Returned:
*
*		NULL	The structure wasn't found.
*		else	The local structure where it was found.
*
******************************************************************************/

static local_buf_t* _lb_from_bm(const gsc_buf_man_t* bm, local_list_t** ll)
{
	local_buf_t*	lb;

	for (;;)	// A convenience loop.
	{
		ll[0]	= &_master.rx;
		lb		= _buf_to_lb(bm->buffer, ll[0]);
		if (lb)	break;

		ll[0]	= &_master.tx;
		lb		= _buf_to_lb(bm->buffer, ll[0]);
		if (lb)	break;

		ll[0]	= &_master.empty;
		lb		= _buf_to_lb(bm->buffer, ll[0]);
		if (lb)	break;

		ll[0]	= &_master.data;
		lb		= _buf_to_lb(bm->buffer, ll[0]);
		if (lb)	break;

		ll[0]	= NULL;
		break;
	}

	return(lb);
}



/******************************************************************************
*
*	Function:	_lb_init
*
*	Purpose:
*
*		Initialize the given structure.
*
*	Arguments:
*
*		lb		The structure to initialize.
*
*		index	The buffer allocation index.
*
*		size	The size of the buffer.
*
*		vp		The buffer.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _lb_init(
	local_buf_t*	lb,
	unsigned char	index,
	size_t			size,
	void*			vp)
{
	lb->bm.buffer	= vp;
	lb->bm.size		= size;
	lb->bm.offset	= 0;
	lb->bm.count	= 0;
	lb->index		= index;
	lb->next		= NULL;
}



/******************************************************************************
*
*	Function:	_lb_create
*
*	Purpose:
*
*		Allocate and create a new local_buf_t structure for a buffer with the
*		given characteristics.
*
*	Arguments:
*
*		index	The buffer allocation index.
*
*		size	The size of the buffer.
*
*	Returned:
*
*		NULL	The request failed.
*		else	A pointer to the successfully allocated structure.
*
******************************************************************************/

static local_buf_t* _lb_create(unsigned char index, size_t size)
{
	local_buf_t*	lb;
	void*			vp;

	vp	= _buf_alloc(size);
	lb	= _lb_alloc();

	if ((vp) && (lb))
	{
		_lb_init(lb, index, size, vp);
	}
	else
	{
		_buf_free(vp);
		_lb_free(lb);
		lb	= NULL;
	}

	return(lb);
}



/******************************************************************************
*
*	Function:	_lb_destroy
*
*	Purpose:
*
*		Totally dismantle and free the given structure.
*
*	Arguments:
*
*		lb		The structure to destroy.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _lb_destroy(local_buf_t* lb)
{
	_buf_free(lb->bm.buffer);
	_lb_free(lb);
}



/******************************************************************************
*
*	Function:	_lb_remove
*
*	Purpose:
*
*		Remove the given structure from the given list.
*
*	Arguments:
*
*		ll		The list where the structure resides.
*
*		lb		The structure to remove.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _lb_remove(local_list_t* ll, local_buf_t* lb)
{
	long	ms;

	local_buf_t*	ptr;

	if (ll->head == lb)
	{
		ll->head	= lb->next;

		if (ll->tail == lb)
			ll->tail	= NULL;
	}
	else
	{
		for (ptr = ll->head; ptr->next; ptr = ptr->next)
		{
			if (ptr->next != lb)
				continue;

			if (ll->tail == lb)
				ll->tail	= ptr;

			ptr->next	= lb->next;
			break;
		}
	}

	if (lb->active)
	{
		ms	= (long) gsc_time_delta_ms() -  lb->pushed;
		ll->stats.present.sum	+= ms;
		ll->stats.present.qty++;
	}

	lb->next	= NULL;
}



/******************************************************************************
*
*	Function:	_ll_buffer_get
*
*	Purpose:
*
*		Retrieve the first element from the give list as soon as it becomes
*		available.
*
*	Arguments:
*
*		ll		The list to access.
*
*		lb		We record the buffer pointer here.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

static int _ll_buffer_get(local_list_t* ll, local_buf_t** lb)
{
	long	begin;	// In milliseconds.
	long	end;	// In milliseconds.
	int		errs	= 0;
	int		i;
	long	period;	// In milliseconds.

	for (;;)	// A convenience loop.
	{
		// Record the starting time so we can see how long we waited.
		begin	= (long) gsc_time_delta_ms();

		// Increment the waiting counter.
		i	= os_sem_lock(&_master.sem);

		if (i == 0)
		{
			ll->waiting++;
			os_sem_unlock(&_master.sem);
		}
		else
		{
			errs	= 1;
			lb[0]	= NULL;
			break;
		}

		// Wait for an entry on this list to become available.
		i	= os_sem_lock(&ll->sem);

		if (i)
		{
			// Get the master lock so we can make changes.
			i	= os_sem_lock(&_master.sem);

			if (ll->waiting)	// Decrement the waiting counter;
				ll->waiting--;	// Do this even if we didn't get the semaphore.

			if (i)
				os_sem_unlock(&_master.sem);

			errs	= 1;
			lb[0]	= NULL;
			break;
		}

		if (_master.stop)
		{
			// Get the master lock so we can make changes.
			i	= os_sem_lock(&_master.sem);

			if (ll->waiting)	// Decrement the waiting counter;
				ll->waiting--;	// Do this even if we didn't get the semaphore.

			if (i)
				os_sem_unlock(&_master.sem);

			// Stop the operation.
			errs	= 1;
			lb[0]	= NULL;
			os_sem_unlock(&ll->sem);	// It is still available on this list.
			break;
		}

		// Get the master lock so we can make changes.
		i	= os_sem_lock(&_master.sem);

		if (ll->waiting)	// Decrement the waiting counter;
			ll->waiting--;	// Do this even if we didn't get the semaphore.

		if (i)
		{
			errs	= 1;
			lb[0]	= NULL;
			os_sem_unlock(&ll->sem);	// It is still available on this list.
			break;
		}

		if (ll->head == NULL)
		{
			lb[0]	= NULL;
			os_sem_unlock(&_master.sem);
			break;
		}

		// Pull the first element off the list.

		if (ll->head == ll->tail)
		{
			lb[0]		= ll->head;
			ll->head	= NULL;
			ll->tail	= NULL;
			lb[0]->next	= NULL;
		}
		else
		{
			lb[0]		= ll->head;
			ll->head	= lb[0]->next;
			lb[0]->next	= NULL;
		}

		if (lb[0]->active == FALSE)
		{
			lb[0]->active	= TRUE;
			os_sem_unlock(&_master.sem);
			break;
		}

		// Statistics:
		end	= (long) gsc_time_delta_ms();
		lb[0]->count	= lb[0]->bm.count;

		// How long was the thread waiting for the element?
		period	= end - begin;
		ll->stats.waiting.sum	+= period;
		ll->stats.waiting.qty	+= 1;

		// How long was the item on the list?
		period	= end - lb[0]->pushed;
		ll->stats.present.sum	+= period;
		ll->stats.present.qty	+= 1;
		os_sem_unlock(&_master.sem);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	_ll_buffer_put
*
*	Purpose:
*
*		Add a buffer to the given list. The master semaphore is held by the
*		caller.
*
*	Arguments:
*
*		ll		The list to access.
*
*		lb		The buffer to add.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _ll_buffer_put(local_list_t* ll, local_buf_t* lb)
{
	long	begin;

	// Record the starting time so we can see how long we waited.
	begin	= (long) gsc_time_delta_ms();

	// Append the buffer to the list.

	if (ll->head == NULL)
	{
		ll->head	= lb;
		ll->tail	= lb;
	}
	else
	{
		ll->tail->next	= lb;
		ll->tail		= lb;
	}

	// Let folks know its here.
	os_sem_unlock(&ll->sem);

	if (lb->active)
		lb->pushed	= begin;
}



/******************************************************************************
*
*	Function:	_ll_free_all
*
*	Purpose:
*
*		Free the given list content.
*
*	Arguments:
*
*		ll		The local list to process.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

static int _ll_free_all(local_list_t* ll)
{
	int				errs	= 0;
	int				i;
	local_buf_t*	lb;

	for (; ll->head; )
	{
		i	= os_sem_lock(&ll->sem);

		if (i)
		{
			errs	= 1;
			break;
		}

		lb			= ll->head;
		ll->head	= lb->next;
		_lb_destroy(lb);
		_master.qty--;
	}

	ll->tail= NULL;
	return(errs);
}



/******************************************************************************
*
*	Function:	_ll_stats
*
*	Purpose:
*
*		Report current statistics on the given list structure.
*
*	Arguments:
*
*		ll		The structure of interest.
*
*		threads	Report the thread stats?
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _ll_stats(const local_list_t* ll, int threads)
{
	long	ave;
	char	buf[64];

	gsc_label(ll->name);
	printf("(%s)\n", ll->desc);
	gsc_label_level_inc();

	// How long were buffers on this list?
	gsc_label("On List");
	printf("\n");
	gsc_label_level_inc();

	gsc_label("Total Buffers");
	printf("%ld\n", (long) ll->stats.present.qty);

	gsc_label("Average Stay");

	if (ll->stats.present.qty)
		ave	= ll->stats.present.sum / ll->stats.present.qty;
	else
		ave	= 0;

	gsc_time_format_ms(ave, buf, sizeof(buf));
	printf("%s\n", buf);
	gsc_label_level_dec();

	if (threads)
	{
		// How long were threads waiting on this list?
		gsc_label("Awaiting Threads");
		printf("\n");
		gsc_label_level_inc();

		gsc_label("Total Waits");
		printf("%ld\n", (long) ll->stats.waiting.qty);

		gsc_label("Average Wait");

		if (ll->stats.waiting.qty)
			ave	= ll->stats.waiting.sum / ll->stats.waiting.qty;
		else
			ave	= 0;

		gsc_time_format_ms(ave, buf, sizeof(buf));
		printf("%s\n", buf);
		gsc_label_level_dec();
	}

	gsc_label_level_dec();
}



/******************************************************************************
*
*	Function:	_bm_request
*
*	Purpose:
*
*		Request a buffer from the given list.
*
*	Arguments:
*
*		bm		The requested data goes here. Things are NULL when the effort
*				fails.
*
*		ll		The local list to get the buffer from.
*
*		lb		We report to the caller the buffer pointer here. We set this
*				to NULL when we're supposed to stop.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

static int _bm_request(gsc_buf_man_t* bm, local_list_t* ll, local_buf_t** lb)
{
	int	errs;

	if (_master.ms_begin == 0)
		_master.ms_begin	= (unsigned long) gsc_time_delta_ms();

	errs	= _ll_buffer_get(ll, lb);	// performs master lock/unlock as needed

	if (lb[0])
		bm[0]	= lb[0]->bm;
	else
		memset(bm, 0, sizeof(gsc_buf_man_t));

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_free_all
*
*	Purpose:
*
*		Free all buffers. This will block the calling thread until all
*		buffers are freed, unless there is an error.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_free_all(void)
{
	int	errs;
	int	i;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE)	||
			(_master.busy))
		{
			errs	= 1;
			break;
		}

		errs	= gsc_buf_man_stop();

		if (errs)
			break;

		// Wait until no one is blocked on a list.

		for (;;)
		{
			i	= os_sem_lock(&_master.sem);

			if (i)
			{
				errs	= 1;
				break;
			}

			if ((_master.empty.waiting)	||
				(_master.rx.waiting)	||
				(_master.data.waiting)	||
				(_master.tx.waiting))
			{
				os_sem_unlock(&_master.sem);
				gsc_time_sleep_ms(100);
				continue;
			}

			errs	= 0;
			break;
		}

		if (errs)
			break;

		errs	+= _ll_free_all(&_master.tx);
		errs	+= _ll_free_all(&_master.data);
		errs	+= _ll_free_all(&_master.rx);
		errs	+= _ll_free_all(&_master.empty);
		os_sem_unlock(&_master.sem);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_init
*
*	Purpose:
*
*		Perform a one-time initialization.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int gsc_buf_man_init(void)
{
	int	errs	= 0;

	for (;;)	// A convenience loop.
	{
		if (_master.busy)
		{
			errs	= 1;
			break;
		}

		// Do this to meke sure everythinh starts fresh.
		_master.init	= FALSE;
		os_sem_destroy(&_master.sem);
		os_sem_destroy(&_master.data.sem);
		os_sem_destroy(&_master.empty.sem);
		os_sem_destroy(&_master.rx.sem);
		os_sem_destroy(&_master.tx.sem);

		memset(&_master, 0, sizeof(local_master_t));
		_master.busy		= TRUE;
		errs				+= os_sem_create(&_master.sem);
		errs				+= os_sem_create_qty(&_master.data.sem, 999, 0);
		errs				+= os_sem_create_qty(&_master.empty.sem, 999, 0);
		errs				+= os_sem_create_qty(&_master.rx.sem, 999, 0);
		errs				+= os_sem_create_qty(&_master.tx.sem, 999, 0);

		_master.data.name	= "Filled Buffers";
		_master.data.desc	= "Filled buffers waiting to be sent by Tx threads.";

		_master.empty.name	= "Empty Buffers";
		_master.empty.desc	= "Buffers waiting to be filled by Rx threads.";

		_master.rx.name		= "Receiving Buffers";
		_master.rx.desc		= "Buffers being filled by Rx threads.";

		_master.tx.name		= "Transmitting Buffers";
		_master.tx.desc		= "Buffers being emptied by Tx threads.";

		if (errs)
		{
			os_sem_destroy(&_master.sem);
			os_sem_destroy(&_master.data.sem);
			os_sem_destroy(&_master.empty.sem);
			os_sem_destroy(&_master.rx.sem);
			os_sem_destroy(&_master.tx.sem);
		}
		else
		{
			_master.init	= TRUE;
		}

		_master.busy	= FALSE;
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_release_buffer
*
*	Purpose:
*
*		Remove the given buffer from its current list and move it to either
*		the DATA or the EMPTY list, depending on if it has data or not.
*
*	Arguments:
*
*		bm		The buffer being released.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_release_buffer(gsc_buf_man_t* bm)
{
	int				errs;
	int				i;
	local_buf_t*	lb;
	local_list_t*	ll;

	for (;;)	// A convenience loop.
	{
		// Perform validation.

		if ((_master.init == FALSE)	||
			(_master.busy)			||
			(bm == NULL))
		{
			errs	= 1;
			break;
		}

		// Get the master lock so we can access everything.
		i	= os_sem_lock(&_master.sem);

		if (i)
		{
			errs	= 1;
			break;
		}

		// Find out where this buffer is located.
		lb	= _lb_from_bm(bm, &ll);

		if (lb == NULL)
		{
			errs	= 1;
			os_sem_unlock(&_master.sem);
			break;
		}

		// Remove the buffer from the list.
		i	= os_sem_lock(&ll->sem);

		if (i)
		{
			errs	= 1;
			os_sem_unlock(&_master.sem);
			break;
		}

		_master.ms_end	= (unsigned long) gsc_time_delta_ms();

		_lb_remove(ll, lb);

		// Put the buffer on the next list.

		if (bm->count)
		{
			lb->bm.count	= bm->count;
			lb->bm.offset	= bm->offset;
			lb->bm.eof		= bm->eof;
			memcpy(lb->bm.user_area, bm->user_area, BUF_MAN_USER_AREA_SIZE);
			_ll_buffer_put(&_master.data, lb);
			os_sem_unlock(&_master.sem);
		}
		else if (bm->eof)
		{
			lb->bm.offset	= 0;
			lb->bm.eof		= 1;
			memcpy(lb->bm.user_area, bm->user_area, BUF_MAN_USER_AREA_SIZE);
			_ll_buffer_put(&_master.data, lb);
			os_sem_unlock(&_master.sem);
		}
		else
		{
			lb->bm.offset	= 0;
			memcpy(lb->bm.user_area, bm->user_area, BUF_MAN_USER_AREA_SIZE);
			_ll_buffer_put(&_master.empty, lb);
			os_sem_unlock(&_master.sem);
		}

		memset(bm, 0, sizeof(gsc_buf_man_t));
		errs	= 0;
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_request_data
*
*	Purpose:
*
*		Request a buffer containing data.
*
*	Arguments:
*
*		bm		The requested data goes here. Things are NULL when the effort
*				fails or we're supposed to stop.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_request_data(gsc_buf_man_t* bm)
{
	int				errs;
	int				i;
	local_buf_t*	lb;
	int				loop;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE)	||
			(_master.busy)			||
			(bm == NULL))
		{
			errs	= 1;
			break;
		}

		if (_master.stop)
		{
			errs	= 0;
			memset(bm, 0, sizeof(gsc_buf_man_t));
			break;
		}

		// Request a buffer with data.
		errs	= _bm_request(bm, &_master.data, &lb);	// performs master lock/unlock as needed

		if (bm->buffer == NULL)
			errs	= 1;

		if (errs)
			break;

		for (loop = 0;; loop++)
		{
			// We try repeatedly to get the master lock to insure the buffer is not lost.

			if (loop >= 100)
			{
				// Ouch, the buffer is now lost!
				errs	= 1;
				break;
			}

			i	= os_sem_lock(&_master.sem);

			if (i)
				continue;

			_ll_buffer_put(&_master.tx, lb);
			os_sem_unlock(&_master.sem);
			break;
		}

		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_request_empty
*
*	Purpose:
*
*		Request a buffer that is empty.
*
*	Arguments:
*
*		bm		The requested data goes here. Things are NULL when the effort
*				fails.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_request_empty(gsc_buf_man_t* bm)
{
	int				errs;
	int				i;
	int				loop;
	local_buf_t*	lb;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE)	||
			(_master.busy)			||
			(bm == NULL))
		{
			errs	= 1;
			break;
		}

		if (_master.stop)
		{
			errs	= 0;
			memset(bm, 0, sizeof(gsc_buf_man_t));
			break;
		}

		errs	= _bm_request(bm, &_master.empty, &lb);	// performs master lock/unlock as needed

		if (bm->buffer == NULL)
			errs	= 1;

		if (errs)
			break;

		for (loop = 0;; loop++)
		{
			// We try repeatedly to get the master lock to insure the buffer is not lost.

			if (loop >= 100)
			{
				// Ouch, the buffer is now lost!
				errs	= 1;
				break;
			}

			i	= os_sem_lock(&_master.sem);

			if (i)
				continue;

			_ll_buffer_put(&_master.rx, lb);
			os_sem_unlock(&_master.sem);
			break;
		}

		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_setup
*
*	Purpose:
*
*		Add buffers of the given characteristics. This can be done only once.
*		To add new buffers, the current ones must be freed. If a request fails
*		due to memory allocations failures, then those buffers that were
*		allocated are freed.
*
*	Arguments:
*
*		qty		The number of buffers to add.
*
*		size	The size of the buffers to allocate.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_setup(size_t qty, size_t size)
{
	int				errs	= 0;
	int				i;
	local_buf_t*	lb;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE)	||
			(_master.busy)			||
			(_master.qty)			||
			(qty <= 0)				||
			(size <= 0))
		{
			errs	= 1;
			break;
		}

		i	= os_sem_lock(&_master.sem);

		if (i)
		{
			errs	= 1;
			break;
		}

		if ((_master.busy)	||
			(_master.qty))
		{
			errs	= 1;
			os_sem_unlock(&_master.sem);
			break;
		}

		_master.busy		= TRUE;
		_master.ms_begin	= 0;
		_master.ms_end		= 0;
		memset(&_master.empty.stats, 0, sizeof(_master.empty.stats));
		memset(&_master.rx.stats, 0, sizeof(_master.rx.stats));
		memset(&_master.data.stats, 0, sizeof(_master.data.stats));
		memset(&_master.tx.stats, 0, sizeof(_master.tx.stats));

		for (i = 0; i < (int) qty; i++)
		{
			lb	= _lb_create((unsigned char) i, size);

			if (lb)
			{
				_ll_buffer_put(&_master.empty, lb);
			}
			else
			{
				errs	= 1;
				break;
			}
		}

		if (errs)
		{
			for (; _master.empty.head;)
			{
				os_sem_lock(&_master.empty.sem);
				lb	= _master.empty.head;
				_master.empty.head	= lb->next;
				_lb_destroy(lb);
			}
		}
		else
		{
			_master.qty	= (unsigned char) qty;
		}

		_master.busy	= FALSE;
		os_sem_unlock(&_master.sem);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_stop
*
*	Purpose:
*
*		Stop dolling out buffers in preparation for termination.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_stop(void)
{
	int	errs;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE) ||
			(_master.busy))
		{
			errs	= 1;
			break;
		}

		errs			= 0;
		_master.stop	= 1;
		os_sem_unlock(&_master.data.sem);
		os_sem_unlock(&_master.empty.sem);
		os_sem_unlock(&_master.rx.sem);
		os_sem_unlock(&_master.tx.sem);
		os_sem_unlock(&_master.sem);
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	gsc_buf_man_stats
*
*	Purpose:
*
*		Report current statistics.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of errors seen.
*
******************************************************************************/

int gsc_buf_man_stats(void)
{
	int	errs;
	int	i;

	for (;;)	// A convenience loop.
	{
		if ((_master.init == FALSE) ||
			(_master.busy))
		{
			errs	= 1;
			break;
		}

		i	= os_sem_lock(&_master.sem);

		if (i)
		{
			errs	= 1;
			break;
		}

		errs	= 0;
		gsc_label("Buffer Manager Statistics");
		printf("\n");
		gsc_label_level_inc();

		_ll_stats(&_master.empty,	(int) TRUE );
		_ll_stats(&_master.rx,		(int) FALSE);
		_ll_stats(&_master.data,	(int) TRUE );
		_ll_stats(&_master.tx,		(int) FALSE);

		gsc_label_level_dec();
		os_sem_unlock(&_master.sem);
		break;
	}

	return(errs);
}


