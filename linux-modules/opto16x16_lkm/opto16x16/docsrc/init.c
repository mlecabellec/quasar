#include <stdio.h>

#include "opto16x16_dsl.h"

int opto16x16_init_dsl(void)
{
	int	errs;
	int	ret;

	ret	= opto16x16_init();

	if (ret)
		printf("ERROR: opto16x16_init() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}