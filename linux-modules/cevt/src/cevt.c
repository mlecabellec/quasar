/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                                         Y.Guillemot, le  9/09/2002   *
 *                           Implementation du "select" le 23/09/2002   *
 *     Inhibition des IT pendant CEVT_signaler() modif. le 30/09/2002   *
 * Debut de traitement de la compatibilite LynxOS/Linux le  7/10/2002   *
 *                                                                      *
 * Ajout gestion de la datation pour les E/S TOR (XS ?) le 22/05/2003   *
 *                                                                      *
 *            Fin du portage (avec compatibilite) Linux le 17/10/2006   *
 *  Adaptation (sous Linux) cevt_signaler_date() au TSC le 25/10/2006   *
 *                                                                      *
 * Retrait des macros MODULE_PARM depreciees dans                       *
 *        Linux 2.18 et remplacement par module_param : le  9/10/2007   *
 *                                          CEVT v1.5 : le  9/10/2007   *
 *                                                                      *
 *                    Adaptation aux systemes 64 bits : le 13/10/2009   *
 *                                                                      *
 *                  Adaptation au noyau Linux v2.6.31 : le  1/12/2009   *
 *                   Adaptation au noyau Linux v3.3.6 : le  8/04/2013   *
 *                                                                      *
 *  Modification de l'interface CEVT/ASDC (utilisation du lien          *
 *  standard Linux au lieu du "bricolage" precedent :   le  2/09/2014   *
 *                                                                      * 
 *  Integration ASTRE simulation drast, S. Devaux le 30/09/2015         *                                                         
 ************************************************************************/


/***********************************************************/
/*   Inclusion des fichiers .h pour LynxOS ou pour Linux   */
/***********************************************************/

#define KERNEL


#ifdef LYNXOS

#include <io.h>
#include <mem.h>
#include <conf.h>
#include <kernel.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <dldd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysctl.h>
#include <time.h>

#elif LINUX

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
# include <linux/poll.h>
# include <linux/vmalloc.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#endif

#else

# error "L'un au moins des 2 symboles LINUX ou LYNXOS doit etre defini"

#endif	/* LynxOS - Linux */


#ifdef LYNXOS
# ifdef LINUX
#  error "Un seul des 2 symboles LINUX ou LYNXOS doit etre defini"
# endif
#endif



#include "cevt_if.h"	/* Compatibilite LynxOS/Linux */
#include "cevtinfo.h"	/* Definit les parametres decrivant la carte */
#include "cevt.h"	/* Definit les structures de donnees internes */
#include "cevtctl.h"	/* Definit les I/F applications */
#include "cevtvarg.h"	/* Definit les "variables globales" */
#include "cevtver.h"	/* Definit le numero de version du pilote */


#ifdef LINUX

  /* Recuperation des parametres du module et initialisations diverses */

  /* Numero majeur propose pour le pilote (si 0, choix laisse au systeme) */
  static int majeur = 0;
  module_param(majeur, int, S_IRUGO);
  MODULE_PARM_DESC(majeur, "Numero de device majeur associe au module");

  /* Les CEVT sont numerotes de 1 � N (N < CEVT_MAX_DEVICES)             */
  /* Le numero du CEVT correspond au mineur du device                    */
  /* Chaque CEVT a un seul parametre : le nombre de tampons dans la fifo */
  /* (parametre "tf" pour taille FIFO)                                   */
  /* Un CEVT avec tf=0 n'est pas implemente                              */

  /*** ATTENTION : CEVT_MAX_DEVICES declarations !!! ***/
  /* Nombre de tampons dans la FIFO du CEVT - CEVT non implemente si 0 */
  static int tf1 = 100;
  static int tf2 = 100;
  static int tf3 = 100;
  static int tf4 = 100;
  static int tf5 = 0;
  static int tf6 = 0;
  static int tf7 = 0;
  static int tf8 = 0;
  static int tf9 = 0;
  static int tf10 = 0;
  static int tf11 = 0;
  static int tf12 = 0;
  static int tf13 = 0;
  static int tf14 = 0;
  static int tf15 = 0;
  static int tf16 = 0;
  static int tf17 = 0;
  static int tf18 = 0;
  static int tf19 = 0;
  static int tf20 = 0;
  static int tf21 = 0;
  static int tf22 = 0;
  static int tf23 = 0;
  static int tf24 = 0;
  static int tf25 = 0;

  /*** ATTENTION : CEVT_MAX_DEVICES appels de la macro !!! ***/
  module_param(tf1, int, S_IRUGO);
  MODULE_PARM_DESC(tf1, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT1");
  module_param(tf2, int, S_IRUGO);
  MODULE_PARM_DESC(tf2, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT2");
  module_param(tf3, int, S_IRUGO);
  MODULE_PARM_DESC(tf3, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT3");
  module_param(tf4, int, S_IRUGO);
  MODULE_PARM_DESC(tf4, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT4");
  module_param(tf5, int, S_IRUGO);
  MODULE_PARM_DESC(tf5, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT5");
  module_param(tf6, int, S_IRUGO);
  MODULE_PARM_DESC(tf6, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT6");
  module_param(tf7, int, S_IRUGO);
  MODULE_PARM_DESC(tf7, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT7");
  module_param(tf8, int, S_IRUGO);
  MODULE_PARM_DESC(tf8, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT8");
  module_param(tf9, int, S_IRUGO);
  MODULE_PARM_DESC(tf9, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT9");
  module_param(tf10, int, S_IRUGO);
  MODULE_PARM_DESC(tf10, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT10");
  module_param(tf11, int, S_IRUGO);
  MODULE_PARM_DESC(tf11, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT11");
  module_param(tf12, int, S_IRUGO);
  MODULE_PARM_DESC(tf12, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT12");
  module_param(tf13, int, S_IRUGO);
  MODULE_PARM_DESC(tf13, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT13");
  module_param(tf14, int, S_IRUGO);
  MODULE_PARM_DESC(tf14, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT14");
  module_param(tf15, int, S_IRUGO);
  MODULE_PARM_DESC(tf15, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT15");
  module_param(tf16, int, S_IRUGO);
  MODULE_PARM_DESC(tf16, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT16");
  module_param(tf17, int, S_IRUGO);
  MODULE_PARM_DESC(tf17, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT17");
  module_param(tf18, int, S_IRUGO);
  MODULE_PARM_DESC(tf18, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT18");
  module_param(tf19, int, S_IRUGO);
  MODULE_PARM_DESC(tf19, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT19");
  module_param(tf20, int, S_IRUGO);
  MODULE_PARM_DESC(tf20, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT20");
  module_param(tf21, int, S_IRUGO);
  MODULE_PARM_DESC(tf21, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT21");
  module_param(tf22, int, S_IRUGO);
  MODULE_PARM_DESC(tf22, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT22");
  module_param(tf23, int, S_IRUGO);
  MODULE_PARM_DESC(tf23, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT23");
  module_param(tf24, int, S_IRUGO);
  MODULE_PARM_DESC(tf24, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT24");
  module_param(tf25, int, S_IRUGO);
  MODULE_PARM_DESC(tf25, "Nombre de tampons dans la FIFO ou 0 si pas de CEVT25");

  /* Description du module */
  MODULE_AUTHOR("Yves Guillemot");
  MODULE_DESCRIPTION("Pilote " CEVT_NOM "v" CEVT_VERSION " du " CEVT_DATE);
#if	(LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0))
  MODULE_SUPPORTED_DEVICE("Pseudo driver, pas de materiel associe");
#endif
  MODULE_LICENSE("EADS-ST");    /* Devrait etre "GPL" */

  static int option(int mineur)
  {
    switch(mineur)
    {
      case 1 : return tf1;
      case 2 : return tf2;
      case 3 : return tf3;
      case 4 : return tf4;
      case 5 : return tf5;
      case 6 : return tf6;
      case 7 : return tf7;
      case 8 : return tf8;
      case 9 : return tf9;
      case 10 : return tf10;
      case 11 : return tf11;
      case 12 : return tf12;
      case 13 : return tf13;
      case 14 : return tf14;
      case 15 : return tf15;
      case 16 : return tf16;
      case 17: return tf17;
      case 18: return tf18;
      case 19: return tf19;
      case 20: return tf20;
      case 21: return tf21;
      case 22: return tf22;
      case 23: return tf23;
      case 24: return tf24;
      case 25: return tf25;
    }

    KPRINTF("ATTENTION : Appel de option(%d) !!!\n", mineur);
    KPRINTF("            (devrait etre compris entre 1 et 25)\n");
    return 0;		/* devrait limiter les degats ... */

  }

#endif /* LINUX */



/* Declarations necessaires de fonctions ... */
#ifdef LYNXOS

  int cevtioctl(struct cevt_statics *dst,
                struct file *f,
                int commande,
                char *arg);

#elif LINUX

#ifdef OLDIOCTL
  int
  cevtioctl(struct inode *inode,
            struct file *f, unsigned int commande, unsigned long arg);
#else
  long
  cevtioctl(struct file *f, unsigned int commande, unsigned long arg);
#endif


  /* Fonctions ou declarations specifiques Linux */
  static int __init init_cevt(void);
  static char __init *cevtinstall(struct cevtinfo *info);
  static int cevtopen(struct inode *inode, struct file *f);
  static int cevtclose(struct inode *inode, struct file *f);
  static unsigned int cevtpoll(struct file *f, poll_table *pt);
  static void __exit cleanup_cevt(void);

  /* Declaration des points d'entree du driver */
  module_init(init_cevt);
  module_exit(cleanup_cevt);

  static struct file_operations cevtfops =
           { owner:            THIS_MODULE,
             open:             cevtopen,
             release:          cevtclose,
             unlocked_ioctl:   cevtioctl,
             poll:             cevtpoll,
           };

#endif /* LYNXOS - LINUX */




/* Pour DEBUG, commenter ou decommenter les 2 lignes ci-dessous */
/* #define DEBUG    */
/* #define DEBUG_IT */

/* dprintf() peut etre changee en kkprintf, cprintf ou ignoree */
#ifdef DEBUG
#define dprintf 	kkprintf
/* #define dprintf	cprintf */
#else
#define dprintf
#endif

/* zzprintf() est utilise dans fonction IT seulement        */
/* Elle peut etre changee en kkprintf ou ignoree            */
/* L'appel de cprintf() est interdit dans une fonction d'IT */
#ifdef DEBUG_IT
#define zzprintf	kkprintf
#else
#define zzprintf
#endif





/*****************************************/
/* Variables statiques "globale systeme" */
/*****************************************/

/* table des structures statiques de chaque device */
struct cevt_statics * cevt_ps[CEVT_MAX_DEVICES];




/*********************************************************************
 * Sous LynxOS, les "devices" sont installes separement, au rythme
 * des appels a l'utilitaire devinstall.
 *
 * Sous Linux, tous les devices doivent etre installes au cours de
 * l'unique appel de la fonction init_cevt() d'initialisation du
 * driver.
 *
 * La fonction init_cevt ci-dessous (Linux uniquement) va donc parcourir
 * la liste des parametres (explicites ou par defaut), preparer une
 * structure info identique a celle utilisee sous LynxOS, puis appeler,
 * pour chacun des CEVT, la fonction cevtinstall() que LynxOS utilise
 * egalement.
 *********************************************************************/
#ifdef LINUX
static int __init init_cevt(void)
{
  struct cevtinfo info;
  int k, rslt;
  int nbtamp;
  struct cevt_statics * r;

  /* Enregistrement du driver */
  rslt = register_chrdev(majeur, CEVT_NOM, &cevtfops);

  if (rslt < 0)
    {
      KPRINTF("\nEchec installation du driver %s v%s\n",
                                              CEVT_NOM, CEVT_VERSION);
      if (rslt == -EINVAL)
        { KPRINTF("        ==> Le Majeur specifie est hors bornes\n");
	}
      if (rslt == -EBUSY)
        { KPRINTF("        ==> Le Majeur specifie est indisponible\n");
	}
      KPRINTF("\n");
      return rslt;
    }

  if (rslt > 0)
    {
      majeur = rslt;
    }

  KPRINTF("\nDebut installation du driver %s v%s   -   Majeur = %d\n",
                                              CEVT_NOM, CEVT_VERSION, majeur);


  /* Initialisation a NULL des entrees de la table des donnees statiques */
  /* (pour pouvoir tester (dans open) la validite des mineur utilises)   */
  for (k = 0; k < CEVT_MAX_DEVICES; k++) cevt_ps[k] = NULL;

  /* Boucle de parcours des parametres */
  for (k=1; k <= CEVT_MAX_DEVICES; k++)
    { nbtamp = option(k);
      if (nbtamp == 0) break;
      info.numero = k;
      info.nb_tamp = nbtamp;

      // KPRINTF("===> cevt : k = %d\n", k);
      r = (struct cevt_statics *) cevtinstall(&info);
      if (((long) r) == SYSERR) break;

      cevt_ps[k-1] = r;	/* Memorisation adresse structure statique CEVT */
      // KPRINTF("===> cevt_ps[%d] = %p\n", k-1, r);
    }


  if (k == 0)
    {
      /* Aucun CEVT n'a ete defini !           */
      /* ==> Desenregistrement du driver !     */
      unregister_chrdev(majeur, CEVT_NOM);
      KPRINTF("Aucun CEVT n'est defini   -   Echec de l'installation !\n\n");
      return -EIO;
    }


  /* Installation achevee */
  KPRINTF("Fin installation du driver %s v%s   -   Majeur = %d\n\n",
                                              CEVT_NOM, CEVT_VERSION, majeur);
  return OK;


}
#endif /* LINUX */




/*********************************************************************
 * input    : <info> pointer to device
 *
 * output   : none
 *
 * return   : a pointer to the static storage
 *
 * function : install entry point
 *
 * call     : during "devinstall"
 *
 *            interrupts not masked
 *********************************************************************/

static char __init *cevtinstall(struct cevtinfo *info)
{
  struct cevt_statics *driver_statics;
  int j;

  kkprintf( "Pseudo-driver CEVT : cevtinstall()\n");


  /* -------------------------------------------------------------------
     -- Le numero du device est-il compatible avec la table allouee ? --
     ------------------------------------------------------------------- */

  if (info->numero > CEVT_MAX_DEVICES)
    { kkprintf("CEVT : numero de device %ld trop grand !\n", info->numero);
      return (char *) SYSERR;
    }

  if (info->numero < 1)
    { kkprintf("CEVT : numero de device %ld non strictement positif !\n",
                                                             info->numero);
      return (char *) SYSERR;
    }





  /* ---------------------------------------------------------------
     -- Allocation memoire pour les variables statiques du driver --
     --------------------------------------------------------------- */
#ifdef LYNXOS

  driver_statics = (struct cevt_statics *)
                              sysbrk(sizeof(struct cevt_statics));
  if (driver_statics == (struct cevt_statics *) 0L)
    { kkprintf("CEVT : Echec allocation memoire pour \"statics\"");
      return (char *) SYSERR;
    }

#elif LINUX

  driver_statics = vmalloc(sizeof(struct cevt_statics));
  if (driver_statics == NULL)
    { kkprintf("CEVT : Echec allocation memoire pour \"statics\"");
      return (char *) SYSERR;
    }

#endif /* LYNXOS - LINUX */



   /* -------------------------------------------------
      --   Allocation des tampons du registre PEPS   --
      ------------------------------------------------- */
#ifdef LYNXOS

  driver_statics->pbcfev = (struct cevt_tfch *)
                    sysbrk(sizeof(struct cevt_tfch) * info->nb_tamp);

  if (driver_statics->pbcfev == (struct cevt_tfch *) 0L)
    { kkprintf("CEVT : Echec allocation memoire pour tampons");

      /* Echec installation ==> Liberation memoire deja allouee */
      sysfree(driver_statics, sizeof(struct cevt_statics));

      return (char *) SYSERR;
    }

#elif LINUX

  driver_statics->pbcfev = vmalloc(sizeof(struct cevt_tfch) * info->nb_tamp);

  if (driver_statics->pbcfev == NULL)
    { kkprintf("CEVT : Echec allocation memoire pour tampons");

      /* Echec installation ==> Liberation memoire deja allouee */
      vfree(driver_statics);

      return (char *) SYSERR;
    }

#endif /* LYNXOS - LINUX */





   /* -------------------------
      --   Initialisations   --
      ------------------------- */

   driver_statics->nb_tampons = info->nb_tamp;
   driver_statics->numero = info->numero;
   cevt_ps[info->numero - 1] = driver_statics;



   /* Initialisation des tampons a "tous vides" */
   driver_statics->pbcfev[0].s = NULL;
   for (j=1; j<driver_statics->nb_tampons; j++)
       { driver_statics->pbcfev[j].s = &(driver_statics->pbcfev[j-1]);
       }
   driver_statics->ptv =
             &(driver_statics->pbcfev[driver_statics->nb_tampons - 1]);
   driver_statics->nb_tamp_dispos = driver_statics->nb_tampons;

   driver_statics->ppt = driver_statics->pdt = NULL;



   /* Semaphore d'attente */
   SINIT(driver_statics->sem_attente);
   spin_lock_init(&driver_statics->lock_attente);


   /* Donnees dispos pour fonction select */
   driver_statics->sel_data = 0;
   driver_statics->rsel_sem = NULL;		/* Utile ?   Necessaire ? */


   /* Initialisation diverses */

   driver_statics->raz = 0;

   driver_statics->dioctl = 0;		/* variables pour autopsie via SKBD */
   driver_statics->dphioctl = 0;
   driver_statics->dit = 0;
   driver_statics->dphit = 0;


   /* ------------------------------------------------------
      -- return the statics memory address to the Lynx OS --
      -- Lynx uses this when calling all functions of the --
      -- driver...					  --
      ------------------------------------------------------ */
   return (char *) driver_statics;
}



#ifdef LYNXOS

/*****************************************************/
/* Driver prevu pour rester installe                 */
/*          ==> pas de fonction asdc_uninstall()     */
/*****************************************************/
int cevtuninstall(struct cevt_statics *dst)
{ kkprintf("\n---   ATTENTION : cevtuninstall non implementee !!!   ---\n");
  pseterr( EPERM );
  return SYSERR;
}

#elif LINUX

static void __exit cleanup_cevt(void)
{
  int i;

  KPRINTF("\nDechargement du driver %s et des devices associes :\n",
           CEVT_NOM);

  /*
  ** Les operations sont effectuees dans l'ordre inverse
  ** de celui utilise a l'installation.
  */

  for (i = 0; i < CEVT_MAX_DEVICES; i++)
    {
      if (cevt_ps[i] != NULL)
	{
	  /* cevt_ps[i] pointe la structure cevt_statics associee au CEVT */

	  /* Liberation de la FIFO */
          vfree(cevt_ps[i]->pbcfev);

	  /* Liberation du reste de la memoire du CEVT */
          vfree(cevt_ps[i]);
	  cevt_ps[i] = NULL;
	}
    }

  /* Desenregistrement du driver aupres du systeme */
  KPRINTF(" Desenregistrement du driver : (toujours OK)\n");
  unregister_chrdev(majeur, CEVT_NOM);
}

#endif /* LYNXOS - LINUX */






/* -----------------------------------------------
   --					        --
   --		fonction open 			--
   --						--
   ----------------------------------------------- */
#ifdef LYNXOS

  int cevtopen(struct cevt_statics *driver_statics,
                                              int device, struct file *f )
  {
    dprintf("cevtopen called...\n");

    return(OK);

  }

#elif LINUX

  int cevtopen(struct inode *inode, struct file *f)
  {
    int mineur;

    mineur = MINOR(inode->i_rdev);

    if ((mineur <= 0) || (mineur >= CEVT_MAX_DEVICES))
      {
        KPRINTF("CEVT : le mineur %d utilise est hors bornes !\n", mineur);
        return -EINVAL;
      }

    if (cevt_ps[mineur - 1] == NULL)
      {
        KPRINTF("CEVT : mineur %d : CEVT non defini !\n", mineur);
        return -EINVAL;
      }

    /* Memorisation des donnees du device */
    f->private_data = cevt_ps[mineur - 1];

    return OK;
  }

#endif	/* LYNXOS - LINUX */








/* -----------------------------------------------
   --				                --
   --		fonction close 	                --
   --			                        --
   ----------------------------------------------- */

/* ATTENTION : N'est appele qu'au DERNIER close() */

#ifdef LYNXOS
  int cevtclose(struct cevt_statics *driver_statics, struct file *f)
  {
    dprintf("\ncevtclose called...");

    return OK;
  }
#elif LINUX
  int cevtclose(struct inode *inode, struct file *f)
  {
    dprintf("\ncevtclose called...");

    return OK;
  }
#endif	/* LYNXOS - LINUX */





/* Pour l'implementation de l'appel UNIX "select()" */
#ifdef LYNXOS

int cevtselect(struct cevt_statics *dst, struct file *f,
                                          int which, struct sel *ffs)
{

  switch (which)
      {
	case SREAD :
		ffs->iosem = &dst->sel_data;
		ffs->sel_sem = &dst->rsel_sem;
		break;

	case SWRITE:
		break;

	case SEXCEPT:
		break;

	default :
		break;
      }


  return (OK);


}

#elif LINUX

static unsigned int cevtpoll(struct file *f, poll_table *pt)
{
 struct cevt_statics *dst;   /* Pointe les donnees statiques du CEVT courant */

 dst = f->private_data;

 poll_wait(f, &dst->sem_attente, pt);

 if (dst->sel_data > 0)
   { return POLLIN | POLLRDNORM;
   }
 else
   { return 0;
   }
}

#endif /* LYNXOS - LINUX */




/*********************************/
/* Test de l'existence d'un CEVT */
/*********************************/
int cevt_existence(int numero_cevt)
{
  struct cevt_statics *dst;

  /* Numero CEVT valide ? */
  if ((numero_cevt < 1)	|| (numero_cevt > CEVT_MAX_DEVICES)) return 0;

  /* Zone statique valide apparente ? */
  dst = cevt_ps[numero_cevt - 1];
  if (dst == NULL) return 0;

  /* Un CEVT semble exister � */
  return 1;
}






/*************************************************************************/
/* Fonction d'entree d'un evenement                                      */
/*                                                                       */
/*   ATTENTION : cette fonction ne doit etre appelee que dans une        */
/*               section critique protegee des interruptions materielles */
/*************************************************************************/
int cevt_signaler(int  numero_cevt,
                  int  nature_es,	/* CEVT_ETOR, CEVT_1553, etc... */
                  int  numero_es,	/* numero voie/coupleur */
                  long evenement,	/* CEVT_AVORTE ou descr. evenement */
                  long donnee		/* Donnee associee eventuelle */
                 )
{


  struct cevt_statics *dst;
  struct cevt_tfch *p, *q;

  unsigned long ps;	/* Pour disable/restore */

  dprintf("CEVT_signaler %d  ES=%d/%d evt=0x%08X, data=0x%x\n",
	   numero_cevt, nature_es, numero_es, evenement, donnee);

  /* Recherche de la zone statique du CEVT concerne */

  /* Numero CEVT valide ? */
  if ((numero_cevt < 1)	|| (numero_cevt > CEVT_MAX_DEVICES)) return 1;

  /* Zone statique valide (peut-etre ...) ? */
  dst = cevt_ps[numero_cevt - 1];
  if (dst == NULL) return 1;

  spin_lock_irqsave(&dst->lock_attente, ps);		/******* Debut section critique *******/

  /********************************/
  /* Ajout de l'evenement au CEVT */
  /********************************/

  /* Est-ce un evenement "SDC_AVORTE" ? */
  if (    (nature_es != CEVT_EXT)
       && (evenement == CEVT_AVORTE))
    {
      /* Montee du fanion "RAZ" */
      dst->raz = 1;
    }
   else
    {
      /* Ajout d'un tampon au PEPS */
      if (dst->ptv == NULL)
        {
          /* Plus de tampons disponibles dans ce flux */
          /* On marque le dernier tampon comme perdu  */
          p = dst->pdt;
          p->tamp.type = CEVT_DEBORD;
        }
      else
        {
          /* Retrait d'un tampon dispo */
          p = dst->ptv;
          dst->ptv = p->s;
          p->s = NULL;

          /* Ajout du tampon au PEPS */
          if (dst->ppt == NULL)
            {
              dst->ppt = p;
              dst->pdt = p;
            }
          else
            {
              q = dst->pdt;
              q->s = p;
              dst->pdt = p;
            }

          /* Memorisation du nouveau nombre de tampons */
          dst->nb_tamp_dispos--;

          /* Mise a jour du contenu du tampon */
          p->tamp.type = nature_es;	/* Type evenement */
          p->tamp.src = numero_es;	/* Numero du coupleur source */
          p->tamp.d1 = evenement;	/* Commande 1553 ou donnee EXT */
          p->tamp.d2 = donnee;		/* Donnee TOR ou 1553 */
        }
    }


  /* Reveil d'une eventuelle tache en attente */
  /* sur le flux d'evenements                 */
  SRESET(&dst->sem_attente);

  /* Calcul du nombre des donnees dispo pour select */
  dst->sel_data = dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;

#ifdef LYNXOS
  /* Reveil d'une eventuelle tache en attente sur select */
  /* (ssignal(), car on ne veut reveiller qu'une seule tache) */
  if (dst->rsel_sem) SSIGNAL(dst->rsel_sem);
#endif /* LYNXOS */

  spin_unlock_irqrestore(&dst->lock_attente, ps);		/******* Fin section critique *******/


  /* Tout s'est bien passe ! */
  return 0;

}



/*************************************************************************/
/* Fonction d'entree d'un evenement                                      */
/*                                                                       */
/*   ATTENTION : cette fonction ne doit etre appelee que dans une        */
/*               section critique protegee des interruptions materielles */
/*************************************************************************/
#ifdef LYNXOS
int cevt_signaler_date(int  numero_cevt,
		       int  nature_es,	/* CEVT_ETOR, CEVT_1553, etc... */
		       int  numero_es,	/* numero voie/coupleur */
		       long evenement,	/* CEVT_AVORTE ou descr. evenement */
		       long donnee,	/* Donnee associee eventuelle */
		       struct timespec tp /* date */
		       )
#elif LINUX
// int cevt_signaler_date(int  numero_cevt,
//                        int  nature_es,  /* CEVT_ETOR, CEVT_1553, etc... */
//                        int  numero_es,  /* numero voie/coupleur */
//                        long evenement,  /* CEVT_AVORTE ou descr. evenement */
//                        long donnee,     /* Donnee associee eventuelle */
//                        unsigned long long tsc /* date */
//                        )
int cevt_signaler_date(void)
#endif /* LYNXOS - LINUX */
{

    printk("Toto li toto\n");
    
    
#if 0 
  struct cevt_statics *dst;
  struct cevt_tfch *p, *q;

  unsigned long ps;	/* Pour disable/restore */

//   printk("CEVT_signaler_date %d  ES=%d/%d evt=0x%08X, data=0x%x, tsc=%Ld\n",
// 	   numero_cevt, nature_es, numero_es, evenement, donnee, tsc);

  /* Recherche de la zone statique du CEVT concerne */

  /* Numero CEVT valide ? */
  if ((numero_cevt < 1)	|| (numero_cevt > CEVT_MAX_DEVICES)) return 1;

  /* Zone statique valide (peut-etre ...) ? */
  dst = cevt_ps[numero_cevt - 1];
  if (dst == NULL) return 1;

  spin_lock_irqsave(&dst->lock_attente, ps);		/******* Debut section critique *******/

  /********************************/
  /* Ajout de l'evenement au CEVT */
  /********************************/

  /* Est-ce un evenement "SDC_AVORTE" ? */
  if (    (nature_es != CEVT_EXT)
       && (evenement == CEVT_AVORTE))
    {
      /* Montee du fanion "RAZ" */
      dst->raz = 1;
    }
   else
    {
      /* Ajout d'un tampon au PEPS */
      if (dst->ptv == NULL)
        {
          /* Plus de tampons disponibles dans ce flux */
          /* On marque le dernier tampon comme perdu  */
          p = dst->pdt;
          p->tamp.type = CEVT_DEBORD;
        }
      else
        {
          /* Retrait d'un tampon dispo */
          p = dst->ptv;
          dst->ptv = p->s;
          p->s = NULL;

          /* Ajout du tampon au PEPS */
          if (dst->ppt == NULL)
            {
              dst->ppt = p;
              dst->pdt = p;
            }
          else
            {
              q = dst->pdt;
              q->s = p;
              dst->pdt = p;
            }

          /* Memorisation du nouveau nombre de tampons */
          dst->nb_tamp_dispos--;

          /* Mise a jour du contenu du tampon */
          p->tamp.type = nature_es;	/* Type evenement */
          p->tamp.src = numero_es;	/* Numero du coupleur source */
          p->tamp.d1 = evenement;	/* Commande 1553 ou donnee EXT */
          p->tamp.d2 = donnee;		/* Donnee TOR ou 1553 */
#ifdef LYNXOS
	  p->tamp.date[0] = tp.tv_sec;  /* date seconde */
	  p->tamp.date[1] = tp.tv_nsec; /* date nanoseconde */
#else /* LINUX */
	  p->tamp.tsc = tsc;  /* date (TSC) */
#endif /* LYNXOS - LINUX */
        }
    }


  /* Reveil d'une eventuelle tache en attente */
  /* sur le flux d'evenements                 */
  SRESET(&dst->sem_attente);

  /* Calcul du nombre des donnees dispo pour select */
  dst->sel_data = dst->nb_tampons - dst->nb_tamp_dispos + dst->raz;

#ifdef LYNXOS
  /* Reveil d'une eventuelle tache en attente sur select */
  /* (ssignal(), car on ne veut reveiller qu'une seule tache) */
  if (dst->rsel_sem) SSIGNAL(dst->rsel_sem);
#endif /* LYNXOS */


  spin_unlock_irqrestore(&dst->lock_attente, ps);		/******* Fin section critique *******/
#endif   /* #if 0 */

  /* Tout s'est bien passe ! */
  return 0;

}






/*
 *********************************************************************
 *
 * Points d'entree du driver non implementes
 *
 *********************************************************************
 */

/*	cevtread()                                  */
#ifdef LYNXOS
  int cevtread(struct cevt_statics *dst, struct file * f, char * buf, int count )
  { kkprintf("\n---   ATTENTION : cevtread non implementee !!!   ---\n");
    pseterr( EPERM );
    return SYSERR;
  }
#elif LINUX
#endif /* LYNXOS - LINUX */

/*	cevtwrite()                                  */
#ifdef LYNXOS
int cevtwrite(struct cevt_statics *dst, struct file * f, char * buf, int count )
{ kkprintf("\n---   ATTENTION : asdcwrite non implementee !!!   ---\n");
  pseterr( EPERM );
  return SYSERR;
}
#elif LINUX
#endif /* LYNXOS - LINUX */






#ifdef LYNXOS
/*
 *********************************************************************
 *
 * Points d'entree du driver pour edition dynamique des liens
 *
 *********************************************************************
 */
  struct dldd entry_points =
  {
          cevtopen,
          cevtclose,
          cevtread,
          cevtwrite, 		/* stub */
          cevtselect,
          cevtioctl,
          cevtinstall,
          cevtuninstall,	 	/* stub */
          (char *) 0
  };
#endif /* LYNXOS */


/**
 * Export fonctions
 */
EXPORT_SYMBOL(cevt_existence);
EXPORT_SYMBOL(cevt_signaler);
EXPORT_SYMBOL(cevt_signaler_date); 





/******************************************************************************

   REMARQUES SUR LE FONCTIONNEMENT DU PILOTE :
   ===========================================





******************************************************************************/


