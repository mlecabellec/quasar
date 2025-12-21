/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                                                                      *
 *                          Fonction ioctl()                            *
 *                         ==================                           *
 *                                                                      *
 *                                         Y.Guillemot, le 10/09/2002   *
 *                                               modif. le 20/09/2002   *
 *      Ajout des fonctions de debug (LETAT et LTAMPON) le  2/10/2002   *
 *          Correction bug dans lire avec mode CEVT_RAZ le  3/10/2002   *
 *                 Debut traitement compatibilite Linux le  7/10/2002   *
 *                                               modif. le 10/10/2002   *
 *      Ajout du traitement des timers : X. Schimowski, le 14/01/2003   *
 * Ajout gestion de la datation pour les E/S TOR (XS ?) le 22/05/2003   *
 *                                                                      *
 * Fin du portage (avec compatibilite) Linux        YG, le 17/10/2006   *
 * Ajout datation via TSC pour le 1553 (Linux/x86)  YG, le 26/10/2006   *
 *          Suppression des tabulations du source   YG, le 30/11/2010   *
 *      Adapte a kernel recent (> 2.6.18) et EMUABI YG, le 18/04/2013   *
 * Fusion des drivers VFX70, ASDC et CEVT en un                         *
 *                                    seul module : YG, le  8/01/2015   *
 ************************************************************************/




# include <linux/module.h>
# include <linux/sched.h>
# include <linux/kernel.h>
# include <linux/fs.h>
# include <linux/errno.h>
# include <linux/wait.h>
# include <linux/delay.h>
# include <asm/uaccess.h>
# include <linux/kdev_t.h>
# include <linux/init.h>
# include <linux/version.h>
# include <linux/interrupt.h>



#include "cevt_if.h"

#include "cevt.h"       /* Definit les structures de donnees internes */
#include "cevtctl.h"    /* Definit les I/F applications */
#include "cevtvarg.h"   /* Definit les variables statiques */
#include "cevtver.h"    /* Definit le numero de version du pilote */




/* Pour DEBUG */
//#define DEBUG

/* dprintf() peut etre changee en kkprintf, cprintf ou ignoree */
#ifdef DEBUG
#define dprintf         printk
/* #define dprintf      kkprintf */
/* #define dprintf      cprintf */
#else
#define dprintf         bidon
#endif






/* -----------------------------------------------
   --                                           --
   --           cevtioctl                       --
   --                                           --
   ----------------------------------------------- */
long cevtioctl(int vfxNum, int asdcNum, struct file *fp,
               unsigned int cmd, unsigned long arg)
{

// Anciens arguments ("unlocked_ioctl() classique") :
//    (
//       struct inode *inode,
//       struct file *f,
//       unsigned int cmd,
//       unsigned long arg
//    );

 struct cevt_statics *dst;   /* Pointe les donnees statiques du CEVT courant */
 int j;

 unsigned long s;    /* Pour masquage IT pendant sections critiques */

 int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */


// /* Le mineur indique le CEVT  */
// /* (test validite inutile : deja fait dans cevtopen()) */
// int mineur = MINOR(inode->i_rdev);
// dst = cevt_ps[mineur - 1];
//    Inutile : deja fait dans cevtopen() !
// dst = f->private_data;
  dst = NULL;




 switch(cmd)
   {





       case CEVT_INIT :                /* Remise a zero du device */

          dprintf("CEVT_INIT unit=%d\n", arg);
          if (arg > CEVT_MAX_DEVICES) return ENODEV;
          dst = cevt_ps[arg - 1];
          if (dst == NULL) return ENODEV;

          // Pour fonctions de debug...
          dst->dioctl = cmd;
          dst->dphioctl = 1;

          /* Section critique, peut etre pas indispensable */
          /* si utilisation correcte, mais securisante.    */

          /* Debut section critique */
          spin_lock_irqsave(&dst->lock_attente, s);

             /* Initialisation des tampons a "tous disponibles" */
             dst->pbcfev[0].s = NULL;
             for (j=1; j<dst->nb_tampons; j++)
                { dst->pbcfev[j].s = &(dst->pbcfev[j-1]);
                }
             dst->ptv = &(dst->pbcfev[dst->nb_tampons - 1]);
             dst->nb_tamp_dispos = dst->nb_tampons;

             dst->ppt = dst->pdt = NULL;

             /* Reinitialisation du drapeau "RAZ recue (SDC_AVORTE)" */
             dst->raz = 0;

          /* Fin section critique */
          spin_unlock_irqrestore(&dst->lock_attente, s);

          RETURN(OK);








       case CEVT_LETAT : /* Lecture des indicateurs d'etat du pilote */
                        /* (pour debug uniquement)                  */
          { struct cevtetapil *e, ze;

            ze.numero = -1;          /* Pour eviter warning a la compilation */
            TVUTIL(ze, e, arg, sizeof(struct cevtetapil));

            if ((e->numero < 1) || (e->numero > CEVT_MAX_DEVICES)) return ENODEV;
            dst = cevt_ps[e->numero - 1];
            if (dst == NULL) return ENODEV;

            /* Debut section critique */
            spin_lock_irqsave(&dst->lock_attente, s);

                // Pour fonctions de debug...
                dst->dioctl = cmd;
                dst->dphioctl = 1;

                e->nb_tampons = dst->nb_tampons;
                e->pbcfev = dst->pbcfev;
                e->nb_tamp_dispos = dst->nb_tamp_dispos;
                e->ptv = dst->ptv;
                e->ppt = dst->ppt;
                e->pdt = dst->pdt;

                e->numero = dst->numero;
                e->raz = dst->raz;
                e->sel_data = dst->sel_data;

            /* Fin section critique */
            spin_unlock_irqrestore(&dst->lock_attente, s);

            VUTIL(ze, e, arg, sizeof(struct cevtetapil));
            RETURN(OK);
          }








       case CEVT_LVER :  /* Lecture du numero de version du pilote */
          { struct cevtver zv, *v;
            char *p, *q;

            TVUTIL(zv, v, arg, sizeof(struct cevtver));

            /* Copie du nom du pilote */
            p = CEVT_NOM;
            q = v->nom;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie de la date du pilote */
            p = CEVT_DATE;
            q = v->date;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie des version et revision du pilote */
            p = __stringify(CEVT_VERSION) "." __stringify(CEVT_REVISION);
            q = v->version;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie des version et revision sous forme numerique */
            v->ver = CEVT_VERSION;
            v->rev = CEVT_REVISION;

            VUTIL(zv, v, arg, sizeof(struct cevtver));
            RETURN(OK);
          }













     case CEVT_LIRE :
        {
          struct cevt_lire *e, ze;
          int to, mode;
          struct cevt_tfch *p;
          struct cevt_tf lu;
          int nbtamp;

          TDUTIL(arg, sizeof(struct cevt_lire));
          DUTIL(ze, e, arg, sizeof(struct cevt_lire));
          TVUTIL(ze, e, arg, sizeof(struct cevt_lire));

          if ((e->numero < 1) || (e->numero > CEVT_MAX_DEVICES)) return ENODEV;
          dst = cevt_ps[e->numero - 1];
          if (dst == NULL) return EXDEV;

          // Pour fonctions de debug...
          dst->dioctl = cmd;
          dst->dphioctl = 1;

          mode = e->type_evt;
          to = e->id_evt;

          /* Mode valide ? */
          if (    (mode != _CEVT_RAZRAZ)
               && (mode != _CEVT_RAZ)
               && (mode != _CEVT_NONBLOQ)
               && (mode != _CEVT_ATTENDRE)
             )
            {
              /* Mode invalide */
              RETURN(ENXIO);
            }


            /* Debut section critique */
            spin_lock_irqsave(&dst->lock_attente, s);


          /* Doit-on effectuer une RAZ ? */
          if ((mode == _CEVT_RAZ) || (mode == _CEVT_RAZRAZ))
            {
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

              if (mode == _CEVT_RAZRAZ)		/* Alors, retour a l'appli */
                {
                  /* Fin section critique */
                  spin_unlock_irqrestore(&dst->lock_attente, s);

                  RETURN(OK);
                }
            }



          /* Y a-t-il un tampon disponible ? */
          if ((dst->ppt == NULL) && (dst->raz == 0))
            {
              /* Pas de tampons disponibles ! */

              /* Mode attente ? */
              if (mode == _CEVT_NONBLOQ)
                {
                  /* Fin section critique */
                  spin_unlock_irqrestore(&dst->lock_attente, s);

                  RETURN(ENOSR);
                }

              /* RAZ condition */
              dst->cond_attente = 0;

              /* Fin section critique */
              spin_unlock_irqrestore(&dst->lock_attente, s);

              /* Evenement attendu non encore arrive ==> attente */
              tmpi = TSWAIT(&dst->sem_attente, &dst->cond_attente, SEM_SIGABORT, to);

              if (tmpi)         /* Attente interrompue */
                {
                  switch (tmpi)
                    {
                      case TSWAIT_NOTOUTS :
                            /* Plus de timer disponible pour time-out */
                            RETURN(EAGAIN);

                      case TSWAIT_ABORTED :
                            /* Interruption par un signal */
                            RETURN(EINTR);

                      case TSWAIT_TIMEDOUT :
                            /* Interruption par time-out */
                            RETURN(ETIMEDOUT);

                      default :
                            /* Erreur interne */
                            kkprintf("CEVTLIRE : retour tswait = 0x%X\n", tmpi);
                            RETURN(EIO);

                    }
                }


            /* Debut section critique */
            spin_lock_irqsave(&dst->lock_attente, s);

              if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
                {
                 dst->raz = 0;

                 /* Mise a jour nombre des donnees dispo pour select */
                 dst->sel_data =
                            dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;


                  /* Fin section critique */
                  spin_unlock_irqrestore(&dst->lock_attente, s);

                  RETURN(ENETRESET);
                }

              /* Un tampon DOIT etre disponible ! */
              if (dst->ppt == NULL)
                {
                  /* Et pourtant, aucun tampon dispo n'est trouve */
                  /*    ==> Erreur interne ???                    */

                  /* Fin section critique */
                  spin_unlock_irqrestore(&dst->lock_attente, s);


                  kkprintf(
                     "CEVTLIRE : fin normale tswait, mais pas de tampon !\n");
                  RETURN(EIO);
                }

            }



          /* Une RAZ (CEVT_AVORTE) a-t-elle eu lieu ? */
          if (dst->raz)
            {
              dst->raz = 0;

              /* Mise a jour du nombre des donnees dispo pour select */
              dst->sel_data = dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;


              /* Fin section critique */
              spin_unlock_irqrestore(&dst->lock_attente, s);

              RETURN(ENETRESET);
            }


          /* Lecture du tampon */
          p = dst->ppt;
          lu = p->tamp;

          /* Retrait du tampon de la liste */
          dst->ppt = p->s;
          if (p->s == NULL) dst->pdt = NULL;
          dst->nb_tamp_dispos++;

          /* Et reinsertion dans la liste des tampons disponibles */
          p->s = dst->ptv;
          dst->ptv = p;

          /* Mise a jour du nombre des donnees dispo pour select */
          nbtamp = dst->nb_tampons - dst->nb_tamp_dispos;
          dst->sel_data = nbtamp + dst->raz;

          /* Fin section critique */
          spin_unlock_irqrestore(&dst->lock_attente, s);


          /* Mise en forme des resultats */
          e->type_evt = lu.type;

          switch (lu.type)
            {
            case CEVT_ETOR :
            case CEVT_STOR :
              /* recopie specifique de la date pour les TOR */
              e->date[0] = lu.date[0];
              e->date[1] = lu.date[1];

            case CEVT_TIMER :
              e->id_evt = lu.src;
              e->donnee = lu.d2;
              break;

            case CEVT_SDCABO :
              e->numero = lu.src;
              e->id_evt = lu.d1;
              e->bus = lu.bus;
              e->donnee = lu.d2;

              /* recopie specifique de la datation TSC si Linux */
              e->tsc = lu.tsc;

              break;

            case CEVT_EXT :
              e->id_evt = lu.d1;
              e->donnee = lu.d2;
              break;

            case CEVT_DEBORD :
              break;

            default :
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




     case CEVT_LTAMPON :                /* Pour debug ... */
        {
          /* Pour debug : lecture d'un tampon d'une FIFO designe par */
          /* son adresse dans le contexte du noyau                   */

          struct cevt_tampon ze, *e;
          struct cevt_tfch *p;

          TDUTIL(arg, sizeof(struct cevt_tfch));
          DUTIL(ze, e, arg, sizeof(struct cevt_tfch));
          TVUTIL(ze, e, arg, sizeof(struct cevt_tfch));

          // Pour fonctions de debug...
          dst->dioctl = cmd;
          dst->dphioctl = 1;

          if ((e->numero < 1) || (e->numero > CEVT_MAX_DEVICES)) return ENODEV;
          dst = cevt_ps[e->numero - 1];
          if (dst == NULL) return ENODEV;

          p = e->tfch.s;

          if (p == NULL) RETURN(ENOENT);                /* Plus de tampons */


       //   if (rbounds((long) p) < sizeof(struct cevt_tfch))
       //                               RETURN(ENXIO);  /* Mauvais pointeur */
       // Test ci-dessus ne fonctionne pas (rbounds renvoi 0 !!!)
       //    ==> Pour bien faire, il faudrait tester la valeur de p
       //        par rapport aux adresses des elements du tableau pbcfef[] ...

          /* Copie du tampon */
          e->tfch = *p;

          VUTIL(ze, e, arg, sizeof(struct cevt_tfch));
          RETURN(OK);
        }






     case CEVT_SIGNALER :
        {
          struct cevt_signal za, *a;

          TDUTIL(arg, sizeof(struct cevt_signal));
          DUTIL(za, a, arg, sizeof(struct cevt_signal));

          if ((a->numero < 1) || (a->numero > CEVT_MAX_DEVICES)) return ENODEV;
          dst = cevt_ps[a->numero - 1];
          if (dst == NULL) return ENODEV;

          // Pour fonctions de debug...
          dst->dioctl = cmd;
          dst->dphioctl = 1;

          cevt_signaler_impl(a->numero, CEVT_EXT, 0, 0, a->d1, a->d2);
        }





     /* Renvoi le nombre de CEVT instancies dans le driver */
     case CEVT_GETNOMBRE :
        {
          int i;
          for (i=0; i < CEVT_MAX_DEVICES; i++) {
              if (cevt_ps[i] == NULL) break;
          }

          return i;
        }





//      case CEVT_EXPORTER :
//         {
//           short mot;
//           struct cevt_export za, *a;
// 
//           TVUTIL(za, a, arg, sizeof(struct cevt_export));
// 
//           a->cevt_signaler      = cevt_signaler_impl;
//           a->cevt_signaler_date = cevt_signaler_date_impl;
//           a->cevt_existence     = cevt_existence_impl;
// 
//           cprintf("CEVT%d : Export des fonctions CEVT\n", dst->numero);
// 
//           VUTIL(za, a, arg, sizeof(struct cevt_export));
//           RETURN(OK);
//         }



     default :
          printk("VFX70(%d) - ASDC(%d) - CEVT : ioctl(0x%X) "
                 "non definie !\n",vfxNum, asdcNum, cmd);
          RETURN(EINVAL);

   }

   cprintf("cevtioctl : Ce message n'aurait jamais du etre ecrit !\n");
   RETURN(ENOTTY);
}




