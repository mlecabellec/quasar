// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/reg.c $
// $Rev: 53723 $
// $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



// variables ******************************************************************

static menu_item_t	_name_list[65];	// Null terminate the list!!!
static int			_reg_id[65];
static menu_t		_reg_names;



//*****************************************************************************
static char* _trim_ws(char* str)
{
	int	i;

	if (str)
	{
		for (; (str) && (isspace(str[0]));)
			str++;

		i	= (int) strlen(str);

		for (; i > 0;)
		{
			i--;

			if (isspace(str[i]))
				str[i]	= 0;
		}
	}

	return(str);
}



//*****************************************************************************
static void _reg_mod(int fd, const char* name, int reg)
{
	char	buf[1024];
	char	c		= 0;
	int		errs;
	int		i;
	long	l;
	char*	psz;
	u32		value;

	printf("    %s\n", name);
	errs	= opto16x16_reg_read(fd, -1, 0, reg, &value);

	if (errs == 0)
	{
		printf("    Current Value:  %08lX\n", (long) value);
		printf("    Value To Write: ");

		// Get the user's input.
		fgets(buf, sizeof(buf), stdin);
		psz	= _trim_ws(buf);

		if ((psz[0] == '0') && (psz[1] == 'x'))
			psz	+= 2;

		i	= sscanf(psz, "%lX%c", &l, &c);

		if (i != 1)
		{
			printf("ERROR: Invalid input\n");
			errs	= 1;
		}
		else if (l != (l & 0xFFFFFFFF))
		{
			printf("ERROR: Invalid value\n");
			errs	= 1;
		}
		else
		{
			errs	= opto16x16_reg_write(fd, -1, 0, reg, l);
		}
	}

	if (errs == 0)
		errs	= opto16x16_reg_read(fd, -1, 0, reg, &value);

	if (errs == 0)
		printf("    Updated Value:  %08lX\n", (long) value);
}



//*****************************************************************************
static void _mod_by_name_util(int fd, u32 first, u32 next)
{
	const gsc_reg_def_t*	def;
	int						i;
	int						index	= 0;
	const char*				name;
	int						start	= 0;

	memset(_name_list, 0, sizeof(_name_list));

	for (i = 0;; i++)
	{
		def	= opto16x16_reg_get_def_index(i);

		if (def == NULL)
			break;

		if (start == 0)
		{
			if (def->reg == first)
				start	= 1;
			else
				continue;
		}

		if (def->reg == next)
			break;

		name	= def->name;

		if (memcmp(name, "GSC ", 4) == 0)
			name	+= 4;

		_reg_id[index]			= def->reg;
		_name_list[index].name	= name;
		_name_list[index].func	= NULL;
		index++;
	}

	_name_list[index].name	= NULL;
	_name_list[index].func	= NULL;
	_reg_names.title		= "GSC Register List";
	_reg_names.list			= _name_list;
	i	= menu_select(&_reg_names);

	_reg_mod(fd, _name_list[i].name, _reg_id[i]);
}



//*****************************************************************************
void reg_mod_by_name(int fd)
{
	_mod_by_name_util(fd, OPTO16X16_GSC_BCSR, 0);
}



//*****************************************************************************
void reg_mod_by_offset(int fd)
{
	char	buf[1024];
	char	c		= 0;
	int		i;
	long	l;
	char*	psz;
	int		reg;

	printf("Edit a register by an offset.\n");
	printf("  Enter a 4-Byte aligned, hex offset from 0x00 to 0x7C: ");

	// Get the user's input.
	fgets(buf, sizeof(buf), stdin);
	psz	= _trim_ws(buf);

	if ((psz[0] == '0') && (psz[1] == 'x'))
		psz	+= 2;

	i	= sscanf(psz, "%lX%c", &l, &c);

	if (i != 1)
	{
		printf("ERROR: Invalid input\n");
	}
	else if ((l < 0) || (l > 0x7C))
	{
		printf("ERROR: The value is out of range.\n");
	}
	else if (l % 4)
	{
		printf("ERROR: The value is not 4-Byte aligned.\n");
	}
	else
	{
		sprintf(buf, "Offset 0x%02lX", (long) l);
		reg	= GSC_REG_ENCODE(GSC_REG_TYPE_BAR2, 4, l);
		_reg_mod(fd, buf, reg);
	}
}



