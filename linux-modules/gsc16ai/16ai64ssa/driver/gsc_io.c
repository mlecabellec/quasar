// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_io.c $
// $Rev: 45074 $
// $Date: 2019-06-12 14:17:47 -0500 (Wed, 12 Jun 2019) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



/******************************************************************************
*
*	Function:	gsc_io_create
*
*	Purpose:
*
*		Perform I/O based initialization as the driver is being loaded.
*
*	Arguments:
*
*		alt		The structure for the device to access.
*
*		io		The I/O structure of interest.
*
*		size	The desired size of the buffer in bytes.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_io_create(GSC_ALT_STRUCT_T* alt, dev_io_t* io, size_t bytes)
{
	int	ret;

	os_sem_create(&io->sem);
	os_mem_dma_alloc(alt, &bytes, &io->mem);
	ret	= io->mem.ptr ? 0 : -ENOMEM;
	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_io_destroy
*
*	Purpose:
*
*		Perform I/O based cleanup as the driver is being unloaded.
*
*	Arguments:
*
*		alt		The structure for the device to access.
*
*		io		The I/O structure of interest.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_io_destroy(GSC_ALT_STRUCT_T* alt, dev_io_t* io)
{
	if (io->mem.ptr)
		os_mem_dma_free(&io->mem);

	os_sem_destroy(&io->sem);
	memset(io, 0, sizeof(dev_io_t));
}


