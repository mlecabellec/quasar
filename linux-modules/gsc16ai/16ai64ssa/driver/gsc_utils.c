// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_utils.c $
// $Rev: 53839 $
// $Date: 2023-11-15 13:53:05 -0600 (Wed, 15 Nov 2023) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



/******************************************************************************
*
*	Function:	_local_stricmp
*
*	Purpose:
*
*		Compare the content of two string macros, ignoring upper and lower case.
*		This service is provided as stricmp is not always available.
*
*	Arguments:
*
*		str1	The first macro to examine.
*
*		str2	The second macro to examine.
*
*	Returned:
*
*		< 0		The first macro is less that the second.
*		0		The macros are equal.
*		> 0		The second macro is less than the first.
*
******************************************************************************/

static int _local_stricmp(const char* str1, const char* str2)
{
	int	c1;
	int	c2;
	int	ret;

	if ((str1 == NULL) && (str2 == NULL))
	{
		ret	= 0;
	}
	else if (str1 == NULL)
	{
		ret	= -1;
	}
	else if (str2 == NULL)
	{
		ret	= 1;
	}
	else
	{
		for (ret = 0;; str1++, str2++)
		{
			c1	= str1[0];
			c1	= (c1 < 'A') ? c1 : ((c1 > 'Z') ? c1 : c1 - 'A' + 'a');
			c2	= str2[0];
			c2	= (c2 < 'A') ? c2 : ((c2 > 'Z') ? c2 : c2 - 'A' + 'a');
			ret	= c2 - c1;

			if ((ret == 0) || (c1 == 0))
				break;
		}
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_macro_test_base_name
*
*	Purpose:
*
*		Validate the text of the XXX_BASE_NAME macro.
*
*	Arguments:
*
*		name	The base name to test.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_macro_test_base_name(const char* name)
{
	int	ret;

	ret	= strcmp(DEV_NAME, name);

	if (ret)
	{
		ret	= -EINVAL;
		printf(	"%s: %d. %s: base name mismatch.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
		printf("%s: DEV_NAME and XXX_BASE_NAME do not agree.\n",	DEV_NAME);
		printf("%s: DEV_NAME is '%s' from main.h.\n",				DEV_NAME, DEV_NAME);
		printf("%s: XXX_BASE_NAME is '%s' from xxx.\n",				DEV_NAME, name);
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_macro_test_model
*
*	Purpose:
*
*		Validate the text of the DEV_MODEL macro.
*
*	Arguments:
*
*		None.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_macro_test_model(void)
{
	int	ret;

	ret	= _local_stricmp(DEV_NAME, DEV_MODEL);

	if (ret)
	{
		ret	= -EINVAL;
		printf(	"%s: %d. %s: model name mismatch.\n",
				DEV_NAME,
				__LINE__,
				__FUNCTION__);
		printf("%s: DEV_NAME and DEV_MODEL do not agree.\n",	DEV_NAME);
		printf("%s: DEV_NAME is '%s' from main.h.\n",			DEV_NAME, DEV_NAME);
		printf("%s: DEV_MODEL is '%s' from main.h.\n",			DEV_NAME, DEV_MODEL);
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_poll_u32
*
*	Purpose:
*
*		Repeatedly query a 32-bit device register waiting for a field to have a
*		specific value.
*
*	Arguments:
*
*		dev		The device whose register is to be accessed.
*
*		ms		The maximum nimber of ms to wait.
*
*		vaddr	The handle for the register to access.
*
*		mask	The set of bits to look at.
*
*		value	The value desired for the masked bits.
*
*	Returned:
*
*		>= 0	The number of bytes transferred.
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_poll_u32(
	dev_data_t*	dev,
	size_t		ms_limit,
	VADDR_T		vaddr,
	u32			mask,
	u32			value)
{
	os_time_tick_t	limit;
	u32				reg;
	int				ret;

	limit	= os_time_tick_get() + os_time_ms_to_ticks((long) ms_limit);

	for (;;)
	{
		reg	= os_reg_mem_rx_u32(dev, vaddr);

		if ((reg & mask) == value)
		{
			ret	= 0;
			break;
		}

		if (os_time_tick_timedout(limit))
		{
			// We've waited long enough.
			ret	= -ETIMEDOUT;
			break;
		}

		ret	= os_time_tick_sleep(1);

		if (ret)
		{
			// The timeout returned prematurely; it was signaled.
			// We've essentially been told to quit.
			ret	= -ECANCELED;
			break;
		}
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_s32_list_reg
*
*	Purpose:
*
*		Assign an s32 value to a register field, given a list of valid options,
*		or retrieve the current setting.
*
*	Arguments:
*
*		dev		The device whose register is to be accessed.
*
*		value	The value to apply. If -1, the current option is retrieved.
*
*		list	The list of valid value options.
*
*		vaddr	The virtual address for the register to access.
*
*		begin	The beginning bit number for the field (left most bit).
*
*		end		The ending bit number for the field (right most bit).
*
*	Returned:
*
*		<0		An errno value for the error case.
*		0		All went well.
*
******************************************************************************/

int gsc_s32_list_reg(
	dev_data_t*	dev,
	s32*		value,
	const s32*	list,
	VADDR_T		vaddr,
	int			begin,
	int			end)
{
	int	i;
	u32	mask;
	int	ret		= -EINVAL;
	u32	v;

	mask	= GSC_FIELD_ENCODE(0xFFFFFFFF, begin, end);

	if (value[0] != -1)
	{
		for (i = 0; (list) && (list[i] >= 0); i++)
		{
			if (list[i] == value[0])
			{
				os_reg_mem_mx_u32(dev, vaddr, list[i] << end, mask);
				ret	= 0;
				break;
			}
		}
	}

	if ((ret == 0) || (value[0] == -1))
	{
		ret	= 0;
		v	= os_reg_mem_rx_u32(dev, vaddr);
		v	&= mask;
		v	>>= end;

		for (i = 0; (list) && (list[i] >= 0); i++)
		{
			if ((u32) list[i] == v)
			{
				value[0]	= (s32) v;
				break;
			}
		}
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_s32_list_var
*
*	Purpose:
*
*		Assign an s32 value to an s32 variabble, given a list of valid options,
*		or retrieve the current setting.
*
*	Arguments:
*
*		value	The value to apply. If -1, the current option is retrieved.
*
*		list	The list of valid value options.
*
*		var		The variable whose value is being accessed.
*
*	Returned:
*
*		<0		An errno value for the error case.
*		0		All went well.
*
******************************************************************************/

int gsc_s32_list_var(s32* value, const s32* list, s32* var)
{
	int	i;
	int	ret	= -EINVAL;

	if (value[0] != -1)
	{
		for (i = 0; (list) && (list[i] >= 0); i++)
		{
			if (list[i] == value[0])
			{
				var[0]	= list[i];
				ret		= 0;
				break;
			}
		}
	}

	if ((ret == 0) || (value[0] == -1))
	{
		ret			= 0;
		value[0]	= var[0];
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_s32_list_reg
*
*	Purpose:
*
*		Assign an s32 value to a register field, given valid range limits,
*		or retrieve the current setting.
*
*	Arguments:
*
*		dev		The device whose register is to be accessed.
*
*		value	The value to apply. If -1, the current option is retrieved.
*
*		min		The minimum valid value.
*
*		max		The maximum valid value.
*
*		vaddr	The virtual address for the register to access.
*
*		begin	The beginning bit number for the field (left most bit).
*
*		end		The ending bit number for the field (right most bit).
*
*	Returned:
*
*		<0		An errno value for the error case.
*		0		All went well.
*
******************************************************************************/

int gsc_s32_range_reg(
	dev_data_t*	dev,
	s32*		value,
	s32			min,
	s32			max,
	VADDR_T		vaddr,
	int			begin,
	int			end)
{
	u32	mask;
	int	ret		= -EINVAL;
	u32	v;

	mask	= GSC_FIELD_ENCODE(0xFFFFFFFF, begin, end);

	if (value[0] != -1)
	{
		if ((value[0] >= min) && (value[0] <= max))
		{
			os_reg_mem_mx_u32(dev, vaddr, value[0] << end, mask);
			ret	= 0;
		}
	}

	if ((ret == 0) || (value[0] == -1))
	{
		ret			= 0;
		v			= os_reg_mem_rx_u32(dev, vaddr);
		v			= (v & mask) >> end;
		value[0]	= (s32) v;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_s32_range_var
*
*	Purpose:
*
*		Assign an s32 value to a variable, given valid range limits,
*		or retrieve the current setting.
*
*	Arguments:
*
*		value	The value to apply. If -1, the current option is retrieved.
*
*		min		The minimum valid value.
*
*		max		The maximum valid value.
*
*		var		The variable whose value is being accessed.
*
*	Returned:
*
*		<0		An errno value for the error case.
*		0		All went well.
*
******************************************************************************/

int gsc_s32_range_var(s32* value, s32 min, s32 max, s32* var)
{
	int	ret	= -EINVAL;

	if (value[0] != -1)
	{
		if ((value[0] >= min) && (value[0] <= max))
		{
			var[0]	= value[0];
			ret		= 0;
		}
	}

	if ((ret == 0) || (value[0] == -1))
	{
		ret			= 0;
		value[0]	= var[0];
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	ul2hex
*
*	Purpose:
*
*		Convert an unsigned long value to a hex string starting with "0x".
*		Leading zeroes are omitted.
*
*	Arguments:
*
*		ul		The unsigned long value to convert.
*
*		dest	The destination buffer, which is assumed to be large enough
*				for the resulting string. This may be NULL, in which case it
*				is ignored.
*
*	Returned:
*
*		>= 0	The length of the resulting string minus the null terminator.
*
******************************************************************************/

int ul2hex(unsigned long ul, char* dest)
{
	const char* hex	= "0123456789ABCDEF";

	char	buf[64];
	int		i;
	int		j;
	int		len		= 0;
	int		size	= sizeof(unsigned long) * 2;
	char*	src;

	memset(buf, 0, sizeof(buf));

	// Compose the initial string of hex characters.

	for (i = 0; i < size; i++)
	{
		j		= (int) ((ul >> ((size - 1 - i) * 4)) & 0xF);
		buf[j]	= hex[j];
	}

	// Identify the characters to be copied to the destination.
	src	= buf;

	for (i = 0; i < size; i++, src++)
	{
		if ((src[0] != '0') || (src[1] == 0))
			break;
	}

	// Copy the converted string to the destination.

	if (dest)
	{
		len		= 2;
		dest[0]	= '0';
		dest[1]	= 'x';
		dest	+= 2;

		for (;;)
		{
			dest[0]	= src[0];

			if (src[0] == 0)
				break;

			dest++;
			src++;
			len++;
		}
	}

	return(len);
}


