// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/rxrate/main.h $
// $Rev: 54967 $
// $Date: 2024-08-07 15:57:16 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

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
	s32	fsamp;			// The desired sample rate.
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	rx_mb;			// MB to receive
	s32	seconds;		// Read data for at least this long.
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

	s32	data_packing;
	s32	io_mode;
	s32	scan_marker;
} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);



#endif
