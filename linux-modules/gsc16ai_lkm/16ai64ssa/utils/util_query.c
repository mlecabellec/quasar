// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/utils/util_query.c $
// $Rev: 54951 $
// $Date: 2024-08-07 15:22:35 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Utilities: source file

#include "main.h"



/******************************************************************************
*
*	Function:	ai64ssa_query
*
*	Purpose:
*
*		Provide a visual wrapper for the AI64SSA_IOCTL_QUERY service.
*
*	Arguments:
*
*		fd		Use this handle to access the device.
*
*		index	The index of the device to access. Ignore if < 0.
*				This is for display purposes only.
*
*		verbose	Work verbosely? 0 = no, !0 = yes
*
*		set		This is the query option to access.
*
*		get		The results are reported here. This may be NULL.
*
*	Returned:
*
*		>= 0	The number of errors encounterred.
*
******************************************************************************/

int	ai64ssa_query(int fd, int index, int verbose, s32 set, s32* get)
{
	char		buf[128];
	int			errs	= 0;
	const char*	ptr;
	s32			query	= set;
	int			ret;
	s32			tmp;

	switch (query)
	{
		default:

			errs++;
			ptr	= buf;
			sprintf(buf, "Query Error (#%ld)", (long) query);
			break;

		case AI64SSA_QUERY_AUTOCAL_MS:		ptr	= "Autocal Period";		break;
		case AI64SSA_QUERY_CHAN_RANGE:		ptr = "Channel Range";		break;
		case AI64SSA_QUERY_CHANNEL_MAX:		ptr	= "Max Channels";		break;
		case AI64SSA_QUERY_CHANNEL_QTY:		ptr	= "Input Channels";		break;
		case AI64SSA_QUERY_COUNT:			ptr	= "Query Options";		break;
		case AI64SSA_QUERY_DATA_PACKING:	ptr	= "Data Packing";		break;
		case AI64SSA_QUERY_DEVICE_TYPE:		ptr	= "Device Type";		break;
		case AI64SSA_QUERY_FIFO_SIZE:		ptr	= "FIFO Size";			break;
		case AI64SSA_QUERY_FSAMP_MAX:		ptr	= "Fsamp Max";			break;
		case AI64SSA_QUERY_FSAMP_MIN:		ptr	= "Fsamp Min";			break;
		case AI64SSA_QUERY_INIT_MS:			ptr	= "Initialize Period";	break;
		case AI64SSA_QUERY_IRQ1_BUF_ERROR:	ptr = "IRQ1 Buffer Error";	break;
		case AI64SSA_QUERY_LOW_LATENCY:		ptr = "Low Latency";		break;
		case AI64SSA_QUERY_MASTER_CLOCK:	ptr	= "Master Clock";		break;
		case AI64SSA_QUERY_NRATE_MAX:		ptr = "Nrate Max";			break;
		case AI64SSA_QUERY_NRATE_MIN:		ptr = "Nrate Min";			break;
		case AI64SSA_QUERY_RATE_GEN_QTY:	ptr = "Rate Generators";	break;
		case AI64SSA_QUERY_REG_ACAR:		ptr = "Reg: ACAR";			break;
		case AI64SSA_QUERY_REG_LLCR:		ptr = "Reg: LLCR";			break;
	}

	if (verbose)
		gsc_label_index(ptr, index);

	ret		= ai64ssa_ioctl(fd, AI64SSA_IOCTL_QUERY, &set);
	errs	+= ret ? 1 : 0;

	switch (query)
	{
		default:

			errs++;
			gsc_label_long_comma_buf(set, buf);
			break;

		case AI64SSA_QUERY_CHANNEL_MAX:
		case AI64SSA_QUERY_CHANNEL_QTY:
		case AI64SSA_QUERY_COUNT:
		case AI64SSA_QUERY_NRATE_MAX:
		case AI64SSA_QUERY_NRATE_MIN:
		case AI64SSA_QUERY_RATE_GEN_QTY:

			gsc_label_long_comma_buf(set, buf);
			break;

		case AI64SSA_QUERY_MASTER_CLOCK:

			gsc_label_long_comma_buf(set, buf);
			strcat(buf, " Hz");
			break;

		case AI64SSA_QUERY_AUTOCAL_MS:
		case AI64SSA_QUERY_INIT_MS:

			gsc_label_long_comma_buf(set, buf);
			strcat(buf, " ms");
			break;

		case AI64SSA_QUERY_FSAMP_MAX:
		case AI64SSA_QUERY_FSAMP_MIN:

			gsc_label_long_comma_buf(set, buf);
			strcat(buf, " S/S");
			break;

		case AI64SSA_QUERY_DATA_PACKING:
		case AI64SSA_QUERY_LOW_LATENCY:
		case AI64SSA_QUERY_CHAN_RANGE:
		case AI64SSA_QUERY_IRQ1_BUF_ERROR:
		case AI64SSA_QUERY_REG_LLCR:
		case AI64SSA_QUERY_REG_ACAR:

			sprintf(buf, "%s", set ? "Supported" : "Not Supported");
			break;

		case AI64SSA_QUERY_FIFO_SIZE:

			ptr	= "";
			tmp	= set;

			if ((tmp) && ((tmp % 1024) == 0))
			{
				ptr	= "K";
				tmp	/= 1024;
			}

			gsc_label_long_comma_buf(tmp, buf);
			strcat(buf, ptr);
			strcat(buf, " Words");
			break;

		case AI64SSA_QUERY_DEVICE_TYPE:

			if (set == GSC_DEV_TYPE_16AI64SSA)
			{
				strcpy(buf, "16AI64SSA");
			}
			else if (set == GSC_DEV_TYPE_16AI64SSC)
			{
				strcpy(buf, "16AI64SSC");
			}
			else
			{
				errs++;
				sprintf(buf, "INVALID: %ld", (long) set);
			}

			break;
	}

	if (verbose)
	{
		if (errs)
			printf("FAIL <---  (%s)\n", buf);
		else
			printf("%s\n", buf);
	}

	if (get)
		get[0]	= set;

	return(errs);
}


