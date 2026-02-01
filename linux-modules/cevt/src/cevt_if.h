/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                                                                      *
 *             Macros d'interfacage, pour utilisation du                *
 *             meme code source sous LynxOS ou sous Linux               *
 *                                                                      *
 *                         ==================                           *
 *                                                                      *
 *                                         Y.Guillemot, le  3/10/2002   *
 *                                          pour v1.1 : le  3/10/2002   *
 *                                          pour v1.4 : le 18/10/2006   *
 *   Inclusion conditionnelle de linux26_irq.h (v1.5) : le  9/01/2008   *
 *                   Ajout time-out sous Linux (v1.8) : le 30/11/2010   *
 ************************************************************************/



/**********************/
/* Macros pour LynxOS */
/**********************/

#ifdef LYNXOS

/* Retour de la fonction ioctl */
#define RETURN(x)    do { dst->dphioctl = -1;				\
			  if (x == OK) { return OK; }			\
			          else { pseterr(x); return SYSERR; }	\
			} while(0)


/* Test du transfert Depuis le domaine UTILisateur */
#define TDUTIL(arg, taille)						\
        do { if (rbounds((long) arg) < taille)	RETURN(EFAULT); } while(0)


/* transfert Depuis le domaine UTILisateur */
#define DUTIL(zone, pzone, arg, taille)					\
        		do { (long) pzone = (long) arg; } while(0)


/* Test du transfert Vers le domaine UTILisateur */
#define TVUTIL(zone, pzone, arg, taille)				\
        do { if (wbounds((long) arg) < taille)	RETURN(EFAULT);		\
             (long) pzone = (long) arg;					\
           } while(0)


/* transfert Vers le domaine UTILisateur */
#define VUTIL(zone, pzone, arg, taille)		do {   } while(0)


/* Semaphore noyau (pour protection, type mutex) */
#define SEMWAIT(x, y)	swait(x, y)
#define SEMSIGNAL(x)	ssignal(x)


/* Semaphore noyau (pour synchronisation) */
#define STYPE		int
#define SINIT(x)	x = 0
#define SWAIT(x, y)	swait(x, y)
#define TSWAIT(x, y)	tswait(x, y)
#define SRESET(x)	sreset(x)
#define SSIGNAL(x)	ssignal(x)


/* Pour inhiber/restaurer les interruptions */interface.h:
#define ITFLAGS		int
#define DISABLE(x)	disable(x)
#define RESTORE(x)	restore(x)


/* Pour effectuer des affichages (debug) */
#define KPRINTF		kkprintf
#define CPRINTF		cprintf


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
#define TDUTIL(arg, taille)	do {   } while(0)


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
#define SEMWAIT(x, y)	do { } while(0)
#define SEMSIGNAL(x)	do { } while(0)


/* Semaphore noyau (pour synchronisation) */
#define STYPE		wait_queue_head_t
#define SINIT(x)	init_waitqueue_head(&(x))
#define SRESET(x)	wake_up_interruptible(x)

#define SSIGNAL(x)	wake_up_interruptible(x)
                        /* Adaptation approximatrive, pour select()   */
                        /* Sans doute sans probleme ici, mais devrait */
                        /* etre etudie de facon plus approfondie...   */

#define SWAIT_IM(x, y)	wq_dormir(dst, x)
/*     -------- TODO ------------                       */
/* L'option y de SWAIT_IM devrait etre prise en compte, */
/* comme dans la definition ci-dessous de SWAIT         */

#define SRESET_IM(x)	wq_reveiller(dst, x)


/* Options des semaphores (definies dans LynxOS) */

#define SEM_SIGIGNORE	   0
#define SEM_SIGABORT	   1
#define SEM_SIGRETRY	   2

#define TSWAIT_NOTOUTS	0x10
#define TSWAIT_ABORTED	0x11
#define TSWAIT_TIMEDOUT	0x12

static inline int SWAIT(wait_queue_head_t *q, int option)
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

/* TSWAIT : Idem SWAIT, mais avec time-out (exprime en jiffies) */
static inline int TSWAIT(wait_queue_head_t *q, int option, int time_out)
{
  long time;

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
            ///interruptible_sleep_on(q);
            if (time_out > 0) {
                time = wait_event_interruptible_timeout(*q, 0, time_out);
            } else {
                time = 1;
                wait_event_interruptible(*q, 0);
            }
            if (signal_pending(current)) return TSWAIT_ABORTED /* SYSERR */;
                else if (time == 0)      return TSWAIT_TIMEDOUT;
                else                     return OK;
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
            ///interruptible_sleep_on(q);
            if (time_out > 0) {
                time = wait_event_interruptible_timeout(*q, 0, time_out);
            } else {
                time = 1;
                wait_event_interruptible(*q, 0);
            }
            if (signal_pending(current)) return TSWAIT_ABORTED /* ERESTARTSYS */;
                else if (time == 0)      return TSWAIT_TIMEDOUT;
                else                     return OK;
    }
}


/* Les fonctions (ou macros) utilisees par les macros ci-dessus  */
/* n'existent plsu dans certaines versions recentes du noyau.    */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26
#include "linux26_irq.h"
#endif



/* Pour effectuer des affichages (debug) */
#define KPRINTF		printk
#define CPRINTF		printk
#define kkprintf	printk
#define cprintf		printk


#endif		/* LINUX */


