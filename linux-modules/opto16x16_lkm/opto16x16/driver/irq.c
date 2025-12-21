// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/driver/irq.c $
// $Rev: 44249 $
// $Date: 2018-12-10 11:09:26 -0600 (Mon, 10 Dec 2018) $

// OPTO16X16: Device Driver: source file

#include "main.h"



//*****************************************************************************
void dev_irq_isr_local_handler(dev_data_t* dev)
{
	u8	bcsr;
	u16	cier;
	u16	cosr;
	u16	mask;

	for (;;)	// A convenience loop.
	{
		// Check the COS interrupts.
		cier	= os_reg_mem_rx_u16(NULL, dev->vaddr.gsc_cier_16);
		cosr	= os_reg_mem_rx_u16(NULL, dev->vaddr.gsc_cosr_16);
		mask	= cier & cosr;

		if (mask)
		{
			// Service the respective interrupts.
			os_reg_mem_tx_u16(NULL, dev->vaddr.gsc_cosr_16, mask);

			// Resume any threads awiting any of these interrupts.
			gsc_wait_resume_irq_gsc(dev, mask);
			break;
		}

		// Check the Rx Event Counter Interrupt.
		bcsr	= os_reg_mem_rx_u8(NULL, dev->vaddr.gsc_bcsr_8);
		mask	= bcsr & 0x48;

		if (mask == 0x48)
		{
			// Service the respective interrupts.
			bcsr	= (bcsr & 0xC0) | 0x8;
			os_reg_mem_tx_u8(NULL, dev->vaddr.gsc_bcsr_8, bcsr);

			// Resume any threads awiting any of these interrupts.
			gsc_wait_resume_irq_gsc(dev, OPTO16X16_WAIT_GSC_EVENT_COUNT);
			break;
		}

		// We don't know the source of the interrupt.
		gsc_wait_resume_irq_main(dev, GSC_WAIT_MAIN_SPURIOUS);
		break;
	}
}



//*****************************************************************************
int dev_irq_create(dev_data_t* dev)
{
	int	ret;

	os_reg_mem_tx_u8(dev, dev->vaddr.gsc_bcsr_8, 0);			// Disable IRQs.
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cier_16, 0);		// Disable IRQs.
	os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cosr_16, 0xFFFF);	// Clear IRQs.
	ret	= gsc_irq_create(dev);
	return(ret);
}



//*****************************************************************************
void dev_irq_destroy(dev_data_t* dev)
{
	if (dev->vaddr.gsc_bcsr_8)
		os_reg_mem_tx_u8(dev, dev->vaddr.gsc_bcsr_8, 0);			// Disable IRQs.

	if (dev->vaddr.gsc_cier_16)
		os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cier_16, 0);		// Disable IRQs.

	if (dev->vaddr.gsc_cosr_16)
		os_reg_mem_tx_u16(dev, dev->vaddr.gsc_cosr_16, 0xFFFF);	// Clear IRQs.

	gsc_irq_destroy(dev);
}


