// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/pci_plx/gsc_main_pci.h $
// $Rev: 50964 $
// $Date: 2022-04-25 08:31:03 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: PLX: header file

#ifndef	__GSC_MAIN_PCI_H__
#define	__GSC_MAIN_PCI_H__




// macros *********************************************************************

// Virtual address items
#define	PLX_VADDR(d,o)						(VADDR_T) (((u8*) (d)->bar.bar[0].vaddr) + (o))

// DMA
#define	GSC_DMA_CSR_DISABLE					GSC_FIELD_ENCODE(0,0,0)
#define	GSC_DMA_CSR_ENABLE					GSC_FIELD_ENCODE(1,0,0)
#define	GSC_DMA_CSR_START					GSC_FIELD_ENCODE(1,1,1)
#define	GSC_DMA_CSR_ABORT					GSC_FIELD_ENCODE(1,2,2)
#define	GSC_DMA_CSR_CLEAR					GSC_FIELD_ENCODE(1,3,3)
#define	GSC_DMA_CSR_DONE					GSC_FIELD_ENCODE(1,4,4)

#define	GSC_DMA_MODE_SIZE_8_BITS			GSC_FIELD_ENCODE(0, 1, 0)
#define	GSC_DMA_MODE_SIZE_16_BITS			GSC_FIELD_ENCODE(1, 1, 0)
#define	GSC_DMA_MODE_SIZE_32_BITS			GSC_FIELD_ENCODE(2, 1, 0)
#define	GSC_DMA_MODE_INPUT_ENABLE			GSC_FIELD_ENCODE(1, 6, 6)
#define	GSC_DMA_MODE_BURSTING_LOCAL			GSC_FIELD_ENCODE(1, 8, 8)
#define	GSC_DMA_MODE_INTERRUPT_WHEN_DONE	GSC_FIELD_ENCODE(1,10,10)
#define	GSC_DMA_MODE_LOCAL_ADRESS_CONSTANT	GSC_FIELD_ENCODE(1,11,11)
#define	GSC_DMA_MODE_BLOCK_DMA				GSC_FIELD_ENCODE(0,12,12)	// Non-Demand Mode
#define	GSC_DMA_MODE_DM_DMA					GSC_FIELD_ENCODE(1,12,12)	// Demand Mode
#define	GSC_DMA_MODE_PCI_INTERRUPT_ENABLE	GSC_FIELD_ENCODE(1,17,17)

#define	GSC_DMA_DPR_END_OF_CHAIN			GSC_FIELD_ENCODE(1,1,1)
#define	GSC_DMA_DPR_TERMINAL_COUNT_IRQ		GSC_FIELD_ENCODE(1,2,2)
#define	GSC_DMA_DPR_HOST_TO_BOARD			GSC_FIELD_ENCODE(0,3,3)		// Tx operation
#define	GSC_DMA_DPR_BOARD_TO_HOST			GSC_FIELD_ENCODE(1,3,3)		// Rx operation

// PLX Interrupt Control and Status Register
#define	GSC_INTCSR_LOCAL_LSERR_ENABLE_1		GSC_FIELD_ENCODE(1, 0, 0)
#define	GSC_INTCSR_LOCAL_LSERR_ENABLE_2		GSC_FIELD_ENCODE(1, 1, 1)
#define	GSC_INTCSR_GEN_PCI_SERR				GSC_FIELD_ENCODE(1, 2, 2)
#define	GSC_INTCSR_MAILBOX_INT_ENABLE		GSC_FIELD_ENCODE(1, 3, 3)
#define	GSC_INTCSR_POWER_MAN_INT_ENABLE		GSC_FIELD_ENCODE(1, 4, 4)
#define	GSC_INTCSR_POWER_MAN_INT_ACTIVE		GSC_FIELD_ENCODE(1, 5, 5)
#define	GSC_INTCSR_LOCAL_PE_INT_ENABLE		GSC_FIELD_ENCODE(1, 6, 6)	// Local Parity Error
#define	GSC_INTCSR_LOCAL_PE_INT_ACTIVE		GSC_FIELD_ENCODE(1, 7, 7)	// Local Parity Error
#define	GSC_INTCSR_PCI_INT_ENABLE			GSC_FIELD_ENCODE(1, 8, 8)
#define	GSC_INTCSR_PCI_DOOR_INT_ENABLE		GSC_FIELD_ENCODE(1, 9, 9)
#define	GSC_INTCSR_ABORT_INT_ENABLE			GSC_FIELD_ENCODE(1,10,10)
#define	GSC_INTCSR_LOCAL_INT_ENABLE			GSC_FIELD_ENCODE(1,11,11)
#define	GSC_INTCSR_RETRY_ABORT_ENABLE		GSC_FIELD_ENCODE(1,12,12)
#define	GSC_INTCSR_PCI_DOOR_INT_ACTIVE		GSC_FIELD_ENCODE(1,13,13)
#define	GSC_INTCSR_ABORT_INT_ACTIVE			GSC_FIELD_ENCODE(1,14,14)
#define	GSC_INTCSR_LOCAL_INT_ACTIVE			GSC_FIELD_ENCODE(1,15,15)
#define	GSC_INTCSR_LOCAL_OUT_ENABLE			GSC_FIELD_ENCODE(1,16,16)
#define	GSC_INTCSR_LOC_DOOR_INT_ENABLE		GSC_FIELD_ENCODE(1,17,17)
#define	GSC_INTCSR_DMA_0_INT_ENABLE			GSC_FIELD_ENCODE(1,18,18)
#define	GSC_INTCSR_DMA_1_INT_ENABLE			GSC_FIELD_ENCODE(1,19,19)
#define	GSC_INTCSR_LOC_DOOR_INT_ACTIVE		GSC_FIELD_ENCODE(1,20,20)
#define	GSC_INTCSR_DMA_0_INT_ACTIVE			GSC_FIELD_ENCODE(1,21,21)
#define	GSC_INTCSR_DMA_1_INT_ACTIVE			GSC_FIELD_ENCODE(1,22,22)
#define	GSC_INTCSR_BIST_INT_ACTIVE			GSC_FIELD_ENCODE(1,23,23)
#define	GSC_INTCSR_MAILBOX_INT_ACTIVE		GSC_FIELD_ENCODE(0xF,31,28)



// data types *****************************************************************

typedef struct _gsc_dma_ch_t
{
	int						index;
	int						in_use;
	unsigned int			flags;
	u32						intcsr_enable;
	int						error;		// There was a DMA related error.

	struct
	{
		VADDR_T				mode_32;	// DMAMODEx
		VADDR_T				padr_32;	// DMAPADRx
		VADDR_T				ladr_32;	// DMALADRx
		VADDR_T				siz_32;		// DMASIZx
		VADDR_T				dpr_32;		// DMADPRx
		VADDR_T				csr_8;		// DMACSRx
	} vaddr;

} gsc_dma_ch_t;

typedef struct _gsc_dma_setup_t
{
	GSC_ALT_STRUCT_T*		alt;
	dev_data_t*				dev;
	struct _dev_io_t*		io;
	u32						mode;
	u32						dpr;
	const os_mem_t*			mem;
	os_time_tick_t			st_end;	// Timeout at this point in time (in system ticks).
	long					bytes;
	u32						ability;
	gsc_dma_ch_t*			dma;
	int						error;	// < 0 if there is an error
} gsc_dma_setup_t;



// variables ******************************************************************



// prototypes *****************************************************************



#endif
