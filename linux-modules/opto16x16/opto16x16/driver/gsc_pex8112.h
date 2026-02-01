// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/pci_plx/gsc_pex8112.h $
// $Rev: 50964 $
// $Date: 2022-04-25 08:31:03 -0500 (Mon, 25 Apr 2022) $

// OS & Device Independent: Device Driver: PLX: header file

#ifndef	__GSC_PEX8112_H__
#define	__GSC_PEX8112_H__

#include "gsc_common.h"



// macros *********************************************************************

#define	GSC_PCI_8112_ENCODE(s,o)	GSC_REG_ENCODE(GSC_REG_TYPE_PCI,(s),(o))
#define	GSC_PLX_8112_ENCODE(s,o)	GSC_REG_ENCODE(GSC_REG_TYPE_ALT,(s),(o))

// PLX PEX8112 PCI Configuration Registers
#define	GSC_PCI_8112_VIDR			GSC_PCI_8112_ENCODE(2, 0x000)	// Vendor ID Register
#define	GSC_PCI_8112_DIDR			GSC_PCI_8112_ENCODE(2, 0x002)	// Device ID Register
#define	GSC_PCI_8112_CR				GSC_PCI_8112_ENCODE(2, 0x004)	// Command Register
#define	GSC_PCI_8112_SR				GSC_PCI_8112_ENCODE(2, 0x006)	// Status Register
#define	GSC_PCI_8112_REV			GSC_PCI_8112_ENCODE(1, 0x008)	// Device Revision ID Register
#define	GSC_PCI_8112_CCR			GSC_PCI_8112_ENCODE(3, 0x009)	// Class Code Register
#define	GSC_PCI_8112_CLSR			GSC_PCI_8112_ENCODE(1, 0x00C)	// Cache Line Size Register
#define	GSC_PCI_8112_BLTR			GSC_PCI_8112_ENCODE(1, 0x00D)	// Bus Latency Timer Register
#define	GSC_PCI_8112_HTR			GSC_PCI_8112_ENCODE(1, 0x00E)	// Header Type Register
#define	GSC_PCI_8112_BISTR			GSC_PCI_8112_ENCODE(1, 0x00F)	// Built-In Self-Test Register
#define	GSC_PCI_8112_BAR0			GSC_PCI_8112_ENCODE(4, 0x010)	// Base Address 0 Register
#define	GSC_PCI_8112_BAR1			GSC_PCI_8112_ENCODE(4, 0x014)	// Base Address 1 Register
#define	GSC_PCI_8112_PRIBNR			GSC_PCI_8112_ENCODE(1, 0x018)	// Primary Bus Number Register
#define	GSC_PCI_8112_SECBNR			GSC_PCI_8112_ENCODE(1, 0x019)	// Secondary Bus Number Register
#define	GSC_PCI_8112_SUBBNR			GSC_PCI_8112_ENCODE(1, 0x01A)	// Subordinate Bus Number Register
#define	GSC_PCI_8112_SECLTR			GSC_PCI_8112_ENCODE(1, 0x01B)	// Secondary Latency Timer Register
#define	GSC_PCI_8112_IOBR			GSC_PCI_8112_ENCODE(1, 0x01C)	// I/O Base Register
#define	GSC_PCI_8112_IOLR			GSC_PCI_8112_ENCODE(1, 0x01D)	// I/O Limit Register
#define	GSC_PCI_8112_SECSR			GSC_PCI_8112_ENCODE(2, 0x01E)	// Secondary Status Register
#define	GSC_PCI_8112_MBR			GSC_PCI_8112_ENCODE(2, 0x020)	// Memory Base Register
#define	GSC_PCI_8112_MLR			GSC_PCI_8112_ENCODE(2, 0x022)	// Memory Limit Register
#define	GSC_PCI_8112_PMBR			GSC_PCI_8112_ENCODE(2, 0x024)	// Prefetchable Memory Base Register
#define	GSC_PCI_8112_PMLR			GSC_PCI_8112_ENCODE(2, 0x026)	// Prefetchable Memory Limit Register
#define	GSC_PCI_8112_PMBUR			GSC_PCI_8112_ENCODE(4, 0x028)	// Prefetchable Memory Base Upper 32-bits Register
#define	GSC_PCI_8112_PMLUR			GSC_PCI_8112_ENCODE(4, 0x02C)	// Prefetchable Memory Limit Upper 32-bits Register
#define	GSC_PCI_8112_IOBUR			GSC_PCI_8112_ENCODE(2, 0x030)	// I/O Base Upper 16-bits Register
#define	GSC_PCI_8112_IOLUR			GSC_PCI_8112_ENCODE(2, 0x032)	// I/O Limit Upper 16-bits Register
#define	GSC_PCI_8112_CPR			GSC_PCI_8112_ENCODE(1, 0x034)	// Capabilities Pointer Register
#define	GSC_PCI_8112_BAERR			GSC_PCI_8112_ENCODE(4, 0x038)	// Base Address for Expansion ROM Register
#define	GSC_PCI_8112_ILR			GSC_PCI_8112_ENCODE(1, 0x03C)	// Interrupt Line Register
#define	GSC_PCI_8112_IPR			GSC_PCI_8112_ENCODE(1, 0x03D)	// Interrupt Pin Register
#define	GSC_PCI_8112_BCR			GSC_PCI_8112_ENCODE(2, 0x03E)	// Bridge Control Register

#define	GSC_PCI_8112_PMCIDR			GSC_PCI_8112_ENCODE(1, 0x040)	// Power Management Capability ID Register
#define	GSC_PCI_8112_PMNCPR			GSC_PCI_8112_ENCODE(1, 0x041)	// Power Management Next Capability Pointer Register
#define	GSC_PCI_8112_PMCR			GSC_PCI_8112_ENCODE(2, 0x042)	// Power Management Capability Register
#define	GSC_PCI_8112_PMCSR			GSC_PCI_8112_ENCODE(2, 0x044)	// Power Management Control/Status Register
#define	GSC_PCI_8112_PMBSR			GSC_PCI_8112_ENCODE(1, 0x046)	// Power Management Bridge Support Register
#define	GSC_PCI_8112_PMDR			GSC_PCI_8112_ENCODE(1, 0x047)	// Power Management Data Register
#define	GSC_PCI_8112_DSCR			GSC_PCI_8112_ENCODE(4, 0x048)	// Device-Specific Control Register
#define	GSC_PCI_8112_MCIDR			GSC_PCI_8112_ENCODE(1, 0x050)	// MSI Capability ID Register
#define	GSC_PCI_8112_MNCPR			GSC_PCI_8112_ENCODE(1, 0x051)	// MSI Next Capability Pointer Register
#define	GSC_PCI_8112_MCR			GSC_PCI_8112_ENCODE(2, 0x052)	// MSI Control Register
#define	GSC_PCI_8112_MAR			GSC_PCI_8112_ENCODE(4, 0x054)	// MSI Address Register
#define	GSC_PCI_8112_MUAR			GSC_PCI_8112_ENCODE(4, 0x058)	// MSI Upper Address Register
#define	GSC_PCI_8112_MDR			GSC_PCI_8112_ENCODE(2, 0x05C)	// MSI Data Register
#define	GSC_PCI_8112_PECIDR			GSC_PCI_8112_ENCODE(1, 0x060)	// PCI Express ID Capability Register
#define	GSC_PCI_8112_PENCPR			GSC_PCI_8112_ENCODE(1, 0x061)	// PCI Express Next Capability Pointer Register
#define	GSC_PCI_8112_PECR			GSC_PCI_8112_ENCODE(2, 0x062)	// PCI Express Capability Register
#define	GSC_PCI_8112_DCR			GSC_PCI_8112_ENCODE(4, 0x064)	// Device Capabilities Register
#define	GSC_PCI_8112_PEDCR			GSC_PCI_8112_ENCODE(2, 0x068)	// PCI Express Device Control Register
#define	GSC_PCI_8112_PEDSR			GSC_PCI_8112_ENCODE(2, 0x06A)	// PCI Express Device Status Register
#define	GSC_PCI_8112_LCAPR			GSC_PCI_8112_ENCODE(4, 0x06C)	// Link Capabilities Register
#define	GSC_PCI_8112_LCTLR			GSC_PCI_8112_ENCODE(1, 0x070)	// Link Control Register
#define	GSC_PCI_8112_LSTSR			GSC_PCI_8112_ENCODE(2, 0x072)	// Link Status Register
#define	GSC_PCI_8112_SCAPR			GSC_PCI_8112_ENCODE(4, 0x074)	// Slot Capabilities Register
#define	GSC_PCI_8112_SCTLR			GSC_PCI_8112_ENCODE(2, 0x078)	// Slot Control Register
#define	GSC_PCI_8112_SSTSR			GSC_PCI_8112_ENCODE(2, 0x07A)	// Slot Status Register
#define	GSC_PCI_8112_MCRIR			GSC_PCI_8112_ENCODE(4, 0x084)	// Main Control Register Index Register
#define	GSC_PCI_8112_MCRDR			GSC_PCI_8112_ENCODE(4, 0x088)	// Main Control Register Data Register

#define	GSC_PCI_8112_PBECHR			GSC_PCI_8112_ENCODE(4, 0x100)	// Power Budget Enhanced Capability Header Register
#define	GSC_PCI_8112_PBDSR			GSC_PCI_8112_ENCODE(1, 0x104)	// Power Budget Data Select Register
#define	GSC_PCI_8112_PBDR			GSC_PCI_8112_ENCODE(4, 0x108)	// Power Budget Data Register
#define	GSC_PCI_8112_PBCR			GSC_PCI_8112_ENCODE(4, 0x10C)	// Power Budget Capability Register
#define	GSC_PCI_8112_SNPEECIDR		GSC_PCI_8112_ENCODE(4, 0x110)	// Serial Number Enhanced Capability Header Register
#define	GSC_PCI_8112_SNLR			GSC_PCI_8112_ENCODE(4, 0x114)	// Serial Number Low Register
#define	GSC_PCI_8112_SNHR			GSC_PCI_8112_ENCODE(4, 0x118)	// Serial Number Hi Register

// PLX PEX8112 Main Control Registers
#define	GSC_PLX_8112_DIR			GSC_PLX_8112_ENCODE(4, 0x000)	// Device Initialization Register
#define	GSC_PLX_8112_SECR			GSC_PLX_8112_ENCODE(4, 0x004)	// Serial EEPROM Control Register
#define	GSC_PLX_8112_SECFR			GSC_PLX_8112_ENCODE(4, 0x008)	// Serial EEPROM Clock Frequency Register
#define	GSC_PLX_8112_PCR			GSC_PLX_8112_ENCODE(4, 0x00C)	// PCI Control Register
#define	GSC_PLX_8112_PEIRER			GSC_PLX_8112_ENCODE(4, 0x010)	// PCI Express Interrupt Request Enable Register
#define	GSC_PLX_8112_IRER			GSC_PLX_8112_ENCODE(4, 0x014)	// Interrupt Request Enable Register (8311 only?)
#define	GSC_PLX_8112_IRSR			GSC_PLX_8112_ENCODE(4, 0x018)	// Interrupt Request Status Register
#define	GSC_PLX_8112_PR				GSC_PLX_8112_ENCODE(4, 0x01C)	// Power Register
#define	GSC_PLX_8112_GPIOCR			GSC_PLX_8112_ENCODE(4, 0x020)	// General Purpose I/O Control Register
#define	GSC_PLX_8112_GPIOSR			GSC_PLX_8112_ENCODE(4, 0x024)	// General Purpose I/O Status Register
#define	GSC_PLX_8112_M0R			GSC_PLX_8112_ENCODE(4, 0x030)	// Mailbox 0 Register
#define	GSC_PLX_8112_M1R			GSC_PLX_8112_ENCODE(4, 0x034)	// Mailbox 1 Register
#define	GSC_PLX_8112_M2R			GSC_PLX_8112_ENCODE(4, 0x038)	// Mailbox 2 Register
#define	GSC_PLX_8112_M3R			GSC_PLX_8112_ENCODE(4, 0x03C)	// Mailbox 3 Register
#define	GSC_PLX_8112_CSRR			GSC_PLX_8112_ENCODE(4, 0x040)	// Chip Silicon Revision Register
#define	GSC_PLX_8112_DCR			GSC_PLX_8112_ENCODE(4, 0x044)	// Diagnostics Control Register
#define	GSC_PLX_8112_TCC0R			GSC_PLX_8112_ENCODE(4, 0x048)	// TLP Controller Configuration 0 Register
#define	GSC_PLX_8112_TCC1R			GSC_PLX_8112_ENCODE(4, 0x04C)	// TLP Controller Configuration 1 Register
#define	GSC_PLX_8112_TCC2R			GSC_PLX_8112_ENCODE(4, 0x050)	// TLP Controller Configuration 2 Register
#define	GSC_PLX_8112_TCTR			GSC_PLX_8112_ENCODE(4, 0x054)	// TLP Controller Tag Register
#define	GSC_PLX_8112_TCTL0R			GSC_PLX_8112_ENCODE(4, 0x058)	// TLP Controller Time Limit 0 Regiter
#define	GSC_PLX_8112_TCTL1R			GSC_PLX_8112_ENCODE(4, 0x05C)	// TLP Controller Time Limit 1 Regiter
#define	GSC_PLX_8112_CTR			GSC_PLX_8112_ENCODE(4, 0x060)	// CSR Timer Register
#define	GSC_PLX_8112_ECAR			GSC_PLX_8112_ENCODE(4, 0x064)	// Enhanced Configuration Address Register



#endif
