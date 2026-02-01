// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/pci_plx/gsc_irq_pci.c $
// $Rev: 52657 $
// $Date: 2023-03-21 10:07:29 -0500 (Tue, 21 Mar 2023) $

// OS & Device Independent: Device Driver: PLX: source file

#include "main.h"



// macros *********************************************************************

#ifdef DEV_SUPPORTS_IRQ
#ifndef	GSC_DMA0_INT_EXTERN
#define	GSC_DMA0_INT_EXTERN(dev)
#endif

#ifndef	GSC_DMA1_INT_EXTERN
#define	GSC_DMA1_INT_EXTERN(dev)
#endif

#ifndef	OS_ISR_INTCSR_READ
#define	OS_ISR_INTCSR_READ(d)			os_reg_mem_rx_u32(NULL, (d)->vaddr.plx_intcsr_32);
#endif
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
static void _isr_abort_service(dev_data_t* dev, u32 intcsr)
{
	u16		mod		= 0;
	int		offset;
	VADDR_T	va;
	u16		val;

	// Report the PCI Abort address.
	va		= PLX_VADDR(dev, 0x104);
	val		= os_reg_mem_rx_u32(NULL, va);
	PRINTF_ISR(	"%s: Abort occurred: PCI address 0x%lX\n",
				DEV_NAME,
				(long) val);

	// Clasify the Abort, and clear it.
	offset	= 0x06;
	val		= os_reg_pci_rx_u16(dev, 0, offset);

	if (val & D11)
	{
		mod	|= D11;
		PRINTF_ISR("%s: Target Abort signaled\n", DEV_NAME);
	}

	if (val & D12)
	{
		mod	|= D12;
		PRINTF_ISR("%s: Target Abort received\n", DEV_NAME);
	}

	if (val & D13)
	{
		mod	|= D13;
		PRINTF_ISR("%s: Master Abort received\n", DEV_NAME);
	}

	if (mod)
		os_reg_pci_tx_u16(dev, 0, offset, mod);

	// Report the party involved in the Abort.

	if ((intcsr & D24) == 0)
		PRINTF_ISR("%s: Abort from Direct Master activity\n", DEV_NAME);

	if ((intcsr & D25) == 0)
	{
		dev->dma.channel[0].error	= 1;
		PRINTF_ISR("%s: Abort from DMA Channel 0 activity\n", DEV_NAME);
	}

	if ((intcsr & D26) == 0)
	{
		dev->dma.channel[1].error	= 1;
		PRINTF_ISR("%s: Abort from DMA Channel 1 activity\n", DEV_NAME);
	}

	if ((intcsr & D27) == 0)
		PRINTF_ISR("%s: Target Abort from 256 Master Retrys\n", DEV_NAME);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_init_pci(dev_data_t* dev, int lock)
{
	u32	val;

	if (dev->vaddr.plx_intcsr_32)
	{
		val	= GSC_INTCSR_PCI_INT_ENABLE		// D8
			| GSC_INTCSR_LOCAL_INT_ENABLE	// D11
			| GSC_INTCSR_ABORT_INT_ENABLE;	// D10

		if (lock)
			os_reg_mem_tx_u32(dev, dev->vaddr.plx_intcsr_32, val);
		else
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, val);
	}

	return(0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_local_disable_pci(dev_data_t* dev)
{
	u32	mask	= GSC_INTCSR_LOCAL_INT_ENABLE;

	os_reg_mem_mx_u32(dev, dev->vaddr.plx_intcsr_32, 0, mask);
	return(0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_local_enable_pci(dev_data_t* dev)
{
	u32	mask	= GSC_INTCSR_LOCAL_INT_ENABLE;

	os_reg_mem_mx_u32(dev, dev->vaddr.plx_intcsr_32, mask, mask);
	return(0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_reset_pci(dev_data_t* dev, int lock)
{
	if (dev->vaddr.plx_intcsr_32)
	{
		if (lock)
			os_reg_mem_tx_u32(dev, dev->vaddr.plx_intcsr_32, 0);
		else
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, 0);
	}

	return(0);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_create_pci(dev_data_t* dev)
{
	int	ret;

	dev->vaddr.plx_intcsr_32	= PLX_VADDR(dev, 0x68);

	// What type device is this?
	dev->irq.did	= os_reg_pci_rx_u16(dev, 1, 0x02);

	switch (dev->irq.did)
	{
		default:
		case 0x906E:

			dev->irq.isr_mask	= GSC_INTCSR_PCI_INT_ENABLE			// D8
								| GSC_INTCSR_PCI_DOOR_INT_ENABLE	// D9
								| GSC_INTCSR_ABORT_INT_ENABLE		// D10
								| GSC_INTCSR_LOCAL_INT_ENABLE		// D11
								| GSC_INTCSR_PCI_DOOR_INT_ACTIVE	// D13
								| GSC_INTCSR_ABORT_INT_ACTIVE		// D14
								| GSC_INTCSR_LOCAL_INT_ACTIVE		// D15
								| GSC_INTCSR_LOC_DOOR_INT_ENABLE	// D17
								| GSC_INTCSR_LOC_DOOR_INT_ACTIVE	// D20
								| GSC_INTCSR_BIST_INT_ACTIVE;		// D23
			break;

		case 0x9080:

			dev->irq.isr_mask	= GSC_INTCSR_MAILBOX_INT_ENABLE		// D3
								| GSC_INTCSR_PCI_INT_ENABLE			// D8
								| GSC_INTCSR_PCI_DOOR_INT_ENABLE	// D9
								| GSC_INTCSR_ABORT_INT_ENABLE		// D10
								| GSC_INTCSR_LOCAL_INT_ENABLE		// D11
								| GSC_INTCSR_PCI_DOOR_INT_ACTIVE	// D13
								| GSC_INTCSR_ABORT_INT_ACTIVE		// D14
								| GSC_INTCSR_LOCAL_INT_ACTIVE		// D15
								| GSC_INTCSR_LOC_DOOR_INT_ENABLE	// D17
								| GSC_INTCSR_DMA_0_INT_ENABLE		// D18
								| GSC_INTCSR_DMA_1_INT_ENABLE		// D19
								| GSC_INTCSR_LOC_DOOR_INT_ACTIVE	// D20
								| GSC_INTCSR_DMA_0_INT_ACTIVE		// D21
								| GSC_INTCSR_DMA_1_INT_ACTIVE		// D22
								| GSC_INTCSR_BIST_INT_ACTIVE		// D23
								| GSC_INTCSR_MAILBOX_INT_ACTIVE;	// D28-D31
			break;

		case 0x9056:
		case 0x9656:

			dev->irq.isr_mask	= GSC_INTCSR_MAILBOX_INT_ENABLE		// D3
								| GSC_INTCSR_POWER_MAN_INT_ENABLE	// D4
								| GSC_INTCSR_POWER_MAN_INT_ACTIVE	// D5
								| GSC_INTCSR_LOCAL_PE_INT_ENABLE	// D6
								| GSC_INTCSR_LOCAL_PE_INT_ACTIVE	// D7
								| GSC_INTCSR_PCI_INT_ENABLE			// D8
								| GSC_INTCSR_PCI_DOOR_INT_ENABLE	// D9
								| GSC_INTCSR_ABORT_INT_ENABLE		// D10
								| GSC_INTCSR_LOCAL_INT_ENABLE		// D11
								| GSC_INTCSR_PCI_DOOR_INT_ACTIVE	// D13
								| GSC_INTCSR_ABORT_INT_ACTIVE		// D14
								| GSC_INTCSR_LOCAL_INT_ACTIVE		// D15
								| GSC_INTCSR_LOC_DOOR_INT_ENABLE	// D17
								| GSC_INTCSR_DMA_0_INT_ENABLE		// D18
								| GSC_INTCSR_DMA_1_INT_ENABLE		// D19
								| GSC_INTCSR_LOC_DOOR_INT_ACTIVE	// D20
								| GSC_INTCSR_DMA_0_INT_ACTIVE		// D21
								| GSC_INTCSR_DMA_1_INT_ACTIVE		// D22
								| GSC_INTCSR_BIST_INT_ACTIVE		// D23
								| GSC_INTCSR_MAILBOX_INT_ACTIVE;	// D28-D31
			break;
	}

	ret	= os_irq_create(dev);
	return(ret);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
void gsc_irq_destroy_pci(dev_data_t* dev)
{
	if (dev)
		os_irq_destroy(dev);
}
#endif



//*****************************************************************************
#ifdef DEV_SUPPORTS_IRQ
int gsc_irq_isr_common_pci(dev_data_t* dev, u32 flags)
{
	gsc_dma_ch_t*	dma;
	u32				intcsr;		// PLX Interrupt Control/Status Register
	int				is_ours;	// Is this one or our interrupts?

	for (;;)
	{
		intcsr	= OS_ISR_INTCSR_READ(dev);
		intcsr	&= dev->irq.isr_mask;

		// PCI ======================================================

		if ((intcsr & GSC_INTCSR_PCI_INT_ENABLE) == 0)	// D8
		{
			// We don't have interrupts enabled. This isn't ours.
			is_ours	= 0;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_OTHER);
			break;
		}

		// DMA 0 ====================================================

		if ((intcsr & GSC_INTCSR_DMA_0_INT_ENABLE) &&	// D18
			(intcsr & GSC_INTCSR_DMA_0_INT_ACTIVE))		// D21
		{
			// This is a DMA0 interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Clear the DMA DONE interrupt.
			dma	= &dev->dma.channel[0];
			os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, GSC_DMA_CSR_CLEAR);

			// Allow any external processing.
			GSC_DMA0_INT_EXTERN(dev);

			// Disable the DMA 0 interrupt.
			intcsr	&= ~GSC_INTCSR_DMA_0_INT_ENABLE;
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN_DMA(dev, dma, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_DMA0);
			break;
		}

		// DMA 1 ====================================================

		if ((intcsr & GSC_INTCSR_DMA_1_INT_ENABLE) &&	// D19
			(intcsr & GSC_INTCSR_DMA_1_INT_ACTIVE))		// D22
		{
			// This is a DMA1 interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Clear the DMA DONE interrupt.
			dma	= &dev->dma.channel[1];
			os_reg_mem_tx_u8(NULL, dma->vaddr.csr_8, GSC_DMA_CSR_CLEAR);

			// Allow any external processing.
			GSC_DMA1_INT_EXTERN(dev);

			// Disable the DMA 1 interrupt.
			intcsr	&= ~GSC_INTCSR_DMA_1_INT_ENABLE;
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN_DMA(dev, dma, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_DMA1);
			break;
		}

		// LOCAL ====================================================

		if ((intcsr & GSC_INTCSR_LOCAL_INT_ENABLE) &&	// D11
			(intcsr & GSC_INTCSR_LOCAL_INT_ACTIVE))		// D15
		{
			// This is a LOCAL interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Let device specific code process local interrupts.
			dev_irq_isr_local_handler(dev);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_GSC);
			break;
		}

		// MAIL BOX =================================================

		if ((intcsr & GSC_INTCSR_MAILBOX_INT_ENABLE) &&		// D3
			(intcsr & GSC_INTCSR_MAILBOX_INT_ACTIVE))		// D28-D31
		{
			// This is an unexpected MAIL BOX interrupt.
			// We should never receive this interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Disable the MAIL BOX interrupt.
			intcsr	&= ~GSC_INTCSR_MAILBOX_INT_ENABLE;
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// PCI DOORBELL =============================================

		if ((intcsr & GSC_INTCSR_PCI_DOOR_INT_ENABLE) &&	// D9
			(intcsr & GSC_INTCSR_PCI_DOOR_INT_ACTIVE))		// D13
		{
			// This is an unexpected PCI DOORBELL interrupt.
			// We should never receive this interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Disable the PCI DOORBELL interrupt.
			intcsr	&= ~GSC_INTCSR_PCI_DOOR_INT_ENABLE;
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// ABORT ====================================================

		if ((intcsr & GSC_INTCSR_ABORT_INT_ENABLE) &&	// D10
			(intcsr & GSC_INTCSR_ABORT_INT_ACTIVE))		// D14
		{
			// This is an unexpected ABORT interrupt.
			// We hope to never receive any of these.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Handle the abort condition.
			_isr_abort_service(dev, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// LOCAL DOORBELL ===========================================

		if ((intcsr & GSC_INTCSR_LOC_DOOR_INT_ENABLE) &&	// D17
			(intcsr & GSC_INTCSR_LOC_DOOR_INT_ACTIVE))		// D20
		{
			// This is an unexpected Local DOORBELL interrupt.
			// We should never receive this interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Disable the Local DOORBELL interrupt.
			intcsr	&= ~GSC_INTCSR_LOC_DOOR_INT_ENABLE;
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// BIST =====================================================

		if (intcsr & GSC_INTCSR_BIST_INT_ACTIVE)	// D23
		{
			// This is an unexpected BIST interrupt.
			// We should never receive this interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Service the interrupt.
			os_reg_pci_tx_u8(dev, 0, 0x0F, 0);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// LOCAL PARITY ERROR =======================================

		if (intcsr & GSC_INTCSR_LOCAL_PE_INT_ACTIVE)	// D7
		{
			// This looks like a local parity error.
			// This can occur even if the interrupt is disabled.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Service the interrupt - write 1 to clear
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// POWER MANAGEMENT =========================================

		if ((intcsr & GSC_INTCSR_POWER_MAN_INT_ENABLE) &&	// D4
			(intcsr & GSC_INTCSR_POWER_MAN_INT_ACTIVE))		// D5
		{
			// This looks like a power management interrupt.
			is_ours	= 1;

			if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
				break;

			// Service the interrupt - write 1 to clear
			os_reg_mem_tx_u32(NULL, dev->vaddr.plx_intcsr_32, intcsr);

			// Resume any blocked threads.
			GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_SPURIOUS);
			break;
		}

		// This is not one of our interrupts.
		is_ours	= 0;

		if (flags & GSC_IRQ_ISR_FLAG_DETECT_ONLY)
			break;

		// Resume any blocked threads.
		GSC_WAIT_RESUME_IRQ_MAIN(dev, GSC_WAIT_MAIN_PCI | GSC_WAIT_MAIN_OTHER);
		break;
	}

	return(is_ours);
}
#endif


