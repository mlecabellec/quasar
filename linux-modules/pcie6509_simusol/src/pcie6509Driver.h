/************************************************************************
 *                                                                      *
 *      Driver pour carte NI PCIe-6509 (96 E/S TOR)                     *
 *     ---------------------------------------------                    *
 *                                                                      *
 * pcie6509Driver.h : Declarations internes au driver.                  *
 *                                                                      *
 *                                 ************ - ************ - ********
 *Anonymized
 **
 ************************************************************************/

/*
   Quand    Qui   Quoi
----------  ----  -------------------------------------------------------------
 3/12/2013   YG  Version initiale
24/03/2012   YG  Ajout de la variable globale "logiquePositive".

*/

#ifndef _PCIE6509DRIVER_H_
#define _PCIE6509DRIVER_H_

/* Un numero de mineur inferieur a BASE_MINEUR_CEVT correspond a un    */
/* device ABI (coupleur 1553), un numero de mineur superieur a         */
/* BASE_MINEUR_CEVT correspond a un pseudo device CEVT (concentrateur  */
/* d'EVenemenTs).                                                      */
#define BASE_MINEUR_CEVT 100

typedef struct xCevt_s xCevt_t;

// #include <sys/iofunc.h>
// #include <sys/dispatch.h>
//
// /// YGYGYG
// #include <sys/neutrino.h>
// #include <sys/mman.h>
// #include <hw/pci.h>
// #include <devctl.h>

/*  Device et vendor ID pour acces PCI a  */
/*  la carte N.I. Pcie6509 :              */
#define PCIE6509_VENDOR_ID 0x1093
#define PCIE6509_DEVICE_ID 0xC4C4
#define PCIE6509_BAR PCI_BASE_ADDRESS_0
#define PCIE6509_BARSIZE 0x80000 // 512 k

#define TAILLE_NOM_MAXI 20
#define SUFFIXE_TIT "_threadIT"
#define SUFFIXE_HIT "_handlerIT"

/* Priorites des messages */
#define DRV_FATAL 5
#define DRV_ERROR 4
#define DRV_WARNING 3
#define DRV_INFO 2
#define DRV_DEBUG 1

/* Destination des messages */
#define SYSLOG 1
#define STDERR 2

/* Registres de la carte PCIE6509 */

/* Registres de validation des interruptions */
#define GlobalInterruptEnable_Lo 0x20078
#define GlobalInterruptEnable_Hi 0x40078

/* Registres d'entree/sortie */

#define DIO_INPUT_P0_P3 0x20530
#define DIO_INPUT_P4_P5 0x200E0
#define DIO_INPUT_P6_P7 0x400E0
#define DIO_INPUT_P8_P11 0x40530

#define DIO_OUTPUT_P0_P3 0x204B0
#define DIO_OUTPUT_P4_P5 0x200E0
#define DIO_OUTPUT_P6_P7 0x400E0
#define DIO_OUTPUT_P8_P11 0x404B0

// #define DIO_OUTPUT_P3                0x204B0
// #define DIO_OUTPUT_P2                0x204B1
// #define DIO_OUTPUT_P1                0x204B2
// #define DIO_OUTPUT_P0                0x204B3
//
// #define DIO_OUTPUT_P5                0x200E0
// #define DIO_OUTPUT_P4                0x200E1
//
// #define DIO_OUTPUT_P7                0x400E0
// #define DIO_OUTPUT_P6                0x400E1
//
// #define DIO_OUTPUT_P11               0x404B0
// #define DIO_OUTPUT_P10               0x404B1
// #define DIO_OUTPUT_P9                0x404B2
// #define DIO_OUTPUT_P8                0x404B3

/* Registre de directions des E/S */
/* 0 ==> Entree, 1 ==> Sortie     */

#define DIO_DIRECTION_P0_P3 0x204B4
#define DIO_DIRECTION_P4_P5 0x200A4
#define DIO_DIRECTION_P6_P7 0x400A4
#define DIO_DIRECTION_P8_P11 0x404B4

/* PFI Select output (ports 4 a 7 seulement) */
#define PFI_SELOUT_P4_0 0x200BA
#define PFI_SELOUT_P4_1 0x200BB
#define PFI_SELOUT_P4_2 0x200BC
#define PFI_SELOUT_P4_3 0x200BD
#define PFI_SELOUT_P4_4 0x200BE
#define PFI_SELOUT_P4_5 0x200BF
#define PFI_SELOUT_P4_6 0x200C0
#define PFI_SELOUT_P4_7 0x200C1
#define PFI_SELOUT_P5_0 0x200C2
#define PFI_SELOUT_P5_1 0x200C3
#define PFI_SELOUT_P5_2 0x200C4
#define PFI_SELOUT_P5_3 0x200C5
#define PFI_SELOUT_P5_4 0x200C6
#define PFI_SELOUT_P5_5 0x200C7
#define PFI_SELOUT_P5_6 0x200C8
#define PFI_SELOUT_P5_7 0x200C9
#define PFI_SELOUT_P6_0 0x400BA
#define PFI_SELOUT_P6_1 0x400BB
#define PFI_SELOUT_P6_2 0x400BC
#define PFI_SELOUT_P6_3 0x400BD
#define PFI_SELOUT_P6_4 0x400BE
#define PFI_SELOUT_P6_5 0x400BF
#define PFI_SELOUT_P6_6 0x400C0
#define PFI_SELOUT_P6_7 0x400C1
#define PFI_SELOUT_P7_0 0x400C2
#define PFI_SELOUT_P7_1 0x400C3
#define PFI_SELOUT_P7_2 0x400C4
#define PFI_SELOUT_P7_3 0x400C5
#define PFI_SELOUT_P7_4 0x400C6
#define PFI_SELOUT_P7_5 0x400C7
#define PFI_SELOUT_P7_6 0x400C8
#define PFI_SELOUT_P7_7 0x400C9

/* Valeur a ecrire dans les registres ci-dessus pour valider les */
/* sorties des voies concernees (a faire une seule fois au boot) */
#define PFI_OUTPUT_SELECT 0x10

/* Positions des ports dans les registres */
#define PPORT0 0
#define PPORT1 8
#define PPORT2 16
#define PPORT3 24
#define PPORT4 0
#define PPORT5 8
#define PPORT6 0
#define PPORT7 8
#define PPORT8 0
#define PPORT9 8
#define PPORT10 16
#define PPORT11 24

/* Masques des ports dans les registres */
#define MPORT0 0x000000FF
#define MPORT1 0x0000FF00
#define MPORT2 0x00FF0000
#define MPORT3 0xFF000000
#define MPORT4 0x000000FF
#define MPORT5 0x0000FF00
#define MPORT6 0x000000FF
#define MPORT7 0x0000FF00
#define MPORT8 0x000000FF
#define MPORT9 0x0000FF00
#define MPORT10 0x00FF0000
#define MPORT11 0xFF000000

/* Pour chaque port, liste des autres ports associes                       */
/* dans les registres materiels :                                          */
/*    - 4 bits de poids faibles : Nombre des autres ports                  */
/*    - Quartets successifs de poids croissants : numeros des autres ports */
#define LPORT0 0x00003213
#define LPORT1 0x00003203
#define LPORT2 0x00003103
#define LPORT3 0x00002103
#define LPORT4 0x00000051
#define LPORT5 0x00000041
#define LPORT6 0x00000071
#define LPORT7 0x00000061
#define LPORT8 0x0000BA93
#define LPORT9 0x0000BA83
#define LPORT10 0x0000B983
#define LPORT11 0x0000A983

typedef struct pcie6509Statics_s pcie6509Statics_t;

/* Cette structure regroupe les donnees liees a une carte PCIE6509 */
struct pcie6509Statics_s {

  void *h_drm;
  void *bvba;    /* Board Virtual Base Address */
  long bpba;     /* Board Physical Base Address */
  int pcie;      /* Non nul si bus est "PCI Express" */
  int msiStatus; /* Si 0, utilisation interruption MSI */
  int numero;    /* Numero de la carte (indice dans pAllStat[] */

  //     int  irq;

  spinlock_t lock; /* Protection contre les acces concurrents */

  /* Tables pour memoriser l'etat courant de chaque voie */
  /* L'indice correspond au numero du port */
  uint8_t dir[12]; /* Direction : 0 ==> Entree, 1 ==> Sortie */
  uint8_t bit[12]; /* Bit courant en sortie */
};

/* Variables globales */
// extern resmgr_connect_funcs_t connect_funcs;
// extern resmgr_io_funcs_t io_funcs;
// extern resmgr_io_funcs_t cevt_funcs;
// extern iofunc_mount_t mountpoint;

extern pcie6509Statics_t **pAllStat;
extern int pcie6509Nbre;

extern char *base_nom;

/* Les tables ci-dessous sont indexees par numeros de port */
extern uint32_t dadr[]; /* Adresses relatives des registres de direction */
extern uint32_t iadr[]; /* Adresses relatives registres d'entree (lecture) */
extern uint32_t oadr[]; /* Adresses relatives registres de sortie (ecriture) */
extern int decalage[];  /* Decalage des ports dans les registres */
extern int masque[];    /* Masques des ports dans les registres */
extern int listep[];    /* Association des ports dans les registres */

extern int logiquePositive; /* Type de logique a appliquer aux E/S */

extern long pcie6509UnlockedIoctl(struct file *fp, unsigned int cmd,
                                  unsigned long arg);

extern void initPrintLog(int destination, int niveau);

extern void printLog(const int msglevel, const char *format, ...);

/* Lecture d'un port en entree */
extern uint8_t lirePortEntree(pcie6509Statics_t *pstat, int port);

/* Ecriture d'un port en sortie */
extern void ecrirePortSortie(pcie6509Statics_t *pstat, int port,
                             uint8_t valeur);

/* Ecriture des directions des bits d'un port */
extern void ecrirePortDirection(pcie6509Statics_t *pstat, int port,
                                uint8_t valeur);

/* Fonctions elementaires appelees par io_devctl() */
// int do_abi_emem(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_lmem(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_firmware_init(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_lpar(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_epar(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_raz(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_go(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abi_alloc(struct file *fp, unsigned int cmd, unsigned long arg);
// int do_abo_wait(struct file *fp, unsigned int cmd, unsigned long arg);

#endif /* _PCIE6509DRIVER_H_ */
