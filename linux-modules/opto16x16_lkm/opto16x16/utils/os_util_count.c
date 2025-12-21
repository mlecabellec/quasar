// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_count.c $
// $Rev: 51523 $
// $Date: 2022-10-03 16:34:18 -0500 (Mon, 03 Oct 2022) $

// Linux: Utility: source file

#include "main.h"



//*****************************************************************************
int os_count_boards(
	int		verbose,
	int*	qty,
	int		(*dev_open)(int index, int share, int* fd),
	int		(*dev_read)(int fd, void* dst, size_t size),
	int		(*dev_close)(int fd))
{
	char	buf[1024];
	int		errs	= 0;
	int		fd;
	int		got;
	char*	ptr;
	int		ret;

	for (;;)	// A convenience loop.
	{
		if (verbose)
			gsc_label("Board Count");

		if ((dev_open == NULL) ||
			(dev_read == NULL) ||
			(dev_close == NULL))
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (Invalid Argument(s))\n");

			break;
		}

		ret	= (dev_open)(-1, 0, &fd);

		if (ret)
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (open error)\n");

			break;
		}

		memset(buf, 0, sizeof(buf));
		got	= dev_read(fd, buf, sizeof(buf));
		dev_close(fd);

		if (got <= 0)
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (read error)\n");

			break;
		}

		buf[sizeof(buf) - 1]	= 0;

		// Boards
		ptr	= strstr(buf, "boards");

		if (ptr == NULL)
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (content error)\n");

			break;
		}

		ptr	+= 8;
		got	= atoi(ptr);

		if (got < 0)
		{
			errs++;

			if (verbose)
				printf("FAIL <---  (content error)\n");

			break;
		}

		if (qty)
			qty[0]	= got;

		if (verbose)
			printf("PASS  (%d Board%s)\n", got, (got == 1) ? "" : "s");

		break;
	}

	return(errs);
}



