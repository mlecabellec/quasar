// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/perform.c $
// $Rev: 53724 $
// $Date: 2023-09-14 10:41:02 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;

	errs	+= reg_read_test(args->fd);			// OPTO16X16_IOCTL_REG_READ
	errs	+= reg_write_test(args->fd);		// OPTO16X16_IOCTL_REG_WRITE
	errs	+= reg_mod_test(args->fd);			// OPTO16X16_IOCTL_REG_MOD
	errs	+= query_test(args->fd);			// OPTO16X16_IOCTL_QUERY
	errs	+= initialize_test(args->fd);		// OPTO16X16_IOCTL_INITIALIZE
	errs	+= clock_divider_test(args->fd);	// OPTO16X16_IOCTL_CLOCK_DIVIDER
	errs	+= cos_polarity_test(args->fd);		// OPTO16X16_IOCTL_COS_POLARITY
	errs	+= debounce_ms_test(args->fd);		// OPTO16X16_IOCTL_DEBOUNCE_MS
	errs	+= debounce_us_test(args->fd);		// OPTO16X16_IOCTL_DEBOUNCE_US
	errs	+= irq_enable_test(args->fd);		// OPTO16X16_IOCTL_IRQ_ENABLE
	errs	+= led_test(args->fd);				// OPTO16X16_IOCTL_LED
	errs	+= rx_data_test(args->fd);			// OPTO16X16_IOCTL_RX_DATA
	errs	+= rx_event_counter_test(args->fd);	// OPTO16X16_IOCTL_RX_EVENT_COUNTER
	errs	+= tx_data_test(args->fd);			// OPTO16X16_IOCTL_TX_DATA
	errs	+= wait_event_test(args->fd);		// OPTO16X16_IOCTL_WAIT_EVENT
	errs	+= wait_cancel_test(args->fd);		// OPTO16X16_IOCTL_WAIT_CANCEL
	errs	+= wait_status_test(args->fd);		// OPTO16X16_IOCTL_WAIT_STATUS

	return(errs);
}


