/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                                                                      *
 *                          Fonction ioctl()                            *
 *                         ==================                           *
 *                                                                      *
 *                                         Y.Anonymized, le 10/09/2002   *
 *                                               modif. le 20/09/2002   *
 *      Ajout des fonctions de debug (LETAT et LTAMPON) le  2/10/2002   *
 *          Correction bug dans lire avec mode CEVT_RAZ le  3/10/2002   *
 *                 Debut traitement compatibilite Linux le  7/10/2002   *
 *                                               modif. le 10/10/2002   *
 *      Ajout du traitement des timers : X. *********, le 14/01/2003   *
 * Ajout gestion de la datation pour les E/S TOR (XS ?) le 22/05/2003   *
 *                                                                      *
 * Fin du portage (avec compatibilite) Linux        YG, le 17/10/2006   *
 * Ajout datation via TSC pour le 1553 (Linux/x86)  YG, le 26/10/2006   *
 *          Suppression des tabulations du source   YG, le 30/11/2010   *
 *                    Adatation au kernel 3.3.6 :   YG, le  8/04/2013   *
 ************************************************************************/

#define KERNEL

#ifdef LYNXOS

#include <conf.h>
#include <dldd.h>
#include <errno.h>
#include <io.h>
#include <kernel.h>
#include <mem.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <time.h>

#elif LINUX

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/wait.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#endif

#else

#error "L'un au moins des 2 symboles LINUX ou LYNXOS doit etre defini"

#endif /* LynxOS - Linux */

#include "cevt_if.h"

#include "cevt.h"     /* Definit les structures de donnees internes */
#include "cevtctl.h"  /* Definit les I/F applications */
#include "cevtvarg.h" /* Definit les variables statiques */
#include "cevtver.h"  /* Definit le numero de version du pilote */

/* Pour DEBUG */

/* dprintf() peut etre changee en kkprintf, cprintf ou ignoree */
#ifdef DEBUG
#define dprintf kkprintf
/* #define dprintf      cprintf */
#else
#define dprintf
#endif

/* -----------------------------------------------
   --                                           --
   --           cevtioctl                       --
   --                                           --
   ----------------------------------------------- */
#ifdef LYNXOS
int cevtioctl(struct cevt_statics *lynx_statics, struct file *f, int commande,
              char *arg)
#elif LINUX
#ifdef OLDIOCTL
int cevtioctl(struct inode *inode, struct file *f, unsigned int commande,
              unsigned long arg)
#else
long cevtioctl(struct file *f, unsigned int commande, unsigned long arg)
#endif
#endif /* LYNXOS - LINUX */
{

  struct cevt_statics *dst; /* Pointe les donnees statiques du CEVT courant */
  int j;

  unsigned long s; /* Pour masquage IT pendant section critique */

  int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */

#ifdef LYNXOS
  /* Le mineur n'est pas utilise */
  dst = lynx_statics;
#elif LINUX

  // /* Le mineur indique le CEVT  */
  // /* (test validite inutile : deja fait dans cevtopen()) */
  // int mineur = MINOR(inode->i_rdev);
  // dst = cevt_ps[mineur - 1];
  //    Inutile : deja fait dans cevtopen() !
  dst = f->private_data;

#endif

  dst->dioctl = commande;
  dst->dphioctl = 1;

  switch (commande) {

  case CEVT_INIT: /* Remise a zero du device */

    dprintf("CEVT_INIT unit=%d\n", dst->numero);

    /* Section critique, peut etre pas indispensable */
    /* si utilisation correcte, mais securisante.    */

    spin_lock_irqsave(&dst->lock_attente, s); /* Debut section critique */

    /* Initialisation des tampons a "tous disponibles" */
    dst->pbcfev[0].s = NULL;
    for (j = 1; j < dst->nb_tampons; j++) {
      dst->pbcfev[j].s = &(dst->pbcfev[j - 1]);
    }
    dst->ptv = &(dst->pbcfev[dst->nb_tampons - 1]);
    dst->nb_tamp_dispos = dst->nb_tampons;

    dst->ppt = dst->pdt = NULL;

    /* Reinitialisation du drapeau "RAZ recue (SDC_AVORTE)" */
    dst->raz = 0;

    spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */

    RETURN(OK);

  case CEVT_LETAT: /* Lecture des indicateurs d'etat du pilote */
                   /* (pour debug uniquement)                  */

    {
      struct cevtetapil *e, ze;

      TVUTIL(ze, e, arg, sizeof(struct cevtetapil));

      e->nb_tampons = dst->nb_tampons;
      e->pbcfev = dst->pbcfev;
      e->nb_tamp_dispos = dst->nb_tamp_dispos;
      e->ptv = dst->ptv;
      e->ppt = dst->ppt;
      e->pdt = dst->pdt;

      e->numero = dst->numero;
      e->raz = dst->raz;
      e->sel_data = dst->sel_data;

      VUTIL(ze, e, arg, sizeof(struct cevtetapil));
      RETURN(OK);
    }

  case CEVT_LVER: /* Lecture du numero de version du pilote */
  {
    struct cevtver zv, *v;
    char *p, *q;

    TVUTIL(zv, v, arg, sizeof(struct cevtver));

    /* Copie du nom du pilote */
    p = CEVT_NOM;
    q = v->nom;
    while (*p) {
      *q++ = *p++;
    }
    *q = '\0';

    /* Copie de la version du pilote */
    p = CEVT_VERSION;
    q = v->version;
    while (*p) {
      *q++ = *p++;
    }
    *q = '\0';

    /* Copie de la date du pilote */
    p = CEVT_DATE;
    q = v->date;
    while (*p) {
      *q++ = *p++;
    }
    *q = '\0';

    /* Copie du numero du CEVT */
    v->numero = dst->numero;

    VUTIL(zv, v, arg, sizeof(struct cevtver));
    RETURN(OK);
  }

  case CEVT_LIRE: {
    struct cevt_lire *e, ze;
    int to, mode;
    struct cevt_tfch *p;
    struct cevt_tf lu;
    int nbtamp;

    TDUTIL(arg, sizeof(struct cevt_lire));
    DUTIL(ze, e, arg, sizeof(struct cevt_lire));
    TVUTIL(ze, e, arg, sizeof(struct cevt_lire));

    mode = e->type_evt;
    to = e->id_evt;

    /* Valeur de time-out compatible avec besoin de tswait() */
    if (to <= 0)
      to = -1;

    /* Mode valide ? */
    if ((mode != _CEVT_RAZRAZ) && (mode != _CEVT_RAZ) &&
        (mode != _CEVT_NONBLOQ) && (mode != _CEVT_ATTENDRE)) {
      /* Mode invalide */
      RETURN(EINVAL);
    }

    spin_lock_irqsave(&dst->lock_attente, s); /* Debut section critique */

    /* Doit-on effectuer une RAZ ? */
    if ((mode == _CEVT_RAZ) || (mode == _CEVT_RAZRAZ)) {
      /* Deplacement de tous les tampons pleins dans */
      /* la chaine des tampons disponibles           */

      if (dst->pdt != NULL) /* Sinon, rien a faire ! */
      {
        /* Deplacement de tous les tampons pleins dans */
        /* la chaine des tampons disponibles           */
        dst->pdt->s = dst->ptv;
        dst->ptv = dst->ppt;
        dst->ppt = dst->pdt = NULL;

        /* Mise a jour des compteurs de tampons */
        dst->nb_tamp_dispos = dst->nb_tampons;
      }

      /* Effacement des eventuelles indications de reset */
      dst->raz = 0;

      /* Mise a jour nombre des donnees dispo pour select */
      dst->sel_data = 0;

      if (mode == _CEVT_RAZRAZ) /* Alors, retour a l'appli */
      {
        spin_unlock_irqrestore(&dst->lock_attente,
                               s); /* Fin section critique */
        RETURN(OK);
      }
    }

    /* Y a-t-il un tampon disponible ? */
    if ((dst->ppt == NULL) && (dst->raz == 0)) {
      /* Pas de tampons disponibles ! */

      /* Mode attente ? */
      if (mode == _CEVT_NONBLOQ) {
        spin_unlock_irqrestore(&dst->lock_attente,
                               s); /* Fin section critique */
        RETURN(ENOSR);
      }

      spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */

      /* Evenement attendu non encore arrive ==> attente */
      tmpi = TSWAIT(&dst->sem_attente, SEM_SIGABORT, to);

      if (tmpi) /* Attente interrompue */
      {
        switch (tmpi) {
        case TSWAIT_NOTOUTS:
          /* Plus de timer disponible pour time-out */
          RETURN(EAGAIN);

        case TSWAIT_ABORTED:
          /* Interruption par un signal */
          RETURN(EINTR);

        case TSWAIT_TIMEDOUT:
          /* Interruption par time-out */
          RETURN(ETIMEDOUT);

        default:
          /* Erreur interne */
          kkprintf("CEVTLIRE : retour tswait = 0x%X\n", tmpi);
          RETURN(EIO);
        }
      }

      spin_lock_irqsave(&dst->lock_attente, s);

      if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
      {
        dst->raz = 0;

        /* Mise a jour nombre des donnees dispo pour select */
        dst->sel_data = dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;

        spin_unlock_irqrestore(&dst->lock_attente,
                               s); /* Fin section critique */
        RETURN(ENETRESET);
      }

      /* Un tampon DOIT etre disponible ! */
      if (dst->ppt == NULL) {
        /* Et pourtant, aucun tampon dispo n'est trouve */
        /*    ==> Erreur interne ???                    */

        spin_unlock_irqrestore(&dst->lock_attente,
                               s); /* Fin section critique */

        kkprintf("CEVTLIRE : fin normale tswait, mais pas de tampon !\n");
        RETURN(EIO);
      }
    }

    /* Une RAZ (CEVT_AVORTE) a-t-elle eu lieu ? */
    if (dst->raz) {
      dst->raz = 0;

      /* Mise a jour du nombre des donnees dispo pour select */
      dst->sel_data = dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;

      spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */
      RETURN(ENETRESET);
    }

    /* Lecture du tampon */
    p = dst->ppt;
    lu = p->tamp;

    /* Retrait du tampon de la liste */
    dst->ppt = p->s;
    if (p->s == NULL)
      dst->pdt = NULL;
    dst->nb_tamp_dispos++;

    /* Et reinsertion dans la liste des tampons disponibles */
    p->s = dst->ptv;
    dst->ptv = p;

    /* Mise a jour du nombre des donnees dispo pour select */
    nbtamp = dst->nb_tampons - dst->nb_tamp_dispos;
    dst->sel_data = nbtamp + dst->raz;

    spin_unlock_irqrestore(&dst->lock_attente,
                           s); /* Fin de la section critique */

    /* Mise en forme des resultats */
    e->type_evt = lu.type;

    switch (lu.type) {
    case CEVT_ETOR:
    case CEVT_STOR:
      /* recopie specifique de la date pour les TOR */
      e->date[0] = lu.date[0];
      e->date[1] = lu.date[1];

    case CEVT_TIMER:
      e->id_evt = lu.src;
      e->donnee = lu.d2;
      break;

    case CEVT_SDCABO:
      e->id_evt = ((lu.src << 16) & 0xFFFF0000) | (lu.d1 & 0x0000FFFF);
      e->donnee = lu.d2;
#ifdef LINUX
      /* recopie specifique de la datation TSC si Linux */
      e->tsc = lu.tsc;
#endif /* LINUX */
      break;

    case CEVT_EXT:
      e->id_evt = lu.d1;
      e->donnee = lu.d2;
      break;

    case CEVT_DEBORD:
      break;

    default:
      e->id_evt = lu.src;
      kkprintf("CEVTLIRE : Type evt 0x%X anormal !\n", lu.type);
      RETURN(EIO);
    }

    /* Nombre de tampons qui restent a lire */
    e->nbtamp = nbtamp;

    /* Fin normale de CEVT_LIRE */
    VUTIL(ze, e, arg, sizeof(struct cevt_lire));
    RETURN(OK);
  }

  case CEVT_LTAMPON: /* Pour debug ... */
  {
    /* Pour debug : lecture d'un tampon d'une FIFO designe par */
    /* son adresse dans le contexte du noyau                   */

    struct cevt_tfch ze, *e, *p;

    TDUTIL(arg, sizeof(struct cevt_tfch));
    DUTIL(ze, e, arg, sizeof(struct cevt_tfch));
    TVUTIL(ze, e, arg, sizeof(struct cevt_tfch));

    p = e->s;

    if (p == NULL)
      RETURN(ENOENT); /* Plus de tampons */

    //   if (rbounds((long) p) < sizeof(struct cevt_tfch))
    //                               RETURN(ENXIO);  /* Mauvais pointeur */
    // Test ci-dessus ne fonctionne pas (rbounds renvoi 0 !!!)
    //    ==> Pour bien faire, il faudrait tester la valeur de p
    //        par rapport aux adresses des elements du tableau pbcfef[] ...

    /* Copie du tampon */
    *e = *p;

    VUTIL(ze, e, arg, sizeof(struct cevt_tfch));
    RETURN(OK);
  }

  case CEVT_SIGNALER: {
    struct cevt_signal za, *a;

    TDUTIL(arg, sizeof(struct cevt_signal));
    DUTIL(za, a, arg, sizeof(struct cevt_signal));

    cevt_signaler(dst->numero, CEVT_EXT, 0, a->d1, a->d2);

    RETURN(OK);
  }

#ifdef SUPPORT_ESIM
/*
 * Support Eurosim :
 * - OS_IOCTL_WAITINT : attente bloquante jusqu'à disponibilité d'un événement.
 * - OS_IOCTL_BREAKWAITINT : debloquer attente s'il y en a une.
 */
#define OS_IOCTL_WAITINT (95)
#define OS_IOCTL_BREAKWAITINT (96)

  case OS_IOCTL_WAITINT:                      /* Attente d'un evenement */
    spin_lock_irqsave(&dst->lock_attente, s); /* Debut section critique */

    /* Y a-t-il un tampon disponible ? */
    if ((dst->ppt == NULL) && (dst->raz == 0)) {
      spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */
      SWAIT(&dst->sem_attente, SEM_SIGABORT);
    } else {
      spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */
    }
    RETURN(OK); /* Fin section critique */

  case OS_IOCTL_BREAKWAITINT:                 /* Stopper l'attente */
    spin_lock_irqsave(&dst->lock_attente, s); /* Debut section critique */
    SRESET(&dst->sem_attente);
    spin_unlock_irqrestore(&dst->lock_attente, s); /* Fin section critique */
    RETURN(OK);                                    /* Fin section critique */
#endif                                             // SUPPORT_ESIM

  default:
    RETURN(EINVAL);
  }

  cprintf("cevtioctl : Ce message n'aurait jamais du etre ecrit !\n");
  RETURN(ENOTTY);
}
