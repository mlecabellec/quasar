// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_write.c $
// $Rev: 51748 $
// $Date: 2022-10-12 10:14:48 -0500 (Wed, 12 Oct 2022) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
ssize_t os_write(
	struct file*	filp,
	const char*		buf,
	size_t			count,
	loff_t*			offp)
{
#if defined(DEV_SUPPORTS_WRITE)

	GSC_ALT_STRUCT_T*	alt;
	dev_io_t*			io;
	ssize_t				ret;

	// Access the device structure.
	alt	= (GSC_ALT_STRUCT_T*) filp->private_data;

	if (alt)
	{
		// The I/O stream selector is encoded in the 0xF0000000 nibble.
		io		= dev_io_write_select(alt, count);
		count	&= GSC_IO_SIZE_QTY_MASK;

		// Is this blocking or non-blocking I/O.
		io->non_blocking	= (filp->f_flags & O_NONBLOCK) ? 1 : 0;

		// Call the common code.
		ret	= gsc_write(alt, io, buf, count);
	}
	else
	{
		// The referenced device doesn't exist.
		ret	= -ENODEV;
	}

	return(ret);
#else
	return (-EPERM);
#endif
}


