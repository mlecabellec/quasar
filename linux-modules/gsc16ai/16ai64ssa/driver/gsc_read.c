// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_read.c $
// $Rev: 53985 $
// $Date: 2023-12-12 08:45:21 -0600 (Tue, 12 Dec 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// macros *********************************************************************

#if defined(DEV_SUPPORTS_READ)

#ifndef	GSC_READ_PIO_PER_1MS
#define	GSC_READ_PIO_PER_1MS				250
#endif

#if (GSC_READ_PIO_PER_1MS > 1000)
#undef	GSC_READ_PIO_PER_1MS
#define	GSC_READ_PIO_PER_1MS				1000
#endif

#endif



/******************************************************************************
*
*	Function:	_read_work
*
*	Purpose:
*
*		Implement the mode independent working portion of the read()
*		procedure. If a timeout is permitted and called for, we wait a single
*		timer tick and check again.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		usr_buf	The data read is placed here.
*
*		count	The number of bytes requested. This is positive.
*
*		st_end	Timeout at this point in time (in system ticks).
*				Zero means never timeout.
*
*		fn_aval	This is the mode specific function that determines the number
*				of bytes that may be read.
*
*		fn_xfer	This is the function implementing the mode specific read
*				functionality.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
static long _read_work(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	char*				usr_buf,
	size_t				count,	// bytes
	os_time_tick_t		st_end,	// The "system tick" time that we timeout.
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
	os_mem_t	mem		= io->mem;
	int			ret;
	long		total	= 0;	// The number of bytes transferred.
	long		xfer_did;		// Number of bytes we got this time.
	long		xfer_do;		// Number of bytes to get this time.

	for (;;)
	{
		// See how much data we can transfer.
		xfer_do	= (fn_aval)(alt, io, count);

		if (xfer_do > (long) mem.bytes)
			xfer_do	= (long) mem.bytes;	// The size of the transfer buffer.

		if (xfer_do > (long) count)
			xfer_do	= (long) count;		// Try to complete the request.

		GSC_RX_DEBUG_ADD(xfer_do, alt->debug.rx_avail_calls, 1);
		GSC_RX_DEBUG_ADD(xfer_do, alt->debug.rx_avail_bytes, xfer_do);

		// Either transfer the data or see what to do next.

		if (io->abort)
		{
			// We've been told to quit.
			io->abort	= 0;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.abort);
			break;
		}

		if (xfer_do > 0)
		{
			xfer_did	= (fn_xfer)(alt, io, &mem, xfer_do, st_end);
			GSC_RX_DEBUG_ADD(xfer_did, alt->debug.rx_xfer_calls, 1);
			GSC_RX_DEBUG_ADD(xfer_did, alt->debug.rx_xfer_bytes, xfer_did);
		}
		else
		{
			xfer_did	= 0;
		}

		if (xfer_did < 0)
		{
			// There was a problem.
			total	= xfer_did;
			gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
			break;
		}
		else if (xfer_did > 0)
		{
			// Copy the data to the application buffer.
			ret	= os_mem_copy_to_user_rx(usr_buf, mem.ptr, xfer_did);

			if (ret)
			{
				// We couldn't copy out the data.
				total	= -EFAULT;
				gsc_wait_resume_io(alt, io->wait.done | io->wait.error);
				printf(	"%s: %d. %s: copy to app memory failed.\n",
						DEV_NAME,
						__LINE__,
						__FUNCTION__);
				break;
			}

			// House keeping.
			count		-= xfer_did;
			total		+= xfer_did;
			usr_buf		+= xfer_did;
#if defined(GSC_RX_IO_BUF_PRESERVE)
			// This prevents us from overwriting the driver's Rx buffer. This
			// is done because the application level code (i.e. a library)
			// accesses the driver's buffer directly.
			mem.ptr		= (char*) mem.ptr + xfer_did;
			mem.bytes	-= xfer_did;
			mem.adrs	+= xfer_did;
#endif
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
		GSC_RX_DEBUG_ADD(1, alt->debug.rx_work_sleeps, 1);
		ret	= os_time_tick_sleep(1);

		if (ret)
		{
			// The timeout returned prematurely; it was signaled.
			// We've essentially been told to quit.
			gsc_wait_resume_io(alt, io->wait.done | io->wait.abort);
			break;
		}
	}

	GSC_RX_DEBUG_ADD(total, alt->debug.rx_work_calls, 1);
	GSC_RX_DEBUG_ADD(total, alt->debug.rx_work_bytes, total);
	return(total);
}
#endif



/******************************************************************************
*
*	Function:	gsc_read
*
*	Purpose:
*
*		Implement the read() procedure. Only one call active at a time.
*
*	Arguments:
*
*		alt		The structure for the device to read from.
*
*		io		The I/O structure for the stream to access.
*
*		buf		The data requested goes here.
*
*		count	The number of bytes requested.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
long gsc_read(GSC_ALT_STRUCT_T* alt, dev_io_t* io, void* buf, size_t count)
{
	long			ret;
	size_t			samples;
	os_time_tick_t	st_end;	// The "system tick" time that we timeout. Zero = never timeout.

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

		GSC_RX_DEBUG_ADD(1, alt->debug.rx_read_calls, 1);
		GSC_RX_DEBUG_ADD(1, alt->debug.rx_read_bytes, count);

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
			ret	= _read_work(	alt,
								io,
								buf,
								count,	// bytes
								st_end,
								io->dev_pio_available,
								io->dev_pio_xfer);
		}
		else if (io->io_mode == GSC_IO_MODE_BMDMA)
		{
			ret	= _read_work(	alt,
								io,
								buf,
								count,	// bytes
								st_end,
								io->dev_bmdma_available,
								io->dev_bmdma_xfer);
		}
		else if (io->io_mode == GSC_IO_MODE_DMDMA)
		{
			ret	= _read_work(	alt,
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
*	Function:	gsc_read_abort_active_xfer
*
*	Purpose:
*
*		Abort an active read.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*	Returned:
*
*		1		An active read was aborted.
*		0		A read was not in progress.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
int gsc_read_abort_active_xfer(GSC_ALT_STRUCT_T* alt, dev_io_t* io)
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
*	Function:	gsc_read_pio_work_8_bit
*
*	Purpose:
*
*		Perform PIO based reads of 8-bit values.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The destination for the data read.
*
*		count	The number of bytes to read.
*
*		st_end	The "system ticks" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
#if defined(GSC_READ_PIO_WORK) || defined(GSC_READ_PIO_WORK_8_BIT)
long gsc_read_pio_work_8_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long	qty			= (long) count;
	VADDR_T	src			= io->io_reg_vaddr;
	u8*		dst			= (u8*) mem->ptr;
	int		interval	= 0;

	for (; count > 0; )
	{
		count	-= 1;
		dst[0]	= os_reg_mem_rx_u8(NULL, src);
		dst++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a read takes about 2.5us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_READ_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more data is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_calls, 1);
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_bytes, qty);
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_read_pio_work_16_bit
*
*	Purpose:
*
*		Perform PIO based reads of 16-bit values.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The destination for the data read.
*
*		count	The number of bytes to read.
*
*		st_end	The "system ticks" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
#if defined(GSC_READ_PIO_WORK) || defined(GSC_READ_PIO_WORK_16_BIT)
long gsc_read_pio_work_16_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long	qty			= (long) count;
	VADDR_T	src			= io->io_reg_vaddr;
	u16*	dst			= (u16*) mem->ptr;
	int		interval	= 0;

	for (; count > 0; )
	{
		count	-= 2;
		dst[0]	= os_reg_mem_rx_u16(NULL, src);
		dst++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a read takes about 2.5us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_READ_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more data is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_calls, 1);
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_bytes, qty);
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_read_pio_work_32_bit
*
*	Purpose:
*
*		Perform PIO based reads of 32-bit values.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The destination for the data read.
*
*		count	The number of bytes to read.
*
*		st_end	The "system ticks" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
#if defined(GSC_READ_PIO_WORK) || defined(GSC_READ_PIO_WORK_32_BIT)
long gsc_read_pio_work_32_bit(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long	qty			= (long) count;
	VADDR_T	src			= io->io_reg_vaddr;
	u32*	dst			= (u32*) mem->ptr;
	int		interval	= 0;

	for (; count > 0; )
	{
		count	-= 4;
		dst[0]	= os_reg_mem_rx_u32(NULL, src);
		dst++;

		// Rather then check for a timeout on every transfer we'll just do it
		// periodically. If a read takes about 2.5us, then the below count
		// means we check about every 1ms.

		if (interval < GSC_READ_PIO_PER_1MS)
		{
			interval++;
			continue;
		}

		interval	= 0;

		if ((st_end) && (io->timeout_s))
		{
			// st_end: 0 = never timeout
			// timeout_s: 0 = stop when no more data is available

			if (os_time_tick_timedout(st_end))
			{
				// We've timed out.
				break;
			}
		}
	}

	qty	-= (long) count;
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_calls, 1);
	GSC_RX_DEBUG_ADD(qty, alt->debug.rx_pio_bytes, qty);
	return(qty);
}
#endif
#endif



/******************************************************************************
*
*	Function:	gsc_read_pio_work
*
*	Purpose:
*
*		Perform PIO based reads of 32, 16 or 8 bit values.
*
*	Arguments:
*
*		alt		The device data structure.
*
*		io		The I/O structure for the stream to access.
*
*		mem		The destination for the data read.
*
*		count	The number of bytes to read.
*
*		st_end	The "system ticks" time at which we timeout.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

#if defined(DEV_SUPPORTS_READ)
#if defined(GSC_READ_PIO_WORK)
long gsc_read_pio_work(
	GSC_ALT_STRUCT_T*	alt,
	dev_io_t*			io,
	const os_mem_t*		mem,
	size_t				count,
	os_time_tick_t		st_end)
{
	long	qty;

	if (io->bytes_per_sample == 4)
	{
		qty	= gsc_read_pio_work_32_bit(alt, io, mem, count, st_end);
	}
	else if (io->bytes_per_sample == 2)
	{
		qty	= gsc_read_pio_work_16_bit(alt, io, mem, count, st_end);
	}
	else if (io->bytes_per_sample == 1)
	{
		qty	= gsc_read_pio_work_8_bit(alt, io, mem, count, st_end);
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


