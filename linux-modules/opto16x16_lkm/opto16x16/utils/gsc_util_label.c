// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/utils/gsc_util_label.c $
// $Rev: 49741 $
// $Date: 2021-11-22 16:29:23 -0600 (Mon, 22 Nov 2021) $

// OS & Device Independent: Utility: source file

#include "main.h"



// variables ******************************************************************

static	int			_level			= 0;
static	int			_width			= 0;

const int* const	gsc_label_level	= &_level;
const int* const	gsc_label_width	= &_width;



/******************************************************************************
*
*	Function:	gsc_label_indent
*
*	Purpose:
*
*		Print the space that preceeds the label.
*
*	Arguments:
*
*		delta	Adjust the indenting by this amount.
*
*	Returned:
*
*		The number of spaces printed.
*
******************************************************************************/

int gsc_label_indent(int delta)
{
	int	i;
	int	spaces	= 0;

	for (i = 0; i < (_level + delta); i++, spaces += 2)
		printf("  ");

	fflush(stdout);
	return(spaces);
}



/******************************************************************************
*
*	Function:	gsc_label
*
*	Purpose:
*
*		Print a message introducing a test.
*
*	Arguments:
*
*		label	The test name. If this is NULL, then we just output spaces.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label(const char* label)
{
	gsc_label_suffix(label, ":");
}



/******************************************************************************
*
*	Function:	gsc_label_float_comma
*
*	Purpose:
*
*		Print a value with commas.
*
*	Arguments:
*
*		value	The positive value to print.
*
*		len		The minimum field width, as in printf("%<len>f", value);
*				Use -1 to ignore this value.
*
*		places	The number of decimal places, as in printf("%<len>.<places>f", value);
*				Use -1 to ignore this value.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_float_comma(double value, int len, int places)
{
	char	buf[128];

	gsc_label_float_comma_buf(value, len, places, buf);
	printf("%s", buf);
}



/******************************************************************************
*
*	Function:	gsc_label_float_comma_buf
*
*	Purpose:
*
*		Print a value with commas.
*
*	Arguments:
*
*		value	The positive value to print.
*
*		len		The minimum field width, as in printf("%<len>f", value);
*				Use -1 to ignore this value.
*
*		places	The number of decimal places, as in printf("%<len>.<places>f", value);
*				Use -1 to ignore this value.
*
*		dest	Print the value to this buffer. It is assumed to be large enough.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_float_comma_buf(double value, int len, int places, char* dest)
{
	char	buf[128];
	char*	dst			= dest;
	char	format[32];
	int		i;
	char*	ptr;

	if ((len >= 0) && (places >= 0))
		sprintf(format, "%%%d.%df", len, places);
	else if (len >= 0)
		sprintf(format, "%%%df", len);
	else if (places >= 0)
		sprintf(format, "%%.%df", places);
	else
		sprintf(format, "%%f");

	sprintf(buf, format, value);
	ptr	= strchr(buf, '.');

	if (ptr)
		ptr[0]	= 0;

	len	= (int) strlen(buf);

	for (i = len; i; i--)
	{
		if (i == len)
			;
		else if ((i % 3) == 0)
			*dest++	=',';

		*dest++	= buf[len - i];
	}

	dest[0]	= 0;

	if (ptr)
	{
		ptr[0]	= '.';
		strcpy(dest, ptr);
	}

	// Remove the leading commas.

	if (dst[0] == ',')
		dst[0]	= ' ';

	for (ptr = dst;; ptr++)
	{
		if ((ptr[0] == 0) || (ptr[0] == '.'))
			break;

		if ((ptr[0] == ',') && (isdigit(ptr[-1]) == 0))
			ptr[0]	= ' ';
	}

	// Move the negative sign up against the digits.

	for (ptr = dst;; ptr++)
	{
		if ((ptr[0] == 0) || (ptr[0] == '.'))
			break;

		if ((ptr[0] == '-') && (ptr[1] == ' '))
		{
			ptr[0]	= ' ';
			ptr[1]	= '-';
		}
	}
}



/******************************************************************************
*
*	Function:	gsc_label_index
*
*	Purpose:
*
*		Print a message introducing a test. If the index number is >= 0, then
*		append the index number to the label.
*
*	Arguments:
*
*		label	The test name.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_index(const char* label, int index)
{
	char	buf[128];

	strcpy(buf, label);

	if (index >= 0)
		sprintf(buf + strlen(buf), " #%d", index);

	gsc_label(buf);
}



/******************************************************************************
*
*	Function:	gsc_label_init
*
*	Purpose:
*
*		Initialize the label code.
*
*	Arguments:
*
*		width	The default width for the label field.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_init(int width)
{
	_width	= width;
}



/******************************************************************************
*
*	Function:	gsc_label_level_dec
*
*	Purpose:
*
*		Decrease the label indenting.
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

void gsc_label_level_dec(void)
{
	_level--;
}



/******************************************************************************
*
*	Function:	gsc_label_level_inc
*
*	Purpose:
*
*		Increase the label indenting.
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

void gsc_label_level_inc(void)
{
	_level++;
}



/******************************************************************************
*
*	Function:	gsc_label_long_comma
*
*	Purpose:
*
*		Print a value with commas.
*
*	Arguments:
*
*		value	The positive value to print.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_long_comma(long long value)
{
	char	buf[64];

	gsc_label_long_comma_buf(value, buf);
	printf("%s", buf);
}



/******************************************************************************
*
*	Function:	gsc_label_long_comma_buf
*
*	Purpose:
*
*		Print a value with commas.
*
*	Arguments:
*
*		value	The positive value to print.
*
*		dest	Print the value to this buffer. It is assumed to be large enough.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_long_comma_buf(long long value, char* dest)
{
	char	buf[32];
	int		i;
	int		len;

	if (value < 0)
	{
		*dest++	= '-';
		value		= -value;
	}

	sprintf(buf, "%lld", value);
	len	= (int) strlen(buf);

	for (i = len; i; i--)
	{
		if (i == len)
			;
		else if ((i % 3) == 0)
			*dest++	=',';

		*dest++	= buf[len - i];
	}

	dest[0]	= 0;
}



/******************************************************************************
*
*	Function:	gsc_label_suffix
*
*	Purpose:
*
*		Print a message introducing a test, but withg a suffix other than ":".
*
*	Arguments:
*
*		label	The test name. If this is NULL, then we just output spaces.
*
*		siffix	The suffix to use in place of the ":" sequence.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_label_suffix(const char* label, const char* suffix)
{
	int	spaces;

	if (label)
	{
		spaces	= _width - (int) strlen(label);
		spaces	= (spaces < 0) ? 0 : spaces;
		spaces	-= gsc_label_indent(0);
		printf("%s%s %*s", label, suffix, spaces, "");
	}
	else
	{
		spaces	= _width;
		spaces	= (spaces < 0) ? 0 : spaces;
		spaces	-= gsc_label_indent(0);
		printf("  %*s", spaces, "");
	}

	fflush(stdout);
}


