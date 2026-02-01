#include <stdio.h>

#include "opto16x16_dsl.h"

int opto16x16_close_dsl(int fd)
{
	int	errs;
	int	ret;

	ret	= opto16x16_close(fd);

	if (ret)
		printf("ERROR: opto16x16_close() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}