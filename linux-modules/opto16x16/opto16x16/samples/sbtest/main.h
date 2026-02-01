// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/main.h $
// $Rev: 53724 $
// $Date: 2023-09-14 10:41:02 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

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
	s32	test_limit;		// for continuous testing

	int	fd;				// File descriptor for the device to access.

	// Device Settings

} args_t;

typedef enum
{
	SERVICE_END_LIST,	// Ends list of service_data_t structures.
	SERVICE_NONE,		// Do nothing.
	SERVICE_NORMAL,
	SERVICE_REG_MOD,
	SERVICE_REG_READ,
	SERVICE_REG_SHOW,
	SERVICE_REG_TEST,
	SERVICE_REG_WRITE,	// arg = # of times to write
	SERVICE_SLEEP,		// arg = # of seconds to sleep
	SERVICE_SLEEP_MS,	// arg = # of milliseconds to sleep
	SERVICE_IOCTL_GET,
	SERVICE_IOCTL_SET
} service_t;

typedef struct
{
	service_t		service;
	unsigned long	cmd;	// IOCTL code
	s32				arg;	// The IOCTL data argument.
	u32				reg;	// The register to access. Use -1 to ignore.
	u32				mask;	// The register bits of interest.
	u32				value;	// The value expected for the bits of interest.
} service_data_t;



// prototypes *****************************************************************

int	clock_divider_test(int fd);			// OPTO16X16_IOCTL_CLOCK_DIVIDER
int	cos_polarity_test(int fd);			// OPTO16X16_IOCTL_COS_POLARITY

int	debounce_ms_test(int fd);			// OPTO16X16_IOCTL_DEBOUNCE_MS
int	debounce_us_test(int fd);			// OPTO16X16_IOCTL_DEBOUNCE_US

int	initialize_test(int fd);			// OPTO16X16_IOCTL_INITIALIZE
int	irq_enable_test(int fd);			// OPTO16X16_IOCTL_IRQ_ENABLE

int	led_test(int fd);					// OPTO16X16_IOCTL_LED

int	perform_tests(const args_t* args);

int	query_test(int fd);					// OPTO16X16_IOCTL_QUERY

int	reg_mod_test(int fd);				// OPTO16X16_IOCTL_REG_MOD
int	reg_read_test(int fd);				// OPTO16X16_IOCTL_REG_READ
int	reg_write_test(int fd);				// OPTO16X16_IOCTL_REG_WRITE
int	rx_data_test(int fd);				// OPTO16X16_IOCTL_RX_DATA
int	rx_event_counter_test(int fd);		// OPTO16X16_IOCTL_RX_EVENT_COUNTER

int	service_ioctl_reg_get_list(int fd, const service_data_t* list);
int	service_ioctl_set_list(int fd, const service_data_t* list);
int	service_ioctl_set_get_list(int fd, const service_data_t* list);
int	service_ioctl_set_reg_list(int fd, const service_data_t* list);

int	tx_data_test(int fd);				// OPTO16X16_IOCTL_TX_DATA

int	wait_cancel_test(int fd);			// OPTO16X16_IOCTL_WAIT_CANCEL
int	wait_event_test(int fd);			// OPTO16X16_IOCTL_WAIT_EVENT
int	wait_status_test(int fd);			// OPTO16X16_IOCTL_WAIT_STATUS



#endif
