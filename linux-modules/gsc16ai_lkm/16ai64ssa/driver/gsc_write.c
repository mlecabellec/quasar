// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_write.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// macros *********************************************************************

#if defined(DEV_SUPPORTS_WRITE)

#ifndef	GSC_WRITE_PIO_PER_1MS
#define	GSC_WRITE_PIO_PER_1MS				250
#endif

#if (GSC_WRITE_PIO_PER_1MS > 1000)
#undef	GSC_WRITE_PIO_PER_1MS
#define	GSC_WRITE_PIO_PER_1MS				1000
#endif

// In previous code, when no space was available we immediately called the sleep
// service. Now we check for space multiple times before resorting to the sleep.
// The number of times we attempt this is based on the board's average register
// read time (in ns) and the maximum amount of time we're willing to spend
// checking for available space.


#if 0	// experimental code

// This is experimental code!

// Tests were performed using the SIO4's Channel-to-Channel applications as of
// 12/10/2021. Those tests revealed that this code provided no significant
// improvement when utilized for the following cases.
//
// 50us period with 2777 ns read time
// 50us period with 2010 ns read time
// 100us period with 2010 ns read time
//
// No other settings were used and no other devices were tested.
// This code may be resurected in the future if a useful case arrises.

#define	GSC_WRITE_SPACE_CHECK_NS			(2010)	// SIO4

#ifndef	GSC_WRITE_SPACE_CHECK_NS
// This value is the median taken from all test_all logs having metrics content
// for performing a GSC register memory read, as of 12/10/2021.
// Obviously some devices are faster and some are slower. But, in the absence
// of a device specific value we're making an assumption for both the device
// and the current host.
#define	GSC_WRITE_SPACE_CHECK_NS			(2777)
#endif

#if (GSC_WRITE_SPACE_CHECK_NS <= 0)
#define	GSC_WRITE_SPACE_CHECK_QTY			(1)
#else
// The numerator here refers to 50,000 ns, or 50 us, which is the maximum
// amount of time being permitted for looping while checking for available
// space. Given the variability from different devices and different hosts the
// actual looping time may be shorter or longer.
#define	GSC_WRITE_SPACE_CHECK_QTY			(50000 / GSC_WRITE_SPACE_CHECK_NS)
#endif

#endif	// experimental code

#endif



/******************************************************************************
*
*	Function:	_write_work
*
*	Purpose:
*
*		Implement the mode independent working portion of the write()
*		procedure. If a timeout is permitted and called for, we wait a single
*		timer tick and check again.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		usr_buf	The bytes to write come from here.
*
*		count	The number of bytes requested. This is positive.
*
*		st_end	Timeout at this point in time (in system ticks).
*				Zero means never timeout.
*
*		fn_aval	This is the mode specific function that determines the number
*				of bytes that may be written.
*
*		fn_xfer	This is the function implementing the mode specific write
*				functionality.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
static long _write_work(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const char*			usr_buf,
	size_t				count,	// bytes
	os_time_tick_t		st_end,	// The "system tick" time that we timeout. Zero = never timeout.
	long				(*fn_aval)(
							GSC_ALT_STRUCT_T*	alt,
							dev_io_t*			io,
							size_t				count),	// bytes
	long				(*fn_xfer)(
							GSC_ALT_STRUCT_T*	alt,
							dev_io_t*			io,
							const os_mem_t*		mem,
							size_t				count,	// bytes
							os_time_tick_t		st_end))
{
	os_mem_t	mem;
	int			ret;
	long		total		= 0;	// The number of bytes transferred.
	long		want		= 0;	// Bytes we want to write this time.
	long		xfer_did;
	long		xfer_do;

	memset(&mem, 0, sizeof(mem));

	for (;;)
	{
		if (want <= 0)
		{
			// Transfer one block at a time.
			mem	= io->mem;

			if (count > mem.bytes)
				want	= mem.bytes;
			else
				want	= (long) count;

			mem.ptr		= io->mem.ptr;
			ret			= os_mem_copy_from_user_tx(mem.ptr, usr_buf, want);
			usr_buf		+= want;

			if (ret)
			{
				// We couldn't get user data.
				total	= -EFAULT;
				gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
				printf(	"%s: %d. %s: copy from app memory failed.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}
		}

		// See how much data we can transfer.
		xfer_do	= (fn_aval)(alt, io, count);

		if (xfer_do > (long) mem.bytes)
			xfer_do	= (long) mem.bytes;	// The size of the transfer buffer.

		if (xfer_do > want)
			xfer_do	= want;			// Try to complete the request.

		// Either transfer the data or see what to do next.

		if (io->abort)
		{
			// We've been told to quit.
			io->abort	= 0;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.abort);
			break;
		}

		if (xfer_do > 0)
			xfer_did	= (fn_xfer)(alt, io, &mem, xfer_do, st_end);
		else
			xfer_did	= 0;

		if (xfer_did < 0)
		{
			// There was a problem.
			total	= xfer_did;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			break;
		}
		else if (xfer_did > 0)
		{
			// House keeping.
			count		-= xfer_did;
			total		+= xfer_did;
			want		-= xfer_did;
			mem.ptr		= (char*) mem.ptr + xfer_did;
			mem.bytes	-= xfer_did;
			mem.adrs	+= xfer_did;
		}

		// Now what do we do?

		if (count == 0)
		{
			// The request has been fulfilled.
			gsc_wait_resume_io(alt, io->wait.done);
			break;
		}
		else if (io->non_blocking)
		{
			// We can't block.

			if (xfer_did > 0)
			{
				// See if we can transfer more data.
				continue;
			}
			else
			{
				// We can't wait to transfer more data.
				// We have essentially timed out.
				gsc_wait_resume_io(alt, io->wait.done | io->wait.timeout);
				break;
			}
		}
		else if ((st_end) && (os_time_tick_timedout(st_end)))
		{
			// We've timed out.
			gsc_wait_resume_io(alt, io->wait.done | io->wait.timeout);
			break;
		}
		else if (xfer_did)
		{
			// Some data was transferred, so go back for more.
			continue;
		}

		if (io->abort)
		{
			// We've been told to quit.
			io->abort	= 0;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.abort);
			break;
		}

		// Wait for 1 timer tick before checking again.
		ret	= os_time_tick_sleep(0);

		if (ret)
		{
			// The timeout returned prematurely; it was signaled.
			// We've essentially been told to quit.
			gsc_wait_resume_io(alt, io->wait.done | io->wait.abort);
			break;
		}
	}

	return(total);
}
#endif



/******************************************************************************
*
*	Function:	gsc_write
*
*	Purpose:
*
*		Implement the write() procedure. Only one call active at a time.
*
*	Arguments:
*
*		alt		The structure for the device to write to.
*
*		io		The I/O structure for the stream to access.
*
*		buf		The data requested goes here.
*
*		count	The number of bytes requested. The I/O stream selector is
*				encoded in the 0xF0000000 nibble.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
long gsc_write(GSC_ALT_STRUCT_T* alt, dev_io_t* io, const void* buf, size_t count)
{
	long			ret;
	size_t			samples;
	os_time_tick_t	st_end;	// The "system tick" time that we timeout.

	for (;;)	// We'll use a loop for convenience.
	{
		// Gain exclusive access to the device structure.
		ret	= os_sem_lock(&alt->sem);

		if (ret)
		{
			// We didn't get the lock.
			printf(	"%s: %d. %s: failed to acquire lock.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		// Perform argument validation.

		if (count <= 0)
		{
			// There is no work to do.
			ret	= 0;
			gsc_wait_resume_io(alt, io->wait.done);
			os_sem_unlock(&alt->sem);
			break;
		}

		if (io->bytes_per_sample == 4)
		{
		}
		else if (io->bytes_per_sample == 2)
		{
		}
		else if (io->bytes_per_sample == 1)
		{
		}
		else
		{
			// This is an internal error.
			ret	= -EINVAL;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			os_sem_unlock(&alt->sem);
			printf(	"%s: %d. %s: invalid 'bytes_per_sample': %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) io->bytes_per_sample);
			break;
		}

		if (count % io->bytes_per_sample)
		{
			// Requests must be in sample size increments.
			ret	= -EINVAL;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			os_sem_unlock(&alt->sem);
			printf(	"%s: %d. %s: 'count' (%d) not a multiple of 'bytes_per_sample' (%d).\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) count,
					(int) io->bytes_per_sample);
			break;
		}

		if (buf == NULL)
		{
			// No buffer provided.
			ret	= -EINVAL;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			os_sem_unlock(&alt->sem);
			printf(	"%s: %d. %s: 'buf' is NULL.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		// Compute the I/O timeout end.

		if (io->timeout_s >= GSC_IO_TIMEOUT_INFINITE)
		{
			st_end	= 0;
		}
		else
		{
			st_end	= os_time_tick_get() + io->timeout_s * os_time_tick_rate();

			if (st_end == 0)
				st_end	= 1;
		}

		// Transfer access control to the read lock.
		ret	= os_sem_lock(&io->sem);

		if (ret)
		{
			// We didn't get the lock.
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			os_sem_unlock(&alt->sem);
			printf(	"%s: %d. %s: failed to acquire lock.\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__);
			break;
		}

		os_sem_unlock(&alt->sem);
		ret	= io->dev_io_startup(alt, io);

		if (ret)
		{
			// There was a problem.
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			os_sem_unlock(&io->sem);
			break;
		}

		// Perform the operation.
		samples	= count / io->bytes_per_sample;

		if ((samples <= (size_t) io->pio_threshold) ||
			(io->io_mode == GSC_IO_MODE_PIO))
		{
			ret	= _write_work(	alt,
								io,
								buf,
								count,	// bytes
								st_end,
								io->dev_pio_available,
								io->dev_pio_xfer);
		}
		else if (io->io_mode == GSC_IO_MODE_BMDMA)
		{
			ret	= _write_work(	alt,
								io,
								buf,
								count,	// bytes
								st_end,
								io->dev_bmdma_available,
								io->dev_bmdma_xfer);
		}
		else if (io->io_mode == GSC_IO_MODE_DMDMA)
		{
			ret	= _write_work(	alt,
								io,
								buf,
								count,	// bytes
								st_end,
								io->dev_dmdma_available,
								io->dev_dmdma_xfer);
		}
		else
		{
			ret	= -EINVAL;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			printf(	"%s: %d. %s: invalid 'io_mode': %d\n",
					DEV_NAME,
					__LINE__,
					__FUNCTION__,
					(int) io->io_mode);
		}

		//	Clean up.

		os_sem_unlock(&io->sem);
		break;
	}

	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_write_abort_active_xfer
*
*	Purpose:
*
*		Abort an active write.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		io		The I/O structure for the stream to access.
*
*	Returned:
*
*		1		An active write was aborted.
*		0		A write was not in progress.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
int gsc_write_abort_active_xfer(GSC_ALT_STRUCT_T* alt, dev_io_t* io)
{
	dev_data_t*	dev	= GSC_ALT_DEV_GET(alt);
	int			i;
	int			ret;

	io->abort	= 1;
	ret			= gsc_dma_abort(dev, io);
	i			= os_sem_lock(&io->sem);
	ret			= ret ? ret : (io->abort ? 0 : 1);
	io->abort	= 0;

	if (i == 0)
	{
		os_sem_unlock(&io->sem);
	}
	else
	{
		printf(	"%s: %d. %s: failed to acquire lock.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
	}

	return(ret);
}
#endif



/******************************************************************************
*
*	Function:	gsc_write_pio_work_8_bit
*
*	Purpose:
*
*		Perform PIO based writes of 8-bit values.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The source for the data to write.
*
*		count	The number of bytes to write.
*
*		st_end	The "system ticks" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
#if defined(GSC_WRITE_PIO_WORK) || defined(GSC_WRITE_PIO_WORK_8_BIT)
long gsc_write_pio_work_8_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long		qty			= (long) count;
	VADDR_T		dst			= io->io_reg_vaddr;
	int			interval	= 0;
	u8*			src			= (u8*) mem->ptr;

	for (; count > 0; )
	{
		count	-= 1;
		os_reg_mem_tx_u8(NULL, dst, src[0]);
		src++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a write takes about 1us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_WRITE_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more space is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_write_pio_work_16_bit
*
*	Purpose:
*
*		Perform PIO based writes of 16-bit values.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The source for the data to write.
*
*		count	The number of bytes to write.
*
*		st_end	The "system tick" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
#if defined(GSC_WRITE_PIO_WORK) || defined(GSC_WRITE_PIO_WORK_16_BIT)
long gsc_write_pio_work_16_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long		qty			= (long) count;
	VADDR_T		dst			= io->io_reg_vaddr;
	int			interval	= 0;
	u16*		src			= (u16*) mem->ptr;

	for (; count > 0; )
	{
		count	-= 2;
		os_reg_mem_tx_u16(NULL, dst, src[0]);
		src++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a write takes about 1us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_WRITE_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more space is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_write_pio_work_32_bit
*
*	Purpose:
*
*		Perform PIO based writes of 32-bit values.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The source for the data to write.
*
*		count	The number of bytes to write.
*
*		st_end	The "system tick" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
#if defined(GSC_WRITE_PIO_WORK) || defined(GSC_WRITE_PIO_WORK_32_BIT)
long gsc_write_pio_work_32_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long		qty			= (long) count;
	VADDR_T		dst			= io->io_reg_vaddr;
	int			interval	= 0;
	u32*		src			= (u32*) mem->ptr;

	for (; count > 0; )
	{
		count	-= 4;
		os_reg_mem_tx_u32(NULL, dst, src[0]);
		src++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a write takes about 1us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_WRITE_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more space is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_write_pio_work
*
*	Purpose:
*
*		Perform PIO based writes of 32, 16 or 8 bit values.
*
*	Arguments:
*
*		alt		The data structure to access.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The source for the data to write.
*
*		count	The number of bytes to write.
*
*		st_end	The "system tick" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_WRITE)
#if defined(GSC_WRITE_PIO_WORK)
long gsc_write_pio_work(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long	qty;

	if (io->bytes_per_sample == 4)
	{
		qty	= gsc_write_pio_work_32_bit(alt, io, mem, count, st_end);
	}
	else if (io->bytes_per_sample == 2)
	{
		qty	= gsc_write_pio_work_16_bit(alt, io, mem, count, st_end);
	}
	else if (io->bytes_per_sample == 1)
	{
		qty	= gsc_write_pio_work_8_bit(alt, io, mem, count, st_end);
	}
	else
	{
		qty	= -EINVAL;
		printf(	"%s: %d. %s: invalid 'bytes_per_sample': %d\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__,
				(int) io->bytes_per_sample);
	}

	return(qty);
}
#endif
#endif


