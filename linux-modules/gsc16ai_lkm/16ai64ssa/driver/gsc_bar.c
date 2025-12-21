// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_bar.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



/******************************************************************************
*
*	Function:	_bar_create
*
*	Purpose:
*
*		Initialize the given structure according the the BAR index given.
*
*	Arguments:
*
*		dev		The structure for this device.
*
*		index	The BAR index to access.
*
*		bar		The structure to initialize.
*
*		map		The mapping data for the given BAR region.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _bar_create(dev_data_t* dev, int index, os_bar_t* bar, const gsc_bar_map_t* map)
{
	int	ret;

	ret		= os_bar_create(dev, index, map->io, map->mem, bar);
	bar->rw	= map->rw;

	if (ret)
	{
		printf(	"%s: %d. %s: BAR%d access error.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__,
				index);
	}

#if DEV_BAR_SHOW
	printf(	"BAR%d"
			": Reg 0x%08lX"
			", Map %s"
			", Adrs 0x%08lX"
			", Size %4ld"
			", Access %s"
#if (BITS_PER_LONG == 32)
			", vaddr 0x%lX"
#else
			", vaddr 0x%llX"
#endif
			"\n",
			(int) index,
			(long) bar->reg,
			bar->size ? (bar->io_mapped ? "I/O" : "mem") : "N/A",
			(long) bar->phys_adrs,
			(long) bar->size,
			bar->rw ? "RW" : "RO",
#if (BITS_PER_LONG == 32)
			(long) bar->vaddr
#else
			(long long) bar->vaddr
#endif
			);
#endif

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_bar_create
*
*	Purpose:
*
*		Initialize the given BAR structure.
*
*	Arguments:
*
*		dev		The structure for this device.
*
*		index	The BAR index to access.
*
*		bar		The structure to initialize.
*
*		mem		Must this BAR be memory mapped?
*
*		io		Must this BAR be I/O mapped?
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_bar_create(dev_data_t* dev, gsc_bar_t* bar, const gsc_bar_maps_t* map)
{
	int	i;
	int	ret		= 0;
	int	tst;

	if ((dev == NULL) || (bar == NULL) || (map == NULL))
	{
		ret	= -EFAULT;
		printf(	"%s: %d. %s: 'dev', 'bar' or 'map' is NULL.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			tst	= _bar_create(dev, i, &bar->bar[i], &map->bar[i]);
			ret	= ret ? ret : tst;
		}
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_bar_destroy
*
*	Purpose:
*
*		Release the given BAR region and its resources.
*
*	Arguments:
*
*		bar		The structure for the BAR to release.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _bar_destroy(os_bar_t* bar)
{
	if (bar)
	{
		os_bar_destroy(bar);
		memset(bar, 0, sizeof(bar[0]));
	}
}



/******************************************************************************
*
*	Function:	gsc_bar_destroy
*
*	Purpose:
*
*		Release the given BAR region and its resources.
*
*	Arguments:
*
*		bar		The structure for the BAR to release.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_bar_destroy(gsc_bar_t* bar)
{
	int	i;

	if (bar)
	{
		for (i = 0; i < 6; i++)
			_bar_destroy(&bar->bar[i]);
	}
}


