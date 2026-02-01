/************************************************************************
 *                                                                      *
 *     Pseudo-driver "Concentrateur d'EVenemenTs" pour ASDC et ETOR     *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *         Definition des I/F application                               *
 *                                                                      *
 *                                                                      *
 *                                         Y.Anonymized, le  9/09/2002   *
 *                                               modif. le 10/10/2002   *
 *         Ajout definition CEVT_TIMER : X. ************, le 13/01/2003   *
 *      Ajout definition CEVT_ABOSUPPR : X. ************, le  6/03/2003   *
 *                    Ajout definition CEVT_STOR : ???, le 22/05/2003   *
 *                   Adaptation a Linux : Anonymized, le 18/10/2006   *
 *                       Adaptation au TSC (sous Linux) le 25/10/2006   *
 ************************************************************************/

#if !defined(_CEVTCTL_H)
#define _CEVTCTL_H

#ifdef LYNXOS

#ifndef _IOCTL_
#include <sys/ioctl.h>
#endif

#elif LINUX

#ifndef _LINUX_IOCTL_H
#include <linux/ioctl.h>
#endif

#endif /* LYNXOS - LINUX */

/* Version du pilote CEVT */
struct cevtver {
  char nom[101];     /* Nom du pilote */
  char version[101]; /* Version du pilote */
  char date[101];    /* Date du pilote */

  int numero; /* Numero du CEVT */
};

/* Structure pour transmettre a application l'etat interne du pilote */
/* (pour debug seulement ...)                                        */
struct cevtetapil {
  int numero;               /* Numero du CEVT */
  int nb_tampons;           /* Nbre tampons flux evt alloues */
  int nb_tamp_dispos;       /* Nbre tampons flux evt inutilises */
  struct cevt_tfch *pbcfev; /* Pointeur vers table des tampons */
  struct cevt_tfch *ptv;    /* Pointeur vers premier tampon disponible */
  struct cevt_tfch *ppt;    /* Pointeur vers premier tampon plein */
  struct cevt_tfch *pdt;    /* Pointeur vers dernier tampon plein */
  int raz;                  /* Fanion raz du CEVT */
  int sel_data;             /* Fanion "data available" pour select */
};

/* Structure de lecture d'un flux d'evenements RT */
struct cevt_lire {
  int type_evt; /* Mode lect. (entree) - Nature evt (sortie)*/
  int id_evt;   /* Identification evenement */
  int donnee;   /* Eventuelle donnee associee */
  int nbtamp;   /* Nombre de tampons encore a lire */
  union {
    long date[2];           /* Datation evt (format UNIX : 0:sec 1:nsec)*/
    unsigned long long tsc; /* Datation via TSC */
  };
  int toto;
};

/* Structure pour CEVT_SIGNALER */
struct cevt_signal {
  long d1; /* Premier mot de donn�es */
  long d2; /* Second mot de donn�es */
};

/* Commandes ioctl() pour CEVT */

#define CEVT_INIT 0xCE0001

#define CEVT_LIRE 0xCE0002
#define CEVT_LTAMPON 0xCE0003

/* Fonction de lecture des indicateurs internes du pilote (pour debug) */
#define CEVT_LETAT 0xCE0004

/* Fonction de lecture de la version du pilote */
#define CEVT_LVER 0xCE0005

/* Fonction pour signaler un evenement logiciel au pilote */
#define CEVT_SIGNALER 0xCE0006

/* Fonction pour exporter certaines fonctions internes du pilote */
#define CEVT_EXPORTER 0xCE0007

/* Modes de lecture (CEVT_LIRE) */
#define _CEVT_RAZRAZ 1
#define _CEVT_RAZ 2
#define _CEVT_NONBLOQ 3
#define _CEVT_ATTENDRE 4

/* Nature des evenements (CEVT_LIRE et ) */
#define CEVT_SDCABO 1
#define CEVT_ETOR 2
#define CEVT_EXT 3
#define CEVT_TIMER 4
#define CEVT_STOR 5
#define CEVT_SDCABOSUPPR 6
#define CEVT_DEBORD 0xFFFFFFFF

/* Evenement special (pour appel cevt_signaler() */
#define CEVT_AVORTE 0xFFFFFFFF

/* ifonction cevt export�e vers les modules utilisateurs */
extern int cevt_existence(int);
extern int cevt_signaler(int, int, int, long, long);
extern int cevt_signaler_date(void);

#endif /* !defined( _CEVTCTL_H ) */
