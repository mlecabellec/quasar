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
 *        Adapte a kernel recent (> 2.6.18) et EMUABI : le 18/04/2013   *
 *  Correction d'une erreur sur detection du time out : le 25/04/2013   *
 ************************************************************************/

/*********************/
/* Macros pour Linux */
/*********************/

/* Constantes definies sous LynxOS, mais pas sous Linux */
#define OK	 0
#define SYSERR  -1


/* Retour de la fonction ioctl */
/*
  #define RETURN(x)        do { int i=0; dst->dphioctl = -1;      \
                                printk("CEVT RETURN %d\n", i++);  \
                                return -(x);                      \
                              } while(0)

  YG, le 16/4/2013
  Linux kernel 2.6.18-164.2.1.el5 (Scientific Linux) sur i686 SMP (32 bits)
  gcc 4.1.2 20080704 (Red Hat 4.1.2-46)
  La macro RETURN ci-dessus bloque le systeme !!!!
  (pas d'explication pour le moment...)
*/
#define RETURN(x)     return -(x)


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






/* Semaphore noyau (pour synchronisation) */

//#define SWAIT_IM(x, y)	wq_dormir(dst, x)
/*     -------- TODO ------------                       */
/* L'option y de SWAIT_IM devrait etre prise en compte, */
/* comme dans la definition ci-dessous de SWAIT         */

//#define SRESET_IM(x)	wq_reveiller(dst, x)


/* Options des semaphores (definies dans LynxOS) */

#define SEM_SIGIGNORE	   0
#define SEM_SIGABORT	   1
#define SEM_SIGRETRY	   2

#define TSWAIT_NOTOUTS	0x10
#define TSWAIT_ABORTED	0x11
#define TSWAIT_TIMEDOUT	0x12

/*
 * Renvoi OK si condition s'est produite, TSWAIT_ABORTED si un signal
 * a interrompu l'attente et TSWAIT_TIMEDOUT si le time_out s'est produit.
 *
 * Le time_out est exprime en jiffies.
 */
static inline int TSWAIT(wait_queue_head_t *q,  int *cond, int option, int time_out)
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
            if (time_out > 0) {
                time = wait_event_interruptible_timeout(*q, *cond, time_out);
            } else {
                time = wait_event_interruptible(*q, *cond);
            }
            if (time == -ERESTARTSYS) return TSWAIT_ABORTED /* SYSERR */;
                else if ((time == 0) && (time_out > 0))  return TSWAIT_TIMEDOUT;
                else    { *cond = 0;  return OK; }
                           /// ATTENTION : 1 seul thread sera reveille
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
            if (time_out > 0) {
                time = wait_event_interruptible_timeout(*q, *cond, time_out);
            } else {
                time = wait_event_interruptible(*q, *cond);
            }
            if (time == -ERESTARTSYS) return TSWAIT_ABORTED /* ERESTARTSYS */;
                else if ((time == 0) && (time_out > 0))  return TSWAIT_TIMEDOUT;
                else    { *cond = 0;  return OK; }
                           /// ATTENTION : 1 seul thread sera reveille
    }
}




/* Pour effectuer des affichages (debug) */
#define KPRINTF		printk
#define CPRINTF		printk
#define kkprintf	printk
#define cprintf		printk

/* Pour supprimer certains warnings lies aux macros d'affichage ci-dessus */
inline static void bidon(const char * fmt, ...) { }



