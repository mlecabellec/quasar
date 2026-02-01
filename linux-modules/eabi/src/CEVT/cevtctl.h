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
 *                        Legere adaptation pour EMUSCI le 23/04/2013   *
 *              Modif. types pour compatibilite 32b/64b le 18/06/2014   *
 ************************************************************************/

#if !defined(_CEVTCTL_H)
#define _CEVTCTL_H

#ifdef KERNEL
#include <linux/types.h>
#else
#include <stdint.h>
#endif

// #ifndef _LINUX_IOCTL_H
// #include <linux/ioctl.h>
// #endif

/*** Structure a usage interne ***/
/* Structure tampon pour flux evenements */
struct cevt_tf {
  int type;   /* Type de l'evenement */
  int src;    /* Source de l'evenement (voie) */
  int bus;    /* Numero du bus (si 1553)  */
  int32_t d1; /* Donnee 1 (cmd 1553) */
  int32_t d2; /* Donnee 2 (donee CC eventuelle) */
  union {
    long date[2];           /* Datation (non encore utilise) */
    unsigned long long tsc; /* Pour Linux */
  };
};

/*** Structure a usage interne ***/
/* Structure de chainage des tampons pour flux_evt RT */
struct cevt_tfch {
  struct cevt_tf tamp; /* Tampon */
  struct cevt_tfch *s; /* Pointeur du suivant */
};

/*** Pour fonction ioctl(LTAMPON) : debug uniquement ***/
struct cevt_tampon {
  int numero;            /* Numero du CEVT */
  struct cevt_tfch tfch; /* Tampon */
};

/* Version du pilote CEVT */
struct cevtver {
  char nom[101];     /* Nom du pilote */
  char version[101]; /* Version du pilote */
  char date[101];    /* Date du pilote */
  int ver;           /* Numero de version */
  int rev;           /* Numero de revision */
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
  int type_evt;   /* Mode lect. (entree) - Nature evt (sortie)*/
  int numero;     /* Numero CEVT (entree) ou coupleur (sortie) */
  int32_t id_evt; /* Identification evenement */
  int bus;        /* Numero bus */
  int32_t donnee; /* Eventuelle donnee associee */
  int nbtamp;     /* Nombre de tampons encore a lire */
  union {
    long date[2];           /* Datation evt (format UNIX : 0:sec 1:nsec)*/
    unsigned long long tsc; /* Datation via TSC */
  };
};

/* Structure pour CEVT_SIGNALER */
struct cevt_signal {
  int numero; /* Numero du CEVT */
  int32_t d1; /* Premier mot de donnees */
  int32_t d2; /* Second mot de donnees */
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

/* Renvoi le nombre de CEVT instancies dans le driver */
#define CEVT_GETNOMBRE 0xCE0008

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
#define CEVT_DEBORD 7

/* Evenement special (pour appel cevt_signaler() */
#define CEVT_AVORTE 0xFFFFFFFF

// /* Pour export/import des fonctions de communication CEVT */
// struct cevt_export
// { int (*cevt_existence)(int);
//   int (*cevt_signaler)(int, int, int, int32_t, int32_t);
//   int (*cevt_signaler_date)(int, int, int, int32_t, int32_t, unsigned long
//   long);
// };

#endif /* !defined( _CEVTCTL_H ) */
