// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/pci_plx/gsc_endian_pci.c $
// $Rev: 51062 $
// $Date: 2022-05-31 10:22:49 -0500 (Tue, 31 May 2022) $

// OS & Device Independent: Device Driver: PLX: source file

#include "main.h"



//*****************************************************************************
int gsc_endian_init_pci(dev_data_t* dev)
{
	u16		did		= os_reg_pci_rx_u16(dev, 1, 0x02);
	int		ret		= 0;
	VADDR_T	va		= PLX_VADDR(dev, 0x0C);
	u32		val;

#if defined(__BIG_ENDIAN)

	val	= 0xE6;

#else

	val	= 0x00;

#endif

	switch (did)
	{
		default:		ret	= -EOPNOTSUPP;
						printf(	"%s: %d. %s: PLX device not supported: 0x%04lX\n",
								DEV_NAME,
								__LINE__,
								__FUNCTION__,
								(long) did);
						break;

		case 0x8111:	// The PEX8111 driver does nothing with Endianness.
		case 0x8112:	// The PEX8112 driver does nothing with Endianness.
						break;

		case 0x906E:	// 9060
						os_reg_mem_tx_u8(dev, va, val & 0x0F);
						break;

		case 0x9056:
		case 0x9080:
		case 0x9656:	os_reg_mem_tx_u8(dev, va, val & 0xFF);
						break;
	}

	return(ret);
}


