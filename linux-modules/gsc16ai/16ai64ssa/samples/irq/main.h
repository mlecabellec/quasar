// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/irq/main.h $
// $Rev: 53544 $
// $Date: 2023-08-07 14:21:13 -0500 (Mon, 07 Aug 2023) $

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

int	irq_0_test(int fd);
int	irq_1_test(int fd);
int	irq_main_test(int fd);
int	irq_wait_test(int fd, u32 main, u32 gsc, u32 io, int (*callback)(int fd));

int	perform_tests(const args_t* args);



#endif
