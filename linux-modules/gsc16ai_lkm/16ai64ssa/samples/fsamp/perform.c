// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/fsamp/perform.c $
// $Rev: 54956 $
// $Date: 2024-08-07 15:51:58 -0500 (Wed, 07 Aug 2024) $

// 16AI64SSA: Sample Application: source file

#include "main.h"



// macros *********************************************************************

#define	FILE_NAME				"fsamp.txt"
#define	LIMIT_MAX(v,max)		(((v) > (max)) ? (max) : (v))
#define	LIMIT_MIN(v,min)		(((v) < (min)) ? (min) : (v))
#define	LIMIT_RANGE(v,min,max)	(LIMIT_MIN((LIMIT_MAX((v),(max))),(min)))



//*****************************************************************************
static int _hold_secs(const args_t* args, s32 fsamp)
{
	char	buf[80];
	int		errs	= 0;

	gsc_label("Config");
	errs	+= ai64ssa_config_ai(args->fd, -1, 0, fsamp);
	printf("%s\n", errs ? "FAIL <---" : "PASS");

	if (args->hold_reg_dump)
		ai64ssa_reg_list(args->fd, 1);

	strcpy(buf, "Holding ");
	gsc_label_long_comma_buf(args->hold_secs, buf + strlen(buf));
	strcat(buf, " secs");
	gsc_label(buf);

	if (errs == 0)
	{
		os_sleep_ms(args->hold_secs * 1000);
		printf("Done\n");
	}
	else
	{
		printf("Aborting due to error.\n");
	}

	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int			errs	= 0;
	FILE*		file	= NULL;
	s32			fsamp;
	s32			max		= args->fsamp;
	s32			min		= args->fsamp;
	const char*	name	= FILE_NAME;
	double		rate;

	gsc_label("Fsamp Computation");
	printf("\n");
	gsc_label_level_inc();

	errs	+= ai64ssa_query(args->fd, -1, 0, AI64SSA_QUERY_FSAMP_MAX, &max);
	errs	+= ai64ssa_query(args->fd, -1, 0, AI64SSA_QUERY_FSAMP_MIN, &min);

	if (args->range == 0)
	{
		min	= LIMIT_RANGE(args->fsamp, min, max);
		max	= min;
	}
	else
	{
		min	= LIMIT_RANGE(args->range_begin, min, max);
		max	= LIMIT_RANGE(args->range_end, min, max);
	}

	if (args->save)
	{
		file	= fopen(name, "w+b");

		if (file == NULL)
		{
			printf("FAIL <---  (unable to create %s)\n", name);
			errs	= 1;
		}
		else
		{
			fprintf(file, "# 16AI64SSA Scan Rates\n");
			fprintf(file, "# Request  Produced\n");
			fprintf(file, "# =======  ==========\n");
		}
	}

	for (fsamp = min; fsamp <= max; fsamp++)
	{
		errs	+= ai64ssa_fsamp_ai_compute(args->fd, -1, 1, fsamp, NULL, NULL, NULL, NULL, NULL, NULL, &rate);

		if (args->hold_secs)
			errs	+= _hold_secs(args, fsamp);

		if (file)
			fprintf(file, "%9ld  %10.3f\n", (long) fsamp, (float) rate);
	}

	if (file)
		fclose(file);

	gsc_label_level_dec();
	return(errs);
}


