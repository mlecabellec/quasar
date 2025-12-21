/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                          Fonction ioctl()                            *
 *                         ==================                           *
 *                                                                      *
 *                                         Y.Guillemot, le 24/01/1991   *
 *                                               modif. le  1/02/1991   *
 *                                               modif. le 14/03/1991   *
 *                                               modif. le 18/03/1991   *
 *                                      derniere modif. le  4/09/1991   *
 *                         Adaptation a microcode SDC : le 31/03/1992   *
 *                                               modif. le  6/04/1992   *
 *                                               modif. le 16/04/1992   *
 *                                               modif. le 28/10/1992   *
 *                                      derniere modif. le 19/11/1992   *
 *     Adaptation a utilisation du mode espion : modif. le 15/12/1993   *
 *                                               modif. le  4/01/1994   *
 *                                               modif. le 16/03/1994   *
 *                                               modif. le 13/04/1994   *
 *                                               modif. le 24/05/1994   *
 *                                               modif. le  8/08/1994   *
 *                                               modif. le  8/09/1994   *
 *                                               modif. le 30/09/1994   *
 *                                               modif. le 21/10/1994   *
 *                                               modif. le  3/11/1994   *
 *                                               modif. le  6/03/1995   *
 *                                      derniere modif. le  9/03/1995   *
 *                                               v3.3 : le 13/04/1995   *
 *						 v3.4 : le 30/11/1995   *
 *					         modif. le 11/12/1995   *
 *				                 modif. le 14/12/1995   *
 *				                 modif. le 19/12/1995   *
 *				        derniere modif. le 20/12/1995   *
 *				                 v3.5 : le 10/01/1996   *
 *				                 modif. le 15/01/1996   *
 *				        derniere modif. le 19/01/1996   *
 *                                                                      *
 *                  Adaptation a LynxOS (v4.0) : modif. le 26/02/2001   *
 *				                 modif. le  1/08/2001   *
 *				                 modif. le  3/08/2001   *
 *				     Ajout des fluxs BC le 23/10/2001   *
 *      Ajout du reveil des attentes sur fluxs BC                       *
 *      quand fin trame                                 le  5/11/2001   *
 *                          Traitement des erreurs flux le  9/11/2001   *
 *                            Ajout des reveils sur RAZ le  9/11/2001   *
 *                                               modif. le 14/11/2001   *
 *           Ajout (gerant) des blocs RTRT et diffusion le 27/11/2001   *
 *      Correction oubli initialisation de dst->attente le 12/12/2001   *
 *      Suppression de divers kkprintf dans ASDCAFINTBC le 14/12/2001   *
 *   Correction bug sur section critique dans ASDCGOTBC le 19/12/2001   *
 *   Ajout fonctions ASDCLEC0 et ASDCECR0 pour acces                    *
 *                   provisoire aux abonnes asynchrones le 23/01/2002   *
 *   Suppression messages debug dans ASDCSUPTBC		le 12/04/2002	*
 *        Ajout utilisation image pour traiter voies RT le  3/05/2002	*
 *                      Ajout des flux_evt des voies RT le  5/06/2002	*
 *    Correction bug retour nbre de donnees par ASDCLEC le  1/07/2002	*
 *         Ajout variables de suivi pour debug si crash le 15/07/2002   *
 *            Correction bug : retour asdc_lire() < 0   le 15/07/2002   *
 *                                                                      *
 *         Correction bug ASDCLEC SYNC "avec attente"   le 20/08/2002   *
 *                      Ajout de la fonction ASDCLVER   le 21/08/2002   *
 *         Ajout des fonctions ASDCLDATE et ASDCLIRIG   le 21/08/2002   *
 *   Ajout du reveil des attentes sur                                   *
 *                     abonnes et flux d'evenements RT  le 22/08/2002   *
 *    Ajout d'un mode "statique" pour les voies abonnes le 23/08/2002   *
 *         Ajout retour des adresses carte par ASDCLVER le 28/08/2002   *
 * Correction de l'initialisation BCIGP, RSPGPA, etc... le  3/09/2002   *
 *                                               v4.5 : le 28/08/2002   *
 *                                                                      *
 * Suppression de la gestion des flux d'evenements RT : le 10/09/2002   *
 *  Connexion des evenements rt au pseudo-driver CEVT : le 10/09/2002   *
 * Traque des adresses anormales dans asdc_lire, ecrire le 17/09/2002   *
 *             Initialisation bloc_suivant dans ASDCRAZ le 17/09/2002   *
 *                 Ajout fonction ASDCEVT_ABO_SUPPRIMER le 20/09/2002   *
 *         Ajout asdc_indice() (detection debord. mem.) le 24/09/2002   *
 *  Utilisation de la macro RETURN pour sortir de ioctl le 24/09/2002   *
 *                   Marquage des LI et EI pour debug : le 24/09/2002   *
 *           Instrumentation sbs_1553_read pour debug : le 24/09/2002   *
 *                                                                      *
 *     Debut de modification pour compatibilite Linux : le 26/09/2002   *
 *              Correction melange modes dans ASDCDEF : le 26/09/2002   *
 *   Suite des modifications pour compatibilite Linux : le 29/09/2002   *
 *              Ajout des fonctions CC en mode abonne : le  3/10/2002   *
 *                                               v4.6 : le  3/10/2002   *
 *                                                                      *
 * Regroupement des 2 coupleurs d'une meme carte en un                  *
 *                                 seul device (v4.7) : le 18/10/20002  *
 *                                                                      *
 *     Correction BUG (Linux seulement) dans ASDCLPAR : le 12/11/2002   *
 *   Fonction d'attente execution d'un bloc BC (v4.8) : le 13/11/2002   *
 *      Mise en commentaire des marqueurs pour VMETRO : le 15/11/2002   *
 *    Init. a 0 des mots reserves dans blocs SCHEDULE : le 22/11/2002   *
 *                                                                      *
 *                  Source commun LynxOS/Linux (v4.9) :                 *
 *                   Correction initialisation RSPGPA : le 23/01/2003   *
 *  Correction temporisations initialisation coupleur : le 23/01/2003   *
 *     Ajout ioctl ASDCVFLASH et flags vflash associe : le 23/01/2003   *
 *                      Ajout fonction ioctl ASDCMBBC : le 28/01/2003   *
 *                   Ajout fonction ioctl ASDC_ABO_MF : le 28/01/2003   *
 *     Meme code pour lire heure "IRIG" et "non IRIG" : le  5/02/2003   *
 *   Ajout fonctions ioctl ASDC_ABO_IV et ASDC_ABO_VV : le 21/02/2003   *
 *                                                                      *
 *                        Correction bug dans ASDCECR : le 31/07/2003   *
 *                                                                      *
 *   Ajout mode "synchrone 2" a simulation RT (v4.12) : le  8/06/2004   *
 *   Ajout mise a jour flags pour enchainement trames                   *
 *                      et ajout fonction ASDCCHTRAME : le  3/08/2004   *
 *   Correction bug dans chainage inverse tampons abo : le  4/10/2004   *
 *     Premiers amenagements pour Noyaux Linux v4.6.x : le 22/10/2004   *
 *                         Ajout fonction ASDCHCALIBR : le  7/01/2005   *
 *                  Corrections pour retour sous LynxOS le  9/03/2005   *
 *                                         ASDC v4.12 : le  7/01/2005   *
 *                                                                      *
 *    Ajout fonction de debug/investigation ASDCTRACE : le 10/03/2005   *
 *      Modif./correction ASDCEECR (nouveau firmware) : le  1/04/2005   *
 *   Modif. ASDCECR : ajout prise en compte SDC_PROT2 : le 12/04/2005   *
 *                    Ajout fonction ASDC_ABO_LIBERER : le 14/04/2005   *
 *                                         ASDC v4.13 : le 14/04/2005   *
 *                                                                      *
 *                                    Ajout ASDCEECRF : le  1/06/2005   *
 *                                         ASDC v4.14 : le 10/06/2005   *
 *                                                                      *
 *                     Correction ASDCEVT_ABO_AJOUTER : le 28/06/2005   *
 * Correction ASDCEECRF                                                 *
 *     (cas ou les chaines 1 et 2 ont la meme taille) : le 30/06/2005   *
 *                   Correction ASDCEVT_ABO_SUPPRIMER : le 11/08/2005   *
 *                                         ASDC v4.15 : le 11/08/2005   *
 *                                                                      *
 * Modifications en raison de la reecriture                             *
 *        de l'I/F avec les wait queues Linux (v4.16) : le  8/09/2005   *
 * Modif. ASDC_ABO_MF                                                   *
 *                (generation d'erreurs transitoires) : le 12/09/2005   *
 *   Interversion codes DSP et Firmware dans ASDCLVER : le 13/09/2005   *
 * Correction bug passage arg. ASDCLECMEMO sous Linux : le 15/09/2005   *
 *  ASDC_ABO_LIBERER ne libere plus table des filtres : le 16/09/2005   *
 *                                         ASDC v4.16 : le 16/09/2005   *
 *                                                                      *
 *                           Adaptation � LynxOS v4.0 : le 18/09/2006   *
 *      Datation evenements CEVT par TSC (sous Linux) : le 25/10/2006   *
 *                                         ASDC v4.17 : le 25/10/2006   *
 *                                                                      *
 *      Correction bug dans ASDC_ABO_VV en mode SYNC2 : le  8/06/2007   *
 *                                         ASDC v4.18 : le  8/06/2007   *
 *                                                                      *
 *                  Ajout de la fonction ASDC_ABO_MFX : le 28/11/2007   *
 *                         Correcion du "bug SIVOL-Q" : le 15/02/2008   *
 *                                         ASDC v4.20 : le 15/02/2008   *
 *                                                                      *
 *          Ajout compatibilite Linux 64 bits (debut) : le  9/01/2009   *
 *                                                                      *
 *                         Adaptation au kernel 3.3.6 : le  8/04/2013   *
 ************************************************************************/


#define KERNEL


#ifdef LYNXOS

# ifdef LYNX_V4
#  include <types.h>
# endif /* LYNX_V4 */

#include <io.h>
#include <mem.h>
#include <sys/file.h>
#include <kernel.h>
#include <dldd.h>
#include <stdio.h>
#include <errno.h>
#include <pci_resource.h>
#include <pci_powerplus.h>
#include <sys/sysctl.h>
#include <sys/drm_sysctl.h>
#include <sys/pci_sysctl.h>
#include <time.h>

#elif LINUX

# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/fs.h>
# include <linux/errno.h>
# include <linux/pci.h>
# include <linux/version.h>
# if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
# include <linux/sched/signal.h>
# include <linux/uaccess.h>
# else
# include <linux/sched.h>
# endif
# include <linux/wait.h>
# include <linux/delay.h>
# include <linux/param.h>
# include <asm/uaccess.h>
# include <linux/sysctl.h>

#else

# error "L'un au moins des 2 symboles LINUX ou LYNXOS doit etre defini"

#endif	/* LynxOS - Linux */


#ifdef LYNXOS
# ifdef LINUX
#  error "Un seul des 2 symboles LINUX ou LYNXOS doit etre defini"
# endif
#endif



#include "version.h"	/* Definit le numero de version du pilote */
#include "interface.h"	/* definit les macros de portage LynxOS/Linux */




/* Pour DEBUG, commenter ou decommenter les 2 lignes ci-dessous */
/* #define DEBUG    */
/* #define DEBUG_IT */

/* dprintf() peut etre changee en KPRINTF, CPRINTF ou ignoree */
/* (Toute cette cuisine de macros devrait etre simplifiee */
/*  en utilisant un traitement plus global ...)           */
#ifdef DEBUG
#define dprintf		KPRINTF
/* #define dprintf	CPRINTF */
#else
#ifdef LINUX
#define dprintf		bidon
#else
#define dprintf
#endif /* LINUX */
#endif /* DEBUG */






#include "asdcctl.h"	/* Definit les structures de donnees et codes ioctl */
#include "asdcvarg.h"	/* Definit les variables statiques */

#ifdef LINUX
# include "asdcwq.h"	/* Definition des chaines de "wait queues" Linux */
#endif

#ifdef CEVT
# include "cevtctl.h"	/* Pour connexion au pseudo-driver CEVT */
#endif




/* Variables globales driver */
extern const struct sdcoco asdc_coco[];



/* Prototypes des fonctions de gestion de la memoire du coupleur*/
extern void asdcrazalloc(struct asdc_varg *driver_statics);
extern int asdcalloc(struct asdc_varg *driver_statics, unsigned int n);
extern int asdclibere(struct asdc_varg *driver_statics,
                      unsigned int a, unsigned int n);
extern void asdclecememo(struct asdc_varg *driver_statics,
                         struct asdcememo *data);
extern int asdclecmemo(struct asdc_varg *driver_statics,
                       struct asdcblibre *data);

/* Prototype d'une fonction utilisee pour gerer les tampons des fluxs BC */
int asdc_fluxbctamp_supprimer(struct asdcbc_tfch **ppsrc,
                              struct asdcbc_tfch **pdsrc,
                              struct asdcbc_tfch **pdst);




/*******************************************************************
 ***   Fonctions pour acceder aux mots de la memoire d'echange   ***
 *******************************************************************/

union echange_octets
 {
   short mot;
   char octet[2];
 };

unsigned short asdc_lire(int adresse_mot, char * base_octet,
                                         int position, int vflash)
{ union echange_octets eo;
  char tmp;

  /* Pour traquer les bugs */
  if ( !vflash && (    (adresse_mot < 0)
                    || (    (adresse_mot != CSR)
                         && (adresse_mot >= FIN_RAM)
                       )
                  )
     )
    {
      kkprintf("asdc_lire(0x%X)   Position=%d\n", adresse_mot, position);
    }

  eo.mot = * ((short *) (base_octet + (adresse_mot * 2)));

#ifdef GROS_BOUT
  tmp = eo.octet[0];
  eo.octet[0] = eo.octet[1];
  eo.octet[1] = tmp;
#endif

  /* cprintf("Lecture 0x%X a l'adresse 0x%X\n", eo.mot, adresse_mot); */

  return eo.mot;
}

void asdc_ecrire(int adresse_mot, char * base_octet,
                                            short valeur, int vflash)
{ union echange_octets eo;
  char tmp;

  /* cprintf("Ecriture 0x%X a l'adresse 0x%X\n", valeur, adresse_mot); */


  /* Pour traquer les bugs */
  if ( !vflash && (    (adresse_mot < 0)
                    || (    (adresse_mot != CSR)
                         && (adresse_mot >= FIN_RAM)
                       )
                  )
     )
    {
      kkprintf("asdc_ecrire(0x%X, 0x%X)\n", adresse_mot, valeur & 0xFFFF);
    }

  eo.mot = valeur;

#ifdef GROS_BOUT
  tmp = eo.octet[0];
  eo.octet[0] = eo.octet[1];
  eo.octet[1] = tmp;
#endif

  * ((short *) (base_octet + (adresse_mot * 2))) = eo.mot;
}


/* Pour traquer les bugs (debordement du tableau image[])     */
/* 	adresse_mot : valeur dont les bornes sont surveillees */
/*	op : 'L' ou 'E' selon appel dans macro LI ou EI       */
/*	pos : marqueur permettant d'identifier l'appel fautif */
int asdc_indice(int adresse_mot, char op, int pos)
{
  if (    (adresse_mot < 0)
       || (adresse_mot > 65535)
     )
    {
      kkprintf("asdc_indice(0x%X, %c, %d)\n", adresse_mot, op, pos);
    }

  return adresse_mot;
}



/**********************/
/***   PROVISOIRE   ***/
/**********************/
/* Fonction ci-dessous pour temporiser le demarrage firmware */
/* (en attendant doc. complete de la PMC2 ...)               */

static int asdc_sleep(struct asdc_varg *dst, int duree_ms)
{

#ifdef LYNXOS

  int cr;
  int nbre_ticks;

  nbre_ticks = (duree_ms * _TICKSPERSEC) / 1000;
  if (!nbre_ticks) nbre_ticks = 1;   /* Attente d'1 tick au minimum ... */

  dst->semretard = 0;

  cr = tswait(&dst->semretard, -1, nbre_ticks);

  if (cr == TSWAIT_TIMEDOUT) return 0;		/* Tout s'est bien passe */
                        else return cr;

#endif 	/* LYNXOS */

#ifdef LINUX

  unsigned long heure;
  int nbre_ticks;

  nbre_ticks = (duree_ms * HZ) / 1000;
  if (!nbre_ticks) nbre_ticks = 1;   /* Attente d'1 tick au minimum ... */

  heure = jiffies + nbre_ticks;

  while (jiffies < heure) schedule();

  return 0;

#endif 	/* LINUX */

}




/* Macros pour simplifier l'ecriture (portage de SUN-OS vers LynxOS) */
#define unit		dst->signal_number
#define bloc_suivant	dst->bloc_suivant




/* Definition provisoire, en attendant mise en place d'un eventuel */
/* mecanisme d'emission de donnees "manifestement fausses" quand   */
/* debordement des tampons en mode RT_SYNC2                        */
#define RT_FIN2		0




/* Liberation d'une chaine de tampons abonne (bouclee ou non)      */
/*     dst : pointeur vers le contexte general                     */
/*     ptamp : pointeur vers le premier tampon de la chaine        */
/*                                                                 */
/*  ATTENTION : Utilise le chainage inverse (en memoire d'echange) */
/*              pour etre compatible du plus grand nombre possible */
/*              de chaines                                         */
/*                                                                 */
static int liberer_tamp(struct asdc_varg *dst, unsigned short ptamp)
{
  int ptamp0;
  int ptampini;
  int premiere_fois;

//  /* Chainage direct : non utilise ici */
//  while ((ptamp != RT_FIN2) && (ptamp != 0))
//       {
//         ptamp0 = L(ptamp);
//         E(ptamp, RT_FIN2, 0);  /* Pour briser la boucle */
//         if (asdclibere(dst, ptamp, 34)) return -1;
//         ptamp = ptamp0;
//       }


  /* Chainage inverse */
  ptampini = ptamp;
  premiere_fois = 1;
  while ((ptamp != RT_FIN2) && (ptamp != 0))
       {
         /* Detection de la boucle */
         if (!premiere_fois && (ptamp == ptampini)) break;

         ptamp0 = LI(ptamp + IRTPREC, 0);
         if (asdclibere(dst, ptamp, 34)) return -1;
         ptamp = ptamp0;
         premiere_fois = 0;
       }

  return 0;
}



/*** Fonctions de debug ***********************************/
/* Ecriture dans le tampon des messages                   */
/* Le tampon des messages est relu par ioctl(ASDCLMSG)    */
/*   ==> cf. l'utilitaire lmsg.c dans ./test              */

/* Ecriture chaine de caracteres */
void asdcmsg_ch(struct asdc_varg *dst, char *x)
{
  int n, i;
  char *p;
  ITFLAGS(s);    /* Pour masquage IT pendant section critique */

  // kkprintf("asdcmsg_ch: dst=0x%04X\n", dst);
  // kkprintf("    ch=\"%s\"\n", x);

  /* Taille chaine */
  n = 0;
  for (p=x; *p; p++) n++;

  // kkprintf("   n=%d\n", n);

  DISABLE(s);   /* Pour permettre appel depuis fonction d'IT */

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+2))) return;

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 1;   // code "chaine"
    for (p=x; *p; p++) dst->msg.ch[dst->msg.ic++] = *p;
    dst->msg.ch[dst->msg.ic++] = 0;

  RESTORE(s);
}

/* Ecriture valeur numerique decimale */
void asdcmsg_d(struct asdc_varg *dst, int x)
{
  int n, i;
  char *p;
  ITFLAGS(s);    /* Pour masquage IT pendant section critique */

  /* Taille objet */
  n = 4;    // sizeof(int)

  DISABLE(s);   /* Pour permettre appel depuis fonction d'IT */

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+1))) return;

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 2;   // code "entier decimal"
    for (p=(char *) &x, i=0; i<4; p++, i++) dst->msg.ch[dst->msg.ic++] = *p;

  RESTORE(s);
}

/* Ecriture valeur numerique hexadecimale */
void asdcmsg_x(struct asdc_varg *dst, int x)
{
  int n, i;
  char *p;
  ITFLAGS(s);    /* Pour masquage IT pendant section critique */

  /* Taille objet */
  n = 4;    // sizeof(int)

  DISABLE(s);   /* Pour permettre appel depuis fonction d'IT */

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (n+1))) return;

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 3;   // code "entier hexadecimal"
    for (p=(char *) &x, i=0; i<4; p++, i++) dst->msg.ch[dst->msg.ic++] = *p;

  RESTORE(s);
}

/* Ecriture indicateur fin message */
void asdcmsg_fin(struct asdc_varg *dst)
{
   ITFLAGS(s);    /* Pour masquage IT pendant section critique */

   DISABLE(s);   /* Pour permettre appel depuis fonction d'IT */

    /* Test debordement */
    if (dst->msg.ic > (TTMSG - (1))) return;

    /* Ecriture */
    dst->msg.ch[dst->msg.ic++] = 4;   // code "fin message"

  RESTORE(s);
}

/*** Fin des fonctions de debug ***************************/








/* -----------------------------------------------
   --				                --
   --		asdcioctl 	                --
   --				                --
   ----------------------------------------------- */
#ifdef LYNXOS
  int asdcioctl(struct asdc_statics *driver_statics,
                    struct file *f, int commande, char *arg )
#elif LINUX
#ifdef OLDIOCTL
  int asdcioctl(struct inode *inode,
                    struct file *f, unsigned int commande, unsigned long arg)
#else
  long
  asdcioctl(struct file *f, unsigned int commande, unsigned long arg)
#endif
#endif /* LYNXOS - LINUX */
{

#ifndef OLDIOCTL
    struct inode *inode;
#endif

 struct sbs1553_ioctl *arguments, zarguments;
 int offset, current_value, nombre;
 char *ptr;
 char data_store[10];
 short *word_ptr;
 short zword_ptr;
 long *long_ptr, zlong_ptr;
 short data_item;
 union byte_swap_type byte_swap;
 char temp;
 short *base_address;
 int *int_ptr, zint_ptr;

 struct asdcvoie *v, zv;
 struct asdctampon *t, zt;
 struct asdcmim *w;
 struct asdcparam *p, zp;
 struct asdcbbc *b, zb;
 struct asdctampbc *c, zc;
 struct asdcmbbc *e, ze;
 struct asdclbbc *d;
 struct asdcltbc *tt;
 struct chtrame *cht, zcht;

 int i, j, k, l, m, mm, mde;
 int afiltre;
 int nbta;
 int prot2, vliee;

 unsigned int zd, cmd, z, iz;

 int erreur;		/* Indicateur (utilise par espion sequentiel) */

 long *oreiller;   /* Pointeur du semaphore ou dormir ... */

 ITFLAGS(s);    /* Pour masquage IT pendant section critique */

 int espion_tr;	/* Indicateur RT est en mode "Espion Temps Reel" */

 //TODO : Toutes les utiliusations de tmpi passent aujourd'hui par une initilisation préalable, mais ce n'est surement pas une bonne idée de garder cette variable globale
 int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */

 int debordement;	/* Indicateur debordement tampons E/S d'une voie RT */

 int tampapp;	/* Pour determiner le "pointeur de tampon application" */

 struct asdc_varg *dst;	/* Pointeur vers variables statiques coupleur */
 int mineur;		/* Numero "mineur" associe au coupleur */

 int asdcabomf;  /* Indicateur pour identifier ASDC_ABO_MF et ASDC_ABO_MFX */
 int liste;      /* Liste de bits pour ASDC_ABO_MFX */


#ifdef LYNXOS
 /* Le mineur indique le coupleur utilise sur la carte PMC */
 mineur = minor(f->dev);
 dst = driver_statics->varg[mineur];
#elif LINUX

#ifndef OLDIOCTL
# if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    inode = f->f_dentry->d_inode;
# elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
    inode = f->f_path.dentry;
# else
    inode = f->f_path.dentry->d_inode;
# endif
#endif

 /* Le mineur indique a la fois la carte PMC et le coupleur utilises  */
 mineur = MINOR(inode->i_rdev);
 dst = asdc_p[mineur / 2]->varg[mineur % 2];
#endif

 // printk("ASDC : ioctl - mineur=%d cmd=%d (0x%X)\n", mineur, commande);

 // kkprintf("Ioctl : Commande = 0x%0X\n", commande);

 dst->dioctl = commande;
 dst->dphioctl = 1;
 asdcabomf = 0;

 switch(commande)
   {

     case sbs1553_read :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;
        i = arguments->device;	/* Instrumentation */

	TVUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        *word_ptr = LL(offset, i);

        VUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));
        RETURN(OK);


     case sbs1553_write :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

        TDUTIL(arguments->buffer, sizeof(short));
        DUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        E(offset, *word_ptr);
        RETURN(OK);


     case sbs1553_get_baseaddr :

        arguments = (struct sbs1553_ioctl *) arg;
        offset = arguments->offset;

	ptr = (char *) &dst->bvba;
        for (i=0 ; i < 4; i++ )
	   {
	     data_store[i] = *ptr;
	     ptr++;
	     *arguments->buffer = (char )data_store[i];
	     arguments->buffer++;
	   }

        RETURN(OK);




     case ABI_LIRE_16 :
        {
          short mot;

          arguments = (struct sbs1553_ioctl *) arg;
          offset = arguments->offset;

          word_ptr = (short *)(dst->bvba + (offset * 2));
          mot = *word_ptr;

          *arguments->buffer = mot;

          dprintf("\nABI_LIRE_16  0x%08x (0x%08x) --> 0x%04x",
                                           offset, word_ptr, mot & 0xFFFF);
          RETURN(OK);
	}


     case ABI_ECRIRE_16 :
        {
          short mot;

          arguments = (struct sbs1553_ioctl *) arg;
          offset = arguments->offset;

          word_ptr = (short *)(dst->bvba + (offset * 2));
          mot = (short) *arguments->buffer;
          *word_ptr = mot;

          dprintf("\nABI_ECRIRE_16  0x%08x (0x%08x) <-- 0x%04x",
                                           offset, word_ptr, mot & 0xFFFF);
          RETURN(OK);
	}


     case ABI_LIRE_32 :
        {
          int mot32;

          arguments = (struct sbs1553_ioctl *) arg;
          offset = arguments->offset;

          int_ptr = (int *)(dst->bvba + (offset * 2));
          mot32 = *int_ptr;

          * ((int *) arguments->buffer) = mot32;

          dprintf("\nABI_LIRE_32  0x%08x (0x%08x) --> 0x%08x",
                                           offset, int_ptr, mot32);
          RETURN(OK);
	}


     case ABI_ECRIRE_32 :
        {
          int mot32;

          arguments = (struct sbs1553_ioctl *) arg;
          offset = arguments->offset;

          int_ptr = (int *)(dst->bvba + (offset * 2));
          mot32 = * ((int *) arguments->buffer);
          *int_ptr = mot32;

          dprintf("\nABI_ECRIRE_32  0x%08x (0x%08x) <-- 0x%08x",
                                           offset, int_ptr, mot32);
          RETURN(OK);
	}




     case ABI_LIRE_BLOC :	/* Lecture d'un bloc de mots (16 bits) */
     				/* en memoire d'echange                */

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;	/* Adresse source */
        nombre = arguments->length;	/* Taille bloc (en mots de 16 bits) */
        word_ptr = arguments->buffer;	/* Adresse destination */

        /* Compatibilite LynxOS/Linux traitee ici explicitement   */
        /* car macros xxUTIL inadaptees aux donnees dimensionnees */
        /* dynamiquement                                          */
#ifdef LYNXOS

        if (wbounds((long) arguments->buffer) < (nombre * sizeof(short)))
          { RETURN(EFAULT);
          }
        for (i=0; i<nombre; i++) *word_ptr++ = L(offset++);

#endif 		/* LYNXOS */
#ifdef LINUX

        for (i=0; i<nombre; i++)
           { short w;
             w = L(offset++);
             if (copy_to_user(word_ptr++, (void *) &w, sizeof(short)))
               { RETURN(EFAULT);
               }
           }

#endif 		/* LINUX */


        RETURN(OK);





     case ABI_ECRIRE_BLOC :	/* Ecriture d'un bloc de mots (16 bits) */
     				/* en memoire d'echange                */

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;	/* Adresse destination */
        nombre = arguments->length;	/* Taille bloc (en mots de 16 bits) */
        word_ptr = arguments->buffer;	/* Adresse source */



        /* Compatibilite LynxOS/Linux traitee ici explicitement   */
        /* car macros xxUTIL inadaptees aux donnees dimensionnees */
        /* dynamiquement                                          */
#ifdef LYNXOS

        if (rbounds((long) arguments->buffer) < (nombre * sizeof(short)))
          { RETURN(EFAULT);
          }
        for (i=0; i<nombre; i++) E(offset++, *word_ptr++);

#endif 		/* LYNXOS */
#ifdef LINUX

        for (i=0; i<nombre; i++)
           { short w;
             if (copy_from_user((void *) &w, word_ptr++, sizeof(short)))
               { RETURN(EFAULT);
               }
             E(offset++, w);
           }

#endif 		/* LINUX */


        RETURN(OK);






     case ABI_FIRMWARE_INIT :
        {
          unsigned int cnfgreg;

          /* Controle acces a la memoire (utile ?) */
          E(0xFF, 0x1234);
          if (L(0xFF) != 0x1234)
            { /* Memoire carte non accessible ... */
              RETURN(ENOTTY);
            }

          /* Chargement du firmware depuis la flash */
          E(CSR, 1);

          cprintf("ABI_FIRMWARE_INIT - Attente 1");

          /* Attente 100 ms   -   utile ??? */
          if (asdc_sleep(dst, 100))
            { RETURN(EAGAIN);		/* Pas de timer disponible */
            }

          E(PCODE, 0xFFFF);
          E(CSR, 0);

          cprintf(" - Attente 2");

          /* Attente 100 ms   -   utile ??? */
          if (asdc_sleep(dst, 100))
            { RETURN(EAGAIN);		/* Pas de timer disponible */
            }

          /* Pourquoi ces operations ??????? */   /* cf. paragraphe 6.1.3 */
          L(0x40);
          L(0x3C);
          E(0x40, 0);
          E(0x3A, 0x0D);
          L(0x40);
          L(0x40);
          L(0xEA);

          cprintf(" - Attente 3");

          /* Attente 10 ms   -   Attente indispensable !               */
	  /*    Faute de quoi :                                        */
	  /*          - CNFGREG ci-dessous n'est pas lu correctement   */
	  /*          - Le parametrage de RSPGPA effectue dans initabi */
	  /*            n'est pas pris en compte par le firmware       */
	  /*                                                           */
	  /* En fait, il faudrait boucler en attendant le passage du   */
	  /* mot d'adresse 0xEA (DSP Product Code) a une valeur non    */
	  /* nulle ...                                                 */
          if (asdc_sleep(dst, 10))
            { RETURN(EAGAIN);		/* Pas de timer disponible */
            }

          cprintf("\n");

          /* Resultat lecture ci-dessous devrait etre memorisee car */
          /* contient les capacites du materiel (ABI/ASF, etc...)   */
          cnfgreg = L(CNFGREG);
          kkprintf("ioctl(ABI_FIRMWARE_INIT) : CNFGREG = 0x%04X\n",
	            cnfgreg);

          RETURN(OK);
	}




     case ABI_FIRMWARE_STOP :
        {

          /* Arret du firmware */
          E(CSR, 1);

          RETURN(OK);
	}







     case ASDC_IMAGE_LIRE :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

	TVUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

        *long_ptr = LI(offset, 1);

	VUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

        RETURN(OK);


     case ASDC_IMAGE_ECRIRE :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

	TDUTIL(arguments->buffer, sizeof(long));
	DUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

        EI(offset, *long_ptr, 300);

        RETURN(OK);





     case ASDC_IMAGE_LBLOC :	/* Lecture d'un bloc de mots (32 bits) */
     				/* en memoire image                    */

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;	/* Adresse source */
        nombre = arguments->length;	/* Taille bloc (en mots de 32 bits) */
        int_ptr = (int *) arguments->buffer;	/* Adresse destination */


        /* Compatibilite LynxOS/Linux traitee ici explicitement   */
        /* car macros xxUTIL inadaptees aux donnees dimensionnees */
        /* dynamiquement                                          */
#ifdef LYNXOS

        if (wbounds((long) arguments->buffer) < (nombre * sizeof(int)))
          { RETURN(EFAULT);
          }
        for (i=0; i<nombre; i++) *int_ptr++ = LI(offset++, 2);

#endif 		/* LYNXOS */
#ifdef LINUX

        for (i=0; i<nombre; i++)
           { short w;
             j = LI(offset++, 2);
             if (copy_to_user(int_ptr++, (void *) &j, sizeof(int)))
               { RETURN(EFAULT);
               }
           }

#endif 		/* LINUX */

        RETURN(OK);





     case ASDC_IMAGE_EBLOC :	/* Ecriture d'un bloc de mots (32 bits) */
     				/* en memoire image                    */

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;	/* Adresse destination */
        nombre = arguments->length;	/* Taille bloc (en mots de 32 bits) */
        int_ptr = (int *) arguments->buffer;	/* Adresse source */

        /* Compatibilite LynxOS/Linux traitee ici explicitement   */
        /* car macros xxUTIL inadaptees aux donnees dimensionnees */
        /* dynamiquement                                          */
#ifdef LYNXOS

        if (rbounds((long) arguments->buffer) < (nombre * sizeof(int)))
          { RETURN(EFAULT);
          }
        for (i=0; i<nombre; i++) EI(offset++, *int_ptr++, 301);

#endif 		/* LYNXOS */
#ifdef LINUX

        for (i=0; i<nombre; i++)
           { if (copy_from_user((void *) &j, int_ptr++, sizeof(int)))
               { RETURN(EFAULT);
               }
             EI(offset++, j, 301);
           }

#endif 		/* LINUX */

        RETURN(OK);






     case ASDC_IMAGE_LIRE_L :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;
        i = arguments->device;	/* Instrumentation */

	TVUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        *word_ptr = LIL(offset, i);
	// cprintf("LIL @0x%X <-- 0x%X\n", offset, *word_ptr);

        VUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));
        RETURN(OK);


     case ASDC_IMAGE_ECRIRE_L :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

        TDUTIL(arguments->buffer, sizeof(short));
        DUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

	// cprintf("EIL @0x%X <-- 0x%X\n", offset, *word_ptr);
        EIL(offset, *word_ptr, 777);
        RETURN(OK);


     case ASDC_IMAGE_LIRE_H :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;
        i = arguments->device;	/* Instrumentation */

	TVUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        *word_ptr = LIH(offset, i);
	// cprintf("LIH @0x%X <-- 0x%X\n", offset, *word_ptr);

        VUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));
        RETURN(OK);


     case ASDC_IMAGE_ECRIRE_H :

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

        TDUTIL(arguments->buffer, sizeof(short));
        DUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

	// cprintf("EIH @0x%X <-- 0x%X\n", offset, *word_ptr);
        EIH(offset, *word_ptr, 7777);
        RETURN(OK);










       case ASDC_TESTH : /* Lecture heure evenement pour DBG */
                         /* Provisoire ...                   */

          ((struct asdcdbg *)arg)->s = dst->dbg_s;
          ((struct asdcdbg *)arg)->ns = dst->dbg_ns;

          RETURN(OK);











       case ASDCRAZ :                /* Remise a zero de la carte AMI */

          dprintf("ASDCRAZ unit=%d\n", unit);

          /* Arret du micro-code */
          E(CMD, 0);

          /* Cette reinitialisation est-elle correcte ?                  */
          /* c.f. 53B-STANDARD MIL-STD-1553B PROGRAMMERS REFERENCE, p.16 */

          /* "Liberation" de la memoire allouee */
          asdcrazalloc(dst);
          /* Dans les quelques allocations qui suivent, on est sur de ne pas */
          /* saturer la memoire de l'AMI : la val. retournee par asdcalloc() */
          /* n'est donc pas testee                                           */

          /* --- Reinitialisation des pointeurs --- */

          /* Initialisation des queues d'interruption */
          E(IQRSP, 0);
          E(IQNUM, dst->asdcdef.iqnum);

          i = asdcalloc(dst, 4*L(IQNUM));
          E(IQPTR1, i);
          i = asdcalloc(dst, 4*L(IQNUM));
          E(IQPTR2 ,i);
          E(IQCNT1 ,0);
          E(IQCNT2 ,0);

          /* Initialisation de la table des protocoles :                     */
          /*  - 1553B, "Dynamic bus control reject", "real time mon." inhibe */
          /*  et validation de l'usage SA=31 pour commandes codees           */
          E(PROPTR, asdcalloc(dst, 32));
          for (i=L(PROPTR); i<L(PROPTR)+32; i++) E(i, 0);

          /* Initialisation de la table des mots d'etat : */
          /*  - tous les RT sont inhibes                  */
          E(SWTPTR, asdcalloc(dst, 64));
          for (i=L(SWTPTR); i<L(SWTPTR)+64; i++) E(i ,0);

          /* Initialisation de la table des adresses et des tempos: */
          /*  - tous les RT sont inhibes : pas de tampons prevus    */
          E(ATPTR, asdcalloc(dst, 32));
          for (i=L(ATPTR); i<L(ATPTR)+32; i++) E(i ,0);

          /* Initialisation de la table des filtres :           */
          /*  - tous les RT sont inhibes : table mise a zero    */
          E(FTPTR, asdcalloc(dst, 32));
          for (i=L(FTPTR); i<L(FTPTR)+32; i++) E(i ,0);

          /* Reservation de la table des "Transmit Last Command" : */
          E(LCDPTR, asdcalloc(dst, 32));

          /* Reservation de la table des "Last Status Word" : */
          E(LSWPTR, asdcalloc(dst, 32));

          /* Initialisation de la table des "Last Synchro Word" :  */
          /*  -> Initialisee par 0xF0F0 a des fins de surveillance */
          E(LSYPTR, asdcalloc(dst, 32));
          for (i=L(LSYPTR); i<L(LSYPTR)+32; i++) E(i, 0xF0F0);

          /* Initialisation de la table des Masques d'IT Commandes Codees : */
          E(MIMPTR, asdcalloc(dst, 64));
          for (i=L(MIMPTR); i<L(MIMPTR)+64; i++) E(i ,0);

          /* Initialisation de la table des "BITs" :  */
          E(BITPTR, asdcalloc(dst, 32));
          for (i=L(BITPTR); i<L(BITPTR)+32; i++) E(i ,0);

          /* Initialisation de la table des "Phases" :  */
          E(RTPPTR, asdcalloc(dst, 32));
          for (i=L(RTPPTR); i<L(RTPPTR)+32; i++) E(i ,0);

          /* Initialisation de la table des "Transmit Vector Word" :  */
          E(TVWPTR, asdcalloc(dst, 32));
          for (i=L(TVWPTR); i<L(TVWPTR)+32; i++) E(i ,0);

          /* Initialisation de la table des commandes codees "RESERVE" :  */
          E(RESPTR, asdcalloc(dst, 32));
          for (i=L(RESPTR); i<L(RESPTR)+32; i++) E(i ,0);

          /* Initialisation des pointeurs utilises en mode BC */
          E(BCIPTR, 0);   /* En esperant que ca suffira */

          /* Initialisation du mode moniteur :                    */
          E(MBLEN, dst->asdcdef.mbleng);
          k = asdcalloc(dst, (int) L(MBLEN));
          if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
	             }
          E(M1PTR, k);
          k = asdcalloc(dst, (int) L(MBLEN));
          if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
	             }
          E(M2PTR, k);
          E(MBFLG, 0);

          /* Initialisation de quelques parametres */
          /*    --> Ces initialisations sont sans doute inutiles, car */
          /*        ecrasees par les valeurs par defaut du firmware   */
          /*        a l'appel de ASDCGO.                              */
          /*    ---> Elles sont donc repetees juste apres ASDCGO      */
          E(BCSMSK, dst->asdcdef.bcsmsk); /* BC Status Word Mask         */
          E(BCIGP, dst->asdcdef.bcigp);   /* BC Inter message Gap Time   */
          E(BRTCNT, dst->asdcdef.brtcnt); /* Bc ReTry CouNT              */
          E(BRTBUS, dst->asdcdef.brtbus); /* Bc ReTry BUS                */
          E(RSPGPA, dst->asdcdef.rspgpa); /* Temps de non rep. RT        */
          E(RSPGPS, dst->asdcdef.rspgps); /* Temps de reponse RT simules */

          /* (Re)Initialisation de la table des commandes codees */
          for (i=0; i<32; i++)
             for(j=0; j<(COCO_MAX+1); j++)
                dst->cocor[i][j] = (struct scoco) { 0, 0, 0, 0};


          /* Initialisation des tampons flux BC a "tous disponibles" */
          dst->pbcf[0].s = NULL;
          for (j=1; j<dst->nombre_tampons_flux; j++)
             { dst->pbcf[j].s = &(dst->pbcf[j-1]);
             }
          dst->pbcfl = &(dst->pbcf[dst->nombre_tampons_flux - 1]);
          dst->nb_tamp_flux_dispos = dst->nombre_tampons_flux;

          /* Initialisation des pointeurs vers la chaine des flux */
          dst->pflux = dst->dflux = 0;

          /* Seule la premiere des 4 lignes ci-dessous est indispensable */
   	  dst->tf_attente = 0;
   	  dst->tf_zd = 0;
   	  dst->tf_flux = 0;
   	  dst->tf_pta = NULL;

          /* Initialisation variable de chainage des blocs dans une trame */
          bloc_suivant = 0;

          /* Initialisation de la memoire "image" */
          for (i=0; i<65536; i++) EI(i, 0L, 302);

#ifdef LINUX
	  /* Initialisation de la liste des "wait queues" Linux */
	  wq_init(dst);
#endif

          /* Initialisation de l'indicateur "trame en cours" */
          dst->fin_bc = 1;

          /* Initialisation (au cas ou ...) du fanion "RAZ en cours" */
          dst->raz = 0;

          /* Initialisation histogramme d'utilisation de la table des ITs */
          dst->nbhistoit = 0;
          dst->deborde = 0;
          for (i=0; i<ASDCTHISTOIT; i++) dst->histoit[i] = 0;

          /* Initialisation des enchainements de trame */
          dst->idtrame = 0;
          for (i=0; i<MAXDESCRT; i++) dst->descr_trame[i].idtrame = 0;

          /* Pour debug : initialisation du tampon des messages */
          kkprintf("asdcraz : ph0\n");
          dst->msg.ch = dst->asdcmsg_ch;
          for (i=0; i<TTMSG; i++) dst->msg.ch[i] = '\0';
          dst->msg.ic = 0;

          /* Et premiere utilisation de ce tampon ... */
          MSG("Initabi (ASDCRAZ)\n");
          MSGFIN;

         RETURN(OK);


       case ASDCSTOP :        /* Arret du micro-code de la carte AMI */

          /* Arret du micro-code */
          E(CMD, 0);

          RETURN(OK);


       case ASDCGO :        /* Demarrage du micro-code de la carte AMI */


          /* Lancement du micro-code */
          E(CMD, 1);

          /* Une petite attente (1 ms) pour laisser au firmware */
          /* le temps de s'initialiser                          */
          if (asdc_sleep(dst, 1))
            { RETURN(EAGAIN);		/* Pas de timer disponible */
            }

          /* Reinitialisation de quelques parametres, vraisemblablement */
          /* ecrasees par des valeurs par defaut au lancement du        */
          /* firmware                                                   */
          E(BCSMSK, dst->asdcdef.bcsmsk); /* BC Status Word Mask         */
          E(BCIGP, dst->asdcdef.bcigp);   /* BC Inter message Gap Time   */
          E(BRTCNT, dst->asdcdef.brtcnt); /* Bc ReTry CouNT              */
          E(BRTBUS, dst->asdcdef.brtbus); /* Bc ReTry BUS                */
          E(RSPGPS, dst->asdcdef.rspgps); /* Temps de reponse RT simules */

	  /* RSPGPA n'est pas reprogramme ici, car ce registre ne peut */
	  /* etre initialise qu'avant le debut du traitement des I/O.  */
	  /* Toute modification ulterieure est sans effet.             */

          RETURN(OK);



       case ASDCAVORTE :     /* Interruption de toutes les taches en attente */

          /* Le premier appel de la fonction - avec arg=1 - leve le    */
          /* fanion raz et fait un sreset sur tous les semaphores      */
          /*                                                           */
          /* Le second appel de la fonction - avec arg=2 - rabaisse le */
          /* fanion raz                                                */
          /*                                                           */
          /* Pour faire une RAZ de la carte, l'application doit :      */
          /*     - 1) appeler ASDCAVORTE avec arg=1                    */
          /*     - 2) attendre (sleep) un temps suffisant pour donner  */
          /*          a toutes les taches reveillees le temps de se    */
          /*          terminer                                         */
          /*     - 3) attendre (sleep) un temps suffisant pour donner  */
          /*     - 4) appeler ASDCAVORTE avec arg=2                    */
          /*     - 5) appeler ABI_FIRMWARE_INIT                        */
          /*	 - 6) appeler ASDCRAZ, etc....                         */

          switch ((int) arg)
            { case 1 : dst->raz = 1;
                       kkprintf("RAZ ASDC en cours ...\n");
                       kkprintf("   ==> Reset de tous les semaphores\n");

                       /* Reveil de toutes les attentes sur flux BC */
                       /* (En parcourant la chaine des flux)        */
                       /*    -> L'utilisation de i est destinee a   */
                       /*       sortir du piege d'un chainage       */
                       /*       reboucle                            */
                       /*    -> L'hypothese d'un nombre de flux BC  */
                       /*       superieur a 100 n'est pas serieuse  */
                       for (i=0, j=dst->pflux;
                            ((j >= DEBUT_RAM) && (j < FIN_RAM) && (i < 100));
                            i++, j=LIH(j+IMFSD, 3))
                          { cprintf("   flux BC sreset : j=%d (0x%0X)\n", j, j);
                            SRESET_IM(&LI(j+IMSEMFLX, 4));
                          }

                       /* Reveil de toutes les attentes trame */
                       SRESET(&dst->sem_finbc);
                       SRESET(&dst->sem_gotbc);

                       /* Reveil de toutes les attentes execution bloc BC */
                       SRESET(&dst->sem_exbc);

                       /* Reveil de toutes les attentes sur abonnes */
                       for (i=0; i<32; i++) /* Balayage adresses RT */
                         {
                           /* Table des sous-adresses */
                           l = L(L(ATPTR) + i);
                           /* L'abonne est-il defini ? */
                           if (l == 0) continue;

                           /* Balayage des sous-adresses :                */
                           /* On balaye d'un seul coup les 32 adresses en */
                           /* reception et les 32 adresses en emission.   */
                           for (j=0; j<64; j++)
                             {
                               /* Pointeur premier tampon */
                               m = L(l + j);
                               /* La voie est-elle definie ? */
                               if (m == 0) continue;

                               zd = LI(l + j, 5);
                               /* Si adresse zd anormale, on abandonne ... */
                               if ((zd < DEBUT_RAM) || (zd >= FIN_RAM))
                                                                  continue;

                               /* Reveil eventuelle tache en attente */
                               /* sur cette voie                     */
                               cprintf("   sreset RT%d,%d %c",
                                         i, j % 2, (j>31) ? 'T' : 'C');
#ifdef LINUX
#ifdef __LP64__
                               cprintf("   IMA[0x%04X] = 0x%16lX\n",
                                                zd + IRSEM, LI(zd + IRSEM, 6));
#else  /* __LP64__ */
                               cprintf("   IMA[0x%04X] = 0x%08lX\n",
                                                zd + IRSEM, LI(zd + IRSEM, 6));
#endif /* __LP64__ */
                               /* Si aucun SWAIT_IM n'a encore ete effectue  */
                               /* sur la voie, le pointeur de wait-queue est */
                               /* nul. Il faut alors ne pas l'utiliser !     */
                               /* (sous peine de segmentation fault !)       */
                               if (LI(zd + IRSEM, 6) != 0)
                                 { SRESET_IM(&LI(zd + IRSEM, 6));
                                 }
#else
                               cprintf("\n");
                               SRESET_IM(&LI(zd + IRSEM, 6));
#endif   /* LINUX */

#ifdef CEVT
                               /* La voie est-elle connectee a un CEVT ? */
                               mm = LI(zd + IRCEVT, 7);
                               if (mm)
                                 {
                                   cevt_signaler(mm, CEVT_SDCABO,
                                                       dst->signal_number,
                                                       CEVT_AVORTE, 0);
                                 }
#endif

                             }
                         }




#ifdef CEVT
                       /* Reveil de tous les CEVT en attente de CC */
                       for (i=0; i<32; i++) /* Balayage adresses RT */
                         {
                           /* Balayage des commandes codees */
                           for (j=0; j < (COCO_MAX + 1); j++)
                             {
                               /* Un CEVT est-il connecte ? */
                               mm = dst->cocor[i][j].cevt;
                               if (mm)
                                 {
                                   /* si oui, signaler la RAZ */
                                   cevt_signaler(mm, CEVT_SDCABO,
                                                       dst->signal_number,
                                                       CEVT_AVORTE, 0);
                                 }
                             }
                         }
#endif


                       /* Reveil de toutes les attentes sur espion seq. */
                       SRESET(&dst->semmon);

                       /* Reveil de toutes les attentes sur horloge IRIG */
                       SRESET(&dst->semirig);

                       break;

              case 2 : dst->raz = 0;
                       break;

              default : RETURN(EINVAL);
             }

          RETURN(OK);




       case ASDCDEF :        /* Definition d'une "voie 1553" */
          // printk("asdcioctl.c entree dans case ASDCDEF \n"); //ajout
          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

          /* En vue determination du "pointeur de tampon application" */
          tampapp = 0;
          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          v->adresse &= 0x1F;
          v->sous_adresse &= 0x1F;
          v->direction &= 0x01;
          v->nmots &= 0x1F;

          /* Il faut reserver au moins un tampon */
          if ((v->ntamp==0) && (v->mode!=RT_VASYNC))
	    { cprintf("Echec 1\n");
	      RETURN(EINVAL);
	    }

	  /* On controle quand meme la validite du mode */
	  if (    (v->mode != RT_VSYNC)
	       && (v->mode != RT_VASYNC)
	       && (v->mode != RT_VSTAT)
	       && (v->mode != RT_VSYNC2)
	     )
	    { cprintf("Echec 8\n");
	      RETURN(EINVAL);
	    }

	  /* Le mode "synchrone 2" est utilisable en transmission seulement */
	  if ((v->mode == RT_VSYNC2) && (v->direction != 1))
	    { cprintf("Echec 9\n");
	      RETURN(EINVAL);
	    }

	  /* RT programme en "espion TR" ? */
	  espion_tr = L(L(PROPTR) + v->adresse) & 2;

	  /* Nombre de tampons a allouer */
	  if (v->mode == RT_VSYNC2)
	    { if (v->adrtamp2 != 0)
	        { nbta = v->adrtamp2;
	        }
	      else
	        { nbta = v->ntamp * 2 + 2;
	        }
	    }
	  else
	    { nbta = v->ntamp;
	    }

          /* Allocation si necessaire de la table des sous_adresses */
          j = L(ATPTR) + v->adresse;
          if (L(j)==0)
             { k = asdcalloc(dst, 64);
               if (k==-1) { cprintf("Echec 2\n");
	                    RETURN(ENOMEM);     /* Plus de memoire */
	                  }
               E(j, k);
               for (i=k; i<k+64; i++) E(i, 0);
             }
            else k = L(j);  /* Preparation suite ... */

          /* Calcul de l'adresse du pointeur de tampon */
          k += (v->direction ? 32 : 0) + v->sous_adresse;

          /* La voie a pu etre deja declaree */
          if (L(k)!=0) { cprintf("Echec 3\n");
	                 RETURN(EADDRINUSE);      /* Voie deja declaree */
	               }

	  /* k = adresse du ptr des tampons */


          /* Allocation si necessaire de la table des filtres */
          /* et de l'image de la table des filtres            */
          j = L(FTPTR) + v->adresse;
          m = L(j);
          if (m==0)
             { m = asdcalloc(dst, 64);
               if (m==-1) { RETURN(ENOMEM);     /* Plus de memoire */
	                  }
               E(j, m);
               for (i=m; i<m+64; i++)
                  { E(i, 0);
                    EI(i, 0, 383);
                  }
             }

          /* Calcul de l'adresse du filtre */
          afiltre = m + v->sous_adresse + ( v->direction ? 32 : 0);

	  /* La voie peut aussi avoir ete declaree, puis inhibee */
	  if (LI(afiltre, 116) != 0)
	     { cprintf("Echec 3-bis\n");
	       RETURN(EADDRINUSE);      /* Voie deja declaree */
	     }


          { /* Des tampons sont maintenant (30/4/02) systematiquement alloues */
            int taille_alloc;
            taille_alloc = (v->adrtamp == 0xFFFF) ? v->nmots : 32;
            if (taille_alloc == 0) taille_alloc = 32;

            /* Allocation du premier tampon */
            j = asdcalloc(dst, taille_alloc + 2);
            if (j==-1) { cprintf("Echec 4\n");
	                 RETURN(ENOMEM);          /* Plus de memoire */
	               }
#ifdef LINUX
            EI(j+IRSEM, 0, 0);        /* Initialisation a 0 imperative */
#endif
            EI(j+IRMEMOF, 0, 0);      /* Initialisation a 0 imperative */
            EI(j+IRMEMOE, 0, 0);      /* Initialisation a 0 facultative */
            EI(j+IRTLET, 0, 0);       /* Initialisation a 0 facultative */
            EI(j+IRAFILTRE, 0, 0);    /* Initialisation a 0 facultative */
	    E(j+1, TRT_LIBRE);
	    EI(j + IRTPREC, 0, 1000);
            if (v->mode != RT_VSYNC2)
              { E(k, j);
              }
            else
              { E(k, RT_FIN2);
              }

            /* Renvoi de l'adresse choisie */
            v->adrtamp = j;

            /* Allocation du ou des tampons suivants */
            if (    (v->mode == RT_VSYNC)
                 || (v->mode == RT_VSTAT)
                 || (v->mode == RT_VSYNC2)
               )
              { /* Voie synchrone ou statique */
                for (i=2; i<=nbta; i++)
                   { l = asdcalloc(dst, taille_alloc + 2);
                     tmpi = l;
                     if (l==-1) { cprintf("Echec 5\n");
	                          RETURN(ENOMEM);     /* Plus de memoire */
		                }
#ifdef LINUX
                     EI(j+IRSEM, 0, 0);
#endif
		     E(l+1, TRT_LIBRE);
                     E(j, l);
                     EI(l+IRTPREC, j, 303);
                     j = l;
                   }
                v->adrtamp2 = 0;

                /* Rebouclage de la chaine des tampons (sauf "asynchrone 2") */
                if (v->mode != RT_VSYNC2)
                  { E(j, L(k));
                    EI(L(k)+IRTPREC, j, 304);
                  }
                else
                  { /* Si "asynchrone 2", pas de rebouclage */
                    E(j, 0);	// Terminaison par 0
                    EI(L(k) + IRTPREC, 0, 390);
                   }
              }
            else
              { /* Voie asynchrone */
                m = asdcalloc(dst, taille_alloc + 2);
                if (m==-1) { cprintf("Echec 6\n");
	                     RETURN(ENOMEM);     /* Plus de memoire */
		           }
		E(m+1, TRT_LIBRE);
                E(m , m);         	/* Pour l'instant, tampons separes */
                EI(m+IRTPREC, j, 305);
                v->adrtamp2 = m;
                tampapp = m;

                /* "Rebouclage" de la chaine des tampons */
                E(j, L(k));
                EI(L(k)+IRTPREC, m, 306);
              }
          }

          /* Programmation du filtre : Inhibition IT voie */
          E(afiltre, L(afiltre) & ~2);

          /* Si asynchrone :                                 */
          /*   si non espion_TR et transmission :            */
          /*   - alors les 2 tampons doivent etre separes    */
          /*   - dans tous les autres cas, les 2 tampons     */
          /*     doivent etre reboucles                      */
          if (v->mode == RT_VASYNC)
            { if (!(!espion_tr && v->direction))
                { /* Rebouclage des 2 tampons */
                  E(v->adrtamp, v->adrtamp2);
                  E(v->adrtamp2, v->adrtamp);
                  tampapp = v->adrtamp;
                }
            }

          /* Initialisation de la memoire image */
          j = L(L(ATPTR) + v->adresse);
          k = j + v->sous_adresse + ( v->direction ? 32 : 0);
          EI(k, v->adrtamp, 307);	/* Memorisation zone de donnees */

          m = v->adrtamp;
          EI(m + IRCMD,   ((v->adresse << 11) & 0xF800)
                        | (v->direction ? 0x400 : 0)
                        | ((v->sous_adresse << 5) & 0x3E0), 308);
          EI(m + IRMODE, v->mode, 309);
          EI(m + IRTCAPP, tampapp ? tampapp : v->adrtamp, 310);
          EI(m + IRNBT, ((v->mode == RT_VASYNC) ? 2 : v->ntamp), 311);
          EI(m + IRSEM, 0, 312);	/* Semaphore voie */
          EI(m + IRCEVT, 0, 313);	/* Pas de connexion CEVT */

          /* Si mode "synchrone 2", initialisation particuliere */
          if (v->mode == RT_VSYNC2)
            {
              EI(m + IRTCH1, 0, 391);
              EI(m + IRTCH2, 0, 392);
              EI(m + IRTFCH1, 0, 393);
              EI(m + IRTFCH2, 0, 394);
              EI(m + IRTFCHINU, v->adrtamp, 395);
              EI(m + IRTCHINU, tmpi, 396);
              EI(m + IRTCACHE, 0, 397);
              EI(m + IRNBT, nbta, 1000);
              EI(m + IRTNMT, v->ntamp, 1000);
              EI(m + IRTNCH1, 0, 1000);
              EI(m + IRTNCH2, 0, 1000);

              /*####################################################*/
              /* Dans ce cas precis, si Espion TR : */
              /* pour le moment, RT non defini !    */
              /* Si, ulterieurementy, RT defini, ecrire ici le code */
              /* de configuration des tampons en espion TR !        */
              /*####################################################*/
            }

          /* Retour des donnees a l'appli appelante */
          VUTIL(zv, v, arg, sizeof(struct asdcvoie));


          /* On laisse le RT dans l'etat ou il etait avant l'appel */
          /* Inhibe (etat initial), espion_TR ou valide ...        */
          RETURN(OK);


          /*****************************************************************/
          /*   ATTENTION : Erreurs ENOMEM ci-avant : dans ces cas, on      */
          /*   devrait liberer la memoire allouee dans la meme operation   */
          /*   ou, autre methode, detecter l'erreur avant la premiere      */
          /*   allocation.                                                 */
          /*****************************************************************/






       case ASDCMODE :        /* Programmation du mode de fonctionnement */
       			      /* d'un RT simule                          */
          // printk("asdcioctl.c entree dans case ASDCMODE \n"); //ajout

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          v->adresse &= 0x1F;

          /* On verifie que les structures de donnees liees */
          /* au RT existent bien                            */

          /* La table des sous_adresses existe-t-elle ? */
          mm = L(L(ATPTR) + v->adresse);
          if (mm == 0)
		    { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
		    }

          /* On ne cherche pas a savoir quel est l'etat initial du RT */
          /* On applique directement le nouveau mode demande          */
          switch (v->nmots)
            { case RT_INHIBE :
                    /* Inhibition du RT dans la table des Mots d'Etat */
                    E(L(SWTPTR) + v->adresse, 0);

                    /* Inhibition de l'espion temps reel dans la */
                    /* table des protocoles                      */
                    tmpi = L(PROPTR) + v->adresse;
	            E(tmpi, L(tmpi) & ~2);

	            break;

              case RT_ESPION :
                    /* Inhibition du RT dans la table des Mots d'Etat */
                    E(L(SWTPTR) + v->adresse, 0);

                    /* Validation de l'espion temps reel dans la */
                    /* table des protocoles                      */
                    tmpi = L(PROPTR) + v->adresse;
	            E(tmpi, L(tmpi) | 2);

	            /* Balayage des voies ASYNC et des voies SYNC2  */
	            /* en transmission pour eventuelle modif. de la */
	            /* structure des tampons                        */
	            for (i=0; i<32; i++)
	               { int modevoie;

	                 j = L(mm + i + 32);
	                 if (j == 0) continue;

	                 /* Voie ASYNC ou SYNC2 ? */
	                 zd = LI(mm + i + 32, 8);
	                 modevoie = LI(zd+IRMODE, 9);

	                 if (modevoie == RT_VASYNC)
	                   { /* Les 2 tampons sont-ils separes ? */
	                     if (L(j) == j)
	                       { /* Oui ==> on les enchaine */
	                         tmpi = LI(zd+IRTCAPP, 10);
	                         E(j, tmpi);
	                         E(tmpi, j);
	                       }
	                   }

	                 if (modevoie == RT_VSYNC2)
	                   { /* Memorisation adresse courante */
	                     EI(zd+IRTCACHE, j, 397);
	                     /* Et inhibition RT (provisoire ...?) */
	                     E(mm + i + 32, 0);
	                     usec_sleep(1);
	                     E(mm + i + 32, 0);
	                   }
	               }

	            break;

              case RT_ABONNE :
                    /* Validation du RT dans la table des Mots d'Etat */
                    if (v->adresse)
                      { /* Si adresse != 0, "status 1553 standard" */
                        E(L(SWTPTR) + v->adresse, v->adresse << 11);
                      }
                    else
                      { /* Si adresse = 0, on met bit ME a 1            */
                        /* (en principe, ca n'affecte pas le mot d'etat */
                        /*  renvoye...)                                 */
                        E(L(SWTPTR) + v->adresse, 0x400);
                      }

                    /* Inhibition de l'espion temps reel dans la */
                    /* table des protocoles                      */
                    tmpi = L(PROPTR) + v->adresse;
	            E(tmpi, L(tmpi) & ~2);

	            /* Balayage des voies ASYNC en transmission pour */
	            /* eventuelle modif. de la structure des tampons */
	            for (i=0; i<32; i++)
	               { int modevoie;

	                 j = L(mm + i + 32);
	                 if (j == 0) continue;

	                 /* Voie ASYNC ou SYNC2 ? */
	                 zd = LI(mm +i + 32, 11);
	                 modevoie = LI(zd+IRMODE, 12);

	                 if (modevoie == RT_VASYNC)
	                   { /* Les 2 tampons sont-ils separes ? */
	                     if (L(j) != j)
	                       { /* Non ==> on les separe */
	                         EI(zd+IRTCAPP, L(j), 314);
	                         E(j, j);
	                       }
	                   }

	                 if (modevoie == RT_VSYNC2)
	                   { int cache;
	                     /* Une chaine de tampons est-elle cachee ? */
	                     cache = LI(zd+IRTCACHE, 112);
	                     if (cache)
	                       { /* Oui ==> on la restore */
	                         E(mm + i + 32, cache);
	                         EI(zd+IRTCACHE, 0, 212);
	                       }
	                   }
	               }

	            break;

              default :
                    RETURN(EINVAL);
            }

          VUTIL(zv, v, arg, sizeof(struct asdcvoie));
          RETURN(OK);









/*************************************************************/
/* Quelques macros pour simplifier la comprehension des      */
/* operations sur les chaines de tampons en mode SYNC2 :     */
/*                                                           */
/* Remarque : Le pointeur du tampon suivant est range dans   */
/*            la memoire d'echange du coupleur.              */
/*            Celui du tampon precedent est range dans la    */
/*            memoire image.                                 */

/* Lecture de l'adresse du tampon suivant */
#define LTSUIV(courant)			L(courant)

/* Ecriture de l'adresse du tampon suivant */
#define ETSUIV(courant, valeur)		E(courant, valeur)

/* Lecture de l'adresse du tampon precedent */
#define LTPREC(courant)			LI(courant + IRTPREC, 1001)

/* Ecriture de l'adresse du tampon precedent */
#define ETPREC(courant, valeur)		EI(courant + IRTPREC, valeur, 1001)

/*                                                           */
/*************************************************************/





                       /*****************************************************/
        case ASDCECR : /* Ecriture dans un tampon :                         */
                       /*  - Un tampon prepare (c'est a dire ecrit) et      */
                       /*  non emis est reconnaissable par flag=TRT_NOUVEAU */
                       /*  - Le nouveau tampon est ecrit a la suite de      */
                       /*  tous les tampons "NOUVEAU" existants.            */
                       /*  - Si tous les tampons existants sont "NOUVEAU",  */
                       /*  rien n'est ecrit et une erreur ENOSR est         */
                       /*  renvoyee.                                        */
		       /*****************************************************/
          // printk("asdcioctl.c entree dans case ASDCECR \n"); //ajout
          TDUTIL(arg, sizeof(struct asdctampon));
          DUTIL(zt, t, arg, sizeof(struct asdctampon));
          TVUTIL(zt, t, arg, sizeof(struct asdctampon));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          t->v.adresse &= 0x1F;
          t->v.sous_adresse &= 0x1F;
          /* t->v.direction : Ecriture ==> imperativement TRANSMISSION */
          t->v.direction = 1;
          if (t->nbr > 32)    t->nbr = 32;

          /* La table des sous_adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + t->v.adresse)) == 0)
	    { cprintf("Echec 1\n");
              RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* Pointeur vers zone des donnees en memoire image */
          zd = LI(i + t->v.sous_adresse + 32, 13);
          if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
            { cprintf("Echec zd (zd[0x%X] = 0x%X)\n",
                          i + t->v.sous_adresse + 32, zd);
              /* Correspond a une tentative d'ecriture de donnees */
              /* dans une voie en lecture !                       */
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }


          /* Mode de la voie (synchrone, asynchrone, etc...) */
          m = LI(zd + IRMODE, 15);

          /* La voie existe-t-elle ? */
          j = L(i + t->v.sous_adresse + 32);
          if ((j == 0) && (m != RT_VSYNC2))
            { cprintf("Echec 2\n");
              RETURN(EADDRNOTAVAIL); /* La voie n'a jamais ete declaree ! */
            }



          /* Le contenu de la memoire image semble-t-il correct ? */
          cmd = (t->v.adresse << 11) | 0x400 | (t->v.sous_adresse << 5);
          if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
            { cprintf("Echec 4\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }


          /* Validation, si necessaire, des ITs associees a la voie */
	  if (t->f == SDC_ATTENDRE)
            { int ef;

              /* Pointeur table des filtres associee a adresse */
              ef = L(L(FTPTR) + t->v.adresse);

              /* Adresse du filtre */
              ef += t->v.sous_adresse + ( t->v.direction ? 32 : 0);

              /* Programmation du filtre : Validation IT voie */
              E(ef, L(ef) | 2);
            }



          /* RT en mode espion ? */
          espion_tr = L(L(PROPTR) + t->v.adresse) & 2;

          /* Si oui, operation invalide ! */
          if (espion_tr)
            { cprintf("Echec 5\n");
              RETURN(EXDEV);
            }




          /* Si mode "synchrone 2", traitement particulier */
          if (m == RT_VSYNC2)
            { int pch;
              int ttrait;	/* Indicateur "tampon traite" */
              			/* (et, donc, recyclable)     */

              /* Memorisation d'un debordement potentiel */
              debordement = (j == RT_FIN2);

              /* Extraction du bit SDC_PROT2 */
              prot2 = t->f & SDC_PROT2;
              t->f &= ~SDC_PROT2;

              switch (t->f)
                {
                  case SDC_RAZRAZ :
                  case SDC_RAZ :
                    /* Normalement, on devrait regarder si un traitement */
                    /* n'est pas en cours : on fait l'impasse sur cette  */
                    /* eventualite ...                                   */

                    /*** RAZ :                                           ***/
                    /*** Tous les tampons sont mis dans la chaine des    ***/
                    /*** tampons disponibles (c'est a dire "inutilises") ***/

                    /*####################################
                         MODIF. A PREVOIR :
                         Pour eviter les pbs, tous les
                         tampons devraient avoir leur
                         ptr "suivant" en mem. echange
                         pointant vers 0 ou vers RT_FIN2
                      ####################################*/

                    if (!prot2)
                      { /* Accrochage chaine 2 aux tampons inutilises */
                        pch = LI(zd + IRTCH2, 1000);
                        if (pch)
                          { EI( LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                            LI(zd + IRTFCH2, 1000), 1000);
                            EI(zd + IRTFCHINU, LI(zd + IRTCH2, 1000), 1000);
                            EI(zd + IRTCH2, 0, 1000);
                            EI(zd + IRTFCH2, 0, 1000);
                            EI(zd + IRTNCH2, 0 , 1000);
                          }
                      }

                    /* Accrochage chaine 1 aux tampons inutilises */
                    pch = LI(zd + IRTCH1, 1000);
                    if (pch)
                      { EI( LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                        LI(zd + IRTFCH1, 1000), 1000);
                        EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                        EI(zd + IRTCH1, 0, 1000);
                        EI(zd + IRTFCH1, 0, 1000);
                        EI(zd + IRTNCH1, 0 , 1000);
                        E(i + t->v.sous_adresse + 32, RT_FIN2);
                      }

                    /* Si SDC_RAZRAZ, on a termine ! */
                    if (t->f == SDC_RAZRAZ) RETURN(OK);

                    /* On est dans le cas SDC_RAZ        */
                    /*      ==> reste a ecrire un tampon */


                  case SDC_NONBLOQ :
                  case SDC_ATTENDRE :
                     // printk("entree dans case SDC_ATTENDRE switch "
                     //        "(t->f)ASDCECR \n"); //ajout
                     ttrait = 0;

                    /* Y a-t-il des tampons deja lus a recycler ?  */
                    tmpi = L(i + t->v.sous_adresse + 32);
                    z = LI(zd + IRTCH1, 1000);
                    if (tmpi != z)
                      { /* Il y a des tampons a recycler ! */
                        ttrait = 1;
                      }
                    else
                      {
                        /* Peut-on encore ajouter un tampon a la chaine ? */
                        if (    (LI(zd + IRTNCH1 , 1000)
                                      >=  LI(zd + IRTNMT , 1000))
                             || (LI(zd + IRTCHINU, 1000) == 0)
                           )
                          { /* Chaine pleine ou plus de tampons disponibles */
                            if (t->f == SDC_NONBLOQ)
                              { RETURN(ENOSR);
                              }

                            /* Attente de la liberation d'un tampon */

	                    /* Semaphore ou attendre */
	                    oreiller = &LI(zd + IRSEM, 1000);

                            /* -------------------------------------------- */
                            /* ---   Debut section critique   ----- vvvvvvv */
                            DISABLE(s);

                            tmpi = 0; /* Pour pouvoir passer tests  */
                                      /* en sortie section critique */
                                      /* si swait non appele        */

                            /* Tampon (de nouveau) disponible */
                            /* pour recyclage ?               */
                            z = LI(zd + IRTCH1, 1000);
                            if (z == L(i + t->v.sous_adresse + 32))
                              { /* Plus de tampon dispo ! */
                                /* Demasquage eventuel IT sur ABI */
                                if ((++dst->nombre_it)==1)
                                  {    /***********************************/
                                       /* DEMASQUAGE IT ABI doit etre ICI */
                                       /***********************************/
                                  }

                                // dprintf("ASDC : j'attend l'IT !\n");
                                // printk("ASDC : j'attend l'IT !\n"); //ajout
                                tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
          			// printk("ASDC : IT arrivee !\n");
                                //ajout
                                if (!(--dst->nombre_it))
                                  {  /*********************************/
                                     /* MASQUAGE IT ABI doit etre ICI */
                                     /*********************************/
                                  }

                              }  /* Fin du second "Si tampons-traites" */

                            RESTORE(s);
                            /* ---   Fin section critique   ------- ^^^^^^^ */
                            /* -------------------------------------------- */

                            if (tmpi) /* Passage semaphore du a un signal ? */
                              { RETURN(EINTR);
                              }

	                    if (dst->raz) /* Passage semaphore du a RAZ ? */
	                      { RETURN(ENETRESET);
	                      }

                            ttrait = 1;	  /* Il y a des tampons a recycler */
                          }
                        }	/* Fin du premier "Si tampons-traites" */


                    /* Recyclage eventuel des tampons deja ecrits */
                    if (ttrait)
                      { int tcour;	/* Adr. tampon courant */

                        tcour = L(i + t->v.sous_adresse + 32);
                        if (tcour == RT_FIN2)
                          { /* La chaine 1 a ete entierement traitee */
                            EI(LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                          LI(zd + IRTFCH1, 1000), 1000);
                            EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                            EI(zd + IRTCH1, 0, 1000);
                            EI(zd + IRTFCH1, 0, 1000);
                            EI(zd + IRTNCH1, 0, 1000);
                          }
                        else
                          { /* La chaine 1 n'est pas encore */
                            /* entierement traitee          */

                            int tprec;	/* Adr. tampon precedent */
                            int n1;

                            tprec = LI(tcour + IRTPREC, 1000);
                            EI(tcour + IRTPREC, 0, 1000);
                            { /* Comptage des tampons a recycler */
                              int p;
                              n1 = 0;
                              p = tprec;
                              while(p)
                                { n1++;
                                  p = LI(p + IRTPREC, 1000);
                                }
                            }

                            EI(LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                                     tprec, 1000);
                            EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                            EI(zd + IRTCH1, tcour, 1000);
                            EI(tcour + IRTPREC, 0, 1000);
                            EI(zd + IRTNCH1, LI(zd + IRTNCH1, 1000) - n1,
			                                              1000);
                          }
                      }


                    /* Il reste a ecrire le tampon                        */
                    /* (et on est maintenant certain de pouvoir le faire) */

                    /* Retrait d'un tampon de la chaine des t. inutilises */
                    l = LI(zd + IRTCHINU, 1000);
                    EI (zd + IRTCHINU, LI(l + IRTPREC, 1000), 1000);

                    /* Ecriture du tampon */
  		    for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
  		    E(l+1, TRT_NOUVEAU);
  		    E(l, RT_FIN2);

  		    /* Insertion du tampon dans chaine 1 */
  		    /* Chainage direct : */
  		    E(l, 0);
  		    if (LI(zd + IRTCH1, 1000) == RT_FIN2)
  		      { /* Chaine 1 est vide ! */
  		        EI(zd + IRTCH1, l, 1000);
  		        EI(zd + IRTFCH1, l, 1000);
  		        EI(l + IRTPREC, 0, 1000);
  		        E(i + t->v.sous_adresse + 32, l);
  		      }
  		    else
  		      { /* Chaine 1 non vide */
  		        E(LI(zd + IRTFCH1, 1000), l);
  		        EI(l + IRTPREC, LI(zd + IRTFCH1, 1000), 1000);
  		        EI(zd + IRTFCH1, l, 1000);
  		      }

  		    /* Incrementation du nombre des tampons utilises */
  		    EI(zd + IRTNCH1, LI(zd + IRTNCH1 , 1000) + 1, 1000);

                    /* On n'a aucun moyen de savoir si un debordement */
                    /* a vraiment eu lieu (il est peut-etre en train  */
                    /* d'avoir lieu, mais on n'en sait rien encore    */
                    /* car le firmware ne permet pas de savoir si un  */
                    /* transfert est en cours ...)                    */
                    /* On avertit donc si un debordement est          */
                    /* potentiellement possible (si le tampon courant */
                    /* est le dernier de la chaine) ...               */
                    /* Ce n'est donc pas parce que le fanion          */
                    /* "debordement" est monte qu'un debordement a eu */
                    /* lieu.                                          */
                    /* Un vrai debordement se traduit par un bit      */
                    /* erreur monte dans le status.                   */
                    /*                                                */
                    /* Remarquons que le fanion "debordement" sera    */
                    /* toujours monte lors de la premiere ecriture    */
                    /* dans la voie !                                 */

                    if (debordement) t->f |= SDC_DEBORD;
                    VUTIL(zt, t, arg, sizeof(struct asdctampon));

                    RETURN(OK);

                  default :
                    RETURN(EINVAL);
                }
            }



          /* Ci-dessous, modes autres que "synchrone 2" */

          /* Tampons separes ? */
          tmpi = L(i + t->v.sous_adresse + 32);
          z = ((LI(zd + IRNBT, 16) == 2) && (L(j) == j));
          					 /* z vrai si t. separes */

          /* Traitement de l'appel si tampons separes */
          if (z)
            {
              l = LI(zd + IRTCAPP, 17);	/* Adr. tampon appli courant */

              /* Copie des donnees */
              for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
	      E(l+1, TRT_NOUVEAU);

	      /* Forcage du basculement des tampons */
	      EI(zd + IRTCAPP, j, 315);
	      E(i + t->v.sous_adresse + 32, l);
	      usec_sleep(1);
	      E(i + t->v.sous_adresse + 32, l);

              VUTIL(zt, t, arg, sizeof(struct asdctampon));
              RETURN(OK);
            }

          /* Traitement selon le mode de l'appel si tampons chaines */
          debordement = 0;

          /* Mode de l'ecriture :                          */
          /*    SDC_ATTENDRE est remplace par SDC_NON_BLOQ */
          /*    si la voie est "statique"                  */
          if ((t->f == SDC_ATTENDRE) && (m == RT_VSTAT))
            { mde = SDC_NONBLOQ;
            }
          else
            { mde = t->f;
            }

          switch (mde)
            { case SDC_RAZRAZ :
              case SDC_RAZ :
                 /* Remise a "zero" de tous les tampons */
                 tmpi = j;
                 for (l=0; l<LI(zd+IRNBT, 18); l++)
                   { E(tmpi+1, TRT_LIBRE);
                     tmpi = L(tmpi);
                   }
                 /* Mise en coherence des pointeurs firmware et appli */
                 EI(zd + IRTCAPP, j, 316);

                 /* Fin si RAZRAZ */
                 if (t->f == SDC_RAZRAZ)
                   {  RETURN(OK);    /* VUTIL inutile : rien a remonter ! */
                   }

              case SDC_NONBLOQ :
                 l = LI(zd + IRTCAPP, 19);   /* Adr. tampon appli courant */

                 /* Si voie statique, on ignore les eventuels debordements */
                 if (m != RT_VSTAT)
                   { debordement =  ((L(l + 1) & 0xFFFF) != TRT_LIBRE);

                     /* Tampon dispo ? */
                     if ((L(L(l) + 1) & 0xFFFF) == TRT_NOUVEAU)
                       { /* Plus de tampon dispo ! */
                         if (debordement) t->f |= SDC_DEBORD;
                         VUTIL(zt, t, arg, sizeof(struct asdctampon));
                         RETURN(ENOSR);
                       }
                   }

                 /* Ecriture du tampon */
  		 for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
  		 E(l+1, TRT_NOUVEAU);

  		 /* Mise a jour du pointeur application */
  		 tmpi = L(l);
  		 EI(zd + IRTCAPP, tmpi, 317);
  		 E(tmpi + 1, TRT_LIBRE);

                 if (debordement) t->f |= SDC_DEBORD;
                 VUTIL(zt, t, arg, sizeof(struct asdctampon));
                 RETURN(OK);


              case SDC_ATTENDRE :
                     // printk("entree dans case SDC_ATTENDRE"
                     // " switch(mde) ASDCECR \n"); //ajout
                     l = LI(zd + IRTCAPP, 20);	/* Adr. tamp. appli courant */

                 debordement =  ((L(l + 1) & 0xFFFF) != TRT_LIBRE);

	         /* Semaphore ou attendre */
	         oreiller = &LI(zd + IRSEM, 21);

                 dprintf("sleep : oreiller RT(%d,%d,%c)\n",
	                 t->v.adresse, t->v.sous_adresse,
	                 t->v.direction ? 'T' : 'R');

                 /* ---------------------------------------------------- */
                 /* ---   Debut section critique   ------------- vvvvvvv */
                 DISABLE(s);

                 tmpi = 0;	/* Pour pouvoir passer tests en sortie  */
                                /* section critique si swait non appele */

                 if ((L(L(l) + 1) & 0xFFFF) == TRT_NOUVEAU)
		                         /* Tampon disponible ? */
                   { /* Plus de tampon dispo ! */

                     /* Demasquage eventuel IT sur ABI */
                     if ((++dst->nombre_it)==1)
                       {    /***********************************/
                            /* DEMASQUAGE IT ABI doit etre ICI */
                            /***********************************/
                       }

                     dprintf("Oreiller = I(%d) ==> 0x%X\n",
                              zd + IRSEM, oreiller);
                     // dprintf("ASDC : j'attend l'IT !\n");
                   // printk("ASDC : j'attend l'IT !\n"); //ajout
		     tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
          	   // printk("ASDC : apr�s le \"j'attend l'IT\" !\n"); //ajout
                     if (!(--dst->nombre_it))
                       {  /*********************************/
                          /* MASQUAGE IT ABI doit etre ICI */
                          /*********************************/
                       }
                   }

                 RESTORE(s);
                 /* ---   Fin section critique   --------------- ^^^^^^^ */
                 /* ---------------------------------------------------- */

                 if (tmpi)	/* Passage semaphore du a un signal ? */
                   { if (debordement) t->f |= SDC_DEBORD;
                     VUTIL(zt, t, arg, sizeof(struct asdctampon));
                     RETURN(EINTR);
                   }

	         if (dst->raz) /* Passage du semaphore du a une RAZ ? */
	           { if (debordement) t->f |= SDC_DEBORD;
                     VUTIL(zt, t, arg, sizeof(struct asdctampon));
                     RETURN(ENETRESET);
	           }


                 /* Ecriture du tampon */
  		 for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
  		 E(l+1, TRT_NOUVEAU);

  		 /* Mise a jour du pointeur application */
  		 tmpi = L(l);
  		 EI(zd + IRTCAPP, tmpi, 318);
  		 E(tmpi + 1, TRT_LIBRE);

  		 if (debordement) t->f |= SDC_DEBORD;
                 VUTIL(zt, t, arg, sizeof(struct asdctampon));
                 RETURN(OK);


              default :
                 RETURN(EINVAL);
            }







                        /****************************************/
        case ASDCECR2 : /* Ecriture d'un tampon en chaine 2     */
                        /*  - Mode "synchrone 2" seulement      */
			/****************************************/
          // printk("asdcioctl.c entree dans case ASDCECR2 \n"); //ajout
          TDUTIL(arg, sizeof(struct asdctampon));
          DUTIL(zt, t, arg, sizeof(struct asdctampon));
          TVUTIL(zt, t, arg, sizeof(struct asdctampon));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          t->v.adresse &= 0x1F;
          t->v.sous_adresse &= 0x1F;
          /* t->v.direction : Ecriture ==> imperativement TRANSMISSION */
          t->v.direction = 1;
          if (t->nbr > 32)    t->nbr = 32;

          /* La table des sous_adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + t->v.adresse)) == 0)
	    { cprintf("Echec 1'\n");
              RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* Pointeur vers zone des donnees en memoire image */
          zd = LI(i + t->v.sous_adresse + 32, 13);
          if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
            { cprintf("Echec zd'\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          /* Mode de la voie (synchrone, asynchrone, etc...) */
          m = LI(zd + IRMODE, 15);

          /* "Synchrone 2" est le seul mode autorise */
          if (m != RT_VSYNC2)
            { RETURN(ENOEXEC);
            }

          /* La voie existe-t-elle ? */
          /* Ce test n'est pas praticable en mode synchrone 2 ...
             j = L(i + t->v.sous_adresse + 32);
             if ((j == 0) && (m != RT_VSYNC2))
               { cprintf("Echec 2'\n");
                 RETURN(EADDRNOTAVAIL);
               }
          */


          /* Le contenu de la memoire image semble-t-il correct ? */
          cmd = (t->v.adresse << 11) | 0x400 | (t->v.sous_adresse << 5);
          if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
            { cprintf("Echec 4'\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }


          /* RT en mode espion ?  ==>  Est-ce vraiment possible ??? */
          espion_tr = L(L(PROPTR) + t->v.adresse) & 2;

          /* Si oui, operation invalide ! */
          if (espion_tr)
            { cprintf("Echec 5'\n");
              RETURN(EXDEV);
            }




          /* Traitement proprement dit */
            { int pch;

              switch (t->f)
                {
                  case SDC_RAZRAZ :
                  case SDC_RAZ :
                    /* Normalement, on devrait regarder si un traitement */
                    /* n'est pas en cours : on fait l'impasse sur cette  */
                    /* eventualite ...                                   */

                    /*** RAZ :                                           ***/
                    /*** Les tampons ch 2 sont mis dans la chaine des    ***/
                    /*** tampons disponibles (c'est a dire "inutilises") ***/

                    /* Accrochage chaine 2 aux tampons inutilises */
                    pch = LI(zd + IRTCH2, 1000);
                    if (pch)
                      { EI( LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                        LI(zd + IRTFCH2, 1000), 1000);
                        EI(zd + IRTFCHINU, LI(zd + IRTCH2, 1000), 1000);
                        EI(zd + IRTCH2, 0, 1000);
                        EI(zd + IRTFCH2, 0, 1000);
                        EI(zd + IRTNCH2, 0 , 1000);
                      }


                    /* Si SDC_RAZRAZ, on a termine ! */
                    if (t->f == SDC_RAZRAZ) RETURN(OK);

                    /* On est dans le cas SDC_RAZ        */
                    /*      ==> reste a ecrire un tampon */


                  case SDC_NONBLOQ :

                      {
                        /* Peut-on encore ajouter un tampon a la chaine ? */
                        if (    (LI(zd + IRTNCH2 , 1000)
                                      >=  LI(zd + IRTNMT , 1000))
                             || (LI(zd + IRTCHINU, 1000) == 0)
                           )
                          { /* Chaine pleine ou plus de tampons disponibles */
                            RETURN(ENOSR);
                          }
                      }


                    /* Il reste a ecrire le tampon                     */
                    /* (on est maintenant certain de pouvoir le faire) */

                    /* Retrait d'un tampon de la chaine des t. inutilises */
                    l = LI(zd + IRTCHINU, 1000);
                    EI (zd + IRTCHINU, LI(l + IRTPREC, 1000), 1000);

                    /* Ecriture du tampon */
  		    for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
  		    E(l+1, TRT_NOUVEAU);
  		    E(l, RT_FIN2);

  		    /* Insertion du tampon dans chaine 2 */
  		    /* Chainage direct : */
  		    if (LI(zd + IRTCH2, 1000) == 0)
  		      { /* Chaine 2 est vide ! */
  		        EI(zd + IRTCH2, l, 1000);
  		        EI(zd + IRTFCH2, l, 1000);
  		        EI(l + IRTPREC, 0, 1000);
  		      }
  		    else
  		      { /* Chaine 2 non vide */
  		        E(LI(zd + IRTFCH2, 1000), l);
  		        EI(l + IRTPREC, LI(zd + IRTFCH2, 1000), 1000);
  		        EI(zd + IRTFCH2, l, 1000);
  		      }

  		    /* Incrementation du nombre des tampons utilises */
  		    EI(zd + IRTNCH2, LI(zd + IRTNCH2 , 1000) + 1, 1000);

                    RETURN(OK);

                  default :
                    RETURN(EINVAL);
                }
            }




                        /**************************************************/
        case ASDCEECR : /* Echange des chaines 1 (active) et 2 (inactive) */
                        /*  - Mode "synchrone 2" seulement                */
			/**************************************************/

          // printk("asdcioctl.c entree dans case ASDCEECR \n");   //ajout
          TDUTIL(arg, sizeof(struct asdctampon));
          DUTIL(zt, t, arg, sizeof(struct asdctampon));
          TVUTIL(zt, t, arg, sizeof(struct asdctampon));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          t->v.adresse &= 0x1F;
          t->v.sous_adresse &= 0x1F;
          /* t->v.direction : Ecriture ==> imperativement TRANSMISSION */
          t->v.direction = 1;

          /* La table des sous_adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + t->v.adresse)) == 0)
	    { cprintf("Echec 1\"\n");
              RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* Pointeur vers zone des donnees en memoire image */
          zd = LI(i + t->v.sous_adresse + 32, 13);
          if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
            { cprintf("Echec zd\"\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          /* Mode de la voie (synchrone, asynchrone, etc...) */
          m = LI(zd + IRMODE, 15);

          /* "Synchrone 2" est le seul mode autorise */
          if (m != RT_VSYNC2)
            { RETURN(ENOEXEC);
            }



          /* La voie existe-t-elle vraiment ? */
          //j = L(i + t->v.sous_adresse + 32);
          //   Lecture de j deplace dans la section critique

          /* Ce test n'est sans doute pas adapte au mode "synchrone 2"
          //if (j == 0)
          //     { cprintf("Echec 2'\n");
          //       RETURN(EADDRNOTAVAIL);
          //     }
          */




          /* Le contenu de la memoire image semble-t-il correct ? */
          cmd = (t->v.adresse << 11) | 0x400 | (t->v.sous_adresse << 5);
          if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
            { cprintf("Echec 4'\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }


          /* RT en mode espion ? [Est-ce vraiment possible ?] */
          espion_tr = L(L(PROPTR) + t->v.adresse) & 2;

          /* Si oui, operation invalide ! */
          if (espion_tr)
            { cprintf("Echec 5'\n");
              RETURN(EXDEV);
            }




          /* Traitement proprement dit */
            { int irtch1, irtfch1, irtnch1;
              int irtch2, irtfch2, irtnch2;
              int irtfchinu;
              int memo1, memo2, pfw;
              int rtsa_cur1;

              rtsa_cur1 =   ((t->v.adresse << 11) & 0xF800)
                          | ((t->v.sous_adresse << 5) & 0x01E0)
                          | 1;


              /*******************************************/
              /* Debut section critique                  */
              /*   Pour garantir une execution continue  */
              /*   et, donc, relativement rapide du code */
              /*   ci-dessous ...                        */
              /*******************************************/
              DISABLE(s);


              /* Lecture des pointeurs initiaux */
              irtch1 = LI(zd + IRTCH1 , 1000);
              irtch2 = LI(zd + IRTCH2 , 1000);
              irtfchinu = LI(zd + IRTFCHINU , 1000);
              irtfch1 = LI(zd + IRTFCH1 , 1000);
              irtfch2 = LI(zd + IRTFCH2 , 1000);
              irtnch1 = LI(zd + IRTNCH1 , 1000);
              irtnch2 = LI(zd + IRTNCH2 , 1000);


              /* Tampon courant (pointe par le firmware) */
              j = L(i + t->v.sous_adresse + 32);

              /* Ce tampon est-il utilise par le firmware ? */
              if ( ((short) j == L(RTSA_BUF)) && (L(RTSA_CUR) & 1) )
                {
                  /* Tampon en cours acces par firmware */

                  /* La chaine 2 est accrochee au tampon actif */
                  memo1 = LTSUIV(j);
                  ETSUIV(j, irtch2);

                  /* Le firwmare accede-t-il toujours a notre RTSA ? */
                  if (L(RTSA_CUR) == (short) rtsa_cur1)
                    {
                      /* Oui */

                      /* Et toujours au meme tampon ?      */
                      /* (Cela va sans doute de soi, mais  */
                      /* on ne sait jamais ...)            */
                      memo2 = L(RTSA_BUF);
                      if ((short) memo2 == (short) j)
                        {
                          /* Oui */

                          /* Dans ce cas, il n'y a rien a faire */
                          /* de particulier ...                 */
                        }
                      else
                        {
                          /* Non : le tampon actif a change */

                          /* Compte tenu du gap inter-messages et du */
                          /* temps de transfert du mot de commande   */
                          /* suivant, c'est invraisemblable :        */
                          /*       ==> Affichage d'une erreur !      */
                          KPRINTF("asdc:ioctl(ASDCEECR): "
                                  "Changement inattendu du tampon actif !\n"
                                  "\tRT%d,%d : 0x%04X --> )x%04X\n",
                                  t->v.adresse, t->v.sous_adresse, j, memo2);
                          RESTORE(s);  /* Fin section critique avant sortie */
                          RETURN(EDOM);
                        }
                    }
                  else
                    {
                      /* Non : Le RTSA initial n'est plus actif */

                      /* Il faut alors verifier que le firmware    */
                      /* pointe bien vers le prochain tampon       */
                      /* (il y a un risque d'erreur en cas         */
                      /* d'acces simultane au pointeur du tampon   */
                      /* suivant par le firmware et par le driver  */
                      /* au moment de l'accrochage de la chaine 2) */

                      memo2 = L(i + t->v.sous_adresse + 32);
                      if ((short) memo2 != (short) irtch2)
                        {
                          /* Le firmware pointe le mauvais tampon ! */
                          /*   ==> On force le tampon courant !     */
                          E(i + t->v.sous_adresse + 32, irtch2);
                        }
                    }

                  /* Fin du chainage :                               */
                  /*   - chaine 2 avec tampon courant (ou excourant) */
                  /*   - debut chaine 1 avec fin chaine 1            */

                  if (irtch2) ETPREC(irtch2, j);

                  memo2 = LTPREC(j);
                  ETPREC(j, 0);
                  if (memo1) ETPREC(memo1, memo2);
                  if (memo2) ETSUIV(memo2, memo1);

                  irtch2 = j;
                  irtnch2++;
                  irtnch1--;

                }
              else
                {
                  /* Tampon non en cours d'acces par firmware */

                  /* La chaine 2 remplace la partie encore */
                  /* inutilisee de la chaine chaine 1      */

                  memo1 = j;
                  E(i + t->v.sous_adresse + 32, irtch2);

                  /* Le firwmare accede-t-il maintenant a notre RTSA ? */
                  if (L(RTSA_CUR) == (short) rtsa_cur1)
                    {
                       /* Oui, mais auquel des 2 tampons ? */
                       /* (l'ancien ou le nouveau ?)       */
                       if (L(RTSA_BUF) == (short) memo1)
                         {
                           int t_avant, t_apres;

                           /* L'ancien tampon est actif !         */
                           /* ==> Il faut raccrocher la chaine 2, */
                           /*     encore inutilisee, a ce tampon. */

                           /* Comme l'entree en activite de ce tampon est   */
                           /* tres recente, nous disposons d'au moins 20 us */
                           /* pour effectuer l'echange, ce qui devrait etre */
                           /* suffisant ...                                 */

                           /* Reconnexion du firmware au tampon actif */
                           E(i + t->v.sous_adresse + 32, memo1);

                           /* Retrait de ce tampon actif de la chaine 1 */
                           /* (l'ordre des autres tampons est conserve) */
                           t_avant = LTPREC(memo1);
                           t_apres = LTSUIV(memo1);
                           if (t_avant) ETSUIV(t_avant, t_apres);
                                   else irtch1 = t_apres;
                           if (t_apres) ETPREC(t_apres, t_avant);
                                   else irtfch1 = t_apres;
                           irtnch1--;

                           /* Connexion de la chaine 2 au tampon actif */
                           ETPREC(memo1, 0);
                           ETSUIV(memo1, irtch2);
                           ETPREC(irtch2, memo1);

                           /* Inclusion du tampon actif dans la chaine 2 */
                           irtch2 = memo1;
                           irtnch2++;
                         }
                       else
                         {
                           /* Le nouveau tampon est actif !       */
                           /* ==> tout peut continuer comme prevu */
                         }
                    }
                 }


              /* Les tampons ont ete echanges, il reste a echanger les     */
              /* valeurs des pointeurs des chaines 1 et 2 (irtch1, etc...) */
              memo1 = irtch1;
              irtch1 = irtch2;
              irtch2 = memo1;
              memo1 = irtfch1;
              irtfch1 = irtfch2;
              irtfch2 = memo1;
              memo1 = irtnch1;
              irtnch1 = irtnch2;
              irtnch2 = memo1;


              /* Recyclage de la chaine 2 (ancienne chaine 1) */
              if (irtfchinu)
                { ETPREC(irtfchinu, irtfch2);
                  irtfchinu = irtch2;
                }
              else
                { irtfchinu = irtch2;
                  EI(zd + IRTCHINU, irtfch2, 1000);
                }
              irtch2 = irtfch2 = 0;
              irtnch2 = 0;

              /* Rangement des pointeurs modifies */
              EI(zd + IRTCH1, irtch1, 1000);
              EI(zd + IRTCH2, irtch2, 1000);
              EI(zd + IRTFCH1, irtfch1, 1000);
              EI(zd + IRTFCH2, irtfch2, 1000);
              EI(zd + IRTNCH1, irtnch1, 1000);
              EI(zd + IRTNCH2, irtnch2, 1000);
              EI(zd + IRTFCHINU, irtfchinu, 1000);


              RESTORE(s);
              /*******************************************/
              /* Fin de la section critique              */
              /*******************************************/

            }

          RETURN(OK);





        case ASDCEECRF : /* Remplacement des N tampons inutilis�s */
	                 /* de la chaine 1 par les N derniers     */
                         /* tampons de la chaine 2                */

          // printk("asdcioctl.c entree dans case ASDCEECR \n");   //ajout
          TDUTIL(arg, sizeof(struct asdctampon));
          DUTIL(zt, t, arg, sizeof(struct asdctampon));
          TVUTIL(zt, t, arg, sizeof(struct asdctampon));


          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          t->v.adresse &= 0x1F;
          t->v.sous_adresse &= 0x1F;
          /* t->v.direction : Ecriture ==> imperativement TRANSMISSION */
          t->v.direction = 1;

          /* La table des sous_adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + t->v.adresse)) == 0)
	    { cprintf("Echec 1\"\n");
              RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* Pointeur vers zone des donnees en memoire image */
          zd = LI(i + t->v.sous_adresse + 32, 13);
          if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
            { cprintf("Echec zd\"\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          /* Mode de la voie (synchrone, asynchrone, etc...) */
          m = LI(zd + IRMODE, 15);

          /* "Synchrone 2" est le seul mode autorise */
          if (m != RT_VSYNC2)
            { RETURN(ENOEXEC);
            }



          /* La voie existe-t-elle vraiment ? */
          //j = L(i + t->v.sous_adresse + 32);
          //   Lecture de j deplace dans la section critique

          /* Ce test n'est sans doute pas adapte au mode "synchrone 2"
          //if (j == 0)
          //     { cprintf("Echec 2'\n");
          //       RETURN(EADDRNOTAVAIL);
          //     }
          */




          /* Le contenu de la memoire image semble-t-il correct ? */
          cmd = (t->v.adresse << 11) | 0x400 | (t->v.sous_adresse << 5);
          if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
            { cprintf("Echec 4'\n");
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }


          /* RT en mode espion ? [Est-ce vraiment possible ?] */
          espion_tr = L(L(PROPTR) + t->v.adresse) & 2;

          /* Si oui, operation invalide ! */
          if (espion_tr)
            { cprintf("Echec 5'\n");
              RETURN(EXDEV);
            }




          /* Traitement proprement dit */
            { int irtch1, irtfch1, irtnch1;
              int irtch2, irtfch2, irtnch2;
              int irtfchinu;
              int memo1, memo2, pfw;
              int rtsa_cur1;
              int ch2inu, fch2inu, nch2inu;
              int p1, pp1, p2, nch;

              int d1, d2, d3;

              rtsa_cur1 =   ((t->v.adresse << 11) & 0xF800)
                          | ((t->v.sous_adresse << 5) & 0x01E0)
                          | 1;


              /*******************************************/
              /* Debut section critique                  */
              /*   Pour garantir une execution continue  */
              /*   et, donc, relativement rapide du code */
              /*   ci-dessous ...                        */
              /*******************************************/
              DISABLE(s);

              /* Lecture des pointeurs initiaux */
              irtch1 = LI(zd + IRTCH1 , 1000);
              irtch2 = LI(zd + IRTCH2 , 1000);
              irtfchinu = LI(zd + IRTFCHINU , 1000);
              irtfch1 = LI(zd + IRTFCH1 , 1000);
              irtfch2 = LI(zd + IRTFCH2 , 1000);
              irtnch1 = LI(zd + IRTNCH1 , 1000);
              irtnch2 = LI(zd + IRTNCH2 , 1000);

              ch2inu = fch2inu = nch2inu = 0;

              // Pour debug
              // MSG("   Vi: irtch1="); MSGX(irtch1);
              //     MSG("  irtfch1="); MSGX(irtfch1); MSG("\n");
              // MSG("   Vi: irtch2="); MSGX(irtch2);
              //     MSG("  irtfch2="); MSGX(irtfch2); MSG("\n");
              // MSG("   Vi: irtfchinu="); MSGX(irtfchinu);
              //    MSG("\n");
              // MSG("   Vi: ch2inu="); MSGX(ch2inu);
              //    MSG("  fch2inu="); MSGX(fch2inu); MSG("\n");

              // MSG(" nch1="); MSGD(irtnch1);
              // MSG(" nch2="); MSGD(irtnch2);
              // MSG("\n");


              /*
               * irtch1 = debut chaine 1
               * irtch2 = debut chaine 2
               * irtfch1 = fin chaine 1
               * irtfch2 = fin chaine 2
               * irtnch1 = nombre de tampons dans chaine 1
               * irtnch2 = nombre de tampons dans chaine 2
               * irtfchinu = ptr chaine des tamp. inutilises
               *
               * ch2inu = Pointeur debut ch2 inutilisee
               * fch2inu = Pointeur fin chaine 2 inutilisee
               * nch2inu = Nbre tampons inutilises dans ch. 2
               */




              /* Tampon courant (pointe par le firmware) */
              j = L(i + t->v.sous_adresse + 32);
              if (j==0)
                {
                  /* Aucun tampon dans cha�ne 1             */
                  /*    ==> Fonction ASDCEECRF impossible ! */
                  RESTORE(s);
                  RETURN(ENOSR);
                }

              // MSG("EECRF: j="); MSGX(j);
              // MSG("   rtsa_cur1="); MSGX(rtsa_cur1);
              // MSG("\n");

              /* Ce tampon est-il utilise par le firmware ? */

              if ( ((short) j == (short) (d1 = L(RTSA_BUF)))
                                            && ((d2 = L(RTSA_CUR)) & 1) )
                {
                  /* Oui : Le tampon est en cours acces par firmware */
                  // MSG("EECRF: Acces Fw   RTSA_BUF="); MSGX(d1);
                  //            MSG("   RTSA_CUR="); MSGX(d2); MSG("\n");

                  /* Recherche de la partie de la chaine 2 qui */
                  /* doit etre accrochee a la chaine 1         */

                  /* Parcours simultane, en arriere, des 2 chaines, */
                  /* tampon par tampon, jusqu'a atteindre le tampon */
                  /* courant de la chaine 1                         */
                  /* --> Comme ce tampon est en cours d'acces, on   */
                  /*     s'arrete au tampon precedent !             */
                  p1 = irtfch1;
                  p2 = irtfch2;

                  if (p1 == j)
                    { /* Aucun tampon � accrocher a chaine 1 */
                      /* Toute la chaine 2 est accrochee     */
                      /* aux tampons inutilises              */
                      if (irtch2)
                        { ETPREC(irtfchinu, irtfch2);
                          EI(zd + IRTFCHINU, irtch2, 1000);
                          EI(zd + IRTCH2, 0, 1000);
                          EI(zd + IRTFCH2, 0, 1000);
                          EI(zd + IRTNCH2, 0 , 1000);
                        }

                      /* On ne touche pas a la chaine 1              */
                      /* (Le prochain acces a ASDCECR s'en chargera) */

                      // MSG("Chaine1 videe ");
                      // MSGFIN;
                      RESTORE(s); /* Fin section critique        */
                      RETURN(OK); /* Fin du traitement ASDCEECRF */
                    }

                  for (nch = 1; ; nch++)
                    {
                      if (p1 == 0)
                        { /* Cela ne devrait jamais se produire : */
                          /*    ==> Erreur interne !              */
                          RESTORE(s); /* Fin section critique avant sortie */
                          KPRINTF("asdc:ioctl(ASDCEECRF: Tampon courant"
                                  " non trouve dans chaine 1 !\n"
                                  "RT%d,%d pfw=0x%04X",
                                  t->v.adresse, t->v.sous_adresse, j & 0xFFFF
                                 );
                          RETURN(EDOM);
                        }

                      if (p2 == 0)
                        { /* Pas assez de tampons dans chaine 2 */
                          RESTORE(s); /* Fin section critique avant sortie */
                          RETURN(E2BIG);
                        }

                      pp1 = LTPREC(p1);
                      if (pp1 == j)
                        { /* Tampon courant trouve */
                          break;
                        }

                      /* Passage aux tampons precedents */
                      p1 = pp1;
                      p2 = LTPREC(p2);
                    }


                  /* On separe de la chaine 2 sa partie inutilisee */
                  /* (c'est a dire son debut)                      */
                  if (p2 != irtch2)
                    {
                      fch2inu = irtch2;
                      ch2inu = LTPREC(p2);
                      nch2inu = irtnch2 - nch;
                      ETPREC(p2, 0);
                      ETSUIV(fch2inu, 0);
                      irtch2 = p2;
                      irtnch2 = nch;
                      // MSG("--Apres fch2inu="); MSGX(fch2inu); MSG("\n");
                      // MSG("--Apres ch2inu="); MSGX(ch2inu); MSG("\n");
                      // MSG("--Apres nch2inu="); MSGD(nch2inu); MSG("\n");
                      // MSG("--Apres irtch2="); MSGX(irtch2); MSG("\n");
                      // MSG("--Apres irtnch2="); MSGX(irtnch2); MSG("\n");
                    }
                  else
                    {
                      /* Toute la chaine 2 est utilisee */
                      fch2inu = 0;
                      ch2inu = 0;
                      nch2inu = 0;
                    }

                  /* Fin recherche des tampons utiles dans chaine 2 */


                  /* La chaine 2 est accrochee au tampon actif */
                  memo1 = LTSUIV(j);
                  ETSUIV(j, irtch2);

                  /* Le firwmare accede-t-il toujours a notre RTSA ? */
                  if ((d3 = L(RTSA_CUR)) == (short) rtsa_cur1)
                    {
                      /* Oui */

                      // MSG("Tj_actif   RTSA_CUR="); MSGX(d3); MSG("  ");

                      /* Et toujours au meme tampon ?      */
                      /* (Cela va sans doute de soi, mais  */
                      /* on ne sait jamais ...)            */
                      memo2 = L(RTSA_BUF);
                      if ((short) memo2 == (short) j)
                        {
                          /* Oui */

                          /* Dans ce cas, il n'y a rien a faire */
                          /* de particulier ...                 */
                        }
                      else
                        {
                          /* Non : le tampon actif a change */

                          /* Compte tenu du gap inter-messages et du */
                          /* temps de transfert du mot de commande   */
                          /* suivant, c'est invraisemblable :        */
                          /*       ==> Affichage d'une erreur !      */
                          KPRINTF("asdc:ioctl(ASDCEECR ou ASDCEECRF): "
                                  "Changement inattendu du tampon actif !\n"
                                  "\tRT%d,%d : 0x%04X --> )x%04X\n",
                                  t->v.adresse, t->v.sous_adresse, j, memo2);
                          RESTORE(s);  /* Fin section critique avant sortie */
                          RETURN(EDOM);
                        }
                    }
                  else
                    {
                      /* Non : Le RTSA initial n'est plus actif */

                      // MSG("Inactif   RTSA_CUR="); MSGX(d3); MSG("  ");

                      /* Il faut alors verifier que le firmware    */
                      /* pointe bien vers le tampon prevu          */
                      /* (il y a un risque d'erreur en cas         */
                      /* d'acces simultane au pointeur du tampon   */
                      /* suivant par le firmware et par le driver  */
                      /* au moment de l'accrochage de la chaine 2) */

                      memo2 = L(i + t->v.sous_adresse + 32);
                      if ((short) memo2 != (short) irtch2)
                        {
                          /* Le firmware pointe le mauvais tampon ! */
                          /*   ==> On force le tampon courant !     */
                          // MSG("Forcage ");
                          E(i + t->v.sous_adresse + 32, irtch2);
                        }
                    }

                  /* Fin du chainage :                               */
                  /*   - chaine 2 avec tampon courant (ou excourant) */
                  /*   - debut chaine 1 avec fin chaine 1            */

                  if (irtch2) ETPREC(irtch2, j);

                  memo2 = LTPREC(j);
                  ETPREC(j, 0);
                  if (memo1) ETPREC(memo1, memo2);
                  if (memo2) ETSUIV(memo2, memo1);

                  if (irtch1 == j) irtch1 = memo1;

                  irtch2 = j;
                  irtnch2++;
                  irtnch1--;

                }
              else
                {
                  /* Tampon non en cours d'acces par firmware */

                  // MSG("EECRF: RT/SA inactif   RTSA_BUF="); MSGX(d1);
                  //              MSG("   RTSA_CUR="); MSGX(d2); MSG("\n");

                  /* La fin de la chaine 2 doit remplacer la partie */
                  /* encore inutilisee de la chaine chaine 1        */

                  /* Recherche de la partie de la chaine 2 qui */
                  /* doit etre accrochee a la chaine 1 :       */

                  /* Parcours simultane, en arriere, des 2 chaines, */
                  /* tampon par tampon, jusqu'a atteindre le tampon */
                  /* courant de la chaine 1                         */
                  p1 = irtfch1;
                  p2 = irtfch2;
                  for (nch = 1; ; nch++)
                    {
                      if (p1 == 0)
                        { /* Cela ne devrait jamais se produire : */
                          /*    ==> Erreur interne !              */
                          // MSG("   EDOM"); MSGFIN;
                          RESTORE(s); /* Fin section critique avant sortie */
                          KPRINTF("asdc:ioctl(ASDCEECRF: Tampon courant"
                                  " non trouve dans chaine 1 !\n"
                                  "RT%d,%d pfw=0x%04X",
                                  t->v.adresse, t->v.sous_adresse, j & 0xFFFF
                                 );
                          RETURN(EDOM);
                        }

                      if (p2 == 0)
                        { /* Pas assez de tampons dans chaine 2 */
                          // MSG("   E2BIG"); MSGFIN;
                          RESTORE(s); /* Fin section critique avant sortie */
                          RETURN(E2BIG);
                        }

                      if (p1 == j)
                        { /* Tampon courant trouve */
                          // MSG("   Tampon ch2 trouve !");
                          break;
                        }

                      /* Passage aux tampons precedents */
                      p1 = LTPREC(p1);
                      p2 = LTPREC(p2);
                    }


                  // MSG("   Nelle ch2 trouvee p1="); MSGX(p1);
                  // MSG(" p2="); MSGX(p2); MSG("\n");

                  /* On separe de la chaine 2 sa partie inutilisee */
                  /* (c'est a dire son debut)                      */
                  if (p2 != irtch2)
                    {
                      fch2inu = irtch2;
                      ch2inu = LTPREC(p2);
                      if (ch2inu == 0)
                        { /* Un seul tampon */
                          ch2inu = fch2inu;
                        }
                      nch2inu = irtnch2 - nch;
                      ETPREC(p2, 0);
                      ETSUIV(fch2inu, 0);
                      irtch2 = p2;
                      irtnch2 = nch;
                    }
                  else
                    {
                      /* Toute la chaine 2 est utilisee */
                      fch2inu = 0;
                      ch2inu = 0;
                      nch2inu = 0;
                    }

                  /* Fin recherche des tampons utiles dans chaine 2 */


                  /* Connexion partie utile chaine 2 a la place */
                  /* de la partie restante de la chaine 1       */
                  memo1 = j;
                  E(i + t->v.sous_adresse + 32, irtch2);

                  /* Le firwmare accede-t-il maintenant a notre RTSA ? */
                  if (L(RTSA_CUR) == (short) rtsa_cur1)
                    {
                       // MSG("Fw_actif ");

                       /* Oui, mais auquel des 2 tampons ? */
                       /* (l'ancien ou le nouveau ?)       */
                       if (L(RTSA_BUF) == (short) memo1)
                         {
                           int t_avant, t_apres;

                           // MSG("Ancien T.\n");

                           /* L'ancien tampon est actif !         */
                           /* ==> Il faut raccrocher la chaine 2, */
                           /*     encore inutilisee, a ce tampon. */

                           /* Comme l'entree en activite de ce tampon est   */
                           /* tres recente, nous disposons d'au moins 20 us */
                           /* pour effectuer l'echange, ce qui devrait etre */
                           /* suffisant ...                                 */

                           // MSG("   Av: irtch1="); MSGX(irtch1);
                           //     MSG("  irtfch1="); MSGX(irtfch1); MSG("\n");
                           // MSG("   Av: irtch2="); MSGX(irtch2);
                           //     MSG("  irtfch2="); MSGX(irtfch2); MSG("\n");
                           // MSG("   Av: irtfchinu="); MSGX(irtfchinu);
                           //     MSG("\n");
                           // MSG("   Av: ch2inu="); MSGX(ch2inu);
                           //     MSG("  fch2inu="); MSGX(fch2inu); MSG("\n");

                           /* Reconnexion du firmware au tampon actif */
                           E(i + t->v.sous_adresse + 32, memo1);

                           /* Retrait de ce tampon actif de la chaine 1 */
                           /* (l'ordre des autres tampons est conserve) */
                           t_avant = LTPREC(memo1);
                           t_apres = LTSUIV(memo1);
                           if (t_avant) ETSUIV(t_avant, t_apres);
                                   else irtch1 = t_apres;
                           if (t_apres) ETPREC(t_apres, t_avant);
                                   else irtfch1 = t_avant;
                           irtnch1--;

                           /* Saut du premier tampon de la chaine 2 */
                           ETPREC(irtch2, ch2inu);
                           ch2inu = irtch2;
                           irtch2 = LTSUIV(irtch2);
                           // ETPREC(irtch2, 0);    // Inutile (cf. ci-apres)
                           nch2inu++;
                           irtnch2--;

                           /* Connexion de la chaine 2 au tampon actif */
                           ETPREC(memo1, 0);
                           ETSUIV(memo1, irtch2);
                           ETPREC(irtch2, memo1);

                           /* Inclusion du tampon actif dans la chaine 2 */
                           irtch2 = memo1;
                           irtnch2++;

                           // MSG("   Ap: irtch1="); MSGX(irtch1);
                           //   MSG("  irtfch1="); MSGX(irtfch1); MSG("\n");
                           // MSG("   Ap: irtch2="); MSGX(irtch2);
                           //   MSG("  irtfch2="); MSGX(irtfch2); MSG("\n");
                           // MSG("   Ap: irtfchinu="); MSGX(irtfchinu);
                           //   MSG("\n");
                           // MSG("   Ap: ch2inu="); MSGX(ch2inu);
                           //   MSG("  fch2inu=");  MSGX(fch2inu); MSG("\n");
                         }
                       else
                         {
                           // MSG("Nouveau T. ");
                           /* Le nouveau tampon est actif !       */
                           /* ==> tout peut continuer comme prevu */
                         }
                    }
                 }


              /* Les tampons ont ete echanges, il reste a echanger les     */
              /* valeurs des pointeurs des chaines 1 et 2 (irtch1, etc...) */
              memo1 = irtch1;
              irtch1 = irtch2;
              irtch2 = memo1;
              memo1 = irtfch1;
              irtfch1 = irtfch2;
              irtfch2 = memo1;
              memo1 = irtnch1;
              irtnch1 = irtnch2;
              irtnch2 = memo1;

              // MSG("  ---\n");
              // MSG("   F1: irtch1="); MSGX(irtch1);
              //   MSG("  irtfch1="); MSGX(irtfch1); MSG("\n");
              // MSG("   F1: irtch2="); MSGX(irtch2);
              //   MSG("  irtfch2="); MSGX(irtfch2); MSG("\n");
              // MSG("   F1: irtfchinu="); MSGX(irtfchinu);
              //   MSG("\n");
              // MSG("   F1: ch2inu="); MSGX(ch2inu);
              //   MSG("  fch2inu="); MSGX(fch2inu); MSG("\n");


              /* Recyclage de la chaine 2 (ancienne chaine 1) */
              if (irtfchinu)
                {
                  // MSG(" 1111111");
                  // MSGX(irtfch2); MSG("-->ETPREC(");
                  // MSGX(irtfchinu); MSG(")\n");
                  // MSGX(irtch2);  MSG("-->irtfchinu\n");

                  ETPREC(irtfchinu, irtfch2);
                  irtfchinu = irtch2;
                }
              else
                {
                  // MSG(" 2222222");

                  irtfchinu = irtch2;
                  EI(zd + IRTCHINU, irtfch2, 1000);
                }
              irtch2 = irtfch2 = 0;
              irtnch2 = 0;

              /* Recyclage du debut de l'ancienne chaine 2 (s'il existe) */
              if (ch2inu)
                {
                  if (irtfchinu)
                    {
                      // MSG(" AAAAAAA\n");
                      // MSGX(ch2inu); MSG("-->ETPREC(");
                      // MSGX(irtfchinu); MSG(")\n");
                      // MSGX(fch2inu);  MSG("-->irtfchinu\n");

                      ETPREC(irtfchinu, ch2inu);
                      irtfchinu = fch2inu;
                    }
                  else
                    {
                      // MSG(" BBBBBBB");

                      irtfchinu = fch2inu;
                      EI(zd + IRTCHINU, ch2inu, 1000);
                    }
                }

              // MSG("  ---\n");
              // MSG("  Fin: irtch1="); MSGX(irtch1);
              //    MSG("  irtfch1="); MSGX(irtfch1); MSG("\n");
              // MSG("  Fin: irtch2="); MSGX(irtch2);
              //    MSG("  irtfch2="); MSGX(irtfch2); MSG("\n");
              // MSG("  Fin: irtfchinu="); MSGX(irtfchinu);
              //    MSG("\n");
              // MSG("  Fin: ch2inu="); MSGX(ch2inu);
              //    MSG("  fch2inu="); MSGX(fch2inu); MSG("\n");
              // MSG("\n");

              /* Rangement des pointeurs modifies */
              EI(zd + IRTCH1, irtch1, 1000);
              EI(zd + IRTCH2, irtch2, 1000);
              EI(zd + IRTFCH1, irtfch1, 1000);
              EI(zd + IRTFCH2, irtfch2, 1000);
              EI(zd + IRTNCH1, irtnch1, 1000);
              EI(zd + IRTNCH2, irtnch2, 1000);
              EI(zd + IRTFCHINU, irtfchinu, 1000);

              // MSGFIN;

              RESTORE(s);
              /*******************************************/
              /* Fin de la section critique              */
              /*******************************************/

            }

          RETURN(OK);








                        /****************************************************/
        case ASDCLEC :  /* Lecture d'un tampon :                            */
                        /*  - Un tampon n'ayant pas recu de donnees depuis  */
                        /*  sa derniere lecture est caracterise par         */
                        /*  flag=TRT_LIBRE .                                */
                        /*  - Le premier tampon non "LIBRE" est lu, puis il */
			/*  est marque "TRT_LIBRE" .                        */
			/*  - Si tous les tampons existants sont "LIBRE",   */
                        /*  rien n'est lu et une erreur ENOSR est           */
                        /*  renvoyee.                                       */
                        /****************************************************/
          // printk("asdcioctl.c entree dans case ASDCLEC \n"); //ajout
          TDUTIL(arg, sizeof(struct asdctampon));
          DUTIL(zt, t, arg, sizeof(struct asdctampon));
          TVUTIL(zt, t, arg, sizeof(struct asdctampon));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          t->v.adresse &= 0x1F;
          t->v.sous_adresse &= 0x1F;

          /* La "mise en forme" de t->v.direction est reportee */
          /* a un peu plus loin...                             */

          /* La table des sous-adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + t->v.adresse)) == 0)
	    { RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* La table des mots d'etat est elle bien declaree ? */
          if (L(SWTPTR) == 0)
	    { RETURN(EXDEV);   	/* (Ne devrait jamais se produire !) */
	    }


          /* Le RT est-il simule ? */
          if (L(L(SWTPTR) + t->v.adresse))
            { /* Le RT est simule ==> imperativement RECEPTION */
              t->v.direction = 0;
            }
          else
            { /* Le RT est espionne ==> la voie peut-etre en EMISSION */
              /* comme en RECEPTION !                                 */
              t->v.direction &= 1;
            }

          /* La voie existe-t-elle ? */
          if ((j = L(i + t->v.sous_adresse
                                       + (t->v.direction ? 32 : 0))) == 0)
	    { RETURN(EADDRNOTAVAIL); /* La voie n'a jamais ete declaree ! */
	    }


          /* Le contenu de la memoire image semble-t-il correct ? */

          zd = LI(i + t->v.sous_adresse + (t->v.direction ? 32 : 0), 22);
          if ((zd < DEBUT_RAM) || (zd >= FIN_RAM ))
            { RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          cmd =   (t->v.adresse << 11)
                | (t->v.direction ? 0x400 : 0)
                | (t->v.sous_adresse << 5);

          if ((LI(zd+IRCMD, 23) & 0xFFE0) != cmd)
            { 		/* Memoire image anormale ! */
              cprintf("LI(zd+IRCMD)=0x%lX\n", LI(zd+IRCMD, 24));
              cprintf("cmd=0x%X\n", cmd);
              cprintf("---2---\n");
              RETURN(ESPIPE);
            }



          /* Validation, si necessaire, des ITs associees a la voie */
	  if (t->f == SDC_ATTENDRE)
            { int ef;

              /* Pointeur table des filtres associee a adresse */
              ef = L(L(FTPTR) + t->v.adresse);

              /* Adresse du filtre */
              ef += t->v.sous_adresse + ( t->v.direction ? 32 : 0);

              /* Programmation du filtre : Validation IT voie */
              E(ef, L(ef) | 2);
            }


          /* RT en mode espion ? */		/*####### UTILE ??? #######*/
          espion_tr = L(L(PROPTR) + t->v.adresse) & 2;

          /* Si non et voie en emission, alors operation invalide ! */
          if (t->v.direction && !espion_tr)
            {
              cprintf("t->v.direction=%d espion_tr=%d\n",
                       t->v.direction, espion_tr);
              RETURN(EINVAL);
            }

          /* Voie en mode synchrone, asynchrone ou statique ? */
          m = LI(zd + IRMODE, 25);

          /* La voie ne devrait pas etre en mode "synchrone 2"   */
          /* (sauf, peut-etre, si espion TR, mais ce type de     */
          /* fonctionnement n'est pas implemente pour le moment) */
          if (m == RT_VSYNC2) RETURN(ENOEXEC);

          /* si la voie est statique :                               */
          /*    - Le mode SDC_ATTENDRE est remplace par SDC_NONBLOQ  */
          /*    - La voie est alors traitee comme une voie synchrone */
          mde = t->f;
          if (m == RT_VSTAT)
            { m = RT_VSYNC;
              if (t->f == SDC_ATTENDRE)
                { mde = SDC_NONBLOQ;
                }
            }

          /* Tampons separes ? */
          z = ((LI(zd + IRNBT, 26) == 2) && (L(j) == j));
                                           /* z vrai si t. separes */

          /* Les tampons a lire par l'appli ne devraient pas etre separes */
          /* 	==> Pb ...                                                */
          if (z)
            { RETURN(EDOM);	/* Erreur interne ... */
            }


          /* Traitement des tampons chaines en mode asynchrone */
          if (m)
            {
              int autret, memoj, cmd;

              /* Traitement RAZ eventuelle et controle validite du mode */
              switch (mde)
                { case SDC_RAZRAZ :
                  case SDC_RAZ :	/* "Remise a zero" des tampons */
                       			E(j+1, TRT_LIBRE);
                       			j = L(j);
                       			E(j+1, TRT_LIBRE);
                       			return OK;

                  case SDC_NONBLOQ :
                  case SDC_ATTENDRE : 	break;

                  default : 		RETURN(EINVAL);
                }

              /* Adresse tampon 2 */
              autret = L(j);
              memoj = j;

              /* Lecture tampon 2 */
  	      cmd = L(autret+1);
              t->nbr = cmd & 0x1F;
              if (t->nbr == 0) t->nbr = 32;
  	      for (k=0; k<t->nbr; k++) t->t[k] = L(autret+2+k);

              /* Relecture du pointeur "firmware" */
              j = L(i + t->v.sous_adresse + (t->v.direction ? 32 : 0));

              if (j != memoj)
                {
                  /* Il faut recommencer ! */
                  autret = L(j);

                  /* Lecture tampon 2 */
  	          cmd = L(autret+1);
                  t->nbr = cmd & 0x1F;
                  if (t->nbr == 0) t->nbr = 32;
  	          for (k=0; k<t->nbr; k++) t->t[k] = L(autret+2+k);
  	        }

              if (cmd == TRT_LIBRE)
                { RETURN(ENOSR);
                }

              VUTIL(zt, t, arg, sizeof(struct asdctampon));
              RETURN(OK);
            }

          /* Traitement des tampons chaines en mode synchrone */
          switch (mde)
            { case SDC_RAZRAZ :
              case SDC_RAZ :
                 /* Remise a "zero" de tous les tampons */
                 tmpi = j;
                 for (l=0; l<LI(zd+IRNBT, 27); l++)
                   { E(tmpi+1, TRT_LIBRE);
                     tmpi = L(tmpi);
                   }
                 /* Mise en coherence des pointeurs firmware et appli */
                 EI(zd + IRTCAPP, j, 319);

                 /* Fin de la RAZ */
                 RETURN(OK);

              case SDC_NONBLOQ :
                 l = LI(zd + IRTCAPP, 28);	/* Adr. tampon appli courant */

                 /* Debordement ? */
                 debordement =
                          ((L(LI(l+IRTPREC, 29) + 1) & 0xFFFF) != TRT_LIBRE);

                 if ((L(l + 1) & 0xFFFF) == TRT_LIBRE) /* Donnees dispo ? */
                   { /* Plus de donnees dispo ! */
                     if (debordement) t->f |= SDC_DEBORD;
                     VUTIL(zt, t, arg, sizeof(struct asdctampon));
                     RETURN(ENOSR);
                   }

                 /* Lecture du tampon */
                 t->nbr = L(l+1) & 0x1f;
                 if (t->nbr == 0) t->nbr = 32;
  		 for (k=0; k<t->nbr; k++) t->t[k] = L(l+2+k);
  		 E(l+1, TRT_LIBRE);

  		 /* Mise a jour du pointeur application */
  		 EI(zd + IRTCAPP, L(l), 320);

  		 if (debordement) t->f |= SDC_DEBORD;
                 VUTIL(zt, t, arg, sizeof(struct asdctampon));
                 RETURN(OK);


              case SDC_ATTENDRE :
                 // printk("entree dans case SDC_ATTENDRE"
                 //        "  switch (mde) ASDCLEC\n");"

                 l = LI(zd + IRTCAPP, 30);	/* Adr. tampon appli courant */

                 /* Debordement ? */
                 debordement =
                         ((L(LI(l+IRTPREC, 31) + 1) & 0xFFFF) != TRT_LIBRE);

	         /* Semaphore ou attendre */
	         oreiller = &LI(zd + IRSEM, 32);

                 dprintf("sleep : oreiller RT(%d,%d,%c)\n",
	                 t->v.adresse, t->v.sous_adresse,
	                 t->v.direction ? 'T' : 'R');

                 /* ---------------------------------------------------- */
                 /* ---   Debut section critique   ------------- vvvvvvv */
                 DISABLE(s);

                 tmpi = 0;	/* Pour pouvoir passer tests en sortie  */
                                /* section critique si swait non appele */

                 if ((L(l + 1) & 0xFFFF) == TRT_LIBRE) /* Donnees dispo ? */
                   { /* Plus de donnees dispo ! */

                     /* Demasquage eventuel IT sur ABI */
                     if ((++dst->nombre_it)==1)
                       {    /***********************************/
                            /* DEMASQUAGE IT ABI doit etre ICI */
                            /***********************************/
                       }


                     // dprintf("ASDC : j'attend l'IT !\n");
                     // printk("ASDC : j'attend l'IT !\n"); //ajout
                     tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
                     //dprintf("ASDC : IT arrivee !\n");
                     // printk("ASDC : IT arrivee !\n"); //ajout
                     if (!(--dst->nombre_it))
                       {  /*********************************/
                          /* MASQUAGE IT ABI doit etre ICI */
                          /*********************************/
                       }
                   }

                 RESTORE(s);
                 /* ---   Fin section critique   --------------- ^^^^^^^ */
                 /* ---------------------------------------------------- */

                 if (tmpi)	/* Passage semaphore du a un signal ? */
                   { if (debordement) t->f |= SDC_DEBORD;
                     VUTIL(zt, t, arg, sizeof(struct asdctampon));
                     RETURN(EINTR);
                   }

	         if (dst->raz) /* Passage du semaphore du a une RAZ ? */
	           { if (debordement) t->f |= SDC_DEBORD;
	             VUTIL(zt, t, arg, sizeof(struct asdctampon));
	             RETURN(ENETRESET);
	           }


                 /* Lecture du tampon */
                 t->nbr = L(l+1) & 0x1F;
                 if (t->nbr == 0) t->nbr = 32;
  		 for (k=0; k<t->nbr; k++) t->t[k] = L(l+2+k);
  		 E(l+1, TRT_LIBRE);

  		 /* Mise a jour du pointeur application */
  		 EI(zd + IRTCAPP, L(l), 321);

  		 if (debordement) t->f |= SDC_DEBORD;
                 VUTIL(zt, t, arg, sizeof(struct asdctampon));
                 RETURN(OK);


              default :
                 RETURN(EINVAL);
            }












       /***********************************************************/
       /*   Faudrait il ajouter une fonction ioctl pour inhiber   */
       /*   une demande d'IT dans la table des filtres  ?...      */
       /***********************************************************/









       case ASDCLPAR :  /* Lecture de la table des parametres du driver  */
                        /*   Cette fonction lit directement la structure */
                        /*   asdcparam                                   */

          TVUTIL(zp, p, arg, sizeof(struct asdcparam));

          *p = dst->asdcdef;

          VUTIL(zp, p, arg, sizeof(struct asdcparam));
          RETURN(OK);




       case ASDCEPAR :  /* Ecriture de la table des parametres du driver   */
                        /*   Cette fonction ecrit directement la structure */
                        /*   asdcparam                                     */
                        /* ATTENTION : Les parametres ainsi ecrits ne      */
                        /*             seront pris en compte qu'a la       */
                        /*             prochaine execution de              */
                        /*             ioctl(ASDCRAZ)                      */

          TDUTIL(arg, sizeof(struct asdcparam));
          DUTIL(zp, p, arg, sizeof(struct asdcparam));

          dst->asdcdef = *p;

          RETURN(OK);







       case ASDCVFLASH :  /* Passage � 1 du flag dst->vflash             */
                          /* Ce flag inhibe l'affichage de messages      */
                          /* d'avertissement sur la console systeme      */
                          /* en cas d'acces a des adresses hors de       */
                          /* la zone autorisee de la memoire d'echange.  */
                          /*                                             */
                          /* En l'absence de ce vflash=1, l'ecriture     */
			  /* de la flash prend un temps gigantesque !    */
			  /*                                             */
			  /* Il n'existe pas de fonction pour remettre   */
			  /* vflag a 0 : apres ecriture de la flash, il  */
			  /* est necessaire de mettre le systeme hors    */
			  /* tension puis, donc, de tout reinitialiser.  */

          dst->vflash = 1;

          RETURN(OK);







       case ASDCINHESPS : /* Inhibition de l'espionnage sequentiel d'un RT */

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));	/* Securite LynxOS */

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
	  v->adresse &= 0x1F;
          v->sous_adresse &= 0x1F;
          v->direction &= 0x01;

          /* On ne cherche pas a savoir si RT est valide ou non : */
          /* On "inhibe" directement !                            */

          /* Allocation si necessaire de la table des filtres */
          /* et initialisation de son image                   */
          j = L(FTPTR) + v->adresse;
          k = L(j);
          if (k==0)
             { k = asdcalloc(dst, 64);
	       if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
                          }
               E(j, k);
               for (i=k; i<k+64; i++)
                  { E(i, 0);  /* Init a 0 */
                    EI(i, 0, 384);
                  }
             }

          /* Calcul de l'adresse du filtre */
          k += v->sous_adresse + ( v->direction ? 32 : 0);

          /* Inhibition de l'espionnage sequentiel du RT */
          /* dans la table des filtres : bit 0 mis a 0   */
          E(k, L(k) & 0xFFFE);

          RETURN(OK);








       case ASDCVALESPS : /* Validation de l'espionnage sequentiel d'un RT */
          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));	/* Securite LynxOS */

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
	  v->adresse &= 0x1F;

          /* On ne cherche pas a savoir si RT est inhibe ou non : */
          /* On "valide" directement !                            */

          /* Allocation si necessaire de la table des filtres */
          /* et initialisation de son image                   */
          j = L(FTPTR) + v->adresse;
          k = L(j);
          if (k==0)
             { k = asdcalloc(dst, 64);
	       if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
                          }
               E(j, k);
               for (i=k; i<k+64; i++)
                  { E(i, 0);  /* Init a 0 */
                    EI(i, 0, 385);
                  }
             }

          /* Calcul de l'adresse du filtre */
          k += v->sous_adresse + ( v->direction ? 32 : 0);

          /* Validation de l'espionnage sequentiel du RT */
          /* dans la table des filtres : bit 0 mis a 1   */
          E(k, L(k) | 1);

          RETURN(OK);






       case ASDCESPS : /* Initialisation de l'espionnage sequentiel */

          /* Forcage du basculement sans IT        */
          /* (garantit demarrage avec tampon vide) */
          E(MBFLG, 1);

          /* Positionnement indicateur */
          dst->monok = 0;  /* Aucun tampon n'est pret */

          /* Initialisation du compteur de blocs */
          E(SMBCNT, 0);

          /*
             Faut-il armer les ITs ? . . . . . . .
          */

          RETURN(OK);






       case ASDCESPSIT : /* Attente IT de l'espionnage sequentiel */

          /* --------------------------------------------------------------- */
          /* ---   Debut section critique   ------------------------ vvvvvvv */
          DISABLE(s);


          dprintf("ASDC-entree : dst->monok = %d\n", dst->monok);
          /* Une IT est elle arrivee trop tot ? */
          if (dst->monok) erreur = 1;
              else { erreur = 0;


                     /* Demasquage eventuel IT sur AMI */
                     if((++dst->nombre_it)==1)
                        {    /***********************************/
                             /* DEMASQUAGE IT AMI doit etre ICI */
                             /***********************************/
                        }


                     // cprintf("ASDC : j'attend l'IT espion !\n");
		     tmpi = SWAIT(&dst->semmon, SEM_SIGABORT);

                     if(!(--dst->nombre_it))
                        {

                          /*********************************/
                          /* MASQUAGE IT AMI doit etre ICI */
                          /*********************************/
                        }

                   }

          /* Restauration de l'indicateur de basculement */
          /* (qu'il y ait eu erreur ou non)              */
          dst->monok = 0;
          // kkprintf("ASDC-sortie : dst->monok = %d\n", dst->monok);


          RESTORE(s);
          /* ---   Fin section critique   -------------------------- ^^^^^^^ */
          /* --------------------------------------------------------------- */

	  if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
	    { // kkprintf("Signal!\n");
	      RETURN(EINTR);
	    }


	  if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	    { RETURN(ENETRESET);
	    }

          if (erreur) { dprintf("Engorgement espion sequentiel !\n");
			RETURN(EIO);
                      }

          /* IT arrivee */
          // cprintf("ASDC : IT espion arrivee !\n");
          RETURN(OK);






       case ASDCESPSBASC : /* Force le basculement des tampons */
                           /* de l'espionnage sequentiel       */

          /* Forcage du basculement avec IT */
          E(MBFLG, 2);

          RETURN(OK);






       case ASDCESPSFIN : /* Fin de l'espionnage sequentiel */

          /* Reste a definir ce qu'on a a faire ... */

          /* ... peut-etre inhiber les ITs ? . . . . . . .
          */

          RETURN(OK);



       case ASDCLECEMEMO :  /* Lecture de l'environement table memo[]        */
                        /*   Cette fonction est uniquement destinee au debug */
                        /*   de l'allocation en memoire d'echange            */

          {
            struct asdcememo *emem, zemem;

            TVUTIL(zemem, emem, arg, sizeof(struct asdcememo));
	    asdclecememo(dst, emem);
            VUTIL(zemem, emem, arg, sizeof(struct asdcememo));
            RETURN(OK);
          }




       case ASDCLECMEMO :  /* Lecture d'une entree de la table memo[]        */
                        /*   Cette fonction est uniquement destinee au debug */
                        /*   de l'allocation en memoire d'echange            */

          {
            struct asdcblibre *bl, zbl;

            TDUTIL(arg, sizeof(struct asdcblibre));
            DUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
            TVUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
	    if (asdclecmemo(dst, bl))
	      { RETURN(EINVAL);
	      }
            else
              {
                 VUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
                 RETURN(OK);
              }
          }





       case ASDCBBC :  /* Creation d'un bloc de commande BC */

          TDUTIL(arg, sizeof(struct asdcbbc));
          DUTIL(zb, b, arg, sizeof(struct asdcbbc));
          TVUTIL(zb, b, arg, sizeof(struct asdcbbc));

          { int taille_bloc;

            taille_bloc = ((b->type == BC_SCHED)
                                || (b->type == BC_MINOR) ) ? 16 : 8;

            if (b->adrbbc == 0)  /* Alors, allocation du bloc */
              {
                b->adrbbc = asdcalloc(dst, taille_bloc);
   //    kkprintf("asdcalloc ==> b->adrbbc = 0x%0X\n", b->adrbbc);
                if ((b->adrbbc & 0xFFFF) == 0xFFFF)
                  { VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                    RETURN(ENOMEM);     /* Plus de memoire */
	          }
              }

            i = b->adrbbc & 0xFFFF;

            /* Effacement zone image correspondant au bloc BC */
            /* (et code erreur par la meme occasion ...)      */
            for (j=0; j<taille_bloc; j++) EI(i+j, 0, 322);
          }

          switch(b->type)
             { case COCO : E(i++, COCO);
                           E(i++, ((b->adresse & 0x1F) << 11)
                                       | (b->cocodee & 0x41F) | 0x3E0);
                           i ++;
                           E(i++, 0xFFFF);
                           i++;
                           E(i++, b->options | (b->bus ? FBC_BUSB : 0));
                           if (b->cocodee & 0x10)
                              { /* Alors, tampon necessaire */
                                if (b->adrtamp == 0)  /* Alors, alloc. tamp. */
                                  { b->adrtamp = asdcalloc(dst, 32);
    //   kkprintf("asdcalloc ==> b->adrtamp = 0x%0X\n", b->adrtamp);
                                    if ((b->adrtamp & 0xFFFF) == 0xFFFF)
                                       { asdclibere(dst, b->adrbbc, 8);
                                         VUTIL(zb, b, arg,
                                                   sizeof(struct asdcbbc));
                                         RETURN(ENOMEM); /* Plus de memoire */
                                       }
                                  }
                              }
                             else b->adrtamp = 0;  /* pour faciliter debug */
                           E(i++, b->adrtamp);
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case BCRT : E(i++, BCRT);
                           E(i++, ((b->adresse & 0x1F) << 11)
                                       | ((b->sous_adresse & 0x1F) << 5)
                                       | (b->nbmots & 0x1F));
                           i ++;
                           E(i++, 0xFFFF);
                           i++;
                           E(i++, b->options | (b->bus ? FBC_BUSB : 0));
                           if (b->adrtamp == 0)  /* Alors, alloc. tamp. */
                                  { b->adrtamp = asdcalloc(dst, 32);
   //    kkprintf("asdcalloc ==> b->adrtamp = 0x%0X\n", b->adrtamp);
                                    if ((b->adrtamp & 0xFFFF) == 0xFFFF)
                                       { asdclibere(dst, b->adrbbc, 8);
                                         VUTIL(zb, b, arg,
                                                     sizeof(struct asdcbbc));
                                         RETURN(ENOMEM); /* Plus de memoire */
                                       }
                                  }
                           E(i++, b->adrtamp);
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case RTBC : E(i++, RTBC);
                           E(i++, ((b->adresse & 0x1F) << 11)
                                       | 0x400
                                       | ((b->sous_adresse & 0x1F) << 5 )
                                       | (b->nbmots & 0x1F));
                           i ++;
                           E(i++, 0xFFFF);
                           i++;
                           E(i++, (b->options | (b->bus ? FBC_BUSB : 0)));
                           if (b->adrtamp == 0)  /* Alors, alloc. tamp. */
                                  { b->adrtamp = asdcalloc(dst, 32);
    //   kkprintf("asdcalloc ==> b->adrtamp = 0x%0X\n", b->adrtamp);
                                    if ((b->adrtamp & 0xFFFF) == 0xFFFF)
                                       { asdclibere(dst, b->adrbbc, 8);
                                         VUTIL(zb, b, arg,
                                                   sizeof(struct asdcbbc));
                                         RETURN(ENOMEM); /* Plus de memoire */
                                       }
                                  }
                           E(i++, b->adrtamp);
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case RTRT : E(i++, RTRT);
                           E(i++, ((b->adresse & 0x1F) << 11)
                                       | ((b->sous_adresse & 0x1F) << 5 )
                                       | (b->nbmots & 0x1F));
                           E(i++, ((b->adresse2 & 0x1F) << 11)
                                       | 0x400
                                       | ((b->sous_adresse2 & 0x1F) << 5 )
                                       | (b->nbmots & 0x1F));
                           E(i++, 0xFFFF);
                           E(i++, 0xFFFF);
                           E(i++, (b->options | (b->bus ? FBC_BUSB : 0)));
                           E(i++, 0);	/* Pas de donnees dans ce mode */
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case BCRT_DI :
                           E(i++, BCRT_DI);
                           E(i++, (0x1F << 11)
                                       | ((b->sous_adresse & 0x1F) << 5)
                                       | (b->nbmots & 0x1F));
                           i ++;
                           E(i++, 0xFFFF);
                           i++;
                           E(i++, b->options | (b->bus ? FBC_BUSB : 0));
                           if (b->adrtamp == 0)  /* Alors, alloc. tamp. */
                                  { b->adrtamp = asdcalloc(dst, 32);
 //      kkprintf("asdcalloc ==> b->adrtamp = 0x%0X\n", b->adrtamp);
                                    if ((b->adrtamp & 0xFFFF) == 0xFFFF)
                                       { asdclibere(dst, b->adrbbc, 8);
                                         VUTIL(zb, b, arg,
                                                     sizeof(struct asdcbbc));
                                         RETURN(ENOMEM); /* Plus de memoire */
                                       }
                                  }
                           E(i++, b->adrtamp);
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case RTRT_DI :
                           E(i++, RTRT_DI);
                           E(i++, (0x1F << 11)
                                       | ((b->sous_adresse & 0x1F) << 5 )
                                       | (b->nbmots & 0x1F));
                           E(i++, ((b->adresse2 & 0x1F) << 11)
                                       | 0x400
                                       | ((b->sous_adresse2 & 0x1F) << 5 )
                                       | (b->nbmots & 0x1F));
                           E(i++, 0xFFFF);
                           E(i++, 0xFFFF);
                           E(i++, (b->options | (b->bus ? FBC_BUSB : 0)));
                           if (b->adrtamp == 0)  /* Alors, alloc. tamp. */
                                  { b->adrtamp = asdcalloc(dst, 32);
   //    kkprintf("asdcalloc ==> b->adrtamp = 0x%0X\n", b->adrtamp);
                                    if ((b->adrtamp & 0xFFFF) == 0xFFFF)
                                       { asdclibere(dst, b->adrbbc, 8);
                                         VUTIL(zb, b, arg,
                                                    sizeof(struct asdcbbc));
                                         RETURN(ENOMEM); /* Plus de memoire */
                                       }
                                  }
                           E(i++, 0);	/* Pas de donnees dans ce mode */
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);

               case DELAI : /* ATTENTION : Test de la validite du retard */
                            /*             est a faire !!!               */
                            E(i++, DELAI);
                            i += 4;
                            E(i++, b->options);
                            E(i++, b->retard);
                            if (b->chainage) E(bloc_suivant, b->adrbbc);
                            bloc_suivant = i;
                            E(i, b->adrbbcsuiv);
                            VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                            RETURN(OK);

               case BC_SCHED :
               case BC_MINOR :
                           E(i++, b->type);
                           E(i++, b->start);
                           E(i++, b->reprate);
                           /* Ne pas initialiser a 0 le mot suivant      */
                           /* a plante, dans certaines conditions        */
                           /* l'appli GEGEN. Par securite, on initialise */
                           /* la totalite des champs reserves !          */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, ((b->heure >> 16) & 0xFFFF));
                           E(i++, (b->heure & 0xFFFF));
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           E(i++, 0);	/* Reserve firmware */
                           if (b->chainage) E(bloc_suivant, b->adrbbc);
                           bloc_suivant = i;
                           E(i, b->adrbbcsuiv);
                           VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                           RETURN(OK);


               default : kkprintf("ASDCBBC : Type de bloc 0x%04X invalide\n",
                                   b->type);
                         VUTIL(zb, b, arg, sizeof(struct asdcbbc));
                         RETURN(EINVAL);
             }




       case ASDCETAMPBC :  /* Ecriture dans un tampon de donnees BC */

          TDUTIL(arg, sizeof(struct asdctampbc));
          DUTIL(zc, c, arg, sizeof(struct asdctampbc));
          TVUTIL(zc, c, arg, sizeof(struct asdctampbc));

          if (c->n > 32) { c->n = 32;		/* 32 donnees au plus ! */
                           k = EINVAL;
                         }
                    else k = OK;

          if (c->n > (FIN_RAM - c->i))	/* Debordement mem. echange */
                         { c->n = FIN_RAM - c->i;
                           k = EINVAL;
                         }

          for (i=c->i, j=0; j<c->n; i++, j++) E(i, c->d[j]);

          VUTIL(zc, c, arg, sizeof(struct asdctampbc));
          RETURN(k);		/* EINVAL si erreur, OK sinon */





       case ASDCLTAMPBC :  /* Lecture dans un tampon de donnees BC */

          TDUTIL(arg, sizeof(struct asdctampbc));
          DUTIL(zc, c, arg, sizeof(struct asdctampbc));
          TVUTIL(zc, c, arg, sizeof(struct asdctampbc));

          if (c->n > 32) { c->n = 32;		/* 32 donnees au plus ! */
                           k = EINVAL;
                         }
                    else k = 0;

          if (c->n > (FIN_RAM - c->i))	/* Sortie mem. echange */
                         { c->n = FIN_RAM - c->i;
                           k = EINVAL;
                         }

          for (i=c->i, j=0; j<c->n; i++, j++) c->d[j] = L(i);

          VUTIL(zc, c, arg, sizeof(struct asdctampbc));
          RETURN(k);		/* EINVAL si erreur, OK sinon */





       case ASDCGOTBC :  /* Lancement d'une trame BC */

          DISABLE(s);	/* Debut section critique */

             if (dst->fin_bc == 0)
                  { cprintf("fin_bc=%d\n", dst->fin_bc);
                    RESTORE(s);		/* Fin section critique */
                    RETURN(EBUSY);	/* Trame en cours d'execution */
                  }

             i = L(BCCPTR);
             if (i)
               { RESTORE(s);		/* Fin section critique */
                 kkprintf("ioctl(ASDCGOTBC), unit=%d : BCCPTR=0x%X non nul !\n",
                           unit, i);
                 RETURN(EBUSY);	/* Trame en cours d'execution */
               }

             dst->fin_bc = 0;
             i = *((MOT *)arg) & 0xFFFF;
             EIH(i + IMERR, 0, 1000);		/* Fanion "exec. en cours */
             dst->idtrame = i;			/* Ref. trame en cours */
             E(BCIPTR, i);			/* Lancement sequence BC */
             SRESET(&dst->sem_gotbc);  /* Reveil taches qui attendent ... */

          RESTORE(s);	/* Fin section critique */
          RETURN(OK);





       case ASDCSTOPTBC :  /* Arret d'une trame BC */

          /* E(BCCPTR, 0);     Ancien fonctionnement,                    */
          /*                   toujours decrit dans manuel (V6.2) p.9-15 */

          E(BCIPTR, 0xFFFF);	/* Fonctionne sur PMC2 le 13/4/2001 */

          /* La memorisation de l'arret de la trame et le reveil  */
          /* d'eventuelles taches en attente est effectue par la  */
          /* fonction d'IT appelee a l'arret effectif de la trame */

          /* dst->fin_bc = 1;      */
          /* wakeup(&dst->fin_bc); */

          RETURN(OK);





       case ASDCAFINTBC :  /* Attente de la fin d'une trame BC */
          /* --------------------------------------------------------------- */
          /* ---   Debut section critique   ------------------------ vvvvvvv */
          DISABLE(s);
          dprintf("ASDCAFINTBC :   unit=%d   fin_bc=%d\n",
                  unit, dst->fin_bc);

          /* La trame est-elle deja terminee ? */
	  if (dst->fin_bc)
	    { RESTORE(s);
              dprintf("Ioctl(ASDCAFINTBC) : fin_bc != 0\n");
              RETURN(OK);
            }


          /* Demasquage eventuel IT sur AMI */
          if((++dst->nombre_it)==1)
             {    /***********************************/
                  /* DEMASQUAGE IT AMI doit etre ICI */
                  /***********************************/
             }


          /* cprintf("ASDC : j'attend l'IT fin trame BC !\n"); */
	  tmpi = SWAIT(&dst->sem_finbc, SEM_SIGABORT);


          if(!(--dst->nombre_it))
             {

               /*********************************/
               /* MASQUAGE IT AMI doit etre ICI */
               /*********************************/
             }


          RESTORE(s);
          /* ---   Fin section critique   -------------------------- ^^^^^^^ */
          /* --------------------------------------------------------------- */


	  if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
	    { RETURN(EINTR);
	    }

	  if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	    { RETURN(ENETRESET);
	    }


          /* IT arrivee */
          dprintf("ASDC : IT fin trame BC arrivee !\n");
          RETURN(OK);




       case ASDCAGOTBC :  /* Attente du debut d'une trame BC */

          /* --------------------------------------------------------------- */
          /* ---   Debut section critique   ------------------------ vvvvvvv */
          DISABLE(s);

          dprintf("ASDCAGOTBC :   unit=%d   fin_bc=%d\n",
                  unit, dst->fin_bc);

          /* La trame est-elle deja commencee ? */
          if (!dst->fin_bc)
            { RESTORE(s);
              kkprintf("Ioctl(ASDCAGOTBC) : fin_bc != 0\n");
              RETURN(OK);
            }

          /* Demasquage eventuel IT sur AMI */
          if((++dst->nombre_it)==1)
             {    /***********************************/
                  /* DEMASQUAGE IT AMI doit etre ICI */
                  /***********************************/
             }


          /* cprintf("ASDC : j'attend le debut d'une trame BC !\n"); */
	  tmpi = SWAIT(&dst->sem_gotbc, SEM_SIGABORT);


          if(!(--dst->nombre_it))
             {

               /*********************************/
               /* MASQUAGE IT AMI doit etre ICI */
               /*********************************/
             }


          RESTORE(s);
          /* ---   Fin section critique   -------------------------- ^^^^^^^ */
          /* --------------------------------------------------------------- */


	  if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
	    { RETURN(EINTR);
	    }


	  if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	    { RETURN(ENETRESET);
	    }


          /* IT arrivee */
          dprintf("ASDC : IT debut trame BC arrivee !\n");
          RETURN(OK);










       case ASDCSUPTBC :  /* Suppression d'une trame BC en memoire d'echange */

          TDUTIL(arg, sizeof(short));
          DUTIL(zword_ptr, word_ptr, arg, sizeof(short));

          i = (*word_ptr) & 0xFFFF;

          /* - La liberation memoire s'arrete sur la decouverte d'un   */
          /*   bloc n'ayant pas un type connu.                         */
          /* - Le type de tout bloc libere est passe a 0xFFFF de facon */
          /*   a arreter le processus a la fin d'une trame bouclee.    */

          /* Si descripteur de trame pour enchainement : liberation */
          j = LIH(i + IMSUIV, 1000);
          if (j) dst->descr_trame[j].idtrame = 0;
          EIH(i + IMSUIV, 0, 1000);

          while (i)				/* Fin si i==0 */
            { m = L(i) & 0xFF;

              if ((m > BC_DERNIER_TYPE) && (m != BC_SCHED))
                { if (m != 0xFF)    /* -1 peut correspondre a fin de boucle */
                    dprintf("ASDCSUPTBC : Arret sur bloc type %d inconnu\n",
		             m);
                  break;		/* Fin si type "anormal" */
                }


              /* Reveil eventuel tache en attente sur execution du bloc */
	      {
#ifdef LYNXOS
                int * semexbc;
                semexbc = (int *) LI(i + IMSEMBC, 110);
#elif LINUX
                wait_queue_head_t * semexbc;
                semexbc = (wait_queue_head_t *) LI(i + IMSEMBC, 110);
#endif
                if (semexbc)
                  { EI(i + IMSEMBC, 0, 380);  /* Indique suppression trame */
                    SRESET(semexbc);	   /* Reveil tache(s) concernee(s) */
                  }
	      }


              j = L(i+6);	/* Pointeur tampon eventuel */
              k = (m != BC_SCHED) ? L(i+7) : L(i+15);   /* Ptr bloc suivant */
              switch(m)
                 { case COCO : if (j)
                                 { if (L(i+1) & 0x10) asdclibere(dst, j, 32);
                                 }
                               break;

                   case BCRT :    ;
                   case RTBC :    ;
                   case BCRT_DI : if (j)
                                    { asdclibere(dst, j, 32);
                                    }
                                  break;

                   case RTRT :    ;
                   case RTRT_DI : ;
                   default :      ;
                 }

              E(i, 0xFFFF);	/* Marquage bloc pour reperage fin boucle */
              asdclibere(dst, i, (m != BC_SCHED) ? 8 : 16);
              i = k;
            }

          RETURN(OK);







       case ASDCAEXBBC :  /* Attente fin d'execution d'un bloc BC */

          TDUTIL(arg, sizeof(struct asdcbbc));
          DUTIL(zb, b, arg, sizeof(struct asdcbbc));
          TVUTIL(zb, b, arg, sizeof(struct asdcbbc));

          /* Adresse du bloc BC */
          k = b->adrbbc;

          /* On verifie (sommairement) que adrbbc pointe bien un bloc BC */
          if (k == 0)  RETURN(ENOSYS);
          switch (L(k))
            { case COCO :
              case BCRT :
              case RTBC :
              case RTRT :
              case BCRT_DI :
              case RTRT_DI :	break;

              default :		RETURN(ENOSYS);
            }


          /*
          	Initialement, il etait prevu d'utiliser image[k+IMSEMBC]
          	comme semaphore noyau LynxOS.

          	Cependant, l'emploi de ce semaphore pose un probleme :
          		- Il est facile de reveiller une tache en
          		  attente sur ce semaphore a la destruction
          		  de la trame
          		- Il est, par contre, difficile d'en faire
          		  autant a la RAZ du coupleur, a moins de chainer
          		  les blocs BC qui font appel a ce genre de semaphores
          		- Mais il ne reste plus assez de place dans l'image
          		  du bloc BC pour placer les pointeurs necessaires
          		  a ce chainage

          		==> A terme, on pourra utiliser le mecanisme mis en
          		  place pour les wait-queues Linux.

          		==> Pour le moment, on reserve un unique semaphore en
          		  memoire statique device. Ce semaphore est dedie au
          		  gerant generique et un seul gerant generique peut
          		  etre fonctionnel, a un instant donne, sur un device
          		  donne.
          */


          {

#ifdef LYNXOS
            int * semexbc;
#elif LINUX
            wait_queue_head_t * semexbc;
#endif

            /* ------------------------------------------------------------- */
            /* ---   Debut section critique   ---------------------- vvvvvvv */
            DISABLE(s);

               /* Montee du fanion "attente en cours" (est-ce utile ???) */
               tmpi = LIH(k + IMFLUX, 111);
               EIH(k + IMFLUX, tmpi | 0x80, 381);

               /* Armement de l'interruption sur "BC bloc complete" */
               tmpi = L(k + 5);		/* Flags */
               E(k + 5, tmpi | 0x0080);	/* Ajout bit IT sur "bloc complete" */

               /* Pointeur vers le semaphore */
               EI(k + IMSEMBC, (long) &dst->sem_exbc, 382);

               /* Memorisation de l'etat "OK" du bloc */
               i = LI(k + IMSEMBC, 113);

               /* Demasquage eventuel IT sur AMI */
               if ((++dst->nombre_it)==1)
                 {    /***************************************/
                      /* DEMASQUAGE IT ABI pourrait etre ICI */
                     /***************************************/
                 }

               /* Attente sur semaphore */
#ifdef LYNXOS
               semexbc = (int *) LI(k + IMSEMBC, 114);
#elif LINUX
               semexbc = (wait_queue_head_t *) LI(k + IMSEMBC, 114);
#endif
               tmpi = SWAIT(semexbc, SEM_SIGABORT);

               /* Controle de l'etat du bloc */
               j = LI(k + IMSEMBC, 114);

               if (!(--dst->nombre_it))
                 {
                   /*************************************/
                   /* MASQUAGE IT ABI pourrait etre ICI */
                   /*************************************/
                 }

            RESTORE(s);
            /* ---   Fin section critique   ------------------------ ^^^^^^^ */
            /* ------------------------------------------------------------- */
          }

          /* Sortie sur signal ? */
	  if (tmpi) RETURN(EINTR);

          /* Sortie sur RAZ coupleur ? */
	  if (dst->raz) RETURN(ENETRESET);

	  /* Sortie sur suppression trame ? */
	  if (i != j) RETURN(ENETRESET);
	  	/* Pour le moment, on ne distingue pas la sortie */
	  	/* sur suppression de trame de la sortie sur RAZ */
	  	/* coupleur                                      */

	  /* Fin "normale" */
          RETURN(OK);










       case ASDCMBBC :  /* Modification d'un bloc de commande BC */

          TDUTIL(arg, sizeof(struct asdcmbbc));
          DUTIL(ze, e, arg, sizeof(struct asdcmbbc));

          /* Adresse du bloc a modifier */
          i = e->adrbbc & 0xFFFF;

	  /* Adresse dans partie autorisee de memoire echange ? */
	  if ((i < DEBUT_RAM) || (i > (FIN_RAM - 8)))
	    { RETURN(ENOMEM);
	    }

          /* Controle des champs a modifier en fonction du type */
	  /* du bloc BC : la contrainte ci-dessous doit etre    */
	  /* respectee.                                         */
	  /*	- Si bloc schedule, seuls les champs start,     */
	  /*	  reprate et heure peuvent etre modifies        */
	  /*	- Si bloc delai, seul le champ delai peut etre  */
	  /*	  change                                        */
	  /*	- Si bloc ni schedule, ni delai, tous les       */
	  /*      autres champs peuvent etre changes            */
	  /*	- Le champ "type" d'un bloc BC peut etre change */
	  /*	  si, et seulement si :				*/
	  /*		- Le type initial du  bloc n'est pas de */
	  /*		  type schedule ou delai                */
	  /*		- Le nouveau type du bloc n'est pas non */
	  /*		  plus de type schedule ou delai        */
	  if (L(i) & BC_SCHED)
	    {
	    	/* Le bloc est de type schedule */
		if (
		     ( e->valid & (   SDC_MBBC_BLOC
		                    | SDC_MBBC_CMD1
		                    | SDC_MBBC_CMD2
		                    | SDC_MBBC_OPT
			          )
		     ) || e->nbre
		   )			RETURN(EPERM);

		/* Mise a jour du bloc */
		if (e->valid & SDC_MBBC_START) E(i+1, e->bloc);
		if (e->valid & SDC_MBBC_RRATE)  E(i+2, e->bloc);
		if (e->valid & SDC_MBBC_HEURE)
		  { E(i+6, (e->heure >> 16) & 0xFFFF);
		    E(i+7, e->heure & 0xFFFF);
		  }

		RETURN(OK);
	    }
	  else if ((L(i) & 0xFF) == BC_DELAI)
	    {
	    	/* Le bloc est de type delai */
		if (
		     ( e->valid & (   SDC_MBBC_BLOC
		                    | SDC_MBBC_CMD2
		                    | SDC_MBBC_START
		                    | SDC_MBBC_RRATE
		                    | SDC_MBBC_HEURE
			          )
		     ) || e->nbre
		   )			RETURN(EPERM);

		/* Mise a jour du bloc */
		if (e->valid & SDC_MBBC_CMD1) E(i+6, e->cmd1);
		if (e->valid & SDC_MBBC_OPT)
		  { int opt = L(i+5);

		    /* Modif. de chaque bit de opt en fonction des bits */
		    /* correspondants des champs opt_0 et opt_1         */
		    opt =   (e->opt_0 & e->opt_1 & ~opt)
		          | ((opt | e->opt_1) & ~e->opt_0);

		    E(i+5, opt);
		  }

		RETURN(OK);
            }
	  else
	    {
	    	/* Le bloc est de type TRANSFERT */
		if ( e->valid & (   SDC_MBBC_START
		                  | SDC_MBBC_RRATE
		                  | SDC_MBBC_HEURE
			        )
		   )			RETURN(EPERM);

	        /* Tentative pour changer le type du bloc */
		/* en "schedule" OU "DELAI" ?             */
		if (    SDC_MBBC_BLOC
		     && (    (e->bloc & BC_SCHED)
		          || ((e->bloc & 0xFF) == BC_DELAI)
			)
		   )
		     RETURN(EPERM);

		/* Mise a jour du bloc */
		if (e->valid & SDC_MBBC_BLOC) E(i, e->bloc);
		if (e->valid & SDC_MBBC_CMD1) E(i+1, e->cmd1);
		if (e->valid & SDC_MBBC_CMD2) E(i+2, e->cmd2);
		if (e->valid & SDC_MBBC_OPT)
		  { int opt = L(i+5);

		    /* Modif. de chaque bit de opt en fonction des bits */
		    /* correspondants des champs opt_0 et opt_1         */
		    opt =   (e->opt_0 & e->opt_1 & ~opt)
		          | ((opt | e->opt_1) & ~e->opt_0);

		    E(i+5, opt);
		  }

		/* Mise a jour eventuelle des donnees */
		j = L(i+6) & 0xFFFF;
		l = e->nbre;
		if (l>32) RETURN(EINVAL);
		for (k=0; k<l; k++)
		  { E(j+k, e->d[k]);
		  }

		RETURN(OK);
	    }











     /* Enchainement de 2 trames */
     case ASDCCHTRAME :
        TDUTIL(arg, sizeof(struct chtrame));
        DUTIL(zcht, cht, arg, sizeof(struct chtrame));
        {
          int i;
          int trame1 = cht->trame1;
          int trame2 = cht->trame2;

          /* Les trames existent-t-elle ? (verif. tres sommaire ...) */
          if (!L(trame1) || !L(trame2)) RETURN(EADDRNOTAVAIL);

          /* La trame 1 n'est elle pas deja enchainee ? */
          if (LIH(trame1 + IMSUIV, 1000)) RETURN(EBUSY);

          /* Recherche d'un descripteur de trame disponible */
          for (i=1; i<MAXDESCRT; i++)
             { if (dst->descr_trame[i].idtrame == 0) break;
             }
          if (i >= MAXDESCRT) RETURN(ENOMEM);

          /* Association du descripteur a la trame 2*/
          dst->descr_trame[i].idtrame = trame2;
          dst->descr_trame[i].periode = cht->periode;
          dst->descr_trame[i].minpmaj = cht->minpmaj;
          dst->descr_trame[i].cycles = cht->cycles;

          /* Connexion a la trame 1 */
          EIH(trame1 + IMSUIV, i, 1000);

          RETURN(OK);
	}











       case ASDCLETAT : /* Lecture des indicateurs d'etat du pilote */
                        /* (pour debug uniquement)                  */
  	  {
  	    struct asdcetapil *etat, zetat;

	    TVUTIL(zetat, etat, arg, sizeof(struct asdcetapil));

            etat->asdc_nbopen = dst->nbopen;
            etat->asdc_nombre_it = dst->nombre_it;
            etat->asdc_monok = dst->monok;
            etat->asdc_fin_bc = dst->fin_bc;
            etat->asdc_stocke_bc = dst->stocke_bc;

            // Ci-dessous : variables utilisees sur CMBA uniquement
            //					==> supprimees le 24/9/2002
            // etat->asdc_n_bc_it = dst->n_bc_it;
            // etat->asdc_ptbc_ie = dst->ptbc_ie;
            // etat->asdc_ptbc_il = dst->ptbc_il;
            // etat->asdc_ntbc = dst->ntbc;
            // etat->asdc_numebc = dst->numebc;

            etat->nombre_tampons_flux = dst->nombre_tampons_flux;
            etat->pbcf = dst->pbcf;
            etat->nb_tamp_flux_dispos = dst->nb_tamp_flux_dispos;
            etat->pbcfl = dst->pbcfl;
            etat->pflux = dst->pflux;
            etat->dflux = dst->dflux;

	    VUTIL(zetat, etat, arg, sizeof(struct asdcetapil));
            RETURN(OK);
  	  }







       case ASDCLVER :  /* Lecture des numeros de version du pilote et du */
                        /* firmware de la carte ABI.                      */
                        /*    ATTENTION : La version du firmware n'est    */
                        /*                pas significative si le dit     */
                        /*                firmware n'est pas initialise   */
                        /*                et en cours de fonctionnement.  */

          { struct asdcver *ve, zve;
            char *p, *q;

	    TVUTIL(zve, ve, arg, sizeof(struct asdcver));

            /* Copie du nom du pilote */
            p = ASDC_NOM;
            q = ve->nom;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie de la version du pilote */
            p = ASDC_VERSION;
            q = ve->version;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie de la date du pilote */
            p = ASDC_DATE;
            q = ve->date;
            while (*p) { *q++ = *p++; }
            *q = '\0';


            /* Copie des versions et revisions du DSP et du firmware     */
            /* (valide seulement si firmware en cours de fonctionnement) */
            ve->vdsp = L(PCODE);
            ve->rdsp = L(VCODE);
            ve->vfirm = L(FWPROC);
            ve->rfirm = L(FWVERC);

            /* Copie des adresses et du numero du coupleur */
            ve->bvba = (long) dst->bvba;
            ve->bpba = (long) dst->bpba;
            ve->signal_number = (int) dst->signal_number;

            /* Envoi a l'application de la structure renseignee */
            VUTIL(zve, ve, arg, sizeof(struct asdcver));

            RETURN(OK);
          }












       case ASDCLDATE :  /* Demande de lecture de l'heure a l'horloge */
                         /* de datation IRIG de la carte ABI          */
          {
            short ccw;

            DISABLE(s);		/* Debut section critique */

            ccw = L(CCW);

            /* Si CCW n'est pas nul, lecture heure deja demandee */
            if (ccw)
              {
                RESTORE(s);	/* Fin section critique */

                RETURN(EBUSY);
              }

            // E(FIN_RAM - 1, 0x9999);   /* Marqueur debug pour VMETRO */

            /* Sinon, demande de lecture de l'heure */
            E(CCW, 1);

            RESTORE(s);		/* Fin section critique */

            RETURN(OK);
          }





       case ASDCLIRIG :  /* Attente de disponibilite, puis acquisition */
                         /* de l'heure demandee par ASDCLDATE          */
          {
            struct asdcirig *ti, zti;

            TVUTIL(zti, ti, arg, sizeof(struct asdcirig));

            DISABLE(s);		/* Debut section critique */

            tmpi = 0;
            if (L(CCW))	/* Heure disponible ? */
              {
                /* Non : alors attente sur le semaphore */
                if ((++dst->nombre_it)==1)  { /* Demasquage IT */ }

                tmpi = SWAIT(&dst->semirig, SEM_SIGABORT);

                // E(FIN_RAM - 1, 0x7777);   /* Marqueur debug pour VMETRO */

                if (!(--dst->nombre_it))    { /* Masquage IT */ }
              }

            if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
              {
                RESTORE(s);	/* Fin section critique */
                RETURN(EINTR);
              }

	    if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	      {
                RESTORE(s);	/* Fin section critique */
            	RETURN(ENETRESET);
	      }

            /* L'heure est maintenant disponible */
            ti->schighi = L(SCHIGHI);
            ti->scmidi = L(SCMIDI);
            ti->sclowi = L(SCLOWI);
            ti->sclow = L(SCLOW);
            ti->schigh = L(SCHIGH);

            RESTORE(s);		/* Fin section critique */

	    VUTIL(zti, ti, arg, sizeof(struct asdcirig));
            RETURN(OK);
          }





       case ASDCHCALIBR : /* Calibration des horloges :                   */
                          /*   ==> Mesure (aussi simultanee que possible) */
                          /*       de l'heure IRIG SBS et de l'heure      */
                          /*       systeme :                              */
                          /*                                              */
                          /*   1 - Lecture TSC (1)                        */
                          /*   2 - Demande lecture heure IRIG             */
                          /*   3 - Lecture TSC (2)                        */
                          /*   4 - Lecture horloge systeme                */
                          /*   5 - Lecture TSC (3)                        */
                          /*   6 - Attente disponibilite heure IRIG       */
                          /*   7 - Acquisition heure IRIG                 */
          {

#ifdef LYNXOS
            unsigned long sec;
            int nano;
#elif LINUX
            unsigned long long tsc1, tsc2, tsc3;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
            struct timespec64 tsys;
#else
            struct timespec tsys;
#endif
#endif
            short ccw;
            struct asdchcalibr *ti, zti;

            TVUTIL(zti, ti, arg, sizeof(struct asdchcalibr));
            // printk("sizeof(struct asdchcalibr)) = %d\n",
            //         sizeof(struct asdchcalibr));

            DISABLE(s);		/* Debut section critique */

            /* Test etat du systeme de lecture heure IRIG */
            ccw = L(CCW);

            /* Si CCW n'est pas nul, lecture heure deja demandee */
            if (ccw)
              {
                RESTORE(s);	/* Fin section critique */

                RETURN(EBUSY);
              }


#ifdef LINUX
            /* Lecture eventuelle du TSC (1) */
            __asm__ volatile ("rdtsc" : "=A"(tsc1));
#endif

            /* Demande de lecture de l'heure IRIG */
            E(CCW, 1);

#ifdef LINUX
            /* Lecture eventuelle du TSC (2) */
            __asm__ volatile ("rdtsc" : "=A"(tsc2));
#endif

            /* Lecture de l'heure systeme */
#ifdef LYNXOS
            nano = nanotime(&sec);
#elif LINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
            ktime_get_coarse_real_ts64(&tsys); 
#else
            tsys = current_kernel_time();
#endif
#endif

#ifdef LINUX
            /* Lecture eventuelle du TSC (3) */
            __asm__ volatile ("rdtsc" : "=A"(tsc3));
#endif

            /* Attente de la disponibilite de l'heure IRIG */
            tmpi = 0;
            if (L(CCW))	/* Heure disponible ? */
              {
                /* Non : alors attente sur le semaphore */
                if ((++dst->nombre_it)==1)  { /* Demasquage IT */ }

                tmpi = SWAIT(&dst->semirig, SEM_SIGABORT);

                // E(FIN_RAM - 1, 0x7777);   /* Marqueur debug pour VMETRO */

                if (!(--dst->nombre_it))    { /* Masquage IT */ }
              }

            if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
              {
                RESTORE(s);	/* Fin section critique */
                RETURN(EINTR);
              }

	    if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	      {
                RESTORE(s);	/* Fin section critique */
            	RETURN(ENETRESET);
	      }

            /* L'heure est maintenant disponible */
            ti->schighi = L(SCHIGHI);
            ti->scmidi = L(SCMIDI);
            ti->sclowi = L(SCLOWI);
            ti->sclow = L(SCLOW);
            ti->schigh = L(SCHIGH);
#ifdef LYNXOS
            ti->sys_s = sec;
            ti->sys_ns = nano;
            ti->tsc1_h = 0;
            ti->tsc1_l = 0;
            ti->tsc2_h = 0;
            ti->tsc2_l = 0;
            ti->tsc3_h = 0;
            ti->tsc3_l = 0;
#elif LINUX
            ti->sys_s = tsys.tv_sec;
            ti->sys_ns = tsys.tv_nsec;
            ti->tsc1_h = (tsc1 >> 32) & 0xFFFFFFFF;
            ti->tsc1_l = tsc1 & 0xFFFFFFFF;
            ti->tsc2_h = (tsc2 >> 32) & 0xFFFFFFFF;
            ti->tsc2_l = tsc2 & 0xFFFFFFFF;
            ti->tsc3_h = (tsc3 >> 32) & 0xFFFFFFFF;
            ti->tsc3_l = tsc3 & 0xFFFFFFFF;

            printk("------- ASDCHCALIBR :\n");
            printk("tsc1, 2, 3 : %016Lx %016Lx %016Lx\n", tsc1, tsc2, tsc3);
            printk("tsc _h, _l : %08x%08x %08x%08x %08x%08x\n",
                    ti->tsc1_h, ti->tsc1_l,
                    ti->tsc2_h, ti->tsc2_l,
                    ti->tsc3_h, ti->tsc3_l );
#endif

            RESTORE(s);     /* Fin section critique */


	    VUTIL(zti, ti, arg, sizeof(struct asdchcalibr));
            RETURN(OK);
          }







       case ASDCRAZHISTO :  /* RAZ histogramme utilisation table ITs */
          { int i;

            DISABLE(s);
               dst->nbhistoit = 0;
               for (i=0; i<ASDCTHISTOIT; i++) dst->histoit[i] = 0;
               dst->deborde = 0;
            RESTORE(s);

            RETURN(OK);
          }



       case ASDCLECHISTO :  /* Lecture histogramme utilisation table ITs */
          {
            struct asdchistoit *h, zh;
            int i;

            TVUTIL(zh, h, arg, sizeof(struct asdchistoit));

            DISABLE(s);
               h->nombre = dst->nbhistoit;
               for (i=0; i<ASDCTHISTOIT; i++) h->histo[i] = dst->histoit[i];
               h->deborde = dst->deborde;
            RESTORE(s);

            VUTIL(zh, h, arg, sizeof(struct asdchistoit));
            RETURN(OK);
          }




       case ASDCALLOUE :  /* Allocation d'un bloc de la memoire d'echange ABI */
          {
            int n;

            TDUTIL(arg, sizeof(int));
            DUTIL(zint_ptr, int_ptr, arg, sizeof(int));
            TVUTIL(zint_ptr, int_ptr, arg, sizeof(int));

            /* Les 16 bits de poids faible pointes par int_ptr contiennent */
            /* la taille du bloc a allouer                                 */

            n = asdcalloc(dst, *int_ptr & 0xFFFF);

            /* L'allocation s'est elle correctement deroulee ? */
            if ( n == -1) RETURN(ENOMEM);

            /* Il n'y a pas eu de probleme ! */

            *int_ptr = n & 0xFFFF;
            /* Les 16 bits de poids faible pointes par int_ptr contiennent */
            /* l'adresse du bloc alloue                                    */

            TVUTIL(zint_ptr, int_ptr, arg, sizeof(int));
            RETURN(OK);
          }






      case ASDCLIBERE :  /* Liberation d'un bloc de la memoire d'echange ABI */
          {
            int n, a;

            TDUTIL(arg, sizeof(int));
            DUTIL(zint_ptr, int_ptr, arg, sizeof(int));

            /* - Les 16 bits de poids fort pointes par int_ptr contiennent   */
            /*   la taille du bloc a liberer                                 */
            /* - Les 16 bits de poids faible pointes par int_ptr contiennent */
            /*   l'adresse du bloc a liberer                                 */

            a = *int_ptr & 0xFFFF;		/* Adresse du bloc */
            n = (*int_ptr >> 16) & 0xFFFF;	/* Taille du bloc */

            if (asdclibere(dst, a, n) == -1) RETURN(EADDRNOTAVAIL);

            /* Il n'y a pas eu de probleme ! */
            RETURN(OK);
          }


#ifdef SUITESUITESUITE2


       case ASDCLECMIM :  /* Lecture du masque IT des commandes codees */
          w = (struct asdcmim *)data;

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          w->bus &= 0x01;
          w->adresse &= 0x1F;

          /* La table des MIM existe-t-elle ? */
          if ((i = a->r.mimptr) == 0) return(ESPIPE);
                                         /* Ne devrait jamais se produire ! */

          /* Lecture du MIM */
          j = i + 2 * w->adresse + (w->bus ? 64 : 0);
          w->mim = ((a->m[j] << 16) & 0xFFFF0000) || (a->m[j+1] & 0xFFFF);

          return 0;



       case ASDCECRMIM :  /* Ecriture du masque IT des commandes codees */

          w = (struct asdcmim *)data;

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          w->bus &= 0x01;
          w->adresse &= 0x1F;

          /* La table des MIM existe-t-elle ? */
          if ((i = a->r.mimptr) == 0) return(ESPIPE);
                                         /* Ne devrait jamais se produire ! */

          /* Ecriture du MIM */
          j = i + 2 * w->adresse + (w->bus ? 64 : 0);
          a->m[j] = (w->mim >> 16) & 0xFFFF;
          a->m[j+1] = w->mim & 0xFFFF;

          return 0;



       case ASDCLECCC :  /* Lecture dern. commande codee ayant provoquee IT */

          w = (struct asdcmim *)data;

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          w->bus &= 0x01;
          w->adresse &= 0x1F;

          w->mim = dst->cocorec[w->bus][w->adresse] & 0xFFFF;

          return 0;



       case ASDCLECCCIT :  /* Ajout bit(s) au MIM et attente commande codee */
         /******************************************************************/
         /*   ATTENTION : Il n'y a pas de queue ==> Si 2 C.C. se succedent */
         /*      rapidement, cette fonction permettra a l'application de   */
         /*      recuperer la premiere d'entre elles, mais la seconde sera */
         /*      certainement perdue !                                     */
         /******************************************************************/

          w = (struct asdcmim *)data;

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          w->bus &= 0x01;
          w->adresse &= 0x1F;

          /* La table des MIM existe-t-elle ? */
          if ((i = a->r.mimptr) == 0) return(ESPIPE);
                                         /* Ne devrait jamais se produire ! */

          /* Ajout de bits au MIM */
          j = i + 2 * w->adresse + (w->bus ? 64 : 0);
          a->m[j] |= (w->mim >> 16) & 0xFFFF;
          a->m[j+1] |= w->mim & 0xFFFF;

          /* Attente de l'IT */

          /* --------------------------------------------------------------- */
          /* ---   Debut section critique   ------------------------ vvvvvvv */
          s = splx(pritospl(md->md_intpri));

          /* Demasquage eventuel IT sur AMI */
          if((++dst->nombre_it)==1)
             {
                  /***********************************/
                  /* DEMASQUAGE IT AMI doit etre ICI */
                  /***********************************/
             }

          dprintf("ASDC (LECCCIT) : j'attend une IT !\n");
          sleep((caddr_t *) &dst->cocorec[w->bus][w->adresse], PZERO+1);

          if(!(--dst->nombre_it))
             {
               /*********************************/
               /* MASQUAGE IT AMI doit etre ICI */
               /*********************************/
             }

          /* Passage a l'application du mot de commande recu */
          w->mim = dst->cocorec[w->bus][w->adresse];


          splx(s);
          /* ---   Fin section critique   -------------------------- ^^^^^^^ */
          /* --------------------------------------------------------------- */

          /* IT arrivee */
          dprintf("ASDC : Une IT \"Commande Codee\" est arrivee !\n");

          return 0;






#endif /* SUITESUITESUITE2 */










       case ASDC_ABO_MF :	/* Modification du mot de la table des */
     				/* filtres associe a une voie RT       */
          asdcabomf = 1;

          /* Dans la structure asdcvoie en entree :                    */
	  /*                                                           */
	  /*	- les champs adresse, sous_adresse et direction        */
	  /*      definissent la voie                                  */
	  /*                                                           */
	  /*	- les champs adrtamp et adrtamp2 definissent, pour     */
	  /*      chacun des bits du filtre, les modifications a       */
	  /*      effectuer :                                          */
	  /*          adrtamp  adrtamp2            Action              */
	  /*         --------  --------    --------------------------- */
	  /*             0         0           Bit inchange            */
	  /*             1         0           Bit mis a 0             */
	  /*             0         1           Bit mis a 1             */
	  /*             1         1           Bit modifie             */
	  /*                                                           */

       case ASDC_ABO_MFX :	/* Modification du mot de la table des */
     				/* filtres associe a une voie RT       */
                                /* Fonction analogue a ASDC_ABO_MF,    */
                                /* mais generation de plusieurs        */
                                /* (jusqu'a 32) erreurs de parite      */
                                /* transitoires en fonction des bits   */
                                /* du champ ntamp.                     */

          /* Difference par rapport a ASDC_ABO_MF : le champ ntamp de  */
          /* la structure asdcvoie contient une liste de bits.         */

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
	  v->adresse &= 0x1F;
          v->sous_adresse &= 0x1F;
          v->direction &= 1;
          v->mode &= RT_ETRANSIT;

          if (asdcabomf) liste = 1;   /* Si ASDC_ABO_MF, au plus 1 erreur */
                    else liste = v->ntamp;

          /* La table des sous_adresses existe-t-elle ? */
          if ((i = L(L(ATPTR) + v->adresse)) == 0)
	    { cprintf("ASDC_ABO_MF: Echec 21\n");
              RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
	    }

          /* Pointeur vers zone des donnees en memoire image */
          zd = LI(i + v->sous_adresse + (v->direction ? 32 : 0), 13);
          if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
            { cprintf("ASDC_ABO_MF: Echec zd (zd[0x%X] = 0x%X)\n",
                       i + v->sous_adresse + (v->direction ? 32 : 0), zd);
              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          /* Le contenu de la memoire image semble-t-il correct ? */
          cmd =   (v->adresse << 11)
                | (v->direction ? 0x400 : 0)
                | (v->sous_adresse << 5);
          if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
            { cprintf("ASDC_ABO_MF: Echec 24 - ");
              cprintf("LI(zd+IRCMD)=0x%08lX   cmd=0x%08X\n",
                       LI(zd+IRCMD, 14) & 0xFFFFFFFF, cmd);

              RETURN(ESPIPE);		/* Memoire image anormale ! */
            }

          /* Mode de la voie (synchrone, asynchrone, etc...) */
          m = LI(zd + IRMODE, 15);

          /* On ne fait aucun test sur l'existence de la table des filtres */
          /* Si la table des sous-adresses existe, cette table DOIT        */
          /* exister aussi !                                               */

          /* Table des filtres */
          j = L(FTPTR) + v->adresse;
          k = L(j);

          /* Calcul de l'adresse du filtre */
          k += v->sous_adresse + ( v->direction ? 32 : 0);

          /* La voie existe-t-elle ? */
          j = L(i + v->sous_adresse +  (v->direction ? 32 : 0));
          if ((j == 0) && (LI(k, 0) == 0) && (m != RT_VSYNC2))
            { cprintf("ASDC_ABO_MF: Echec 22 - ");
              cprintf(" j=%d m=%d\n", j, m);
              RETURN(EADDRNOTAVAIL); /* La voie n'a jamais ete declaree ! */
            }
          /* On considere que la voie existe si :
                 - Un pointeur de tampons est defini
              ou - L'image du filtre est non nulle (cas d'une voie inhibee)
              ou - la voie est en mode synchrone 2
             ### Il faudrait sans doute rationaliser ce type      ###
             ### de test en utilisant un unique indicateur fiable ###
             ### (par exemple un fanion en memoire image).        ###
          */

          /* Modif. de chaque bit de opt en fonction des bits */
          /* correspondants des champs adrtamp et adrtamp2    */
	  { int filtre, memofiltre;

            /* -------------------------------------------- */
            /* ---   Debut section critique   ----- vvvvvvv */
            DISABLE(s);

              /* Inhibition des ITs pour eviter pb si modification   */
              /* filtre (mem) et/ou memofiltre (image) simultanement */
              /* par ioctl(ASDC_ABO_MF) et fonction d'IT ...         */

              /* Memorisation de l'adresse du filtre en memoire image */
              /* (pour accelerer le traitement de l'interruption)     */
              EI((zd+IRAFILTRE), k, 0);

              /* Lecture filtre courant */
              filtre = memofiltre = L(k);

              /* Modif. de chaque bit de opt en fonction des bits */
              /* correspondants des champs adrtamp et adrtamp2    */
              filtre =   (v->adrtamp & v->adrtamp2 & ~filtre)
                       | ((filtre | v->adrtamp2) & ~v->adrtamp);

              /* Memorisation eventuelle des filtres en memoire d'image */
              if (v->mode)
                {
                  /* Nombre total courant d'erreurs de parite */
                  int nb_err = L(ERRPAR_NB);

                  /* Memorisation des 2 filtres OK et Erreur */
                  EI((zd+IRMEMOF), (   (memofiltre & 0xFFFF)
                                     | 0x80000000
                                     | ((nb_err << 16) & 0x7FFF0000) ), 0);
                  EI((zd+IRMEMOE), (filtre & 0xFFFF), 0);

                  /* La valeur du filtre courant depend du bit 0 de liste */
                  if (liste & 1) E(k, filtre);
                    else         E(k, memofiltre);
                  EI((zd+IRTLET), liste, 0);
                }
              else
                { /* Nouvelle valeur imposee au filtre,  */
                  /* inhibition des erreurs transitoires */
                  E(k, filtre);
                  EI((zd+IRMEMOF), 0, 0);
                  EI((zd+IRMEMOE), 0, 0);
                  EI((zd+IRTLET), 0, 0);
                }

             RESTORE(s);
             /* ---   Fin section critique   ------- ^^^^^^^ */
             /* -------------------------------------------- */
          }

          RETURN(OK);









       case ASDC_ABO_IV :	/* Inhibition d'une "voie" (adr, sa, sens) */
     				/* d'un abonne                             */

          /* Dans la structure asdcvoie en entree :                    */
	  /*                                                           */
	  /*	- les champs adresse, sous_adresse et direction        */
	  /*      definissent la voie                                  */
	  /*                                                           */

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
	  v->adresse &= 0x1F;
	  v->sous_adresse &= 0x1F;
	  v->direction &= 1;

          /* Allocation si necessaire de la table des filtres */
          /* et initialisation de son image                   */
          j = L(FTPTR) + v->adresse;
          k = L(j);
          if (k==0)
             { k = asdcalloc(dst, 64);
	       if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
                          }
               E(j, k);
               for (i=k; i<k+64; i++)
                  { E(i, 0);  /* Init a 0 */
                    EI(i, 0, 387);
                  }
             }
          /*### ### ###
              Le code ci-dessus est inutile : Si la voie existe
              (test fait ci-dessous), la table des filtres
              existe forcement aussi !!!
            ### ### ###*/

          /* Calcul de l'adresse du filtre */
          k += v->sous_adresse + ( v->direction ? 32 : 0);

	  /* Calcul de l'adresse de la table des sous-adresses */
	  l = L(ATPTR) + v->adresse;
	  l = L(l);
	  if (l == 0) { return(EADDRNOTAVAIL);	/* Abonne non defini ! */
	  	      }

	  /* Calcul de l'adresse du pointeur des tampons */
	  l += v->sous_adresse + ( v->direction ? 32 : 0);

	  /* Adresse des tampons */
	  m = L(l);

	  /* Si pointeur nul, voie inexistante ou deja inhibee */
	  if (m == 0) { return(EADDRNOTAVAIL);	/* Voie non definie ! */
	  	      }

          /* Sauvegarde adresse des tampons dans l'image */
          /* de la table des filtres (FTPTR)             */
          EI(k, m, 388);

          /* Inhibition voie */
          E(l, 0);
          usec_sleep(1);	/* Attente 1 us */
          E(l, 0);		/* Si acces concurrent par firmware */

          RETURN(OK);






       case ASDC_ABO_VV :	/* Revalidation d'une "voie"     */
     				/* (adr, sa, sens) d'un abonne   */

          /* Dans la structure asdcvoie en entree :                    */
	  /*                                                           */
	  /*	- les champs adresse, sous_adresse et direction        */
	  /*      definissent la voie                                  */
	  /*                                                           */

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));

          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
	  v->adresse &= 0x1F;
	  v->sous_adresse &= 0x1F;
	  v->direction &= 1;

	  /* Calcul de l'adresse de la table des sous-adresses */
	  l = L(ATPTR) + v->adresse;
	  l = L(l);
	  if (l == 0) { return(EADDRNOTAVAIL);	/* Abonne non defini ! */
	  	      }

	  /* Calcul de l'adresse du pointeur des tampons */
	  l += v->sous_adresse + ( v->direction ? 32 : 0);

	  /* Si pointeur non nul, voie non inhibee ! */
	  if (L(l) != 0) { return(EADDRINUSE);	/* Voie non inhibee ! */
	  	         }


          /* Recherche de la table des filtres */
          j = L(FTPTR) + v->adresse;
          k = L(j);
          if (k==0)
             { /* Table des filtres non initialisee                        */
               /*   ==> voie non definie, puisque pointeur tampons est nul */
	       RETURN(EADDRNOTAVAIL);
	     }

          /* Calcul de l'adresse du filtre */
          k += v->sous_adresse + ( v->direction ? 32 : 0);

          /* Recuperation de l'adresse des tampons */
          m = LI(k, 115);
          if (m == 0)
            { /* Adresse tampons sauvevardee est nulle ! */

              /* Si la voie est en mode synchrone 2, nous sommes */
              /* en presence d'une voie valide, et il n'y a rien */
              /* de plus a faire (retour du code EADDRINUSE)     */
              /* Sinon, la voie n'a pas ete definie (retour du   */
              /* code EADDRNOTAVAIL)                             */
              /*    ==> il faut determiner le mode de la voie    */

              /* Pointeur vers zone des donnees en memoire image */
//TODO : La ligne cidessous peut conduire à un panic (i non initialise.
//Toutefois, je ne suis pas sur que cet ioctl soit nécessaire
              zd = LI(i + v->sous_adresse + (v->direction ? 32 : 0), 13);
              if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
                { cprintf("ASDC_ABO_VV: Echec zd (zd[0x%X] = 0x%X)\n",
                           i + v->sous_adresse + (v->direction ? 32 : 0), zd);
                  RETURN(ESPIPE);		/* Memoire image anormale ! */
                }

              /* Le contenu de la memoire image semble-t-il correct ? */
              cmd =   (v->adresse << 11)
                    | (v->direction ? 0x400 : 0)
                    | (v->sous_adresse << 5);
              if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
                { cprintf("ASDC_ABO_VV: Echec 24 - ");
                  cprintf("LI(zd+IRCMD)=0x%08lX   cmd=0x%08X\n",
                           LI(zd+IRCMD, 14) & 0xFFFFFFFF, cmd);

                  RETURN(ESPIPE);		/* Memoire image anormale ! */
                }

              /* Mode de la voie (synchrone, asynchrone, etc...) */
              m = LI(zd + IRMODE, 15);

              /* Si mode "synchrone 2", la voie existe et est valide */
              if (m == RT_VSYNC2) RETURN(EADDRINUSE);

              /* Sinon, la voie est non definie, puisque le pointeur */
              /* des tampons est nul                                 */
	      RETURN(EADDRNOTAVAIL);
	    }

          /* Restauration adresse tampons */
          E(l, m);
          EI(k, 0, 389);

          RETURN(OK);










       case ASDC_ABO_LIBERER :        /* Suppression d'un abonne 1553 */

          { TDUTIL(arg, sizeof(struct asdcvoie));
            DUTIL(zv, v, arg, sizeof(struct asdcvoie));

 /* Ce qu'il faut faire :

 	1 - Controle existence abonne
 	2 - inhibition abonne (dans SWTPTR)
 	3 - Pour toutes les CC :
 		- Si connexion CEVT :
 			- envoi message "suppression abonne"
 			- deconnexion CEVT
 	4 - Pour toutes les voies existantes :
 		- Envoi signal "reset" sur semaphore associe a la voie
 		- Si connexion CEVT :
 			- envoi message "suppression abonne"
 			- deconnexion CEVT
 		- Si voie inhibee, restauration de la chaine des tampons
 		- Si voie liee, suppression du lien
 		- Mise a 0 du pointeur des tampons
 		- Mise a zero de la memoire image associee
 		- Liberation des tampons
 	5 - Liberation de la table des sous-adresses
 	6 - NE PAS LIBERER la table des filtres (sert a l'espion)
 	7 - Autres tables a liberer ???
 */


            /* Controle de la validite des parametres :                 */
            /*   - Pour le moment, pas de controle a proprement parler, */
            /*  on se contente de supprimer tous les bits inutilises    */
            v->adresse &= 0x1F;

	    /* RT programme en "espion TR" ? */
	    espion_tr = L(L(PROPTR) + v->adresse) & 2;

	    /* Inhibition du RT via la table des status */
            E(L(SWTPTR) + v->adresse, 0);

	    /* Table des sous-adresses */
            j = L(L(ATPTR) + v->adresse);

	    /* Table des filtres */
            k = L(L(FTPTR) + v->adresse);

            if ((j == 0) || (k == 0))  /* Table inexistante ==> pas d'abonne */
              { RETURN(EADDRNOTAVAIL);
              }

#ifdef CEVT
            /* Squelette d'un mot de commande CC */
 	    cmd = ((v->adresse << 11) & 0xF800) | 0x3E0;

            /* Parcours des commandes codees pour signaler la suppression */
            /* de l'abonne aux eventuels CEVT connectes aux CC            */
            for (i=0; i < (COCO_MAX + 1); i++)
               {
                 /* Un CEVT est-il connecte ? */
                 mm = dst->cocor[v->adresse][i].cevt;
                 if (mm)
                   {
                     /* si oui, signaler la RAZ (si possible) */
                     cevt_signaler(mm, CEVT_SDCABOSUPPR,
                                     dst->signal_number,
                                     cmd
                                     | (((asdc_coco[i].def & 1) << 10) & 0x400)
                                     | (i & 0x1F),
                                     0);

                     /* puis effectuer la deconnexion */
                     dst->cocor[v->adresse][i].cevt = 0;
                   }
               }
#endif


            /* Parcours des sous-adresses    *** Sauf 0 et 31 (CC) *** */
            for (i=1; i<31; i++)
               {
                 /*****************************/
                 /* Sous-adresse en reception */
                 /*****************************/

                 /* La voie existe-t-elle ?                          */
                 /* L'image de la table des filtres doit etre testee */
                 /* au meme titre que le pointeur de tampons         */
                 tampapp = LI(k + i, 0);
                 if (tampapp == 0) tampapp = L(j + i);
                 if (tampapp)
                   { /* La voie existe !                            */
                     /* et tampapp pointe la chaine des tampons que */
                     /* la voie soit inhibee ou non.                */

                     /* Inhibition ITs */
                     E(k + i, L(k + i) & ~0x7A);

                     zd = LI(j + i, 0);

                     /* Si adresse zd anormale, on abandonne ... */
                     if ((zd < DEBUT_RAM) || (zd >= FIN_RAM)) RETURN(EDOM);


                     /* Envoi signal RESET sur semaphore associe a la voie  */
                     /* (Reveil eventuelle tache en attente sur cette voie) */
                     // cprintf("   sreset RT%d,%d %c",
                     //              i, j % 2, (j>31) ? 'T' : 'C');
#ifdef LINUX
                     // cprintf("   IMA[0x%04X] = 0x%08X\n",
                     //                     zd + IRSEM, LI(zd + IRSEM, 6));
                     /* Si aucun SWAIT_IM n'a encore ete effectue  */
                     /* sur la voie, le pointeur de wait-queue est */
                     /* nul. Il faut alors ne pas l'utiliser !     */
                     /* (sous peine de segmentation fault !)       */
                     if (LI(zd + IRSEM, 6) != 0)
                       { SRESET_IM(&LI(zd + IRSEM, 6));
                       }
#else
                      // cprintf("\n");
                      SRESET_IM(&LI(zd + IRSEM, 6));
#endif   /* LINUX */


#ifdef CEVT
                     /* On n'utilise pas le mutex habituel pour proteger */
                     /* le code ci-dessous car il semble deraisonnable   */
                     /* qu'une tache tente de connecter une voie a un    */
                     /* CEVT pendant qu'une autre tache supprime cette   */
                     /* voie : un minimum de synchronisation doit etre   */
                     /* fourni par les applications elles-memes !        */

                     /* La voie est-elle connectee a un CEVT ? */
                     mm = LI(zd + IRCEVT, 0);
                     if (mm)
                       {
                         /* On signale, si possible, la deconnexion au CEVT */
                         cevt_signaler(mm, CEVT_SDCABOSUPPR,
                                                  dst->signal_number,
                                                  ((v->adresse << 11) & 0xF800)
                                                  | ((i << 5) & 0x3E0),
                                                  0);

                         /* Deconnexion de la voie */
                         EI(zd + IRCEVT, 0, 324);
                       }
#endif

                     /* La voie est-elle liee ?                   */
                     /* Si c'est le cas, le mot de commande place */
                     /* dans la zone zd est celui d'une voie en   */
                     /* emission (bien que la voie courante soit  */
                     /* en reception)                             */
                     vliee = LI(zd + IRCMD, 0) & 0x400;

                     /* Deconnexion voie courante de */
                     /* la chaine des tampons        */
                     EI(k + i, 0, 0);
                     E(j + i, 0);
                     usec_sleep(1);
                     E(j + i, 0);


                     /* Si la voie est liee, on laisse a la voie en */
                     /* emission le soin de liberer le tampon       */
                     if (!vliee)
                       {
                         /* Pour une voie en reception, la structure */
                         /* des tampons ne depend pas du mode de la  */
                         /* voie                                     */
                         if (liberer_tamp(dst, tampapp))
                                                     RETURN(EADDRNOTAVAIL);

                         /* Mise a zero memoire image */
                         for (iz=0; iz<34; iz++)
                            { EI(zd + iz, 0, 0);

                              /*** ATTENTION ***   #########################
                                Sous Linux, il faudrait sans doute   #######
                                liberer ici la wait_queue eventuelle ... ###
                              ##############################################
                              TODO
                              */

                            }

                         /* RAZ du pointeur de zd */
                         EI(j + i, 0, 0);
                       }

                   }


                 /****************************/
                 /* Sous-adresse en emission */
                 /****************************/

                 /* La voie existe-t-elle ?                          */
                 /* L'image de la table des filtres doit etre testee */
                 /* au meme titre que le pointeur de tampons         */
                 tampapp = LI(k + i + 32, 0);
                 if (tampapp == 0) tampapp = L(j + i + 32);
                 if (tampapp)
                   { /* La voie existe !                            */
                     /* et tampapp pointe la chaine des tampons que */
                     /* la voie soit inhibee ou non.                */

                     /* Inhibition ITs */
                     E(k + i + 32, L(k + i + 32) & ~0x7A);

                     zd = LI(j + i + 32, 0);

                     /* Si adresse zd anormale, on abandonne ... */
                     if ((zd < DEBUT_RAM) || (zd >= FIN_RAM)) RETURN(EDOM);


                     /* Envoi signal RESET sur semaphore associe a la voie  */
                     /* (Reveil eventuelle tache en attente sur cette voie) */
                     // cprintf("   sreset RT%d,%d %c",
                     //              i, j % 2, (j>31) ? 'T' : 'C');
#ifdef LINUX
                     // cprintf("   IMA[0x%04X] = 0x%08X\n",
                     //                     zd + IRSEM, LI(zd + IRSEM, 6));
                     /* Si aucun SWAIT_IM n'a encore ete effectue  */
                     /* sur la voie, le pointeur de wait-queue est */
                     /* nul. Il faut alors ne pas l'utiliser !     */
                     /* (sous peine de segmentation fault !)       */
                     if (LI(zd + IRSEM, 6) != 0)
                       { SRESET_IM(&LI(zd + IRSEM, 6));
                       }
#else
                      // cprintf("\n");
                      SRESET_IM(&LI(zd + IRSEM, 6));
#endif   /* LINUX */


#ifdef CEVT
                     /* On n'utilise pas le mutex habituel pour proteger */
                     /* le code ci-dessous car il semble deraisonnable   */
                     /* qu'une tache tente de connecter une voie a un    */
                     /* CEVT pendant qu'une autre tache supprime cette   */
                     /* voie : un minimum de synchronisation doit etre   */
                     /* fourni par les applications elles-memes !        */

                     /* La voie est-elle connectee a un CEVT ? */
                     mm = LI(zd + IRCEVT, 0);
                     if (mm)
                       {
                         /* On signale, si possible, la deconnexion au CEVT */
                         cevt_signaler(mm, CEVT_SDCABOSUPPR,
                                                  dst->signal_number,
                                                  ((v->adresse << 11) & 0xF800)
                                                  | 0x400 | ((i << 5) & 0x3E0),
                                                  0);

                         /* Deconnexion de la voie */
                         EI(zd + IRCEVT, 0, 324);
                       }
#endif


                     /* Si la voie etait liee, le retrait du lien */
                     /* a ete fait au cours du traitement de la   */
                     /* voie en reception associee                */

                     /* Deconnexion voie courante de */
                     /* la chaine des tampons        */
                     EI(k + i + 32, 0, 0);
                     E(j + i + 32, 0);
                     usec_sleep(1);
                     E(j + i + 32, 0);


                     /* Pour une voie en emission, la structure des */
                     /* tampons depend du mode de la voie           */
                     switch (LI(zd + IRMODE, 0))
                        {
                          case RT_VSYNC :
                          case RT_VASYNC :
                             if (liberer_tamp(dst, tampapp))
                                                     RETURN(EADDRNOTAVAIL);
                             break;

                          case RT_VSTAT :
                             /* Tampons separes ? */
                             /*   --> Ils sont quand meme lies par IRTPREC */
                             if (liberer_tamp(dst, tampapp))
                                                     RETURN(EADDRNOTAVAIL);
                             break;

                          case RT_VSYNC2 :
                             if (liberer_tamp(dst, LI(zd + IRTCHINU, 0)))
                                                     RETURN(EADDRNOTAVAIL);
                             if (liberer_tamp(dst, LI(zd + IRTFCH1, 0)))
                                                     RETURN(EADDRNOTAVAIL);
                             if (liberer_tamp(dst, LI(zd + IRTFCH2, 0)))
                                                     RETURN(EADDRNOTAVAIL);
                             break;

                          default : RETURN(EBADF);
                        }


                     /* Remise a zero memoire image */
                     for (iz=0; iz<34; iz++)
                        { EI(zd + iz, 0, 0);

                              /*** ATTENTION ***
                                Sous Linux, il faudrait sans doute
                                liberer ici la wait_queue eventuelle ...
                              */

                        }

                     /* RAZ du pointeur de zd */
                     EI(j + i, 0, 0);

                   }
               }



          /* Liberation de la table des sous-adresses */
          if (asdclibere(dst, j, 64) == -1) RETURN(EADDRNOTAVAIL);
          E(L(ATPTR) + v->adresse, 0);

          /* On ne doit pas liberer la table des filtres !!! */
          /* (sert a l'espion sequentiel)                    */
          /* if (asdclibere(dst, k, 64) == -1) RETURN(EADDRNOTAVAIL); */
          /* E(L(FTPTR) + v->adresse, 0); */

          /* Mais, par securite, on remet a zero */
          /* la memoire image correspondante     */
          for (iz=0; iz<64; iz++) EI(k+iz, 0, 0);


          /* Fin normale de ASDC_ABO_LIBERER */
          RETURN(OK);
        }















#ifdef CEVT
     case ASDCEVT_ABO_AJOUTER :
        {
          int ad, sa, di;
          int flux;
          int zd;

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));


          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          v->adresse &= 0x1F;
          v->sous_adresse &= 0x1F;
          v->direction &= 1;


          /* La voie a connecter au CEVT existe-t-elle ? */

          /* On verifie que les structures de donnees liees */
          /* au RT existent bien                            */

          /* La table des sous_adresses existe-t-elle ? */
          k = L(L(ATPTR) + v->adresse);
          if (k == 0)
	    { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
	    }

          /* Zone de donnees en mem. image de la voie */
          m = k + v->sous_adresse + v->direction * 32;
	  zd = LI(m, 33);



          /* Des tampons de donnees existent-t-ils ? (sauf si mode sync 2) */
          if (LI(zd+IRMODE, 0) != RT_VSYNC2)
            { l = L(m);
              if (l == 0)
	        { RETURN(EADDRNOTAVAIL);   /* La voie n'a jamais ete declaree */
	        }
            }


	  else
	    {
              if (!cevt_existence(v->adrtamp))
	        { RETURN(EXDEV);   /* Le numero de CEVT est invalide */
	        }
	    }


	  /* Pour le moment, les flux evenements RT et les flux BC */
	  /* partagent le meme mutex ...                           */
          SEMWAIT(&dst->mutexflux, SEM_SIGIGNORE); /* Debut section critique */

	     /* La voie ne doit pas etre deja connectee a un CEVT */
             if (LI(zd + IRCEVT, 34))
	       { RETURN(EEXIST);   /* La voie est deja connectee a un CEVT */
	       }

             /* Connexion de la voie */
             EI(zd + IRCEVT, v->adrtamp, 323);	/* Numero CEVT */

          SEMSIGNAL(&dst->mutexflux);  	 /* Fin section critique */


          /* Validation des interruptions si message destine a la voie */
          { int ef;

            /* Pointeur table des filtres associee a adresse */
            ef = L(L(FTPTR) + v->adresse);

            /* Adresse du filtre */
            ef += v->sous_adresse + ( v->direction ? 32 : 0);

            /* Programmation du filtre : Validation IT voie */
            E(ef, L(ef) | 2);
          }

         /*
          *  REMARQUE : Les interruptions sur reception message sont validees
          *             en differents endroits (dont ci-dessus), mais ne sont
          *             sans doute pas toujours inhibees (par exemple, en cas
          *             de suppression d'un CEVT). [En effet, quand un CEVT
          *             est supprime, rien permet d'affirmer qu'une tache n'est
          *             pas alors en train d'effectuer une lecture bloquante
          *             sur la voie concernee ...].
          *		==> La strategie actuellement employee consiste a
          *                 considerer qu'il vaut mieux laisser une IT validee
          *                 (ce qui introduit seulement une perte de quelques
          *                 us de termps UC en cas d'arrivee du message associe
          *                 a la voie) plutot qu'inhiber une IT qui est
          *                 utilisee par ailleurs.
          *             ==> Cette strategie pourra toujours etre revue
          *                 ulterieurement, si le besoin s'en fait sentir ...
          *
          *					Y. Guillemot, le 5/6/2002
          */

          /* Fin normale de ASDCEVT_ABO_AJOUTER */
          RETURN(OK);
        }






     case ASDCEVT_ABO_SUPPRIMER :
        {
          int ad, sa, di;
          int flux;
          int zd;

          TDUTIL(arg, sizeof(struct asdcvoie));
          DUTIL(zv, v, arg, sizeof(struct asdcvoie));
          TVUTIL(zv, v, arg, sizeof(struct asdcvoie));


          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          v->adresse &= 0x1F;
          v->sous_adresse &= 0x1F;
          v->direction &= 1;


          /* La voie a deconnecter d'un CEVT existe-t-elle ? */

          /* On verifie que les structures de donnees liees */
          /* au RT existent bien                            */

          /* La table des sous_adresses existe-t-elle ? */
          k = L(L(ATPTR) + v->adresse);
          if (k == 0)
	    { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
	    }

          /* Zone de donnees en mem. image de la voie */
          m = k + v->sous_adresse + v->direction * 32;
	  zd = LI(m, 35);

          /* Des tampons de donnees existent-t-ils ? (sauf si mode sync 2) */
          if (LI(zd+IRMODE, 0) != RT_VSYNC2)
            { l = L(m);
              if (l == 0)
	        { RETURN(EADDRNOTAVAIL);   /* La voie n'a jamais ete declaree */
	        }
            }


	  /* Pour le moment, les flux evenements RT et les flux BC */
	  /* partagent le meme mutex ...                           */
          SEMWAIT(&dst->mutexflux, SEM_SIGIGNORE); /* Debut section critique */
	     /* La voie est-elle bien connectee a un CEVT ? */
             if (LI(zd + IRCEVT, 36) == 0)
	       { RETURN(EEXIST);   /* La voie n'est pas connectee ! */
	       }

             /* Deconnexion de la voie */
             EI(zd + IRCEVT, 0, 324);

          SEMSIGNAL(&dst->mutexflux);  	 /* Fin section critique */


          /* Pas d'invalidation des interruptions ...          */
          /* (cf. commentaire a la fin de ASDCEVT_ABO_AJOUTER) */

         /*
          *  REMARQUE : Les interruptions sur reception message sont validees
          *             en differents endroits (dont ci-dessus), mais ne sont
          *             sans doute pas toujours inhibees (par exemple, en cas
          *             de suppression d'un CEVT). [En effet, quand un CEVT
          *             est supprime, rien permet d'affirmer qu'une tache n'est
          *             pas alors en train d'effectuer une lecture bloquante
          *             sur la voie concernee ...].
          *		==> La strategie actuellement employee consiste a
          *                 considerer qu'il vaut mieux laisser une IT validee
          *                 (ce qui introduit seulement une perte de quelques
          *                 us de termps UC en cas d'arrivee du message associe
          *                 a la voie) plutot qu'inhiber une IT qui est
          *                 utilisee par ailleurs.
          *             ==> Cette strategie pourra toujours etre revue
          *                 ulterieurement, si le besoin s'en fait sentir ...
          *
          *					Y. Guillemot, le 5/6/2002
          */

          /* Fin normale de ASDCEVT_ABO_AJOUTER */
          RETURN(OK);
        }





     case ASDCEVT_CC_AJOUTER :
        {
          struct asdcvoiecc ze, *e;
          int code;
          int mim;
          unsigned short hmim, lmim;

          TDUTIL(arg, sizeof(struct asdcvoiecc));
          DUTIL(ze, e, arg, sizeof(struct asdcvoiecc));
          TVUTIL(ze, e, arg, sizeof(struct asdcvoiecc));


          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          e->adresse &= 0x1F;
          e->coco &= 0x41F;


          /* La voie a connecter au CEVT existe-t-elle ? */

          /* On verifie que les structures de donnees liees */
          /* au RT existent bien                            */

          /* La table des sous_adresses existe-t-elle ? */
          k = L(L(ATPTR) + e->adresse);
          if (k == 0)
	    { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
	    }


	  else
	    {
              if (!cevt_existence(e->cevt))
	        { RETURN(EXDEV);   /* Le numero de CEVT est invalide */
	        }
	    }


	  /* Code "de base" de la commande */
	  code = e->coco & 0x1F;

	  /* Commande existante ? */
	  if ((!(asdc_coco[code].def & 2)) || (code > COCO_MAX))
	    {
	      RETURN(ENOENT);	/* Non */
	    }



	  /* La voie ne doit pas etre deja connectee a un CEVT */
          if (dst->cocor[e->adresse][code].cevt)
	    { RETURN(EEXIST);   /* La voie est deja connectee a un CEVT */
	    }

          /* Connexion de la voie */
          dst->cocor[e->adresse][code].cevt = e->cevt;   /* Numero CEVT */




          /* Validation des interruptions si message destine a la voie */

          mim = L(MIMPTR);		/* Adresse de la table des MIM */

          mim += 2 * e->adresse;	/* Adresse du MIM de la voie */

          hmim = asdc_coco[code].hmim;
          lmim = asdc_coco[code].lmim;


	  /* Pour le moment, les flux evenements RT et les flux BC */
	  /* partagent le meme mutex ...                           */
          SEMWAIT(&dst->mutexflux, SEM_SIGIGNORE); /* Debut section critique */

            if (lmim) E(mim, L(mim) | lmim);
            if (hmim) E(mim + 1, L(mim + 1) | hmim);

          SEMSIGNAL(&dst->mutexflux);  	 /* Fin section critique */




         /*
          *  REMARQUE : Les interruptions sur reception CC sont validees
          *             en differents endroits (dont ci-dessus), mais ne sont
          *             sans doute pas toujours inhibees (par exemple, en cas
          *             de suppression d'un CEVT). [En effet, quand un CEVT
          *             est supprime, rien permet d'affirmer qu'une tache n'est
          *             pas alors en train d'effectuer une lecture bloquante
          *             sur la voie concernee ...].
          *		==> La strategie actuellement employee consiste a
          *                 considerer qu'il vaut mieux laisser une IT validee
          *                 (ce qui introduit seulement une perte de quelques
          *                 us de termps UC en cas d'arrivee du message associe
          *                 a la voie) plutot qu'inhiber une IT qui est
          *                 utilisee par ailleurs.
          *             ==> Cette strategie pourra toujours etre revue
          *                 ulterieurement, si le besoin s'en fait sentir ...
          *
          *					Y. Guillemot, le 5/6/2002
          */

          /* Fin normale de ASDCEVT_CC_AJOUTER */
          RETURN(OK);
        }







     case ASDCEVT_CC_SUPPRIMER :
        {
          struct asdcvoiecc ze, *e;
          int code;
          int mim;

          TDUTIL(arg, sizeof(struct asdcvoiecc));
          DUTIL(ze, e, arg, sizeof(struct asdcvoiecc));
          TVUTIL(ze, e, arg, sizeof(struct asdcvoiecc));


          /* Controle de la validite des parametres :                 */
          /*   - Pour le moment, pas de controle a proprement parler, */
          /*  on se contente de supprimer tous les bits inutilises    */
          e->adresse &= 0x1F;
          e->coco &= 0x41F;


          /* La voie a deconnecter d'un CEVT existe-t-elle ? */

          /* On verifie que les structures de donnees liees */
          /* au RT existent bien                            */

          /* La table des sous_adresses existe-t-elle ? */
          k = L(L(ATPTR) + e->adresse);
          if (k == 0)
	    { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
	    }




	  /* Code "de base" de la commande */
	  code = e->coco & 0x1F;

	  /* Commande existante ? */
	  if ((!(asdc_coco[code].def & 2)) || (code > COCO_MAX))
	    { RETURN(ENOENT);	/* Non */
	    }



	  /* La voie doit etre connectee a un CEVT */
          if (!dst->cocor[e->adresse][code].cevt)
	    { RETURN(EEXIST);   /* La voie n'est pas connectee a un CEVT */
	    }

          /* Deconnexion de la voie */
          dst->cocor[e->adresse][code].cevt = 0;



         /*
          *  REMARQUE : Les interruptions sur reception CC sont validees
          *             en differents endroits , mais ne sont
          *             sans doute pas toujours inhibees (par exemple, en cas
          *             de suppression d'un CEVT). [En effet, quand un CEVT
          *             est supprime, rien permet d'affirmer qu'une tache n'est
          *             pas alors en train d'effectuer une lecture bloquante
          *             sur la voie concernee ...].
          *		==> La strategie actuellement employee consiste a
          *                 considerer qu'il vaut mieux laisser une IT validee
          *                 (ce qui introduit seulement une perte de quelques
          *                 us de termps UC en cas d'arrivee du message associe
          *                 a la voie) plutot qu'inhiber une IT qui est
          *                 utilisee par ailleurs.
          *             ==> Cette strategie pourra toujours etre revue
          *                 ulterieurement, si le besoin s'en fait sentir ...
          *
          *					Y. Guillemot, le 5/6/2002
          */

          /* Fin normale de ASDCEVT_CC_SUPPRIMER */
          RETURN(OK);
        }
#endif	/* CEVT */





     case ASDCBCFLUX_CREER :
     case ASDCBCFLUX_AJOUTER :
        {
          struct asdcbcflux_aj *e, ze;
          int flux, bc;
          int zd;

          TDUTIL(arg, sizeof(struct asdcbcflux_aj));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_aj));

          flux = e->flux & 0xFFFF;
          bc = e->bc & 0xFFFF;

          SEMWAIT(&dst->mutexflux, SEM_SIGIGNORE);  /* Debut section critique */

	  /* En cas de creation d'un nouveau flux, les tampons */
	  /* necessaires sont-ils disponibles ?                */
	     /* nb_tamp_flux_dispos - 1 car reservation "tampon en attente" */
	  if (bc == flux)
	    { if ((e->nbte + e->nbts) > (dst->nb_tamp_flux_dispos - 1))
                {
	          /* Pas assez de tampons disponibles */

                  SEMSIGNAL(&dst->mutexflux);	  /* Fin section critique */
                  RETURN(ENOMEM);
                }
	    }



          /* Controles divers */

          i = L(bc);
          j = L(flux);

          switch(i)		/* Type du bloc BC correct ? */
             { case COCO :
               case BCRT :
               case RTBC :
               case RTRT :
               case BCRT_DI :
               case RTRT_DI : break;	/* OK */

               default : 		/* Bloc BC est anormal */
              	      SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              	      RETURN(EFAULT);
             }

          switch(j)		/* Type du bloc flux correct ? */
             { case COCO :
               case BCRT :
               case RTBC :
               case RTRT :
               case BCRT_DI :
               case RTRT_DI : break;	/* OK */

               default : 		/* Bloc BC est anormal */
              	      SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              	      RETURN(EFAULT);
             }

          zd = L(flux + 6); 		/* Zone donnees du bloc flux */

          if (LIL(bc+IMFLUX, 37))
            { /* Bloc BC deja lie a un flux ! */
              SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              RETURN(EEXIST);
            }

          if (i == j)
            {
               if (zd == 0)
                 { /* Bloc BC definissant flux doit avoir zone donnees */
              	      SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              	      RETURN(EFAULT);
                 }
            }
          else
            {
               if (LIL(flux+IMFLUX, 38) != (flux))
                 { /* Bloc FLUX ne correspond pas a un flux !!! */
              	      SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              	      RETURN(EXDEV);
                 }
            }



	  /* Marquage du lien vers le flux */
	  EIL(bc + IMFLUX, flux, 325);

	  /* Est-ce une creation de flux ou l'ajout d'un element ? */
	  if (bc != flux)
	    { /* Ajout d'un element */

	      /* Chainage */
	      EIL(LIL(flux + IMFSD, 39) + IMSUIV, bc, 326);
	      EIL(bc + IMSUIV, 0, 329);
	      EIL(flux + IMFSD, bc, 330);  /* Dernier element flux */
	    }
	  else
	    { /* Creation d'un flux */

	      EIL(flux + IMFSD, bc, 340);  /* Dernier element flux */
	      EIL(flux + IMSUIV, 0, 341);
	      EI(flux + IMPZD, zd, 342);  /* Correspond a zone donnees */
	      EI(flux + IMSEMFLX, 0, 343);	 /* Semaphore */

	      /* Chainage des flux */
              DISABLE(s);	/* Section critique */
	         if (dst->pflux) EIH(dst->dflux + IMFSD, flux, 344);
	                    else dst->pflux = flux;
	         EIH(flux + IMFSD, 0, 345);
                 dst->dflux = flux;
              RESTORE(s);	/* Fin section critique */


	      /* Mise a jour des compteurs et allocations des tampons */
              dst->nb_tamp_flux_dispos -= e->nbte + e->nbts;
	      EI(zd + IMNBMTE, e->nbte, 346);
	      EI(zd + IMNBCTE, 0, 347);
	      EI(zd + IMNBLTE, -1, 348);
	      EI(zd + IMPPTE, 0, 349);
	      EI(zd + IMPDTE, 0, 350);
	      EI(zd + IMNBMTS, e->nbts, 351);
	      EI(zd + IMNBCTS, 0, 352);
	      EI(zd + IMNBLTS, -1, 353);
	      EI(zd + IMPPTS, 0, 354);
	      EI(zd + IMPDTS, 0, 355);
	      EI(zd + IMNBERR, 0, 356);
	    }

	  /* RAZ du compteur d'execution du bloc BC */
	  EI(bc + IMCPTR, 0, 357);

          /* Validation de l'interruption en fin d'execution du bloc BC */
          E(bc + 5, L(bc + 5) | FBC_IT);

          SEMSIGNAL(&dst->mutexflux);  	 /* Fin section critique */
          RETURN(OK);
        }



     case ASDCBCFLUX_SUPPRIMER :
        {
          struct asdcbcflux_aj *e, ze;
          int flux, bc;
          int zd;

          TDUTIL(arg, sizeof(struct asdcbcflux_aj));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_aj));

          flux = e->flux & 0xFFFF;

          SEMWAIT(&dst->mutexflux, SEM_SIGIGNORE);  /* Debut section critique */

          /* Controles divers */

          zd = L(flux + 6); 		/* Zone donnees du bloc flux */

          if (zd != LI(flux + IMPZD, 40))
            { /* Incoh�rence entre MEM et IMAGE ! */
              SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              RETURN(EXDEV);
            }

          if (LIL(flux + IMFLUX, 41) != (flux))
            { /* Bloc FLUX ne correspond pas a un flux !!! */
              SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
              RETURN(EXDEV);
            }

            /***********************************************************/
            /* ATTENTION : En cas d'incoherence malheureuse de la      */
            /* memoire image, on pourrait boucler indefiniment dans    */
            /* certaines des boucles "for" ci-dessous.                 */
            /* L'utilisation de la variable j dans ces boucles est     */
            /* destinee a eliminer ce risque.                          */
            /*                                                         */
            /* Les valeurs limites 5000 et 1000 ont ete prises un peu  */
            /* au pif en supposant :                                   */
            /*         - moins de 5000 blocs BC dans un flux           */
            /*         - moins de 1000 fluxs definis simultanement     */
            /***********************************************************/

          /* Inhibition des interruptions li�es au flux et du flux lui-meme */
          /* et retrait du flux de la cha�ne des flux                       */
          DISABLE(s);	/* Section critique */

            for (bc=flux, j=0; bc && (j<5000); bc=LIL(bc+IMSUIV, 42), j++)
               { E(bc + 5, L(bc + 5) & ~FBC_IT);   /* Inhib. IT */
                 EIL(bc + IMFLUX, 0, 358);		/* Masquage flux */
               }

            if (j>=5000)
              { /* Incoh�rence du chainage et des entrees */
                RESTORE(s);	/* Fin section critique */
                SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
                RETURN(EXDEV);
              }


            /* Recherche du flux precedent le flux a supprimer */
            for (i=0, j=0, bc=dst->pflux;
                 bc && (bc != flux) && (j<1000);
                 bc = LIH(bc + IMFSD, 43))
               { i = bc;
                 j++;
               }

            if ((bc == 0) || (j >= 1000))
              { /* Incoh�rence du chainage et des entrees */
                RESTORE(s);	/* Fin section critique */
                SEMSIGNAL(&dst->mutexflux);   /* Fin section critique */
                RETURN(EXDEV);
              }

            if (i==0) dst->pflux = LIH(flux + IMFSD, 44);
                 else EIH(i + IMFSD, LIH(flux + IMFSD, 45), 359);
            if (LIH(flux + IMFSD, 46) == 0) dst->dflux = i;

            /* Liberation des tampons */
            i = asdc_fluxbctamp_supprimer(
                     (struct asdcbc_tfch **) &LI(zd+IMPPTE, 47),
                     (struct asdcbc_tfch **) &LI(zd+IMPDTE, 48), &dst->pbcfl);
            j = asdc_fluxbctamp_supprimer(
                        (struct asdcbc_tfch **) &LI(zd+IMPPTS, 49),
                        (struct asdcbc_tfch **) &LI(zd+IMPDTS, 50),
                        &dst->pbcfl);
            dst->nb_tamp_flux_dispos += LI(zd+IMNBMTS, 51) + LI(zd+IMNBMTE, 52);

          RESTORE(s);	/* Fin section critique */

          /* Effacement de l'image du BC "flux" (utile ???) */
	  EI(zd + IMNBMTE, 0, 360);
	  EI(zd + IMNBCTE, 0, 361);
	  EI(zd + IMNBLTE, 0, 362);
	  EI(zd + IMPPTE, 0, 363);
	  EI(zd + IMPDTE, 0, 364);
	  EI(zd + IMNBMTS, 0, 365);
	  EI(zd + IMNBCTS, 0, 366);
	  EI(zd + IMNBLTS, 0, 367);
	  EI(zd + IMPPTS, 0, 368);
	  EI(zd + IMPDTS, 0, 369);
          EI(flux + IMPZD, 0, 370);
#ifdef LINUX
          { struct wq_liste *pwq = (struct wq_liste *) LI(flux + IMSEMFLX, 0);
            if (pwq) wq_rendre(dst, pwq);
          }
#endif /* LINUX */
          EI(flux + IMSEMFLX, 0, 371);
          EIL(flux + IMSUIV, 0, 372);

          	/* EIL(flux + IMFSD, 0, 373); */
          	/* EIH(flux + IMFSD, 0, 374); */
          EI(flux + IMFSD, 0, 373);

          SEMSIGNAL(&dst->mutexflux);  	 /* Fin section critique */
          RETURN(OK);
        }



     case ASDCBCFLUX_ETAT :
        {
          struct asdcbcflux_etat *e, ze;
          int flux;
          int zd;

          TDUTIL(arg, sizeof(struct asdcbcflux_etat));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));

          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 53);

	  e->nbmte = LI(zd + IMNBMTE, 54);  /* Nbre max. tampons en entree */
	  e->nbmts = LI(zd + IMNBMTS, 55);  /* Nbre max. tampons en sortie */
	  e->nbcte = LI(zd + IMNBCTE, 56);  /* Nbre courant tampons en entree */
	  e->nbcts = LI(zd + IMNBCTS, 57);  /* Nbre courant tampons en sortie */
	  e->nblte = LI(zd + IMNBLTE, 58);  /* Nbre limite tampons en entree */
	  e->nblts = LI(zd + IMNBLTS, 59);  /* Nbre limite tampons en sortie */

          e->evt = LI(zd + IMEVT, 60);     /* Evenements traites par le flux */
                                           /* (FLX_xxx)                      */

          VUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          RETURN(OK);
        }





     case ASDCBCFLUX_REGLER :
        {
          struct asdcbcflux_etat *e, ze;
          int flux;
          int zd;

          TDUTIL(arg, sizeof(struct asdcbcflux_etat));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));

          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 61);

          /* Programmation */
	  LI(zd + IMNBLTE, 62) = e->nblte ;  /* Nbre limite tampons en entree */
	  LI(zd + IMNBLTS, 63) = e->nblts ;  /* Nbre limite tampons en sortie */
          LI(zd + IMEVT, 64)
                   = e->evt  & 0xFFFF00FF; /* Evenements traites par le flux */
                                           /* (FLX_xxx)                      */

          /* Relecture etat */
	  e->nbmte = LI(zd + IMNBMTE, 65);  /* Nbre max. tampons en entree */
	  e->nbmts = LI(zd + IMNBMTS, 66);  /* Nbre max. tampons en sortie */
	  e->nbcte = LI(zd + IMNBCTE, 67);  /* Nbre courant tampons en entree */
	  e->nbcts = LI(zd + IMNBCTS, 68);  /* Nbre courant tampons en sortie */
	  e->nblte = LI(zd + IMNBLTE, 69);  /* Nbre limite tampons en entree */
	  e->nblts = LI(zd + IMNBLTS, 70);  /* Nbre limite tampons en sortie */

          e->evt = LI(zd + IMEVT, 71);     /* Evenements traites par le flux */
                                           /* (FLX_xxx)                      */

          VUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          RETURN(OK);
        }





     case ASDCBCFLUX_ATTENDRE :
        {
          struct asdcbcflux_etat *e, ze;
          int flux;
          int zd, etat;
          int nbcte, nbcts, nblte, nblts, evt;

          TDUTIL(arg, sizeof(struct asdcbcflux_etat));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));

          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 72);

          DISABLE(s);	/* Debut section critique */

          /* "Etat" du flux */
          etat = 0;
             /* Trame en cours ? */
          if (dst->fin_bc || ((!L(BCCPTR)) && (L(BCLPTR)))) etat |= FLX_FTRAME;
             /* Erreur interne au flux ? */
          if (LI(zd + IMNBERR, 73)) etat |= FLX_ERRF;

          /* Lecture de l'etat courant */
	  nbcte = LI(zd + IMNBCTE, 74);   /* Nbre courant tampons en entree */
	  nbcts = LI(zd + IMNBCTS, 75);   /* Nbre courant tampons en sortie */
	  nblte = LI(zd + IMNBLTE, 76);   /* Nbre limite tampons en entree */
	  nblts = LI(zd + IMNBLTS, 77);   /* Nbre limite tampons en sortie */
          evt = LI(zd + IMEVT, 78);     /* Evenements attendus */

          /* Evenement arrive ? */
	  if (nbcte > nblte) etat |= FLX_LIME;
	  if (nbcts < nblts) etat |= FLX_LIMS;
	  /* Evenement arrive etait attendu ? */
          if ((etat >> 8) & 0xFF & (evt | (FLX_FTRAME >> 8)))
            {
              /* Oui ==> Preparation des donnee en sortie et fin */
              RESTORE(s);	/* Fin section critique */

	      e->nbmte = LI(zd + IMNBMTE, 79); /* Nbre max. tampons en entree */
	      e->nbmts = LI(zd + IMNBMTS, 80); /* Nbre max. tampons en sortie */
	      e->nbcte = nbcte;         /* Nbre courant tampons en entree */
	      e->nbcts = nbcts;         /* Nbre courant tampons en sortie */
	      e->nblte = nblte;         /* Nbre limite tampons en entree */
	      e->nblts = nblts;         /* Nbre limite tampons en sortie */
              e->evt = evt | etat;      /* Evenements */

	      VUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
              RETURN(OK);
            }

          /* Evenement attendu non encore arrive ==> attente */

          if ((++dst->nombre_it)==1)  { /* Demasquage IT */ }

          tmpi = SWAIT_IM(&LI(flux + IMSEMFLX, 81), SEM_SIGABORT);

          if (!(--dst->nombre_it))  { /* Masquage IT */ }

          /* Lecture de l'etat courant */
	  nbcte = LI(zd + IMNBCTE, 82);   /* Nbre courant tampons en entree */
	  nbcts = LI(zd + IMNBCTS, 83);   /* Nbre courant tampons en sortie */
          evt = LI(zd + IMEVT, 84);     /* Evenements attendus */
          etat = dst->fin_bc ? FLX_FTRAME : 0;	/* Trame achevee ? */

          RESTORE(s);	/* Fin section critique */

	  e->nbmte = LI(zd + IMNBMTE, 85);   /* Nbre max. tampons en entree */
	  e->nbmts = LI(zd + IMNBMTS, 86);   /* Nbre max. tampons en sortie */
	  e->nbcte = nbcte;         /* Nbre courant tampons en entree */
	  e->nbcts = nbcts;         /* Nbre courant tampons en sortie */
	  e->nblte = nblte;         /* Nbre limite tampons en entree */
	  e->nblts = nblts;         /* Nbre limite tampons en sortie */

          /* Evenement arrive ? */
          if (LI(zd + IMNBERR, 87)) etat |= FLX_ERRF;
	  if (nbcte > nblte) etat |= FLX_LIME;
	  if (nbcts < nblts) etat |= FLX_LIMS;

          e->evt = (evt & 0xFF) | etat;             /* Evenements */

          VUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));

          if (tmpi)	/* Le passage du semaphore est-il du a un signal ? */
            { RETURN(EINTR);
            }

	  if (dst->raz) /* Le passage du semaphore est-il du a une RAZ ? */
	    { RETURN(ENETRESET);
	    }

          RETURN(OK);
        }


     case ASDCBCFLUX_LIRE :
        {
          struct asdcbcflux *e, ze;
          struct asdcbc_tf *z, *q;
          struct asdcbc_tfch *p;
          int flux;
          int zd;
          int nbt, nbcte;
          int i, j, n, noctets, pb;

          TDUTIL(arg, sizeof(struct asdcbcflux));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux));

          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 88);

          /* Relecture etat */
	  nbcte = LI(zd + IMNBCTE, 89);   /* Nbre courant tampons en entree */

          /* Nombre de tampons a transferer */
          nbt = (e->nbtt < nbcte) ? e->nbtt : nbcte;

	  /* Pointe la zone destination en memoire utilisateur */
          z = e->z;

	  /* Indicateur d'erreur de transfert (pour Linux surtout) */
	  pb = 0;

#ifdef LYNXOS
          /* Acces memoire possible ? */
          if (wbounds((long) z) < (nbt * sizeof(struct asdcbc_tf)))
            { RETURN(EFAULT);
            }

          /* Transfert de nbt tampons */
          for (i=0; i<nbt; i++)
             { /* Retrait du premier tampon de la chaine */
               DISABLE(s);	/* Debut section critique */

                  /* Necessite de relire nbcte dans memoire image    */
                  /* car lecture precedente etait faite hors section */
                  /* critique et une modification par fonction IT    */
                  /* a pu etre effectuee depuis ...                  */
	          nbcte = LI(zd + IMNBCTE, 90);

                  p = (struct asdcbc_tfch *) LI(zd + IMPPTE, 91);
                  (struct asdcbc_tfch *) LI(zd + IMPPTE, 92) = p->s;

	          nbcte --;
                  EI(zd + IMNBCTE, nbcte, 375);

                  /* Si plus de tampons, pointeur dernier tampon = NULL */
                  if (nbcte == 0) EI(zd + IMPDTE, NULL, 376);

               RESTORE(s);	/* Fin section critique */

               /* Copie du tampon */
               q = &p->tamp;
               z[i].type = q->type;
               z[i].err = q->err;
               z[i].cmd1 = q->cmd1;
               z[i].cmd2 = q->cmd2;
               z[i].sts1 = q->sts1;
               z[i].sts2 = q->sts2;
               z[i].date[0] = q->date[0];
               z[i].date[1] = q->date[1];
               z[i].date[2] = q->date[2];
               z[i].date[3] = q->date[3];
               n = q->cmd1 & 0x1F;
               if (!n) n = 32;
               for (j=0; j<n; j++)
                  { z[i].d[j] = q->d[j];
                  }

               /* Retour du tampon lu a la liste des tampons vides */
               DISABLE(s);	/* Debut section critique */
                  p->s = dst->pbcfl;
                  dst->pbcfl = p;
               RESTORE(s);	/* Fin section critique */
             }
#endif 	/* LYNXOS */

#ifdef LINUX
          /* Transfert de nbt tampons */
          for (i=0; i<nbt; i++)
             { /* Retrait du premier tampon de la chaine */
               DISABLE(s);	/* Debut section critique */

                  /* Necessite de relire nbcte dans memoire image    */
                  /* car lecture precedente etait faite hors section */
                  /* critique et une modification par fonction IT    */
                  /* a pu etre effectuee depuis ...                  */
	          nbcte = LI(zd + IMNBCTE, 90);

                  p = (struct asdcbc_tfch *) LI(zd + IMPPTE, 91);
                  LI(zd + IMPPTE, 92) = (long) p->s;

	          nbcte --;
                  EI(zd + IMNBCTE, nbcte, 375);

                  /* Si plus de tampons, pointeur dernier tampon = NULL */
                  if (nbcte == 0) EI(zd + IMPDTE, (long) NULL, 376);

               RESTORE(s);	/* Fin section critique */

               /* Copie du tampon */

               /* Calcul du nombre d'octets a copier :                   */
               /* ATTENTION : ce calcul est directement lie a la def. de */
               /* la structure asdcbc_tf. Si cette def. est modifiee, le */
               /* code suivant devra aussi etre modifie !!!              */

               q = &p->tamp;

               n = q->cmd1 & 0x1F;
               if (!n) n = 32;		/* Nombre de donnees 1553 */

	       /* Nombre d'octets a copier */
               noctets = sizeof(struct asdcbc_tf) - 2 * (32 - n);

               pb += copy_to_user(&z[i], (void *) q, noctets);

               /* Retour du tampon lu a la liste des tampons vides */
               DISABLE(s);	/* Debut section critique */
                  p->s = dst->pbcfl;
                  dst->pbcfl = p;
               RESTORE(s);	/* Fin section critique */
             }
#endif 	/* LINUX */

          /* Renvoi du nombre de tampons effectivement lus */
          e->nbtet = nbt;

          VUTIL(ze, e, arg, sizeof(struct asdcbcflux));
          if (pb) RETURN(EFAULT);
            else  RETURN(OK);
        }




     case ASDCBCFLUX_ECRIRE :
        {
          struct asdcbcflux *e, ze;
          struct asdcbc_tf *z, *q;
          struct asdcbc_tfch *p;
          int flux;
          int zd;
          int nbt, nbcts, nbmts;
          int i, j, n, noctets, pb;

          TDUTIL(arg, sizeof(struct asdcbcflux));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux));

	  e = (struct asdcbcflux *) arg;
          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 93);

          /* Relecture etat */
	  nbmts = LI(zd + IMNBMTS, 94);   /* Nbre max tampons en sortie */
	  nbcts = LI(zd + IMNBCTS, 95);   /* Nbre courant tampons en sortie */

          /* Nombre de tampons a transferer */
          nbt = (e->nbtt < nbmts - nbcts) ? e->nbtt : nbmts - nbcts;

	  /* Pointe la zone destination en memoire utilisateur */
          z = e->z;

	  /* Indicateur d'erreur de transfert (pour Linux surtout) */
	  pb = 0;

#ifdef LYNXOS
          /* Acces memoire possible ? */
          if (rbounds((long) z) < (nbt * sizeof(struct asdcbc_tf)))
            { RETURN(EFAULT);
            }

          /* Transfert de nbt tampons */
          for (i=0; i<nbt; i++)
             { /* Saisie d'un tampon disponible */
               DISABLE(s);	/* Debut section critique */
                  p = dst->pbcfl;
                  dst->pbcfl = p->s;
               RESTORE(s);	/* Fin section critique */

               /* Copie du tampon */
               q = &p->tamp;
               q->type = z[i].type;
               q->err = z[i].err;
               q->cmd1 = z[i].cmd1;
               q->cmd2 = z[i].cmd2;
               q->sts1 = z[i].sts1;
               q->sts2 = z[i].sts2;
               n = q->cmd1 & 0x1F;
               if (!n) n = 32;
               for (j=0; j<n; j++)
                  { q->d[j] = z[i].d[j];
                  }

               /* Accrochage du tampon ecrit � la liste des tampons du flux */
               DISABLE(s);	/* Debut section critique */
                   p->s = 0;

                  /* Necessite de relire nbcts dans memoire image    */
                  /* car lecture precedente etait faite hors section */
                  /* critique et une modification par fonction IT    */
                  /* a pu etre effectuee depuis ...                  */
                  nbcts = LI(zd + IMNBCTS, 96);

                  /* Si premier tampon, initialiser les pointeurs */
                  /* vers le debut et la fin de la FIFO           */
                  if (nbcts == 0)
                    { EI(zd+IMPPTS, (long) p, 377);
                    }
                  else
                    { ((struct asdcbc_tfch *) LI(zd + IMPDTS, 97))->s = p;
                    }

                  EI(zd+IMPDTS, (long) p, 378);

                  nbcts ++;
                  EI(zd + IMNBCTS, nbcts, 379);
               RESTORE(s);	/* Fin section critique */
             }
#endif 	/* LYNXOS */

#ifdef LINUX

          /* Transfert de nbt tampons */
          for (i=0; i<nbt; i++)
             { /* Saisie d'un tampon disponible */
               DISABLE(s);	/* Debut section critique */
                  p = dst->pbcfl;
                  dst->pbcfl = p->s;
               RESTORE(s);	/* Fin section critique */

               /* ATTENTION : le code ci-dessous est directement lie a  */
               /* la def. de la structure asdcbc_tf. Si cette def. est  */
               /* modifiee, ce code devra aussi etre modifie !!!        */

               q = &p->tamp;

               /* Taille de la structure sans les donnees */
               noctets = sizeof(struct asdcbc_tf) - 2 * 32;

               /* Transfert du debut du tampon */
               pb += copy_from_user(q->d, z[i].d, noctets);

               n = q->cmd1 & 0x1F;
               if (!n) n = 32;		/* Nombre de donnees 1553 */

	       /* Transfert des donnees restantes */
               pb += copy_from_user((char *)q + noctets, &z[i], 2 * n);


               /* Accrochage du tampon ecrit � la liste des tampons du flux */
               DISABLE(s);	/* Debut section critique */
                   p->s = 0;

                  /* Necessite de relire nbcts dans memoire image    */
                  /* car lecture precedente etait faite hors section */
                  /* critique et une modification par fonction IT    */
                  /* a pu etre effectuee depuis ...                  */
                  nbcts = LI(zd + IMNBCTS, 96);

                  /* Si premier tampon, initialiser les pointeurs */
                  /* vers le debut et la fin de la FIFO           */
                  if (nbcts == 0)
                    { EI(zd+IMPPTS, (long) p, 377);
                    }
                  else
                    { ((struct asdcbc_tfch *) LI(zd + IMPDTS, 97))->s = p;
                    }

                  EI(zd+IMPDTS, (long) p, 378);

                  nbcts ++;
                  EI(zd + IMNBCTS, nbcts, 379);
               RESTORE(s);	/* Fin section critique */
             }
#endif 	/* LINUX */

          /* Renvoi du nombre de tampons effectivement ecrits */
          e->nbtet = nbt;

	  VUTIL(ze, e, arg, sizeof(struct asdcbcflux));
	  RETURN(OK);
        }


#ifdef EN_COURS_DE_CREATION

     case ASDCBCFLUX_RAZ :

#endif /* EN_COURS_DE_CREATION */






     case ASDCBCFLUX_ETAT_COMPLET :
        {
          /* Pour debug : idem ASDCBCFLUX_ETAT + renvoi des pointeurs */
          /* (contexte noyau) vers les tampons des FIFOs              */

          struct asdcbcflux_etat *e, ze;
          int flux;
          int zd;

          TDUTIL(arg, sizeof(struct asdcbcflux_etat));
          DUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          TVUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));

          flux = e->flux & 0xFFFF;
          zd = LI(flux + IMPZD, 98);

	  e->nbmte = LI(zd + IMNBMTE, 99);  /* Nbre max. tampons en entree */
	  e->nbmts = LI(zd + IMNBMTS, 100); /* Nbre max. tampons en sortie */
	  e->nbcte = LI(zd + IMNBCTE, 101); /* Nbre courant tampons en entree */
	  e->nbcts = LI(zd + IMNBCTS, 102); /* Nbre courant tampons en sortie */
	  e->nblte = LI(zd + IMNBLTE, 103); /* Nbre limite tampons en entree */
	  e->nblts = LI(zd + IMNBLTS, 104); /* Nbre limite tampons en sortie */

          e->evt = LI(zd + IMEVT, 105);   /* Evenements traites par le flux */
                                          /* (FLX_xxx)                      */

          e->ppte = LI(zd + IMPPTE, 106);  /* Pointeur premier tampon E */
          e->pdte = LI(zd + IMPDTE, 107);  /* Pointeur dernier tampon E */
          e->ppts = LI(zd + IMPPTS, 108);  /* Pointeur premier tampon S */
          e->pdts = LI(zd + IMPDTS, 109);  /* Pointeur dernier tampon S */

          VUTIL(ze, e, arg, sizeof(struct asdcbcflux_etat));
          RETURN(OK);
        }





     case ASDCBCFLUX_LTAMPON :
        {
          /* Pour debug : lecture d'un tampon d'une FIFO designe par */
          /* son adresse dans le contexte du noyau                   */

          struct asdcbc_tfch *e, ze, *p;

          TDUTIL(arg, sizeof(struct asdcbc_tfch));
          DUTIL(ze, e, arg, sizeof(struct asdcbc_tfch));
          TVUTIL(ze, e, arg, sizeof(struct asdcbc_tfch));

          p = e->s;

          /* Copie du tampon */
          *e = *p;

          VUTIL(ze, e, arg, sizeof(struct asdcbc_tfch));
          RETURN(OK);
        }





       /*** Fonctions de debug ******************************************/



       case ASDCTRACE :  /* Accumulation des valeurs successives */
                         /* de differents registres              */

          {
            int i, j, val0;
            struct asdctrace *c, zc;
            int nba;

            TDUTIL(arg, sizeof(struct asdctrace));
            DUTIL(zc, c, arg, sizeof(struct asdctrace));
            TVUTIL(zc, c, arg, sizeof(struct asdctrace));

            nba = c->nba;
            if (nba > 8) nba = 8;

            DISABLE(s);

            if (c->mdecl)
              { /* Attente du declencheur */
                c->videcl = val0 = L(c->adecl);
                for (;;)
                  { if (L(c->adecl) !=  val0) break;
                    /* Comment rendre la main au systeme ? */
                    /*    ==> Simplement en tournant sous  */
                    /*        une priorite faible !        */
                  }
              }
            else
              { /* Declenchement immediat */
              }

            /* Memorisation des traces */
            for (i=0; i<1000; i++)
                {
                  for (j=0; j<nba; j++)
                     { dst->trace[j][i] = L(c->a[j]);
                     }
                  // usec_sleep(1);
                }

            RESTORE(s);


            /* Renvoi des traces a l'utilisateur */
#ifndef LINUX
            /* Cas LynxOS */
            for (j=0; j<nba; j++)
               { if (wbounds((unsigned long)c->tr[j]) < (1000 * sizeof(short)))
                   { kkprintf("ASDCTRACE : Probleme wbounds(tr[%d]) !\n", j);
                     RETURN(EFAULT);
                   }
               }

            for (i=0; i<1000; i++)
               for (j=0; j<nba; j++)
                  {
                    ((short *)(c->tr[j]))[i] = dst->trace[j][i];
                  }
#else
            /* Cas Linux */
            /* non traite pour le moment :( */
#endif  /* LINUX */

            VUTIL(zc, c, arg, sizeof(struct asdctrace));
            RETURN(OK);		/* EINVAL si erreur, OK sinon */
          }








       case ASDCLMSG :    /* Relecture par une appli du tampon des messages  */
                          /* (debug) ecrits par le driver (fonction asdcmsg) */
                          /* [Pour le moment, LynxOS uniquement]             */

          {
            int i, j, val0;
            struct asdcmsg *c, zc;

            TDUTIL(arg, sizeof(struct asdcmsg));
            DUTIL(zc, c, arg, sizeof(struct asdcmsg));

#ifndef LINUX
            /* Transfert des donnees sous LynxOS */
            if (wbounds((long) c->ch) < TTMSG)
              {
                kkprintf("asdcmsg : pb wbounds\n");
                RETURN(EFAULT);
              }

            // kkprintf("=======        ASDCLMSG        =======\n");

            for (i=0; i<TTMSG; i++)
               {
                 c->ch[i] = dst->msg.ch[i];
               }
#else
            /* Transfert des donnees sous Linux */
            if (copy_to_user(c->ch, dst->msg.ch, TTMSG)) RETURN(EFAULT);
#endif  /* LINUX */

            RETURN(OK);        /* EINVAL si erreur, OK sinon */
         }

       /*** Fin des fonctions de debug *************************************/




     default:
        /* Commande ioctl inexistante */
        RETURN(EINVAL);
   }






   /* Erreur qui ne peut pas arriver !!! */
   cprintf("asdcioctl : Ce message n'aurait jamais du etre ecrit !\n");
   RETURN(ENOTTY);
}

