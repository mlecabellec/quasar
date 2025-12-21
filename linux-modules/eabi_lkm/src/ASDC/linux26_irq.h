/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *             Redefinition locale de cli(), save_flags()               *
 *             et restore_flags() pour Linux v2.6.x                     *
 *                                                                      *
 *    (Provisoire, en attendant remplacement par spin_locks ...)        *
 *                         ==================                           *
 *                                                                      *
 *                            Y.Guillemot, pour v4.13 : le 25/10/2004   *
 *                    Adaptation aux processurs 64 bits le 12/01/2009   *
 ************************************************************************/


#ifndef _LINUX_INTERRUPT_H
#include <linux/interrupt.h>
#endif

// Sur machine CCC "RedHawk" (SVF Vega) : decommenter ligne ci-desous
// Et sur noyaux > v2.6.x avec 16 < x < 22
#define _PROVISOIREMENT_INUTILE_ET_A_CORRIGER_

/* Les redefinitions ci-dessous sont provisoirement devenues inutiles */
#ifdef _PROVISOIREMENT_INUTILE_ET_A_CORRIGER_

#define cli()		 __asm__ __volatile__("cli": : :"memory")

#define sti()		 __asm__ __volatile__("sti": : :"memory")

#ifdef __LP64__

#define save_flags(x)	 __asm__ __volatile__("pushfq ; popq %0":"=g" (x): /* no input */)

#define restore_flags(x) __asm__ __volatile__("pushq %0 ; popfq": /* no output */ :"g" (x):"memory", "cc")

#else  /* __LP64 __ */

#define save_flags(x)	 __asm__ __volatile__("pushfl ; popl %0":"=g" (x): /* no input */)

#define restore_flags(x) __asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"g" (x):"memory", "cc")

#endif  /* __LP64 __ */

#endif  /* _PROVISOIREMENT_INUTILE_ET_A_CORRIGER_ */

