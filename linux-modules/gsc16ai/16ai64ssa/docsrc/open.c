#include <stdio.h>

#include "16ai64ssa_dsl.h"

int ai64ssa_open_dsl(int device, int share, int* fd)
{
	int	errs;
	int	ret;

	ret	= ai64ssa_open(device, share, fd);

	if (ret)
		printf("ERROR: ai64ssa_open() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}