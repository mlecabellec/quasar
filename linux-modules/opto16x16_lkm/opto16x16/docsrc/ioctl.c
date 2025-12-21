#include <stdio.h>

#include "opto16x16_dsl.h"

int opto16x16_ioctl_dsl(int fd, int request, void* arg)
{
	int	errs;
	int	ret;

	ret	= opto16x16_ioctl(fd, request, arg);

	if (ret)
		printf("ERROR: opto16x16_ioctl() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}