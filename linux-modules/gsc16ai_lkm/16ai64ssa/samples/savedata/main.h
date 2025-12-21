// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/savedata/main.h $
// $Rev: 54968 $
// $Date: 2024-08-07 15:57:37 -0500 (Wed, 07 Aug 2024) $

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

#define	CHAN_TAG_EXCLUDE		1
#define	CHAN_TAG_ONLY			2

#define	FORMAT_HEX				0
#define	FORMAT_DEC				1
#define	FORMAT_DEC_0			2



// data types *****************************************************************

typedef struct
{
	// Application Settings

	int		argc;
	char**	argv;

	s32	chan_tag;		// Channel tag processing.
	s32	continuous;		// > 0 = ignore errors, < 0 = stop on errors
	s32	format;			// Save data in this format.
	s32	force_save;		// Save data even if there are errors?
	s32	fsamp;			// Desired sample rate.
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	repeat;			// repeat data retrieval this many times
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for device to access.

	// Device Settings

	s32	ai_mode;		// Analog Input Mode
	s32	chan_active;
	s32	chan_first;
	s32	chan_last;
	s32	chan_single;
	s32	io_mode;
	s32	v_range;		// AI64SSA_AI_RANGE_XXXX
} args_t;



// prototypes *****************************************************************

int	perform_tests(const args_t* args);



#endif
