/************************************************************************
 *                                                                      *
 *      Driver pour carte AMI d'interface 1553 (fabriquee par SBS)      *
 *     ------------------------------------------------------------     *
 *                                                                      *
 *                     VARIABLES GLOBALES DU PILOTE                     *
 *                                                                      *
 *     ou, pour LynxOS, STRUCTURE DES VARIABLES STATIQUES DU PILOTE     *
 *                                                                      *
 *                                                                      *
 *                                         Y.Guillemot, le 25/09/1994   *
 *                                      derniere modif. le              *
 *                                               v3.3 : le 13/04/1995   *
 *                                               v3.4 : le  1/12/1995   *
 *                                               modif. le 14/12/1995   *
 *                                               v3.5 : le 10/01/1996   *
 *                                                                      *
 *                         Adaptation a LynxOS - v4.0 : le  6/04/2001   *
 *                                  Ajout des flux BC : le 23/10/2001   *
 *                            Ajout des reveils sur RAZ le  9/11/2001   *
 *  Ajout memorisation histogramme utilisation queue IT le 27/11/2001   *
 *        Ajout memorisation tampon isole pour fluxs BC le 28/11/2001   *
 *                        Ajout des flux_evt du mode RT le 29/04/2002   *
 *        Ajout variables pour autopsie via SKBD (v4.4) le 15/07/2002   *
 *       Ajout variables pour lecture heure IRIG (v4.5) le 21/08/2002   *
 *      Ajout memorisation adr. phys. (bpba) pour debug le 28/08/2002   *
 *                                                                      *
 * Suppression de la gestion des flux d'evenements RT : le 10/09/2002   *
 *  Connexion des evenements rt au pseudo-driver CEVT : le 10/09/2002   *
 *                   bloc_suivant : passage en unsigned le 17/09/2002   *
 * Passages divers en unsigned, et suppression                          *
 *                             de variables inutilisees le 24/09/2002   *
 *                                               v4.6 : le 17/09/2002   *
 *                                                                      *
 * Regroupement des 2 coupleurs d'une meme carte en un                  *
 *                                 seul device (v4.7) : le 18/10/20002  *
 *                                                                      *
 *   Fonction d'attente execution d'un bloc BC (v4.8) : le 13/11/2002   *
 *                                                                      *
 *                  Source commun LynxOS/Linux (v4.9) : le 15/01/2003   *
 *                          Ajout indicateur "vflash" : le 23/01/2003   *
 *                                                                      *
 *       Ajout - ref. trame courante "idtrame"                          *
 *             - definition des descripteurs de trame : le  3/08/2004   *
 *                                                                      *
 *      Modif. definition de asdc_p pour Linux v2.6.x : le 22/10/2004   *
 *                                         ASDC v4.12 : le  7/01/2005   *
 *                                                                      *
 *    Ajout fonction de debug/investigation ASDCTRACE : le 10/03/2005   *
 *      Ajout fonction de debug/investigation ASDCMSG : le 10/06/2005   *
 *                                                                      *
 * Adaptation aux "wait-queues" corrigees (Linux) :     le  7/09/2005   *
 *                                                                      *
 *                  Ajout compatibilite Linux 64 bits : le 14/03/2008   *
 *            Ajout compatibilite noyau Linux v2.6.31 : le  1/12/2009   *
 ************************************************************************/



#ifdef LINUX

  /* Structure pour definir les chaines de wait queues  */
  /* utilisees sous Linux pour remplacer les semaphores */
  /* LynxOS places directement en memoire image.        */
  struct wq_liste
    {
      wait_queue_head_t	wq;     /* Wait Queue */
      unsigned int	nbre;	/* Nombre d'attentes en cours */
      int		cond;	/* Condition autorisant le reveil */
      struct wq_liste	*p;     /* Precedent */
      struct wq_liste	*s;     /* Suivant */
    };

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
                       int minpmaj;	/* Nb cycle min. par cycle maj. */
                       unsigned int cycles;
                     };



/* Structure des donnees statiques associees a un coupleur (une voie) */

struct asdc_varg
{
   int dioctl;		/* Debug : Dernier ioctl appele */
   int dphioctl;	/* Debug : Phase ioctl en cours */
   int dit;		/* Debug : Derniere IT appelee */
   int dphit;		/* Debug : Phase IT en cours */


   void *h_drm;
   void *bvba;				/* Board Virtual Base Address */
   void *bpba;				/* Board Physical Base Address */
   int	bus;
   int 	device_number;			/* Non utilise pour le moment */
   char *int_handler;			/* not used at present */
   int 	status;
   int 	base_address;
   int	int_vector;
   int	isr_installed;
   int  lock;
   int  Vendor_ID;
   int  Device_ID;
   int	signal_number;		/* Pour messages de debug uniquement ... */

   int link_id;			/* Pour chainage des IT */

   int vflash;			/* Indicateur chargement memoire flash */


   struct asdcparam asdcdef; /* Parametres "par defaut" */
   int nbopen;			/* Nombre de processus ayant */
				/* "ouvert" le driver        */


   int nombre_it;   /* Nombre de processus ayant */
			 /* attente IT en cours       */

     /*** Ci-dessous : sans doute devenu inutile ... ***/
   int asdc_ici; /* Indicateur presence carte ABI :               */
                 /*    Si VMV : prend valeur PRESENTE, ABSENTE ou IGNORE */
                 /*    sinon, 0 pour carte absente et 1 pour presente    */


   int monok;	        /* Indicateur pour espion sequentiel */
   SDECLARE(semmon);	/* Semaphore pour espion sequentiel */

   /* Pour creation de trame */
   unsigned short int bloc_suivant;	/* Memorisation adresse pointeurs de */
					/* chainage des blocs BC             */



   SDECLARE(sem_finbc); /* Semaphore pour attendre fin trame BC */
   SDECLARE(sem_gotbc); /* Semaphore pour attendre debut trame BC */
   int fin_bc;	        /* Indicateur trame achevee (mode BC) */
   int stocke_bc;       /* Validation acquisition (bufferisation) BC */
   SDECLARE(sem_exbc);  /* Semaphore pour "gerant generique" */


//	/* Stockage derniere commande codee recue par abonne :  */
//	/*   Deuxieme indice = bus, troisieme indice = adresse 1553 */
//   short int cocorec[2][32];

   /* Pour traitement des commandes codees */
   /*   - Indice 1 : Numero du RT                                   */
   /*   - Indice 2 : Numero de la CC (les 5 bits du code seulement) */
   struct scoco cocor[32][COCO_MAX + 1];

   /* Table des blocs memoires libres (pour allocation memoire) */
   struct smemo memo[TMEMO];
   int ipremier;  /* Indice (dans memo) plus petit bloc disponible */
   int idernier;  /* Indice (dans memo) plus grand bloc disponible */

   /* Semaphore de protection des fonctions d'allocation memoire */
   int mutexalloc;

   /* Semaphore pour insertion retard ... (PROVISOIRE (?) ...) */
   int semretard;

   /* Semaphore pour attendre la disponibilite de l'heure IRIG */
   SDECLARE(semirig);

   /* Image de la memoire d'echange du coupleur */
   long image[65536];

   /* Semaphore de protection des fonctions d'allocation des fluxs */
   int mutexflux;

   /* Nombre total de tampons pour flux BC et pointeur vers ces tampons */
   int nombre_tampons_flux;
   struct asdcbc_tfch *pbcf;	/* Pointe la base du tableau qui contient */
				/* tous les tampons                       */

   /* Nombre de tampons restants et pointeur vers le premier de ces tampons */
   int nb_tamp_flux_dispos;
   struct asdcbc_tfch *pbcfl;	/* Pointe le premier element de la chaine */
				/* des tampons libres                     */

   unsigned int pflux, dflux;	/* Adresse des premiers et derniers */
   				/* flux definis                     */

   int idtrame;			/* Adresse trame en cours execution */



   int raz;		/* Indicateur de RAZ coupleur/driver en cours */

   int nbhistoit;		/* Nbre d'ITs accumulees dans l'histogramme */
   int histoit[ASDCTHISTOIT];	/* Histogramme de l'usage table des ITs */
   int deborde;			/* Indicateur de debordement histogramme */

   /* Pour memoriser un tampon flux BC isole en attendant une eventuelle */
   /* interruption 0x5 indiquant une erreur                              */
   unsigned int tf_attente;	/* Indicateur tampon isole et memorisation */
   				/* du bloc BC concerne			   */
   unsigned int tf_zd;	      /* Memorise adr. zone donnee du flux concerne */
   unsigned int tf_flux;      /* Memorise flux concerne */
   struct asdcbc_tfch *tf_pta;	/* Pointe tampon isole */


   /* Descripteurs de trame (pour enchainements) */
#define MAXDESCRT	100
   struct descr_trame_s descr_trame[MAXDESCRT];


   /* Provisoire : pour essai ... */
   int dbg_ns;
   int dbg_s;


   int nombre_wait_queues;	/* Nombre de structures "wait queues" */
   				/* Linux allouees                     */

  /* Les variables ci-dessous ne sont utilisées que sous Linux */
#ifdef LINUX

  unsigned int irq;	/* Pour memo ligne d'IT sous Linux */

  /* Pointeurs et lock pour gestion des listes de "Wait queues" */
  struct wq_liste *wq_base;
  struct wq_liste *wq_libres;
  struct wq_liste *wq_occ_debut;
  struct wq_liste *wq_occ_fin;
  spinlock_t wq_lock;

#endif /* LINUX */

  /* Pour debug */
  short trace[8][1000];

  struct asdcmsg msg;
  char asdcmsg_ch[TTMSG];
};



/* Structure des donnees statiques associees a une carte (une ou deux voies) */

struct asdc_statics
{
  int bivoie;			/* 1 si carte parametree comme "bi-voie" */
  int voie2;			/* 1 si seconde voie effectivement detectee */
  struct asdc_varg *varg[2];  	/* Le numero de mineur servira d'indice    */
  				/* (Sous Linux, l'indice est   mineur % 2) */
#ifdef LINUX
  void *pci_dev_texas;          /* "Device" PCI associe au bridge TEXAS */ 
#endif /* LINUX */
};



#ifdef LINUX

  /* Table des pointeurs vers les variables statiques des differents devices */

# ifdef PRINCIPAL
    struct asdc_statics * asdc_p_alloue[ASDC_MAX_CARTES];
    struct asdc_statics ** asdc_p = asdc_p_alloue;
# else
    extern struct asdc_statics ** asdc_p;
# endif

#endif	/* LINUX */
