// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/sbtest/query.c $
// $Rev: 51414 $
// $Date: 2022-07-14 14:34:50 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _service_test(int fd)
{
	return(0);
}



//*****************************************************************************
static int _function_test(int fd)
{
	s32	data;
	s32	count;
	int	errs;
	int	i;

	errs	= opto16x16_query(fd, -1, 0, OPTO16X16_QUERY_COUNT, &count);

	if (count != OPTO16X16_IOCTL_QUERY_LAST)
	{
		errs	= 1;
		printf(	"FAIL <---  (expected %ld, got %ld, rebuild application)\n",
				(long) OPTO16X16_IOCTL_QUERY_LAST,
				(long) count);
	}

	for (i = 0; (errs == 0) && (i < OPTO16X16_IOCTL_QUERY_LAST); i++)
		errs	= opto16x16_query(fd, -1, 0, i, &data);

	return(errs);
}



/******************************************************************************
*
*	Function:	query_test
*
*	Purpose:
*
*		Perform a test of the IOCTL service OPTO16X16_IOCTL_QUERY.
*
*	Arguments:
*
*		fd		The handle for the device to access.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int query_test(int fd)
{
	int	errs	= 0;

	gsc_label("OPTO16X16_IOCTL_QUERY");
	errs	+= _service_test(fd);
	errs	+= _function_test(fd);

	if (errs == 0)
		printf("PASS\n");

	return(errs);
}


