// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/fsamp/main.h $
// $Rev: 54956 $
// $Date: 2024-08-07 15:51:58 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
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
	s32	hold_secs;		// Apply the settings and hold it for # seconds.
	s32	hold_reg_dump;	// Generate a detailed register dump after each hold.
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	range;			// Go from Fsamp min to max.
	s32	range_begin;	// Begin range at # S/S.
	s32	range_end;		// End range at # S/S.
	s32	save;			// Save scan data to disk?
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);



#endif
