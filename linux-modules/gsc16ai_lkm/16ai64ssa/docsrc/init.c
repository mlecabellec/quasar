#include <stdio.h>

#include "16ai64ssa_dsl.h"

int ai64ssa_init_dsl(void)
{
	int	errs;
	int	ret;

	ret	= ai64ssa_init();

	if (ret)
		printf("ERROR: ai64ssa_init() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}