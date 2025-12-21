#include <stdio.h>

#include "opto16x16_dsl.h"

int opto16x16_open_dsl(int device, int share, int* fd)
{
	int	errs;
	int	ret;

	ret	= opto16x16_open(device, share, fd);

	if (ret)
		printf("ERROR: opto16x16_open() returned %d\n", ret);

	errs	= ret ? 1 : 0;
	return(errs);
}