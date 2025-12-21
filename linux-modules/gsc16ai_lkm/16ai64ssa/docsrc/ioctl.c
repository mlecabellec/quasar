#include <stdio.h>

#include "16ai64ssa_dsl.h"

int ai64ssa_ioctl_dsl(int fd, int request, void* arg)
{
	int	errs;
	int	ret;

	ret	= ai64ssa_ioctl(fd, request, arg);

	if (ret)
		printf("ERROR: ai64ssa_ioctl() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}