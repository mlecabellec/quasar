// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/service.c $
// $Rev: 53553 $
// $Date: 2023-08-07 14:25:13 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	_15_SEC		(15L * 1000L)



// data types *****************************************************************

struct _wait_src_t
{
	const char*	name;
	int			(*function)(int fd);
	gsc_wait_t	wait;
};



// variables ******************************************************************

static	sem_t		sem_enter;
static	sem_t		sem_exit;
static	gsc_wait_t	wait_dst;
static	int			wait_fd;



//*****************************************************************************
static int _wait_thread(void* arg)
{
	int	ret;

	sem_post(&sem_enter);
	ret	= ai64ssa_ioctl(wait_fd, AI64SSA_IOCTL_WAIT_EVENT, &wait_dst);

	if (ret)
		wait_dst.flags	= 0xFFFFFFFF;

	sem_post(&sem_exit);
	return(0);
}



//*****************************************************************************
static int _process_thread(int fd, const struct _wait_src_t* src)
{
	char		buf[64];
	int			errs	= 0;
	int			i;
	int			ret;
	os_thread_t	thread;
	gsc_wait_t	wait;

	errs	+= ai64ssa_initialize(fd, -1, 0);

	for (;;)	// A convenience loop.
	{
		sem_init(&sem_enter, 0, 0);
		sem_init(&sem_exit, 0, 0);
		memset(&thread, 0, sizeof(thread));
		sprintf(buf, "wait event");
		wait_dst	= src->wait;
		errs		+= os_thread_create(&thread, buf, _wait_thread, NULL);

		// Wait for the thread to become waiting.
		sem_wait(&sem_enter);

		if (errs == 0)
		{
			wait	= src->wait;

			for (i = 0; i < 100; i++)
			{
				ret	= ai64ssa_ioctl(wait_fd, AI64SSA_IOCTL_WAIT_STATUS, &wait);

				if (ret)
				{
					printf(	"FAIL <---  (%d. status request: returned %d)\n",
							__LINE__,
							ret);
					errs++;
					break;
				}

				if (wait.count == 0)
				{
					gsc_time_sleep_ms(100);
					continue;
				}

				if (wait.count == 1)
					break;

				errs++;
				printf(	"FAIL <---  (%d. invalid wait count %ld)\n",
						__LINE__,
						(long) wait.count);
				break;
			}
		}

		// Initiate the respective event.
		errs	+= src->function(wait_fd);

		if (errs)
			break;		// There was an error.

		// Wait for the resumed thread to run.
		sem_wait(&sem_exit);

		for (i = 0;; i++)
		{
			if (wait_dst.flags)
				break;

			if (i >= 100)
			{
				printf(	"FAIL <---  (%d. thread failed to resume)\n",
						__LINE__);
				errs++;
				break;
			}

			gsc_time_sleep_ms(100);
		}

		// Verify that the wait ended as expected.

		if (src->wait.flags == 0xFFFFFFFF)
			break;		// There was an error.

		if (errs)
			break;		// There was an error.

		if (src->wait.timeout_ms == _15_SEC)
		{
			if (wait_dst.flags != GSC_WAIT_FLAG_DONE)
			{
				printf(	"FAIL <---  (%d. flag: expect 0x%lX, got 0x%lX)\n",
						__LINE__,
						(long) GSC_WAIT_FLAG_DONE,
						(long) wait_dst.flags);
				errs++;
			}

			break;
		}

		printf("FAIL <---  (%d. INTERNAL ERROR)\n", __LINE__);
		errs++;
		break;
	}

	// Force termination, just in case of an error.
	wait.flags			= 0;
	wait.main			= 0xFFFFFFFF;
	wait.gsc			= 0xFFFFFFFF;
	wait.alt			= 0xFFFFFFFF;
	wait.io				= 0xFFFFFFFF;
	wait.timeout_ms		= 0;
	wait.count			= 0;
	ret		= ai64ssa_ioctl(wait_fd, AI64SSA_IOCTL_WAIT_CANCEL, &wait);
	errs	+= ret ? 1 : 0;
	errs	+= os_thread_destroy(&thread);

	errs	+= ai64ssa_initialize(fd, -1, 0);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}



//*****************************************************************************
int wait_test(int fd, u32 main, u32 gsc, u32 io, int (*callback)(int fd))
{
	int					errs	= 0;
	struct _wait_src_t	src;

	wait_fd	= fd;
	memset(&wait_dst, 0, sizeof(wait_dst));
	memset(&src, 0, sizeof(src));
	src.function		= callback;
	src.wait.main		= main;
	src.wait.gsc		= gsc;
	src.wait.io			= io;
	src.wait.timeout_ms	= _15_SEC;
	errs	+= _process_thread(fd, &src);
	return(errs);
}


