/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                                         Y.Anonymized, le 23/01/1991   *
 *                                               modif. le 29/01/1991   *
 *                                               modif. le 14/03/1991   *
 *                                      derniere modif. le 18/03/1991   *
 *                         Adaptation a microcode SDC : le 31/03/1992   *
 *                                               modif. le  6/04/1992   *
 *                                               modif. le  6/10/1992   *
 *                                      derniere modif. le 28/10/1992   *
 *     Adaptation a utilisation du mode espion : modif. le 15/12/1993   *
 *                                               modif. le 13/04/1994   *
 *                                               modif. le 20/05/1994   *
 *                                               modif. le  8/08/1994   *
 *                                      derniere modif. le 30/09/1994   *
 *                                               v3.3 : le 13/04/1995   *
 *                                               v3.4 : le  1/12/1995   *
 *                                      derniere modif. le 19/12/1995   *
 *                                               v3.5 : le 11/01/1996   *
 *                                               modif. le 19/01/1996   *
 *                                                                      *
 *                  Adaptation a LynxOS (v4.0) : modif. le  6/04/2001   *
 *                                               modif. le 14/05/2001   *
 *                                               modif. le  5/06/2001   *
 *                                               modif. le   3/8/2001   *
 *                                               modif. le 23/10/2001   *
 *      Ajout du reveil des attentes sur fluxs BC                       *
 *      quand fin trame                                 le  5/11/2001   *
 *                          Traitement des erreurs flux le  9/11/2001   *
 *                            Ajout des reveils sur RAZ le  9/11/2001   *
 *            Ajout detection debordement table des ITs le 16/11/2001   *
 *           Ajout (gerant) des blocs RTRT et diffusion le 27/11/2001   *
 *      Correction fluxs BC pour IT 0x5 suivant IT 0x11 le 28/11/2001   *
 *      Correction oubli initialisation de dst->attente le 12/12/2001   *
 *                         Ajout des flux evenements RT le  4/06/2002   *
 *         Ajout variables de suivi pour debug si crash le 15/07/2002   *
 *      Ajout memorisation adr. phys. (bpba) pour debug le 28/08/2002   *
 *                                                                      *
 * Suppression de la gestion des flux d'evenements RT : le 10/09/2002   *
 *  Connexion des evenements rt au pseudo-driver CEVT : le 10/09/2002   *
 *                   Suppression des vieux codes CMBA : le 24/09/2002   *
 * Passage en "unsigned" de certains entiers utilises                   *
 *           comme pointeurs en mem. echange et image : le 24/09/2002   *
 *                   Marquage des LI et EI pour debug : le 24/09/2002   *
 *    Correction pointeur zone erreur dans bcflux_err : le 24/09/2002   *
 *                                               v4.6 : le 24/09/2002   *
 *      Modifications pour compatibilite LynxOS/Linux : le 30/09/2002   *
 *     Connexion des commandes codees (abo) a un CEVT : le  4/10/2002   *
 * Mise en synchronisme acces bridge Texas et acces                     *
 *        coupleurs 1553 en utilisant "signal_number" : le 16/10/2002   *
 *                                                                      *
 * Regroupement des 2 coupleurs d'une meme carte en un                  *
 *                                 seul device (v4.7) : le 18/10/20002  *
 *                                      derniere modif. le              *
 *                                                                      *
 *   Fonction d'attente execution d'un bloc BC (v4.8) : le 13/11/2002   *
 *      Mise en commentaire des marqueurs pour VMETRO : le 15/11/2002   *
 *                                                                      *
 *                  Source commun LynxOS/Linux (v4.9) : le 21/01/2003   *
 *                          Ajout indicateur "vflash" : le 23/01/2003   *
 *                                                                      *
 *   Ajout mode "synchrone 2" a simulation RT (v4.12) : le  8/06/2004   *
 *            Ajout fonction d'enchainement de trames : le  3/08/2004   *
 *                                                                      *
 *                     Adaptation au kernel Linux 2.6.x le 22/10/2004   *
 *                                                                      *
 *   Suppression declaration de license "GPL" (v4.14) : le  3/06/2005   *
 *                                                                      *
 * Modifications en raison de la reecriture                             *
 *        de l'I/F avec les wait queues Linux (v4.16) : le  8/09/2005   *
 * Modif. ASDC_ABO_MF                                                   *
 *                (generation d'erreurs transitoires) : le 13/09/2005   *
 *                                                                      *
 *                           Adaptation a LynxOS v4.0 : le 18/09/2006   *
 *      Datation evenements CEVT par TSC (sous Linux) : le 25/10/2006   *
 *                                         ASDC v4.17 : le 25/10/2006   *
 *                                                                      *
 * Retrait des macros MODULE_PARM depreciees dans                       *
 *        Linux 2.18 et remplacement par module_param : le  9/10/2007   *
 *                                         ASDC v4.19 : le  9/10/2007   *
 *                                                                      *
 * Correction de l'appel des macros module_param_array :                *
 *                                                      le 28/11/2007   *
 *      Traitement des erreurs transitoires multiples : le 28/11/2007   *
 *                        Correction du "bug SIVOL-Q" : le 15/02/2008   *
 *                                         ASDC v4.20 : le 15/02/2008   *
 *                                                                      *
 *          Ajout compatibilite Linux 64 bits (debut) : le 14/03/2008   *
 *                  Suite ajout compatibilite 64 bits : le  9/01/2008   *
 *                                                                      *
 *                 Modifs mineures pour compatibilite                   *
 *                                noyau Linux v2.6.31 : le  1/12/2009   *
 *                                                                      *
 *           Rempacement ioctl() par unlocked_ioctl()                   *
 *                    pour compatibilite Linux v3.3.x : le  8/04/2013   *
 *                                                                      *
 *  Modification de l'interface CEVT (utilisation du lien               *
 *  standard Linux au lieu du "bricolage" precedent :   le  1/09/2014   *
 *                                                                      *
 ************************************************************************/

/*
 * Repartition des numeros Majeur/mineur entre les cartes et les coupleurs
 * selon le systeme d'exploitation utilise :
 *
 *                                      LynxOS      Linux
 *
 *         Carte/Coupleur              Maj/min     Maj/min
 *    ------------------------------ ----------- -----------
 *     carte 1 - coupleur 1             M / 0       M / 0
 *     carte 1 - coupleur 2		 M / 1	     M / 1
 *     carte 2 - coupleur 1		M+1 / 0	     M / 2
 *     carte 2 - coupleur 2		M+1 / 1	     M / 3
 *     carte 3 - coupleur 1		M+2 / 0	     M / 4
 *     carte 3 - coupleur 1		M+2 / 1	     M / 5
 *     carte 4 - coupleur 1		M+3 / 0	     M / 6
 *     carte 4 - coupleur 1		M+3 / 1	     M / 7
 */

/***********************************************************/
/*   Inclusion des fichiers .h pour LynxOS ou pour Linux   */
/***********************************************************/

#define KERNEL

#ifdef LYNXOS

#ifdef LYNX_V4
#include <types.h>
#endif /* LYNX_V4 */

#include <dldd.h>
#include <errno.h>
#include <io.h>
#include <kernel.h>
#include <mem.h>
#include <pci_powerplus.h>
#include <pci_resource.h>
#include <stdio.h>
#include <sys/drm_sysctl.h>
#include <sys/file.h>
#include <sys/pci_sysctl.h>
#include <sys/sysctl.h>

#elif LINUX

#define PRINCIPAL

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#else
#include <linux/sched.h>
#endif

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#include <linux/wait.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#include <linux/vmalloc.h>
#endif
#else

#error "L'un au moins des 2 symboles LINUX ou LYNXOS doit etre defini"

#endif /* LynxOS - Linux */

#ifdef LYNXOS
#ifdef LINUX
#error "Un seul des 2 symboles LINUX ou LYNXOS doit etre defini"
#endif
#endif

#include "interface.h" /* definit les macros de portage LynxOS/Linux */
#include "version.h"   /* Definit le numero de version du pilote */

/* Pour DEBUG, commenter ou decommenter les 2 lignes ci-dessous */
/* #define DEBUG    */
/* #define DEBUG_IT */

/* dprintf() peut etre changee en KPRINTF, CPRINTF ou ignoree */
#ifdef DEBUG
#define dprintf KPRINTF
/* #define dprintf	CPRINTF */
#else
#define dprintf bidon
#endif

/* zzprintf() est utilise dans fonction IT seulement        */
/* Elle peut etre changee en KPRINTF ou ignoree            */
/* L'appel de CPRINTF() est interdit dans une fonction d'IT */
#ifdef DEBUG_IT
#define zzprintf KPRINTF
#else
#define zzprintf bidon
#endif

#ifdef LINUX

/* Nombre maximum de cartes 1553 utilisables sur un systeme Linux	*/
/* (avec 1 ou 2 coupleurs 1553 par cartes)				*/
/*									*/
/* En cas de modification de ce symbole, penser a modifier aussi	*/
/* ci-dessous :								*/
/* 	- la liste des declarations des opti[] et n_opti		*/
/*	- la liste des appels a module_param_array(opti, ..)		*/
/*	- le code de la fonction option()				*/
/*									*/
#define ASDC_MAX_CARTES 7

/* Variables utilisees pour le passage de parametres sous Linux */
/*    Remarque : ces variables devraient peut-etre */
/*               etre marquees "__initdata" ???    */

/* Numero majeur propose pour le pilote (si 0, choix laisse au systeme) */
static int majeur = 0;

/* Options associees a chacune des cartes :                               */
/*                                                                        */
/*    opti[0] a opti[2] sont associes au premier coupleur de la carte :   */
/*	opti[0] = numero "device_number" associe au coupleur              */
/*	opti[1] = nombre de tampons flux BC alloues                       */
/*	opti[2] = nombre de "wait queues" Linux allouees                  */
/*                                                                        */
/*    opti[3] a opti[5] sont associes au second coupleur de la carte :    */
/*	opti[3] = numero "device_number" associe au coupleur              */
/*	opti[4] = nombre de tampons flux BC alloues                       */
/*	opti[5] = nombre de "wait queues" Linux allouees                  */
/*                                                                        */
/* Un numero "device_number" nul indique un coupleur absent.              */
/* Par defaut : une seule carte et un seul coupleur.                      */
/*                                                                        */
static int opt1[6] = {1, 11904, 100, 0, 11904, 100};
static int opt2[6] = {0, 11904, 100, 0, 11904, 100};
static int opt3[6] = {0, 11904, 100, 0, 11904, 100};
static int opt4[6] = {0, 11904, 100, 0, 11904, 100};
static int opt5[6] = {0, 11904, 100, 0, 11904, 100};
static int opt6[6] = {0, 11904, 100, 0, 11904, 100};
static int opt7[6] = {0, 11904, 100, 0, 11904, 100};
static int n_opt1 = 3;
static int n_opt2 = 0;
static int n_opt3 = 0;
static int n_opt4 = 0;
static int n_opt5 = 0;
static int n_opt6 = 0;
static int n_opt7 = 0;

module_param(majeur, int, S_IRUGO);
MODULE_PARM_DESC(majeur, "Numero de device majeur associe au module");

module_param_array(opt1, int, &n_opt1, S_IRUGO);
module_param_array(opt2, int, &n_opt2, S_IRUGO);
module_param_array(opt3, int, &n_opt3, S_IRUGO);
module_param_array(opt4, int, &n_opt4, S_IRUGO);
module_param_array(opt5, int, &n_opt5, S_IRUGO);
module_param_array(opt6, int, &n_opt6, S_IRUGO);
module_param_array(opt7, int, &n_opt7, S_IRUGO);
MODULE_PARM_DESC(opt1, "Carte 1 : numero coupl. 1 ou 0, nb tamp. flux C1,"
                       " nb wait queues C1, numero Coupl. ou 0, "
                       "nb tamp. flux C2, nb wait queues C2");
MODULE_PARM_DESC(opt2, "Idem opt1 pour carte 2");
MODULE_PARM_DESC(opt3, "Idem opt1 pour carte 3");
MODULE_PARM_DESC(opt4, "Idem opt1 pour carte 4");
MODULE_PARM_DESC(opt5, "Idem opt1 pour carte 5");
MODULE_PARM_DESC(opt6, "Idem opt1 pour carte 6");
MODULE_PARM_DESC(opt7, "Idem opt1 pour carte 7");

/* Description du module */
MODULE_AUTHOR("************ GUILLEMOT & Alexandre BEAUGY");
MODULE_DESCRIPTION("Pilote " ASDC_NOM "v" ASDC_VERSION " du " ASDC_DATE);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0))
MODULE_SUPPORTED_DEVICE("Coupleurs 1553 SBS ABI/ASF-PMC2");
#endif

/* Fonctions pour faciliter l'acces aux differents parametres optionnels */
static int option(int carte, int i) {
  if ((carte < 1) || (carte > ASDC_MAX_CARTES) || (i < 0) || (i > 5)) {
    KPRINTF("ATTENTION : Appel de option(%d, %d) !!!\n", carte, i);
    return 0; /* devrait limiter les degats ... */
  }

  switch (carte) {
  case 1:
    return opt1[i];
  case 2:
    return opt2[i];
  case 3:
    return opt3[i];
  case 4:
    return opt4[i];
  case 5:
    return opt5[i];
  case 6:
    return opt6[i];
  case 7:
    return opt7[i];
  }

  return 0; /* Cette instruction ne devrait jamais �tre utilis�e */
}

static int nombre_options(int carte) {
  if ((carte < 1) || (carte > ASDC_MAX_CARTES)) {
    KPRINTF("ATTENTION : Appel de nombre_options(%d) !!!\n", carte);
    return 0; /* devrait limiter les degats ... */
  }

  switch (carte) {
  case 1:
    return n_opt1;
  case 2:
    return n_opt2;
  case 3:
    return n_opt3;
  case 4:
    return n_opt4;
  case 5:
    return n_opt5;
  case 6:
    return n_opt6;
  case 7:
    return n_opt7;
  }

  return 0; /* Cette instruction ne devrait jamais �tre utilis�e */
}

#endif /* LINUX */

#include "asdcctl.h"  /* Definit les structures de donnees et codes ioctl */
#include "asdcinfo.h" /* Definit les parametres decrivant la carte */
#include "asdcvarg.h" /* Definit les "variables globales" */

#ifdef LINUX
#include "asdcwq.h" /* Implementation des "wait queues" Linux */
#endif

#ifdef CEVT
#include "cevtctl.h" /* Pour connexion au driver CEVT */
#endif               /* CEVT */

/*********************************************/
/* Declarations necessaires de fonctions ... */
/*********************************************/

/* Fonctions non specifiques */

static int initialisation_coupleur(struct asdc_varg **pstat,
                                   struct asdc_statics *drstat, int num,
                                   int nb_tamp, int nb_wq);
static void asdcintr_coupleur(struct asdc_varg *dst);
static void bcflux_err(struct asdc_varg *dst, uint16 bc, uint16 flux, uint16 zd,
                       uint16 code, uint16 arg1, uint16 arg2);

int asdc_fluxbctamp_nouveau(struct asdcbc_tfch **psrc, struct asdcbc_tfch **pt);
int asdc_fluxbctamp_retirer(struct asdcbc_tfch **psrc,
                            struct asdcbc_tfch **pdst);
int asdc_fluxbctamp_supprimer(struct asdcbc_tfch **ppsrc,
                              struct asdcbc_tfch **pdsrc,
                              struct asdcbc_tfch **pdst);
int asdc_fluxbctamp_ajouter(struct asdc_varg *dst, struct asdcbc_tfch *p,
                            int flux, int zd, struct asdcbc_tfch **pdispo);

#ifdef LYNXOS

int asdcioctl(struct asdc_statics *driver_statics, struct file *f, int commande,
              char *arg);

/* Fonctions specifiques LynxOS */
void asdcintr(struct asdc_statics *);

#elif LINUX

#ifdef OLDIOCTL
int asdcioctl(struct inode *inode, struct file *f, unsigned int commande,
              unsigned long arg);
#else
long asdcioctl(struct file *f, unsigned int commande, unsigned long arg);
#endif

/* Fonctions ou declarations specifiques Linux */

static int __init init_asdc(void);
char *asdcinstall(struct abi_pmc_info *info);
int asdcopen(struct inode *inode, struct file *f);
int asdcclose(struct inode *inode, struct file *f);

#ifdef LINUX26
irqreturn_t asdcintr(int irq, void *dev_id /*, struct pt_regs *reg_ptr*/);
#else
void asdcintr(int irq, void *dev_id, struct pt_regs *reg_ptr);
#endif /* LINUX26 */

static void __exit cleanup_asdc(void);

/* Fonctions d'initialisation et de desinstallation */
module_init(init_asdc);
module_exit(cleanup_asdc);

/*
** License du module
** ("GPL" evite un warning lors du chargement du module sous Linux,
**  mais, le 3/6/2005, la politique de TE641/EADS-ST concernant
**  cette license reste a definir  ... )
*/
MODULE_LICENSE("EADS-ST");

/* Points d'entree du driver */
static struct file_operations asdcfops = {
  owner : THIS_MODULE,
  open : asdcopen,
  release : asdcclose,
  unlocked_ioctl : asdcioctl
};

/* Structures pour rechercher les cartes sur le bus PCI */
static struct pci_dev *ti9050_pci_dev = NULL;
static struct pci_dev *sbs1553_pci_dev = NULL;

#endif /* LYNXOS - LINUX */

/*********************************************************************
 * Donnees statiques du driver (communes aux differents devices)     *
 *********************************************************************/

/* Table de description des commandes codees autorisees */
/* Indice = code de la commande                            */
/* Mot 1 (def) :  bit 0 = bit T/R associe a la commande             */
/*                bit 1 = 1 si commande definie (autorisee)         */
/* Mot 2 (hmim) : Bit correspondant dans mot de poids fort du MIM   */
/* Mot 3 (lmim) : Bit correspondant dans mot de poids faible du MIM */
const struct sdcoco asdc_coco[] = {
    {3, 0x0000, 0x0001}, /*  0 : GESDYN (Dynamic Bus Control) */
    {3, 0x0000, 0x0002}, /*  1 : SYNC ("Synchronize) */
    {3, 0x0000, 0x0004}, /*  2 : DSTATUS (Transmit Status) */
    {3, 0x0000, 0x0008}, /*  3 : TEST (Initiate Self Test) */
    {3, 0x0000, 0x0010}, /*  4 : TS (Transmitter Shutdown) */
    {3, 0x0000, 0x0020}, /*  5 : OTS (Override Transmitter Shutdown) */
    {3, 0x0000, 0x0040}, /*  6 : ITFB (Inhibit Terminal Flag Bit) */
    {3, 0x0000, 0x0080}, /*  7 : OITFB (Override Inhibit Terminal Flag Bit) */
    {3, 0x0000, 0x0100}, /*  8 : REINIT (Reset RT) */
    {3, 0x0000, 0x0200}, /*  9 : LP (Lancement Programme) */
    {3, 0x0000, 0x0400}, /* 10 : ABORT (Arrete Programme) */
    {3, 0x0000, 0x0800}, /* 11 : FLASH (Simulation Flash) */
    {3, 0x0000, 0x1000}, /* 12 : WMSO (Autorisation Ecriture en MSO) */
    {3, 0x0000, 0x2000}, /* 13 : VALID (Valide la Fonction Gerant) */
    {3, 0x0000, 0x4000}, /* 14 : INHIB (Inhibe la Fonction Gerant) */
    {3, 0x0000, 0x8000}, /* 15 : MODGER (Autorise Changement Mode gerant) */

    {3, 0x0001, 0x0000}, /* 16 : VECTEUR (Transmit Vector Word) */
    {2, 0x0002, 0x0000}, /* 17 : SD (Synchronize (with data word)) */
    {3, 0x0004, 0x0000}, /* 18 : DCDE (Transmit Last Command) */
    {3, 0x0008, 0x0000}, /* 19 : BIT (Transmit Built In Test Word) */
    {2, 0x0010, 0x0000}, /* 20 : STS (Selected Transmitter Shutdown) */
    {2, 0x0020, 0x0000}, /* 21 : OSTS (Override Select. Transm. Shutdown) */
    {2, 0x0040, 0x0000}, /* 22 : RES (Reserve) */
    {0, 0x0000, 0x0000}, /* 23 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 24 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 25 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 26 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 27 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 28 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 29 : Commande invalide */
    {0, 0x0000, 0x0000}, /* 30 : Commande invalide */
    {0, 0x0000, 0x0000}  /* 31 : Commande invalide */
};

/*********************************************************************
 * Sous LynxOS, les "devices" sont installes separement, au rythme
 * des appels a l'utilitaire devinstall.
 *
 * Sous Linux, tous les devices doivent etre installes au cours de
 * l'unique appel de la fonction init_asdc() d'initialisation du
 * driver.
 *
 * La fonction init_asdc ci-dessous (Linux uniquement) va donc parcourir
 * la liste des parametres (explicites ou par defaut), preparer une
 * structure info identique a celle utilisee sous LynxOS, puis appeler,
 * pour chacune des cartes, la fonction asdcinstall() que LynxOS utilise
 * egalement.
 *********************************************************************/
#ifdef LINUX
static int __init init_asdc(void) {
  struct abi_pmc_info info;
  int k, rslt;
  int sig1, sig2;
  struct asdc_statics *r;

  /* Enregistrement du driver */
  rslt = register_chrdev(majeur, ASDC_NOM, &asdcfops);

  if (rslt < 0) {
    KPRINTF("\nEchec installation du driver %s v%s\n", ASDC_NOM, ASDC_VERSION);
    if (rslt == -EINVAL) {
      KPRINTF("        ==> Le Majeur specifie est hors bornes\n");
    }
    if (rslt == -EBUSY) {
      KPRINTF("        ==> Le Majeur specifie est indisponible\n");
    }
    KPRINTF("\n");
    return rslt;
  }

  if (rslt > 0) {
    majeur = rslt;
  }

  KPRINTF("\nDebut installation du driver %s v%s   -   Majeur = %d\n", ASDC_NOM,
          ASDC_VERSION, majeur);

  /* Initialisation a NULL des entrees de la table des donnees statiques */
  /* (pour pouvoir tester (dans open) la validite des mineur utilises)   */
  for (k = 0; k < ASDC_MAX_CARTES; k++)
    asdc_p[k] = NULL;

  /* Boucle de parcours des parametres */
  for (k = 1; k <= ASDC_MAX_CARTES; k++) {
    sig1 = option(k, 0);
    if (sig1 == 0)
      break;

    info.signal_number_1 = sig1;
    info.nombre_tampons_flux_1 = option(k, 1);
    info.nombre_wq_1 = option(k, 2);
    info.bivoie = 0;

    sig2 = option(k, 3);
    if (sig2 != 0) {
      info.signal_number_2 = sig2;
      info.nombre_tampons_flux_2 = option(k, 4);
      info.nombre_wq_2 = option(k, 5);
      info.bivoie = 1;
    }

    r = (struct asdc_statics *)asdcinstall(&info);
    if (((long)r) == SYSERR)
      break;

    asdc_p[k - 1] = r; /* Memorisation adresse structure statique carte */
  }

  if (k == 0) {
    /* Aucun coupleur n'a ete trouve sur le bus PCI */
    /* ==> Desenregistrement du driver !            */
    unregister_chrdev(majeur, ASDC_NOM);
    KPRINTF("Aucun coupleur trouve   -   Echec de l'installation !\n\n");
    return -EIO;
  }

  /* Installation achevee */
  KPRINTF("Fin installation du driver %s v%s   -   Majeur = %d\n\n", ASDC_NOM,
          ASDC_VERSION, majeur);
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

char *asdcinstall(struct abi_pmc_info *info) {
  int ret;
  struct asdc_statics *driver_statics;
  struct drm_node_s *sbs_device_node;
  int device;
  unsigned int base;
  short int *temp;
  char *wrdptr;
  int off;
  int cs_buffer;
  int *base_address;
  int ti_cmd1, ti_cmd2, ti_cmd3, ti_cmd4, ti_cmd5, ti_cmd6, ti_cmd7, ti_cmd8;
  void *ti9050_h_drm;
  int i, j, k, l;

#ifdef LYNXOS
  kkprintf("\nSBS1553 PMC Lynx driver asdcinstall()\n");
#endif

  /*********************************************************************/
  /* Coupleurs ABI/ASF-PMC2 seulement : Recherche du bridge TI PCi2050 */
  /*********************************************************************/

  KPRINTF("\nRecherche du bridge TI PCi2050 : ");

#ifdef LYNXOS

  /* -----------------------------------------------------------------
     -- PMC2 ONLY!                                                  --
     -- Locate the Texas Instruments PCi2050 bridge chip on the PMC --
     ----------------------------------------------------------------- */

  ret = drm_get_handle(PCI_BUSLAYER, ASDC_TEXAS_VENDOR_ID, ASDC_TEXAS_DEVICE_ID,
                       &ti9050_h_drm);

  if (ret != 0) /* Anciennement ( ret == SYSERR ) */
  {
    kkprintf("drm_get_handle() = 0x%X  ==> Echec !\n", ret);

    /* Initialement :                                             */
    /*    Pas de sortie sur erreur : Un seul bridge pour les deux */
    /*    voies ==> il est normal qu'une erreur soit detectee sur */
    /*    la seconde voie !                                       */
    /* Depuis v4.7 :                                              */
    /*    Un bridge par carte (simple ou double, selon (params))  */

    pseterr(ret);
    return (char *)SYSERR;
  } else {
    /* -------------------------------------------------
       -- Lecture faite dans le driver initial de Bob Pickles
       -- et conservee ici, bien que son utilite ne saute pas
       -- aux yeux ...
       ------------------------------------------------- */
    ret = drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 0, 0, &ti_cmd1);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 1, 0, &ti_cmd2);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 2, 0, &ti_cmd3);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 3, 0, &ti_cmd4);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 4, 0, &ti_cmd5);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 5, 0, &ti_cmd6);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 6, 0, &ti_cmd7);
    ret |= drm_device_read(ti9050_h_drm, PCI_RESID_REGS, 7, 0, &ti_cmd8);

    kkprintf("OK\n");
  }

#elif LINUX

  ti9050_pci_dev = pci_get_device(ASDC_TEXAS_VENDOR_ID, ASDC_TEXAS_DEVICE_ID,
                                  ti9050_pci_dev);

  if (ti9050_pci_dev == NULL) {
    KPRINTF(" Echec !\n");

    /* Initialement :                                             */
    /*    Pas de sortie sur erreur : Un seul bridge pour les deux */
    /*    voies ==> il est normal qu'une erreur soit detectee sur */
    /*    la seconde voie !                                       */
    /* Depuis v4.7 :                                              */
    /*    Un bridge par carte (simple ou double, selon (params))  */

    return (char *)SYSERR;
  } else {
    /* -------------------------------------------------
       -- Lecture faite dans le driver initial de Bob Pickles
       -- et conservee ici, bien que son utilite ne saute pas
       -- aux yeux ...
       ------------------------------------------------- */
    ret = pci_read_config_dword(ti9050_pci_dev, 0x00, &ti_cmd1);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x04, &ti_cmd2);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x08, &ti_cmd3);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x0c, &ti_cmd4);

    ret |= pci_read_config_dword(ti9050_pci_dev, 0x10, &ti_cmd5);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x14, &ti_cmd6);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x18, &ti_cmd7);
    ret |= pci_read_config_dword(ti9050_pci_dev, 0x1c, &ti_cmd8);

    KPRINTF(" OK\n");
  }

#endif /* LYNXOS - LINUX */

  if (ret) {
    KPRINTF("Probleme d'acces aux registres du bridge !\n");
    /* Pas de sortie sur erreur : ;l'utilite de cet acces */
    /* au bridge n'est pas evident ! 		     */
  } else {
    dprintf("TI PCi2050 Bridge Command Reg is \n");
    dprintf("Dev_Ven %x \nStat_Cmd %x \nCls_Rev %x \n", ti_cmd1, ti_cmd2,
            ti_cmd3);
    dprintf("Bist_Hd_PLT_CLS %x\n", ti_cmd4);
    dprintf("BAR0 %x \nBAR1 %x \nSblt_sebn_Sbn_Pbn %x \n %x\n", ti_cmd5,
            ti_cmd6, ti_cmd7, ti_cmd8);
  }

  /****************************************************************/
  /* Un bridge ayant ete detecte, on peut supposer qu'un coupleur */
  /* existe et allouer les structures de donnees necessaires      */
  /****************************************************************/

#ifdef LYNXOS

  /*** Allocation de la structure mere des donnees statiques ***/

  driver_statics = (struct asdc_statics *)sysbrk(sizeof(struct asdc_statics));
  if (driver_statics == (struct asdc_statics *)0L) {
    kkprintf("ASDC : Echec allocation memoire pour \"asdc_statics\"\n");
    return (char *)SYSERR;
  }

#elif LINUX

  driver_statics = vmalloc(sizeof(struct asdc_statics));
  if (driver_statics == NULL) {
    KPRINTF("ASDC : Echec allocation memoire pour \"statics\"\n");
    return (char *)SYSERR;
  }

#endif /* LYNXOS - LINUX */

  /*** Initialisation de la structure mere ***/

  /* Nombre de coupleurs attendu sur la carte */
  driver_statics->bivoie = info->bivoie;

  /* Pour l'instant, la presence du second coupleur reste a verifier */
  driver_statics->voie2 = 0;

  /* Structures de donnees non encore allouees */
  driver_statics->varg[0] = NULL;
  driver_statics->varg[1] = NULL;

#ifdef LINUX
  /* Memorisation du "device PCI" gere par le noyau Linux */
  driver_statics->pci_dev_texas = ti9050_pci_dev;
#endif /* LINUX */

  /* Initialisation du premier coupleur */
  ret = initialisation_coupleur(&driver_statics->varg[0], driver_statics,
                                info->signal_number_1,
                                info->nombre_tampons_flux_1, info->nombre_wq_1);

  /* En cas d'echec, liberation de la structure mere des donnees statiques */
  if (ret) {
#ifdef LYNXOS
    sysfree((char *)driver_statics, sizeof(struct asdc_statics));
    pseterr(ret);
    return (char *)SYSERR;
#elif LINUX
    /* TODO : Desallocation eventuelle de la liste des wait queues */
    vfree(driver_statics);
    return (char *)SYSERR;
#endif /* LYNXOS - LINUX */
  }

  /* Tout s'est bien passe */
  /* Faut-il initialiser un second coupleur ? */
  if (driver_statics->bivoie) {
    /* Initialisation du second coupleur */
    ret = initialisation_coupleur(
        &driver_statics->varg[1], driver_statics, info->signal_number_2,
        info->nombre_tampons_flux_2, info->nombre_wq_2);

    /* En cas de succes, memorisation disponibilite du second coupleur */
    if (!ret) {
      driver_statics->voie2 = 1;
    } else { /* En cas d'echec, on ne fait rien pour laisser au systeme */
      /* la possibilite d'utiliser le premier coupleur.          */
      KPRINTF("*** ECHEC installation du second 1/2 coupleur %d ***\n",
              info->signal_number_2);
    }
  }

  /* En cas de succes, total ou partiel, retour de */
  /* l'adresse de la zone des donnees statiques    */
  return (char *)driver_statics;
}

/*******************************************************************
 * Fonction d'initialisation d'un unique coupleur                  *
 *   pstat : Emplacement ou ecrire l'adresse des donnees statiques *
 *           du coupleur qui seront allouees par la fonction       *
 *   drstat : Adresse des donnees statiques de la carte            *
 *   num : Numero attribue au coupleur (Attention : c'est un       *
 *         parametre d'installation qui peut differer du mineur    *
 *   nb_tamp : nombre de tampons "flux BC" a allouer               *
 *   nb_wq : nombre de "wait queues" Linux a allouer               *
 *******************************************************************/
static int initialisation_coupleur(struct asdc_varg **pstat,
                                   struct asdc_statics *drstat, int num,
                                   int nb_tamp, int nb_wq) {

  int dev_cmd_stat;
  int ret;
  void *h_drm;
  void *bvba;
  int i, j;

#ifdef LINUX
  unsigned long val;
  unsigned int val32;
#endif

  /* -----------------------------------------------
     -- Recherche du coupleur 1553 sur le bus PCI --
     ----------------------------------------------- */
  KPRINTF("Recherche vendor/device 0x%04X/0x%04X : ", ASDC_VENDOR_ID,
          ASDC_DEVICE_ID);

#ifdef LYNXOS

  ret = drm_get_handle(PCI_BUSLAYER, ASDC_VENDOR_ID, ASDC_DEVICE_ID, &h_drm);

  if (ret != 0) /* Anciennement ( ret == SYSERR ) */
  {
    kkprintf("drm_get_handle()=0x%X\n", ret);
    kkprintf("  ==> ECHEC recherche coupleur !\n");

    return ret;
  } else {
    kkprintf("OK\n");

#ifdef DEVELOPMENT_TEST
    /* Affichage des registres de    */
    /* configuration PCI du coupleur */
    {
      int ret;
      int val;
      int i;

      cprintf("\n---------------\n");
      for (i = 0; i < 64; i++) {
        ret = drm_device_read(h_drm, PCI_RESID_REGS, i, 0, &val);
        if (ret) {
          cprintf("\nERREUR = 0x%X\n", ret);
        }
        cprintf(" %08X", val);
        if ((i + 1) % 8 == 0)
          cprintf("\n");
      }
      cprintf("\n---------------\n");
    }
#endif /* DEVELOPMENT_TEST */

    /* ------------------------------------------------------------------
       -- Enabling PMC Device Memory  --                              --
       ---------------------------------                              --
       -- In order to enable PMC memory with the TI9050 bridge chip,  --
       -- the Status/Command Register MUST be READ firstly!!!         --
       --                                                             --
       -- If the Status register flags errors, then clear the errors  --
       --                                                             --
       -- Update the Command Register to 0x0002 to enable Memory and  --
       -- 0x0003 to allow IO access for DMA...                        --
       --                                                             --
       -- Note : Bit 0 : IO enable, Bit 1 : Mem enable                --
       --        Bit 2 : Bus Master Enable.                           --
       -- NOTE : Always READ before WRITING to ENABLE memory!         --
       ----------------------------------------------------------------- */

    ret = drm_device_read(h_drm, PCI_RESID_REGS, 1, 0, &dev_cmd_stat);
    // kkprintf("read PCI_RESID_REGS, 1, 0 : 0x%08X\n",  dev_cmd_stat);

    /* -- Clear any errors by writing a high BIT to register -- */
    /******* POURQUOI NE MODIFIE-T-ON PAS dev_cmd_stat ??????? *******/
    ret = drm_device_write(h_drm, PCI_RESID_REGS, 1, 0, &dev_cmd_stat);
    // kkprintf("write PCI_RESID_REGS, 1, 0 : 0x%08X\n",  dev_cmd_stat);

    /* -- Enable Memory Access -- */
    dev_cmd_stat = dev_cmd_stat | 0x0000002;
    ret = drm_device_write(h_drm, PCI_RESID_REGS, 1, 0, &dev_cmd_stat);
    // kkprintf("write PCI_RESID_REGS, 1, 0 : 0x%08X\n",  dev_cmd_stat);

    /* ----------------------------------------------------------
       -- Map the device IO memory of 1mb using PCI_RESID_BAR2 --
       -- Mapping PCI_BAR_0 will map only the 256hex bytes     --
       -- of PCI configuration space...                        --
       ---------------------------------------------------------- */
    /*
       Pourquoi, dans le code ci-dessous, ne fait-on pas ce qui
       est ecrit dans le commentaire ci-dessus ???!!!
    */
    bvba = 0;
    ret = drm_map_resource(h_drm, PCI_RESID_BAR0, &bvba);
    dprintf("mapped device no. %d to virtual address 0x%X \n", num, bvba);

    if (ret) {
      kkprintf("\nProbleme DRM_MAP_RESOURCE(device %d) : 0x%X\n", num, ret);
      return ret;
    }
  }

#elif LINUX

  sbs1553_pci_dev =
      pci_get_device(ASDC_VENDOR_ID, ASDC_DEVICE_ID, sbs1553_pci_dev);

  if (sbs1553_pci_dev == NULL) {
    kkprintf("ECHEC recherche coupleur !\n");
    return -ENODEV;
  } else {
    kkprintf("OK\n");

#ifdef DEVELOPMENT_TEST
    /* Affichage des registres de    */
    /* configuration PCI du coupleur */
    {
      int ret;
      int val;
      int i;

      KPRINTF("\n---------------\n");
      for (i = 0; i < 64; i++) {
        ret = pci_read_config_dword(sbs1553_pci_dev, i * 4, &val);
        if (ret) {
          KPRINTF("\nERREUR = 0x%X\n", ret);
        }
        KPRINTF(" %08X", val);
        if ((i + 1) % 8 == 0)
          KPRINTF("\n");
      }
      KPRINTF("\n---------------\n");
    }
#endif /* DEVELOPMENT_TEST */

    /* ---------------------------------
       -- Enabling PMC Device Memory  --
       ---------------------------------*/

    ret = pci_enable_device(sbs1553_pci_dev);
    if (ret) {
      kkprintf("Echec pci_enable_device() !\n");
      return ret;
    }

    /* Lecture adresse physique du coupleur */
    ret = pci_read_config_dword(sbs1553_pci_dev, PCI_BASE_ADDRESS_0, &val32);
    if (ret) {
      kkprintf("Echec lecture adresse physique coupleur !\n");
      return ret;
    }

    /* Mappage du coupleur dans l'espace d'adressage virtuel */
    val = val32;
    bvba = ioremap(val, 0x100000);
  }

#endif /* LYNXOS - LINUX */

  KPRINTF("Adresse virtuelle du coupleur = 0x%lX\n", (unsigned long)bvba);

#ifdef LYNXOS
  KPRINTF("Adresse physique du coupleur =  0x%X\n", (long)get_phys((long)bvba));
#elif LINUX
  KPRINTF("Adresse physique du coupleur =  0x%lX\n", val);
#endif

#ifdef DEVELOPMENT_TEST
  /* ------------------------------------------------
     -- Development test to see memory on the PMC2 --
     ------------------------------------------------ */
  if (1) {
    base_address = (int *)bvba;
    KPRINTF("\nAffichage de la memoire PCI a partir de l'adresse de base\n");

    for (i = 0; i < 256; i++) {
      KPRINTF("0x%08X,%c", *base_address++, ((i + 1) % 8) ? ' ' : '\n');
    };
  }
#endif /* DEVELOPMENT_TEST */

  /* ---------------------------------------------------------------
     -- Allocation memoire pour les variables statiques du driver --
     --------------------------------------------------------------- */

#ifdef LYNXOS

  *pstat = (struct asdc_varg *)sysbrk(sizeof(struct asdc_varg));
  if (*pstat == (struct asdc_varg *)0L) {
    kkprintf("ASDC : Echec allocation memoire pour \"asdc_varg\"");
    return ENOMEM;
  }

#elif LINUX

  *pstat = vmalloc(sizeof(struct asdc_varg));
  if (*pstat == NULL) {
    KPRINTF("ASDC : Echec allocation memoire pour \"asdc_varg\"");
    return SYSERR;
  }

#endif /* LYNXOS - LINUX */

  /* ------------------------------------------------------------
     -- Mise a jour de la structure asdc_varg avec les donnees --
     -- issues de la structure info et de la configuration PCI --
     ------------------------------------------------------------ */

#ifdef LYNXOS
  (*pstat)->h_drm = h_drm;
  (*pstat)->bpba = (void *)get_phys((long)bvba);
#elif LINUX
  (*pstat)->h_drm = sbs1553_pci_dev;
  (*pstat)->bpba = (void *)val;
  (*pstat)->irq = sbs1553_pci_dev->irq;
#endif

  (*pstat)->bvba = bvba;
  (*pstat)->signal_number = num;
  (*pstat)->nombre_tampons_flux = nb_tamp;
  (*pstat)->nombre_wait_queues = nb_wq;

  /* -------------------------------------------------
     --   Allocation des tampons pour E/S flux BC   --
     ------------------------------------------------- */

#ifdef LYNXOS

  (*pstat)->pbcf =
      (struct asdcbc_tfch *)sysbrk(sizeof(struct asdcbc_tfch) * nb_tamp);
  if ((*pstat)->pbcf == (struct asdcbc_tfch *)0L) {
    kkprintf("ASDC : Echec allocation memoire pour tampons flux BC\n");

    /* Echec installation ==> Liberation memoire deja allouee */
    sysfree((char *)*pstat, sizeof(struct asdc_varg));

    return ENOMEM;
  }

#elif LINUX

  (*pstat)->pbcf = vmalloc(sizeof(struct asdcbc_tfch) * nb_tamp);
  if ((*pstat)->pbcf == NULL) {
    kkprintf("ASDC : Echec allocation memoire pour tampons flux BC\n");

    /* Echec installation ==> Liberation memoire deja allouee */
    vfree(*pstat);

    // /* liberation du device PCI ??????? */
    // iounmap(bvba);
    // pci_disable_device(sbs1553_pci_dev);

    return ENOMEM;
  }

#endif /* LynxOS - LINUX */

  /* ---------------------------------------------------------
     --   Declaration de la fonction de traitement des IT   --
     --------------------------------------------------------- */

#ifdef LYNXOS

  (*pstat)->link_id = drm_register_isr((*pstat)->h_drm, asdcintr, drstat);

  if ((*pstat)->link_id == SYSERR) {
    kkprintf("asdc : ERREUR declaration ISR   (signal_number = %d)\n", num);

    /* Echec installation ==> Liberation memoire deja allouee */
    sysfree((char *)(*pstat)->pbcf, sizeof(struct asdcbc_tfch) * nb_tamp);
    sysfree((char *)(*pstat), sizeof(struct asdc_varg));

    return SYSERR;
  }

#elif LINUX
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
  if (request_irq(sbs1553_pci_dev->irq, asdcintr, IRQF_DISABLED | IRQF_SHARED,
                  ASDC_NOM, drstat))
#else
  if (request_irq(sbs1553_pci_dev->irq, asdcintr, IRQF_SHARED, ASDC_NOM,
                  drstat))
#endif
  {
    KPRINTF("asdc: Echec utilisation IRQ %d\n", sbs1553_pci_dev->irq);

    /* Echec installation ==> Liberation memoire deja allouee */
    // /* liberation du device PCI ??????? */
    // iounmap(bvba);
    // pci_disable_device(sbs1553_pci_dev);
    vfree((*pstat)->pbcf);
    vfree(*pstat);

    return -EIO;
  }

#endif /* LynxOS - LINUX */

  /* ---------------------------------------------
     --   Memorisation des valeurs par defaut   --
     --------------------------------------------- */
  (*pstat)->asdcdef.iqnum = DEF_IQNUM;
  (*pstat)->asdcdef.mbleng = DEF_MBLENG;
  (*pstat)->asdcdef.bcsmsk = DEF_BCSMSK;
  (*pstat)->asdcdef.bcigp = DEF_BCIGP;
  (*pstat)->asdcdef.brtcnt = DEF_BRTCNT;
  (*pstat)->asdcdef.brtbus = DEF_BRTBUS;
  (*pstat)->asdcdef.rspgpa = DEF_RSPGPA;
  (*pstat)->asdcdef.rspgps = DEF_RSPGPS;

  /* ---------------------------------------
     --   Initialisation des semaphores   --
     --------------------------------------- */
  (*pstat)->mutexalloc = 1;
  (*pstat)->mutexflux = 1;
  SINIT((*pstat)->semmon);
  SINIT((*pstat)->sem_finbc);
  SINIT((*pstat)->sem_gotbc);
  SINIT((*pstat)->sem_exbc);
  SINIT((*pstat)->semirig);

  /* ------------------------------------------------------
     --   Initialisation de la liste des "wait queues"   --
     ------------------------------------------------------ */

#ifdef LYNXOS

#elif LINUX

  wq_creer(*pstat);

#endif /* LYNXOS - LINUX */

  /* ---------------------------------------------------------
     --   Initialisation de la table des commandes codees   --
     --------------------------------------------------------- */
  {
    for (i = 0; i < 32; i++)
      for (j = 0; j < (COCO_MAX + 1); j++)
        (*pstat)->cocor[i][j] = (struct scoco){0, 0, 0, 0};
  }

  /* ----------------------------------------------------------------
     --   Initialisation a "non disponibles" des tampons flux BC   --
     ---------------------------------------------------------------- */
  for (j = 0; j < (*pstat)->nombre_tampons_flux; j++) {
    (*pstat)->pbcf[j].s = NULL;
  }
  (*pstat)->pbcfl = NULL;
  (*pstat)->nb_tamp_flux_dispos = 0;

  /* ---------------------------------
     --   Initialisation diverses   --
     --------------------------------- */
  (*pstat)->vflash = 0;
  (*pstat)->raz = 0;
  (*pstat)->nbhistoit = 0;
  (*pstat)->deborde = 0;
  (*pstat)->tf_attente = 0;
  (*pstat)->tf_zd = 0;
  (*pstat)->tf_flux = 0;
  (*pstat)->tf_pta = NULL;
  (*pstat)->idtrame = 0;
  for (i = 0; i < MAXDESCRT; i++)
    (*pstat)->descr_trame[i].idtrame = 0;

  (*pstat)->dioctl = 0; /* variables pour autopsie via SKBD (LynxOS) */
  (*pstat)->dphioctl = 0;
  (*pstat)->dit = 0;
  (*pstat)->dphit = 0;

  for (i = 0; i < 65536; i++)
    (*pstat)->image[i] = 0;

  return 0; /* Tout s'est bien passe ! */
}

/*****************************/
/* Desinstallation du driver */
/*****************************/

#ifdef LYNXOS

/*** Fonction de desinstallation non implementee sous LynxOS ***/
// asdc_uninstall()
//		{
//		}

#elif LINUX

static void __exit cleanup_asdc(void) {
  int i;
  struct asdc_varg *varg;

  KPRINTF("\nDechargement du driver %s et des devices associes :\n", ASDC_NOM);

  /*
  ** Les operations sont effectuees dans l'ordre inverse
  ** de celui utilise a l'installation.
  */

  for (i = ASDC_MAX_CARTES - 1; i >= 0; i--) {
    if (asdc_p[i] != NULL) {

      /* asdc_p[i] pointe la structure asdc_statics associee a la carte */

      /* Traitement eventuel de la seconde voie */
      if (asdc_p[i]->voie2) {
        /* Traitement de la seconde voie */
        varg = asdc_p[i]->varg[1];

        /* Desenregistrement des gestionnaires d'interruptions */
        free_irq(varg->irq, asdc_p[i]);
        KPRINTF(" - IRQ [%d] liberee\n", i);

        /* Liberation memoire des elements de dst->wq_base */
        KPRINTF(" Liberation des listes de Wait_queue : ");
        wq_liberer(varg);
        if (varg->wq_base)
          KPRINTF("Echec !");
        else
          KPRINTF("OK");

        KPRINTF(" Liberation des adresses virtuelles PCI\n");
        iounmap(varg->bvba);

        /* Liberation du device (vraiment utile ???) */
        pci_dev_put(varg->h_drm);

        /* Liberation de  la memoire alouee au coupleur */
        KPRINTF(" Liberation de la memoire allouee au coupleur\n");
        vfree(varg->pbcf);
        vfree(varg);
      }

      /* Traitement de la premiere voie */
      varg = asdc_p[i]->varg[0];

      /* Desenregistrement des gestionnaires d'interruptions */
      free_irq(varg->irq, asdc_p[i]);
      KPRINTF(" - IRQ [%d] liberee\n", i);

      /* Liberation memoire des elements de dst->wq_base */
      KPRINTF(" Liberation des listes de Wait_queues : ");
      wq_liberer(varg);
      if (varg->wq_base)
        KPRINTF("Echec !");
      else
        KPRINTF("OK");

      KPRINTF(" Liberation des adresses virtuelles PCI\n");
      iounmap(varg->bvba);

      /* Liberation du device (vraiment utile ???) */
      pci_dev_put(varg->h_drm);

      /* Liberation de  la memoire alouee au coupleur */
      KPRINTF(" Liberation de la memoire allouee au coupleur\n");
      vfree(varg->pbcf);
      vfree(varg);

      /* Liberation du device associe au bridge TI (vraiment utile ???) */
      pci_dev_put(asdc_p[i]->pci_dev_texas);

      /* Liberation de  la memoire alouee a la carte */
      vfree(asdc_p[i]);
      KPRINTF(" Liberation de la memoire allouee a la carte\n");
      asdc_p[i] = NULL;
    }
  }

  /* Desenregistrement du driver aupres du systeme */
  KPRINTF(" Desenregistrement du driver : (ne peut echouer !)\n");
  unregister_chrdev(majeur, ASDC_NOM);
}

#endif /* LINUX */

/* -----------------------------------------
   --					  --
   --		    asdcopen 		  --
   --					  --
   ----------------------------------------- */

#ifdef LYNXOS

int asdcopen(struct asdc_statics *driver_statics, int device, struct file *f) {
  dprintf("asdcopen(mineur=%d, voie2=%d)\n", minor(f->dev),
          driver_statics->voie2);

  /* Controle de l'existence du coupleur */
  if (minor(f->dev) == 0)
    return OK;
  if ((minor(f->dev) == 1) && (driver_statics->voie2 == 1))
    return OK;

  dprintf(" Echec asdcopen : mineur=%d bivoie=%d\n", minor(f->dev),
          driver_statics->bivoie);
  pseterr(ENODEV);
  return SYSERR;
}

#elif LINUX

int asdcopen(struct inode *inode, struct file *f) {
  int i, mineur;

  mineur = MINOR(inode->i_rdev);
  // printk("ASDC : ouverture - mineur=%d   ", mineur);

  i = mineur / 2;
  if ((i < 0) || (i >= ASDC_MAX_CARTES)) {
    KPRINTF("ASDC : le mineur %d utilise est hors bornes !\n", mineur);
    return -EINVAL;
  }

  if (asdc_p[i] == NULL) {
    KPRINTF("ASDC : mineur %d : pas de carte associee !\n", mineur);
    return -EINVAL;
  }

  if ((mineur % 2) && (asdc_p[i]->voie2 == 0)) {
    KPRINTF("ASDC : mineur %d : pas de coupleur #2 associe !\n", mineur);
    return -EINVAL;
  }

  // printk(" ==> OK\n");
  return OK;
}

#endif /* LYNXOS - LINUX */

/* -----------------------------------------------
   --				                --
   --		abipmc2_close 	                --
   --			                        --
   ----------------------------------------------- */

/* ATTENTION : N'est appele qu'au DERNIER close() */

#ifdef LYNXOS
int asdcclose(struct asdc_statics *driver_statics, struct file *f) {
  dprintf("ASDC : Appel de close()\n");
  return OK;
}
#elif LINUX
int asdcclose(struct inode *inode, struct file *f) {
  dprintf("ASDC : Appel de close()\n");
  // printk("ASDC : fermeture\n");
  return OK;
}
#endif /* LYNXOS - LINUX */

/*****************************************************/
/* Fonctions non implementees :                      */
/*	asdc_read()                                  */
/*	asdc_write()                                 */
/*	asdc_select()                                */
/*****************************************************/

/* Traitement des interruptions (de l'un ou de l'autre des coupleurs) */

/* REMARQUE :   Compte tenu de l'incertitude sur le comportement  */
/*            du coupleur (le present code etant un contournement */
/*            d'un probleme d'IT rencontre quand chaque coupleur  */
/*            etait condidere comme un device independant),       */
/*            ioint_link() est appele 2 fois pour etre sur de     */
/*            l'avoir appele avec le bon argument.                */
/*              L'execution de asdcintr_coupleur() pour un        */
/*            coupleur n'ayant pas interrompu devrait etre rapide */
/*            et sans effet.                                      */

#ifdef LYNXOS
void asdcintr(struct asdc_statics *pstat)
#elif LINUX
#ifdef LINUX26
irqreturn_t asdcintr(int irq, void *dev_id /*, struct pt_regs *reg_ptr*/)
#else
void asdcintr(int irq, void *dev_id, struct pt_regs *reg_ptr)
#endif /* LINUX26 */
#endif /* LYNXOS - LINUX */
{

  struct asdc_varg *dst;
  int csr;

#ifdef LINUX
  struct asdc_statics *pstat = (struct asdc_statics *)dev_id;
#endif

  // kkprintf("IT : bivoie=%d\n", pstat->bivoie);

  /* Le premier coupleur 1553 est imperativement present    */
  /*		 ==> appel immediat de la fonction concernee */
  dst = pstat->varg[0];
  asdcintr_coupleur(dst);

  /* Passage du controle a un eventuel autre driver */
#ifdef LYNXOS
  ioint_link(dst->link_id);
#endif

  /* Existe-t-il un second coupleur 1553 sur la carte PMC ? */
  if (pstat->bivoie) {
    /* Coupleur double ==> appel de la fonction du second coupleur */
    dst = pstat->varg[1];
    asdcintr_coupleur(dst);

    /* Passage du controle a un eventuel autre driver */
#ifdef LYNXOS
    ioint_link(dst->link_id);
#endif
  }

#ifdef LINUX26
  /* Pour le moment, on ne tente pas de repondre autre chose ... */
  /* Ne devrait pas poser Pb puisque, si ligne partag�e, une IT  */
  /* peut en cacher une autre !                                  */
  return IRQ_HANDLED;
#else
  return;
#endif /* LINUX26 */
}

/* Traitement de l'interruption d'un coupleur donne */
static void asdcintr_coupleur(struct asdc_varg *dst) {
  int cnt, icnt, i, j, tmp;
  unsigned int ptr;
  int itbc; /* Indicateur arrivee ITs du mode BC */

#ifdef LINUX
  unsigned long long tsc;
#endif /* LINUX */

  // E(FIN_RAM - 1, 0x5A5A);   /* Marqueur debug pour VMETRO */

  dst->dit = 1;
  dst->dphit = 1;

  // kkprintf("{%d", dst->signal_number);
  zzprintf("--- IT ASDC ---   ");

  zzprintf("SN=%d  CSR=0x%04X  IQRSP=0x%04X  ATPTR=0x%04X\n",
           dst->signal_number, L(CSR), L(IQRSP), L(ATPTR));
#ifdef LYNXOS
  zzprintf("                bvba=0x%08X  h_drm=0x%08X  link_id=%d\n", dst->bvba,
           dst->h_drm, dst->link_id);
#elif LINUX
  zzprintf("                bvba=0x%08X  pci_dev=0x%08X\n", dst->bvba,
           dst->h_drm);
#endif

  /* Le coupleur courant est-il bien a l'origine de l'IT ? */
  if (!(L(CSR) & BIT_CSR_IT)) {
    zzprintf("IT non ASDC : CSR=0x%04X\n", L(CSR));
    // kkprintf(")");
    return; /* Si "non" : on arrete ! */
  }

  /* Attente du passage a zero de IQRSP */
  /* [Remarque : Ci-dessous, (tmp=L(IQRSP)) est bien un assignement, */
  /*             pas une comparaison !]                              */
  for (i = 0; (tmp = L(IQRSP)); i++) {
    if (i > 10) {
      kkprintf("ASDC : IT et IQRSP=0x%04X\n", tmp);
      kkprintf("ARRET ANTICIPE pour IQRSP!=0\n");
      return;
    }
    usec_sleep(1); /*** A Supprimer si possible ... ***/
  }

  /* Bloquage de la queue des comptes rendus d'IT */
  E(IQRSP, 0xFFFF);
  usec_sleep(2); /***####### Toujours OK ??? #######***/

  /* Position de la queue */
  cnt = L(IQCNT1);
  ptr = L(IQPTR1);

  if (cnt == 0) { /* L'IT ne provenait pas de l'ABI ! */
    zzprintf("ABI : IT recue et IQCNT1=0 ==> ???\n");

    /* Suppression source IT */
    E(CSR, (L(CSR) | BIT_CSR_IT));

    E(IQRSP, 1); /* Debloquage de la queue */

    // kkprintf("]");
    dst->dphit = -1;
    return;
  }

  if (cnt < 0) {
    kkprintf("ASDC : compte d'ITs = %d < 0 !!!\n", cnt);
    /* Au point ou on en est, on continue ... */

    /*###########################################
       Ici, il faudrait positionner un flag pour
       indiquer le probleme rencontre, puis
       reveiller toutes les applications en
       attente semaphores noyaux.
       ==> Pour cela, il faudrait fonction adaptee,
       qui pourrait etre appelee la ou on a besoin
       d'elle (par exemple dans ioctl(ASDCAVORTE))...
      ###########################################*/
  }

  /* Mise a jour de l'histogramme des nombres d'ITs dans la table */
  /* (pour permettre le reglage de la taille de la table des ITs) */
  dst->nbhistoit++;
  if (cnt >= ASDCTHISTOIT)
    dst->deborde++;
  else
    dst->histoit[cnt]++;

  itbc = 0;

  for (icnt = 0; icnt < cnt; icnt++) {
    int bus, cmd, commande, adresse, sous_adresse, direction;
    long *oreiller;
    unsigned int aadr, zd;
    unsigned int flx;
    int memofiltre, afiltre, filtre;
    int liste, erreur;

    /* zzprintf("QIT(%d) : 0x%X 0x%X 0x%X 0x%X\n",
            dst->signal_number, L(ptr),  L(ptr+1), L(ptr+2), L(ptr+3)); */
    /* printk("QIT(%d) : 0x%X 0x%X 0x%X 0x%X\n",
            dst->signal_number, L(ptr),  L(ptr+1), L(ptr+2), L(ptr+3)); */
    dst->dit = L(ptr); /* Code d'interruption : */
                       /* mot1 des 4 mots pointes par IQPTR1 */
    switch (L(ptr)) {
    case 0x1:
      zzprintf("ASDC : Timer Data Ready\n");
      // E(FIN_RAM - 1, 0x8888);   /* Marqueur debug pour VMETRO */
      /* On reveille la (les ???) tache(s) en attente */
      SRESET(&dst->semirig);
      break;

    case 0x2:
      zzprintf("ASDC : Monitor Buffer Swap\n");
      /* On indique qu'un buffer est disponible */
      dst->monok = 1;
      /* On reveille tache associee a espion sequentiel */
      /* qui est censee dormir sur semmon               */
      SRESET(&dst->semmon);
      break;

    case 0x3: /* Message received */
      zzprintf("ASDC : Filter Table Bit Set\n");

      /* Instrumentation pour mise en evidence instant */
      /* IT relativement au message 1553 :             */
      /*  ==> Incrementation du mot 0 de la mem. image */
      EI(0, LI(0, 0) + 1, 0);

      /* Ici, il faut rechercher l'adresse du RT(A,SA)  */
      /* dans l'ABI et reveiller les taches qui dorment */
      /* dessus                                         */
      if (L(ptr + 2))
        KPRINTF("ATTENTION : word2 = 0x%x\n", L(ptr + 2));
      /* Devrait entrainer un traitement de la   */
      /* 2� cmd identique a celui de la premiere */

      cmd = commande = L(ptr + 1);
      commande >>= 5;
      sous_adresse = commande & 0x1F;
      commande >>= 5;
      direction = commande & 1; /* 1 ==> T ;  0 ==> R */
      commande >>= 1;
      adresse = commande & 0x1F;

      /* Dans ce qui suit, on ne fait aucun test.    */
      /* On suppose que si une IT s'est produite les */
      /* structures de donnees ABI existent.         */

      aadr = L(L(ATPTR) + adresse);
      zd = LI(aadr + sous_adresse + direction * 32, 200);
      oreiller = &LI(zd + IRSEM, 201);

      dprintf("zd=0x%X   oreiller : 0x%X 0x%0X\n", zd, zd + IRSEM, oreiller);

      /* On fait le meme traitement (un reveil) pour */
      /* une emission comme pour une reception !     */

      dprintf("wake up : oreiller(0x%0X) a=%d sa=%d  sens=%d\n", oreiller,
              adresse, sous_adresse, direction);

      /* Reveil de la tache endormie sur l'adresse du */
      /* pointeur du (des) tampon(s) de donnees       */
      SRESET_IM(oreiller);

      /* Traitement eventuel des flux d'evenements RT */
      /* (c'est a dire des connexions CEVT)           */
#ifdef CEVT
#ifdef LYNXOS
      /* Une connexions CEVT est-elle liee a la voie ? */
      flx = LI(zd + IRCEVT, 202);
      if (flx) { /* Oui ! */

        /* ==> Transmission evenement au flux concerne */
        cevt_signaler(flx, CEVT_SDCABO, dst->signal_number, cmd & 0xFFFF, 0);
      }
#elif LINUX
      /* Une connexion CEVT est-elle liee a la voie ? */
      flx = LI(zd + IRCEVT, 202);
      if (flx) { /* Oui ! */

        /* Lecture du TSC
         * TODO: verifier que ceci a toujours un sens
         */
        __asm__ volatile("rdtsc" : "=A"(tsc));
        /* ==> Transmission evenement au flux concerne */
        cevt_signaler(flx, CEVT_SDCABO, dst->signal_number, cmd & 0xFFFF, 0);
      }
#endif /* LYNXOS - LINUX */
#endif /* CEVT	*/

      /* Des erreurs transitoires (multishot errors) */
      /* ont elle ete demandees ?                    */
      memofiltre = LI((zd + IRMEMOF), 0);
      /* Si aucune erreur transitoire en cours : */
      /* fin du traitement de l'IT !             */
      if (!(memofiltre & 0x80000000))
        break;

      /* Dans le cas contraire, le traitement */
      /* est effectue ci-dessous.             */

      liste = LI((zd + IRTLET), 0);
      afiltre = LI((zd + IRAFILTRE), 0);

      /* Erreur demandee ou non, arrivee ou non ? */
      erreur = liste & 1;
      if (erreur) {
        /* Une erreur etait demandee */
        int msg_err;

        /* Une erreur de parite doit s'etre produite */
        int nb_err = L(ERRPAR_NB) & 0x7FFF;
        int nb_err0 = (LI(zd + IRMEMOF, 0) >> 16) & 0x7FFF;
        /* si non : fin du traitement ! */
        if (nb_err == nb_err0)
          break;

        /* Derniere err. par. doit concerner cette voie */
        msg_err = L(ERRPAR_DC);
        /* si non : fin du traitement ! */
        if (msg_err != cmd)
          break;
      }

      /* Decalage de la liste */
      liste = (liste >> 1) & 0x7FFFFFFF;
      EI((zd + IRTLET), liste, 0);

      /* Preparation prochain message :            */
      /* (inutile de modifier le filtre si le mode */
      /*  erreur ou OK courant est conserve)       */

      /* Passage de OK a erreur ? */
      if ((!erreur) & (liste & 1)) { /* Programmation erreur sur prochain msg */
        /* (on force a 1 le bit IT au cas ou...) */
        E(afiltre, (LI((zd + IRMEMOE), 0) | 2) & 0xFFFF);
      }

      /* Passage d'erreur a OK ? */
      if (erreur & !(liste & 1)) { /* Suppression erreur sur prochain msg   */
        /* (on force a 1 le bit IT au cas ou...) */
        E(afiltre, (LI(zd + IRMEMOF, 0) | 2) & 0xFFFF);
      }

      /* Mise a jour de memofiltre si erreur */
      if (liste & 1) {
        int nb_err = L(ERRPAR_NB);
        memofiltre =
            (memofiltre & 0xFFFF) | 0x80000000 | ((nb_err << 16) & 0x7FFF0000);
        EI((zd + IRMEMOF), memofiltre, 0);
      }

      /* Si erreurs restent dans la sequence, */
      /* fin du traitement.                   */
      if (liste)
        break;

      /* Sinon, la sequence est terminee et le */
      /* flag "erreur multishot" peut etre     */
      /* supprime                              */
      EI((zd + IRMEMOF), 0, 0);

      break; /* Fin du "case 0x3" */

    case 0x4:
      KPRINTF("ASDC : Reserved\n");
      break;

    case 0x5: /* RT response error (programmation BC) */
    {
      unsigned int bc, err;

      bc = L(ptr + 1);  /* Bloc BC concerne */
      err = L(ptr + 2); /* Code erreur */

      /* Stockage du code erreur dans image[] */
      zzprintf("ASDC : BC Program Error\n");
      EIL(bc + IMERR, err, 500);

      /* Traitement d'un eventuel tampon en attente */
      if (dst->tf_attente) {
        /* Si erreur concerne le bloc BC du tampon */
        /* en attente, memorisation erreur dans ce */
        /* tampon                                  */
        if (dst->tf_attente == bc) {
          dst->tf_pta->tamp.err = err;
        }

        tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta, dst->tf_flux,
                                      dst->tf_zd, &dst->pbcfl);

        /* Si tmp, traitement de l'erreur */
        if (tmp) {
          bcflux_err(dst, dst->tf_attente, dst->tf_flux, dst->tf_zd, tmp, 0, 0);
        }

        /* Abaissement indicateur */
        dst->tf_attente = 0;
      }

    } break;

    case 0x6: /* KPRINTF("ASDC : BC Program End\n"); */
      dst->fin_bc = 1;

      /* Provisoire : Pour experimentation
        { // int s, ns;
          // ns = nanotime(&s);
          extern void heure(long int *phaut, long int *pbas);
          heure(&dst->dbg_s, &dst->dbg_ns);
          //dst->dbg_ns = ns;
          //dst->dbg_s = s;
        }
      */

      /* Traitement d'un eventuel tampon en attente */
      if (dst->tf_attente) {
        tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta, dst->tf_flux,
                                      dst->tf_zd, &dst->pbcfl);

        /* Si tmp, traitement de l'erreur */
        if (tmp) {
          bcflux_err(dst, dst->tf_attente, dst->tf_flux, dst->tf_zd, tmp, 0, 0);
        }

        /* Abaissement indicateur */
        dst->tf_attente = 0;
      }

      /* Indication "Trame achevee" */
      EIH(dst->idtrame + IMERR, 1, 1000);

      /* Reveil tache qui attend fin de la trame */
      SRESET(&dst->sem_finbc);

      /* Reveil des taches en attente sur des fluxs BC */
      /* (En parcourant la chaine des fluxs */
      for (j = dst->pflux; j; j = LIH(j + IMFSD, 203)) {
        SRESET_IM(&LI(j + IMSEMFLX, 204));
      }

      /* Faut-il enchainer une nouvelle trame ? */
      tmp = LIH(dst->idtrame + IMSUIV, 1000);
      EIH(dst->idtrame + IMSUIV, 0, 1000);
      if (tmp) { /* Oui */
        dst->idtrame = dst->descr_trame[tmp].idtrame;
        dst->fin_bc = 0;
        E(MFTVALH, ((dst->descr_trame[tmp].periode >> 16) & 0xFFFF));
        E(MFTVALL, ((dst->descr_trame[tmp].periode) & 0xFFFF));
        E(MFPERFR, ((dst->descr_trame[tmp].minpmaj) & 0xFFFF));
        E(MFTEXEH, ((dst->descr_trame[tmp].cycles >> 16) & 0xFFFF));
        E(MFTEXEL, ((dst->descr_trame[tmp].cycles) & 0xFFFF));
        EIH(tmp + IMERR, 0, 1000); /* exec. en cours */
        E(BCIPTR, dst->idtrame);   /* Lancement trame */
        SRESET(&dst->sem_gotbc);   /* Reveil taches ... */
      } else {                     /* Non */
        dst->idtrame = 0;
      }

      /* Reveil tache qui lit les trames BC */
      /***      wakeup((caddr_t) &asdc_ntbc[asdcunit]); ***/
      break;

    case 0x7:
      zzprintf("ASDC : BC Program Masked Status\n");
      break;

    case 0x8:
      zzprintf("ASDC : Interrupt Overflow\n");

      /* La trame est-elle terminee ? */
      /* (l'IT perdue pouvait indiquer une fin de trame) */
      if ((!L(BCCPTR)) && (L(BCLPTR))) { /* Indicateur "trame finie" */
        dst->fin_bc = 1;

        /* Reveil tache qui attend fin de la trame */
        SRESET(&dst->sem_finbc);
      }

      /* Parcours de la chaine des fluxs pour */
      /* remonter l'erreur (code 3)           */
      for (j = dst->pflux; j; j = LIH(j + IMFSD, 205)) {
        int zd;
        zd = LI(j + IMPZD, 206);
        bcflux_err(dst, j, j, zd, 3, 0, 0);

        /* Si trame terminee, reveil (eventuel) flux */
        if (dst->fin_bc == 1)
          SRESET_IM(&LI(j + IMSEMFLX, 207));
      }
      break;

    case 0x9:
      zzprintf("ASDC : MIM Interrupt\n");
      {
        unsigned short cmd;
        int code, rt, mot, cevt;

        cmd = L(ptr + 1);

        code = cmd & 0x1F;
        rt = (cmd >> 11) & 0x1F;

        /* Traitement eventuel des flux d'evenements RT */
        /* (c'est a dire des connexions CEVT)           */
#ifdef CEVT
        /* Une connexions CEVT est-elle liee a la CC ? */
        cevt = dst->cocor[rt][code].cevt;
#ifdef LYNXOS
        if (cevt)
#elif LINUX
        if (cevt)
#endif    /* LYNXOS - LINUX */
        { /* Oui ! */

          /* Recuperation du mot de donnee eventuel */
          switch (code) {
          case 2: /* Transmit last status */
            mot = L(L(LSWPTR) + rt);
            break;

          case 16: /* Transmit vector word */
            mot = L(L(TVWPTR) + rt);
            break;

          case 17: /* Synchronize (with data) */
            mot = L(L(LSYPTR) + rt);
            break;

          case 18: /* Transmit last command */
            mot = L(L(LCDPTR) + rt);
            break;

          case 19: /* Transmit Built In Test */
            mot = L(L(BITPTR) + rt);
            break;

          case 22: /* RESERVE */
            mot = L(L(RESPTR) + rt);
            break;

          default:
            mot = 0;
            break;
          }
#ifdef LYNXOS
          /* ==> Transmission evenement au cevt concerne */
          cevt_signaler(cevt, CEVT_SDCABO, dst->signal_number, cmd & 0xFFFF,
                        mot & 0xFFFF);
#elif LINUX
          /* Lecture du TSC
           * TODO: vérifier que c'est toujours à faire.
           */
          __asm__ volatile("rdtsc" : "=A"(tsc));

          /* ==> Transmission evenement au cevt concerne */
          cevt_signaler(cevt, CEVT_SDCABO, dst->signal_number, cmd & 0xFFFF,
                        mot & 0xFFFF);
#endif /* LYNXOS - LINUX */
        }
#endif /* CEVT */
      }
      break;

    case 0x10: /* Sequential monitor swap on message */
      zzprintf("ASDC : Monitor Buffer Swap on Message\n");
      break;

    case 0x11: /* BC bloc complete */
    {
      unsigned int bc, flux;
      unsigned int zd, ad;
      int nbmte, nbcte, nblte;
      int nbmts, nbcts, nblts;
      unsigned int evt;
      struct asdcbc_tf *t;
      int cm, tb, n;
      int x; /* Memorisation temp. erreurs internes */

#ifdef LYNXOS
      int *semexbc;
#elif LINUX
      wait_queue_head_t *semexbc;
#endif

      /* Traitement d'un eventuel tampon en attente */
      if (dst->tf_attente) {
        tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta, dst->tf_flux,
                                      dst->tf_zd, &dst->pbcfl);

        /* Si tmp, traitement de l'erreur */
        if (tmp) {
          bcflux_err(dst, dst->tf_attente, dst->tf_flux, dst->tf_zd, tmp, 0, 0);
        }

        /* Abaissement indicateur */
        dst->tf_attente = 0;
      }

      bc = L(ptr + 1);              /* Bloc BC */
      flux = LIL(bc + IMFLUX, 208); /* Flux eventuel */

      /* Traitement d'une eventuelle attente de */
      /* la fin d'execution du bloc BC courant  */
#ifdef LYNXOS
      semexbc = (int *)LI(bc + IMSEMBC, 231);
#elif LINUX
      semexbc = (wait_queue_head_t *)LI(bc + IMSEMBC, 231);
#endif
      if (semexbc) {
        SRESET(semexbc);
      }

      /* Pas de traitement si pas de flux */
      if (!flux)
        break;

      /* Parametres du flux */
      zd = LI(flux + IMPZD, 209);
      nbmte = LI(zd + IMNBMTE, 210);
      nbcte = LI(zd + IMNBCTE, 211);
      nblte = LI(zd + IMNBLTE, 212);
      nbmts = LI(zd + IMNBMTS, 213);
      nbcts = LI(zd + IMNBCTS, 214);
      nblts = LI(zd + IMNBLTS, 215);
      evt = LI(zd + IMEVT, 216);

      /* Pointeur bloc de donnees */
      ad = L(bc + 6);

      /* Comptage des executions */
      EI(bc + IMCPTR, LI(bc + IMCPTR, 217) + 1, 501);

      /******************************************/
      /* Traitement flux entree (bus --> appli) */
      /******************************************/

      /* Recuperation tampon dispo a remplir */
      x = asdc_fluxbctamp_nouveau(&dst->pbcfl, &dst->tf_pta);

      /* Si x, traitement de l'erreur */
      if (x) {
        bcflux_err(dst, bc, flux, zd, x, 0, 0);
      } else {
        /* Mise a jour du tampon */
        t = &((dst->tf_pta)->tamp);
        t->type = L(bc);
        t->err = LIL(bc + IMERR, 218);
        EIL(bc + IMERR, 0, 502); /* RAZ erreur 1553 */
        t->cmd1 = L(bc + 1);
        t->cmd2 = L(bc + 2);
        t->sts1 = L(bc + 3);
        t->sts2 = L(bc + 4);
        /* Pour test */ *((int *)(&t->date[0])) = LI(bc + IMCPTR, 219);
        /* Enregistrement eventuel des donnees */
        if ((t->type == RTBC) || (t->type == BCRT) || (t->type == BCRT_DI)) {
          n = t->cmd1 & 0x1F;
          if (!n)
            n = 32;
          for (i = 0; i < n; i++)
            t->d[i] = L(ad + i);
        } else if (t->type == COCO) { /* y-a-t-il une donnee ? */
          if ((t->cmd1 & 0x410) == 0x410) {
            t->d[0] = L(ad);
          }
        }

        /* Memorisation des elements lies au tampon */
        /* et leve indicateur "tampon en attente"   */
        dst->tf_attente = bc;
        dst->tf_zd = zd;
        dst->tf_flux = flux;
      }

      /******************************************/
      /* Traitement flux sortie (appli --> bus) */
      /******************************************/
      tb = L(bc) & 0xFF; /* Type du bloc */
      cm = L(bc + 1);    /* Mot de commande */
      /* Le filtrage par 0xFF elimine les bits */
      /* susceptibles d'avoir ete modifies par */
      /* le firmware                           */

      /* Si transferts sans donnees en sortie : fini */
      if ((tb == BCRT) || ((tb == COCO) && !(cm & 0x400)) || (tb == BCRT_DI)) {
        /* Recherche du tampon a vider */
        if (!nbcts) {
          /* Plus de tampons disponibles ! */
          /* Traitement erreur flux (code 1) */
          bcflux_err(dst, bc, flux, zd, 1, 0, 0);
        } else { /* Tampons encore disponibles */

          /* Mise a jour du tampon en mem. echange */
          t = &((struct asdcbc_tfch *)LI(zd + IMPPTS, 220))->tamp;

          if ((t->type != tb) || (t->cmd1 != cm)) {
            /* Incoh�rence entre trame et flux ! */
            /* Traitement erreur flux (code 2) */
            bcflux_err(dst, bc, flux, zd, 2, t->type, t->cmd1);

            // kkprintf("bc=0x%04X : Incoherence entre trame et donnees du flux
            // S !\n",
            //              bc);
            // kkprintf("   IT -   p : 0x%X\n" , LI(zd+IMPPTS, 221));
            // kkprintf("   tb=%X t->type=%X   cmd=%X t->cmd1=%X\n",
            //                       tb, t->type, L(bc+1), t->cmd1);

          } else {            /* Copie des donnees dans le coupleur */
            if (tb == COCO) { /* Commande codee */
              E(ad, t->d[0]);
            } else { /* Transfert BC-RT */
              n = t->cmd1 & 0x1F;
              if (!n)
                n = 32;
              for (i = 0; i < n; i++) {
                E(ad + i, t->d[i]);
              }
            }
          }

          /* Suppression du tampon lu */
          x = asdc_fluxbctamp_retirer(
              (struct asdcbc_tfch **)&LI(zd + IMPPTS, 222), &dst->pbcfl);
          nbcts--;
          EI(zd + IMNBCTS, nbcts, 503);

          /* Si plus de tampons, pointeur */
          /* dernier tampon = NULL        */
          if (nbcts == 0)
            EI(zd + IMPDTS, (long)NULL, 504);

          /* Si x, traitement de l'erreur */
          if (x) {
            bcflux_err(dst, bc, flux, zd, x, 0, 0);
          }
        }
      }

      /* Reveil eventuel de la tache utilisateur */
      j = 0;
      if (nbcte > nblte)
        j |= FLX_LIME;
      if (nbcts < nblts)
        j |= FLX_LIMS;
      if ((j >> 8) & evt)
        SRESET_IM(&LI(flux + IMSEMFLX, 223));
    } break;

    case 0x20: /* RT block Xfer complete */
    case 0x21: /* RT block Xfer no data */
    case 0x41: /* General interrupt */
      KPRINTF("ASDC : Code IT %d non traite !\n", L(ptr));
      break;

    default:
      KPRINTF("ASDC : Code IT %d inconnu !\n", L(ptr));
    }
    ptr += 4;
  }

  zzprintf("ASDC/IT2\n");

  /* Reveil tache qui attend liste des ITs */
  /* if (itbc) wakeup((caddr_t) &asdc_n_bc_it[asdcunit]); */

  /* Suppression source IT */
  E(CSR, L(CSR) | BIT_CSR_IT);

  E(IQRSP, 1); /* Debloquage de la queue */

  // KPRINTF("}");

  /* Pour tentative de debug en cas de crash dans KDB */
  dst->dphit = -1;
}

/***********************************************************
 *   Fonctions de gestion des chaines de tampons flux BC   *
 ***********************************************************/

/* Ajout d'un tampon isole a la fin de la chaine des tampons actifs
 * d'un flux entrant (sens 1553 --> application)
 *
 *	p = pointe le tampon (qui doit etre isole) a ajouter
 *	flux = adresse du flux concerne
 *	zd = adresse de la zone des donnees du flux concerne
 *     dispo = pointeur premier tampon de la chaine des tampons disponibles
 *
 *	Renvoi 0 si OK, autre chose en cas d'erreur
 *	Si erreur, un message est affiche sur la console de boot
 */
int asdc_fluxbctamp_ajouter(struct asdc_varg *dst, struct asdcbc_tfch *p,
                            int flux, int zd, struct asdcbc_tfch **pdispo) {
  int nbmte, nbcte;
  struct asdcbc_tfch **pdst, **psrc, *q;

  /* Pointeur vers le tampon a accrocher */
  if (p == NULL) {
    KPRINTF("asdc_fluxbctamp_ajouter : Tampon isole inexistant !!!\n");
    return 0x1001;
  }

  nbmte = LI(zd + IMNBMTE, 224);
  nbcte = LI(zd + IMNBCTE, 225);
  pdst = (struct asdcbc_tfch **)&(LI(zd + IMPDTE, 226));
  psrc = (struct asdcbc_tfch **)&(LI(zd + IMPPTE, 227));

  /* Est-il possible d'ajouter le tampon a la liste ? */
  if (nbmte == nbcte) {
    /* Liste complete ==> on ecrase un vieux tampon */

    /* Pointeur vers le premier tampon de la FIFO */
    q = *psrc;
    if (q == NULL) {
      KPRINTF("asdc_fluxbctamp_ajouter : Liste est vide !!!\n");
      return 0x1002;
    }

    /* Pointeur vers le second tampon de la FIFO (c'est ce tampon qui va */
    /* etre echange avec le tampon en attente                            */
    q = q->s;
    if (q == NULL) {
      KPRINTF("asdc_fluxbctamp_ajouter : ");
      KPRINTF("Liste n'a qu'un seul element !!!\n");
      return 0x1003;
    }

    /* Echange des tampons */
    p->s = q->s;
    (*psrc)->s = p;

    /* Marquage du premier tampon de la liste source comme "tampon perdu" */
    (*psrc)->tamp.err = 0xFFFF;

    /* Retour du tampon dispo a la liste */
    q->s = *pdispo;
    *pdispo = q;
  } else {
    /* Ajout du tampon a la liste des tampons utilises */
    p->s = NULL;
    if (*pdst != NULL)
      (*pdst)->s = p;
    *pdst = p;

    /* Si premier tampon, initialiser aussi pointeur vers debut FIFO */
    if (nbcte == 0)
      *psrc = p;

    /* Mise a jour du nombre des tampons */
    EI(zd + IMNBCTE, nbcte + 1, 505);
  }

  return 0;
}

/* Extraction d'un tampon d'une chaine de tampons disponibles
 *
 *	src = pointe une liste de tampons disponibles
 *	t = pointe le tampon extrait de la liste
 *
 *	Renvoi 0 si OK, 1 en cas d'erreur
 *	Si erreur, un message est affiche sur la console de boot
 */
int asdc_fluxbctamp_nouveau(struct asdcbc_tfch **psrc,
                            struct asdcbc_tfch **pt) {
  struct asdcbc_tfch *p;

  /* Pointeur vers le tampon a deplacer */
  p = *psrc;
  if (p == NULL) {
    kkprintf("asdc_fluxbctamp_nouveau : Plus de tampons disponibles !!!\n");
    return 0x1041;
  }

  /* Retrait du tampon de la liste des tampons disponibles */
  *psrc = p->s;

  /* Isolement du tampon et passage de son adresse */
  p->s = NULL;
  *pt = p;

  return 0;
}

/* Retrait du premier tampon d'une chaine de tampons actifs
 *
 *	src = pointe le premier tampon de la chaine de tampons actifs
 *	dst = pointe la liste des tampons disponibles
 *
 *	Renvoi 0 si OK, 1 en cas d'erreur
 *	Si erreur, un message est affiche sur la console de boot
 */
int asdc_fluxbctamp_retirer(struct asdcbc_tfch **psrc,
                            struct asdcbc_tfch **pdst) {
  struct asdcbc_tfch *p;

  /* Pointeur vers le tampon a deplacer */
  p = *psrc;
  if (p == NULL) {
    KPRINTF("asdc_fluxbctamp_retirer : Liste source est vide !!!\n");
    return 0x1011;
  }

  /* Retrait du tampon de la liste source */
  *psrc = p->s;

  /* Ajout du tampon a la liste des tampons inutilises */
  p->s = *pdst;
  *pdst = p;

  return 0;
}

/* Retrait de l'ensemble d'une chaine de tampons actifs
 *
 *	psrc = pointe le premier tampon de la chaine de tampons actifs
 *	dsrc = pointe le dernier tampon de la chaine de tampons actifs
 *	dst = pointe la liste des tampons disponibles
 *
 *	Renvoi 0 si OK, 1 en cas d'erreur
 *	Si erreur, un message est affiche sur la console de boot
 */
int asdc_fluxbctamp_supprimer(struct asdcbc_tfch **ppsrc,
                              struct asdcbc_tfch **pdsrc,
                              struct asdcbc_tfch **pdst) {
  struct asdcbc_tfch *p;

  // KPRINTF("flux tamp supprimer : psrc=0x%X  dsrc=0x%X  dst=0x%X\n",
  //          *ppsrc, *pdsrc, *pdst);

  if ((*ppsrc == NULL) && (*pdsrc == NULL))
    return 0; /* Rien a faire ! */

  if ((*pdsrc == NULL) || (*pdsrc == NULL)) {
    KPRINTF("asdc_fluxbctamp_supprimer : psrc ou dsrc nul,");
    KPRINTF(" mais pas les deux !\n");
    return 0x1031;
  }

  if ((*pdsrc)->s != 0) {
    KPRINTF("asdc_fluxbctamp_supprimer : La chaine a supprimer ");
    KPRINTF("n'est pas achevee !\n");
    return 0x1032;
  }

  /* Pointeur vers la liste des tampons a deplacer */
  p = *ppsrc;

  /* Insertion dans la liste des tampons inutilises */
  (*pdsrc)->s = *pdst;
  *pdst = *ppsrc;

  *ppsrc = *pdsrc = 0;

  return 0;
}

/*******************************************************
 *   Fonction de memorisation des erreurs de flux BC   *
 *******************************************************/

static void bcflux_err(struct asdc_varg *dst, uint16 bc, uint16 flux, uint16 zd,
                       uint16 code, uint16 arg1, uint16 arg2) {
  int i, j;

  /* Incrementation du compteur d'erreurs */
  i = LI(zd + IMNBERR, 228);
  j = i + 1;
  EI(zd + IMNBERR, j, 506);

  /* Si nombre maxi d'erreurs memorisables atteint : finir */
  if (j >= IMNBMAXERR)
    return;

  /* Adresse zone ou ecrire */
  i *= IMNBMPERR;
  i += zd + IMERRFLX; /* Ligne ajoutee le 24/9/2002                      */
                      /* Correction GROS BUG des v4.5 et anterieures !!! */

  /* Memorisation adresse bloc BC et code erreur */
  EI(i, ((bc << 16) & 0xFFFF0000) | (code & 0xFFFF), 507);

  /* Memorisation arguments "1" et "2" */
  EI(i + 1, ((arg2 << 16) & 0xFFFF0000) | (arg1 & 0xFFFF), 508);

  /* Memorisation compteur d'execution BC */
  EI(i + 2, LI(bc + IMCPTR, 229), 509);

  /* Memorisation numero de trame mineure */
  EI(i + 3, ((L(MFTCNTH) << 16) & 0xFFFF0000) | (L(MFTCNTL) & 0xFFFF), 510);

  /* Arret trame si demandee */
  if (LI(zd + IMEVT, 230) & FLX_ATSERR) {
    E(BCIPTR, 0xFFFF); /* Force l'arret de la trame */
  } /* cf ioctl(ASDCSTOPTBC)     */
}

/*
 *********************************************************************
 *
 * Points d'entree du driver non implementes
 *
 *********************************************************************
 */

#ifdef LYNXOS

int asdcuninstall(struct asdc_statics *dst) {
  kkprintf("\n---   ATTENTION : asdcuninstall non implementee !!!   ---\n");
  pseterr(EPERM);
  return SYSERR;
}

int asdcread(struct asdc_statics *dst, struct file *f, char *buf, int count) {
  kkprintf("\n---   ATTENTION : asdcread non implementee !!!   ---\n");
  pseterr(EPERM);
  return SYSERR;
}

int asdcwrite(struct asdc_statics *dst, struct file *f, char *buf, int count) {
  kkprintf("\n---   ATTENTION : asdcwrite non implementee !!!   ---\n");
  pseterr(EPERM);
  return SYSERR;
}

int asdcselect() {
  kkprintf("\n---   ATTENTION : asdcselect non implementee !!!   ---\n");
  pseterr(EPERM);
  return SYSERR;
}

#endif /* LYNXOS */

/*
 *********************************************************************
 *
 * Points d'entree du driver pour edition dynamique des liens
 *
 *********************************************************************
 */

#ifdef LYNXOS

struct dldd entry_points = {asdcopen,   asdcclose,   asdcread,      /* stub */
                            asdcwrite,                              /* stub */
                            asdcselect,                             /* stub */
                            asdcioctl,  asdcinstall, asdcuninstall, /* stub */
                            (char *)0};

#endif /* LYNXOS */

/******************************************************************************

   REMARQUES SUR LE FONCTIONNEMENT DU PILOTE :
   ===========================================

 ATTENTION : Les explications ci-dessous datent du CMBA (sous SunOS)
               ---> Elles ne sont sans doute plus valables sous LynxOS !!!


QUEUES ITE (Pour stocker mot de commande sur IT emission par abonne simule) :
-----------------------------------------------------------------------------

Emplissage queue :

   La queue peut contenir deux mots particuliers (Vide et Perdu) qui
   se distinguent des valeurs devant etre stockees (symbolisees ici
   par "Plein").


Premieres ecritures :

e-> Vide  <-l       Plein <-l       Plein <-l       Plein <-l       Plein <-l
    Vide        e-> Vide            Plein           Plein           Plein
    Vide            Vide        e-> Vide            Plein           Plein
    Vide            Vide            Vide        e-> Vide            Plein
    Vide            Vide            Vide            Vide        e-> Vide
    Vide            Vide            Vide            Vide            Vide



Lectures :

    Plein <-l       Vide            Vide            Vide            Vide
    Plein           Plein <-l       Vide            Vide            Vide
    Plein           Plein           Plein <-l       Vide            Vide
    Plein           Plein           Plein           Plein <-l       Vide
e-> Vide        e-> Vide        e-> Vide        e-> Vide        e-> Vide <-l
    Vide            Vide            Vide            Vide            Vide



Debordement :

e-> Vide            Plein           Plein           Plein           Plein
    Vide        e-> Vide            Plein           Plein           Plein
    Plein <-l       Plein <-l   e-> Perdu <-l       Plein           Plein
    Plein           Plein           Plein       e-> Perdu <-l       Plein
    Plein           Plein           Plein           Plein       e-> Perdu <-l
    Plein           Plein           Plein           Plein           Plein


Remarque : L'indicateur "Perdu" n'est pas indispensable : il pourrait etre
           remplace par "Plein", mais compliquerait la detection de la
           perte d'un tampon (necessiterait une comparaison d'indice et,
           peut-etre, une memorisation dans une variable dediee).


D'ou l'algorithme suivant :

  ECRITURE :
     flag_debordement = (*e == Perdu)
     *e = valeur a ecrire
     si (flag_debordement) { l++ }
     e++
     si (l == e) { *e = Perdu }

  LECTURE :
     Si (*l == Vide) ERREUR !
     valeur lue = *l		// Peut-etre soit "Plein", soit "Perdu"
     *l = Vide
     l++



DETECTION D'UN DEBIT INSUFFISANT POUR EMPLIR LES TAMPONS ABI :
--------------------------------------------------------------

La variable asdc_ite_vide[bus][adr][sa] du pilote est incrementee chaque fois
qu'un tampon est emis (l'incrementation est faite dans la fonction IT)

La variable equivalente asdc_ite_emplit[bus][adr][sa] de l'application est
incrementee chaque fois qu'un tampon est ecrit (l'incrementation est faite
dans l'application)

Si emplit - vide <= 0, il y a probleme !!!
(c'est a l'application de faire le calcul, en tenant compte de l'eventel
debordement du compteur).

Le contenu de vide (pour la voie concernee) est renvoye a chaque appel
de ioctl(ASDCITE).

 ---> Sous SUN-OS :
        L'experience a montre que cette methode etait insuffisante :
          Il peut arriver (acces disque ?) que les interruptions VME
        soient bloquees pendant plusieurs dizaines de ms !
          Dans un tel cas, les tampons ABI continuent a se vider mais
        la variable asdc_ite_vide reste inchangee : La perte de donnees
        n'est pas detectee par l'application.

       --> Solution : tester le "flag" du tampon courant de la voie dans
        la carte ABI.

       --> Bien qu'insuffisant et, sans doute, inutile, le mecanisme lie a
        asdc_ite_vide a ete maintenu pour le moment.

 ---> Sous LynxOS :
        Le portage reste a achever ...


******************************************************************************/
