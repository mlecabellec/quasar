/************************************************************************
 *                                                                      *
 *      Driver pour carte NI PCIe-6509 (96 E/S TOR)                     *
 *     ---------------------------------------------                    *
 *                                                                      *
 * pcie6509init.c : Principaux points d'entree du driver.               *
 *                                                                      *
 *                                 ASTRIUM ST - TEA343 - Yves Guillemot *
 ************************************************************************/

/*
   QUAND    QUI   QUOI
---------- ----- ---------------------------------------------------------------
 3/12/2013   YG  Version initiale
24/03/2012   YG  Ajout d'une option pour choisir la logique (positive ou
                 negative) des sorties de la carte.
*/



#include "DriverIncludes.h"
# if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0) 
#  include <linux/vmalloc.h>
# endif





/* Nombre maximum de cartes PCIe-6509 utilisables sur un systeme Linux	*/
#define PCIE6509_MAX_CARTES		8

/***************************************************************/
/* Variables globales associees au driver lui meme             */
/* (et non pas a un "device" particulier, ces dernieres sont   */
/* placees dans les structures pcie6509Statics_t)              */

/* Table des pointeurs vers les variables statiques des differents devices  */
/* pStat est le pointeur global d'acces aux donnees associees aux devices.  */
/* pcie6509Nbre memorise le nombre de cartes (de devices) trouvees.         */
static pcie6509Statics_t * pStat_alloue[PCIE6509_MAX_CARTES];
pcie6509Statics_t ** pAllStat = pStat_alloue;
int pcie6509Nbre = 0;

/* Valeurs par defaut des noms de base des coupleurs */
char *base_nom = "pcie6509_";



/* Memorisation des adresses des registres en fonction du numero des ports : */

/* Adresses relatives des registres de direction */
uint32_t dadr[] = {
    DIO_DIRECTION_P0_P3,  DIO_DIRECTION_P0_P3,  DIO_DIRECTION_P0_P3,
    DIO_DIRECTION_P0_P3,  DIO_DIRECTION_P4_P5,  DIO_DIRECTION_P4_P5,
    DIO_DIRECTION_P6_P7,  DIO_DIRECTION_P6_P7,  DIO_DIRECTION_P8_P11,
    DIO_DIRECTION_P8_P11, DIO_DIRECTION_P8_P11, DIO_DIRECTION_P8_P11 };

/* Adresse relative registres d'entree (lecture) */
uint32_t iadr[] = {
    DIO_INPUT_P0_P3,  DIO_INPUT_P0_P3,  DIO_INPUT_P0_P3,
    DIO_INPUT_P0_P3,  DIO_INPUT_P4_P5,  DIO_INPUT_P4_P5,
    DIO_INPUT_P6_P7,  DIO_INPUT_P6_P7,  DIO_INPUT_P8_P11,
    DIO_INPUT_P8_P11, DIO_INPUT_P8_P11, DIO_INPUT_P8_P11 };

/* Adresse relative registres de sortie (ecriture) */
uint32_t oadr[] = {
    DIO_OUTPUT_P0_P3,  DIO_OUTPUT_P0_P3,  DIO_OUTPUT_P0_P3,
    DIO_OUTPUT_P0_P3,  DIO_OUTPUT_P4_P5,  DIO_OUTPUT_P4_P5,
    DIO_OUTPUT_P6_P7,  DIO_OUTPUT_P6_P7,  DIO_OUTPUT_P8_P11,
    DIO_OUTPUT_P8_P11, DIO_OUTPUT_P8_P11, DIO_OUTPUT_P8_P11 };

/* Decalage des octets associes aux differents ports */
/* dans les registres ci-dessus                      */
int decalage[] = {
    PPORT0, PPORT1, PPORT2, PPORT3, PPORT4, PPORT5,
    PPORT6, PPORT7, PPORT8, PPORT9, PPORT10, PPORT11 };

/* Masques des octets associes aux differents ports */
/* dans les registres ci-dessus                     */
int masque[] = {
    MPORT0, MPORT1, MPORT2, MPORT3, MPORT4, MPORT5,
    MPORT6, MPORT7, MPORT8, MPORT9, MPORT10, MPORT11 };

/* Liste des ports associes a chaque port dans les registres materiels */
/* (Cette table sert a reconstituer les registres 32 bits associes aux */
/*  ports en partant du numero d'un port unique).                      */
int listep[] = {
    LPORT0, LPORT1, LPORT2, LPORT3, LPORT4, LPORT5,
    LPORT6, LPORT7, LPORT8, LPORT9, LPORT10, LPORT11 };


/*                                                             */
/***************************************************************/



/* Variables utilisees pour le passage de parametres sous Linux */
/*    Remarque : ces variables devraient peut-etre */
/*               etre marquees "__initdata" ???    */

/* Numero majeur propose pour le pilote (si 0, choix laisse au systeme) */
static int majeur = 0;

/* Niveau d'affichage par defaut de printLog */
static int loglevel = DRV_INFO;

/* Type de logique (positive ou negative) */
int logiquePositive = 0;

module_param(majeur, int, S_IRUGO);
MODULE_PARM_DESC(majeur, "Numero de device majeur associe au module");

module_param(loglevel, int, S_IRUGO);
MODULE_PARM_DESC(loglevel, "Niveau d'affichage sur syslog\n"
                           "\t\t   1 ==> DEBUG\n"
                           "\t\t   2 ==> INFO (defaut)\n"
                           "\t\t   3 ==> WARNING\n"
                           "\t\t   4 ==> ERROR\n"
                           "\t\t   5 ==> FATAL");

module_param(logiquePositive, int, S_IRUGO);
MODULE_PARM_DESC(logiquePositive,
                 "Association entre niveau logique et tension\n"
                 "\t\t si 0 :   0 ==> 5V et 1 ==> 0V (defaut)\n"
                 "\t\t si 1 :   0 ==> 0V et 1 ==> 5V");

/* Description du module */
MODULE_AUTHOR("Yves GUILLEMOT");
MODULE_DESCRIPTION("Pilote " PCIE6509_NOM " v" __stringify(PCIE6509_VERSION)
                   "." __stringify(PCIE6509_REVISION) " du " PCIE6509_DATE);
#if	(LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0))
MODULE_SUPPORTED_DEVICE("Carte E/S TOR PCIe-6509 de National Instrument");
#endif


/*********************************************/
/* Declarations necessaires de fonctions ... */
/*********************************************/

static int
initialisation_carte(pcie6509Statics_t **pstat, int numero);

static int __init
pcie6509Init(void);

static void __exit
pcie6509Cleanup(void);

int
pcie6509Open(struct inode *inode, struct file *f);

int
pcie6509Close(struct inode *inode, struct file *f);

irqreturn_t
pcie6509Intr(int irq, void *dev_id);

/* Designation des fonctions d'initialisation et de desinstallation */
module_init(pcie6509Init);
module_exit(pcie6509Cleanup);

/*
** License du module
** ("GPL" evite un warning lors du chargement du module sous Linux,
**  mais, le 3/6/2005, la politique de TE641/EADS-ST concernant
**  cette license reste a definir  ... )
*/
MODULE_LICENSE("EADS-ST");

/* Points d'entree du driver */
static struct file_operations pcie6509Fops =
                        { owner:          THIS_MODULE,
                          open:           pcie6509Open,
                          release:        pcie6509Close,
                          unlocked_ioctl: pcie6509UnlockedIoctl,
                        };

/* Structure pour rechercher les cartes sur le bus PCI */
static struct pci_dev * pcie6509_pci_dev = NULL;












/*********************************************************************
 * Sous Linux, tous les devices doivent etre installes au cours de
 * l'unique appel de la fonction pcie6509Init() d'initialisation du
 * driver.
 *
 *********************************************************************/

static int __init pcie6509Init(void)
{
    int k, rslt;
    int cr;

    initPrintLog(SYSLOG, loglevel);
    printLog(DRV_INFO, "Debut pcie6509Init()\n");
    printLog(DRV_INFO, "Utilisation d'une logique %s.\n",
                       logiquePositive ? "positive" : "negative");


    /* Enregistrement du driver */
    rslt = register_chrdev(majeur, PCIE6509_NOM, &pcie6509Fops);

    if (rslt < 0)
      {
        printLog(DRV_FATAL, "Echec installation du driver %s v%d.%d\n",
                PCIE6509_NOM, PCIE6509_VERSION, PCIE6509_REVISION);
        if (rslt == -EINVAL)
          { printLog(DRV_FATAL,
                    "        ==> Le Majeur specifie est hors bornes\n");
          }
        if (rslt == -EBUSY)
          { printLog(DRV_FATAL,
                    "        ==> Le Majeur specifie est indisponible\n");
          }
        return rslt;
      }

    if (rslt > 0)
      {
        majeur = rslt;
      }

    printLog(DRV_INFO, "Debut installation du driver %s v%d.%d   -   Majeur = %d\n",
            PCIE6509_NOM, PCIE6509_VERSION, PCIE6509_REVISION, majeur);

    /* Initialisation a NULL des entrees de la table des donnees statiques */
    /* (pour pouvoir tester (dans open) la validite des mineurs utilises)  */
    pcie6509Nbre = 0;
    for (k = 0; k < (PCIE6509_MAX_CARTES); k++) pAllStat[k] = NULL;

    /* Recherche des cartes et allocation/initialisation des structures */
    /* associees chaque fois qu'une carte est trouvee.                  */
    for (k = 0, cr=0; k < (PCIE6509_MAX_CARTES); k++) {
        cr = initialisation_carte(&pAllStat[k], k);
        if (cr) break;
        pcie6509Nbre++;  /* Comptage des cartes trouvees */
    }

    /* Une valeur de cr = -ENODEV est normale (apres avoir trouve toutes  */
    /* les cartes existantes, on a constate qu'il n'en restait plus).     */
    /* Une valeur de cr = -OK indique que au moins PCIE6509_MAX_CARTES    */
    /* sont presentes.                                                    */
    /* Une autre valeur est caracteristique d'une erreur.                 */

    /* Desenregistrement du driver si une erreur a ete constatee. */
    if ((cr != -OK) && (cr != -ENODEV)) {
        unregister_chrdev(majeur, PCIE6509_NOM);
        printLog(DRV_FATAL,
                "Abandon de l'installation !\n");  
        return -EIO;
    }

    /* Desenregistrement du driver si aucune carte n'a ete trouvee */
    if (pcie6509Nbre == 0) {
        /* Aucune carte n'a ete trouve sur le bus PCI */
        /* ==> Desenregistrement du driver !            */
        unregister_chrdev(majeur, PCIE6509_NOM);
        printLog(DRV_ERROR,
                 "Aucun coupleur trouve   -   Echec de l'installation !\n");
        return -EIO;
    }

    /* Installation achevee */
    printLog(DRV_WARNING, "Fin installation du driver %s v%d.%d   -   Majeur = %d\n",
            PCIE6509_NOM, PCIE6509_VERSION, PCIE6509_REVISION, majeur);
  
    printLog(DRV_WARNING, "Nombre de devices PCIe-6509 = %d\n", pcie6509Nbre);
    return -OK;
}



/* Recherche la prochaine carte disponible sur le bus PCI (ou PCIe) */
/* et effectue les initialisations necessaires si elle est trouvee. */
/* Renvoie 0 si OK (carte trouvee), autre chose en cas d'erreur.    */
static int
initialisation_carte(pcie6509Statics_t **pstat, int numero)
{

    int ret;
    uint32_t val32;
    void *bvba;
    int isPciExpress;
    pcie6509Statics_t * bigPtr;
    int i;




    /* --------------------------------------------------
      -- Recherche de la carte sur le bus PCI ou PCIe --
      -------------------------------------------------- */
    printLog(DRV_INFO, "Recherche vendor/device 0x%04X/0x%04X : ",
                                  PCIE6509_VENDOR_ID, PCIE6509_DEVICE_ID);


    pcie6509_pci_dev = pci_get_device(PCIE6509_VENDOR_ID,
                                      PCIE6509_DEVICE_ID,
                                      pcie6509_pci_dev);

    if (pcie6509_pci_dev == NULL) {
          if (numero) printLog(DRV_INFO, "Plus de carte.\n");
          else        printLog(DRV_INFO, "ECHEC recherche carte !\n");
          return -ENODEV;
    }
    printLog(DRV_INFO, "OK : Carte trouvee\n");


    /* -----------------------------------
        --  Enabling PCIe Device Memory  --
        -----------------------------------*/

    ret = pci_enable_device(pcie6509_pci_dev);
    if (ret) {
        printLog(DRV_FATAL, "Echec pci_enable_device() !\n");
        return ret;
      }

    /* Lecture adresse physique de la carte */
    ret = pci_read_config_dword(pcie6509_pci_dev, PCIE6509_BAR, &val32);
    if (ret) {
        printLog(DRV_FATAL, "Echec lecture adresse physique coupleur !\n");
        return ret;
      }

    /* Mappage de la carte dans l'espace d'adressage virtuel */
    bvba = ioremap(val32, PCIE6509_BARSIZE);

    printLog(DRV_INFO, "Adresse virtuelle de la carte = 0x%0lX\n",
                        (unsigned long) bvba);
    printLog(DRV_INFO, "Adresse physique de la carte =  0x%0X\n", val32);

    /* Le bus utilise est-il "PCI standard" ou "PCI Express" ? */
    /* La fonction pci_is_pcie() n'existe que sur les tous derniers noyaux */
    /* (vers v3.8.13). Dans tous les cas, les memes fonctions doivent etre */
    /* utilisees.                                                          */
    /* Le but de cette determination etait de savoir si les interruptions  */
    /* devaient/pouvaient ou non etre passees en mode MSI.                 */
    /* De toutes facons, ce driver n'implemente pas les interruptions pour */
    /* le moment.                                                          */
#ifdef KERNEL_RECENT
    isPciExpress = pci_is_pcie(pcie6509_pci_dev);
    if (isPciExpress) {
        printLog(DRV_INFO, "Le coupleur utilise un bus PCI Express\n");
    } else {
        printLog(DRV_INFO, "Le coupleur utilise un bus PCI standard\n");
    }
#else
    isPciExpress = 0;
#endif    /* KERNEL_RECENT */





    /* ---------------------------------------------------------------
      -- Allocation memoire pour les variables statiques du driver --
      --------------------------------------------------------------- */

    bigPtr = vmalloc(sizeof(pcie6509Statics_t));
    if (bigPtr == NULL) {
        printLog(DRV_FATAL, "PCIE6509 : Echec allocation \"pcie6509Statics_t\"");
        return -ENOMEM;
    }


    /* Memorisation de l'adresse de base de la carte */
    bigPtr->h_drm = pcie6509_pci_dev;
    bigPtr->bpba = val32;
    bigPtr->bvba = bvba;
//    bigPtr->signal_number = numero;
    bigPtr->pcie = isPciExpress;
    bigPtr->msiStatus = -1;          /* Indicateur "non valide" */

    bigPtr->numero = numero;

    /* Initialisations du verrou */
    spin_lock_init(&bigPtr->lock);

    /* Initialisation des memoires de direction et de sortie des bits */
    for (i=0; i<12; i++) {
        bigPtr->dir[i] = 0;   /* 0 ==> Entree, 1 ==> Sortie */
        bigPtr->bit[i] = 0;   /* Bits en sortie */
    }

    /* On force les E/S materielles en mode entree (conformement au contenu */
    /* de la table bigPtr->dir[] initialisee ci-dessus)                     */
    writel(0, bigPtr->bvba + DIO_DIRECTION_P0_P3);
    writel(0, bigPtr->bvba + DIO_DIRECTION_P4_P5);
    writel(0, bigPtr->bvba + DIO_DIRECTION_P6_P7);
    writel(0, bigPtr->bvba + DIO_DIRECTION_P8_P11);



    /* Validation de l'utilisation en mode sortie sur les 32 voies pour */
    /* lesquelles cette operation est necessaire.                       */
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_0);
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_1);
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_2);
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_3);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_4);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_5);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_6);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P4_7);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_0);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_1);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_2);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_3);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_4);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_5);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_6);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P5_7);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_0);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_1);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_2);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_3);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_4);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_5);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_6);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P6_7);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_0);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_1);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_2);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_3);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_4);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_5);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_6);      
    writeb(PFI_OUTPUT_SELECT, bigPtr->bvba + PFI_SELOUT_P7_7);      


    /* Mise a 0 des registres de sortie materiels (pour conformite */
    /* avec le contenu de la memoire associee (utile ?))           */
    writel(0, bigPtr->bvba + DIO_OUTPUT_P0_P3);
    writel(0, bigPtr->bvba + DIO_OUTPUT_P4_P5);
    writel(0, bigPtr->bvba + DIO_OUTPUT_P6_P7);
    writel(0, bigPtr->bvba + DIO_OUTPUT_P8_P11);

    

    /* Inhibition toutes les interruptions et watchdog */
    // (devrait sans doute etre fait en premier lieu, avant
    //  toute autre operation...)
    // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
    
    

 /* ---------------------------------------------------------
    --   Declaration de la fonction de traitement des IT   --
    --------------------------------------------------------- */

//   if (request_irq(sbs1553_pci_dev->irq, asdcintr,
//                   IRQF_DISABLED | IRQF_SHARED, QABI_NOM, bigPtr)) {
//       KPRINTF("asdc: Echec utilisation IRQ %d\n", sbs1553_pci_dev->irq);
// 
//       /* Echec installation ==> Liberation memoire deja allouee */
//       /* liberation du device PCI */
// 
//       /// Correct ???
//       iounmap(bvba);
//       pci_disable_device(sbs1553_pci_dev);
//       vfree(bigPtr);
// 
//       return -EIO;
//   }


  /* Renvoi de l'adresse des donnees statiques */
  *pstat = bigPtr;

  return -OK;	/* Tout s'est bien passe ! */
}




/*****************************/
/* Desinstallation du driver */
/*****************************/

static void __exit pcie6509Cleanup(void)
{
  int i;
  pcie6509Statics_t * varg;


  printLog(DRV_INFO, "Dechargement du driver %s et des devices associes :\n",
           PCIE6509_NOM);

  
  /*
  ** Les operations sont effectuees dans l'ordre inverse
  ** de celui utilise a l'installation.
  */
  

  /* Liberation des structures de donnees associees a chaque carte */
  for (i = PCIE6509_MAX_CARTES - 1; i >= 0; i--)
    {
      printLog(DRV_DEBUG, "i=%d  pAllStat[i]=%p\n", i, pAllStat[i]);
      if (pAllStat[i] != NULL)
        {

          /* Pointeur vers la carte a traiter */
          varg = pAllStat[i];
          printLog(DRV_DEBUG,
                   "Debut liberation du device %d [statics = 0x%p]\n", i, varg);


//           /* Desenregistrement des gestionnaires d'interruptions */
//           free_irq(varg->irq, asdc_p[i]);
//           KPRINTF(" - IRQ [%d] liberee\n", i);
// 
//           /* Liberation du pool de wait queues */
//           KPRINTF(" Liberation du pool de Wait_queues\n");
//           wq_liberer(&varg->wqPool);
// 
//           /* Arret du mode MSI */
//           if (varg->msiStatus == 0) {
//               pci_disable_msi(varg->h_drm);
//           }

          printLog(DRV_DEBUG, " Liberation des adresses virtuelles PCI\n");
          iounmap(varg->bvba);

          /* Liberation du device (Utile ? : c'est une fonction vide !) */
          pci_disable_device(varg->h_drm);


          /* Liberation de  la memoire allouee a la carte */
          printLog(DRV_DEBUG, " Liberation de la memoire allouee au coupleur\n");
          vfree(varg);

          pAllStat[i] = NULL;
        }
    }

  /* Desenregistrement du driver aupres du systeme */
  printLog(DRV_DEBUG, " Desenregistrement du driver : (ne peut echouer !)\n");
  unregister_chrdev(majeur, PCIE6509_NOM);
}





/* -----------------------------------------
  --                                      --
  --              pcie6509Open            --
  --                                      --
  ----------------------------------------- */

int pcie6509Open(struct inode *inode, struct file *f)
{
  int mineur;

  mineur = MINOR(inode->i_rdev);
  
  if ((mineur < 0) || (mineur >= PCIE6509_MAX_CARTES))
    {
      printLog(DRV_ERROR, "PCIE6509 : le mineur %d utilise est hors bornes !\n", mineur);
      return -EINVAL;
    }

  if (pAllStat[mineur] == NULL)
    {
      printLog(DRV_ERROR, "PCIE6509 : mineur %d : pas de carte associee !\n", mineur);
      return -EINVAL;
    }
    
    printLog(DRV_DEBUG,
             "PCIE6509 [%d] : Appel de open()\n", MINOR(inode->i_rdev));

    /* Et maintenant que la validite du mineur a ete duement */
    /* verifiee, on ne fait plus rien !   :-)  :-)  :-)      */
  return -OK;
}






/* -----------------------------------------------
   --                                           --
   --           pcie6509Close                   --
   --                                           --
   ----------------------------------------------- */

/* ATTENTION : N'est appele qu'au DERNIER close() [SunOS, LynxOS] */

  int pcie6509Close(struct inode *inode, struct file *f)
  {
    printLog(DRV_DEBUG,
             "PCIE6509 [%d] : Appel de close()\n", MINOR(inode->i_rdev));
    return -OK;
  }








/*****************************************************/
/* Fonctions non implementees :                      */
/*      asdc_read()                                  */
/*      asdc_write()                                 */
/*      asdc_select()                                */
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

// irqreturn_t asdcintr(int irq, void *dev_id  /*, struct pt_regs *reg_ptr*/ )
// {
//    abiStatics_t * bigPtr;
// 
//    /* Le premier coupleur 1553 est imperativement present    */
//    /*		 ==> appel immediat de la fonction concernee */
//    bigPtr = (abiStatics_t *) dev_id;
//    asdcintr_coupleur(bigPtr);
// 
//    /* Existe-t-il un second coupleur 1553 sur la carte PMC ? */
//    if (bigPtr->autreCoupleur)
//      {
//        /* Coupleur double ==> appel de la fonction du second coupleur */
//        bigPtr = bigPtr->autreCoupleur;
//        asdcintr_coupleur(bigPtr);
//      }
// 
//    /* Pour le moment, on ne tente pas de repondre autre chose ... */
//    /* Ne devrait pas poser Pb puisque, si ligne partagee, une IT  */
//    /* peut en cacher une autre !                                  */
//    return IRQ_HANDLED;
// }




