// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/gsc_reg.c $
// $Rev: 44662 $
// $Date: 2019-04-04 16:00:49 -0500 (Thu, 04 Apr 2019) $

// OS & Device Independent: Device Driver: source file

#include "main.h"



// data types *****************************************************************

typedef struct
{
	unsigned long	reg;
	unsigned long	type;	// BARx, PCI, Alt
	unsigned long	size;	// 1, 2, 3 or 4 bytes
	u16				offset;	// Allign per size
	VADDR_T			vaddr;	// Virtual Address
	const os_bar_t*	bar;
	dev_data_t*		dev;
} _reg_t;



// variables ******************************************************************

#ifndef	GSC_PCI_SPACE_SIZE
#define	GSC_PCI_SPACE_SIZE	256
#endif

static const os_bar_t	_pci_region	=
{
	/* index		*/	0,	// not needed for PCI registers
	/* offset		*/	0,	// not needed for PCI registers
	/* reg			*/	0,	// not needed for PCI registers
	/* flags		*/	0,	// not needed for PCI registers
	/* io_mapped	*/	0,	// not needed for PCI registers
	/* phys_adrs	*/	0,	// not needed for PCI registers
	/* size			*/	GSC_PCI_SPACE_SIZE,
	/* rw			*/	GSC_REG_TYPE_PCI_ACCESS
} ;



/******************************************************************************
*
*	Function:	_reg_decode
*
*	Purpose:
*
*		Decode a register id.
*
*	Arguments:
*
*		dev		The structure for the device to access.
*
*		bar		This is the BAR region where the register resides.
*
*		reg		This is the register of interest.
*
*		rt		The decoded register definition.
*
*	Returned:
*
*		None.
*
******************************************************************************/

static void _reg_decode(
	dev_data_t*		dev,
	const os_bar_t*	bar,
	u32				reg,
	_reg_t*			rt)
{
	rt->dev		= dev;
	rt->reg		= reg;
	rt->bar		= bar;
	rt->type	= GSC_REG_TYPE(reg);
	rt->offset	= GSC_REG_OFFSET(reg);
	rt->size	= GSC_REG_SIZE(reg);
	rt->vaddr	= (VADDR_T) ((u8*) bar->vaddr + rt->offset);
}



/******************************************************************************
*
*	Function:	_reg_validate
*
*	Purpose:
*
*		Verify that a regiter id is valid.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _reg_validate(_reg_t* rt)
{
	int				ret	= -EINVAL;
	unsigned long	ul;

	for (;;)	// We'll use a loop for convenience.
	{
		// Are there extranious bits in the register id?

		if (rt->reg & ~GSC_REG_ENCODE_MASK)
		{
			// An device may use some of the otherwise unused bits.
			// If any remaining bits are set, then there is a problem.
			printf(	"%d. _reg_validate: "
					"invalid register definition bits are set: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		// Are the fields known here valid?
		ul	= GSC_REG_ENCODE(rt->type, rt->size, rt->offset);

		if ((ul & GSC_REG_ENCODE_MASK_DEFAULT) != (rt->reg & GSC_REG_ENCODE_MASK_DEFAULT))
		{
			printf(	"%d. _reg_validate: default encoding bits are invalid: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		// Is the BAR mapped?

		if ((rt->bar->size <= 0 ) || ((rt->offset) && (rt->vaddr == NULL)))
		{
			printf(	"%d. _reg_validate: Region not accessible: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		// Does the register extend past the end of the region?
		ul	= rt->offset + rt->size - 1;

		if (ul >= rt->bar->size)
		{
			printf(	"%d. _reg_validate: register definition extends beyond end of region: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		// Is the register's size valid?

		if (strchr("\001\002\003\004", (int) rt->size) == NULL)
		{
			printf(	"%d. _reg_validate: register definition size is invalid: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		// Is the register properly aligned?

		if (((rt->size == 2) && (rt->offset & 0x1)) ||
			((rt->size == 4) && (rt->offset & 0x3)))
		{
			printf(	"%d. _reg_validate: "
					"register definition size and alignment are invalid: 0x%lX\n",
					__LINE__,
					(long) rt->reg);
			break;
		}

		//	We don't test the "type" since that is validated
		//	before the register is decoded.

		ret	= 0;
		break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_mem_mod
*
*	Purpose:
*
*		Perform a read-modify-write operation on a memory mapped register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value bits to apply.
*
*		mask	The mask of bits to modify.
*
*	Returned:
*
*		0		All went well.
*		<0		A negative errno value.
*
******************************************************************************/

static int _reg_mem_mod(const _reg_t* rt, u32 value, u32 mask)
{
	int	ret	= 0;

	// There is a special case in which arg->reg == 0. This maps to an 8-bit
	// register and is passed along for processing by lower level code.

	switch (rt->size)
	{
		default:

			ret	= -EINVAL;
			break;

		case 1:	os_reg_mem_mx_u8	(rt->dev, rt->vaddr, value, mask);	break;
		case 2:	os_reg_mem_mx_u16	(rt->dev, rt->vaddr, value, mask);	break;
		case 4:	os_reg_mem_mx_u32	(rt->dev, rt->vaddr, value, mask);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_mem_read
*
*	Purpose:
*
*		Read a value from a memory mapped register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value read is recorded here.
*
*	Returned:
*
*		0		All went well.
*		<0		A negative errno value.
*
******************************************************************************/

static int _reg_mem_read(const _reg_t* rt, u32* value)
{
	int	ret	= 0;

	// There is a special case in which arg->reg == 0. This maps to an 8-bit
	// register and is passed along for processing by lower level code.

	switch (rt->size)
	{
		default:

			value[0]		= 0xDEADBEEF;
			ret				= -EINVAL;
			break;

		case 1:	value[0]	= os_reg_mem_rx_u8	(rt->dev, rt->vaddr);	break;
		case 2:	value[0]	= os_reg_mem_rx_u16	(rt->dev, rt->vaddr);	break;
		case 4:	value[0]	= os_reg_mem_rx_u32	(rt->dev, rt->vaddr);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_mem_write
*
*	Purpose:
*
*		Write a value to a memory mapped register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value to write to the register.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _reg_mem_write(const _reg_t* rt, u32 value)
{
	int	ret	= 0;

	// There is a special case in which arg->reg == 0. This maps to an 8-bit
	// register and is passed along for processing by lower level code.

	switch (rt->size)
	{
		default:

			ret	= -EINVAL;
			break;

		case 1:	os_reg_mem_tx_u8	(rt->dev, rt->vaddr, value);	break;
		case 2:	os_reg_mem_tx_u16	(rt->dev, rt->vaddr, value);	break;
		case 4:	os_reg_mem_tx_u32	(rt->dev, rt->vaddr, value);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_pci_mod
*
*	Purpose:
*
*		Perform a read-modify-write operation on a PCI register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value bits to apply.
*
*		mask	The mask of bits to modify.
*
*	Returned:
*
*		0		All went well.
*		<0		A negative errno value.
*
******************************************************************************/

static int _reg_pci_mod(const _reg_t* rt, u32 value, u32 mask)
{
	int	ret	= 0;

	switch (rt->size)
	{
		default:

			ret	= -EINVAL;
			break;

		case 1:	os_reg_pci_mx_u8	(rt->dev, 1, rt->offset, value, mask);	break;
		case 2:	os_reg_pci_mx_u16	(rt->dev, 1, rt->offset, value, mask);	break;
		case 4:	os_reg_pci_mx_u32	(rt->dev, 1, rt->offset, value, mask);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_pci_read_3
*
*	Purpose:
*
*		Read a PCI register that is three bytes long. Any alignment is
*		supported.
*
*	Arguments:
*
*		dev		The structure for the device to access.
*
*		rt		The decoded register definition.
*
*		value	The value read is recorded here.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _reg_pci_read_3(const _reg_t* rt, u32* value)
{
	int	ret;
	u32	v1;
	u32	v2;

	if (((rt->offset & 3) == 0) && (rt->offset <= (GSC_PCI_SPACE_SIZE - 4)))
	{
		v1			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset);
		value[0]	= v1 & 0x00FFFFFF;
		ret			= 0;
	}
	else if (((rt->offset & 3) == 1) && (rt->offset <= (GSC_PCI_SPACE_SIZE - 4)))
	{
		v1			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset - 1);
		value[0]	= (v1 & 0xFFFFFF00) >> 8;
		ret			= 0;
	}
	else if (((rt->offset & 3) == 2) && (rt->offset <= (GSC_PCI_SPACE_SIZE - 8)))
	{
		v1			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset - 2);
		v2			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset + 2);
		value[0]	= ((v1 & 0xFFFF0000) >> 16) | ((v2 & 0x000000FF) << 16);
		ret			= 0;
	}
	else if (((rt->offset & 3) == 3) && (rt->offset <= (GSC_PCI_SPACE_SIZE - 8)))
	{
		v1			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset - 3);
		v2			= os_reg_pci_rx_u32(rt->dev, 1, rt->offset + 1);
		value[0]	= ((v1 & 0xFF000000) >> 24) | ((v2 & 0x0000FFFF) << 8);
		ret			= 0;
	}
	else
	{
		value[0]	= 0;
		ret			= -EINVAL;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_pci_read
*
*	Purpose:
*
*		Read a value from a PCI register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value read is recorded here.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _reg_pci_read(const _reg_t* rt, u32* value)
{
	int	ret	= 0;

	switch (rt->size)
	{
		default:

			ret				= -EINVAL;
			break;

		case 3:	ret			= _reg_pci_read_3(rt, value);					break;

		case 1:	value[0]	= os_reg_pci_rx_u8	(rt->dev, 1, rt->offset);	break;
		case 2:	value[0]	= os_reg_pci_rx_u16	(rt->dev, 1, rt->offset);	break;
		case 4:	value[0]	= os_reg_pci_rx_u32	(rt->dev, 1, rt->offset);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	_reg_pci_write
*
*	Purpose:
*
*		Write a value to a PCI register.
*
*	Arguments:
*
*		rt		The decoded register definition.
*
*		value	The value to write to the register.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

static int _reg_pci_write(const _reg_t* rt, u32 value)
{
	int	ret	= 0;

	switch (rt->size)
	{
		default:

			ret	= -EINVAL;
			break;

		case 1:	os_reg_pci_tx_u8	(rt->dev, 1, rt->offset, value);	break;
		case 2:	os_reg_pci_tx_u16	(rt->dev, 1, rt->offset, value);	break;
		case 4:	os_reg_pci_tx_u32	(rt->dev, 1, rt->offset, value);	break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_reg_mod_ioctl
*
*	Purpose:
*
*		Implement the Register Modify (read-modify-write) IOCTL service.
*
*	Arguments:
*
*		alt		The structure to access.
*
*		arg		The IOCTL service's required argument.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_reg_mod_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg)
{
	dev_data_t*		dev		= GSC_ALT_DEV_GET(alt);
	int				ret;
	_reg_t			rt;
	unsigned long	type	= GSC_REG_TYPE(arg->reg);

	// There is a special case in which arg->reg == 0. This maps to a BAR0
	// register and is passed along for processing by lower level code.

	switch (type)
	{
		default:

			ret	= -EINVAL;
			break;

		case GSC_REG_TYPE_ALT:

			ret	= dev_reg_mod_alt(alt, arg);
			break;

		case GSC_REG_TYPE_BAR0:
		case GSC_REG_TYPE_BAR1:
		case GSC_REG_TYPE_BAR2:
		case GSC_REG_TYPE_BAR3:
		case GSC_REG_TYPE_BAR4:
		case GSC_REG_TYPE_BAR5:

			// There is a special case in which arg->reg == 0. This maps to a
			// BAR0 register and is passed along for processing by lower level
			// code.

			_reg_decode(dev, &dev->bar.bar[(int) type], arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (arg->reg == 0)
				rt.vaddr	= 0;

			if (ret)
				;
			else if (rt.bar->rw == 0)
				ret	= -EINVAL;
			else if (rt.bar->io_mapped)
				ret	= -EINVAL;
			else
				ret	= _reg_mem_mod(&rt, arg->value, arg->mask);

			break;

		case GSC_REG_TYPE_PCI:

			_reg_decode(dev, &_pci_region, arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (ret)
				;
			else if (rt.bar->rw == 0)
				ret	= -EINVAL;
			else
				ret	= _reg_pci_mod(&rt, arg->value, arg->mask);

			break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_reg_read_ioctl
*
*	Purpose:
*
*		Implement the Register Read IOCTL service.
*
*	Arguments:
*
*		alt		The structure to access.
*
*		arg		The IOCTL service's required argument.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_reg_read_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg)
{
	dev_data_t*		dev		= GSC_ALT_DEV_GET(alt);
	int				ret;
	_reg_t			rt;
	unsigned long	type	= GSC_REG_TYPE(arg->reg);

	// There is a special case in which arg->reg == 0. This maps to a BAR0
	// register and is passed along for processing by lower level code.

	switch (type)
	{
		default:

			ret	= -EINVAL;
			break;

		case GSC_REG_TYPE_ALT:

			ret	= dev_reg_read_alt(alt, arg);
			break;

		case GSC_REG_TYPE_BAR0:
		case GSC_REG_TYPE_BAR1:
		case GSC_REG_TYPE_BAR2:
		case GSC_REG_TYPE_BAR3:
		case GSC_REG_TYPE_BAR4:
		case GSC_REG_TYPE_BAR5:

			// There is a special case in which arg->reg == 0. This maps to a
			// BAR0 register and is passed along for processing by lower level
			// code.

			_reg_decode(dev, &dev->bar.bar[(int) type], arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (arg->reg == 0)
				rt.vaddr	= 0;

			if (ret)
				;
			else if (rt.bar->io_mapped)
				ret	= -EINVAL;
			else
				ret	= _reg_mem_read(&rt, &arg->value);

			break;

		case GSC_REG_TYPE_PCI:

			_reg_decode(dev, &_pci_region, arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (ret == 0)
				ret	= _reg_pci_read(&rt, &arg->value);

			break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_reg_write_ioctl
*
*	Purpose:
*
*		Implement the Register Write IOCTL service.
*
*	Arguments:
*
*		alt		The structure to access.
*
*		arg		The IOCTL service's required argument.
*
*	Returned:
*
*		0		All went well.
*		< 0		There was a problem and this is the error status.
*
******************************************************************************/

int gsc_reg_write_ioctl(GSC_ALT_STRUCT_T* alt, gsc_reg_t* arg)
{
	dev_data_t*		dev		= GSC_ALT_DEV_GET(alt);
	int				ret;
	_reg_t			rt;
	unsigned long	type	= GSC_REG_TYPE(arg->reg);

	// There is a special case in which arg->reg == 0. This maps to a BAR0
	// register and is passed along for processing by lower level code.

	switch (type)
	{
		default:

			ret	= -EINVAL;
			break;

		case GSC_REG_TYPE_ALT:

			ret	= dev_reg_write_alt(alt, arg);
			break;

			// There is a special case in which arg->reg == 0. This maps to a
			// BAR0 register and is passed along for processing by lower level
			// code.

		case GSC_REG_TYPE_BAR0:
		case GSC_REG_TYPE_BAR1:
		case GSC_REG_TYPE_BAR2:
		case GSC_REG_TYPE_BAR3:
		case GSC_REG_TYPE_BAR4:
		case GSC_REG_TYPE_BAR5:

			_reg_decode(dev, &dev->bar.bar[(int) type], arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (arg->reg == 0)
				rt.vaddr	= 0;

			if (ret)
				;
			else if (rt.bar->rw == 0)
				ret	= -EINVAL;
			else if (rt.bar->io_mapped)
				ret	= -EINVAL;
			else
				ret	= _reg_mem_write(&rt, arg->value);

			break;

		case GSC_REG_TYPE_PCI:

			_reg_decode(dev, &_pci_region, arg->reg, &rt);
			ret	= _reg_validate(&rt);

			if (ret)
				;
			else if (rt.bar->rw == 0)
				ret	= -EINVAL;
			else
				ret	= _reg_pci_write(&rt, arg->value);

			break;
	}

	return(ret);
}



/******************************************************************************
*
*	Function:	gsc_reg_mod
*
*	Purpose:
*
*		Perform a read-modify-write on a register. GSC REGISTERS ONLY!
*
*	Arguments:
*
*		alt		The structure to access.
*
*		reg		The register to access.
*
*		val		The value to write to the register.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_reg_mod(GSC_ALT_STRUCT_T* alt, u32 reg, u32 val, u32 mask)
{
	gsc_reg_t	arg;

	arg.reg		= reg;
	arg.value	= val;
	arg.mask	= mask;
	gsc_reg_mod_ioctl(alt, &arg);
}


/******************************************************************************
*
*	Function:	gsc_reg_read
*
*	Purpose:
*
*		Read a value from a register.
*
*	Arguments:
*
*		alt		The structure to access.
*
*		reg		The register to access.
*
*	Returned:
*
*		The value read.
*
******************************************************************************/

u32 gsc_reg_read(GSC_ALT_STRUCT_T* alt, u32 reg)
{
	gsc_reg_t	arg;

	arg.reg		= reg;
	arg.value	= 0xDEADBEEF;
	arg.mask	= 0;
	gsc_reg_read_ioctl(alt, &arg);
	return(arg.value);
}



/******************************************************************************
*
*	Function:	gsc_reg_write
*
*	Purpose:
*
*		Write a value to a register. GSC REGISTERS ONLY!
*
*	Arguments:
*
*		alt		The structure to access.
*
*		reg		The register to access.
*
*		val		The value to write to the register.
*
*	Returned:
*
*		None.
*
******************************************************************************/

void gsc_reg_write(GSC_ALT_STRUCT_T* alt, u32 reg, u32 val)
{
	gsc_reg_t	arg;

	arg.reg		= reg;
	arg.value	= val;
	arg.mask	= 0;
	gsc_reg_write_ioctl(alt, &arg);
}


