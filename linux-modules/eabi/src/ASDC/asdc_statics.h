
/*

 DRIVER ASDC POUR EMUABI, PLACE AU-DESSUS DU DRIVER VFX70

 statics : Definition des donnees statiques liees a chaque device

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
23/02/2013  YG   Version initiale.
 8/04/2013  YG   Adaptation a ASDC du squelette generique initial.
 9/04/2013  YG   Taille table des coco doublee pour bus 2
 2/05/2013  YG   Ajout flag "jamais_initialise".
12/06/2014  YG   Ajout d'un pool de tampons pour memoriser les dernieres
                 donnees emises par chaque sous-adresse en mode transmit.
                 (C'est un besoin de l'application SESAME).
18/06/2014  YG   Modification types pour compatibilite 32b/64b 

*/



#ifndef ASDC_STATICS_H_
#define ASDC_STATICS_H_


#include <linux/wait.h>
#include <linux/types.h>


#include "asdcwq.h"
#include "asdc.h"


#define MAX_EMUABI    10

#define CEVT        /* Valide la compilation de l'interface avec le CEVT */

#ifndef DEVICE_NAME
#define DEVICE_NAME   "EASDC"
#endif


/* Structure pour definir un bloc libre dans la  memoire du coupleur */
/* (cf. table memo[] dans asdc_statics)                              */
struct smemo { unsigned short int a;
               unsigned short int l;
               short int s;
               short int p;
             };


struct descr_trame_s { int idtrame;
                       unsigned int periode;
                       int minpmaj;     /* Nb cycle min. par cycle maj. */
                       unsigned int cycles;
                     };

/* Pool des tampons utilises pour memoriser les donnees ecrites      */
/* par l'application dans les sous-adresses en emission :            */
/*                                                                   */
/* Le nombre de tampons necessaire peut être estime ainsi :          */
/*    2 bus x 32 adresses x 32 sous-adresses x N tampons x M cartes  */
/*                                                                   */
/* Si M = 1 (une seule carte)                                        */
/* et N = 5 (5 tampons en moyenne par sous-adresse en emission, ce   */
/*           qui est tres largement superieur a la realite sachant   */
/*           qu'en general, seuls 2 tampons sont utilises et que     */
/*           la plupart des sous-adresses en emission ne sont pas    */
/*           utilisees)                                              */
/* on obtient : Nombre de tampons = 10240                            */
/* Si 80 octets par tampon, l'alloc. globale est de 819200 octets,   */
/* qui est tres raisonnable compte tenu de la memoire disponible sur */
/* les systemes actuels.                                             */
#define TAILLEPOOL 10240

/* Pour la simulation SESAME : Tampon elementaire de la FIFO soft utilisee */
/* pour memoriser les tampons ecrits.                                      */
#define TCARGO 36 /* mot de cmd + 32 data + alignement sur 64 bits           */
                  /* ==> 72 octets par cargo et 80 octets par tampon en 64b. */
typedef struct tampon_s tampon_t;
struct tampon_s {
    tampon_t * s;           /* Tampon suivant dans la chaine */
    uint16_t cargo[TCARGO]; /* Zone utile */
};


/* Pour la simulation SESAME : structure d'ancrage d'une FIFO pour memoriser */
/* les tampons ecrits et le dernier tampon emis.                             */
typedef struct fifoTampons_s fifoTampons_t;
struct fifoTampons_s {
    tampon_t * demis;       /* dernier tampon emis */
    tampon_t * p;           /* premier tampon dans la chaine */
    tampon_t * d;           /* dernier tampon dans la chaine */
    uint64_t nbt;           /* Nombre de tampons dans la FIFO */
    spinlock_t lock;        /* Pour proteger l'acces a la FIFO */
};
/* Remarque : Les champs p et d ci-dessus pointent une chaine de tampons */
/*            dans le cas d'une sous-adresse en mode SYNC. Si la         */
/*            sous-adresse est en mode ASYNC, p pointe une copie du      */
/*            dernier tampon ecrit par ASDCLEC (mais pas encore emis) et */
/*            d est inutilise et toujours nul.                           */
/* Remarque : Compte tenu du fonctionnement du 1553, l'utilisation */
/*            d'un spinlock par FIFO est sans doute aussi luxueux. */
/*            Un seul spinlock par coupleur 1553 devrait etre      */
/*            suffisant.                                           */

  
/* Structure des donnees statiques associees a un coupleur EMUABI */
struct asdc_varg {
    int numVfx;                    /* Numero de la carte VFX70 */
    int numero;                    /* Numero du coupleur ASDC */
    int open_dev;                  /* Compteur des ouvertures */

    void * pcibar0;         /* Adresse de base PCI_BAR0 de la VFX70 */
    void * pcibar1;         /* Adresse de base PCI_BAR1 de la VFX70 */
    void * pcibar2;         /* Adresse de base PCI_BAR2 de la VFX70 */

    int jamais_initialise;  /* Indicateur pour eviter crash au reset  */
                            /* coupleur+driver non encore initialise. */


   int dioctl;          /* Debug : Dernier ioctl appele */
   int dphioctl;        /* Debug : Phase ioctl en cours */
   int dit;             /* Debug : Derniere IT appelee */
   int dphit;           /* Debug : Phase IT en cours */


   int  bus;
   int  device_number;                  /* Non utilise pour le moment */
   char *int_handler;                   /* not used at present */
   int  status;
   int  base_address;
   int  int_vector;
   int  isr_installed;
   int  lock;
   int  Vendor_ID;
   int  Device_ID;
   int  signal_number;          /* Pour messages de debug uniquement ... */

   int link_id;                 /* Pour chainage des IT */

   int vflash;                  /* Indicateur chargement memoire flash */


   struct asdcparam asdcdef; /* Parametres "par defaut" */
   int nbopen;                  /* Nombre de processus ayant */
                                /* "ouvert" le driver        */


   int nombre_it;   /* Nombre de processus ayant */
                         /* attente IT en cours       */

     /*** Ci-dessous : sans doute devenu inutile ... ***/
   int asdc_ici; /* Indicateur presence carte ABI :               */
                 /*    Si VMV : prend valeur PRESENTE, ABSENTE ou IGNORE */
                 /*    sinon, 0 pour carte absente et 1 pour presente    */


   int monok;                   /* Indicateur pour espion sequentiel */
   wait_queue_head_t semmon;    /* Wait queue pour espion sequentiel */

   /* Pour creation de trame */
   unsigned short int bloc_suivant;     /* Memorisation adresse pointeurs de */
                                        /* chainage des blocs BC             */



   wait_queue_head_t sem_finbc;  /* Wait queue pour attendre fin trame BC */
   wait_queue_head_t sem_gotbc;  /* Wait queue pour attendre debut trame BC */
   int fin_bc;          /* Indicateur trame achevee (mode BC) */
   int stocke_bc;       /* Validation acquisition (bufferisation) BC */
   wait_queue_head_t sem_exbc;      /* Wait queue pour "gerant generique" */


//      /* Stockage derniere commande codee recue par abonne :  */
//      /*   Deuxieme indice = bus, troisieme indice = adresse 1553 */
//   short int cocorec[2][32];

   /* Pour traitement des commandes codees */
   /*   - Indice 1 : Numero du bus                                  */
   /*   - Indice 2 : Numero du RT                                   */
   /*   - Indice 3 : Numero de la CC (les 5 bits du code seulement) */
   struct scoco cocor[2][32][COCO_MAX + 1];

   /* Table des blocs memoires libres (pour allocation memoire) */
   struct smemo memo[TMEMO];
   int ipremier;  /* Indice (dans memo) plus petit bloc disponible */
   int idernier;  /* Indice (dans memo) plus grand bloc disponible */

   /* Spinlock de protection des fonctions d'allocation memoire */
   spinlock_t alloc_lock;
   
   /* Spinlock de protection des connexions/deconnexions aux CEVTs */
   spinlock_t cevt_lock;
   
   /* Pour sections critiques des messages de debug */
   spinlock_t lock_debug;

   /* Semaphore pour insertion retard ... (PROVISOIRE (?) ...) */
   int semretard;

   /* Wait queue pour attendre la disponibilite de l'heure IRIG */
   wait_queue_head_t semirig;

   /* Image de la memoire d'echange du coupleur */
   long image[65536];
   /* 32 bits sont utilises pour la partie "purement data" decrite  */
   /* dans la doc. sur la memoire image, mais il faut aussi pouvoir */
   /* enregistrer un pointeur sur une wait_queue, ce qui necessite  */
   /* un long quand l'architecture est 64 bits.                     */

   /* Semaphore de protection des fonctions d'allocation des fluxs */
   int mutexflux;

   /* Nombre total de tampons pour flux BC et pointeur vers ces tampons */
   int nombre_tampons_flux;
   struct asdcbc_tfch *pbcf;    /* Pointe la base du tableau qui contient */
                                /* tous les tampons                       */

   /* Nombre de tampons restants et pointeur vers le premier de ces tampons */
   int nb_tamp_flux_dispos;
   struct asdcbc_tfch *pbcfl;   /* Pointe le premier element de la chaine */
                                /* des tampons libres                     */

   unsigned int pflux, dflux;   /* Adresse des premiers et derniers */
                                /* flux definis                     */

   int idtrame;                 /* Adresse trame en cours execution */



   int raz;             /* Indicateur de RAZ coupleur/driver en cours */

   int nbhistoit;               /* Nbre d'ITs accumulees dans l'histogramme */
   int histoit[ASDCTHISTOIT];   /* Histogramme de l'usage table des ITs */
   int deborde;                 /* Indicateur de debordement histogramme */

   /* Pour memoriser un tampon flux BC isole en attendant une eventuelle */
   /* interruption 0x5 indiquant une erreur                              */
   unsigned int tf_attente;     /* Indicateur tampon isole et memorisation */
                                /* du bloc BC concerne                     */
   unsigned int tf_zd;        /* Memorise adr. zone donnee du flux concerne */
   unsigned int tf_flux;      /* Memorise flux concerne */
   struct asdcbc_tfch *tf_pta;  /* Pointe tampon isole */


   /* Descripteurs de trame (pour enchainements) */
#define MAXDESCRT       100
   struct descr_trame_s descr_trame[MAXDESCRT];

   /* Provisoire : pour essai ... */
   int dbg_ns;
   int dbg_s;

   int nombre_wait_queues;      /* Nombre de structures "wait queues" */
                                /* Linux allouees                     */





  /* Les variables ci-dessous ne sont utilisées que sous Linux */

  unsigned int irq;     /* Pour memo ligne d'IT sous Linux */

  /* Pointeurs et lock pour gestion des listes de "Wait queues" */
  struct wq_liste *wq_base;
  struct wq_liste *wq_libres;
  struct wq_liste *wq_occ_debut;
  struct wq_liste *wq_occ_fin;
  spinlock_t wq_lock;


  /* Pour la simulation SESAME : chaines des tampons ecrits.     */
  /* Ce tableau pointe, pour chaque sous-adresse en emission,    */
  /* vers une FIFO soft qui contient les donnees ecrites par     */
  /* l'application dans la sous-adresse.                         */
  /* La FIFO est remplie a chaque appel de ASDCECR et est videe  */
  /* a chaque interruption associee a une emission sur le bus.   */
  /* Le dernier tampon emis est conserve pour etre lu et renvoye */
  /* par ASDCLEC appele sur une sous-adresse en emission.        */
  fifoTampons_t pTampEcr[2][32][32];


  /* Pour debug */
  short trace[8][1000];

  struct asdcmsg msg;
  char asdcmsg_ch[TTMSG];


};




/* Table des pointeurs des variables "statiques" ... */
extern struct asdc_varg ** asdc_p;

/* Acces au pool des tampons des FIFOs */
extern tampon_t * asdcBasePool;
extern tampon_t * asdcFreePool;
extern uint64_t   asdcNbrePool;
extern spinlock_t * pasdcPoolLock;

/* Pointeurs vers les fonctions de connexion aux CEVT */
extern int (*cevt_existence)(int);
extern int (*cevt_signaler)(int, int, int, int, int32_t, int32_t);
extern int (*cevt_signaler_date)(int, int, int, int, int32_t, int32_t, unsigned long long);
extern long (*cevt_ioctl)(int vfx, int asdc, struct file *fp,
                          unsigned int cmd, unsigned long arg);



/* Constantes globales driver */
extern const struct sdcoco asdc_coco[];


/* Definition d'alias utilises dans les sources du driver */
#define COCO    BC_COCO
#define BCRT    BC_BCRT
#define RTBC    BC_RTBC
#define RTRT    BC_RTRT
#define DELAI   BC_DELAI
#define BCRT_DI BC_BCRT_DI
#define RTRT_DI BC_RTRT_DI
#define BC_DERNIER_TYPE BC_RTRT_DI      /* Pour reperer les types anormaux */



/* Definition provisoire, en attendant mise en place d'un eventuel */
/* mecanisme d'emission de donnees "manifestement fausses" quand   */
/* debordement des tampons en mode RT_SYNC2                        */
#define RT_FIN2         0





#endif   /* ASDC_STATICS_H_ */
