#ifndef _PCIGDRV_H
#define _PCIGDRV_H

#include <asm/ioctl.h>

typedef struct RESOURCES_INFO
{
    unsigned long				PlxMemBase;				// Adresse physique des registres PLX vu du CPU
    unsigned int				ulPlxMemSize;
    unsigned long				ulPlxMapMemBase;		// Adresse virtuelle noyau des registres PLX
    unsigned long				CfigMemBase;			// Adresse physique de l'espace de configuration / mémoire vu du CPU
    unsigned int				ulCfigMemSize;
    unsigned long				ulCfigMapMemBase;		// Adresse virtuelle noyau de l'espace de configuration / mémoire
    unsigned int				ulCfigLocMemBase;		// Adresse physique d'accès à l'espace de configuration / mémoire sur le bus local
    unsigned long				UseMemBase;				// Adresse physique de l'espace d'accès aux registres FPGA vu du CPU
    unsigned int				ulUseMemSize;			
    unsigned long				ulUseMapMemBase;		// Adresse virtuelle noyau de l'espace d'accès aux registres FPGA
    unsigned int				ulUseLocMemBase;		// Adresse physique d'accès aux registres FPGA sur le bus local
    unsigned int				ulIrq;
} RESOURCES_INFO;

typedef struct CONFIG_FPGA
{
    unsigned int				ulConfigLen;
    unsigned char *				lpszConfig;
} CONFIG_FPGA;

typedef struct DMA_BUFFER_DESCRIPTOR
{
    unsigned int				ulSize;
    unsigned int				ulFlags;
//    unsigned long				ulLinAddr;
    unsigned int				bOwned;
} DMA_BUFFER_DESCRIPTOR;

typedef struct DMA_CHANNEL
{
    unsigned int				ulChannelNo;
    unsigned int				ulLocalAddress;
    DMA_BUFFER_DESCRIPTOR *		lpBufferDescriptor;
    unsigned int				ulNbBuffers;
    unsigned int				ulFlags;
} DMA_CHANNEL;

typedef struct REG_DES
{
    unsigned int				ulRegName;
    unsigned int				ulRegValue;
} REG_DES;

typedef struct REGS_DES
{
	unsigned int				ulStartRegName;
	unsigned int				ulAddressInc;
	unsigned int				ulNbRegs;
	unsigned int *				lpulRegsValue;
} REGS_DES;

typedef struct REGS_DES_WC
{
	unsigned int				ulInStartRegName;
	unsigned int				ulInStartRegQual;
	unsigned int				ulInAddressInc;
	unsigned int				ulOutStartRegName;
	unsigned int				ulOutStartRegQual;
	unsigned int				ulOutAddressInc;
	unsigned int				ulNbRegs;
	unsigned int *				lpulInRegsValue;
	unsigned int *				lpulOutRegsValue;
} REGS_DES_WC;

typedef struct REG_DES_EX
{
	unsigned int				ulRegQual;
    unsigned int				ulRegName;
    unsigned int				ulRegValue;
} REG_DES_EX;

typedef struct REGS_DES_EX
{
	unsigned int				ulRegQual;
    unsigned int				ulRegName;
	unsigned int				ulAddressInc;
	unsigned int				ulRegCount;
    unsigned int *				lpulRegValue;
} REGS_DES_EX;

typedef struct VERSION_STRING
{
	char		lpszVersionInfo[50];
} VERSION_STRING;

typedef struct WAIT_DESC
{
    unsigned int				ulUserIntIndex;
    unsigned int				ulTimeOut;
	unsigned int				ulLastIntCount;
	unsigned int				ulReturnValue;
}WAIT_DESC;
#define		PCIG_TIMEOUT		0
#define		PCIG_INTERRUPT		1

typedef struct INT_DESC
{
    unsigned int				ulStatusAddress;
    unsigned int				ulStatusBit;
    unsigned int				ulAckAddress;
    unsigned int				ulAckBit;
    unsigned int				ulDisableAddress;
    unsigned int				ulDisableBit;
    unsigned int				ulFlags;
}INT_DESC;

typedef struct _INTERRUPTS
{
	unsigned int				ulInterruptCount;
	INT_DESC*					lpInterruptDesc;
} INTERRUPTS;

#define PCIGIOC_MAGIC	0x4c

/*****************************/
/* Io control function codes */
/*****************************/
#define PCIGIOC_GET_ALLOCATED_RESOURCES		_IOR (PCIGIOC_MAGIC, 1,  RESOURCES_INFO)
#define PCIGIOC_GET_PCI_CFG_REGS			_IOR (PCIGIOC_MAGIC, 2,  REG_DES)
#define PCIGIOC_GET_LOCAL_CFG_REGS			_IOR (PCIGIOC_MAGIC, 3,  PLXLCFGREGS)
#define PCIGIOC_SET_LOCAL_CFG_REGS			_IOW (PCIGIOC_MAGIC, 4,  PLXLCFGREGS)
#define PCIGIOC_GET_LOCAL_CFG_REG			_IOR (PCIGIOC_MAGIC, 5,  REG_DES)
#define PCIGIOC_SET_LOCAL_CFG_REG			_IOWR(PCIGIOC_MAGIC, 6,  REG_DES)
#define PCIGIOC_INITIALIZE_DMA_CHANNEL		_IOWR(PCIGIOC_MAGIC, 7,  DMA_CHANNEL)
#define PCIGIOC_START_DMA_CHANNEL			_IOWR(PCIGIOC_MAGIC, 8,  unsigned int)
#define PCIGIOC_STOP_DMA_CHANNEL			_IOWR(PCIGIOC_MAGIC, 9,  unsigned int)
#define PCIGIOC_RESET_DMA_CHANNEL			_IOWR(PCIGIOC_MAGIC, 10, unsigned int)
#define PCIGIOC_FREE_DMA_CHANNEL			_IOWR(PCIGIOC_MAGIC, 11, unsigned int)
#define PCIGIOC_GET_XILINX_REGS				_IOWR(PCIGIOC_MAGIC, 12, REGS_DES)
#define PCIGIOC_SET_XILINX_REGS				_IOWR(PCIGIOC_MAGIC, 13, REGS_DES)
#define PCIGIOC_GET_XILINX_REG				_IOWR(PCIGIOC_MAGIC, 14, REG_DES)
#define PCIGIOC_SET_XILINX_REG				_IOWR(PCIGIOC_MAGIC, 15, REG_DES)
#define PCIGIOC_RESET_LOCAL_BUS				_IOWR(PCIGIOC_MAGIC, 16, int)
#define PCIGIOC_LOAD_LOGICAL_DESIGN			_IOW (PCIGIOC_MAGIC, 17, CONFIG_FPGA)
#define PCIGIOC_ENABLE_INTERRUPTS			_IOWR(PCIGIOC_MAGIC, 18, unsigned int)
#define PCIGIOC_READ_ONE_EEPROM				_IOWR(PCIGIOC_MAGIC, 19, REG_DES)
#define PCIGIOC_WRITE_ONE_EEPROM			_IOWR(PCIGIOC_MAGIC, 20, REG_DES)
#define PCIGIOC_REGISTER_USER_INT			_IOWR(PCIGIOC_MAGIC, 21, INTERRUPTS)
#define PCIGIOC_RELEASE_USER_INT			_IOWR(PCIGIOC_MAGIC, 22, int)
#define PCIGIOC_GET_NB_DEVICE				_IOWR(PCIGIOC_MAGIC, 23, unsigned int)
#define PCIGIOC_GET_VERSION					_IOWR(PCIGIOC_MAGIC, 24, VERSION_STRING)
#define PCIGIOC_ALLOCATE_DESIGN_BUFFER		_IOW (PCIGIOC_MAGIC, 25, unsigned int)
#define PCIGIOC_WAIT_FOR_DMA_0				_IOWR(PCIGIOC_MAGIC, 26, WAIT_DESC)
#define PCIGIOC_WAIT_FOR_DMA_1				_IOWR(PCIGIOC_MAGIC, 27, WAIT_DESC)
#define PCIGIOC_WAIT_FOR_USER_INT			_IOWR(PCIGIOC_MAGIC, 28, WAIT_DESC)
#define PCIGIOC_GET_DMA0_INT_COUNT			_IOR (PCIGIOC_MAGIC, 29, unsigned int)
#define PCIGIOC_GET_DMA1_INT_COUNT			_IOR (PCIGIOC_MAGIC, 30, unsigned int)
#define PCIGIOC_GET_USER_INT_COUNT			_IOWR(PCIGIOC_MAGIC, 31, unsigned int)
#define PCIGIOC_WAIT_FOR_DMA_0_EX			_IOWR(PCIGIOC_MAGIC, 32, WAIT_DESC)
#define PCIGIOC_WAIT_FOR_DMA_1_EX			_IOWR(PCIGIOC_MAGIC, 33, WAIT_DESC)
#define PCIGIOC_WAIT_FOR_USER_INT_EX		_IOWR(PCIGIOC_MAGIC, 34, WAIT_DESC)
#define PCIGIOC_PROGRAM_LOGICAL_DESIGN		_IOWR(PCIGIOC_MAGIC, 35, CONFIG_FPGA)
#define PCIGIOC_READ_ONE_PCIE_EEPROM		_IOWR(PCIGIOC_MAGIC, 36, REG_DES)
#define PCIGIOC_WRITE_ONE_PCIE_EEPROM		_IOWR(PCIGIOC_MAGIC, 37, REG_DES)
#define PCIGIOC_WRITE_AND_CHECK_XILINX_REGS	_IOWR(PCIGIOC_MAGIC, 38, REGS_DES_WC)
#define PCIGIOC_GET_REG						_IOWR(PCIGIOC_MAGIC, 39, REG_DES_EX)
#define PCIGIOC_SET_REG						_IOWR(PCIGIOC_MAGIC, 40, REG_DES_EX)
#define PCIGIOC_GET_REGS					_IOWR(PCIGIOC_MAGIC, 41, REGS_DES_EX)
#define PCIGIOC_SET_REGS					_IOWR(PCIGIOC_MAGIC, 42, REGS_DES_EX)
#define PCIGIOC_GET_DEVICE_ID				_IOR (PCIGIOC_MAGIC, 43, unsigned int)

#define PCIGIOC_MAXNR	43

/* Error Code */
#define PCIG_SUCCESS					0x00000000
#define PCIG_ERROR						0x00000001
#define PCIG_INCORRECT_INIT				0x00000002
#define PCIG_INVALID_PARAMETERS			0x00000003
#define PCIG_DMA_NOT_INITIALIZED		0x00000004
#define PCIG_NOT_ENOUGH_MEMORY			0x00000005
#define PCIG_DMA_ALREADY_ALLOCATED		0x00000006
#define PCIG_EEPROM_NOT_PRESENT			0x00000007
#define PCIG_EEPROM_NOT_SUPPORTED		0x00000008

/****************************************************/
/* Flags de la fonction PCIG_INITIALIZE_DMA_CHANNEL */
/****************************************************/
#define PCIG_DMA_FLAG_READ              0x00000001
#define PCIG_DMA_FLAG_LOOP              0x00000002
#define PCIG_DMA_INT_TCD                0x00000004

/******************************************************************/
/* Definition des identificateurs de registres pour les fonctions */
/* PCIG_GET_LOCAL_CFG_REG et PCIG_SET_LOCAL_CFG_REG               */
/******************************************************************/
#define PCIG_LCR_LAS0RR                 0x0F000000
#define PCIG_LCR_LAS0BA                 0x0F000004
#define PCIG_LCR_LARBR                  0x0F000008
#define PCIG_LCR_BIGEND                 0x0F00000C
#define PCIG_LCR_EROMRR                 0x0F000010
#define PCIG_LCR_EROMBA                 0x0F000014
#define PCIG_LCR_LBRD0                  0x0F000018
#define PCIG_LCR_DMRR                   0x0F00001C
#define PCIG_LCR_DMLBAM                 0x0F000020
#define PCIG_LCR_DMLBAI                 0x0F000024
#define PCIG_LCR_DMPBAM                 0x0F000028
#define PCIG_LCR_DMCFGA                 0x0F00002C
#define PCIG_LCR_LAS1RR                 0x0F0000F0
#define PCIG_LCR_LAS1BA                 0x0F0000F4
#define PCIG_LCR_LBRD1                  0x0F0000F8

#define PCIG_RTR_MBOX0                  0x0F000040
#define PCIG_RTR_MBOX1                  0x0F000044
#define PCIG_RTR_MBOX2                  0x0F000048
#define PCIG_RTR_MBOX3                  0x0F00004C
#define PCIG_RTR_MBOX4                  0x0F000050
#define PCIG_RTR_MBOX5                  0x0F000054
#define PCIG_RTR_MBOX6                  0x0F000058
#define PCIG_RTR_MBOX7                  0x0F00005C
#define PCIG_RTR_P2LDBELL               0x0F000060
#define PCIG_RTR_L2PDBELL               0x0F000064
#define PCIG_RTR_INTCSR                 0x0F000068
#define PCIG_RTR_CNTRL                  0x0F00006C
#define PCIG_RTR_PCIHIDR                0x0F000070
#define PCIG_RTR_PCIHREV                0x01000074

#define PCIG_LDR_DMAMODE0               0x0F000080
#define PCIG_LDR_DMAPADR0               0x0F000084
#define PCIG_LDR_DMALADR0               0x0F000088
#define PCIG_LDR_DMASIZ0                0x0F00008C
#define PCIG_LDR_DMADPR0                0x0F000090
#define PCIG_LDR_DMAMODE1               0x0F000094
#define PCIG_LDR_DMAPADR1               0x0F000098
#define PCIG_LDR_DMALADR1               0x0F00009C
#define PCIG_LDR_DMASIZ1                0x0F0000A0
#define PCIG_LDR_DMADPR1                0x0F0000A4
#define PCIG_LDR_DMACSR0                0x010000A8
#define PCIG_LDR_DMACSR1                0x010000A9
#define PCIG_LDR_DMATHR                 0x0F0000B0

#define PCIG_MQR_OPLFIS                 0x0F000030
#define PCIG_MQR_OPLFIM                 0x0F000034
#define PCIG_MQR_IQP                    0x0F000040
#define PCIG_MQR_OQP                    0x0F000044
#define PCIG_MQR_MQCR                   0x0F0000C0
#define PCIG_MQR_QBAR                   0x0F0000C4
#define PCIG_MQR_IFHPR                  0x0F0000C8
#define PCIG_MQR_IFTPR                  0x0F0000CC
#define PCIG_MQR_IPHPR                  0x0F0000D0
#define PCIG_MQR_IPTPR                  0x0F0000D4
#define PCIG_MQR_OFHPR                  0x0F0000D8
#define PCIG_MQR_OFTPR                  0x0F0000DC
#define PCIG_MQR_OPHPR                  0x0F0000E0
#define PCIG_MQR_OPTPR                  0x0F0000E4
#define PCIG_MQR_QSR                    0x0F0000E8

#define PCIE_MCR_DEVINIT		0x0F031000
#define PCIE_MCR_EECTL			0x0F031004
#define PCIE_MCR_EECLKFREQ		0x0F031008
#define PCIE_MCR_PCICTL			0x0F03100C
#define PCIE_MCR_PCIEIRQENB		0x0F031010
#define PCIE_MCR_PCIIRQENB		0x0F031014
#define PCIE_MCR_IRQSTAT		0x0F031018
#define PCIE_MCR_POWER			0x0F03101C
#define PCIE_MCR_GPIOCTL		0x0F031020
#define PCIE_MCR_GPIOSTAT		0x0F031024
#define PCIE_MCR_MAILBOX0		0x0F031030
#define PCIE_MCR_MAILBOX1		0x0F031034
#define PCIE_MCR_MAILBOX2		0x0F031038
#define PCIE_MCR_MAILBOX3		0x0F03103C
#define PCIE_MCR_CHIPREV		0x0F031040
#define PCIE_MCR_DIAG			0x0F031044
#define PCIE_MCR_TLPCFG0		0x0F031048
#define PCIE_MCR_TLPCFG1		0x0F03104C
#define PCIE_MCR_TLPCFG2		0x0F031050
#define PCIE_MCR_TLPTAG			0x0F031054
#define PCIE_MCR_TLPTIMELIMIT0		0x0F031058
#define PCIE_MCR_TLPTIMELIMIT1		0x0F03105C
#define PCIE_MCR_CRSTIMER		0x0F031060
#define PCIE_MCR_ECFGADDR		0x0F031064

/******************************************************************/
/* Definition des identificateurs de registres pour les fonctions */
/* PCIG_GET_EEPROM_REG et PCIG_SET_EEPROM_REG				      */
/******************************************************************/
#define PCIG_EEPROM_PCIIDR		0x0F010000
#define PCIG_EEPROM_PCICCR		0x0F010004
#define PCIG_EEPROM_PCIIPR		0x0F010008
#define PCIG_EEPROM_MBOX0		0x0F01000C
#define PCIG_EEPROM_MBOX1		0x0F010010
#define PCIG_EEPROM_LAS0RR		0x0F010014
#define PCIG_EEPROM_LAS0BA		0x0F010018
#define PCIG_EEPROM_LARBR		0x0F01001C
#define PCIG_EEPROM_BIGEND		0x0F010020
#define PCIG_EEPROM_EROMRR		0x0F010024
#define PCIG_EEPROM_EROMBA		0x0F010028
#define PCIG_EEPROM_LBRD0		0x0F01002C
#define PCIG_EEPROM_DMRR		0x0F010030
#define PCIG_EEPROM_DMLBAM		0x0F010034
#define PCIG_EEPROM_DMLBAI		0x0F010038
#define PCIG_EEPROM_DMPBAM		0x0F01003C
#define PCIG_EEPROM_DMPBAI		0x0F010040
#define PCIG_EEPROM_PCIIDR2		0x0F010044
#define PCIG_EEPROM_LAS1RR		0x0F010048
#define PCIG_EEPROM_LAS1BA		0x0F01004C
#define PCIG_EEPROM_LBRD1		0x0F010050
#define PCIG_EEPROM_PCIERBAR		0x0F010054

#define PCIE_EEPROM_			0x0f040000
/******************************************************************/
/* Definition des validations des interruptions                   */
/* PCIG_ENABLE_INTERRUPTS                                         */
/******************************************************************/
#define PCIG_INT_LINTA_ENABLE           0x10000000
#define PCIG_INT_DBELL_ENABLE           0x00000001
#define PCIG_INT_ABORT_ENABLE           0x00000002
#define PCIG_INT_LOCAL_ENABLE           0x00000004
#define PCIG_INT_DMACH0_ENABLE          0x00000008
#define PCIG_INT_DMACH1_ENABLE          0x00000010

/******************************************************************/
/* Definition des flags pour l'enregistrement des interruptions   */
/* PCIG_REGISTER_USER_INT                                         */
/******************************************************************/
#define PCIG_FLAG_INT_ACK0              0x00000001
#define PCIG_FLAG_INT_ACK1              0x00000002
#define PCIG_FLAG_INT_DIS0              0x00000004
#define PCIG_FLAG_INT_DIS1              0x00000008
#define PCIG_FLAG_INT_STS0              0x00000010
#define PCIG_FLAG_INT_STS1              0x00000020
#define PCIG_FLAG_INT_UCACK0            0x00000040
#define PCIG_FLAG_INT_UCACK1            0x00000080
#define PCIG_FLAG_INT_CHKVAL0           0x00000100
#define PCIG_FLAG_INT_CHKVAL1           0x00000200
#define PCIG_FLAG_INT_STARTDMA0         0x00000400
#define PCIG_FLAG_INT_STARTDMA1         0x00000800

#endif /* _PCIGDRV_H */
