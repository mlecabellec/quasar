// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/utils/util_query.c $
// $Rev: 53726 $
// $Date: 2023-09-14 10:45:45 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	opto16x16_query
*
*	Purpose:
*
*		Provide a visual wrapper for the OPTO16X16_IOCTL_QUERY service.
*
*	Arguments:
*
*		fd		Use this handle to access the device.
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely?
*
*		set		This is the query option to access.
*
*		get		The results are reported here. This may be NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int opto16x16_query(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs	= 0;
	const char*	ptr;
	s32			query	= set;
	int			ret;

	switch (query)
	{
		default:

			errs++;
			ptr	= buf;
			sprintf(buf, "Query Error (#%ld)", (long) query);
			break;

		case OPTO16X16_QUERY_COUNT:			ptr	= "Query Options";	break;
		case OPTO16X16_QUERY_DEVICE_TYPE:	ptr	= "Device Type";	break;
	}

	if (verbose)
		gsc_label_index(ptr, index);

	ret		= opto16x16_ioctl(fd, OPTO16X16_IOCTL_QUERY, &set);
	errs	+= ret ? 1 : 0;

	switch (query)
	{
		default:

			errs++;
			sprintf(buf, "%ld", (long) set);
			break;

		case OPTO16X16_QUERY_COUNT:

			sprintf(buf, "%ld", (long) set);
			break;

		case OPTO16X16_QUERY_DEVICE_TYPE:

			if (set == GSC_DEV_TYPE_OPTO16X16)
			{
				strcpy(buf, "OPTO16X16");
			}
			else
			{
				errs++;
				sprintf(buf, "INVALID: %ld", (long) set);
			}

			break;
	}

	if (verbose)
	{
		if (errs)
			printf("FAIL <---  (%s)\n", buf);
		else
			printf("%s\n", buf);
	}

	if (get)
		get[0]	= set;

	return(errs);
}


