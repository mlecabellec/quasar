/*
** asdcwqlinux.c for Drivers Linux pour SBS1553 in /home/alex
**
** Made by BEAUGY Alexandre
** Login   <alex@epita.fr>
**
** Started on  Mon Oct 28 10:21:22 2002 BEAUGY Alexandre
** Last update Mon Nov 18 14:09:24 2002 BEAUGY Alexandre
**
**	YG, le 21/1/2003
**
** Ajout include <linux/module.h> et <linux/version.h> pour compilation
** avec noyau 2.6
**      YG, le 26/10/2004
**
** Reecriture complete (car mauvais fonctionnement manifeste)
**                                             Y. Guillemot, le  8/09/2005
**
** Ajout #include <linux/sched.h>              Y. Guillemot, le 28/11/2007
**
** Ajout compatibilite Linux 64 bits           Y. Guillemot, le 12/01/2009
**
** Rassemblement des includes dans un seul fichier       YG, le 16/06/2014
*/


#include "driverIncludes.h"
# if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0) 
#  include <linux/vmalloc.h>
# endif


/* Creation d'un pool de nb_wq wait queues */
/* Renvoi valeur non nulle si OK */
int wq_creer(struct asdc_varg *dst)
{
  int i, nbwq;
  struct wq_liste *liste;

  nbwq = dst->nombre_wait_queues;

  /* Initialisation du lock */
  spin_lock_init(&dst->wq_lock);

  /* Allocation liste en memoire */
  liste = (struct wq_liste *) vmalloc(nbwq * sizeof(struct wq_liste));
  if (liste == NULL)
    {
      KPRINTF("ASDC: Echec d'allocation memoire de la liste de ");
      KPRINTF("\"wait_queue\" !!nb!\n");
      return 0;
    }

  /* Initialisation des pointeurs statiques */
  dst->wq_base = liste;
  dst->wq_libres = &liste[nbwq-1];
  dst->wq_occ_debut = NULL;
  dst->wq_occ_fin = NULL;

  /* Chainage des elements (les seuls pointeurs s sont necessaires ici) */
  for (i=nbwq-1; i; i--) liste[i].s = &(liste[i-1]);
  liste[0].s = NULL;

  /* Initialisation des wait queues elementaires */
  for (i=0; i<nbwq; i++)
    { init_waitqueue_head(&liste[i].wq);
      liste[i].nbre = 0;
      liste[i].p = NULL;
    }

  /* Tout s'est bien passe */
  return 1;
}


/* Reinitialisation de la chaine des wait queues */
/* (pour utilitaire asdcinit, via ioctl(ASDCRAZ) seulement */
void wq_init(struct asdc_varg *dst)
{
  int i, nbwq;
  struct wq_liste *pwq, *liste;

  nbwq = dst->nombre_wait_queues;

  /* ATTENTION : les reveils ci-dessous ont deja ete faits  */
  /*             en parcourant les structures mem/image ... */
  for (pwq=dst->wq_occ_debut; pwq; pwq=pwq->s)
    { /* Reveil d'eventuels processus endormis */
      wake_up_interruptible(&pwq->wq);
    }

  /* Ci-dessous, on voudrait ignorer totalement wq_lock : on  suppose  */
  /* que cette fonction est appelee a bon escient et au bon moment ... */
  /*                                                                   */
  /* Cependant, si par malchance on appelle la fonction pendant que le */
  /* lock est ferme, on risque de perturber la modification des        */
  /* chainage en cours dans un autre thread.                           */
  /*   ==> C'est pourquoi on utilise le lock ici (sans pouvoir         */
  /*       affirmer que cette utilisation ait un reel interet)         */


  spin_lock(&dst->wq_lock);

    /* Remise de la liste dans son etat initial */
    liste = dst->wq_base;
    liste[0].s = NULL;
    liste[0].nbre = 0;
    liste[0].p = NULL;
    for (i=nbwq-1; i; i--)
      { liste[i].s = &(liste[i-1]);
        liste[i].nbre = 0;
        liste[i].p = NULL;
      }

  spin_unlock(&dst->wq_lock);
}


/* Selection d'une wait queue disponible */
struct wq_liste * wq_prendre(struct asdc_varg *dst)
{
  struct wq_liste *pwq;

  if (dst->wq_libres == NULL)
    {
      KPRINTF("ASDC: Plus de wait queue disponible !\n");
      return NULL;
    }


  spin_lock(&dst->wq_lock);

    /* Retrait de la premiere entree dans la liste des wq disponibles */
    pwq = dst->wq_libres;
    dst->wq_libres = pwq->s;

    /* Insertion dans la liste des wq utilisees */
    pwq->nbre = 0;
    pwq->cond = 0;
    pwq->p = NULL;
    if (dst->wq_occ_debut)
      { pwq->s = dst->wq_occ_debut;
        dst->wq_occ_debut->p = pwq;
        dst->wq_occ_debut = pwq;
      }
    else
      { pwq->s = NULL;
        dst->wq_occ_debut = pwq;
        dst->wq_occ_fin = pwq;
      }

  spin_unlock(&dst->wq_lock);

  return pwq;
}


/* Retour d'une wait queue dans la liste des wait queues disponibles */
void wq_rendre(struct asdc_varg *dst, struct wq_liste * wq)
{
  struct wq_liste *pwq;

  if (wq == NULL)
    { KPRINTF("wq_rendre : wq = NULL !!!\n");
      return;
    }

  spin_lock(&dst->wq_lock);

    /* Retrait de l'element de la liste des wq utilisees */
    pwq = wq->s;
    if (pwq != NULL) pwq->p = wq->p;
                else dst->wq_occ_fin = wq;

    pwq = wq->p;
    if (pwq != NULL) pwq->s = wq->s;
                else dst->wq_occ_debut = wq;

    /* Insertion de l'element dans la liste des wq libres */
    wq->p = NULL;
    wq->nbre = 0;
    wq->s = dst->wq_libres;
    dst->wq_libres = wq;

  spin_unlock(&dst->wq_lock);
}


/* Attente d'un evenement sur une wait queue */
int wq_dormir(struct asdc_varg *dst, long *lynx_sem)
{
  int nbre_restant, cr;
  struct wq_liste *wq;

  /* TODO : Il manque ici une protection (semaphore ?, autre chose ?) */

  if (*lynx_sem == (long) NULL)
    { wq = wq_prendre(dst);
      if (wq == NULL)
        { KPRINTF("wq_dormir : Plus de waitqueues disponiblkes !\n");
          return -SYSERR;
        }
      *lynx_sem = (long) wq;
    }
  else
    {  wq = (struct wq_liste *) *lynx_sem;
    }

  /* Incrementation du nombre de threads endormis sur cette wq */
  spin_lock(&dst->wq_lock);
    wq->nbre++;
  spin_unlock(&dst->wq_lock);

  /* Attente */
  cr = wait_event_interruptible(wq->wq, wq->cond);

  /* Decrementation du nombre de threads endormis sur cette wq */
  spin_lock(&dst->wq_lock);
    nbre_restant = --wq->nbre;
  spin_unlock(&dst->wq_lock);

  /* TODO : Il manque ici une protection (semaphore ?, autre chose ?) */

  /* Si plus personne n'attend, recyclage de la waitqueue */
  if (nbre_restant == 0)
    { wq_rendre(dst, wq);
      *lynx_sem = 0;
    }

  if (cr) return -ERESTARTSYS;
     else return 0;
}


/* Reveil des taches en attente sur une wait queue */
void wq_reveiller(struct asdc_varg *dst, long *lynx_sem)
{
  struct wq_liste *wq;

  /* Y-a-t-il vraiment quelqu'un a reveiller */
  if (*lynx_sem == (long) NULL) return;

  /* Si oui : reveil */
  wq = (struct wq_liste *) *lynx_sem;
  wq->cond = 1;
  wake_up_interruptible_all(&wq->wq);
}



void wq_liberer(struct asdc_varg *dst)
{
  vfree(dst->wq_base);
  dst->wq_base = NULL;
  dst->wq_libres = NULL;
  dst->wq_occ_debut = NULL;
  dst->wq_occ_fin = NULL;
}
