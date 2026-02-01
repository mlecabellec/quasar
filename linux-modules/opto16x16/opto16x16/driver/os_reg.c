// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_reg.c $
// $Rev: 50965 $
// $Date: 2022-04-25 08:35:42 -0500 (Mon, 25 Apr 2022) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
u8 os_reg_mem_mx_u8(dev_data_t* dev, VADDR_T va, u8 value, u8 mask)
{
	u8	reg	= 0xEF;	// lower portion of DEADBEEF
	u8	set;

	// There is a special case for reads of register id value 0. This maps to
	// an 8-bit, BAR0 register. At this level we see it as va == 0.

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
	{
		reg	= readb(va);
		set	= (reg & ~mask) | (value & mask);
		writeb(set, va);
	}

	if (dev)
		gsc_irq_access_unlock(dev);

	return(reg);
}



//*****************************************************************************
u16 os_reg_mem_mx_u16(dev_data_t* dev, VADDR_T va, u16 value, u16 mask)
{
	u16	reg	= 0xBEEF;	// lower portion of DEADBEEF
	u16	set;

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
	{
		reg	= readw(va);
		set	= (reg & ~mask) | (value & mask);
		writew(set, va);
	}

	if (dev)
		gsc_irq_access_unlock(dev);

	return(reg);
}



//*****************************************************************************
u32 os_reg_mem_mx_u32(dev_data_t* dev, VADDR_T va, u32 value, u32 mask)
{
	u32	reg	= 0xDEAFBEEF;
	u32	set;

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
	{
		reg	= readl(va);
		set	= (reg & ~mask) | (value & mask);
		writel(set, va);
	}

	if (dev)
		gsc_irq_access_unlock(dev);

	return(reg);
}



//*****************************************************************************
u8 os_reg_mem_rx_u8(dev_data_t* dev, VADDR_T va)
{
	u8	value	= 0xEF;	// lower portion of DEADBEEF

	// There is a special case for reads of register id value 0. This maps to
	// an 8-bit, BAR0 register. At this level we see it as va == 0.

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		value	= readb(va);

	if (dev)
		gsc_irq_access_unlock(dev);

	return(value);
}



//*****************************************************************************
u16 os_reg_mem_rx_u16(dev_data_t* dev, VADDR_T va)
{
	u16	value	= 0xBEEF;	// lower portion of DEADBEEF

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		value	= readw(va);

	if (dev)
		gsc_irq_access_unlock(dev);

	return(value);
}



//*****************************************************************************
u32 os_reg_mem_rx_u32(dev_data_t* dev, VADDR_T va)
{
	u32	value	= 0xDEADBEEF;

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		value	= readl(va);

	if (dev)
		gsc_irq_access_unlock(dev);

	return(value);
}



//*****************************************************************************
void os_reg_mem_tx_u8(dev_data_t* dev, VADDR_T va, u8 value)
{
	// There is a special case for reads of register id value 0. This maps to
	// an 8-bit, BAR0 register. At this level we see it as va == 0.

	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		writeb(value, va);

	if (dev)
		gsc_irq_access_unlock(dev);
}



//*****************************************************************************
void os_reg_mem_tx_u16(dev_data_t* dev, VADDR_T va, u16 value)
{
	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		writew(value, va);

	if (dev)
		gsc_irq_access_unlock(dev);
}



//*****************************************************************************
void os_reg_mem_tx_u32(dev_data_t* dev, VADDR_T va, u32 value)
{
	if (dev)
		gsc_irq_access_lock(dev);

	if (va)
		writel(value, va);

	if (dev)
		gsc_irq_access_unlock(dev);
}



//*****************************************************************************
u8 os_reg_pci_mx_u8(dev_data_t* dev, int lock, u16 offset, u8 value, u8 mask)
{
	u8	reg	= 0xEF;	// lower portion of DEADBEEF
	u8	set;

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_byte(dev->pci.pd, offset, &reg);
		set	= (reg & ~mask) | (value & mask);
		pci_write_config_byte(dev->pci.pd, offset, set);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
u16 os_reg_pci_mx_u16(dev_data_t* dev, int lock, u16 offset, u16 value, u16 mask)
{
	u16	reg	= 0xBEEF;	// lower portion of DEADBEEF
	u16	set;

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_word(dev->pci.pd, offset, &reg);
		set	= (reg & ~mask) | (value & mask);
		pci_write_config_word(dev->pci.pd, offset, set);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
u32 os_reg_pci_mx_u32(dev_data_t* dev, int lock, u16 offset, u32 value, u32 mask)
{
	u32	reg	= 0xDEADBEEF;
	u32	set;

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_dword(dev->pci.pd, offset, &reg);
		set	= (reg & ~mask) | (value & mask);
		pci_write_config_dword(dev->pci.pd, offset, set);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
u8 os_reg_pci_rx_u8(dev_data_t* dev, int lock, u16 offset)
{
	u8	reg	= 0xEF;	// lower portion of DEADBEEF

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_byte(dev->pci.pd, offset, &reg);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
u16 os_reg_pci_rx_u16(dev_data_t* dev, int lock, u16 offset)
{
	u16	reg	= 0xBEEF;	// lower portion of DEADBEEF

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_word(dev->pci.pd, offset, &reg);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
u32 os_reg_pci_rx_u32(dev_data_t* dev, int lock, u16 offset)
{
	u32	reg	= 0xDEADBEEF;

	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_read_config_dword(dev->pci.pd, offset, &reg);

		if (lock)
			gsc_irq_access_unlock(dev);
	}

	return(reg);
}



//*****************************************************************************
void os_reg_pci_tx_u8(dev_data_t* dev, int lock, u16 offset, u8 value)
{
	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_write_config_byte(dev->pci.pd, offset, value);

		if (lock)
			gsc_irq_access_unlock(dev);
	}
}



//*****************************************************************************
void os_reg_pci_tx_u16(dev_data_t* dev, int lock, u16 offset, u16 value)
{
	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_write_config_word(dev->pci.pd, offset, value);

		if (lock)
			gsc_irq_access_unlock(dev);
	}
}



//*****************************************************************************
void os_reg_pci_tx_u32(dev_data_t* dev, int lock, u16 offset, u32 value)
{
	if (dev)
	{
		if (lock)
			gsc_irq_access_lock(dev);

		pci_write_config_dword(dev->pci.pd, offset, value);

		if (lock)
			gsc_irq_access_unlock(dev);
	}
}


