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
 ************************************************************************/



/**********************/
/* Macros pour LynxOS */
/**********************/

#ifdef LYNXOS

/* Modif. provisoire (?) suite a Pb fonctionnement kkprintf le 20/9/2006 */
#ifdef LYNX_V4
#  define kkprintf cprintf
#endif LYNX_V4

/* Retour de la fonction ioctl */
#define RETURN(x)	do { dst->dphioctl = -1;			  \
			     if (x == OK) { return OK; }		  \
			             else { pseterr(x); return SYSERR; }  \
			   } while(0)


/* Test du transfert Depuis le domaine UTILisateur */
#define TDUTIL(arg, taille)						\
        { if (rbounds((long) arg) < taille)	RETURN(EFAULT); }


/* transfert Depuis le domaine UTILisateur */
#define DUTIL(zone, pzone, arg, taille)					\
        		{ (long) pzone = (long) arg; }


/* Test du transfert Vers le domaine UTILisateur */
#define TVUTIL(zone, pzone, arg, taille)				\
        { if (wbounds((long) arg) < taille)	RETURN(EFAULT);		\
          (long) pzone = (long) arg;					\
        }


/* transfert Vers le domaine UTILisateur */
#define VUTIL(zone, pzone, arg, taille)		{   }


/* Semaphore noyau (pour protection, type mutex) */
#define SEMWAIT(x, y)	swait(x, y)
#define SEMSIGNAL(x)	ssignal(x)


/* Semaphore noyau (pour synchronisation) */
#define SDECLARE(x)	int x
#define SINIT(x)	x = 0
#define SWAIT(x, y)	swait(x, y)
#define SRESET(x)	sreset(x)
#define SWAIT_IM(x, y)	swait(x, y)
#define SRESET_IM(x)	sreset(x)


/* Pour inhiber/restaurer les interruptions */
#define ITFLAGS(x)	long x
#define DISABLE(x)	disable(x)
#define RESTORE(x)	restore(x)


/* Pour effectuer des affichages (debug) */
#define KPRINTF		kkprintf
#define CPRINTF		cprintf

/* Pour eviter les warnings trop nombreux, et en esperant que */
/* l'optimiseur sache se debarasser du code resultant ...     */
#define bidon

#endif		/* LYNXOS */






/*********************/
/* Macros pour Linux */
/*********************/

#ifdef LINUX

/* Constantes definies sous LynxOS, mais pas sous Linux */
#define OK	 0
#define SYSERR  -1


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
#define SDECLARE(x)	wait_queue_head_t x
#define SINIT(x)	init_waitqueue_head(&(x))
#define SRESET(x)	wake_up_interruptible_all(x)

#define SWAIT_IM(x, y)	wq_dormir(dst, x)
/*     -------- TODO ------------                       */
/* L'option y de SWAIT_IM devrait etre prise en compte, */
/* comme dans la definition ci-dessous de SWAIT         */

#define SRESET_IM(x)	wq_reveiller(dst, x)


static inline int SWAIT(wait_queue_head_t *q, int option)
                     {
                       switch (option)
                         { default :
                                 /* C'est un cas d'erreur, mais qui est
                                    difficile � prendre en compte puisque
                                    seules les 2 valeurs de retour OK et
                                    SYSERR sont pr�vues et d�j� utilis�es !
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
                                  wait_event_interruptible(*q,0);
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
                                  wait_event_interruptible(*q,0);
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

#endif          /* LINUX */





/*
	Restent a traiter :
	===================

	- Attentes (delais)	(???)

*/
