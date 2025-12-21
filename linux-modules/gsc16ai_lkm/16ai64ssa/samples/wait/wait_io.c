// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/wait_io.c $
// $Rev: 54969 $
// $Date: 2024-08-07 15:58:15 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// variables ******************************************************************

static	int			io_fd;
static	sem_t		sem_enter;
static	sem_t		sem_exit;



//*****************************************************************************
static int _read_thread(void* arg)
{
	char	buf[1024];

	sem_post(&sem_enter);
	ai64ssa_read(io_fd, buf, sizeof(buf));
	sem_post(&sem_exit);
	return(0);
}



//*****************************************************************************
static int _abort(int fd)
{
	s32			aborted	= 0;
	char		buf[64];
	int			errs	= 0;
	os_thread_t	thread;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_rx_io_timeout(fd, -1, 0, 15, NULL);

	if (errs == 0)
	{
		io_fd	= fd;
		sem_init(&sem_enter, 0, 0);
		sem_init(&sem_exit, 0, 0);
		memset(&thread, 0, sizeof(thread));
		sprintf(buf, "I/O Abort");
		errs	+= os_thread_create(&thread, buf, _read_thread, NULL);

		if (errs == 0)
		{
			// Wait for the thread to become waiting.
			sem_wait(&sem_enter);

			// Cause the I/O to abort.
			errs	+= ai64ssa_rx_io_abort	(fd, -1, 0, &aborted);

			// Wait for the resumed thread to run.
			sem_wait(&sem_exit);

			errs	+= os_thread_destroy(&thread);
		}
	}

	return(errs);
}



//*****************************************************************************
static int _io_rx_abort(int fd)
{
	int	errs;

	gsc_label("I/O Rx Abort");
	errs	= wait_test(fd, 0, 0, AI64SSA_WAIT_IO_RX_ABORT, _abort);
	return(errs);
}



//*****************************************************************************
static int _done(int fd)
{
	char	buf[1024];
	int		errs	= 0;

	errs	+= ai64ssa_initialize	(fd, -1, 0);
	errs	+= ai64ssa_rx_io_timeout(fd, -1, 0, 1, NULL);
	ai64ssa_read(fd, buf, sizeof(buf));
	return(errs);
}



//*****************************************************************************
static int _io_rx_done(int fd)
{
	int	errs;

	gsc_label("I/O Rx Done");
	errs	= wait_test(fd, 0, 0, AI64SSA_WAIT_IO_RX_DONE, _done);
	return(errs);
}



//*****************************************************************************
static int _error(int fd)
{
	int	errs;

	errs	= ai64ssa_initialize(fd, -1, 0);
	ai64ssa_read(fd, NULL, 3);
	return(errs);
}



//*****************************************************************************
static int _io_rx_error(int fd)
{
	int	errs	= 0;

	gsc_label("I/O Rx Error");
	errs	+= wait_test(fd, 0, 0, AI64SSA_WAIT_IO_RX_DONE, _error);
	return(errs);
}



//*****************************************************************************
int wait_io_test(int fd)
{
	int	errs	= 0;

	gsc_label("Wait I/O Activity");
	printf("\n");
	gsc_label_level_inc();

	errs	+= _io_rx_abort(fd);
	errs	+= _io_rx_done(fd);
	errs	+= _io_rx_error(fd);

	gsc_label_level_dec();
	return(errs);
}


