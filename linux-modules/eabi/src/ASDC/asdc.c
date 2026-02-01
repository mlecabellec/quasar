
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

asdc.c : Toutes les fonctions de base

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
23/02/2013  YG   Version initiale.


???????     YG   - Remplacement de ioctl() par unlocked_ioctl() a la suite de la
                   suppression du Big Kernel Lock du noyau Linux.
                 - Remplacement de l'appel de __ioremap() par celui de
                   ioremap_nocache().

23/02/2013  YG   Debut d'adaptation au driver ASDC de la carte EMUABI place
                 au dessus du driver de la carte VFX70.

 8/04/2013  YG   Deplacement de unlocked_ioctl() vers le fichier specifique
                 asdcioctl.c
 9/04/2013  YG   Adaptation a la separation des bus (taille table des coco a
                 doublee)
19/04/2013  YG   Remplacement d'anciens semaphores LynxOS par des spinlocks
 2/05/2013  YG   Ajout flag "jamais_initialise".
12/06/2014  YG   Ajout d'un pool de tampons pour memoriser les dernieres
                 donnees emises par chaque sous-adresse en mode transmit.
                 Ajout de l'initialisation des FIFOs qui utilisent ce pool.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
18/06/2014  YG   Modif. types pour compatibilite 32b/64b

*/

#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL
#endif

#include "driverIncludes.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)
#include <linux/vmalloc.h>
#endif

/*********************************************************************
 * Donnees statiques du driver (communes aux differents devices)     *
 *********************************************************************/

/* Allocation de la table globale des pointeurs vers les structures statiques */
static struct asdc_varg *table_allouee[MAX_EMUABI];
struct asdc_varg **asdc_p = table_allouee;

/* Pointeur vers le pool des tampons (ce pool est partage par toutes les */
/* instances du driver EMUABI).                                          */
tampon_t *asdcBasePool = NULL;  /* Tous les tampons (libres ou non) */
tampon_t *asdcFreePool = NULL;  /* La liste des tampons libres */
uint64_t asdcNbrePool = 0;      /* Nombre de tampons dans le pool */
static spinlock_t asdcPoolLock; /* Pour proteger l'acces au pool */
spinlock_t *pasdcPoolLock = &asdcPoolLock;

/* Adresses des fonctions de connexion aux CEVT */
int (*cevt_existence)(int) = NULL;
int (*cevt_signaler)(int, int, int, int, int32_t, int32_t) = NULL;
int (*cevt_signaler_date)(int, int, int, int, int32_t, int32_t,
                          unsigned long long) = NULL;
long (*cevt_ioctl)(int vfx, int asdc, struct file *fp, unsigned int cmd,
                   unsigned long arg) = NULL;

/* Indicateur d'installation correcte de la partie CEVT */
static int cevt_present = 0;

/* Prototypes divers */
void cleanup_asdc(void);
int init_cevt(void);
void cleanup_cevt(void);

struct file;
struct inode;
struct pt_regs;

/* Cette fonction alloue en memoire le pool des tampons utilises pour     */
/* memoriser les donnees ecrites par l'application dans les sous-adresses */
/* en emission. C'est un besoin lia a l'application SESAME.               */
static int asdcPoolAllouer(void) {

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
  /*  TAILLEPOOL = 10240 est defini dans asdc_statics.h                */
  asdcBasePool = (tampon_t *)vmalloc(TAILLEPOOL * sizeof(tampon_t));
  if (asdcBasePool == NULL) {
    /* Echec allocation */
    printk("ASDC : Echec allocation du pool des tampons\n");
    return -ENOMEM;
  }
  printk("ASDC : Succes allocation du pool des tampons : 0x%p\n", asdcBasePool);
  /* Chainage des tampons libres dans le pool */
  {
    int i, j;
    for (i = 1; i < (TAILLEPOOL); i++) {
      asdcBasePool[i].s = NULL;
      asdcBasePool[i - 1].s = &asdcBasePool[i];
      for (j = 0; j < TCARGO; j++)
        asdcBasePool[i].cargo[j] = 0x5A5A;
    }
  }
  asdcFreePool = &asdcBasePool[0];
  asdcNbrePool = TAILLEPOOL;
  spin_lock_init(&asdcPoolLock);
  return 0;
}

/* Liberation de la memoire allouee par la fonction precedente */
static void asdcPoolLiberer(void) {
  if (asdcBasePool)
    vfree(asdcBasePool);
  asdcBasePool = NULL;
  asdcFreePool = NULL;
}

/// Plus necessaire : Le driver VFX70 qui est maintenant utilise comme
///                   point d'entree.// static int
// open( struct inode *inode, struct file *fp )
// {
//    int minor;
//
//    minor = inode->i_rdev & 0xf;
//
//    if( minor > (MAX_EMUABI - 1))
// 	   return( -ENODEV );
//
//    /* if( open_dev[minor] )      Suppression limitation a 1 seul acces (YG)
//    */
//    /*	   return( -EBUSY ); */
//
//
//    if (asdc_p[minor] == NULL)
// 	   return( -ENODEV );
//
//    /* Comptage des acces (YG) */
//    asdc_p[minor]->open_dev++;
//    // printk("open DX2004 : open_dev[%d] = %d\n", minor, open_dev[minor]);
//
//    return( 0 );
// }

/// Plus necessaire : Le driver VFX70 qui est maintenant utilise comme
///                   point d'entree.// static int
// release( struct inode *inode, struct file *fp )
// {
//    int minor;
//
//    minor = inode->i_rdev & 0xf;
//
//    if( minor > (MAX_EMUABI - 1))
// 	   return( -ENODEV );
//
//
//    if (asdc_p[minor] == NULL)
// 	   return( -ENODEV );
//
//    if (asdc_p[minor]->open_dev) {
//            /* Comptage des acces (YG) */
// 	   asdc_p[minor]->open_dev--;
//            // printk("release : open_dev[%d] = %d\n", minor,
//            open_dev[minor]);
// 	   return( 0 );
//    }
//    return( -ENODEV );
// }

/// Plus necessaire : Le driver VFX70 qui est maintenant utilise comme
///                   point d'entree.
// static struct file_operations ygslv_ops = {
//   .owner = THIS_MODULE, /* owner of the world */
// //  NULL,                 /* seek */
// //   .read = read,         /* read */
// //   .write = write,       /* write */
// //  NULL,                 /* readdir */
// //  NULL,                 /* poll */
//   .unlocked_ioctl = asdc_ioctl,       /* ioctl */
// //  NULL,                 /* compat_ioctl */
// //  .mmap = mmap,         /* mmap */
//   .open = open,         /* open */
// //  NULL,                 /* flush */
//   .release = release  //,   /* release */
// //   NULL,                 /* fsync */
// //   NULL,                 /* fasync */
// //   NULL,                 /* lock */
// //   NULL,                 /* readv */
// //   NULL,                 /* writev */
// //   NULL,                 /* sendpage */
// //   NULL                  /* get_unmapped_area */
// };

int init_asdc(void) {
  int i, j, k;
  int ivfx;  /* Indice pour parcourir les cartes VFX70 */
  int iasdc; /* Indice pour parcourir les coupleurs EMUABI */
  int nbreVFX;

  /* nb_tamp : nombre de tampons "flux BC" a allouer */
  /* nb_wq : nombre de "wait queues" Linux a allouer */
  /* Ci-dessous, valeurs provisoires en attendant passage comme parametres */
  int nb_tamp = 20;
  int nb_wq = 100;

  printk("Installation du driver %s\n", DEVICE_NAME);

  /* Initialisation a NULL des pointeurs des variables statiques pour */
  /* pouvoir :                                                        */
  /*     - Detecter la tentative d'acces a une carte inexistante      */
  /*       (specification d'un mauvais mineur)                        */
  /*     - Desallouer correctement la memoire a la desinstallation    */
  for (i = 0; i < MAX_EMUABI; i++)
    asdc_p[i] = NULL;

  /* Initialisation a NULL des pointeurs sur les fonctions fournies */
  /* par le driver CEVT.                                            */
  cevt_existence = NULL;
  cevt_signaler = NULL;
  cevt_signaler_date = NULL;
  cevt_ioctl = NULL;

  /* Combien de cartes VFX70 sur le systeme ? */
  nbreVFX = VFX70_getNumberOfCards();

  for (ivfx = 0, iasdc = 0; ivfx < nbreVFX; ivfx++) {
    int appliDesc;
    appliDesc = VFX70_getApplicationDescriptor(ivfx);

    if ((appliDesc & 0xFFFF0000) == 0xAB100000) {
      struct asdc_varg *pstat;

      /* Une carte configuree a ete trouvee */
      iasdc++;

      /* Allocation de la structure de donnees statique */
      asdc_p[iasdc] = (struct asdc_varg *)vmalloc(sizeof(struct asdc_varg));
      if (asdc_p[iasdc] == NULL) {
        printk("%s : Echec allocation memoire pour \"asdc_varg\" !\n",
               DEVICE_NAME);
        return -ENOMEM;
      }

      /* Initialisation des donnees statiques */
      asdc_p[iasdc]->numero = iasdc;
      asdc_p[iasdc]->signal_number = iasdc;
      asdc_p[iasdc]->numVfx = ivfx;
      asdc_p[iasdc]->open_dev = 0;

      /* Cette variable est utilisee pour faciliter la recuperation */
      /* du code ASDC des CAMBUS/CMB                                */
      pstat = asdc_p[iasdc];

      pstat->nombre_tampons_flux = nb_tamp;
      pstat->nombre_wait_queues = nb_wq;

#ifdef IMPLEMENTATION_DES_TRAMES_DU_MODE_GERANT
      /* -------------------------------------------------
         --   Allocation des tampons pour E/S flux BC   --
         ------------------------------------------------- */

      pstat->pbcf = vmalloc(sizeof(struct asdcbc_tfch) * nb_tamp);
      if (pstat->pbcf == NULL) {
        kkprintf("ASDC : Echec allocation memoire pour tampons flux BC\n");

        // TODO : Fonctionnement du code ci-dessous n'est pas trop clair !!!

        /* Echec installation ==> Liberation memoire deja allouee */
        vfree(pstat);

        // /* liberation du device PCI ??????? */
        // iounmap(bvba);
        // pci_disable_device(sbs1553_pci_dev);

        return ENOMEM;
      }
#else  /* IMPLEMENTATION_DES_TRAMES_DU_MODE_GERANT */
      pstat->pbcf = NULL;
#endif /* IMPLEMENTATION_DES_TRAMES_DU_MODE_GERANT */

      /* ---------------------------------------------
         --   Memorisation des valeurs par defaut   --
         --------------------------------------------- */
      pstat->asdcdef.iqnum = DEF_IQNUM;
      pstat->asdcdef.mbleng = DEF_MBLENG;
      pstat->asdcdef.bcsmsk = DEF_BCSMSK;
      pstat->asdcdef.bcigp = DEF_BCIGP;
      pstat->asdcdef.brtcnt = DEF_BRTCNT;
      pstat->asdcdef.brtbus = DEF_BRTBUS;
      pstat->asdcdef.rspgpa = DEF_RSPGPA;
      pstat->asdcdef.rspgps = DEF_RSPGPS;

      /* -------------------------------------------------------
         --   Initialisation des semaphores (ou wait queues)  --
         ------------------------------------------------------- */
      spin_lock_init(&pstat->alloc_lock);
      spin_lock_init(&pstat->cevt_lock);
      spin_lock_init(&pstat->lock_debug);
      pstat->mutexflux = 1;
      SINIT(pstat->semmon);
      SINIT(pstat->sem_finbc);
      SINIT(pstat->sem_gotbc);
      SINIT(pstat->sem_exbc);
      SINIT(pstat->semirig);

      /* ------------------------------------------------------
         --   Initialisation de la liste des "wait queues"   --
         ------------------------------------------------------ */
      wq_creer(pstat);
      // TODO : Il faudrait examiner le code de retour !!!

      /* ---------------------------------------------------------
         --   Initialisation de la table des commandes codees   --
         --------------------------------------------------------- */
      for (i = 0; i < 2; i++)
        for (j = 0; j < 32; j++)
          for (k = 0; k < (COCO_MAX + 1); k++)
            pstat->cocor[i][j][k] = (struct scoco){0, 0, 0, 0};

      /* --------------------------------------------------------------
         --  Initialisation a "non disponibles" des tampons flux BC  --
         -------------------------------------------------------------- */
#ifdef IMPLEMENTATION_DES_TRAMES_DU_MODE_GERANT
      for (j = 0; j < pstat->nombre_tampons_flux; j++) {
        pstat->pbcf[j].s = NULL;
      }
#endif
      pstat->pbcfl = NULL;
      pstat->nb_tamp_flux_dispos = 0;

      /* ----------------------------------------------------------
         --  Initialisation des structures de memorisation des   --
         --  donnees ecrites dans les sous-adresses en emission  --
         ---------------------------------------------------------- */
      for (i = 0; i < 2; i++)
        for (j = 0; j < 32; j++)
          for (k = 0; k < 32; k++) {
            pstat->pTampEcr[i][j][k].demis = NULL;
            pstat->pTampEcr[i][j][k].p = NULL;
            pstat->pTampEcr[i][j][k].d = NULL;
            pstat->pTampEcr[i][j][k].nbt = 0;
            spin_lock_init(&pstat->pTampEcr[i][j][k].lock);
          }

      /* ---------------------------------
         --   Initialisation diverses   --
         --------------------------------- */
      pstat->vflash = 0;
      pstat->raz = 0;
      pstat->nbhistoit = 0;
      pstat->deborde = 0;
      pstat->tf_attente = 0;
      pstat->tf_zd = 0;
      pstat->tf_flux = 0;
      pstat->tf_pta = NULL;
      pstat->idtrame = 0;
      for (i = 0; i < MAXDESCRT; i++)
        pstat->descr_trame[i].idtrame = 0;

      /* Variables pour autopsie via SKBD (LynxOS) : encore utiles ??? */
      pstat->dioctl = 0;
      pstat->dphioctl = 0;
      pstat->dit = 0;
      pstat->dphit = 0;

      /* Indicateur systeme inutilise */
      pstat->jamais_initialise = 1;

      /* Initialisation de la memoire image */
      for (i = 0; i < 65536; i++)
        pstat->image[i] = 0;

      /* Enregistrement du diver ASDC aupres du driver VFX70 */
      VFX70_registerUserDriver(ivfx, iasdc, asdc_ioctl, isr_asdc);

      /* Recuperation des adresses de base PCI */
      VFX70_getBaseAddresses(ivfx, &pstat->pcibar0, &pstat->pcibar1,
                             &pstat->pcibar2);
    }
  }

  //     ret_val = register_chrdev ( MAJOR_NUM, DEVICE_NAME, &ygslv_ops );
  //     if (ret_val < 0) {
  //         printk(DEVICE_NAME);
  //         printk("%s : Failed to register error = %d\n", DEVICE_NAME,
  //         ret_val);
  //     } else {
  //         return(0);
  //     }

  if (iasdc) {
    /* Si au moins une carte a ete trouvee, le pool des tampons doit */
    /* etre alloue.                                                  */
    /* asdcPoolAllouer() renvoi 0 en cas de succes.                  */
    if (asdcPoolAllouer()) {
      cleanup_asdc(); /* Il faut liberer la memoire deja allouee */
      return -ENOMEM;
    }

    /* Au moins une carte configuree a ete trouvee */
    /*   ==> On maintient le driver en memoire.    */
    printk("ASDC : Driver installe\n");

    /* Et on installe le driver CEVT */
    cevt_present = init_cevt() == 0;

    /* On renvoi un code OK que le driver CEVT ait ete correctement */
    /* installe ou non.                                             */
    return 0;
  } else {
    /* Aucune carte configuree n'a ete trouvee                 */
    /*   ==> On abandonne (l'execution du driver s'acheve).    */
    printk("ASDC : Echec installation\n");
    return -ENODEV;
  }
}

void cleanup_asdc() {
  int i;

  /* La partie CEVT du driver doit etre desinstallee prealablement */
  /* a la partie ASDC.                                             */
  if (cevt_present)
    cleanup_cevt();

  printk("Desinstallation du driver %s\n", DEVICE_NAME);

  /* Pour chacune des instances du driver, demande de    */
  /* deconnexion du driver de la VFX70, puis liberation  */
  /* de la memoire allouee.                              */

  for (i = 0; i < MAX_EMUABI; i++) {
    if (asdc_p[i]) {
      VFX70_unregisterUserDriver(asdc_p[i]->numVfx);
      vfree(asdc_p[i]);
      asdc_p[i] = NULL;
    }
  }

  /* Et, finalement, liberation du pool de tampons global */
  asdcPoolLiberer();

  //     if (ret_val >= 0) {
  //         unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
  //         for (i = 0; i < MAX_EMUABI; i++) {
  //             if (asdc_p[i]) {
  //                 vfree(asdc_p[i]);
  //                 asdc_p[i] = NULL;
  //             }
  //         }
  //     }
}

/* Cree les liens entre les fonctions definies dans le driver FPGA et les */
/* appels effectuees dans le driver VFX70.                                */
int ASDC_registerCEVTDriver(long (*ioctl)(int vfx, int asdc, struct file *fp,
                                          unsigned int cmd, unsigned long arg),
                            int (*existence)(int),
                            int (*signaler)(int, int, int, int, int32_t,
                                            int32_t),
                            int (*signaler_date)(int, int, int, int, int32_t,
                                                 int32_t, unsigned long long)) {
  /* Enregistrement */
  cevt_ioctl = ioctl;
  cevt_existence = existence;
  cevt_signaler = signaler;
  cevt_signaler_date = signaler_date;

  printk("ASDC : cevt_ioctl=%p cevt_existence=%p\n", cevt_ioctl,
         cevt_existence);
  printk("ASDC : cevt_signaler=%p cevt_signaler_date=%p\n", cevt_signaler,
         cevt_signaler_date);
  return 0;
}

/* Supprime les liens avec un driver utilisateur */
int ASDC_unregisterCEVTDriver(void) {
  /* Desenregistrement */
  cevt_ioctl = NULL;
  cevt_existence = NULL;
  cevt_signaler = NULL;
  cevt_signaler_date = NULL;
  return 0;
}

// /* Exportation des fonctions utilisees pour communiquer avec d'autres drivers
// */ EXPORT_SYMBOL(ASDC_registerCEVTDriver);
// EXPORT_SYMBOL(ASDC_unregisterCEVTDriver);

// MODULE_AUTHOR("************ Anonymized");
// MODULE_DESCRIPTION("Pilote " ASDC_NOM " v" __stringify(ASDC_VERSION) "."
//                     __stringify(ASDC_REVISION) " du " ASDC_DATE);
// MODULE_SUPPORTED_DEVICE("Place au-dessus du pilote de la carte VFX70. Le
// FPGA"
//                         "de cette derniere doit etre configure avec EMUABI");
// MODULE_LICENSE("GPL");
