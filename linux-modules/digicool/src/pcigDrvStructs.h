#ifndef _PCIGDRVSTRUCTS_H
#define _PCIGDRVSTRUCTS_H

#include "pcigdrv.h"
#include "plx90xx.h"

#define MAX_PCIG					10			/* Maximum PCIG card count in a single PC */
#define PCIG_MAX_NAME_LENGTH		50
#define PCIG_NB_DMA_CHANNEL			2
#define PCIG_VERSION_INFO			"Version 3.0.7"

/* userful in 2.0.35 */
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c)((a)*65536+(b)*256+(c))
#endif

typedef struct INTERNAL_INT_DESC
{
        unsigned long				ulStatusAddress;
        unsigned long				ulStatusBit;
        unsigned long				ulAckAddress;
        unsigned long				ulAckBit;
        unsigned long				ulDisableAddress;
        unsigned long				ulDisableBit;
        unsigned long				ulFlags;
        unsigned long				ulChNoInitDone;
		unsigned long				ulInternalInterruptCount;
        struct INTERNAL_INT_DESC  *	lpNextIntDescriptor;
}INTERNAL_INT_DESC;

typedef struct INTERNAL_DMA_BUFFER
{
		unsigned long			ulBufferOffset;
		unsigned long			ulBufferSize;
} INTERNAL_DMA_BUFFER;

typedef struct INTERNAL_DMA_CHANNEL
{
        unsigned long           ulChannelNo;
        unsigned long           ulLocalAddress;
        DMA_DESCRIPTOR_BLOCK   *lpDescriptorBlock;
        unsigned long			ulNbDescriptorBlock;
		INTERNAL_DMA_BUFFER*	lpDmaBuffer;
        unsigned long			ulNbBuffers;
        unsigned long			ulNbBlocks;
        unsigned long			ulFlags;
        unsigned long			ulFirstDescriptorFlags;
        unsigned long			ulInternalDmaInterruptCount;
} INTERNAL_DMA_CHANNEL;

typedef struct INTERNAL_INTERRUPTS
{
        INTERNAL_INT_DESC     *lpInterruptDescriptor;
        unsigned long          ulNbIntDesc;
		unsigned long			ulNbIntDescAllocated;
} INTERNAL_INTERRUPTS;

typedef struct _DEVICE_EXTENSION
{
	int							iPresent;
	char						lpszName[PCIG_MAX_NAME_LENGTH];
//	struct						pci_dev *dev;
	unsigned char				ucBusNo;
	unsigned char				ucFunctionNo;
	unsigned long				ulMemoryCount;
	unsigned long				ulEepromType;
	unsigned char *				lpszConfig;
	unsigned long				ulConfigLenght;
	unsigned int				DeviceId;
	unsigned int				SubDeviceId;
	unsigned long				PcieBar;
	int							PcieIsIoSpace;
	unsigned int				PcieBarLen;
	int							bPcieIsIoSpace;
	int							bPcieResourceClaimed;
	unsigned int				Bar[PLX_BAR_COUNT];
	int							bIsIoSpace[PLX_BAR_COUNT];
	int							bResourceClaimed[PLX_BAR_COUNT];
	unsigned int				BarLen[PLX_BAR_COUNT];
	union
	{
		unsigned long				PcieBarVa;
		volatile unsigned int       *pPcieConfigMem;
	};
	union
	{
		unsigned long				BarVa[PLX_BAR_COUNT];
		struct
		{
			volatile PLXLCFGREGS		*pPlxMem;
			unsigned long				reserved;
			volatile unsigned int		*pConfigMem;
			volatile unsigned int		*pUseMem;
		};
	};
	int							iInterruptLevel;
    INTERNAL_DMA_CHANNEL		lpDmaChannels[PCIG_NB_DMA_CHANNEL];
    INTERNAL_INTERRUPTS			Interrupts;
	void*						lpSystemDescriptor;
	void*						lpLock;
	unsigned long				ulCardNo;
	unsigned long				ulPlxType;
} DEVICE_EXTENSION;

typedef struct _PCIG_STATIC
{
	int					iPcigCount;		/* PCIG card found count	*/
	int					iMajor;			/* Major number*/
	DEVICE_EXTENSION	PcigBrd[MAX_PCIG];
} PCIG_STATIC;

#endif /* _PCIGDRVSTRUCTS_H */

