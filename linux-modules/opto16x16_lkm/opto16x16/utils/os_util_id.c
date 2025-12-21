// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/linux/os_util_id.c $
// $Rev: 53573 $
// $Date: 2023-08-23 15:19:07 -0500 (Wed, 23 Aug 2023) $

// Linux: Utility: source file

#include "main.h"



// macros *********************************************************************

#define	ISEOL(_c)	((((_c) == 0)		|| \
					  ((_c) == '\r')	|| \
					  ((_c) == '\n')) ? 1 : 0)



/******************************************************************************
*
*	Function:	_get_total_mb
*
*	Purpose:
*
*		Report the amount of total RAM in megabytes.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of megabytes installed..
*
******************************************************************************/

static long _get_total_mb(void)
{
	char	buf[1024];
	int		end;
	char	factor	= 0;
	FILE*	file;
	int		i;
	long	total	= 0;

	file	= fopen("/proc/meminfo", "r");

	if (file)
	{
		for (;;)
		{
			end	= feof(file);

			if (end)
				break;

			fgets(buf, sizeof(buf), file);
			i	= sscanf(buf, "MemTotal: %ld %c", &total, &factor);

			if (i == 2)
				break;
		}

		fclose(file);
	}

	if (factor == 0)
	{
		total	= (total + 1023) / 1024;
		factor	= 'k';
	}

	if ((factor == 'k') || (factor == 'K'))
	{
		total	= (total + 1023) / 1024;
		factor	= 'm';
	}

	if ((factor == 't') || (factor == 'T'))
	{
		total	*= 1024;
		factor	= 'm';
	}

	if ((factor == 'g') || (factor == 'G'))
	{
		total	*= 1024;
		//factor	= 'm';
	}

	return(total);
}



/******************************************************************************
*
*	Function:	_skipws
*
*	Purpose:
*
*		Skip the leading white space in the given string.
*
*	Arguments:
*
*		psz		The string to process.
*
*	Returned:
*
*		The resulting string, if non-NULL.
*
******************************************************************************/

static char* _skipws(char* psz)
{
	for (; (psz) && (isspace(psz[0]));)
		psz++;

	return(psz);
}



/******************************************************************************
*
*	Function:	_id_host_memory_total
*
*	Purpose:
*
*		Identify total host memory.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _id_host_memory_total(void)
{
	long	total;

	gsc_label("Total Memory");
	total	= _get_total_mb();

	if (total == 0)
		printf("Unknown\n");
	else
		printf("%ld MB\n", total);
}



/******************************************************************************
*
*	Function:	_id_host_memory_physical
*
*	Purpose:
*
*		Identify total physical memory. We could get the exact amount and
*		nature of physical memory, but that is an indepth process. See the
*		source code for the dmidecode utility for details. Here I'll just
*		guesstimate physical memory by rounding total memory up a bit.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _id_host_memory_physical(void)
{
	long	mb;
	long	total;

	gsc_label("Physical Memory");
	mb	= _get_total_mb();

	if (mb == 0)
	{
		printf("Unknown\n");
	}
	else if (mb <= 512)
	{
		total	= (((mb + 63) / 64) * 64);
		printf("%ld MB\n", total);
	}
	else if (mb <= 1024)
	{
		total	= (((mb + 127) / 128) * 128);
		printf("%ld MB\n", total);
	}
	else
	{
		total	= (((mb + 255) / 256) * 256);
		printf("%ld MB\n", total);
	}
}



/******************************************************************************
*
*	Function:	_id_host_name
*
*	Purpose:
*
*		Identify the host machine.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _id_host_name(void)
{
	char	buf[1024];

	memset(buf, 0, sizeof(buf));
	gethostname(buf, sizeof(buf));
	// We ignore the return value for now and assume success.
	buf[sizeof(buf) - 1]	= 0;

	gsc_label("Host Name");
	printf("%s\n", buf);
}



/******************************************************************************
*
*	Function:	_id_host_os
*
*	Purpose:
*
*		Identify the host os.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _id_host_os(void)
{
	struct utsname	uts;

	memset(&uts, 0, sizeof(uts));
	uname(&uts);

	gsc_label("Operating System");
	printf("%s\n", uts.sysname);
	gsc_label("Kernel");
	printf("%s\n", uts.release);
}



/******************************************************************************
*
*	Function:	_id_host_processor
*
*	Purpose:
*
*		Identify the host processor.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _id_host_processor(void)
{
	char			buf[1024];
	int				end;
	FILE*			file;
	int				i;
	int				index;
	char			model[1024]	= "";
	char*			psz;
	int				qty			= 0;
	char			tmp[1024];
	struct utsname	uts;

	memset(&uts, 0, sizeof(uts));
	uname(&uts);

	gsc_label("Architecture");
	printf("%s\n", uts.machine);

	file	= fopen("/proc/cpuinfo", "r");

	if (file)
	{
		for (;;)
		{
			end	= feof(file);

			if (end)
				break;

			fgets(buf, sizeof(buf), file);
			i	= sscanf(buf, "processor : %d", &index);

			if (i == 1)
			{
				qty++;
				continue;
			}

			i	= sscanf(buf, "model name : %s", tmp);

			if (i == 1)
			{
				psz	= strchr(buf, ':');

				if (psz)
				{
					psz	+= 2;
					psz	= _skipws(psz);
				}

				if (psz)
					strcpy(model, psz);

				continue;
			}
		}

		fclose(file);

		gsc_label("CPU Model");
		printf("%s", model);

		gsc_label("CPU Count");
		printf("%d\n", qty);
	}
}



//*****************************************************************************
int os_id_driver(
	int	(*dev_open)(int index, int share, int* fd),
	int	(*dev_read)(int fd, void* dst, size_t size),
	int	(*dev_close)(int fd))
{
	char	buf[1024];
	int		errs	= 0;
	int		fd;
	int		got;
	char*	ptr;
	int		ret;

	for (;;)	// A convenience loop.
	{
		gsc_label("Driver");

		if ((dev_open == NULL) ||
			(dev_read == NULL) ||
			(dev_close == NULL))
		{
			errs++;
			printf("FAIL <---  (Invalid Argument(s))\n");
			break;
		}

		ret	= (dev_open)(-1, 0, &fd);

		if (ret)
		{
			errs++;
			printf("FAIL <---  (open error)\n");
			break;
		}

		memset(buf, 0, sizeof(buf));
		got	= dev_read(fd, buf, sizeof(buf));
		dev_close(fd);

		if (got <= 0)
		{
			errs++;
			printf("FAIL <---  (read error)\n");
			break;
		}

		buf[sizeof(buf) - 1]	= 0;

		// Version
		printf("Version ");
		ptr	= strstr(buf, "version");

		if (ptr == NULL)
		{
			errs++;
			printf("FAIL <---  (File Content Error)\n");
			break;
		}

		for (ptr += 9; !ISEOL(ptr[0]); ptr++)
			printf("%c", ptr[0]);

		printf("\n");
		break;
	}

	return(errs);
}



/******************************************************************************
*
*	Function:	os_id_host
*
*	Purpose:
*
*		Identify the host os and machine.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void os_id_host(void)
{
	gsc_label("Host");
	printf("\n");
	gsc_label_level_inc();

	_id_host_name();
	_id_host_os();
	_id_host_processor();
	_id_host_memory_total();
	_id_host_memory_physical();

	gsc_label_level_dec();
}


