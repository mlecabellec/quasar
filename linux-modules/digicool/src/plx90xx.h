#ifndef _PLX90XX_H
#define _PLX90XX_H

#define PLX_ID					0x908010b5
#define VENDOR_ID               (PLX_ID & 0xFFFF)
#define DEVICE_ID               ((PLX_ID >> 16) & 0xFFFF)
#define PLX_CFG_SIZE			0x100
#define PLX_BAR_COUNT			6

/* Registres de configuration PCI*/
typedef struct PLXCFGREGS
{
	unsigned int	PCIIDR;		/* @00h PCI Configuration ID Register */
	unsigned int	PCICR : 16;	/* @04h PCI Command Register */
	unsigned int	PCISR : 16;	/* @06h PCI Status Register */
	unsigned int	PCIREV : 8;	/* @08h PCI Revision ID Register */
	unsigned int	PCICCR : 24;	/* @09h PCI Class Code Register */
	unsigned int	PCICLSR : 8;	/* @0Ch PCI Cache Line Size Register */
	unsigned int	PCILTR : 8;	/* @0Dh PCI Latency Timer */
	unsigned int	PCIHTR : 8;	/* @0Eh PCI Header Type Register */
	unsigned int	PCIBISTR : 8;	/* @0Fh PCI Built-In Self Test */
	unsigned int	PCIBAR0;	/* @10h PCI Base Address Register for memory Accesses to Local, Runtime and DMA registers */
	unsigned int	PCIBAR1;	/* @14h PCI Base Address Register for IO Accesses to Local, Runtime and DMA Registers */
	unsigned int	PCIBAR2;	/* @18h PCI Base Address Register for Memory Accesses to Local Address Space 0 */
	unsigned int	PCIBAR3;	/* @1Ch PCI Base Address Register for Memory Accesses to Local Address Space 1 */
	unsigned int	PCIBAR4;	/* @20h PCI Base Address Register */
	unsigned int	PCIBAR5;	/* @24h PCI Base Address Register */
	unsigned int	PCICIS;		/* @28h PCI Cardbus CIS Pointer */
	unsigned int	PCISID : 16;	/* @2Ch PCI Subsystem ID */
	unsigned int	PCISVID : 16;	/* @2Eh PCI Subsystem Vendor ID */
	unsigned int	PCIERBAR;	/* @30h PCI Expansion ROM Base Register */
	unsigned int	reserved[2];
	unsigned int	PCIILR : 8;	/* @3Ch PCI Interrupt Line Register */
	unsigned int	PCIIPR : 8;	/* @3Dh PCI Interrupt Pin Register */
	unsigned int	PCIMGR : 8;	/* @3Eh PCI Min_GntRegister */
	unsigned int	PCIMLR : 8;	/* @3Fh PCI Max_LatRegister */
}PLXCFGREGS;

#define	PCI_CFG_VENDOR_ID				0x00
#define	PCI_CFG_DEVICE_ID				0x02
#define	PCI_CFG_COMMAND					0x04
#define	PCI_CFG_STATUS					0x06
#define	PCI_CFG_REVISION				0x08
#define	PCI_CFG_PROGRAMMING_IF			0x09
#define	PCI_CFG_SUBCLASS				0x0a
#define	PCI_CFG_CLASS					0x0b
#define	PCI_CFG_CACHE_LINE_SIZE			0x0c
#define	PCI_CFG_LATENCY_TIMER			0x0d
#define	PCI_CFG_HEADER_TYPE				0x0e
#define	PCI_CFG_BIST					0x0f
#define	PCI_CFG_BASE_ADDRESS_0			0x10
#define	PCI_CFG_BASE_ADDRESS_1			0x14
#define	PCI_CFG_BASE_ADDRESS_2			0x18
#define	PCI_CFG_BASE_ADDRESS_3			0x1c
#define	PCI_CFG_BASE_ADDRESS_4			0x20
#define	PCI_CFG_BASE_ADDRESS_5			0x24
#define	PCI_CFG_CIS						0x28
#define	PCI_CFG_SUB_VENDOR_ID			0x2c
#define	PCI_CFG_SUB_SYSTEM_ID			0x2e
#define	PCI_CFG_EXPANSION_ROM			0x30
#define	PCI_CFG_RESERVED_0				0x34
#define	PCI_CFG_RESERVED_1				0x38
#define	PCI_CFG_INTERRUPT_LINE			0x3c
#define	PCI_CFG_INTERRUPT_PIN			0x3d
#define	PCI_CFG_MIN_GRANT				0x3e
#define	PCI_CFG_MAX_LATENCY				0x3f

/* Registres de configuration du bus local */
typedef struct PLXLCFGREGS
{
	unsigned int	LAS0RR;		/* @00h Local Address Space 0 Range Register for PCI to Local Bus */
	unsigned int	LAS0BA;		/* @04h Local Address Space 0 Local Base Address (Remap) Register */
	unsigned int	LARBR;		/* @08h Local / DMA Arbitration Register */
	unsigned int	BIGEND;		/* @0Ch Big/Little Endian Descriptor Register */
	unsigned int	EROMRR;		/* @10h Expansion ROM Range Register */
	unsigned int	EROMBA;		/* @14h Expansion ROM Local Base Address (Rempa) Register */
	unsigned int	LBRD0;		/* @18h Local Address Space 0 / Expansion ROM Bus Region Descriptor Register */
	unsigned int	DMRR;		/* @1Ch Local Range Register for Direct Master to PCI */
	unsigned int	DMLBAM;		/* @20h Local Bus Base Address Register for Direct Master to Memory */
	unsigned int	DMLBAI;		/* @24h Local Base Address Register for Direct Master to PCI IO/CFG */
	unsigned int	DMPBAM;		/* @28h PCI Base Address (Remap) Register for Direct Master to PCI Memory */
	unsigned int	DMCFGA;		/* @2Ch PCI Configuration Address Register for Direct Master to PCI IO/CFG */
	unsigned int	OPLFIS;		/* @30h Outbound Post List FIFO Interrupt Status Register */
	unsigned int	OPLFIM;		/* @34h Outbound Post List FIFO Interrupt Mask Register */
	unsigned int	rsrvd0[2];
	unsigned int	IQP;		/* @40h Inbound Queue Port */
	unsigned int	OQP;		/* @44h Outbound Queue Port */
	unsigned int	MBOX2;		/* @48h MailBox Register 2 */
	unsigned int	MBOX3;		/* @4Ch MailBox Register 3 */
	unsigned int	MBOX4;		/* @50h MailBox Register 4 */
	unsigned int	MBOX5;		/* @54h MailBox Register 5 */
	unsigned int	MBOX6;		/* @58h MailBox Register 6 */
	unsigned int	MBOX7;		/* @5Ch MailBox Register 7 */
	unsigned int	P2LDBELL;	/* @60h PCI to Local Doorbell Register */
	unsigned int	L2PDBELL;	/* @64h Local to PCI Doorbell Register */
	unsigned int	INTCSR;		/* @68h Interrupt Control / Status */
	unsigned int	CNTRL;		/* @6Ch EEPROM Control, PCI Command Codes, User I/O Control, Init Control Register */
	unsigned int	PCIHIDR;	/* @70h PCI Permanent Configuration ID Register */
	unsigned int	PCIHREV:8;	/* @74h PCI Permanent Revision ID Register */
	unsigned int	rsrvd1:24;
	unsigned int	MBOX0;		/* @78h MailBox Register 0 */
	unsigned int	MBOX1;		/* @7Ch MailBox Register 1 */
	unsigned int	DMAMODE0;	/* @80h DMA Channel 0 Mode Register */
	unsigned int	DMAPADR0;	/* @84h DMA Channel 0 PCI Address Register */
	unsigned int	DMALADR0;	/* @88h DMA Channel 0 Local Address Register */
	unsigned int	DMASIZ0;	/* @8Ch DMA Channel 0 Transfert Size (Bytes) Register */
	unsigned int	DMADPR0;	/* @90h DMA Channel 0 Descriptor Pointer Register */
	unsigned int	DMAMODE1;	/* @94h DMA Channel 1 Mode Register */
	unsigned int	DMAPADR1;	/* @98h DMA Channel 1 PCI Address Register */
	unsigned int	DMALADR1;	/* @9Ch DMA Channel 1 Local Address Register */
	unsigned int	DMASIZ1;	/* @A0h DMA Channel 1 Transfert Size (Bytes) Register */
	unsigned int	DMADPR1;	/* @A4h DMA Channel 1 Descriptor Pointer Register */
	unsigned int   DMACSR0:8;      /* @A8h DMA Channel 1 Command / Status Register */
	unsigned int   DMACSR1:8;      /* @A9h DMA Channel 0 Command / Status Register */
	unsigned int	rsrvd2:16;
	unsigned int	DMAARB;		/* @ACh DMA Arbitration Register */
	unsigned int	DMATHR;		/* @B0h DMA Threshold Register */
	unsigned int	rsrvd3[3];
	unsigned int	MQCR;		/* @C0h Messaging Queue Configuration Register */
	unsigned int	QBAR;		/* @C4h Queue Base Address Register */
	unsigned int	IFHPR;		/* @C8h Inbound Free Head Pointer Register */
	unsigned int	IFTPR;		/* @CCh Inbound Free Tail Pointer Register */
	unsigned int	IPHPR;		/* @D0h Inbound Post Head Pointer Register */
	unsigned int	IPTPR;		/* @D4h Inbound Post Tail Pointer Register */
	unsigned int	OFHPR;		/* @D8h Outbound Free Head Pointer Register */
	unsigned int	OFTPR;		/* @DCh Outbound Free Tail Pointer Register */
	unsigned int	OPHPR;		/* @E0h Outbound Post Head Pointer Register */
	unsigned int	OPTPR;		/* @E4h Outbound Post Tail Pointer Register */
	unsigned int	QSR;		/* @E8h Queue Status / Control Register */
	unsigned int	rsrvd4;
	unsigned int	LAS1RR;		/* @F0h Local Address Space 1 Range Register for PCI to Local Bus */
	unsigned int	LAS1BA;		/* @F4h Local Address Space 1 Local Base Address (Remap) Register */
	unsigned int	LBRD1;		/* @F8h Local Address Space 1 Bus Region Descriptor Register */
} PLXLCFGREGS;

typedef struct DMA_DESCRIPTOR_BLOCK
{
        unsigned int                   ulPciAddress;
        unsigned int                   ulLocalAddress;
        unsigned int                   ulBufferSize;
        unsigned int                   ulNextDescriptor;
} DMA_DESCRIPTOR_BLOCK;

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* Big/Little Endian Descriptor Register BIGEND  @PCI=0Ch            */
/*********************************************************************/
#define PLX_BIGEND_CONFIG_SPACE        0x00000001          /* Bit 00 */
#define PLX_BIGEND_SPACE_0             0x00000004          /* Bit 02 */
#define PLX_BIGEND_SPACE_1             0x00000020          /* Bit 05 */
#define PLX_BIGEND_DMA_CH1             0x00000040          /* Bit 06 */
#define PLX_BIGEND_DMA_CH0             0x00000080          /* Bit 07 */

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* Interrupt Control / Status INTCSR  @PCI=68h                       */
/*********************************************************************/
#define PLX_INTCSR_PCI_INT_ENABLE      0x00000100          /* Bit 08 */
#define PLX_INTCSR_PCI_DBELL_INT_EN    0x00000200          /* Bit 09 */
#define PLX_INTCSR_PCI_ABORT_INT_EN    0x00000400          /* Bit 10 */
#define PLX_INTCSR_PCI_LOCAL_INT_EN    0x00000800          /* Bit 11 */
#define PLX_INTCSR_PCI_DBELL_INT_STS   0x00002000          /* Bit 13 */
#define PLX_INTCSR_PCI_ABORT_INT_STS   0x00004000          /* Bit 14 */
#define PLX_INTCSR_PCI_LOCAL_INT_STS   0x00008000          /* Bit 15 */
#define PLX_INTCSR_PCI_DMA00_INT_EN    0x00040000          /* Bit 18 */
#define PLX_INTCSR_PCI_DMA01_INT_EN    0x00080000          /* Bit 19 */
#define PLX_INTCSR_PCI_DMA00_INT_STS   0x00200000          /* Bit 21 */
#define PLX_INTCSR_PCI_DMA01_INT_STS   0x00400000          /* Bit 22 */

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* EEPROM Control, PCI Command Codes, User I/O Control,              */
/* Init Control Register CNTRL @PCI=6Ch                              */
/*********************************************************************/
#define PLX_CNTRL_USER_OUTPUT          0x00010000          /* Bit 16 */
#define PLX_CNTRL_USER_INPUT           0x00020000          /* Bit 17 */
#define PLX_CNTRL_EEPROM_CLK           0x01000000          /* Bit 24 */
#define PLX_CNTRL_EEPROM_CS            0x02000000          /* Bit 25 */
#define PLX_CNTRL_EEPROM_WBIT          0x04000000          /* Bit 26 */
#define PLX_CNTRL_EEPROM_RBIT          0x08000000          /* Bit 27 */
#define PLX_CNTRL_EEPROM_PRESENT       0x10000000          /* Bit 28 */
#define PLX_CNTRL_RELOAD_CONFIG        0x20000000          /* Bit 29 */
#define PLX_CNTRL_RESET_LOCAL          0x40000000          /* Bit 30 */
#define PLX_CNTRL_EEPROM_EIBIT         0x80000000          /* Bit 31 */

#define PLX_PCIE_EECTL_BUSY		0x00080000
#define PLX_PCIE_EECTL_CS		0x00040000
#define PLX_PCIE_EECTL_START_RD		0x00020000
#define PLX_PCIE_EECTL_START_WR		0x00010000
#define PLX_PCIE_EECTL_WRDATA_MASK	0x000000FF
#define PLX_PCIE_EECTL_WRDATA_SHIFT	0
#define PLX_PCIE_EECTL_RDDATA_MASK	0x0000FF00
#define PLX_PCIE_EECTL_RDDATA_SHIFT	8
#define PLX_PCIE_EECTL_VALID		0x00100000
#define PLX_PCIE_EECTL_PRESENT		0x00200000
#define PLX_PCIE_EECTL_WIDTH_MASK	0x01800000
#define PLX_PCIE_EECTL_WIDTH_ERR	0x00000000
#define PLX_PCIE_EECTL_WIDTH_1B		0x00800000
#define PLX_PCIE_EECTL_WIDTH_2B		0x01000000
#define PLX_PCIE_EECTL_WIDTH_3B		0x01800000
#define PLX_PCIE_EECTL_RELOAD		0x80000000

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* DMA Channel 0/1 Mode Register DMAMODEX @PCI=80h / 94h             */
/*********************************************************************/
#define PLX_DMAMODEX_BASE              0x00021943
#define PLX_DMAMODEX_CHAINING          0x00000200          /* Bit 09 */
#define PLX_DMAMODEX_DONE_INT_EN       0x00000400          /* Bit 10 */

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* DMA Channel 0/1 Descriptor Pointer Register DMADPRX @PCI=90h / A4h*/
/*********************************************************************/
#define PLX_DMADPRX_PCI_LOC            0x00000001          /* Bit 00 */
#define PLX_DMADPRX_END_CHAIN          0x00000002          /* Bit 01 */
#define PLX_DMADPRX_TC_INT             0x00000004          /* Bit 02 */
#define PLX_DMADPRX_READ_WRITE         0x00000008          /* Bit 03 */

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* DMA Channel 0/1 Command/Status Register DMACSR0/1 @PCI=A8h / A9h  */
/*********************************************************************/
#define PLX_DMACSRX_ENABLE             0x00000001          /* Bit 00 */
#define PLX_DMACSRX_START              0x00000002          /* Bit 01 */
#define PLX_DMACSRX_ABORT              0x00000004          /* Bit 02 */
#define PLX_DMACSRX_INT_ACK            0x00000008          /* Bit 03 */
#define PLX_DMACSRX_STATUS             0x00000010          /* Bit 04 */

/*********************************************************************/
/* Definition des bits importants concernant le registre :           */
/* General purpose I/O control @PCIe=1020h                           */
/*********************************************************************/
#define PLX_PCIE_MCR_GPIO0             0x00000001          /* Bit 00 */
#define PLX_PCIE_MCR_GPIO1             0x00000002          /* Bit 01 */
#define PLX_PCIE_MCR_GPIO2             0x00000004          /* Bit 02 */
#define PLX_PCIE_MCR_GPIO3             0x00000008          /* Bit 03 */
#define PLX_PCIE_MCR_GPIO0_OE          0x00000010          /* Bit 04 */
#define PLX_PCIE_MCR_GPIO1_OE          0x00000020          /* Bit 05 */
#define PLX_PCIE_MCR_GPIO2_OE          0x00000040          /* Bit 06 */
#define PLX_PCIE_MCR_GPIO3_OE          0x00000080          /* Bit 07 */
#define PLX_PCIE_MCR_GPIO_DIAG_NORMAL  0x00000000          /* Bit 12/13 */

#endif  /* _PLX90XX_H */
