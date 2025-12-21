// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/tftest/main.h $
// $Rev: 53725 $
// $Date: 2023-09-14 10:41:18 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "opto16x16_main.h"



// macros *********************************************************************

#define	SIZEOF_ARRAY(a)			(sizeof((a))/sizeof((a)[0]))



// data types *****************************************************************

typedef struct
{
	// Application Settings

	s32	continuous;		// > 0 = ignore errors, < 0 = stop on errors
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for the device to access.

	// Device Settings

} args_t;

// prototypes *****************************************************************

int	d00_test(int fd);
int	d01_test(int fd);
int	d02_test(int fd);
int	d03_test(int fd);
int	d04_test(int fd);
int	d05_test(int fd);
int	d06_test(int fd);
int	d07_test(int fd);
int	d08_test(int fd);
int	d09_test(int fd);
int	d10_test(int fd);
int	d11_test(int fd);
int	d12_test(int fd);
int	d13_test(int fd);
int	d14_test(int fd);
int	d15_test(int fd);
int	dxx_test(int fd, int index);

int	perform_tests(const args_t* args);

int	rx_event_counter_test(int fd);



#endif
