#ifndef _PCIGINTERNALS_H
#define _PCIGINTERNALS_H
#include "pcigDrvStructs.h"
#include "pcigWrappers.h"

#define PCIG_LINUX_DEVICE_NAME	"Pcig"

#define PCIG_ERROR_SUCCESS		0
#define PCIG_ERROR_ERROR		1

#define PCIG_PLX_UNKNOW			0
#define PCIG_PLX_9080			1
#define PCIG_PLX_9056			2
#define PCIG_PLX_9054			3
#define PCIG_PLX_8311			4

#define PCIGDEVID_80			0x9080		/* PLX device ID */
#define PCIGVENDID_80			0x10b5		/* PLX vendor ID */
#define PCIGSUBDEVID_80			0x9080		/* on PCIG : same as device ID */
#define PCIGSUBVENDID_80		0x10b5		/* on PCIG : same as vendor ID */

#define PCIGDEVID_54			0x9054		/* PLX device ID */
#define PCIGVENDID_54			0x10b5		/* PLX vendor ID */
#define PCIGSUBDEVID_54			0x9054		/* on PCIG : same as device ID */
#define PCIGSUBVENDID_54		0x10b5		/* on PCIG : same as vendor ID */

#define PCIGDEVID_56			0x9056		/* PLX device ID */
#define PCIGVENDID_56			0x10b5		/* PLX vendor ID */
#define PCIGSUBDEVID_56			0x9056		/* on PCIG : same as device ID */
#define PCIGSUBVENDID_56		0x10b5		/* on PCIG : same as vendor ID */

#define PCIEGDEVID_8311			0x9056		/* PLX device ID */
#define PCIEGVENDID_8311		0x10b5		/* PLX vendor ID */
#define PCIEGSUBDEVID_8311		0x8311		/* Specific device ID for PCIEG */
#define PCIEGSUBVENDID_8311		0x10b5		/* on PCIEG : same as vendor ID */

#define PCIEGDEVID_8111			0x8111		/* PLX device ID */
#define PCIEGVENDID_8111		0x10b5		/* PLX vendor ID */

#define PCIGINTERNAL_DMA_CHANNEL_MASK	0x80000000
#define PCIGINTERNAL_CARD_NO_MASK		0x0000FFFF
#define PCIGINTERNAL_MAPPING_MASK		0x000F0000
#define PCIGINTERNAL_MAPPING_SHIFT		16

int						OnPcigInitModule(void);
void					OnPcigRemoveOne(void* pdev);
int						OnPcigInitOne(void* pdev);

void					PcigCleanupModule(void);
int 					PcigIntHandlerModule(DEVICE_EXTENSION* lpDev);
DEVICE_EXTENSION*		OnPcigOpen(int iMinorNumber);
DEVICE_EXTENSION*		OnPcigIntHandler(int iMinorNumber);

int						OnPcigAllocateDesignBuffer(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigGetAllocatedResources(DEVICE_EXTENSION * lpDev, RESOURCES_INFO *);
int						OnPcigGetLocalCfgReg(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int *);
int						OnPcigSetLocalCfgReg(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int);
int						OnPcigGetXilinxRegs(DEVICE_EXTENSION * lpDev, unsigned int ulStartRegName, unsigned int ulNbRegs, unsigned int * lpulRegsValue, unsigned int ulAddressInc);
int						OnPcigSetXilinxRegs(DEVICE_EXTENSION * lpDev, unsigned int ulStartRegName, unsigned int ulNbRegs, unsigned int * lpulRegsValue, unsigned int ulAddressInc);
int						OnPcigWriteAndCheckXilinxRegs(DEVICE_EXTENSION * lpDev, unsigned int ulInStartRegName, unsigned int ulInStartRegQual, unsigned int ulOutStartRegName, unsigned int ulOutStartRegQual, unsigned int ulNbRegs, unsigned int * lpulRegsValue, unsigned int ulInAddressInc, unsigned int ulOutAddressInc);
unsigned int 			OnPcigGetElementSize(unsigned int ulAddressInc);
int						OnPcigGetXilinxReg(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int*);
int						OnPcigSetXilinxReg(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int);
unsigned long			IdentifyEeprom(volatile unsigned int *, unsigned int);
int						OnPcigReadEERegister(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int *);
int						OnPcigWriteEERegister(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int );
int						OnPcigReadPcieEERegister(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int *);
int						OnPcigWritePcieEERegister(DEVICE_EXTENSION * lpDev, unsigned int, unsigned int );
int						OnPcigLoadLogicalDesign(DEVICE_EXTENSION * lpDev, char*, unsigned int);
int						OnPcigProgramLogicalDesign(DEVICE_EXTENSION * lpDev, char*, unsigned int);
int						OnPcigResetLocalBus(DEVICE_EXTENSION * lpDev);
int						OnPcigReleaseDmaChannel(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigReleaseUserInt(DEVICE_EXTENSION * lpDev);
int						OnPcigInitializeDmaChannel(DEVICE_EXTENSION * lpDev, DMA_CHANNEL *);
int						OnPcigStartDmaChannel(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigStopDmaChannel(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigResetDmaChannel(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigEnableInterrupts(DEVICE_EXTENSION * lpDev, unsigned int);
int						OnPcigRegisterUserInt(DEVICE_EXTENSION * lpDev, unsigned int, INT_DESC*);
int						OnPcigWaitForDma(DEVICE_EXTENSION * lpDev, unsigned int, WAIT_DESC*, unsigned int);
int						OnPcigWaitForUserInt(DEVICE_EXTENSION * lpDev, WAIT_DESC*, unsigned int);
int						OnPcigGetRegs(DEVICE_EXTENSION * lpDev, unsigned int * lpBuffer, unsigned int ulNbRegs, unsigned int ulStartAddr, unsigned int ulRegQual, unsigned int ulAddressInc);
int						OnPcigSetRegs(DEVICE_EXTENSION * lpDev, unsigned int * lpBuffer, unsigned int ulNbRegs, unsigned int ulStartAddr, unsigned int ulRegQual, unsigned int ulAddressInc);
int						OnPcigGetReg(DEVICE_EXTENSION * lpDev, unsigned int ulRegAddr, unsigned int ulRegQual, unsigned int* lpRegValue);
int						OnPcigSetReg(DEVICE_EXTENSION * lpDev, unsigned int ulRegAddr, unsigned int ulRegQual, unsigned int ulRegValue);

unsigned int			OnPcigGetPcigCount(void);
void					OnPcigSetMajor(int iMajor);
int						OnPcigGetMajor(void);
unsigned int			OnPcigGetDeviceID(DEVICE_EXTENSION * lpDev);

unsigned int			OnPcigGetDmaInterrruptCount(DEVICE_EXTENSION * lpDev, unsigned int ulChannelNo);
unsigned int			OnPcigGetUserInterruptCount(DEVICE_EXTENSION * lpDev, unsigned int ulInterruptNo);

void					OnPcigMmapOpen(void* lpVma);
void					OnPcigMmapClose(void* lpVma);
void*					OnPcigMmapFaultHandler(void* lpVma, unsigned long ulAddress, int* lpWriteAcess);
int						OnPcigMmapFaultHandlerVmf(void* lpVma, void* lpVmf);
int						OnPcigMmapAccessVmf(void *lpVma, unsigned long ulAddr, void *lpBuf, int iLength, int iWrite);
unsigned long			OnPcigRemapPlxRegisters(void* lpVma, unsigned long ulCardNo);
unsigned long			OnPcigRemapXilinxRegisters(void* lpVma, unsigned long ulCardNo);
unsigned long			OnPcigRemapConfigRegisters(void* lpVma, unsigned long ulCardNo);
int						OnPcigRemapPfnRange(void* lpVma, unsigned long ulCard, unsigned long ulDmaChannel);

int						PcigGetJTagTDO(DEVICE_EXTENSION *pDe);
int						PcigSetJTagPort(DEVICE_EXTENSION * pDe, int iTck, int iTms, int iTdi);
#endif /* _PCIG_INTERNALS_H */
