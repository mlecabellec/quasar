// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/signals/main.h $
// $Rev: 53549 $
// $Date: 2023-08-07 14:23:19 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
	s32	seconds;		// Output the signals for this number of seconds.
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);



#endif
