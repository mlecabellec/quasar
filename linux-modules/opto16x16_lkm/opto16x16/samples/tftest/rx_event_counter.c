// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/tftest/rx_event_counter.c $
// $Rev: 51415 $
// $Date: 2022-07-14 14:35:19 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



// variables ******************************************************************

static	os_sem_t	sem_enter;
static	os_sem_t	sem_exit;
static	int			wait_fd;



//*****************************************************************************
static int _wait_thread(void* arg)
{
	int			ret;
	gsc_wait_t*	wait	= (void*) arg;

	os_sem_unlock(&sem_enter);
	ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_EVENT, wait);

	if (ret)
		wait->flags	= 0xFFFFFFFF;

	os_sem_unlock(&sem_exit);
	return(0);
}



//*****************************************************************************
static int _wait_for_thread_start(int fd)
{
	int			errs	= 0;
	int			i;
	int			ret;
	gsc_wait_t	wait;

	// Wait for the thread to become waiting.
	os_sem_lock(&sem_enter);
	memset(&wait, 0, sizeof(wait));
	wait.gsc	= 0x10000;

	for (i = 0; i < 100; i++)
	{
		ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_STATUS, &wait);

		if (ret < 0)
		{
			printf(	"FAIL <---  (%d. status request 0x%lX: error %d)\n",
					__LINE__,
					(long) 0x10000,
					ret);
			errs++;
			break;
		}

		if (wait.count == 0)
		{
			os_sleep_ms(100);
			continue;
		}

		if (wait.count == 1)
			break;

		errs++;
		printf(	"FAIL <---  (%d. invalid wait count %ld)\n",
				__LINE__,
				(long) wait.count);
		break;
	}

	if ((errs == 0) && (wait.count != 1))
	{
		errs++;
		printf("FAIL <---  (%d. thread did not start)\n", __LINE__);
	}

	return(errs);
}



//*****************************************************************************
static int _wait_for_thread_end(int fd)
{
	int			errs	= 0;
	int			i;
	int			ret;
	gsc_wait_t	wait;

	// Wait for the thread to stop waiting.
	os_sem_lock(&sem_exit);
	memset(&wait, 0, sizeof(wait));
	wait.gsc	= 0x10000;

	for (i = 0; i < 100; i++)
	{
		ret	= opto16x16_ioctl(wait_fd, OPTO16X16_IOCTL_WAIT_STATUS, &wait);

		if (ret < 0)
		{
			printf(	"FAIL <---  (%d. status request 0x%lX: error %d)\n",
					__LINE__,
					(long) 0x10000,
					ret);
			errs++;
			break;
		}

		if (wait.count == 0)
			break;

		if (wait.count == 1)
		{
			os_sleep_ms(100);
			continue;
		}

		errs++;
		printf(	"FAIL <---  (%d. invalid wait count %ld)\n",
				__LINE__,
				(long) wait.count);
		break;
	}

	if ((errs == 0) && (wait.count != 0))
	{
		errs++;
		printf("FAIL <---  (%d. thread did not end)\n", __LINE__);
	}

	return(errs);
}



//*****************************************************************************
static int _led_on(int fd)
{
	int	errs;
	int	i;
	int	ret;
	s32	v;

	v		= 0x8000;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_TX_DATA, &v);
	errs	= ret ? 1 : 0;

	// Wait for upto 200ms for the LED to come on.

	for (i = 0; (errs == 0) && (i < 20); i++)
	{
		os_sleep_ms(100);
		ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_RX_DATA, &v);
		errs	+= ret ? 1 : 0;

		if (v & 0x8000)
			break;
	}

	return(errs);
}



//*****************************************************************************
static int _led_off(int fd)
{
	int	errs;
	int	i;
	int	ret;
	s32	v;

	v		= 0x0000;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_TX_DATA, &v);
	errs	= ret ? 1 : 0;

	// Wait for upto 200ms for the LED to go off.

	for (i = 0; (errs == 0) && (i < 20); i++)
	{
		os_sleep_ms(100);
		ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_RX_DATA, &v);
		errs	+= ret ? 1 : 0;

		if ((v & 0x8000) == 0)
			break;
	}

	return(errs);
}



//*****************************************************************************
static int _irq_initiate(int fd)
{
	int			errs	= 0;
	int			i;
	int			ret;
	s32			v;
	gsc_wait_t	wait;

	memset(&wait, 0, sizeof(wait));

	// Configure the Debounce Period.
	v		= 150;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_DEBOUNCE_MS, &v);
	errs	+= ret ? 1 : 0;

	// Set the event counter.
	v		= 0xFFFA;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_RX_EVENT_COUNTER, &v);
	errs	+= ret ? 1 : 0;

	// Enable the interrupt.
	v		= 0x10000;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_IRQ_ENABLE, &v);
	errs	+= ret ? 1 : 0;

	// Drive the output.
	// On the test fixture a "1" will illuminate the corresponding LED.
	// If the test fixture is configured for loopback operation, this
	// will also cause an event increment.
	// If the test fixture is not configured for loopback operation,
	// the operator must depress the switch when the LED comes on and
	// must release it when the LED goes off.

	for (i = 0; (errs == 0) && (i < 100); i++)
	{
		errs	+= _led_on(fd);

		if (errs == 0)
			errs	+= _led_off(fd);

		memset(&wait, 0, sizeof(wait));
		wait.gsc	= 0x10000;

		if (errs == 0)
		{
			ret		= opto16x16_ioctl(	fd,
										OPTO16X16_IOCTL_WAIT_STATUS,
										&wait);
			errs	+= ret ? 1 : 0;

			if (wait.count == 0)
				break;
		}
	}

	if ((errs == 0) && (wait.count))
	{
		errs++;
		printf("FAIL <---  (%d. interrupt not generated)\n", __LINE__);
	}

	return(errs);
}



//*****************************************************************************
static int _process_irq(int fd, gsc_wait_t* wait)
{
	char		buf[64];
	int			errs	= 0;
	int			ret;
	os_thread_t	thread;

	wait_fd	= fd;
	os_sem_create_qty(&sem_enter, 1, 0);
	os_sem_create_qty(&sem_exit, 1, 0);

	wait->flags			= 0;
	wait->main			= 0;
	wait->gsc			= 0x10000;
	wait->alt			= 0;
	wait->io			= 0;
	wait->timeout_ms	= 10000;
	wait->count			= 0;

	memset(&thread, 0, sizeof(thread));
	sprintf(buf, "wait event 0x%lX", (long) 0x10000);
	errs	+= os_thread_create(&thread, buf, _wait_thread, wait);

	if (errs == 0)
		errs	= _wait_for_thread_start(fd);

	if (wait->flags == 0xFFFFFFFF)
			errs++;

	if (errs == 0)
		errs	+= _irq_initiate(fd);

	if (errs == 0)
		errs	+= _wait_for_thread_end(fd);

	if ((errs == 0) && (wait->flags != GSC_WAIT_FLAG_DONE))
	{
		printf(	"FAIL <---  (%d. flag: expect 0x%lX, got 0x%lX)\n",
				__LINE__,
				(long) GSC_WAIT_FLAG_DONE,
				(long) wait->flags);
		errs++;
	}

	// Force termination, just in case of an error.
	wait->flags			= 0;
	wait->main			= 0xFFFFFFFF;
	wait->gsc			= 0xFFFFFFFF;
	wait->alt			= 0xFFFFFFFF;
	wait->io			= 0xFFFFFFFF;
	wait->timeout_ms	= 0;
	wait->count			= 0;
	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_WAIT_CANCEL, &wait);
	errs	+= ret ? 1 : 0;
	errs	+= os_thread_destroy(&thread);
	os_sem_destroy(&sem_enter);
	os_sem_destroy(&sem_exit);

	return(errs);
}



//*****************************************************************************
int	rx_event_counter_test(int fd)
{
	int			errs	= 0;
	gsc_wait_t	wait;

	gsc_label("Rx Event Counter");

	errs	+= opto16x16_initialize(fd, -1, 0);
	errs	+= _process_irq(fd, &wait);
	errs	+= opto16x16_initialize(fd, -1, 0);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


