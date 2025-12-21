#ifndef _PCIGWRAPPERS_H
#define _PCIGWRAPPERS_H

#include "pcigSkel.h"
#define DEBUGK
#ifdef DEBUGK
#define pdebug(x)				if(SkelGetDebugEnableParam() == 1) {print_string x; print_string("\n");} 	/* output to console */
#else
#define pdebug(x)
#endif

int slen( char *str);
void szero( char *str, int len);
void print_string(char *str);

#define WRAP_ERROR_SUCCESS			0
#define WRAP_ERROR_ERROR			1


typedef void* PCIGDEVICE;	

int				WrapVerifyAreaRead(void* lpData, int iSize);
int				WrapVerifyAreaWrite(void* lpData, int iSize);

PCIGDEVICE		WrapPciFindDevice(unsigned short usVendorId, unsigned short usDeviceId, PCIGDEVICE lpLastDevice);
void			WrapPciEnableDevice(PCIGDEVICE lpDevice);
int				WrapPciIsParent(PCIGDEVICE lpDev, PCIGDEVICE lpParentDev);
int				WrapPciReadConfigByte(PCIGDEVICE lpDevice, int iAddr, unsigned char* lpValue);
int				WrapPciReadConfigWord(PCIGDEVICE lpDevice, int iAddr, unsigned short* lpValue);
int				WrapPciReadConfigDword(PCIGDEVICE lpDevice, int iAddr, unsigned int* lpValue);
unsigned char	WrapGetBusNumber(PCIGDEVICE lpDevice);
unsigned char	WrapGetFunctionNumber(PCIGDEVICE lpDevice);
unsigned int	WrapGetDeviceId(PCIGDEVICE lpDevice);
unsigned int	WrapGetSubDeviceId(PCIGDEVICE lpDevice);
unsigned char	WrapGetIrq(PCIGDEVICE lpDevice);
unsigned long	WrapPciResourceStart(PCIGDEVICE lpDevice, unsigned long ulBar);
unsigned long	WrapPciResourceFlags(PCIGDEVICE lpDevice, unsigned long ulBar);
unsigned long	WrapIsIoResource(PCIGDEVICE lpDevice, unsigned long ulBar);
unsigned long	WrapPciResourceLen(PCIGDEVICE lpDevice, unsigned long ulBar);
//unsigned long	WrapPciBarResourceMap(PCIGDEVICE lpDevice, unsigned long ulBar);

void*			WrapRequestRegion(unsigned long ulStart, unsigned long ulLen, char* lpszName);
void*			WrapRequestMemRegion(unsigned long ulStart, unsigned long ulLen, char* lpszName);
void*			WrapIoremapNocache(unsigned long ulStart, unsigned long ulLen);

void			WrapReleaseRegion(unsigned long ulStart, unsigned long ulLen);
void			WrapReleaseMemRegion(unsigned long ulStart, unsigned long ulLen);
void			WrapIounmap(unsigned long ulStart);

void*			WrapVmalloc(unsigned long ulLen);
void			WrapVfree(void* lpBuffer);
void*			WrapRvmalloc(unsigned long ulLen);
void			WrapRvfree(void* lpBuffer, unsigned long ulLen);

void *			WrapAllocDmaDescriptors(PCIGDEVICE lpDevice, unsigned long size, void* dma_handle, int flag);
void			WrapFreeDmaDescriptors(PCIGDEVICE dev, unsigned long size, void* lpBuffer, void* dma_handle);
int				WrapAllocDmaBuffers(PCIGDEVICE lpDevice, unsigned long size, void* lpWrapDeviceDesc, int iChannelNo, int flag);
void			WrapFreeDmaBuffers(PCIGDEVICE dev, void* lpWrapDeviceDesc, int iChannelNo);

int				WrapRequestIrq(int level, char* lpszName, void* pDeviceExtension);
void			WrapFreeIrq(int level, void* pDeviceExtension);

void			WrapInitWaitQueueHead(void* lpWaitQueue);
void			WrapWakeUpInterruptible(void* lpWaitQueue);
void			WrapInterruptibleSleepOnTimeout(void* lpWaitQueue, unsigned long ulTimeOut);

void *			WrapAllocDeviceDesc(int iInterruptCount, int iMaxDmaBufferCount, PCIGDEVICE lpDevice);
void			WrapReleaseDeviceDesc(void* lpWrapDeviceDesc);
void*			WrapGetDevice(void* lpWrapDeviceDesc);
void*			WrapGetDmaDescriptorHandle(void* lpWrapDeviceDesc, int iChannelNo);
//void*			WrapGetDmaBufferHandle(void* lpWrapDeviceDesc, int iChannelNo);
unsigned long	WrapGetDmaDescriptorHandleContent(void* lpWrapDeviceDesc, int iChannelNo);
//unsigned long	WrapGetDmaBufferHandleContent(void* lpWrapDeviceDesc, int iChannelNo);
unsigned long	WrapGetDmaBufferPhysicalAddressAtOffset(void* lpWrapDeviceDesc, int iChannelNo, unsigned long ulOffset, unsigned long *lpulRemindSize);
void*			WrapGetPageAtOffset(void* lpWrapDeviceDesc, int iChannelNo, unsigned long ulOffset);
void*			WrapGetInterruptWaitQueue(void* lpWrapDeviceDesc, int iInterruptNo);
void*			WrapGetDmaInterruptWaitQueue(void* lpWrapDeviceDesc, int iChannelNo);

void			WrapMdelay(unsigned long ulDelay);
void			WrapUdelay(unsigned long ulDelay);

void*			WrapVirtToPage(void* lpVirtualAddress);
int				WrapGetVmFaultMinor(void);
void			WrapGetPage(void* lpPageAddress);
unsigned long	WrapGetVmPrivateData(void* lpVma);
unsigned long	WrapGetVmStart(void* lpVma);
unsigned long	WrapGetVmPgOffset(void* lpVma);
unsigned long	WrapGetVmVirtualAddress(void* lpVmf);
int		WrapGetVmFaultSigBus(void);
unsigned long	WrapGetVmPgOff(void* lpVmf);
void		WrapGetVmSetPage(void* lpVmf, void*lpPhysicalAddr);
unsigned long	WrapGetPageShift(void);
int		WrapRemapPfnRange(void* lpVma, void* lpWrapDeviceDesc, int iChannelNo);
int		WrapRemapRegisters(void* lpVma, unsigned long ulPhysicalAddress, unsigned long ulSize);
void	WrapDeviceCreate(void* lpClass, int iMajor, int iMinor, char* lpszDevName);
void	WrapDeviceDestroy(void* lpClass, int iMajor, int iMinor);
void*	WrapGetLock(unsigned long ulLockIndex);
void	WrapSpinLockIrqSave(void* lpLock);
void	WrapSpinUnlockIrqRestore(void* lpLock);
unsigned long   WrapMsTimeToJiffies(unsigned long ulMsTime);
int		WrapSprintf(char *str, const char *format, ...);

#endif /* _PCIGWRAPPERS_H */
