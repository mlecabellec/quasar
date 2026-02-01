// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/main.h $
// $Rev: 54970 $
// $Date: 2024-08-07 15:58:34 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "16ai64ssa_main.h"



// macros *********************************************************************

#define	SIZEOF_ARRAY(a)				(sizeof((a)) / sizeof((a)[0]))

#define	RX_OPTION_ZERO_DATA			0
#define	RX_OPTION_READ_DEV			1

#define	TX_OPTION_BIT_BUCKET		0
#define	TX_OPTION_WRITE_FILE_BIN	1
#define	TX_OPTION_WRITE_FILE_TEXT	2

#define	_1M							(1L * 1024 * 1024)

#define	BUFFER_QTY					4
#define	BUFFER_SIZE					_1M

#define	CHAN_TAG_EXCLUDE			1
#define	CHAN_TAG_ONLY				2



// data types *****************************************************************

typedef struct		// We store this inside the gsc_buf_man_t.user_area.
{
	os_time_ns_t	timestamp;
} bm_user_area_t;

typedef struct
{
	s32				chan_qty;		// Set by controlling thread to number of channels on device.
	double			fsamp;			// Set by controlling thread to actual sample rate.
	int				start;			// set by controlling thread to request that we start processing
	int				stop;			// set by controlling thread to request that we stop processing
	os_thread_t		thread;			// Set by controlling thread when thread is created

	int				done;			// set by new thread to report that it is done
	int				errs;			// Set by new thread to report any errors
	char			err_buf[2048];	// Set by new thread to describe error condition.
	s32				reading;		// set by new thread to indicate data Rx has started.
	os_time_ns_t	start_time;		// Set by new thread to indicate when Rx processing started.
	int				started;		// set by new thread to indicate it has started
	long long		total_bytes;	// Set by new thread for accounting purposes
	long			total_ms;		// Set by new thread for accounting purposes
} rx_data_t;

typedef struct
{
	s32				chan_qty;		// Set by controlling thread to number of channels on device.
	FILE*			file;			// Set by controlling thread for data destination.
	int				start;			// set by controlling thread to request that we start processing
	int				stop;			// set by controlling thread to request that we stop processing
	os_thread_t		thread;			// Set by controlling thread when thread is created

	int				done;			// set by new thread to report that it is done
	int				errs;			// Set by new thread to report any errors
	char			err_buf[2048];	// Set by new thread to describe error condition.
	long			file_samples;	// Set by new thread for accounting purposes.
	char			name[256];		// Set by new thread for data destination.
	int				started;		// set by new thread to indicate it has started
	long long		total_bytes;	// Set by new thread for accounting purposes
	long			total_ms;		// Set by new thread for accounting purposes
} tx_data_t;

typedef struct
{
	// Application Settings

	s32	context_info;	// Add context info to the file name?
	s32	continuous;		// > 0 = ignore errors, < 0 = stop on errors
	s32	delay_test_s;	// Delay between repeated tests in seconds.
	s32	file_samples;	// Limit file sizes to # samples.
	s32	force;			// Force operation even if errors occur.
	s32	index;			// device index
	s32	minute_limit;	// for continuous testing
	int	qty;			// Number of devices detected.
	s32	rx_mb;			// Read a minimum of # megabytes of data.
	s32	rx_option;
	s32	rx_seconds;		// Read for # seconds.
	s32	stats;
	s32	test_limit;		// for continuous testing
	int	tests;			// Test number for continuous testing.
	s32	tx_decimal;		// Save .txt file content as decimal. Otherwise as hex.
	s32	tx_option;
	s32	tx_chan_tag;	// Channel Tag: exclude or only
	s32	tx_validate;

	int	fd;				// File descriptor for device to access.

	// Device Settings

	s32	fsamp;
	s32	io_mode;

	// Additional Fields
	rx_data_t*	rx;
	tx_data_t*	tx;
} args_t;



// prototypes *****************************************************************

int		perform_tests(const args_t* args);

int		rx_start(const args_t* args);
int		rx_stop(const args_t* args);

int		tx_start(const args_t* args);
int		tx_stop(const args_t* args);
void	tx_validation_results(void);



#endif
