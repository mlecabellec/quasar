/*

 DRIVER LINUX POUR LE COUPLEUR 1553 EMUABI
 
 asdc.h : Definition des constantes et de structures de l'interface LN1-LN2.

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
23/02/2013  YG   Version initiale.
 9/04/2013  YG   Adaptation des valeurs par defaut des parametres
19/04/2013  YG   Ajout nouveaux champs a quelques structures
19/04/2013  YG   Ajout nouveaux champs a quelques structures
 7/05/2013  YG   Suppression de la zone de 0x800 mots reserves mais jamais
                 utilises dans le haut de la memoire d'echange.
11/06/2014  YG   Ajout fonctions ioctl ASDC_SA_LIER et ASDC_SA_DELIER.
11/02/2015  YG   Ajout constante ASDC_MEMESS pour debug

*/



#ifndef ASDC_EMUABI_H_
#define ASDC_EMUABI_H_


/* Taille zone memoire partagee (en mots de 32 bits) */
#define TAILLEMEM   0x10000


typedef struct Yg_DoubleInt_st {
            int data1;
            int data2;
        } Yg_DoubleInt;
        


/* Type a usage interne du pilote */
#define uint16  unsigned short


/* Taille de certaines structures internes du pilote */
#define TTITBC  150             /* Taille table des IT du mode BC */
#define NBTLTBC 50              /* Nombre de Tampons de Lecture Trame BC */
#define TMEMO   200             /* Taille de la table des blocs mem. libres */
#define TQITE   200             /* Taille table des IT des RT en emission */


/******************************************************
   Les valeurs ci-dessous sont elles utilisees ???   et par qui ???
*************************************************************/

/* Valeurs par defaut */
#define DEF_IQNUM         4
#define DEF_MBLENG       40
    /* ATTENTION : Experience montre que Pbs ITs si mbleng < 24 !!! */
#define DEF_BCSMSK      0x0     /* Pas d'IT sur status */
#define DEF_BCIGP       270     /* (Gap - 4) * 15   [Gap en us] soit 25 us */
#define DEF_BRTCNT        1     /* Nombre de reprises si erreur (BC) */
#define DEF_BRTBUS        0     /* Reprise toujours sur le meme bus */
#define DEF_RSPGPA      180     /* Temps de reponse maxi autorise : 20 us */
                                /* Param = (T - 2) x 10 avec T en us */
#define DEF_RSPGPS       80     /* Temps de reponse par defaut des RT simules */
                                /* Param = (T - 2) x 10 avec T en us */




/* Memoire disponible (mots) :                                   */
/* Pour l'instant, seul le cas 64 kmots est traite               */
#define ASDC_TAILLEMAX 0x10000


#define MOT unsigned short int


/************************************************************************/
/* Adresses des registres PCI_BAR2 d'une carte Acromag VFX70 configuree */
/* en emulateur ABI                                                     */
/************************************************************************/
#define BAR2_MAGIC1   0x9000
#define BAR2_MAGIC2   0x9004
#define BAR2_CSR      0x9008
#define BAR2_TIMEH    0x900C
#define BAR2_TIMEL    0x9010
#define BAR2_EMISSA   0x9014
#define BAR2_EMISSB   0x9018
#define BAR2_RECEPA   0x901C
#define BAR2_TIMEH_A  0x9020
#define BAR2_TIMEL_A  0x9024
#define BAR2_RECEPB   0x9028
#define BAR2_TIMEH_B  0x902C
#define BAR2_TIMEL_B  0x9030

#define BAR2_TRIG     0x9094
#define BAR2_RTIM     0x9098
#define BAR2_FPGA_UC1 0x909C
#define BAR2_FPGA_UC2 0x90A0
#define BAR2_UC_FPGA1 0x90A4
#define BAR2_UC_FPGA2 0x90A8
#define BAR2_UC_FPGA3 0x90AC


/*****************************************************************************/
/* Adresses des registres et zones memoires d'une carte ABI/ASF-PMC          */
/*      ---> Adressage en "mots" par rapport au debut RAM echange            */
/*****************************************************************************/

#define MAP             0x10 /* Sequencer MAP Register   */
#define RAM_ERR         0x19 /*         */
#define ENCODER_ERR     0x22 /*         */
#define TEST_SELECT_REG 0x3D /*         */
#define FWPROC          0x3E /* Firmware product code    */
#define FWVERC          0x3F /* Firmware version code    */
#define PCODE           0x40 /* Product code for DSP     */
#define VCODE           0x41 /* Version code for DSP     */
#define CNFGREG         0x43 /* Configuration register   */
#define CMD             0x80 /* Command Word             */
#define RESP            0x81 /* Response Word            */
#define IQRSP           0x82 /* It Queue Response Flag   */
#define IQPTR1          0x83 /* It Queue Pointer 1       */
#define IQPTR2          0x84 /* It Queue Pointer 2       */
#define IQCNT           0x85 /* It Queue Count 1         */
#define IQCNT1  IQCNT
#define IQCNTS          0x86 /* It Queue Count 2         */
#define IQCNT2  IQCNTS
#define IQNUM           0x87 /* It Queue Nbr of Entries  */
#define SWTPTR          0x88 /* Status Word Table Ptr    */
#define ATPTR           0x89 /* Address Table Pointer    */
#define FTPTR           0x8A /* Filter Table Pointer     */
#define SFMS            0x8B /*                          */
#define M1PTR           0x8C /* Monitor Buffer 1 Pointer */
#define M2PTR           0x8D /* Monitor Buffer 2 Pointer */
#define MBLEN           0x8E /* Monitor Buffer Length    */
#define BCIPTR          0x8F /* BC Initial Pointer       */
#define BCCPTR          0x90 /* BC Current Pointer       */
#define BCLPTR          0x91 /* BC Last Block Pointer    */
#define BCSMSK          0x92 /* BC Status Word Mask      */
#define SCHIGH          0x93 /* System Clock Hight       */
#define SCLOW           0x94 /* System Clock Low         */
#define RTPPTR          0x95 /* RT Phase Table Pointer   */
#define BITPTR          0x96 /* RT Bit Word Table Ptr    */
#define LCDPTR          0x97 /* Last Command Table Ptr   */
#define TVWPTR          0x98 /* Tr. Vector Word Tbl Ptr  */
#define LSWPTR          0x99 /* Last Status Word Tbl Ptr */
#define LSYPTR          0x9A /* Last Synch Word Tbl Ptr  */
#define PROPTR          0x9C /* Protocol Table Pointer   */
#define CCW             0x9D /* Clock Control Word       */
#define MIMPTR          0x9E /* Mode It Mask Table Ptr   */
#define MBFLG           0x9F /* Seq. Mon. Control Word   */
#define SCHIGHI         0xA0 /* IRIG clock hight         */
#define SCMIDI          0xA1 /* IRIG clock middle        */
#define SCLOWI          0xA2 /* IRIG clock low           */
#define BCERVL          0xA7 /*   */
#define SMBCNT          0xAB /*   */
#define RSPGPA          0xAC /* max ReSPonse status GaP time Allowed */
#define RSPGPS          0xAD /* actual ReSPonse status GaP time  */
#define BCIGP           0xAE /* BC Inter message Gap Time  */
#define BRTCNT          0xAF /* Bc ReTry CouNT  */
#define BRTBUS          0xB0 /* Bc ReTry BUS  */
#define BRTCMD          0xB1 /*   */
#define BRTRTC          0xB2 /*   */
#define STUBSEL         0xBE /*   */
#define MFPERFR         0xBF /*   */
#define MFTVALH         0xC0 /*   */
#define MFTVALL         0xC1 /*   */
#define MFTCNTH         0xC2 /*   */
#define MFTCNTL         0xC3 /*   */
#define MFTEXEH         0xC4 /*   */
#define MFTEXEL         0xC5 /*   */
#define ASYNCBC         0xC6 /*   */
#define ASYNCBCT        0xC7 /*   */
#define RESPTR          0xCF /*   */
#define BCPAM           0xD0 /*   */
#define VVCTL           0xD5 /*   */
#define VVALUE          0xD6 /*   */
#define GINT            0xD7 /*   */
#define TRG3_CTL        0xDE /*   */
#define GLBLCTL         0xE9 /* Global control register  */
#define ERRTBL         0x104 /* Debut table des erreurs  */
#define ERRPAR_DC       0x11E /* Derniere cmd ayant eu une erreur de parite */
#define ERRPAR_NB       0x11F /* Nbre total d'erreurs de parite */
#define INITOK          0x13C /*                          */

#define RTSA_CUR       0x2F0 /* RTSA (T) en cours utilisation */
#define RTSA_BUF       0x2F1 /* Tampon RT (T) en cours utilisation */

#define DEBUT_RAM      0x800 /* Debut memoire "utile"    */
#define FIN_RAM       0xFFFF /* Fin memoire "utile"      */

// #define CSR        0x40000 /* Registre CSR */ // ABI : CE N'EST PLUS LE BON

#define BIT_CSR_IT    0x0080 /* Bit IT arrivee du CSR */




struct sbs1553_ioctl
 {
   int device;
   int *buffer;
   int length;
   unsigned long offset;
   int interrupt_vector;
   char *interrupt_handler;
 };


union byte_swap_type
 {
   int data_long;
   short data_word[2];
   char byte[4];
 };



/* Structure de description d'une voie */
struct asdcvoie { unsigned char bus;                    /* 0 ou 1         */
                  unsigned char adresse;                /* 0 a 31         */
                  unsigned char sous_adresse;           /* 0 a 31         */
                  unsigned char direction;              /* 1=T, 0=R       */
                  int nmots;              /* nbre mots : 0 a 31, 0 => 32  */
                  int ntamp;              /* nbre tampons   au moins 1     */
                  int mode;               /* Si O : SYNC, Si 1 : ASYNC     */
                  int adrtamp;            /* Adresse tampons               */
                  int adrtamp2;           /* Adresse tampon 2 si ASYNC     */
                };



/* Structure de description d'une commande codee associee a une voie */
struct asdcvoiecc { unsigned char bus;          /* 0 ou 1         */
                    unsigned char adresse;      /* 0 a 31  */
                    unsigned short coco;        /* CC_DBC, CC_S, etc...  */
                    int cevt;                   /* Numero CEVT associe */
                    int to;                     /* Time-out (en ticks) */
                  };



/* Structure d'acces au "MIM" (Mode Command Interrupt Mask) */
struct asdcmim { unsigned char adresse; /* 0 a 31                       */
                 int mim;               /* Masque IT commandes codees   */
                };


/* Structure de description d'un tampon */
struct asdctampon
   { int num;              /* Numero du tampon a ecrire          */
     int p;                /* Pointeur du tampon suivant         */
     int f;                /* "Flag" du tampon                   */
     int nbr;              /* Nombre de mots valides             */
     MOT t[32];            /* Contenu du tampon                  */
     struct asdcvoie v;    /* Descript. voie (bus, adr, sa, dir) */
   };





/* Structure de lecture d'un flux d'evenements RT */
struct asdcrteflux { short flux;        /* Id flux */
                     short mode;        /* Mode lecture */
                     short adr; /* Adresse voie lue */
                     short sa;  /* Sous-adresse voie lue */
                     short dir; /* Sens voie lue */
                     short opt; /* N. U. */
                     long  cmd; /* Commande 1553 lue */
                     short date[4];     /* Datation evenement */
                   };


/* Structure tampon pour flux evenements RT */
struct asdcev_tf { short cmd;           /* Message 1553 cause de l'evenement */
                   short err;           /* Indicateur erreur */
                   short date[4];       /* Datation (non encore utilise) */
                 };

/* Structure de chainage des tampons pour flux_evt RT */
struct asdcev_tfch { struct asdcev_tf tamp;     /* Tampon */
                     struct asdcev_tfch *s;     /* Pointeur du suivant */
                   };


/* Structure pour rebouclage de sous-adresses (test reseau via SA30) */
typedef struct abiboucl_s {
    unsigned char bus;
    unsigned char adr;
    unsigned char sar;
    unsigned char sat;
} abiboucl_t;



/* Structure de description de l'environnement de */
/* la table memo[] (pour debug uniquement)        */
struct asdcememo { int tmemo;            /* Taille de la table memo[]  */
                   short int ipremier;   /* Indice plus petit bloc     */
                   short int idernier;   /* Indice plus grand bloc     */
                 };

/* Structure de description d'une entree "bloc libre en memoire" de */
/* la table memo[] (pour debug uniquement)                          */
struct asdcblibre { int i;            /* Indice dans la table memo[]       */
                    MOT a;            /* Adresse debut bloc                */
                    MOT l;            /* Taille du bloc                    */
                    short int p;      /* Indice entree precedente          */
                    short int s;      /* Indice entree suivante            */
                  };


/* Structure de description d'un bloc BC */
struct asdcbbc { MOT type;
                 unsigned char bus;             /* 0 (A) ou 1 (B) */
                 unsigned char adresse;         /* 0 a 31         */
                 unsigned char sous_adresse;    /* 0 a 31         */
                 unsigned char adresse2;        /* 0 a 31         */
                 unsigned char sous_adresse2;   /* 0 a 31         */
                 unsigned char nbmots;          /* nbre mots 1 a 32   */
                 MOT options;                   /*  */
                 MOT cocodee;                   /* Type commande codee */
                 MOT adrbbc;                    /* Adresse du bloc */
                 MOT adrtamp;                   /* Adresse tampon    */
                 MOT adrbbcsuiv;                /* Adresse bloc suivant */
                 MOT retard;                    /*  */
                 int chainage;                  /* 1 si doit etre chaine a */
                                                /* bloc precedent          */
                 MOT start;             /* Pour bloc SCHEDule */
                 MOT reprate;           /* Pour bloc SCHEDule */
                 unsigned int heure;    /* Pour bloc SCHEDule */
                };



/* Structure pour acceder aux tampons de donnees BC */
struct asdctampbc { MOT i;      /* Indice tampon dans memoire d'echange */
                    MOT n;      /* Nombre des donnees a transferer */
                    MOT d[32];  /* Donnees */
                  };


/* Structure pour modifier un bloc BC */
struct asdcmbbc { MOT adrbbc;   /* Adresse du bloc BC */
                  MOT valid;    /* Indicateur validite des champs suivants */
                                /* (cf. ci-dessous)                        */
                  MOT bloc;     /* Type du bloc BC */
                  MOT cmd1;     /* Premier mot de commande 1553 */
                                /* (ou delai si "bloc delai")   */
                  MOT cmd2;     /* Second mot de commande 1553 */
                  MOT opt_1;    /* Options (fanions) : */
                  MOT opt_0;    /*      Bit a 1 dans opt_1 ==> Passage a 1 */
                                /*      Bit a 1 dans opt_0 ==> Passage a 0 */
                                /*      Bits a 1 dans opt_0 et opt_1       */
                                /*                     ==> Changement etat */
                  MOT nbre;     /* Nombre de donnees valides */
                  MOT start;    /* Bloc schedule uniquement */
                  MOT reprate;  /* Bloc schedule uniquement */
                  unsigned int heure;   /* Pour bloc schedule uniquement */
                  MOT d[32];    /* Liste des donnees */
                };

/* Bits de l'indicateur validite des champs de la structure asdcmbbc */
#define SDC_MBBC_BLOC   0x01    /* Type bloc */
#define SDC_MBBC_CMD1   0x02    /* Premier mot de commande (ou delai) */
#define SDC_MBBC_CMD2   0x04    /* Second mot de commande */
#define SDC_MBBC_OPT    0x08    /* Options (fanion) */
#define SDC_MBBC_START  0x10    /* Start (bloc schedule) */
#define SDC_MBBC_RRATE  0x20    /* Reprate (bloc schedule) */
#define SDC_MBBC_HEURE  0x40    /* Heure (bloc schedule) */


/* Structure pour attendre la fin d'une trame BC (et lire la liste des ITs) */
struct asdcitbc
   { int fini;  /* 1 quand trame achevee, 0 sinon */
     MOT (*tit)[4];/* Pointe une table pour copie des ITs recues */
     int nbit;  /* Nombre d'ITs stockees dans tit[] */
   };
      /* Remarque : la zone pointee par tit doit avoir une taille */
      /*            au moins egale a  TTITBC * 4 * sizeof(short)  */
      /******************************************************************/
      /* ATTENTION : Ne pas oublier d'allouer la zone pointee par tit ! */
      /*   Ne pas oublier non plus de copier son adresse dans tit !     */
      /******************************************************************/


/* Structure de description d'un bloc BC utilisee en memoire d'echange */
struct asdcbbcme { MOT type;    /* type bloc */
                   MOT cmd1;    /* Mot de commande 1 */
                   MOT cmd2;    /* Mot de commande 2 */
                   MOT sts1;    /* Mot d'etat 1 */
                   MOT sts2;    /* Mot d'etat 2 */
                   MOT flags;
                   MOT bptr;    /* Pointeur tampon ou delai */
                   MOT link;    /* Adresse bloc suivant */
                 };

/* Structure pour relire les tampons BC apres execution trame */
struct asdclbbc { MOT tous;         /* Tous ou tranferts 1553 seulement ? */
                  MOT adrbbc;           /* Adresse du bloc a lire */
                  struct asdcbbcme *bbc;/* Pointe zone ou stocker bloc BC */
                  MOT *tampon;          /* Pointe zone ou stocker donnees */
                };
       /* Remarques : - La zone pointee par bbc doit avoir une taille */
       /*               au moins egale a 8 mots de 16 bits            */
       /*             - La zone pointee par tampon doit avoir une     */
       /*               taille au moins egale a 32 mots de 16 bits    */
       /******************************************************************/
       /* ATTENTION : - Ne pas oublier d'allouer les zones pointees par  */
       /*               bbc et tampon !                                  */
       /*             - Ne pas oublier non plus de copier leurs adresses */
       /*               dans bbc et tampon !                             */
       /******************************************************************/




/* Structure de description d'un Echange (mode BC) utilisee pour lire */
/* le deroulement d'une trame                                         */
/* Les 40 derniers mots doivent, une fois modifies, correspondre a un */
/* enregistrement obtenu avec la carte SCI (hasdc)                    */
/* Les 3 premiers champs contiennent des donnees specifiques ASDC     */
struct asdcebc { short nbd;    /* -3 - Nombre de mots de donnees  */
                 short cit;    /* -2 - Code IT */
                 short type;   /* -1 - Type bloc BC */
                 short flg;    /*  0 - Numero bus et flags ... */
                 short itr;    /*  1 - Numero du transfert */
                 short nbr;    /*  2 - Nombre reprises sur err. effectuees */
                 short err;    /*  3 - Code erreur ABI */
                 short cmd1;   /*  4 - Mot de commande */
                 short sts1;   /*  5 - Mot de status */
                 short cmd2;   /*  6 - Mot de commande 2 */
                 short sts2;   /*  7 - Mot de status 2 */
                 short adrabi; /*  8 - Adresse bloc BC dans memoire ABI */
                 short msg[32]; /* 9-40 - Mots de donnees */
               };


/* Structure pour Lire une Trame (en mode BC) en cours de deroulement */
struct asdcltbc { short instr;                  /* Instruction demandee */
                  short etat;                   /* Resultat de l'execution */
                  struct asdcebc *tampon;       /* Pointe zone tampon */
                };

/* Structure tampon pour flux BC */
struct asdcbc_tf { short type;          /* Type du bloc BC */
                   short err;           /* Mot d'erreur eventuel */
                   short date[4];       /* Datation (non encore utilise) */
                   short cmd1;          /* Mot de commande 1 */
                   short cmd2;          /* Mot de commande 2 */
                   short sts1;          /* Mot de status 1 */
                   short sts2;          /* Mot de status 2 */
                   short d[32];         /* Donnees */
                 };

/* Structure de chainage des tampons pour flux BC */
struct asdcbc_tfch { struct asdcbc_tf tamp;     /* Tampon */
                     struct asdcbc_tfch *s;     /* Pointeur du suivant */
                   };

/* Structure pour creer ou augmenter un flux BC */
struct asdcbcflux_aj
                   { int flux;          /* Adresse du premier bloc du flux */
                     int bc;            /* Adresse du bloc BC a ajouter */
                     int nbte;          /* Nbre max tampons en entree */
                     int nbts;          /* Nbre max tampons en sortie */
                   };


/* Structure pour attendre un evenement flux BC ou lire son etat */
struct asdcbcflux_etat
                   { int flux;  /* Adresse du premier bloc du flux */
                     int evt;   /* Evenements attendus/recus */
                     int nbmte; /* Nbre max tampons en entree */
                     int nbmts; /* Nbre max tampons en sortie */
                     int nbcte; /* Nbre courant tampons en entree */
                     int nbcts; /* Nbre courant tampons en sortie */
                     int nblte; /* Nbre limite tampons en entree */
                     int nblts; /* Nbre limite tampons en sortie */

                     int ppte;  /* Pour debug : pointeur premier tampon E */
                     int pdte;  /* Pour debug : pointeur dernier tampon E */
                     int ppts;  /* Pour debug : pointeur premier tampon S */
                     int pdts;  /* Pour debug : pointeur dernier tampon S */
                   };

/* Structure pour lire ou ecrire les donnees d'un flux BC */
struct asdcbcflux { int flux;     /* Adresse du premier bloc du flux */
                    int   nbtt;   /* Nbre de tampons a transferer */
                    int   nbtet;  /* Nbre tampons effectivement transferes */
                    struct asdcbc_tf *z;  /* Ptr zone tampon utilisateur */
                  };







/* Parametres du pilote AMI */
struct asdcparam { int iqnum;     /* Taille tampon ITs    */
                   int mbleng;    /* Taille tampon espion */
                   int bcsmsk;    /* Masque pour IT sur status BC */
                   int bcigp;     /* Intermessage gap time pour BC */
                   int brtcnt;    /* Nbre de reprises pour BC */
                   int brtbus;    /* Bus a utiliser pour reprises */
                   int rspgpa;    /* Temps de reponse maxi RT */
                   int rspgps;    /* Temps de reponse RT simules */
                   int tmaj;      /* Periode de la trame Majeure (en th) */
                   int tmin;      /* Periode de la trame mineure (en th) */
                   int th;        /* Periode horloge de datation (en us) */
                   /* Si autres parametres, les definir ici .... */
                 };



/* Version du pilote AMI */
struct asdcver {
                 char nom[101];        /* Nom du pilote */
                 char version[101];    /* Version du pilote */
                 char date[101];       /* Date du pilote */
                 int  ver;             /* Numero de version du pilote */
                 int  rev;             /* Numero de revision du pilote */

                 int tfpga;            /* Type de la configuration du FPGA */
                 int vfpga;            /* Numero de version du FPGA */
                 int rfpga;            /* Numero de revision du FPGA */
                 
                 int tfw;              /* Type du firmware */
                 int vfw;              /* Numero de version du firmware */
                 int rfw;              /* Numero de revision du firmware */

                 int tiroir_ok;        /* Indicateur tiroir connecte et OK */
  
                 int tiroir_type;      /* Type du tiroir */
                 int tiroir_sn;        /* Numero de serie du tiroir */
                 int tiroir_vuc;       /* Version uC principal du tiroir */
                 int tiroir_ruc;       /* Revision uC principal du tiroir */

                 int signal_number;    /* Numero du coupleur */
                 int bvba;             /* Adresse virtuelle */
                 int bpba;             /* Adresse physique */
               };



/* Periodes des trames Majeures et mineures                         */
/* Ces periodes sont exprimees en nombre de cycles de l'horloge ABI */
struct asdchmm { unsigned long  tmaj;   /* Periode trame Majeure */
                 unsigned short tmin;   /* Periode trame mineure */
               };



/* Structure pour lire l'horloge de datation de la carte ABI :        */
/* Cette structure contient a la fois les registres de l'horloge IRIG */
/* et ceux du "timer ordinaire" mais, bien sur, le contenu d'un seul  */
/* de ces deux groupes de registres est significatif !                */
struct asdcirig { unsigned short schighi;    /* Bits de poids fort IRIG */
                  unsigned short scmidi;     /* Bits de poids moyen IRIG */
                  unsigned short sclowi;     /* Bits de poids faible IRIG */
                  unsigned short sclow;      /* Bits de poids faible timer */
                  unsigned short schigh;     /* Bits de poids fort timer */
                };



/* Structure pour lire l'horloge de datation de la carte ABI et  */
/* l'horloge systeme (calibration et mesure des derives)         */
struct asdchcalibr {
                     /* Heure IRIG (si carte et firmware IRIG seulement) */
                     unsigned short schighi; /* Bits de poids fort IRIG */
                     unsigned short scmidi;  /* Bits de poids moyen IRIG */
                     unsigned short sclowi;  /* Bits de poids faible IRIG */

                     /* Heure non IRIG (si firmware non IRIG seulement) */
                     unsigned short sclow;   /* Bits de poids faible timer */
                     unsigned short schigh;  /* Bits de poids fort timer */

                     /* Pour alignement (utile ?) */
                     unsigned short bidon1;
                     unsigned short bidon2;
                     unsigned short bidon3;

                     /* heure systeme */
                     unsigned int sys_s;    /* secondes */
                     unsigned int sys_ns;   /* nanosecondes */

                     /* Pour Linux/x86 seulement (x86 >= 586) */
                     unsigned int tsc1_h;  /* Poids forts TSC avant */
                     unsigned int tsc1_l;  /* Poids faibles TSC avant */
                     unsigned int tsc2_h;  /* Poids forts TSC au milieu */
                     unsigned int tsc2_l;  /* Poids faibles TSC au milieu */
                     unsigned int tsc3_h;  /* Poids forts TSC apres */
                     unsigned int tsc3_l;  /* Poids faibles TSC apres */

                   };



/* Structure provisoire pour essais ... */
struct asdcdbg  { int s;
                  int ns;
                };



/* Structure pour transmettre a application l'etat interne du pilote */
/* (pour debug seulement ...)                                        */
struct asdcetapil
   { int asdc_nbopen;     /* Nombre de processus ayant */
                          /* "ouvert" le driver        */

     int asdc_nombre_it;  /* Nombre de processus ayant */
                          /* attente IT en cours       */

     int asdc_monok;      /* Indicateurs pour espion sequentiel */

     int asdc_fin_bc;     /* Indicateur trame achevee (mode BC) */
     int asdc_stocke_bc;  /* Validation acquisition (tampons) BC */

     int asdc_n_bc_it;    /* Nombre d'entrees dans asdc_bc_it[][]     */
                          /* Si > TTITBC : debordement asdc_bc_it[][] */

     int asdc_ptbc_ie;    /* Indice prochain tampon a ecrire */
     int asdc_ptbc_il;    /* Indice prochain tampon a lire */
     int asdc_ntbc;       /* Nombre de tampons prets a etre lus */
     int asdc_numebc;     /* Numero de l'echange BC courant */

     int nombre_tampons_flux;   /* Nbre de tampons flux alloues au driver */
     int nb_tamp_flux_dispos;   /* Nombre de tampons flux inutilises */
     struct asdcbc_tfch *pbcf;  /* Pointeur vers table des tampons */
     struct asdcbc_tfch *pbcfl; /* Pointeur vers premier tampon disponible */
     int pflux;         /* Adresse du premier flux defini (ou 0 si aucun) */
     int dflux;         /* Adresse du dernier flux defini (ou 0 si aucun) */

     int nombre_tampons_flux_ev;  /* Nbre tamp. flux evt alloues au driver */
     int nb_tamp_flux_ev_dispos;  /* Nbre tampons flux evt inutilises */
     struct asdcev_tfch *pbcfev;  /* Pointeur vers table des tampons */
     struct asdcev_tfch *pbcflev; /* Ptr vers premier tampon disponible */
     int pfluxev;     /* Adresse du premier flux evt defini (0 si aucun) */
     int dfluxev;     /* Adresse du dernier flux evt defini (0 si aucun) */
   };



/* Structure pour observation du comportement des ITs */

#define ASDCTHISTOIT    200     /* Taille histogramme d'utilisation des ITs */

struct asdchistoit
   { int nombre;
     int histo[ASDCTHISTOIT];
     int deborde;
   };


/* Structure pour traitement des commandes codees */
struct scoco {  int cevt;       /* Numero CEVT associe */
                int sem;        /* Semaphore pour attente */
                short cmd;      /* Mot de commande recu */
                short donnee;   /* Eventuelle donnee associee */
             };


struct sdcoco { int def;                /* B0 : T/R     B1 : cmd definie */
                unsigned short hmim;    /* Bit MIM associes (Pd fort)    */
                unsigned short lmim;    /* Bit MIM associes (Pd faible)  */
              };


/* Structure pour enchainement des trames */
struct chtrame { int trame1;            /* adr. trame 1 */
                 int trame2;            /* adr. trame 2 */
                 unsigned long periode;
                 int minpmaj;           /* Nb trames min. par trame maj. */
                 unsigned long cycles;
               };

/* Pour debug et investigations (cf. ioctl(ASDCTRACE))                       */
struct asdctrace { int adecl;     /* Adresse du registre declencheur         */
                   int videcl;    /* Valeur initiale reg. declencheur  */
                   int mdecl;     /* Mode declenchement :              */
                                  /*  0 : immediat                         */
                                  /*  1 : quand le contenu de adecl change */
                   int nba;       /* Nombre des acquisitions           */
                   int a[8];      /* Adresses des registres a tracer   */
                   short *tr[8];  /* Ptr zones stockage resultats      */
                 };



/* Pour acces bas niveau a une zone memoire/regsitres quelconque */
/* (cf. ioctl(ASDCACCES))                                        */
struct asdcacces { int zone;     /* Zone memoire concernee (ASDC_PCIBAR0, */
                                 /* ASDC_PCIBAR1, ASDC_PCIBAR2, ASDC_MEM  */
                                 /* ou ASDC_IMAGE)                        */
                   int op;    /* Operation : ASDC_LIREMOT ou ASDC_ECRIREMOT */
                   long adresse;
                   int valeur;
                 };

#define ASDC_PCIBAR0      0x1000
#define ASDC_PCIBAR1      0x1001
#define ASDC_PCIBAR2      0x1002
#define ASDC_MEM          0x2004
#define ASDC_MEMESS       0x2008
#define ASDC_IMAGE        0x200C

#define ASDC_LIREMOT      0x0103
#define ASDC_ECRIREMOT    0x010C




#define SLV_LEC_MEM    91
// #define ECR_MEM       92
#define SLV_GET_NAME   94
        
#define GET_IRQNUM    105
#define GET_PCIBAR2   106
#define IT2_WAIT      107
#define IT2_FORCE     108
#define GET_VERSION   109
#define VALIDIT       110
#define LEC_MEM       111
#define ECR_MEM       112
#define INITMEM       113
#define GET_NAME      114

                 
                 
                 
/* Commandes ioctl() pour carte ABI */

#define sbs1553_read            0x100
#define sbs1553_write           0x101
#define sbs1553_status          0x102
#define sbs1553_wait_interrupt  0x103
#define sbs1553_get_baseaddr    0x104
#define sbs1553_install_isr     0x105
#define sbs1553_uninstall_isr   0x106
#define sbs1553_wait_isr        0x107

/* Fonction d'import des fonctions d'acces direct CEVT */
#define ASDC_CEVTIMPORTER       0x110

#define ABI_LIRE_16             0x201
#define ABI_ECRIRE_16           0x202
#define ABI_LIRE_32             0x203
#define ABI_ECRIRE_32           0x204

#define ABI_LIRE_BLOC           0x210
#define ABI_ECRIRE_BLOC         0x211

#define ABI_FIRMWARE_INIT       0x301
#define ABI_FIRMWARE_STOP       0x302

#define ASDC_IMAGE_LIRE         0x401
#define ASDC_IMAGE_ECRIRE       0x402
#define ASDC_IMAGE_LBLOC        0x403
#define ASDC_IMAGE_EBLOC        0x404

#define ASDC_IMAGE_LIRE_L       0x405
#define ASDC_IMAGE_LIRE_H       0x406
#define ASDC_IMAGE_ECRIRE_L     0x407
#define ASDC_IMAGE_ECRIRE_H     0x408

#define ASDCACCES               0x410


/* Pour essai lecture duree remonte IT */
#define ASDC_TESTH              0x500


/* Allocation/liberation d'une zone dans la memoire d'echange de la carte ABI */
#define ASDCALLOUE              0x0701
#define ASDCLIBERE              0x0702

/* Fonction destinee au deverminage de l'allocation memoire */
#define ASDCLECEMEMO            0x0703   /* Lecture envir. memo[] */
#define ASDCLECMEMO             0x0704   /* Lecture entree memo[] */



#define ASDCRAZ         0x1001  /* Reinitialisation carte AMI      */
#define ASDCSTOP        0x1002  /* Arret du microcode carte AMI    */
#define ASDCGO          0x1003  /* Demarrage microcode carte AMI   */
#define ASDCAVORTE      0x1004  /* Reveil de toutes les taches en attente */


#define ASDCLPAR        0x1005  /* Lecture parametres driver  */
#define ASDCEPAR        0x1006  /* Ecriture parametres driver */
#define ASDCVALPAR      0x1007  /* Validation parametres driver */

#define ASDCVFLASH      0x1008  /* Valide l'acces aux adresses utilisees */
                                /* pour ecrire dans la memoire flash     */
                                /* (supprime les messages console)       */

/* Fonction de lecture des indicateurs internes du pilote (pour debug) */
#define ASDCLETAT       0x101F

/* Fonction de lecture des numeros de version du pilote et du microcode */
#define ASDCLVER        0x1020

/* Lecture de l'horloge de datation IRIG de la carte ABI */
#define ASDCLDATE       0x1022
#define ASDCLIRIG       0x1023

/* Lecture "quasi simultanee" horloges ABI et systeme (pour calibrartion) */
#define ASDCHCALIBR     0x1024

/* RAZ et lecture de l'histogramme d'utilisation de la table des ITs */
#define ASDCRAZHISTO    0x1025
#define ASDCLECHISTO    0x1026



/* Fonctions liees a la simulation d'abonnes : */

#define ASDCDEF         0x1101  /* Definition d'une voie        */
#define ASDCMODE        0x1102  /* Choix du mode de fonctionnement d'un RT */
                                /* prealablement defini                    */
                                /* (INHIBE, VALIDE ou ESPION_TR)           */
#define ASDCLIREMODE    0x1103  /* Lecture du mode de fonctionnement d'un RT */

#define ASDCECR         0x1104  /* Ecriture d'un tampon        */
                                /* si flag >=0, flag est ecrit */

#define ASDCLEC         0x1105  /* Lect. d'un tampon */

/* Echange de la chaine de tampons associee a une voie */
#define ASDCXTRT        0x1106

/* Lecture/ecriture de la table des masques IT sur reception commande codee */
#define ASDCLECMIM      0x1107
#define ASDCECRMIM      0x1108

/* Lecture derniere commande codee recue */
#define ASDCLECCC       0x1109

/* Attente prochaine commande codee */
#define ASDCLECCCIT     0x1110


/* Attente IT sur commande emission recue par un abonne */
#define ASDCITE         0x1111

/* Remise a zero des compteurs utilises par ASDCITE pour calculer le */
/* nombre de tampons pleins disponibles.                             */
#define ASDCITERAZ      0x1112

/* Modification du mot de la table des filtres associe a une voie RT */
#define ASDC_ABO_MF     0x1113


#define ASDCECR2        0x1114  /* Ecriture d'un tampon dans "chaine 2" */
                                /* Voie en mode "Synchrone 2" seulement */

#define ASDCEECR        0x1115  /* Echange des chaines de tampons 1 et 2  */
                                /* Voie en mode "Synchrone 2" seulement   */

#define ASDCEECRF       0x1116  /* Echange de la chaine de tampons 1 avec */
                                /* la fin de la chaine de tampons 2       */
                                /* Voie en mode "Synchrone 2" seulement   */


/* Version etendue de ASDC_ABO_MF : generation d'une suite */
/* d'erreurs de parite                                     */
#define ASDC_ABO_MFX    0x1117

/* Relecture du mot "filtre" */
#define ASDC_ABO_GF    0x1118


/* Inhibition/Validation d'une Voie abonne (pour simulation d'erreur bus) */
#define ASDC_ABO_IV     0x1119
#define ASDC_ABO_VV     0x1120

/* Relecture de l'etat valide/inhibee d'une voie */
#define ASDC_ABO_GV     0x1121

/* Suppression totale d'un abonne simule avec liberation memoire d'echange */
#define ASDC_ABO_LIBERER        0x1125

/* Programmation du temps de reponse d'un RT */
#define ASDC_RT_GETTREP       0x1130
#define ASDC_RT_SETTREP       0x1131

/* Programmation du nombre de mots (legal) d'une SA */
#define ASDC_SA_GETNBM        0x1132
#define ASDC_SA_SETNBM        0x1133

/* Modification du mot de status */
#define ASDC_RT_GETSTS        0x1134
#define ASDC_RT_SETSTS        0x1135

/* Modification du mot de donnee associe a une CC */
#define ASDC_CC_GETDATA        0x1136
#define ASDC_CC_SETDATA        0x1137

/* Rebouclage/Debouclage des voies (test SA30) */
#define ASDC_SA_LIER           0x1138
#define ASDC_SA_DELIER         0x1139



/* Fonctions de connexions des evenements RT a un concentrateur CEVT */
#define ASDCEVT_ABO_AJOUTER     0x1161
#define ASDCEVT_ABO_SUPPRIMER   0x1162
#define ASDCEVT_CC_AJOUTER      0x1163
#define ASDCEVT_CC_SUPPRIMER    0x1164




/* Fonctions liees a l'espionnage sequentiel : */

#define ASDCINHESPS     0x1201  /* Inhibition espionnage RT */
#define ASDCVALESPS     0x1202  /* Validation espionnage RT */

#define ASDCESPS        0x1203  /* Initialisation espionnage */
#define ASDCESPSIT      0x1204  /* Attente IT espionnage     */
#define ASDCESPSBASC    0x1205  /* Force le basculement      */
#define ASDCESPSFIN     0x1206  /* Termine l'espionnage      */



/* Fonctions liees a l'utilisation en mode gerant : */

/* Fonction de creation de blocs BC */
#define ASDCBBC         0x1301

/* Fonction d'ecriture d'un tampon de donnees BC */
#define ASDCETAMPBC     0x1302

/* Fonction de lecture d'un tampon de donnees BC */
#define ASDCLTAMPBC     0x1303

/* Fonction de lancement d'une trame BC */
#define ASDCGOTBC       0x1304

/* Fonction d'arret d'une trame BC */
#define ASDCSTOPTBC     0x1305

/* Attente de la fin d'une trame BC */
#define ASDCAFINTBC     0x1306

/* Attente du debut d'une trame BC */
#define ASDCAGOTBC      0x1307          /* *** ATTENTION *** */

/* Attente d'une serie d'ITs du mode BC */
#define ASDCAITBC       0x1308

/* Relecture d'un bloc BC et des donnees associees */
#define ASDCLBBC        0x1309

/* Fonction de suppression en memoire echange d'une trame BC */
#define ASDCSUPTBC      0x1310

/* Fonction de lecture d'une trame BC en cours de deroulement */
#define ASDCLTBC        0x1311

/* Programmation de la periodicite des trames Majeures et mineures */
#define ASDCHMM         0x1312

/* Attente de la fin d'execution d'un bloc BC donne */
#define ASDCAEXBBC      0x1313

/* Modification d'un bloc BC et/ou des donnees associees */
#define ASDCMBBC        0x1314

/* Enchainement de 2 trames */
#define ASDCCHTRAME     0x1315



/* Fonctions liees au flux BC */
#define ASDCBCFLUX_CREER        0x1350
#define ASDCBCFLUX_AJOUTER      0x1351
#define ASDCBCFLUX_SUPPRIMER    0x1352
#define ASDCBCFLUX_ETAT         0x1353
#define ASDCBCFLUX_REGLER       0x1354
#define ASDCBCFLUX_ATTENDRE     0x1355
#define ASDCBCFLUX_LIRE         0x1356
#define ASDCBCFLUX_ECRIRE       0x1357

#define ASDCBCFLUX_ETAT_COMPLET 0x1358
#define ASDCBCFLUX_LTAMPON      0x1359

/* Fonctions de debug et d'investigation */
#define ASDCTRACE       0x1500
#define ASDCLMSG        0x1501






        /*******************************************************/
        /* 0xFC00 correspond a une commande 1553B impossible : */
        /* c'est une demande de Transmission en "Broadcast" !  */
        /* Il est donc possible de l'utiliser comme indicateur */
        /* de tampon vide dans la zone "flag" des tampons      */
        /* (cette zone recoit un mot de commande - en principe */
        /* valide - si le tampon a ete recu ou emis)           */
        /*                                                     */
        /*  Il est possible de disposer de 1024 commandes du   */
        /* meme types grace aux codes 0xFCXX, 0xFDxx, 0xFExx   */
        /* et 0xFFxx : Nous disposons ainsi d'un nombre de     */
        /* drapeaux confortable pour distinguer les differents */
        /* cas imaginables ...                                 */
        /*******************************************************/
/* Codes pour definir l'etat des Tampons RT : */
#define TRT_LIBRE   0xFC00
#define TRT_NOUVEAU 0xFC01

/* Codes pour definir l'etat des elements des queues ITE */
#define ITE_LIBRE   0xFC02      /* Element libre */
#define ITE_SATUREE 0xFC03      /* Il y a eu saturation de la queue ITE */
#define ITE_QITSAT  0xFC04      /* Il y a eu saturation queue IT carte ABI */


/* Codes pour programmer le mode de fonctionnement d'un abonne (RT) */
#define RT_INHIBE       1    /* mode "inhibition du RT" */
#define RT_ESPION       2    /* mode "espion temps-reel" */
#define RT_ABONNE       3    /* mode "abonne simule" */
#define RT_ABONNE_ACC   4    /* mode "abonne simule" avec acceptation GESDYN */

/* Codes pour definir le mode d'une voie RT */
#define RT_VSYNC        0
#define RT_VASYNC       1
#define RT_VSTAT        2
#define RT_VSYNC2       3
#define RT_ETRANSIT     0x8000

/* Codes pour definir le sens des transferts */
#define RT_RECEPTION    0x00
#define RT_EMISSION     0x01

/* Codes pour definir le mode d'E/S aux tampons d'une voie RT */
#define SDC_RAZRAZ      1
#define SDC_RAZ         2
#define SDC_NONBLOQ     3
#define SDC_ATTENDRE    4
#define SDC_PROT2       0x800   /* Fanion "Protection chaine 2 */

/* Bit indicateur d'un debordement des tampons */
#define SDC_DEBORD      0x1000



/* Definition des types de blocs BC */
#define BC_COCO    0    /* Emission d'une commande codee */
#define BC_BCRT    1    /* Emission d'une commande de transfert BC -> RT */
#define BC_RTBC    2    /* Emission d'une commande de transfert RT -> BC */
#define BC_RTRT    3    /* Emission d'une commande de transfert RT -> RT */
#define BC_DELAI   4    /* Attente pendant un temps donne */
#define BC_BCRT_DI 5    /* Emission commande (BC->RT) en mode diffusion */
#define BC_RTRT_DI 6    /* Emission commande (RT->RT) en mode diffusion */

#define BC_SCHED 0x0080 /* Attente d'un instant donne */
#define BC_MINOR 0x8080 /* Debut de "trame mineure" */



/* Definition des commandes codees :                                      */
/*   --> Les 5 bits de poids faibles correspondent au code de la commande */
/*   --> Le bit numero 10 est le bit T/R associe a la commnande           */
#define CC_GESDYN       0x400 | 0
#define CC_DBC          0x400 | 0       /* Dynamic Bus Control */
#define CC_SYNCHRO      0x400 | 1
#define CC_S            0x400 | 1       /* Synchronize (sans mot de donnee) */
#define CC_DSTATUS      0x400 | 2
#define CC_TSW          0x400 | 2       /* Transmit Status Word */
#define CC_IST          0x400 | 3       /* Initiate Self-Test */
#define CC_TS           0x400 | 4       /* Transmitter Shutdown */
#define CC_OTS          0x400 | 5       /* Override Transmitter Shutdown */
#define CC_ITFB         0x400 | 6       /* Inhibit Terminal Flag Bit */
#define CC_OITFB        0x400 | 7       /* Override Inhibit Terminal Flag Bit */
#define CC_INIT         0x400 | 8
#define CC_REINIT       0x400 | 8
#define CC_RRT          0x400 | 8       /* Reset Remote Terminal */
#define CC_LP           0x400 | 9       /* Lancement programme (specifique) */
#define CC_ABORT        0x400 | 0xA     /* Arret programme (specifique) */
#define CC_FLASH        0x400 | 0xB     /* Simulation flash (specifique) */
#define CC_WMSO         0x400 | 0xC     /* Ecriture en MSO (specifique) */
#define CC_VALID        0x400 | 0xD     /* Validation gerant (specifique) */
#define CC_INHIB        0x400 | 0xE     /* Inhibition gerant (specifique) */
#define CC_MODGER       0x400 | 0xF     /* Autor. chgt mode gerant (specif.) */
#define CC_TVW          0x400 | 0x10    /* Transmit Vector Word */
#define CC_SD                   0x11    /* Synchronize (avec mot de donnee) */
#define CC_DCOMMANDE    0x400 | 0x12
#define CC_TLC          0x400 | 0x12    /* Transmit Last Command */
#define CC_BIT          0x400 | 0x13
#define CC_TBW          0x400 | 0x13    /* Transmit BIT Word */
#define CC_STS                  0x14    /* Selected Transmitter Shutdown */
#define CC_OSTS                 0x15    /* Override Sel. Transmitter Shutdown */
#define CC_RES                  0x16    /* Reserve (specifique) */


/* Definition des bits associes aux commandes codees dans les registres MIM */
#define MIM_GESDYN      0x00010000
#define MIM_DBC         0x00010000      /* Dynamic Bus Control */
#define MIM_SYNCHRO     0x00020000
#define MIM_S           0x00020000      /* Synchronize (sans mot de donnee) */
#define MIM_DSTATUS     0x00040000
#define MIM_TSW         0x00040000      /* Transmit Status Word */
#define MIM_IST         0x00080000      /* Initiate Self-Test */
#define MIM_TS          0x00100000      /* Transmitter Shutdown */
#define MIM_OTS         0x00200000      /* Override Transmitter Shutdown */
#define MIM_ITFB        0x00400000      /* Inhibit Terminal Flag Bit */
#define MIM_OITFB       0x00800000      /* Override Inhibit Terminal Flag Bit */
#define MIM_REINIT      0x01000000
#define MIM_RRT         0x01000000      /* Reset Remote Terminal */
#define MIM_TVW         0x00000001      /* Transmit Vector Word */
#define MIM_SD          0x00000002      /* Synchronize (avec mot de donnee) */
#define MIM_DCOMMANDE   0x00000004
#define MIM_TLC         0x00000004      /* Transmit Last Command */
#define MIM_BIT         0x00000008
#define MIM_TBW         0x00000008      /* Transmit BIT Word */
#define MIM_STS         0x00000010      /* Selected Transmitter Shutdown */
#define MIM_OSTS        0x00000020      /* Override Sel. Transmitter Shutdown */
#define MIM_RES         0x00000040      /* Reserve (specifique) */

/* Plus grand code (5 bits) associe a une commande traitee */
#define COCO_MAX        22




/* Definition des "flags" pour blocs de commande BC */
#define FBC_PAS_IT_ERR  1               /* Inhibition IT si erreur */
#define FBC_CONT_SI_ERR 2               /* Continue si erreur */
#define FBC_IT_MASQUE   4               /* IT si status masque */
#define FBC_PAS_IT_FIN  8               /* Inhibition IT si fin trame */
#define FBC_REPRISE     (1<<6)          /* Validation reprise en cas d'erreur */
#define FBC_IT          (1<<7)          /* IT quand exec. bloc achevee */
#define FBC_INOP        (1<<8)          /* Bloc inoperant */
#define FBC_MBUF        (1<<9)          /* Buffers multiples */
#define FBC_ESYNC       (1<<11)         /* Attente synchro en entree */
#define FBC_SSYNC       (1<<12)         /* Emission synchro en sortie */
#define FBC_BUSB        (1<<15)         /* Emission sur bus B (et non A) */


/* Definition des "Instructions" pour ioctl(ASDCLTBC) */
#define LTBC_VALIDE     0
#define LTBC_INHIBE     1
#define LTBC_ETAT       2
#define LTBC_RAZ        3
#define LTBC_LEC        4
#define LTBC_ATLEC      5

/* Definition des "etats" rendus par ioctl(ASDCLTBC) */
#define ELTBC_OK                0
#define ELTBC_VIDE              1
#define ELTBC_SATU              2
#define ELTBC_STOP              3
#define ELTBC_TAMP_PERDU        4
#define ELTBC_IT_PERDUE         5


/* Definition des "evenements" en entree de l'ioctl(ASDCBCFLUX_REGLER) */
#define FLX_EVTLIME     0x0001  /* Franchissement limite en entree */
#define FLX_EVTLIMS     0x0002  /* Franchissement limite en sortie */

/* Definition des "etats" en sortie de l'ioctl(ASDCBCFLUX_ATTENDRE) */
#define FLX_LIME        0x0100  /* Franchissement limite en entree */
#define FLX_LIMS        0x0200  /* Franchissement limite en sortie */
#define FLX_FTRAME      0x0400  /* Fin de la trame */
#define FLX_ERRF        0x0800  /* Erreur flux */

/* Options de controle du flux */
#define FLX_ATSERR     0x10000  /* Demande d'arret trame si erreur flux */


/****************************************************************/
/* ATTENTION : Les bits FLX_xxx et FLX_EVTxxx ci-dessus doivent */
/* imperativement etre decales de 8 bits !!!                    */
/****************************************************************/



/***************************************************************/
/* Definition des positions des mots utilises pour definir les */
/* fluxs dans la "memoire image" de la "memoire d'echange"     */
/***************************************************************/

/* Decalage dans l'image du bloc BC */
#define IMFLUX          0       /* Adresse flux en ME */
#define IMERR           1       /* Code erreur 1553 */
#define IMPZD           2       /* Pointeur donnees (redondant) */
#define IMSEMFLX        3       /* Semaphore flux */
#define IMSUIV          4       /* Adresse bloc BC suivant du flux */
#define IMSEMBC         5       /* Semaphore bloc BC */

#define IMFSD           6       /* 16 bits poids faible = IMDERN */
                                /*      Adresse dernier bloc BC du flux */
                                /* 16 bits poids fort = IMFLXSUIV */
                                /*      Adresse flux suivant */

#define IMCPTR          7       /* Compteur d'executions du bloc BC */

/* Decalage dans l'image du tampon BC */
#define IMNBMTE          0      /* Nombre maxi tampons en entree */
#define IMNBCTE          1      /* Nombre courant tampons en entree */
#define IMNBLTE          2      /* Nombre limite tampons en entree */
#define IMPPTE           3      /* Pointeur premier tampon en entree */
#define IMPDTE           4      /* Pointeur dernier tampon en entree */
#define IMNBMTS          5      /* Nombre maxi tampons en sortie */
#define IMNBCTS          6      /* Nombre courant tampons en sortie */
#define IMNBLTS          7      /* Nombre limite tampons en sortie */
#define IMPPTS           8      /* Pointeur premier tampon en sortie */
#define IMPDTS           9      /* Pointeur dernier tampon en sortie */
#define IMEVT           10      /* Mot d'etat/evenements du flux */
#define IMNBERR         11      /* Nombre d'erreurs flux */
#define IMERRFLX        12      /* Debut de la table des erreurs */

/* Autres constantes */
#define IMNBMPERR        4      /* Nombre de mots memorises par erreur */
#define IMNBMAXERR       5      /* Nombre maxi d'erreurs flux memorisees */



/**************************************************************************/
/* Definition des positions des mots utilises pour programmer les RT et   */
/* definir les flux-evt dans la "memoire image" de la "memoire d'echange" */
/**************************************************************************/

/* Decalage dans l'image du tampon RT */
#define IRCMD           0       /* Commande associee a la voie RT */
#define IRMODE          1       /* Mode de fonctionnement de la voie RT */
#define IRTCAPP         2       /* Adresse "tampon application" courant */
#define IRNBT           3       /* Nombre de tampons materiels */
#define IRSEM           4       /* Semaphore attente message sur voie */
#define IRMEMOF         5       /* Memo filtre pour erreur transitoire */
#define IRCEVT          6       /* CEVT associe a la voie (ou 0 sinon) */
#define IRAFILTRE       7       /* Adresse du "filtre" associe a la voie */
#define IRTCH1          8       /* Chaine 1 si SYNC2 */
#define IRTCH2          9       /* Chaine 2 si SYNC2 */
#define IRTCHINU       10       /* Chaine "inutilisees" si SYNC2 */
#define IRTFCH1        IRTCAPP  /* Fin chaine 1 si SYNC2 */
#define IRTFCH2        11       /* Fin chaine 2 si SYNC2 */
#define IRTFCHINU      12       /* Fin chaine "inutilisees" si SYNC2 */
#define IRTCACHE       13       /* Memo tampons caches qd esp. TR si SYNC2 */

#define IRTPREC        15       /* "Tampon precedent" (chainage inverse) */

#define IRTLET         20       /* Liste erreurs transitoires (ASDC_ABO_MFX) */
#define IRMEMOE        21       /* Memo filtre pour err transitoire etendue */

#define IRTNCH1        30       /* Nombre courant tampons dans chaine 1 */
#define IRTNCH2        31       /* Nombre courant tampons dans chaine 2 */
#define IRTNMT         32       /* Nombre maxi tampons dans chaines (1 ou 2) */






/****************************************************************/
/* Pour debug : definition d'un tampon dans lequel des messages */
/* peuvent etre ecrits sequentiellement avec les macros MSGxx   */
/* definies ci-dessous.                                         */
/* TTMSG definit la taille de ce tampon (alloue dans la struct. */
/* des var. statiques (cf. asdcvarg.h)                          */
/* L'utilitaire de test "lmsg.c" permet de relire le contenu    */
/* de ce tampon.                                                */

// #define TTMSG        200000
#define TTMSG   10000
struct asdcmsg { int ic;            // Indice courant prochaine entree
                 char *ch;          // Pointeur vers zone de memorisation
               };

/* Le driver peut ecrire dans ce tampon en utilisant les macros */
/* definies dans msg_debug.h.                                   */
/****************************************************************/




#endif   /* ASDC_EMUABI_H_ */


