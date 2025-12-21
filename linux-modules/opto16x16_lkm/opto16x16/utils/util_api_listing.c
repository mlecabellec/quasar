// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_api_listing.c $
// $Rev: 51397 $
// $Date: 2022-07-14 13:14:08 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Utilities: source file

#include "main.h"



#if 0	// for file format examination
if (verbose
if (verbose
#endif



//*****************************************************************************
static int _ioctl_settings(int fd)
{
	int			errs	= 0;
	gsc_wait_t	wait;

	errs	+= opto16x16_clock_divider		(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_CLOCK_DIVIDER
	errs	+= opto16x16_cos_polarity		(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_COS_POLARITY
	errs	+= opto16x16_debounce_ms		(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_DEBOUNCE_MS
	errs	+= opto16x16_debounce_us		(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_DEBOUNCE_US
	errs	+= opto16x16_irq_enable			(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_IRQ_ENABLE
	errs	+= opto16x16_led				(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_LED
	errs	+= opto16x16_rx_data			(fd, -1, 1,		NULL);	// OPTO16X16_IOCTL_RX_DATA
	errs	+= opto16x16_rx_event_counter	(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_RX_EVENT_COUNTER
	errs	+= opto16x16_tx_data			(fd, -1, 1, -1, NULL);	// OPTO16X16_IOCTL_TX_DATA

	wait.flags		= 0;
	wait.main		= GSC_WAIT_MAIN_ALL;
	wait.gsc		= OPTO16X16_WAIT_GSC_ALL;
	wait.alt		= OPTO16X16_WAIT_ALT_ALL;
	wait.io			= OPTO16X16_WAIT_IO_ALL;
	wait.timeout_ms	= 0;
	wait.count		= 0;

	errs	+= opto16x16_wait_status		(fd, -1, 1, &wait);		// OPTO16X16_IOCTL_WAIT_STATUS

	return(errs);
}



//*****************************************************************************
int opto16x16_api_listing(int fd)
{
	int	errs	= 0;

	printf("\n");
	printf("==== API LISTING: BEGIN =============================================\n");

	errs	+= _ioctl_settings(fd);
	printf("\n");
	errs	+= opto16x16_reg_list(fd, 1);

	printf("==== API LISTING: END ===============================================\n");
	printf("\n");
	return(errs);
}



