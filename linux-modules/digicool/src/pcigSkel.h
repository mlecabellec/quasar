#ifndef _PCIGSKEL_H
#define _PCIGSKEL_H

#include "pcigDrvStructs.h"

/*
 * Fonctions d'interrogation des paramètres du module
 */
int	SkelGetDebugEnableParam(void);
int	SkelGetMaxInterruptDescriptorsParam(void);
int	SkelGetMaxDmaBufferDescriptorsParam(void);
int	SkelGetMaxDmaBuffersParam(void);
int	SkelGetWaitAfterResetLocalBus(void);
void* SkelGetPcigClass(void);

#ifdef PCIGWRAPPERS
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
void (*SkelGetInterruptHandler())( int, void *, struct pt_regs *);
#else
irqreturn_t (*SkelGetInterruptHandler(void))( int, void *, struct pt_regs *);
#endif
#endif
#endif /* _PCIGSKEL_H */
