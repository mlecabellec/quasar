#include <stdio.h>

#include "16ai64ssa_dsl.h"

int ai64ssa_close_dsl(int fd)
{
	int	errs;
	int	ret;

	ret	= ai64ssa_close(fd);

	if (ret)
		printf("ERROR: ai64ssa_close() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}