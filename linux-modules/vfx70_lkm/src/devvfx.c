
/*
{+D}
   SYSTEM:		Acromag Pmcvfx

   MODULE
	NAME:		devvfx.c

   VERSION:		A

   CREATION
	DATE:		04/01/09

   CODED BY:	FJM

   ABSTRACT:	apmcvfx device.

   CALLING
	SEQUENCE:

   MODULE TYPE:

   I/O RESOURCES:

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:

   REVISIONS:

 DATE      BY       PURPOSE
---------- ----  ---------------------------------------------------------------
15/07/2009  LH   Modification de la fonction "open" pour permettre
                 a plusieurs applications d'acceder a la carte de
                 maniere simultanee

20/06/2012  YG   Fusion du vieux driver SunOS de la carte ASDC avec le driver
                 Linux de la carte Acromag VFX70

20/06/2012  YG   Deplacement de la fonction ioctl dans un fichier specifique

12/04/2013  YG   Adaptation au nouveau driver en 3 modules CEVT/ASDC/VFX70. Ce
                 nouveau driver accepte plusieurs cartes VFX70 sur la meme
                 machine et des applications differentes, associees a des
                 modules noyau differents, sur les differentes cartes VFX70.

17/04/2013  YG   Correction du bug qui empechait la connexion a l'interruption.
                 (Le 5eme argument passe a request_irq() est suppose etre un
                 pointeur et ne doit pas etre nul (verification faite par l'OS)
                 ==> le passage de l'indice du device (en lieu et place du
                 pointeur attendu) echouait pour le premier device avec
                 indice = 0.
                 Solution : on passe un pointeur sur la structure statique du
                 device. Cette structure possede elle meme un champ contenant
                 l'indice du device pour le cas ou la fonction d'interruption en
                 aurait besoin.)

16/07/2014  YG   - Suppression inclusion inutile de fichiers <asm/xxx.h>
                 - Suppression declaration d'une cariable inutilisee

  {-D}
*/


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	/* controls conditional inclusion in file pmcvxmulticommon.h */
   /// ==> Ne devrait plus servir a rien !!!
#endif

/* Permet d'affecter les variables globales a cette unite de compilation */
/* et non aux autres pour lesquelles elles sont externes.                */
/* Doit etre defini dans un des fichiers du module et dans un seul.      */
#define PRINCIPAL

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/stringify.h>
# if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0) 
#  include <linux/vmalloc.h>
# endif


// #include "../h/pmcvxmulticommon.h"

#include "vfx_ctl.h"
#include "vfxvarg.h"
#include "vfxExport.h"
#include "version.h"

#define MAJOR_NUM         46

#ifndef KERNEL
#define KERNEL
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
# define ioremap_nocache ioremap
#endif


/* Prototype des fonctions ioctl() */
#ifdef OLDIOCTL
int
vfx_ioctl(struct inode *inode, struct file *fp,
          unsigned int cmd, unsigned long arg);
#else
long
vfx_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
#endif





/*#define KERNEL_DBG_OUTPUT*/


/* ///////////////////////////////////////////////////////////////// */
/* Select CPU type that corresponds to your hardware.                */
/* See "../pmcvlvs.h" for selection.   Default is A32.               */
/* ///////////////////////////////////////////////////////////////// */

struct scatterVFXlist
{
    uint32_t		PCI_address;
    uint32_t		PCI_address_Hi;
    uint32_t		length;
    uint32_t		Nxt_Desc_Ptr;
    uint32_t		Local_address;
    uint32_t		offset;
    struct page		*page;
#ifndef BUILDING_FOR_A64
    uint32_t		alignment;			/* keep structure aligned */
#endif
};



/* ///////////////////////////////////////////////////////////////// */
/* Select board, array type, and device name by using the defines    */
/* that correspond to your hardware.                                 */
/* See "../pmcvfx.h" for selection.   Default is Pmcvfx70.           */
/* ///////////////////////////////////////////////////////////////// */

static int countOfVfx70 = 0;  /* Nombre de cartes VFX70 sur le bus PCI */


int ret_val = 0;

/* !  Les variables globales ci-dessous sont declarees dans vfxvarg.h */
int open_dev[MAX_PMCS];
unsigned int board_irq[MAX_PMCS];
unsigned long pmc_address[MAX_PMCS];
unsigned long pmc_address1[MAX_PMCS];
unsigned long pmc_address2[MAX_PMCS];
struct pci_dev *pvfxBoard[MAX_PMCS];
vfxvarg_t *pvargs[MAX_PMCS];      /* Table des structures statiques */


/* ! Les variables globales ci-dessous NE SONT PAS declarees dans vfxvarg.h */
struct page **pages[MAX_PMCS];
unsigned int nr_pages[MAX_PMCS];
struct scatterVFXlist *sgl0[MAX_PMCS];			/* PCI to LOCAL transfers */
struct scatterVFXlist *sgl1[MAX_PMCS];			/* LOCAL to PCI transfers */

struct file;
struct inode;
struct pt_regs;

static int
open( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;

   if( minor > (MAX_PMCS - 1))
	   return( -ENODEV );
  
//    if( open_dev[minor] )
// 	   return( -EBUSY );

   open_dev[minor]++;

// printk("OPEN FAIT : open_dev[%d] = %d\n", minor, open_dev[minor]);
   return( 0 );
} 


static int
release( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;

   if( minor > (MAX_PMCS - 1))
	   return( -ENODEV );

   if( open_dev[minor] )
   {
	   open_dev[minor]--;
	   return( 0 );
   }
   return( -ENODEV );

   return 0;
}



static ssize_t
read( struct file *fp, char *buf, size_t length, loff_t *offset )
{
    unsigned long adata, ldata;
    unsigned short sdata;
    unsigned char cdata;

    get_user( adata, (unsigned long *)buf );   /* pickup address */
    switch( length ) {
        case 1:	/* 8 bit */
            cdata = readb( (void *) adata );
            ldata = ( unsigned long )cdata;    /* convert to long */
        break;
        case 2:	/* 16 bit */
            sdata = readw( (void *) adata );
            ldata = ( unsigned long )sdata;    /* convert to long */
        break;
        case 4:	/* 32 bit */
            ldata = readl( (void *) adata );
        break;

        /* Read 32 bit from configuration space */
        case 0x40: {     /* Read 32 bit from configuration space */
            struct inode *inode;
            int unit;

# if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
            inode = fp->f_dentry->d_inode;
# else
            inode = fp->f_path.dentry->d_inode;
# endif
            unit = MINOR(inode->i_rdev);

            /* pickup EE address */
            get_user( adata, (unsigned long *)adata );

            /* read config space */
            pci_read_config_dword( pvfxBoard[unit], (int)adata, (u32*)&ldata );
            break;
        }

        default:
            cdata = sdata = adata = ldata = 0;
            return( -EINVAL );
        break;
    }

    /* update user data */
    put_user( ldata,(unsigned long *)( buf + (sizeof(unsigned long)) ) );
    return( length );
}



static ssize_t
write( struct file *fp, const char *buf, size_t length, loff_t *offset )
{
    unsigned long adata, ldata;

    /* pickup address */
    get_user( adata, (unsigned long *)buf );
    /* and pickup data */
    get_user( ldata, (unsigned long *)( buf + (sizeof(unsigned long))) );

    switch( length ) {
        case 1:	/* 8 bit */
            writeb( (int)ldata, (void *)adata );
        break;
        case 2:	/* 16 bit */
            writew( (int)ldata, (void *)adata );
        break;
        case 4:	/* 32 bit */
            writel( (int)ldata, (void *)adata );
        break;


        case 0x40: {      /* Write 32 bit to configuration space */
            struct inode *inode;
            int unit;

# if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
            inode = fp->f_dentry->d_inode;
# else
            inode = fp->f_path.dentry->d_inode;
# endif
            unit = MINOR(inode->i_rdev);

            /* pickup EE address */
            get_user( adata, (unsigned long *)adata );

            /* write config space */
            pci_write_config_dword(pvfxBoard[unit], (int)adata, (u32)ldata);
            break;
        }

        default:
            return( -EINVAL );
        break;
    }
    return( length );
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#define NULLMEMBER
#define NULLLASTMEMBER
#else
#define NULLMEMBER NULL,
#define NULLLASTMEMBER NULL
#endif


static struct file_operations pmcvfxboard_ops = {
  .owner = THIS_MODULE, /* owner of the world */
  NULLMEMBER                 /* seek */
  .read = read,         /* read */
  .write = write,       /* write */
  NULLMEMBER                 /* readdir */
  NULLMEMBER                 /* poll */
#ifdef OLDIOCTL
  .ioctl = vfx_ioctl,   /* ioctl */
#else
  .unlocked_ioctl = vfx_ioctl,   /* ioctl */
#endif
  NULLMEMBER                 /* mmap */
  .open = open,         /* open */
  NULLMEMBER                 /* flush */
  .release = release,   /* release */
  NULLMEMBER                 /* fsync */
  NULLMEMBER                 /* fasync */
  NULLMEMBER                 /* lock */ 
  NULLMEMBER                 /* readv */
  NULLMEMBER                 /* writev */
  NULLMEMBER                 /* sendpage */
  NULLLASTMEMBER                  /* get_unmapped_area */
};


static int
apmcvfxboard_handler( int irq, void *did, struct pt_regs *cpu_regs )
{ 
//  volatile uint32_t lValue;
//  volatile word wValue;
//  volatile VX_DMA_MEMORY_MAP* pVx;
  int int_status;
//  int i;

  int unit;
  int etat;
  void * base0;

  unit = ((vfxvarg_t *)did)->unit;
  base0 = (void *) pmc_address[unit];

  // printk("*** apmcvfxboard_handler : IRQ=%d   unit=%d\n", irq, unit);
  // printk("*** apmcvfxboard_handler : Base0 = 0x%p\n", base0);

  etat = readw((void *) base0) & 0xFFFF;
  if (etat) {
      writel(0, (void *) (base0 + 8)); // Arret des ITs a venir
      writew(0xF, (void *) base0);     // Acquittement ITs recues (utile ici ?)
  } else {
    return IRQ_NONE;
  }
  // printk("*** apmcvfxboard_handler : VFX70 !\n");

//   printk("IT bloquees, on continue tranquillement !\n");

    /* Appel de la fonction de traitement */
    if (isr_pmcvfx(unit) == 0) {
        /* La fonction d'interruption du driver utilisateur a pu etre */
        /* executee ==> L'interruption devrait avoir ete acquittee et */
        /* le mecanisme des interruptions peut etre rearme.           */
        writel(0x80000000, (void *) (pmc_address[unit] + 8));
        // printk("Revalid\n");
    }


        int_status = 1;


   return IRQ_HANDLED;


#if 0
   int_status = 0;	/* indicate interrupt not handled */

   for(i = 0; i < MAX_PMCS; i++)        /* check all boards */
   {
     if( open_dev[i] )          	/* if board is open */
     {
		pVx = ( VX_DMA_MEMORY_MAP* )pmc_address[i];

		lValue = readl((unsigned long*)&pVx->GlobalInterruptEnable);

		if( lValue & BoardInterruptEnable )       /* interrupts enabled */
		{
		  wValue = readw((void *)&pVx->InterruptStatus);

// 		  if( wValue & DMACh0InterruptPending ) /* DMA 0 interrupt */
// 		  {
// 
// printk("\nPmcvfx DMA0_Isr HwdAdr=%lX\n",(unsigned long)pmc_address[i]);
// 
// 			/* writing asserted bit back to the register clears the interrupt */
// 			writew( (int)DMACh0InterruptPending, (void *)&pVx->InterruptStatus );
// 			int_status = 1;
// 		  }
// 
// 	      if( wValue & DMACh1InterruptPending ) /* DMA 1 interrupt */
// 		  {
// 
// printk("\nPmcvfx DMA1_Isr HwdAdr=%lX\n",(unsigned long)pmc_address[i]);
// 
// 			/* writing asserted bit back to the register clears the interrupt */
// 			writew( (int)DMACh1InterruptPending, (void *)&pVx->InterruptStatus );
// 			int_status = 1;
// 		  }

	      if( wValue &  FGPAInterruptPending )  /* FPGA interrupt */
		  {
                        int xn;
			isr_pmcvfx(i);
			int_status = 1;

                        /* Rearmement des interruptions ASDC    */
                        /* (0x8014 : IntEnable)                 */
                        /* [Est-ce vraiment le bon endroit ???] */
//                     xn = readl(pmc_address1[unit] + 8);
//                     if (xn < 10) {   /// SYSTEME ANTIGEL
//                         writew(1, pmc_address2[unit] + 0x8014, 1);
//                         xn++;
//                         writel(xn, pmc_address1[unit] + 8, xn);
//                     }
		  }
		}
	 }
   }

   if( int_status )
		return( IRQ_HANDLED);
   else
		return( IRQ_NONE);

#endif   // "#if 0"
}


int
init_module( void )
{
  extern struct pci_dev *pvfxBoard[MAX_PMCS];
  struct pci_dev *pvfx;
  int i,j;
  char devnamebuf[32];
  char devnumbuf[8];


  memset( &pages[0], 0, sizeof(pages));
  memset( &nr_pages[0], 0, sizeof(nr_pages));
  memset( &sgl0[0], 0, sizeof(sgl0));
  memset( &sgl1[0], 0, sizeof(sgl1));

  memset( &pvfxBoard[0], 0, sizeof(pvfxBoard));
  memset( &open_dev[0], 0, sizeof(open_dev));
  memset( &board_irq[0], 0, sizeof(board_irq));
  memset( &pmc_address[0], 0, sizeof(pmc_address));
  memset( &pmc_address1[0], 0, sizeof(pmc_address1));
  memset( &pmc_address2[0], 0, sizeof(pmc_address2));

  /* Recherche des cartes VFX70 presentes sur le bus PCI (a partir des */
  /* numeros VEBDOR_ID et PMCVFXBOARD)                                 */
  pvfx = NULL;
  for( i = 0, j = 0; i < MAX_PMCS; i++ ) {

 int cr;
      /*  Use pci_find_device() for earlier versions FC3/4/5/6 */
      /* pvfx = (struct pci_dev *)pci_find_device(VENDOR_ID, PMCVFXBOARD, pvfx); */

      printk("VFX - Debut boucle install i=%d\n", i);
      pvfx = ( struct pci_dev *)pci_get_device( VENDOR_ID, PMCVFXBOARD, pvfx ); 
	if( pvfx ) {
	    pvfxBoard[i] = pvfx;
	    
            printk("VFX trouvee (irq=%d)\n", pvfx->irq);
            cr = pci_enable_device(pvfx);
            printk("PCI enabled cr=%d\n", cr);
	    
            // cr = pci_enable_msi(pvfx);
            // printk("MSI enabled cr=%d\n", cr);
            printk("New irq = %d\n", pvfx->irq);

            /* Get BAR0 region */
	    pmc_address[i] = (unsigned long)pvfxBoard[i]->resource[0].start;	
	    pmc_address[i] = (unsigned long)ioremap_nocache( pmc_address[i], PMC_VX_BAR0_MAP_SIZE); /* no cache! */

            /* Get BAR1 region */
	    pmc_address1[i] = (unsigned long)pvfxBoard[i]->resource[1].start;
	    pmc_address1[i] = (unsigned long)ioremap_nocache( pmc_address1[i], PMC_VX_BAR1_MAP_SIZE); /* no cache! */

            /* Get BAR2 region */
	    pmc_address2[i] = (unsigned long)pvfxBoard[i]->resource[2].start;
	    pmc_address2[i] = (unsigned long)ioremap_nocache( pmc_address2[i], PMC_VX_BAR2_MAP_SIZE); /* no cache! */
printk("VFX70 - Apres ioremap_nocache() (x3)\n");
	    if (    (pmc_address[i] != 0)
                 && (pmc_address1[i] != 0)
                 && (pmc_address2[i] != 0)
               ) {
                memset( &devnamebuf[0], 0, sizeof(devnamebuf));
                memset( &devnumbuf[0], 0, sizeof(devnumbuf));
                strcpy(devnamebuf, DEVICE_NAME);
                sprintf(&devnumbuf[0],"%d",i);
                strcat(devnamebuf, devnumbuf);
                board_irq[i] = pvfxBoard[i]->irq;

                /* Allocation de la structure statique du device d'indice i */
                pvargs[i] = vmalloc(sizeof(vfxvarg_t));
                if (pvargs[i] == NULL) {
                    printk("VFX70 unit=%d : Echec allocation de la "
                           "structure statique\n", i);
                    /* TODO : Normalement, il faudrait liberer les structures */
                    /*        deja allouees, etc...                           */
                    /*        Cependant, si on a deja un probleme             */
                    /*        d'allocation memoire, le systeme est en plutot  */
                    /*        mauvaise sante et ca n'en vaut probablement pas */
                    /*        la peine !                                      */
                    return ENOMEM;
                }

                /* Initialisation de cette structure */
                pvargs[i]->unit = i;
                pvargs[i]->userNum = -1;
                pvargs[i]->up_isr = NULL;
                pvargs[i]->up_ioctl = NULL;
                pvargs[i]->base0 = pmc_address[i];     /* Preparation futur  */
                pvargs[i]->base1 = pmc_address1[i];    /* retrait des tables */
                pvargs[i]->base2 = pmc_address2[i];    /* pmc_address...     */


                printk("VFX70 : Connexion IRQ %d au device #%d\n",
                        board_irq[i], i);
                ret_val = request_irq(board_irq[i],
                                      (irq_handler_t)apmcvfxboard_handler,
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
                                      IRQF_DISABLED | IRQF_SHARED,
#else
                                      IRQF_SHARED,
#endif
                                      "VFX70", (void *)(pvargs[i]));

                          /* ATTENTION : Le passage d'une valeur nulle comme */
                          /* dernier argument de request_irq() interdit le   */
                          /* fonctionnement de cette fonction qui renvoi un  */
                          /* code d'erreur. Il faut donc imperativement      */
                          /* passer un pointeur (pvarg[i]) vers les donnees  */
                          /* du device et non pas un indice (i) conduisant   */
                          /* a ces memes donnees sous peine de ne pas        */
                          /* pouvoir se connecter a l'interruption pour i=0. */
                if (ret_val) {
                    printk("VFX70 : Echec connexion IRQ %d. CR=%d [0x%X]\n",
                            board_irq[i], ret_val, ret_val);

                    /* TODO : Liberer la memoire deja allouee !!! */
                    return -EIO;
                }

                printk("VFX70 : i=%d j=%d : IRQ=%d requested\n",
                        i, j, board_irq[i]);

                printk("%s mapped   BAR0=%08lX BAR1=%08lX\n"
                        "            BAR2=%08lX IRQ=%d\n",
                        devnamebuf,
                        (unsigned long)pmc_address[i],
                        (unsigned long)pmc_address1[i],
                        (unsigned long)pmc_address2[i], board_irq[i]);

// disable_irq(board_irq[i]);
// enable_irq(board_irq[i]);
//
// printk("enable_irq : IRQ #%d enabled\n", board_irq[i]);

                j++;   /* Comptage des cartes trouvees */
            }
        } else {
            break;
        }
  }

  if( j )	/* found at least one device */
  {
      ret_val = register_chrdev ( MAJOR_NUM, DEVICE_NAME, &pmcvfxboard_ops );

      /* Memorisation du nombre de cartes trouvees */
      countOfVfx70 = j;
      printk("Nombre de cartes VFX70 trouvees : %d\n", countOfVfx70);

      printk("NOUVEAU DRIVER \"%s\" INSTALLE !\n", DEVICE_NAME);

	if( ret_val < 0)
	{
	    printk(DEVICE_NAME);
	    printk(" Failed to register error = %d\n", ret_val);
	}
	else
		return( 0 );
  }
  return( -ENODEV );
}




void
cleanup_module( void )
{
    char devnamebuf[32];
    char devnumbuf[8];
    int i;

    if( ret_val >= 0 ) {
        unregister_chrdev( MAJOR_NUM, DEVICE_NAME );
        for( i = 0; i < MAX_PMCS; i++ ) {
            if( pmc_address1[i] ) {
                memset( &devnamebuf[0], 0, sizeof(devnamebuf));
                memset( &devnumbuf[0], 0, sizeof(devnumbuf));
                strcpy(devnamebuf, DEVICE_NAME);
                sprintf(&devnumbuf[0],"%d",i);
                strcat(devnamebuf, devnumbuf);
                // free_irq(board_irq[i], (void *)pmc_address1[i]);
                printk("VFX free_irq irq=%d i=%d\n", board_irq[i], i);
                // free_irq(board_irq[i], (void *) ((long) i));
                free_irq(board_irq[i], (void *) pvargs[i]);
                iounmap( (void *)pmc_address2[i] );
                iounmap( (void *)pmc_address1[i] );
                iounmap( (void *)pmc_address[i] );
                printk("%s unmapped BAR0=%08lX BAR1=%08lX BAR2=%08lX\n",
                       devnamebuf, (unsigned long)pmc_address[i],
                       (unsigned long)pmc_address1[i],
                       (unsigned long)pmc_address2[i]);
                pci_disable_device(pvfxBoard[i]);
                printk("VFX - pci_disable_device called\n");
                open_dev[i] = 0;
                memset(pvargs[i], 0, sizeof(vfxvarg_t));
                vfree(pvargs[i]);
                pvargs[i] = 0;
            }
        }
    }
}


MODULE_AUTHOR("FJM (Acromag)");
MODULE_AUTHOR("Louis Herve (Astrium)");
MODULE_AUTHOR("Yves Guillemot (Astrium)");
MODULE_DESCRIPTION("Pilote " VFX_NOM " v" __stringify(VFX_VERSION) "."
                    __stringify(VFX_REVISION) " du " VFX_DATE);
#if	(LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0))
MODULE_SUPPORTED_DEVICE("Carte Acromag VFX70");
#endif
MODULE_LICENSE("GPL and additional rights");

/***************************************************/
/* Interface vers les couches drivers specialisees */
/***************************************************/

/* Renvoi le nombre de cartes VFX70 trouvees sur le bus PCI */
int
VFX70_getNumberOfCards(void)
{
    return countOfVfx70;
}

/* Renvoi le mot caracteristique de l'application FPGA chargee sur */
/* la carte VFX70 dont l'indice a ete passe en argument.           */
/* Ce mot est situe a l'adresse PCI_BAR2 + 0x9000                  */
int32_t
VFX70_getApplicationDescriptor(unsigned int vfxUnit)
{
    unsigned long adata;
    unsigned int ldata;

    /* Controle de la validite du numero de l'instance VFX70 */
    if ((vfxUnit >= MAX_PMCS) || (pvargs[vfxUnit] == NULL)) {
        printk("VFX70_getApplicationDescriptor(Unit = %ud) : unit hors bornes !\n",
               vfxUnit);
        return 0;
    }

    adata = pmc_address2[vfxUnit] + 0x9000;
    ldata = readl( (void *) adata );

    printk("VFX70 Unit = %d   App Descriptor = 0x%08X\n", vfxUnit, ldata);
    return (int) ldata;
}

/* Renvoi les adresses de base des trois plages du bus PCI */
void
VFX70_getBaseAddresses(unsigned int vfxNum, void ** pcibar0,
                       void ** pcibar1, void ** pcibar2)
{
    /* Controle de la validite du numero de l'instance VFX70 */
    if ((vfxNum >= MAX_PMCS) || (pvargs[vfxNum] == NULL)) {
        printk("VFX70_getBaseAddresses(Unit = %ud) : unit hors bornes !\n",
               vfxNum);
        *pcibar0 = 0;
        *pcibar1 = 0;
        *pcibar2 = 0;
        return;
    }

    *pcibar0 = (void *) pvargs[vfxNum]->base0;
    *pcibar1 = (void *) pvargs[vfxNum]->base1;
    *pcibar2 = (void *) pvargs[vfxNum]->base2;
 }

/* Cree les liens entre les fonctions definies dans le driver FPGA et les */
/* appels effectues dans le driver VFX70.                                 */
int
VFX70_registerUserDriver(unsigned int vfxNum, unsigned int userNum,
                         long (* up_ioctl)(int vfx, int asdc, struct file *fp,
                                           unsigned int cmd, unsigned long arg),
                         void (* up_isr)(int unit))
{
    /* Controle de la validite du numero de l'instance VFX70 */
    if (vfxNum >= MAX_PMCS) return -ENODEV;
    if (pvargs[vfxNum] == NULL) return -ENODEV;

    /* Enregistrement */
    pvargs[vfxNum]->userNum = userNum;
    pvargs[vfxNum]->up_isr = up_isr;
    pvargs[vfxNum]->up_ioctl = up_ioctl;

    return 0;
}

/* Supprime les liens avec un driver utilisateur */
int
VFX70_unregisterUserDriver(unsigned int vfxNum)
{
    /* Controle de la validite du numero de l'instance VFX70 */
    if (vfxNum >= MAX_PMCS) return -ENODEV;
    if (pvargs[vfxNum] == NULL) return -ENODEV;

    /* Enregistrement */
    pvargs[vfxNum]->userNum = -1;
    pvargs[vfxNum]->up_isr = NULL;
    pvargs[vfxNum]->up_ioctl = NULL;

    return 0;
}

/* Exportation des fonctions utilisees pour communiquer avec d'autres drivers */
EXPORT_SYMBOL(VFX70_getNumberOfCards);
EXPORT_SYMBOL(VFX70_getApplicationDescriptor);
EXPORT_SYMBOL(VFX70_registerUserDriver);
EXPORT_SYMBOL(VFX70_getBaseAddresses);
EXPORT_SYMBOL(VFX70_unregisterUserDriver);



