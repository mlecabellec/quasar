// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/wait/main.h $
// $Rev: 53553 $
// $Date: 2023-08-07 14:25:13 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include "16ai64ssa_main.h"



// macros *********************************************************************

#define	SIZEOF_ARRAY(a)			(sizeof((a)) / sizeof((a)[0]))



// data types *****************************************************************

typedef struct
{
	// Application Settings

	s32	continuous;		// > 0 = ignore errors, < 0 = stop on errors
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);

int	wait_gsc_test(int fd);
int	wait_io_test(int fd);
int	wait_irq0_initialize_test(int fd);
int	wait_irq0_autocal_test(int fd);
int	wait_irq0_sync_start_test(int fd);
int	wait_irq0_sync_done_test(int fd);
int	wait_irq0_burst_start_test(int fd);
int	wait_irq0_burst_done_test(int fd);
int	wait_irq1_in_buf_thr_l2h_test(int fd);
int	wait_irq1_in_buf_thr_h2l_test(int fd);
int	wait_irq1_in_buf_error_test(int fd);
int	wait_main_test(int fd);
int	wait_test(int fd, u32 main, u32 gsc, u32 io, int (*callback)(int fd));



#endif
