#include <stdio.h>

#include "16ai64ssa_dsl.h"

int ai64ssa_read_dsl(int fd, void* dst, size_t bytes, size_t* qty)
{
	int	errs;
	int	ret;

	ret	= ai64ssa_read(fd, dst, bytes);

	if (ret < 0)
		printf("ERROR: ai64ssa_read() returned %d\n", ret);

	if (qty)
		qty[0]	= (ret < 0) ? 0 : (size_t) ret;

	errs	= (ret < 0) ? 1 : 0;
	return(errs);
}