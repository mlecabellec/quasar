#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <asm/pgtable.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/stat.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/segment.h>
#include <asm/bitops.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
#include <linux/vmalloc.h>
#endif

#define PCIGWRAPPERS
#include "pcigWrappers.h"

spinlock_t wrap_lock[MAX_PCIG] = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_UNLOCKED} ;
#else
	__SPIN_LOCK_UNLOCKED(lock_0),
	__SPIN_LOCK_UNLOCKED(lock_1),
	__SPIN_LOCK_UNLOCKED(lock_2),
	__SPIN_LOCK_UNLOCKED(lock_3),
	__SPIN_LOCK_UNLOCKED(lock_4),
	__SPIN_LOCK_UNLOCKED(lock_5),
	__SPIN_LOCK_UNLOCKED(lock_6),
	__SPIN_LOCK_UNLOCKED(lock_7),
	__SPIN_LOCK_UNLOCKED(lock_8),
	__SPIN_LOCK_UNLOCKED(lock_9),
};
#endif

#ifndef VM_RESERVED
# define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
//SETPAGEFLAG(locked, locked, PF_ANY);
//CLEARPAGEFLAG(locked, locked, PF_ANY);
#else
//SETPAGEFLAG(Reserved, reserved);
SETPAGEFLAG(locked, locked);
//CLEARPAGEFLAG(Reserved, reserved);
CLEARPAGEFLAG(locked, locked);
#endif
#endif

#define DMA_ALLOCATION_STEP		(32 * PAGE_SIZE)

typedef struct _WRAP_DMA_BUFFER_DESC
{
	dma_addr_t			DmaHandle;
	void*				lpBuffer;
	unsigned long		DmaBufferSize;
} WRAP_DMA_BUFFER_DESC;

typedef struct _WRAP_DMA_DESC
{
		WRAP_DMA_BUFFER_DESC*	lpDmaBufferHandle;
		dma_addr_t				DmaDescriptorHandle;
		unsigned long			ulAllocatedDmaBuffer;
		unsigned long			ulUsedDmaBuffer;
		wait_queue_head_t 		WaitQueue;
} WRAP_DMA_DESC;

typedef struct _WRAP_INT_DESC
{
		wait_queue_head_t 		WaitQueue;
} WRAP_INT_DESC;

typedef struct _WRAP_DEVICE_DESC
{
	WRAP_DMA_DESC				lpDmaChannels[PCIG_NB_DMA_CHANNEL];
	WRAP_INT_DESC*				lpInterrupts;
	struct pci_dev*				lpDev;
	unsigned long				ulNbIntDescAllocated;
} WRAP_DEVICE_DESC;

inline struct pci_dev* hDeviceToPciDev(PCIGDEVICE lpDevice)
{
	return((struct pci_dev*)lpDevice);
}
/********************************/
/* print a string to actual tty */
/********************************/
int slen( char *str)
{
	int	cpt=0;

	while( *(str++) != '\0')
	{
		cpt++;
	}
	return(cpt);
}

/********************************/
/* print a string to actual tty */
/********************************/
void szero( char *str, int len)
{
	while(len--)
	{
		*(str++) = 0;
	}
}

/********************************/
/* print a string to actual tty */
/********************************/
void print_string(char *str)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	struct tty_struct *my_tty;
	
	my_tty = current->tty;
	if (my_tty == NULL)
	{
		return;
	}
	(*(my_tty->driver).write)(my_tty, 0, str, slen(str));
	(*(my_tty->driver).write)(my_tty, 0, "\015\012", 2);
#else
	printk(str);
#endif
}

int	WrapVerifyAreaRead(void* lpData, int iSize)
{
//	return(verify_area(VERIFY_READ, lpData, iSize));
	return(0);
}

int	WrapVerifyAreaWrite(void* lpData, int iSize)
{
//	return(verify_area(VERIFY_WRITE, lpData, iSize));
	return(0);
}

PCIGDEVICE WrapPciFindDevice(unsigned short usVendorId, unsigned short usDeviceId, PCIGDEVICE lpLastDevice)
{
	struct pci_dev* oldDev = NULL;

	oldDev = (struct pci_dev*)lpLastDevice;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
	return(pci_find_device(usVendorId, usDeviceId, oldDev));
#else
	return(pci_get_device(usVendorId, usDeviceId, oldDev));
#endif
}

void WrapPciEnableDevice(PCIGDEVICE lpDevice)
{
	pci_enable_device(hDeviceToPciDev(lpDevice));
	pci_set_master(hDeviceToPciDev(lpDevice));
}

int WrapPciIsParent(PCIGDEVICE lpDev, PCIGDEVICE lpParentDev)
{
	struct pci_dev* Device = NULL;
	struct pci_dev* ParentDevice = NULL;

	Device = hDeviceToPciDev(lpDev);
	ParentDevice = hDeviceToPciDev(lpParentDev);
	if(Device->bus == ParentDevice->subordinate)
	{
		return(0);
	}
	return(1);
}

int WrapPciReadConfigByte(PCIGDEVICE lpDevice, int iAddr, unsigned char* lpValue)
{
	int iReturn = 0;

	iReturn = pci_read_config_byte(hDeviceToPciDev(lpDevice), iAddr, lpValue);
	if(iReturn == PCIBIOS_SUCCESSFUL)
	{
		return(WRAP_ERROR_SUCCESS);
	}
	return(WRAP_ERROR_ERROR);
}

int	WrapPciReadConfigWord(PCIGDEVICE lpDevice, int iAddr, unsigned short* lpValue)
{
	int iReturn = 0;

	iReturn = pci_read_config_word(hDeviceToPciDev(lpDevice), iAddr, lpValue);
	if(iReturn == PCIBIOS_SUCCESSFUL)
	{
		return(WRAP_ERROR_SUCCESS);
	}
	return(WRAP_ERROR_ERROR);
}

int WrapPciReadConfigDword(PCIGDEVICE lpDevice, int iAddr, unsigned int* lpValue)
{
	int iReturn = 0;

	iReturn = pci_read_config_dword(hDeviceToPciDev(lpDevice), iAddr, lpValue);
	if(iReturn == PCIBIOS_SUCCESSFUL)
	{
		return(WRAP_ERROR_SUCCESS);
	}
	return(WRAP_ERROR_ERROR);
}

unsigned char WrapGetBusNumber(PCIGDEVICE lpDevice)
{
	struct pci_dev* dev = hDeviceToPciDev(lpDevice);

	return(dev->bus->number);
}

unsigned char WrapGetFunctionNumber(PCIGDEVICE lpDevice)
{
	struct pci_dev* dev = hDeviceToPciDev(lpDevice);

	return(dev->devfn);
}

unsigned int WrapGetDeviceId(PCIGDEVICE lpDevice)
{
	struct pci_dev* dev = hDeviceToPciDev(lpDevice);

	return(dev->device);
}

unsigned int WrapGetSubDeviceId(PCIGDEVICE lpDevice)
{
	struct pci_dev* dev = hDeviceToPciDev(lpDevice);

	return(dev->subsystem_device);
}

unsigned char WrapGetIrq(PCIGDEVICE lpDevice)
{
	struct pci_dev* dev = hDeviceToPciDev(lpDevice);

	return(dev->irq);
}

unsigned long WrapPciResourceStart(PCIGDEVICE lpDevice, unsigned long ulBar)
{
	return(pci_resource_start(hDeviceToPciDev(lpDevice), ulBar));
}

unsigned long WrapPciResourceFlags(PCIGDEVICE lpDevice, unsigned long ulBar)
{
	return(pci_resource_flags(hDeviceToPciDev(lpDevice), ulBar));
}

unsigned long WrapIsIoResource(PCIGDEVICE lpDevice, unsigned long ulBar)
{
	return(pci_resource_flags(hDeviceToPciDev(lpDevice), ulBar) & IORESOURCE_IO);
}

unsigned long WrapPciResourceLen(PCIGDEVICE lpDevice, unsigned long ulBar)
{
	return(pci_resource_len(hDeviceToPciDev(lpDevice), ulBar));
}

void* WrapRequestRegion(unsigned long ulStart, unsigned long ulLen, char* lpszName)
{
	return(request_region(ulStart, ulLen, lpszName));
}

void* WrapRequestMemRegion(unsigned long ulStart, unsigned long ulLen, char* lpszName)
{
	return(request_mem_region(ulStart, ulLen, lpszName));
}

void* WrapIoremapNocache(unsigned long ulStart, unsigned long ulLen)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
	return(ioremap(ulStart, ulLen));
#else
	return(ioremap_nocache(ulStart, ulLen));
#endif
}

void WrapReleaseRegion(unsigned long ulStart, unsigned long ulLen)
{
	return(release_region(ulStart, ulLen));
}

void WrapReleaseMemRegion(unsigned long ulStart, unsigned long ulLen)
{
	return(release_mem_region(ulStart, ulLen));
}

void WrapIounmap(unsigned long ulStart)
{
	return(iounmap((void*)ulStart));
}

void* WrapVmalloc(unsigned long ulLen)
{
	return(vmalloc(ulLen));
}

void WrapVfree(void* lpBuffer)
{
	vfree(lpBuffer);
}

void* WrapRvmalloc(unsigned long ulLen)
{
	void*			mem;
	unsigned long	adr;
	unsigned long	page;
    long            lsize;

	mem = vmalloc(ulLen);
	if(mem != 0)
	{
		adr = (unsigned long)mem;
        lsize = ulLen;
		while(lsize > 0)
		{
			page = (unsigned long)vmalloc_to_page((void*)adr);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20)
			SetPageReserved(vmalloc_to_page((void*)adr));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
			lock_page(vmalloc_to_page((void*)adr));
#else
                        SetPagelocked(vmalloc_to_page((void*)adr));
#endif
#else
			SetPageLocked(vmalloc_to_page((void*)adr));
#endif
#else
			mem_map_reserve(vmalloc_to_page((void*)adr));
			LockPage(vmalloc_to_page((void*)adr));
#endif
			adr += PAGE_SIZE;
			lsize -= PAGE_SIZE;
		}
	}
	return(mem);
}

void WrapRvfree(void* lpBuffer, unsigned long ulLen)
{
	unsigned long	adr;
	unsigned long	page;
        long            lsize;
	if(lpBuffer != 0)
	{
		adr = (unsigned long)lpBuffer;
        lsize = ulLen;
		while(lsize > 0)
		{
			page = (unsigned long)vmalloc_to_page((void*)adr);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,20)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
			unlock_page(vmalloc_to_page((void*)adr));
#else
			ClearPagelocked(vmalloc_to_page((void*)adr));
#endif
#else
			ClearPageLocked(vmalloc_to_page((void*)adr));
#endif
			ClearPageReserved(vmalloc_to_page((void*)adr));
#else
			mem_map_unreserve(vmalloc_to_page((void*)adr));
			UnlockPage(vmalloc_to_page((void*)adr));
#endif
			adr += PAGE_SIZE;
			lsize -= PAGE_SIZE;
		}
		vfree(lpBuffer);
	}
}

void *WrapAllocDmaDescriptors(PCIGDEVICE lpDevice, unsigned long size, void* dma_handle, int flag)
{
	void* lpBuffer = NULL;
	unsigned long	adr;
    long            lsize;
	struct pci_dev* dev = NULL;

	dev = (struct pci_dev*)lpDevice;

	lpBuffer = pci_alloc_consistent(dev, size, dma_handle);
	if(lpBuffer != NULL)
	{
		adr = (unsigned long)lpBuffer;
		lsize = size;
		while(lsize > 0)
		{
			get_page(virt_to_page((void *)adr));
			SetPageReserved(virt_to_page((void*)adr));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
			lock_page(virt_to_page((void*)adr));
#else
                        SetPagelocked(virt_to_page((void*)adr));
#endif
#else
			SetPageLocked(virt_to_page((void*)adr));
#endif
			adr += PAGE_SIZE;
			lsize -= PAGE_SIZE;
		}
	}
	return(lpBuffer);
}

void WrapFreeDmaDescriptors(PCIGDEVICE lpDevice, unsigned long size, void*lpBuffer, void* dma_handle)
{
	unsigned long	adr;
    long            lsize;
	dma_addr_t*		lpDma = NULL;
	struct pci_dev* dev = NULL;

	dev = (struct pci_dev*)lpDevice;

	lpDma = (dma_addr_t*)dma_handle;
	if(lpBuffer != NULL)
	{
		adr = (unsigned long)lpBuffer;
		lsize = size;
		while(lsize > 0)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
			unlock_page(virt_to_page((void*)adr));
#else
			ClearPagelocked(virt_to_page((void*)adr));
#endif
#else
			vmalloc_to_page(virt_to_page((void*)adr));
#endif
			ClearPageReserved(virt_to_page((void*)adr));
			put_page_testzero(virt_to_page((void *)adr));
			adr += PAGE_SIZE;
			lsize -= PAGE_SIZE;
		}
		pci_free_consistent(dev, size, lpBuffer, *lpDma);
	}
	return;
}

int WrapAllocDmaBuffers(PCIGDEVICE lpDevice, unsigned long size, void* lpWrapDeviceDesc, int iChannelNo, int flag)
{
	unsigned long	adr;
	unsigned long	ulBytesLeftInBuffer;
	unsigned long	ulAllocationSize;
    long            lsize;
	struct pci_dev* dev = NULL;
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	dev = (struct pci_dev*)lpDevice;
	if(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer != 0)
	{
		return(-1);
	}
	if((size / DMA_ALLOCATION_STEP) > lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulAllocatedDmaBuffer)
	{
		return(-1);
	}
	ulBytesLeftInBuffer = size;
	while(ulBytesLeftInBuffer > 0)
	{
		if(ulBytesLeftInBuffer > DMA_ALLOCATION_STEP)
		{
			ulAllocationSize = DMA_ALLOCATION_STEP;
		}
		else
		{
			ulAllocationSize = ulBytesLeftInBuffer;
		}
		lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer].lpBuffer = pci_alloc_consistent(dev, ulAllocationSize, &(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer].DmaHandle));
		if(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer].lpBuffer != NULL)
		{
			lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer].DmaBufferSize = ulAllocationSize;
			adr = (unsigned long)lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer].lpBuffer;
			lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer++;
			lsize = ulAllocationSize;
			while(lsize > 0)
			{
				get_page(virt_to_page((void *)adr));
				SetPageReserved(virt_to_page((void*)adr));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
				lock_page(virt_to_page((void*)adr));
#else
        	                SetPagelocked(virt_to_page((void*)adr));
#endif
#else
				SetPageLocked(virt_to_page((void*)adr));
#endif
				adr += PAGE_SIZE;
				lsize -= PAGE_SIZE;
			}
		}
		else
		{
			return(-1);
		}
		ulBytesLeftInBuffer -= ulAllocationSize;
	}
	return(0);
}

void WrapFreeDmaBuffers(PCIGDEVICE lpDevice, void* lpWrapDeviceDesc, int iChannelNo)
{
	unsigned long	adr;
    long            lsize;
	struct pci_dev* dev = NULL;
	unsigned long	i;
	WRAP_DEVICE_DESC* lpDeviceDesc;

	dev = (struct pci_dev*)lpDevice;
	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	for(i = 0; i < lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer; i++)
	{
		if(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer != NULL)
		{
			adr = (unsigned long)lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer;
			lsize = lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize;
			while(lsize > 0)
			{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
				unlock_page(virt_to_page((void*)adr));
#else
				ClearPagelocked(virt_to_page((void*)adr));
#endif
#else
				ClearPageLocked(virt_to_page((void*)adr));
#endif
				ClearPageReserved(virt_to_page((void*)adr));
				put_page_testzero(virt_to_page((void *)adr));
				adr += PAGE_SIZE;
				lsize -= PAGE_SIZE;
			}
			pci_free_consistent(dev, lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize, lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer, lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaHandle);
			lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize = 0;
			lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer = NULL;
		}
	}
	lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer = 0;
	return;
}

void WrapInitWaitQueueHead(void* lpWaitQueue)
{
	init_waitqueue_head(lpWaitQueue);
	return;
}

void WrapWakeUpInterruptible(void* lpWaitQueue)
{
	wake_up_interruptible(lpWaitQueue);
	return;
}

void WrapInterruptibleSleepOnTimeout(void* lpWaitQueue, unsigned long ulTimeOut)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	interruptible_sleep_on_timeout(lpWaitQueue, ulTimeOut);
#else
	DEFINE_WAIT(wait);
	prepare_to_wait(lpWaitQueue, &wait, TASK_INTERRUPTIBLE);
	ulTimeOut = schedule_timeout(ulTimeOut);
	finish_wait(lpWaitQueue, &wait);
#endif
	return;
}

int WrapRequestIrq(int level, char* lpszName, void* pDeviceExtension)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	request_irq(level, (irq_handler_t)SkelGetInterruptHandler(), IRQF_SHARED, lpszName, pDeviceExtension);
#else
	request_irq(level, SkelGetInterruptHandler(), SA_INTERRUPT | SA_SHIRQ, lpszName, pDeviceExtension);
#endif
	return(0);
}

void WrapFreeIrq(int level, void* pDeviceExtension)
{
	free_irq(level, pDeviceExtension);
	return;
}


void * WrapAllocDeviceDesc(int iInterruptCount, int iMaxDmaBufferCount, PCIGDEVICE lpDevice)
{
	WRAP_DEVICE_DESC*	lpDeviceDesc;
	unsigned long		i = 0;
	unsigned long		j = 0;

	lpDeviceDesc = WrapVmalloc(sizeof(WRAP_DEVICE_DESC));
	if(lpDeviceDesc == NULL)
	{
		return(NULL);
	}
	lpDeviceDesc->ulNbIntDescAllocated = 0;
	for(i = 0; i < PCIG_NB_DMA_CHANNEL; i++)
	{
		lpDeviceDesc->lpDmaChannels[i].DmaDescriptorHandle = 0;
		lpDeviceDesc->lpDmaChannels[i].lpDmaBufferHandle = NULL;
		lpDeviceDesc->lpDmaChannels[i].lpDmaBufferHandle = WrapVmalloc(iMaxDmaBufferCount * sizeof(WRAP_DMA_BUFFER_DESC));
		if(lpDeviceDesc->lpDmaChannels[i].lpDmaBufferHandle == NULL)
		{
			for(j = i; j > 0; j++)
			{
				WrapVfree(lpDeviceDesc->lpDmaChannels[j - 1].lpDmaBufferHandle);
				lpDeviceDesc->lpDmaChannels[j - 1].lpDmaBufferHandle = NULL;
			}
			WrapVfree(lpDeviceDesc);
			return(NULL);
		}
		for(j = 0; j < iMaxDmaBufferCount; j++)
		{
			lpDeviceDesc->lpDmaChannels[i].lpDmaBufferHandle[j].lpBuffer = NULL;
			lpDeviceDesc->lpDmaChannels[i].lpDmaBufferHandle[j].DmaBufferSize = 0;
		}
		lpDeviceDesc->lpDmaChannels[i].ulAllocatedDmaBuffer = iMaxDmaBufferCount;
		lpDeviceDesc->lpDmaChannels[i].ulUsedDmaBuffer = 0;
	}
	lpDeviceDesc->lpInterrupts = NULL;
	lpDeviceDesc->lpInterrupts = WrapVmalloc(iInterruptCount * sizeof(WRAP_INT_DESC));
	if(lpDeviceDesc->lpInterrupts == NULL)
	{
		WrapVfree(lpDeviceDesc);
		return(NULL);
	}
	lpDeviceDesc->ulNbIntDescAllocated = iInterruptCount;
	lpDeviceDesc->lpDev = lpDevice;
	return(lpDeviceDesc);
}

void WrapReleaseDeviceDesc(void* lpWrapDeviceDesc)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	if(lpDeviceDesc->ulNbIntDescAllocated != 0)
	{
		WrapVfree(lpDeviceDesc->lpInterrupts);
		lpDeviceDesc->lpInterrupts = NULL;
	}
	WrapVfree(lpDeviceDesc);
	return;
}

void* WrapGetDevice(void* lpWrapDeviceDesc)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(lpDeviceDesc->lpDev);
}

void* WrapGetDmaDescriptorHandle(void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(&(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].DmaDescriptorHandle));
}

/*
void* WrapGetDmaBufferHandle(void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(&(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].dma_buffer_handle));
}
*/
unsigned long	WrapGetDmaDescriptorHandleContent(void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].DmaDescriptorHandle);
}

/*
unsigned long	WrapGetDmaBufferHandleContent(void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].dma_buffer_handle);
}
*/

unsigned long WrapGetDmaBufferPhysicalAddressAtOffset(void* lpWrapDeviceDesc, int iChannelNo, unsigned long ulOffset, unsigned long * lpulRemindSize)
{
	WRAP_DEVICE_DESC*	lpDeviceDesc;
	unsigned long		ulBufferOffset;
	unsigned long		i;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	ulBufferOffset = 0;
	for(i = 0; i < lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer; i++)
	{
		if((ulBufferOffset <= ulOffset) && ((ulBufferOffset + lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize) > ulOffset))
		{
			*lpulRemindSize = (ulBufferOffset + lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize) - ulOffset;
			return(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaHandle + (ulOffset - ulBufferOffset));
		}
		ulBufferOffset += lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize;
	}
	return(0);
}

void* WrapGetPageAtOffset(void* lpWrapDeviceDesc, int iChannelNo, unsigned long ulOffset)
{
	WRAP_DEVICE_DESC*	lpDeviceDesc;
	unsigned long		ulBufferOffset;
	unsigned long		i;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	ulBufferOffset = 0;
	for(i = 0; i < lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer; i++)
	{
		if((ulBufferOffset <= ulOffset) && ((ulBufferOffset + lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize) > ulOffset))
		{
			return(virt_to_page(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer + (ulOffset - ulBufferOffset)));
		}
		ulBufferOffset += lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize;
	}
	return(0);
}

void* WrapGetInterruptWaitQueue(void* lpWrapDeviceDesc, int iInterruptNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	if(lpDeviceDesc->ulNbIntDescAllocated > iInterruptNo)
	{
		return(&(lpDeviceDesc->lpInterrupts[iInterruptNo].WaitQueue));
	}
	return(NULL);
}

void* WrapGetDmaInterruptWaitQueue(void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	return(&(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].WaitQueue));
}

void WrapMdelay(unsigned long ulDelay)
{
	mdelay(ulDelay);
	return;
}

void WrapUdelay(unsigned long ulDelay)
{
	udelay(ulDelay);
	return;
}

void* WrapVirtToPage(void* lpVirtualAddress)
{
	return(virt_to_page(lpVirtualAddress));
}

int WrapGetVmFaultMinor()
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,6,0)
        return(0);
#else
        return(VM_FAULT_MINOR);
#endif
}

void WrapGetPage(void* lpPageAddress)
{
	get_page((struct page *)lpPageAddress);
	return;
}

unsigned long WrapGetVmPrivateData(void* lpVma)
{
	return((unsigned long)(((struct vm_area_struct*)lpVma)->vm_private_data));
}

unsigned long WrapGetVmStart(void* lpVma)
{
	return((unsigned long)(((struct vm_area_struct*)lpVma)->vm_start));
}

unsigned long WrapGetVmPgOffset(void* lpVma)
{
	return((unsigned long)(((struct vm_area_struct*)lpVma)->vm_pgoff));
}

unsigned long	WrapGetVmVirtualAddress(void* lpVmf)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	return((unsigned long)(((struct vm_fault *)lpVmf)->address));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	return((unsigned long)(((struct vm_fault *)lpVmf)->virtual_address));
#else
	return(0);
#endif
}

unsigned long	WrapGetVmPgOff(void* lpVmf)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	return((unsigned long)(((struct vm_fault *)lpVmf)->pgoff) << PAGE_SHIFT);
#else
	return(0);
#endif
}

int WrapGetVmFaultSigBus()
{
	return(VM_FAULT_SIGBUS);
}

void WrapGetVmSetPage(void* lpVmf, void* lpPhysicalAddr)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	((struct vm_fault *)lpVmf)->page = lpPhysicalAddr;
#endif
	return;
}

unsigned long WrapGetPageShift()
{
	return(PAGE_SHIFT);
}

int WrapRemapPfnRange(void* lpVma, void* lpWrapDeviceDesc, int iChannelNo)
{
	WRAP_DEVICE_DESC* lpDeviceDesc;
	unsigned long		i;
	unsigned long		ulAreaSize;
	unsigned long		ulRemapSize;
	unsigned long		ulRemapOffset;
	int			iReturn;

	lpDeviceDesc = (WRAP_DEVICE_DESC*)lpWrapDeviceDesc;
	ulRemapOffset = 0;
	if(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer != 0)
	{
		ulAreaSize = ((struct vm_area_struct*)lpVma)->vm_end - ((struct vm_area_struct*)lpVma)->vm_start;
		for(i = 0; i < lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].ulUsedDmaBuffer; i++)
		{
			ulRemapSize = lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].DmaBufferSize;
			if((ulRemapOffset + ulRemapSize + PAGE_SIZE) > ulAreaSize)
			{
				ulRemapSize = ulAreaSize - ulRemapOffset;
			}
			iReturn = remap_pfn_range(((struct vm_area_struct*)lpVma),
				((struct vm_area_struct*)lpVma)->vm_start + ulRemapOffset,
				__pa(lpDeviceDesc->lpDmaChannels[iChannelNo == 0?0:1].lpDmaBufferHandle[i].lpBuffer) >> PAGE_SHIFT, 
				ulRemapSize,
				((struct vm_area_struct*)lpVma)->vm_page_prot);
			if(iReturn < 0)
			{
				return(-EIO);
			}
			ulRemapOffset += ulRemapSize;
			if(ulRemapOffset >= ulAreaSize)
			{
				break;
			}
		}
	}
	else
	{
		return(-EIO);
	}
	return(0);
}

int WrapRemapRegisters(void* lpVma, unsigned long ulPhysicalAddress, unsigned long ulSize)
{
	((struct vm_area_struct*)lpVma)->vm_flags |= VM_RESERVED;
	((struct vm_area_struct*)lpVma)->vm_flags |= VM_IO;
	((struct vm_area_struct*)lpVma)->vm_page_prot = pgprot_noncached(((struct vm_area_struct*)lpVma)->vm_page_prot);
	return(io_remap_pfn_range(((struct vm_area_struct*)lpVma),
		((struct vm_area_struct*)lpVma)->vm_start,
		ulPhysicalAddress >> PAGE_SHIFT,
		((struct vm_area_struct*)lpVma)->vm_end - ((struct vm_area_struct*)lpVma)->vm_start,
		((struct vm_area_struct*)lpVma)->vm_page_prot));
}
void WrapDeviceCreate(void* lpClass, int iMajor, int iMinor, char* lpszDevName)
{
	device_create(lpClass, NULL, MKDEV(iMajor, iMinor), NULL, lpszDevName, iMinor);
	return;
}

void WrapDeviceDestroy(void* lpClass, int iMajor, int iMinor)
{
	device_destroy(lpClass,  MKDEV(iMajor, iMinor));
	return;
}


void*   WrapGetLock(unsigned long ulLockIndex)
{
	if(ulLockIndex >= MAX_PCIG)
	{
		return(NULL);
	}
	return(&(wrap_lock[ulLockIndex]));
}

void    WrapSpinLockIrqSave(void* lpLock)
{
	unsigned long		flags;

	spin_lock_irqsave(lpLock, flags);
}

void    WrapSpinUnlockIrqRestore(void* lpLock)
{
	unsigned long		flags = 0;

	spin_unlock_irqrestore(lpLock, flags);

}

unsigned long WrapMsTimeToJiffies(unsigned long ulMsTime)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
        u64 ns=1000ull * ulMsTime;
        return(nsecs_to_jiffies64(ns));
#else
        struct timeval tv;

        tv.tv_sec = ulMsTime / 1000;
        tv.tv_usec = (ulMsTime % 1000) * 1000;
        return(timeval_to_jiffies(&tv));
#endif

}

int	WrapSprintf(char *str, const char *format, ...)
{
	va_list va;
	int	iReturn = 0;

	va_start(va, format);
	iReturn = vsprintf(str, format, va);
	va_end(va);
	return(iReturn);
}
