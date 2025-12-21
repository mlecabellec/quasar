// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/tftest/d15.c $
// $Rev: 51402 $
// $Date: 2022-07-14 13:18:38 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
int	d15_test(int fd)
{
	int	errs;

	errs	= dxx_test(fd, 15);
	return(errs);
}


