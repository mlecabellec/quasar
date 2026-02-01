// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/wait_status.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	_15_SEC		(15L * 1000L)



// variables ******************************************************************

static	const gsc_wait_t	wait_src[]	=
{
	// flags		main							...			timeout_ms
	{ 0,			GSC_WAIT_MAIN_DMA0,				0, 0, 0,	_15_SEC	},
	{ 0,			GSC_WAIT_MAIN_DMA1,				0, 0, 0,	_15_SEC	},
	{ 0,			GSC_WAIT_MAIN_GSC,				0, 0, 0,	_15_SEC	},

	// flags ...	gsc								...			timeout_ms
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_00,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_01,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_02,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_03,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_04,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_05,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_06,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_07,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_08,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_09,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_10,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_11,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_12,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_13,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_14,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_COS_15,		0, 0,		_15_SEC	},
	{ 0, 0,			OPTO16X16_WAIT_GSC_EVENT_COUNT,	0, 0,		_15_SEC	},

	// terminate list
	{ 1	}
};

static	gsc_wait_t	wait_dst[SIZEOF_ARRAY(wait_src)];
static	int			wait_fd;
static	os_thread_t	wait_thread[SIZEOF_ARRAY(wait_src)];



//*****************************************************************************
static int _service_test(int fd)
{
	// There are no settings bits to test here.
	return(0);
}



//*****************************************************************************
static int _wait_thread(void* arg)
{
	int	i		= (VP2I) arg;
	int	ret;

	ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_EVENT, &wait_dst[i]);

	if (ret < 0)
		wait_dst[i].flags	= 0xFFFFFFFF;

	return(0);
}


//*****************************************************************************
static int _create_thread_1(int index)
{
	char		buf[64];
	int			errs	= 0;
	int			i;
	int			ret;
	gsc_wait_t	wait;

	wait_dst[index]	= wait_src[index];
	sprintf(buf, "wait status %d", index);
	errs	+= os_thread_create(	&wait_thread[index],
									buf,
									_wait_thread,
									(I2VP) index);

	// Wait for the thread to become waiting.

	if (errs == 0)
	{
		wait	= wait_dst[index];

		for (i = 0; i < 100; i++)
		{
			ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_STATUS, &wait);

			if (ret < 0)
			{
				printf(	"FAIL <---  (%d. status request %d: error %d)\n",
						__LINE__,
						index,
						ret);
				errs++;
				break;
			}

			if (wait.count == 1)
				break;

			os_sleep_ms(100);
		}

		if ((errs == 0) && (wait.count != 1))
		{
			printf(	"FAIL <---  (%d. count: expect 1, got %ld)\n",
					__LINE__,
					(long) wait.count);
			errs++;
		}
	}

	// Verify the wait status for all created threads.

	if (errs == 0)
	{
		wait.flags		= 0;
		wait.main		= 0xFFFFFFFF;
		wait.gsc		= 0xFFFFFFFF;
		wait.alt		= 0xFFFFFFFF;
		wait.io			= 0xFFFFFFFF;
		wait.timeout_ms	= 0;
		wait.count		= 0;
		ret				= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_STATUS, &wait);

		if (ret < 0)
		{
			printf(	"FAIL <---  (%d. status request %d: error %d)\n",
					__LINE__,
					index,
					ret);
			errs++;
		}

		if ((errs == 0) && (wait.count != (index + 1)))
		{
			printf(	"FAIL <---  (%d. count: expect %d, got %ld)\n",
					__LINE__,
					index + 1,
					(long) wait.count);
			errs++;
		}
	}

	return(errs);
}



//*****************************************************************************
static int _create_thread_2(int index)
{
	char		buf[64];
	int			errs	= 0;
	int			i;
	int			ret;
	gsc_wait_t	wait;

	wait_dst[index]	= wait_src[0];
	sprintf(buf, "wait status %d", index);
	errs	+= os_thread_create(	&wait_thread[index],
									buf,
									_wait_thread,
									(I2VP) index);

	// Wait for the thread to become waiting.

	if (errs == 0)
	{
		wait	= wait_dst[index];

		for (i = 0; i < 100; i++)
		{
			ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_STATUS, &wait);

			if (ret < 0)
			{
				printf(	"FAIL <---  (%d. status request %d: error %d)\n",
						__LINE__,
						index,
						ret);
				errs++;
				break;
			}

			if (wait.count == (index + 1))
				break;

			os_sleep_ms(100);
		}

		if ((errs == 0) && (wait.count != index + 1))
		{
			printf(	"FAIL <---  (%d. count: expect %d, got %ld)\n",
					__LINE__,
					index + 1,
					(long) wait.count);
			errs++;
		}
	}

	return(errs);
}



//*****************************************************************************
static int _delete_thread_1(int index)
{
	int			errs	= 0;
	int			ret;
	gsc_wait_t	wait;

	if (wait_dst[index].flags != 0xFFFFFFFF)
	{
		wait	= wait_dst[index];
		ret		= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_CANCEL, &wait);

		if (ret < 0)
		{
			printf(	"FAIL <---  (%d. calcel request %d: error %d)\n",
					__LINE__,
					index,
					ret);
			errs++;
		}

		if ((errs == 0) && (wait.count != 1))
		{
			printf(	"FAIL <---  (%d. count: expect 1, got %ld)\n",
					__LINE__,
					(long) wait.count);
			errs++;
		}

		errs	+= os_thread_destroy(&wait_thread[index]);
	}

	return(errs);
}



//*****************************************************************************
static int _delete_thread_2(int count)
{
	int			errs	= 0;
	int			i;
	int			ret;
	gsc_wait_t	wait;

	wait	= wait_dst[0];
	ret		= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_CANCEL, &wait);

	if (ret < 0)
	{
		printf(	"FAIL <---  (%d. cancel request: error %d)\n",
				__LINE__,
				ret);
		errs++;
	}

	if ((errs == 0) && (wait.count != count))
	{
		printf(	"FAIL <---  (%d. count: expect %d, got %ld)\n",
				__LINE__,
				count,
				(long) wait.count);
		errs++;
	}

	for (i = 0; i < count; i++)
	{
		if (wait_dst[i].flags != 0xFFFFFFFF)
			errs	+= os_thread_destroy(&wait_thread[i]);
	}

	return(errs);
}



//*****************************************************************************
static int _function_test(int fd)
{
	int		errs	= 0;
	int		i;

	errs	+= opto16x16_initialize(fd, -1, 0);

	wait_fd	= fd;
	memset(wait_dst, 0, sizeof(wait_dst));
	memset(wait_thread, 0, sizeof(wait_thread));

	// Create threads awaiting each of the source crtiteria.

	for (i = 0; (errs == 0) && (wait_src[i].flags == 0) ; i++)
		errs	+= _create_thread_1(i);

	// Cancel each wait and delete the thread.

	for (i = 0; wait_src[i].flags == 0 ; i++)
		errs	+= _delete_thread_1(i);

	// Create multiple threads awaiting the same criteria.

	for (i = 0; (errs == 0) && (wait_src[i].flags == 0) ; i++)
		errs	+= _create_thread_2(i);

	// Cancel each wait and delete the thread.
	errs	+= _delete_thread_2(SIZEOF_ARRAY(wait_src) - 1);

	return(errs);
}



/******************************************************************************
*
*	Function:	wait_status_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_WAIT_STATUS.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int wait_status_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_WAIT_STATUS");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


