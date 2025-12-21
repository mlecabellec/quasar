// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/gsc_util_reg.c $
// $Rev: 51063 $
// $Date: 2022-05-31 10:24:03 -0500 (Tue, 31 May 2022) $

// OS & Device Independent: Utility: source file

#include "main.h"



/******************************************************************************
*
*	Function:	gsc_reg_field_show
*
*	Purpose:
*
*		Print out details on a register field.
*
*	Arguments:
*
*		label_w	This gives the display width of the register name data
*				column, which is towards the left of the display.
*
*		name_w	This is the display width of the field name column.
*
*		value	This is the register value.
*
*		hi_bit	This is the index of the field's high bit.
*
*		low_bit	This is the index of the field's low bit.
*
*		eol		Terminate the output with an end-of-line character?
*
*		list	This is a list of name for each field value.
*
*		name	This is the name of the field.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_reg_field_show(
	int					label_w,
	int					name_w,
	u32					value,
	int					hi_bit,
	int					low_bit,
	int					eol,
	const char* const *	list,
	const char*			name)
{
	char	buf[256];
	int		i;
	u32		field;
	char	format[32];

	u32		mask	= GSC_FIELD_ENCODE(0xFFFFFFFF, hi_bit, low_bit);
	int		w;

	// The leading white space under the register name column.
	printf("%*s", label_w, "");

	// The beginning of the field value.
	printf("0x");

	// The leading "-" characters.
	w	= (31 - hi_bit) / 4;

	for (i = 0; i < w; i++)
		printf("-");

	// The field value.
	w		= (hi_bit / 4) - (low_bit / 4) + 1;
	field	= (value & mask) >> ((low_bit / 4) * 4);
	sprintf(format, "%%0%dlX", (int) w);
	printf(format, (long) field);

	// The leading "-" characters.
	w	= low_bit / 4;

	for (i = 0; i < w; i++)
		printf("-");

	// The field bits.

	if (hi_bit == low_bit)
		sprintf(buf, "D%d", (int) hi_bit);
	else
		sprintf(buf, "D%d-D%d", (int) hi_bit, (int) low_bit);

	printf("  %-7s", buf);

	// The field name.
	sprintf(buf, "%s:", name);
	printf("  %-*s", name_w, buf);

	// The field value name.

	if (list)
	{
		i	= (int) ((value & mask) >> low_bit);
		printf("%s", list[i]);
	}
	else if (eol)
	{
		field	= (value & mask) >> low_bit;
		printf("(0x%lX, %lu)", (long) field, (long) field);
	}

	if (eol)
		printf("\n");

	fflush(stdout);
}



/******************************************************************************
*
*	Function:	gsc_reg_list
*
*	Purpose:
*
*		List the given registers and their values.
*
*	Arguments:
*
*		fd		The handle to access the device.
*
*		list	The register list to display.
*
*		detail	Show register details?
*
*		func	Call this service to read a register.
*
*	Returned:
*
*		>= 0	The number of errors encountered here.
*
******************************************************************************/

int gsc_reg_list(
	int				fd,
	gsc_reg_def_t*	list,
	int				detail,
	int				(func)(int fd, int index, int verbose, u32 reg, u32* value))
{
	int				errs		= 0;
	int				i;
	int				len			= 0;
	u32				offset;
	gsc_reg_def_t*	reg_list	= list;
	u32				size;
	int				support;
	char			t_bar[128];
	char			title[128];
	int				tmp;

	// Calculate the width of the name field.

	for (list = reg_list; list->name; list++)
	{
		tmp	= (int) strlen(list->name);

		if (len < tmp)
			len	= tmp;
	}

	// Read the register values up front.

	for (list = reg_list; list->name; list++)
		list->err	= (func)(fd, -1, 0, list->reg, &list->value);

	// Generate the column titles and separators.

	// Register Name.
	strcpy(title, "Register");
	strcpy(t_bar, "========");
	len	= (len < 8) ? 8 : len;

	for (i = 8; i < len; i++)
	{
		strcat(title, " ");
		strcat(t_bar, "=");
	}

	// The offset.
	strcat(title, "  Offset");
	strcat(t_bar, "  ======");

	// The value.
	strcat(title, "  Value     ");
	strcat(t_bar, "  ==========");

	// The field delimiters.

	if (detail)
	{
		strcat(title, "  Field  ");
		strcat(t_bar, "  =======");
	}

	// The description.
	strcat(title, "  Description");
	strcat(t_bar, "  ===========");

	printf("%s\n", title);
	printf("%s\n", t_bar);
	fflush(stdout);

	// Print the data.

	for (list = reg_list; list->name; list++)
	{
		offset	= GSC_REG_OFFSET(list->reg);
		size	= GSC_REG_SIZE(list->reg);

		printf("%-*s", len, list->name);
		printf("  0x%04lX", (long) offset);

		switch (size)
		{
			default:
			case 1:	printf("  0x%*s%02lX", 6, "", (long) list->value);	break;
			case 2:	printf("  0x%*s%04lX", 4, "", (long) list->value);	break;
			case 3:	printf("  0x%*s%06lX", 2, "", (long) list->value);	break;
			case 4:	printf("  0x%*s%08lX", 0, "", (long) list->value);	break;
		}

		errs	+= list->err;

		if (list->err)
		{
			printf("  FAIL <---  ((func)())\n");
		}
		else
		{
			if ((list->decode) && (list->ask_support))
				support	= (list->decode)(fd, 1, list->value, len);
			else
				support	= 1;

			if (support)
			{
				if (list->desc)
				{
					if (detail)
						printf("         ");

					printf("  %s", list->desc);
				}

				printf("\n");

				if ((detail) && (list->decode))
					(list->decode)(fd, 0, list->value, len);
			}
			else
			{
				if (detail)
					printf("         ");

				printf("  (Not Supported)\n");
			}
		}

		fflush(stdout);
	}

	return(errs);
}


