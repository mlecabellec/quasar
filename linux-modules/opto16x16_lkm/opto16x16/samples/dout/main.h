// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/dout/main.h $
// $Rev: 53721 $
// $Date: 2023-09-14 10:39:36 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "opto16x16_main.h"



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
	s32	ms_wait;		// Wait this long after each output.
	s32	pause;
	s32	repeat;
	s32	share;
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);



#endif
