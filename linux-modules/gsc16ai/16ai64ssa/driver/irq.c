// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/driver/irq.c $
// $Rev: 53568 $
// $Date: 2023-08-07 16:29:27 -0500 (Mon, 07 Aug 2023) $

// 16AI64SSA: Device Driver: source file

#include "main.h"



//*****************************************************************************
static void _wait_resume_irq0(dev_data_t* dev, u32 icr)
{
	u32	irq;

	irq	= GSC_FIELD_DECODE(icr, 2, 0);

	switch (irq)
	{
		default:

			gsc_wait_resume_irq_main(dev, GSC_WAIT_MAIN_SPURIOUS);
			break;

		case AI64SSA_IRQ0_INIT_DONE:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_INIT_DONE);
			break;

		case AI64SSA_IRQ0_AUTOCAL_DONE:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_AUTOCAL_DONE);
			break;

		case AI64SSA_IRQ0_SYNC_START:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_SYNC_START);
			break;

		case AI64SSA_IRQ0_SYNC_DONE:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_SYNC_DONE);
			break;

		case AI64SSA_IRQ0_BURST_START:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_BURST_START);
			break;

		case AI64SSA_IRQ0_BURST_DONE:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_BURST_DONE);
			break;
	}
}



//*****************************************************************************
static void _wait_resume_irq1(dev_data_t* dev, u32 icr)
{
	u32	irq;

	irq	= GSC_FIELD_DECODE(icr, 6, 4);

	switch (irq)
	{
		default:

			gsc_wait_resume_irq_main(dev, GSC_WAIT_MAIN_SPURIOUS);
			break;

		case AI64SSA_IRQ1_IN_BUF_THR_L2H:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_IN_BUF_THR_L2H);
			break;

		case AI64SSA_IRQ1_IN_BUF_THR_H2L:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_IN_BUF_THR_H2L);
			break;

		case AI64SSA_IRQ1_BUF_ERROR:

			gsc_wait_resume_irq_gsc(dev, AI64SSA_WAIT_GSC_BUF_ERROR);
			break;
	}
}



//*****************************************************************************
void dev_irq_isr_local_handler(dev_data_t* dev)
{
	#define	ICR_IRQ0_REQUEST	0x08
	#define	ICR_IRQ1_REQUEST	0x80

	u32	icr;
	u32	irq;

	// IRQ Anomaly: On older firmware, an unintentional interrupt is generated
	// when writing a "1" to an IRQ Request field that is "0". For firmware
	// with this anomaly there is a window of time where an interrupt can
	// occur but be missed due to inadvertently being cleared here by the ISR.
	// This ISR code reduces the window to just a very few microseconds. That
	// windows begins when the ICR is read by the ISR, and ends when the ISR
	// subsequently writes back to the ICR. This condition can arrise only when
	// the ISR is servicing one IRQ (IRQ0 or IRQ1, but not both) and a second
	// interrupt occurs within this window by the other IRQ (IRQ1 or IRQ0).

	icr	= os_reg_mem_rx_u32(NULL, dev->vaddr.gsc_icr_32);
	irq	= icr & (ICR_IRQ0_REQUEST | ICR_IRQ1_REQUEST);

	switch (irq)
	{
		default:
		case 0:

			gsc_wait_resume_irq_main(dev, GSC_WAIT_MAIN_SPURIOUS);
			break;

		case ICR_IRQ0_REQUEST:

			if (dev->cache.icr_anomaly)
				irq	= icr & ~(ICR_IRQ0_REQUEST | ICR_IRQ1_REQUEST);
			else
				irq	= (icr & ~ICR_IRQ0_REQUEST) | ICR_IRQ1_REQUEST;

			os_reg_mem_tx_u32(NULL, dev->vaddr.gsc_icr_32, irq);
			_wait_resume_irq0(dev, icr);
			break;

		case ICR_IRQ1_REQUEST:

			if (dev->cache.icr_anomaly)
				irq	= icr & ~(ICR_IRQ1_REQUEST | ICR_IRQ0_REQUEST);
			else
				irq	= (icr & ~ICR_IRQ1_REQUEST) | ICR_IRQ0_REQUEST;

			os_reg_mem_tx_u32(NULL, dev->vaddr.gsc_icr_32, irq);
			_wait_resume_irq1(dev, icr);
			break;

		case ICR_IRQ0_REQUEST | ICR_IRQ1_REQUEST:

			irq	= icr & ~(ICR_IRQ0_REQUEST | ICR_IRQ1_REQUEST);
			os_reg_mem_tx_u32(NULL, dev->vaddr.gsc_icr_32, irq);
			_wait_resume_irq0(dev, icr);
			_wait_resume_irq1(dev, icr);
			break;
	}
}



//*****************************************************************************
int dev_irq_create(dev_data_t* dev)
{
	int	ret;

	os_reg_mem_tx_u32(dev, dev->vaddr.gsc_icr_32, 0);
	ret	= gsc_irq_create(dev);
	return(ret);
}



//*****************************************************************************
void dev_irq_destroy(dev_data_t* dev)
{
	if (dev->vaddr.gsc_icr_32)
		os_reg_mem_tx_u32(dev, dev->vaddr.gsc_icr_32, 0);

	gsc_irq_destroy(dev);
}


