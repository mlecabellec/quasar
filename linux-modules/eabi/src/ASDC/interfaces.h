/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *             Macros d'interfacage, pour utilisation du                *
 *             meme code source sous LynxOS ou sous Linux               *
 *                                                                      *
 *                         ==================                           *
 *                                                                      *
 *                                         Y.Anonymized, le 26/09/2002   *
 *                                          pour v4.6 : le 26/09/2002   *
 *                                          pour v4.9 : le  8/01/2003   *
 *                                         pour v4.11 : le 21/04/2004   *
 *         Pour v4.12, adaptation au noyau Linux v2.6.x le 26/10/2004   *
 *                  Corrections pour retour sous LynxOS le  9/03/2005   *
 *               Adaptation a la reecriture de asdcwq.c le  8/09/2005   *
 *                                                                      *
 *          Ajout compatibilite Linux 64 bits (debut) : le  9/01/2009   *
 *   Pour essais : Modif. compatibilite Linux recente : le 22/03/2013   *
 *               Ajout fonctions SA_LIER et SA_DELIER : le 11/06/2014   *
 *                Ajout fonction decalerFifo_unlocked : le 17/06/2014   *
 ************************************************************************/



#ifndef ASDC_LINUX_INTERFACE_H
#define ASDC_LINUX_INTERFACE_H



/*********************/
/* Macros pour Linux */
/*********************/

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif

#include "asdc_statics.h"


/* Constantes definies sous LynxOS, mais pas sous Linux */
#define OK	 0
#define SYSERR  -1

/* Constante definie sous QNX, mais pas sous Linux */
#define EOK      0


/* Retour de la fonction ioctl */
#define RETURN(x)	do { dst->dphioctl = -1;			\
			     return -x;					\
			   } while(0)


/* Test du transfert Depuis le domaine UTILisateur */
#define TDUTIL(arg, taille)			{   }


/* transfert Depuis le domaine UTILisateur */
#define DUTIL(zone, pzone, arg, taille)					\
        do { pzone = &zone;						\
             if (copy_from_user((void *)pzone, (void *)arg, taille)) 	\
               { KPRINTF("asdc : Echec de DUTIL !\n");			\
                 RETURN(EFAULT);					\
               }							\
           } while(0)


/* Test du transfert Vers le domaine UTILisateur */
#define TVUTIL(zone, pzone, arg, taille)	do { pzone = &zone; } while(0)


/* transfert Vers le domaine UTILisateur */
#define VUTIL(zone, pzone, arg, taille)					\
        do { if (copy_to_user((void *)arg, (void *)pzone, taille))	\
               { KPRINTF("asdc : Echec de VUTIL !\n");			\
                 RETURN(EFAULT);					\
               }							\
           } while(0)


/* Semaphore noyau (pour protection, type mutex) */
#define SEMWAIT(x, y)	bidon("", x, y)
#define SEMSIGNAL(x)	bidon("", x)


/* Options des semaphores (definies dans LynxOS) */
#define SEM_SIGIGNORE	0
#define SEM_SIGABORT	1
#define SEM_SIGRETRY	2


/* Semaphore noyau (pour synchronisation) */
///#define SDECLARE(x)	wait_queue_head_t x
#define SINIT(x)	init_waitqueue_head(&(x))
#define SRESET(x, c)	do { *c = 1; wake_up_interruptible_all(x); } while(0)

/// ATTENTION ATTENTION ATTENTION ATTENTION ATTENTION ATTENTION ATTENTION
/// Ajoute pour faciliter compilation
/// DOIT ETRE REMPLACE PAR SRESET (avec 2 params) ASAP !!!!!!!!!!!!!!
#define SRESET_OLD(x)	wake_up_interruptible_all(x)

#define SWAIT_IM(x, y)	wq_dormir(dst, x)
/*     -------- TODO ------------                       */
/* L'option y de SWAIT_IM devrait etre prise en compte, */
/* comme dans la definition ci-dessous de SWAIT         */

#define SRESET_IM(x)	wq_reveiller(dst, x)


static inline int SWAIT(wait_queue_head_t *q, int *cond, int option)
                     {
                       switch (option)
                         { default :
                                 /* C'est un cas d'erreur, mais qui est
                                    difficile a prendre en compte puisque
                                    seules les 2 valeurs de retour OK et
                                    SYSERR sont prevues et deja utilisees !
                                    ==> Pour l'instant, on traite alors
                                    simplement le cas le + frequent ...
                                 */
                                 printk("BUG ASDC : Appel de SWAIT avec option"
                                        " = %d = 0x%X\n", option, option);
                           case SEM_SIGIGNORE :
                                  /* On devrait, ici, appeler sleep_on()
                                     mais il faudrait alors que SRESET appelle
                                     soit wake_up_interruptible(), soit
                                     wake_up(). Ceci necessite des modifs
                                     qui restent A FAIRE !!!
                                     ==> Pour l'instant, pas de diference entre
                                     SEM_SIGIGNORE et SEM_SIGABORT
                                  */
                           case SEM_SIGABORT :
                                  /// interruptible_sleep_on(q);
                                  wait_event_interruptible(*q, *cond);
                                  *cond = 0;  /// ATTENTION : 1 seul
                                              /// thread sera reveille
                                  if (signal_pending(current)) return SYSERR;
                                     else                      return OK;
                           case SEM_SIGRETRY :
                                  /* ATTENTION : il faut sans doute que le
                                     driver appelant s'attendre a ce type de
                                     retour. Ce n'est pas le cas en ce moment !
                                     De plus, il est probable que le
                                     comportement de Linux differe de celui de
                                     LynxOS : A VERIFIER ET A ETUDIER !!!
                                     --> Pour l'instant, le driver ASDC
                                         n'utilise jamais SEM_SIGRETRY !
                                  */
                                  /// interruptible_sleep_on(q);
                                  wait_event_interruptible(*q, *cond);
                                  *cond = 0;  /// ATTENTION : 1 seul
                                              /// thread sera reveille
                                  if (signal_pending(current)) return ERESTARTSYS;
                                     else                      return OK;
                         }
                     }


/* Pour effectuer des affichages (debug) */
#define KPRINTF		printk
#define CPRINTF		printk
#define kkprintf	printk
#define cprintf		printk




/* Pour inhiber/restaurer les interruptions */
#define ITFLAGS(x)	unsigned long x
#define DISABLE(x)	{ save_flags(x);   cli(); }
#define RESTORE(x)	restore_flags(x)



/* Attente active */
#define usec_sleep(a)	udelay(a)
                /* Sous Linux, attention a ne pas confondre :      */
                /*                     udelay()  ==> attente en us */
                /*               avec  mdelay()  ==> attente en ms */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26
#include "linux26_irq.h"
#endif


/* Pour eviter les warnings trop nombreux, et en esperant que */
/* l'optimiseur sache se debarasser de ce genre de code ...   */
static inline void bidon(const char * fmt, ...) {}

/* La fonction ioctl */
extern long
asdc_ioctl(int vfxNum, int asdcNum, struct file *fp,
           unsigned int cmd, unsigned long arg);



/* Les sous-fonctions appelees par la fonction ioctl : */
long do_sbs1553_read(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_sbs1553_write(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_IMAGE_LIRE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_IMAGE_ECRIRE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCACCES(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCAVORTE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCRAZ(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCEPAR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLPAR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCGO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCSTOP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCDEF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCECR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLEC(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCMODE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLIREMODE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLECEMEMO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLECMEMO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCALLOUE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLIBERE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCEVT_ABO_AJOUTER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCEVT_ABO_SUPPRIMER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_LIBERER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCEVT_CC_AJOUTER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCEVT_CC_SUPPRIMER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_MF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_MFX(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_GF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_IV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_VV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_ABO_GV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_RT_GETTREP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_RT_SETTREP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_RT_GETSTS(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_RT_SETSTS(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_CC_GETDATA(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_CC_SETDATA(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_SA_GETNBM(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_SA_SETNBM(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_SA_LIER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDC_SA_DELIER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);
long do_ASDCLVER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg);

void isr_asdc(int unit);


/* Les fonctions de gestion des FIFOs (definies dans tampons.c) */
void viderCompletementFifo(fifoTampons_t * fifo);
void viderFifo(fifoTampons_t * fifo);
tampon_t * prendre(void);
void rendre(tampon_t * tampon);
tampon_t * lireFifo(fifoTampons_t * fifo);
tampon_t * lireFifoPrincipale(fifoTampons_t * fifo);
tampon_t * decalerFifo_unlocked(fifoTampons_t * fifo);
void ecrireFifo(fifoTampons_t * fifo, tampon_t * tampon);


/*
	Restent a traiter :
	===================

	- Attentes (delais)	(???)

*/

#endif   /* ASDC_LINUX_INTERFACE_H */

