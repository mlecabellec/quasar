#include <stdio.h>

#include "opto16x16_dsl.h"

int opto16x16_read_dsl(int fd, void* dst, size_t bytes, size_t* qty)
{
	int	errs;
	int	ret;

	ret	= opto16x16_read(fd, dst, bytes);

	if (ret < 0)
		printf("ERROR: opto16x16_read() returned %d\n", ret);

	if (qty)
		qty[0]	= (ret < 0) ? 0 : (size_t) ret;

	errs	= (ret < 0) ? 1 : 0;
	return(errs);
}