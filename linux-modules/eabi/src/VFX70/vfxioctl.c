
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
---------- ----  ------------------------------------------------
15/07/2009  LH   Modification de la fonction "open" pour permettre
                 a plusieurs applications d'acceder a la carte de
                 maniere simultanee

20/06/2012  YG   Fusion du vieux driver SunOS de la carte ASDC avec le driver
                 Linux de la carte Acromag VFX70

20/06/2012  YG   Deplacement de la fonction ioctl dans un fichier specifique

24/01/2013  YG   Ajout ioctl ASDCTESTIT (pour test seulement)
14/03/2013  YG   Correction ioctl ASDCTESTIT (le mauvais tampon etait lu)
22/03/2013  YG   Modif. pour compatibilite Linux recents
22/03/2013  YG   Modif. pour compatibilite Linux recents
10/04/2013  YG   Separation du driver VFX70 du driver utilisateur
                 du FPGA configure.
16/07/2014  YG   Suppression inclusion inutile de fichiers <asm/xxx.h>

  {-D}
*/


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	/* controls conditional inclusion in file pmcvxmulticommon.h */
#endif


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


//#include "../h/pmcvxmulticommon.h"

#include "vfx_ctl.h"
#include "vfxvarg.h"
#include "version.h"

#define MAJOR_NUM	46

// #include "asdc.h"
// #include "asdcvarg.h"
// #include "version.h"
// #include "interface.h"
// #include "asdcwq.h"

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



extern unsigned int board_irq[MAX_PMCS];
extern unsigned long pmc_address[MAX_PMCS];
extern unsigned long pmc_address1[MAX_PMCS];
extern unsigned long pmc_address2[MAX_PMCS];


#define OK 0


#ifdef OLDIOCTL
int
vfx_ioctl(struct inode *inode, struct file *fp,
          unsigned int cmd, unsigned long arg)
#else
long
vfx_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
#endif
{
    extern struct page **pages[MAX_PMCS];
    extern struct scatterVFXlist *sgl0[MAX_PMCS];  /* PCI to LOCAL transfers */
    extern struct scatterVFXlist *sgl1[MAX_PMCS];  /* LOCAL to PCI transfers */
    extern unsigned int nr_pages[MAX_PMCS];

#ifndef OLDIOCTL
    struct inode *inode;
#endif

    unsigned long ldata, gdata, idata;
    unsigned char *ddata;
    unsigned long count;
    int res, i, j;

    /* Variables specifiques des fonctions ioctl ASDC */
    int unit;
    unsigned long baseRam;


#ifndef OLDIOCTL
# if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    inode = fp->f_dentry->d_inode;
# else
    inode = fp->f_path.dentry->d_inode;
# endif
#endif

    unit = MINOR(inode->i_rdev);

    baseRam = (unsigned long)pmc_address1[unit];  /* Mem. echange : PCI_BAR1 */

    /* Fin des variables specifiques ASDC */

/* DEBUG */

    // printk("IOCTL : unit=%d cmd=0x%X\n", unit, cmd);

    switch( cmd )
    {
        case 0: /* reserved for future use */
        case 1:
        case 2:

        case VFX_GET_PCIBAR2 :
            ldata = ( unsigned long )pmc_address2[unit];  /* convert to long */
            put_user(ldata, (unsigned long *)arg);        /* update user data */
        return -OK;

        case VFX_FLUSH_CACHE :
            flush_cache_all();
        return -OK;

        case VFX_GET_PCIBAR0 : /* return PMC_VX_BAR0 address */
            ldata = (unsigned long)pmc_address[unit];  /* convert to long */
            put_user(ldata, (unsigned long *)arg);     /* update user data */
        return -OK;

        case VFX_GET_IRQ : /* return IRQ number */
            ldata = (unsigned long)board_irq[unit];   /* convert IRQ to long */
            put_user(ldata, (unsigned long *)arg);    /* update user data */
        return -OK;

        case VFX_GET_PCIBAR1 : /* return PMC_VX_BAR1 address1 */
            ldata = (unsigned long)pmc_address1[unit];  /* convert to long */
            put_user(ldata, (unsigned long *)arg);      /* update user data */
        return -OK;

        case VFX_DMA_V2P : /* return virt_to_phys address of DMA buffer */

           gdata = (unsigned long ) arg;
           get_user( ldata, (unsigned long *)gdata );		/* pickup PCI address */
           get_user( count, (unsigned long *)(gdata+(1*(sizeof(unsigned long)))) );	/* pickup size */
           get_user( gdata, (unsigned long *)(gdata+(2*(sizeof(unsigned long)))) );	/* pickup LOCAL address */
           ddata = (unsigned char *)gdata;
           get_user( idata, (unsigned long *)(arg+(3*(sizeof(unsigned long)))) );	/* pickup instance index */

#ifdef KERNEL_DBG_OUTPUT
printk("\nLdata=%lX Ddata=%lX Count=%lX index=%lX",(unsigned long)ldata, (unsigned long)ddata,
				(unsigned long)count,(unsigned long)idata );
#endif
           if (count == 0)				/* No count */
           {
                printk("\nCount  0? ");
                return -OK;
           }
           if ((ldata + count) < ldata)			/* Overflow! */
           {
                printk("\nAddress Overflow!");
                return -EINVAL;
           }
           nr_pages[idata] = ((ldata & (~PAGE_MASK)) + count + (~PAGE_MASK)) >> PAGE_SHIFT;
   if (nr_pages[idata] > MAX_MEMORY_PAGES )	/* Too big */
           {
                printk("\nRequested pages nr_pages[%lX] > MAX_MEMORY_PAGES ", (unsigned long)idata);
                return -ENOMEM;
           }

           /* request/obtain kernel memory for the physical DMA buffer */
           if ((pages[idata] = kmalloc(MAX_MEMORY_PAGES * sizeof(*pages), GFP_KERNEL | __GFP_DMA)) == NULL)
           {
                printk("\nkmalloc_0- No memory");
                return -ENOMEM;
           }

           /* Request/obtain kernel memory for the scatter/gather linked list used for PCI to LOCAL transfers */
           /* If we fail here, we also must free the previously obtained kernel memory for the physical DMA buffer */
           if ((sgl0[idata] = kmalloc( sizeof( struct scatterVFXlist[MAX_MEMORY_PAGES] ), GFP_KERNEL | __GFP_DMA)) == NULL)
           {
                kfree(pages[idata]);
                printk("\nkmalloc_1 - No memory");
                return -ENOMEM;
           }

           /* Request/obtain kernel memory for the scatter/gather linked list used for LOCAL to PCI transfers */
           /* If we fail here, we also must free the previously obtained kernel memory for the physical DMA buffer */
           /* We must also free the previously obtained PCI to LOCAL scatter/gather list */
           if ((sgl1[idata] = kmalloc( sizeof( struct scatterVFXlist[MAX_MEMORY_PAGES] ), GFP_KERNEL | __GFP_DMA)) == NULL)
           {
                kfree(sgl0[idata]);
                kfree(pages[idata]);
                printk("\nkmalloc_2 - No memory");
                return -ENOMEM;
           }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
           down_read(&current->mm->mmap_lock);	       /* Try to fault in all of the necessary pages */
#else
           down_read(&current->mm->mmap_sem);	       /* Try to fault in all of the necessary pages */
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
           res = get_user_pages(
                        ldata,
                        nr_pages[idata],
                        0, /* don't force */
                        pages[idata],
                        NULL);
#else
           res = get_user_pages(
                        current,
                        current->mm,
                        ldata,
                        nr_pages[idata],
                        1,		/*rw == 1 means read from drive, write into memory area */
                        0, /* don't force */
                        pages[idata],
                        NULL);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
           up_read(&current->mm->mmap_lock);
#else
           up_read(&current->mm->mmap_sem);
#endif

           if (res < nr_pages[idata])			/* Errors and no page mapped */
       {
             if (res > 0)
             {
                        for (j=0; j < res; j++)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
                                put_page(pages[idata][j]);
#else
                                page_cache_release(pages[idata][j]);
#endif
             }
             kfree(sgl1[idata]);
             kfree(sgl0[idata]);
             kfree(pages[idata]);
             printk("\nNo pages mapped");
             return res;
		   }
#ifdef KERNEL_DBG_OUTPUT
printk("\nUser pages res=%X ldata=%lX nr_pages=%X index=%lX",
		   (unsigned int)res,(unsigned long)ldata,(unsigned int)nr_pages[idata], (unsigned long) idata);
#endif

             /* flush */
             for (i=0; i < nr_pages[idata]; i++)
                          flush_dcache_page(pages[idata][i]);

             /* Populate the scatter/gather lists. A list is maintained for DMA transfers in each direction */
             /* NOTE: Only one DMA buffer is allocated and two scatter/gather lists */
             /* The same DMA buffer is used for transfers in either direction */
             /* One for PCI to LOCAL (sgl0) and One for LOCAL to PCI (sgl1) */
             sgl0[idata][0].page  = sgl1[idata][0].page  = pages[idata][0]; 
             sgl0[idata][0].offset = sgl1[idata][0].offset = ldata & ~PAGE_MASK;

             /* get kernel virtual address */
             ldata = (unsigned long)(kmap( sgl0[idata][0].page ) + sgl0[idata][0].offset );

#ifdef KERNEL_DBG_OUTPUT
printk("\nKernel Virtual addr Page[0]=%lX data %lX", (unsigned long)ldata , (unsigned long)*(long*)ldata );
#endif

             /* convert kernel virtual address to physical address for DMA controller use */
             sgl0[idata][0].PCI_address = sgl1[idata][0].PCI_address = (uint32_t)__pa(ldata);
             sgl0[idata][0].PCI_address_Hi = sgl1[idata][0].PCI_address_Hi = 0;

#ifdef KERNEL_DBG_OUTPUT
printk("\nKernel Physical addr Page[0]=%X", (unsigned int)sgl0[idata][0].PCI_address );
#endif

             /* insert local destination address */
             sgl0[idata][0].Local_address = sgl1[idata][0].Local_address = (uint32_t)((unsigned long)ddata);	/* destination address */


#ifdef KERNEL_DBG_OUTPUT
printk("\nDestination addr Page[0]=%X", (unsigned int)sgl0[idata][0].Local_address );
#endif

             /* The "next descriptor pointer" is the main reason there is a list for each transfer direction */
             /* It holds the next descriptor address */
             /* In this example the descriptor location is in PCI address space. */
             /* set up the first list */
             sgl0[idata][0].Nxt_Desc_Ptr = 0;								 /* list in PCI address space */ 

             /* set up the second list */
             sgl1[idata][0].Nxt_Desc_Ptr = 0;								 /* list in PCI address space */ 

             if (nr_pages[idata] > 1)
             {
                  sgl0[idata][0].length = sgl1[idata][0].length = PAGE_SIZE - sgl0[idata][0].offset;
                  count -= sgl0[idata][0].length;
                  ddata += sgl0[idata][0].length;								/* increment destination address */

                  /* update (sgl0) transfers scatter chain for mutiple pages */
                  sgl0[idata][0].Nxt_Desc_Ptr = (uint32_t) __pa( &sgl0[idata][1].PCI_address );	/* next link (physical address) in chain */

                  /* update (sgl1) transfers scatter chain for mutiple pages */
                  sgl1[idata][0].Nxt_Desc_Ptr = (uint32_t) __pa( &sgl1[idata][1].PCI_address );	/* next link (physical address) in chain */

#ifdef KERNEL_DBG_OUTPUT
printk("\nPage[0]=%lX offset =%X length =%X count = %X",
		(unsigned long)sgl0[idata][0].page, (unsigned int)sgl0[idata][0].offset,
		(unsigned int)sgl0[idata][0].length, (unsigned int)count);
#endif

                  for (i=1; i < nr_pages[idata] ; i++)
                  {
                          sgl0[idata][i].offset = sgl1[idata][i].offset = 0;
                          sgl0[idata][i].page = sgl1[idata][i].page = pages[idata][i];
                          sgl0[idata][i].length = sgl1[idata][i].length = count < PAGE_SIZE ? count : PAGE_SIZE;
                          count -= PAGE_SIZE;

                          /* update (sgl0) transfers scatter chain for mutiple pages */
                          sgl0[idata][i].Nxt_Desc_Ptr = (uint32_t) __pa( &sgl0[idata][i+1].PCI_address );	/* next links (physical address) in chain */

                          /* update (sgl1) transfers scatter chain for mutiple pages */
                          sgl1[idata][i].Nxt_Desc_Ptr = (uint32_t) __pa( &sgl1[idata][i+1].PCI_address );	/* next links (physical address) in chain */

                          /* get kernel virtual address */
                          ldata = (unsigned long)kmap( sgl0[idata][i].page );

#ifdef KERNEL_DBG_OUTPUT
printk("\nKernel Virtual addr Page[%X]=%X data %X",i, (unsigned int)ldata , (unsigned int)*(long*)ldata );
#endif

                          /* convert kernel virtual address to physical address for DMA controller use */
                          sgl0[idata][i].PCI_address = sgl1[idata][i].PCI_address = (uint32_t)__pa(ldata);
                          sgl0[idata][i].PCI_address_Hi = sgl1[idata][i].PCI_address_Hi = 0;

#ifdef KERNEL_DBG_OUTPUT
printk("\nKernel Physical addr Page[%X]=%X", i, sgl0[idata][i].PCI_address );
#endif

                          /* insert local address */ 
                          sgl0[idata][i].Local_address =sgl1[idata][i].Local_address = (uint32_t)((unsigned long)ddata);

#ifdef KERNEL_DBG_OUTPUT
printk("\nDestination addr Page[%X]=%X",i ,sgl0[idata][i].Local_address );
#endif

                          ddata += sgl0[idata][i].length;				/* increment local address */

#ifdef KERNEL_DBG_OUTPUT
printk("\nPage[%X]=%lX offset =%X length =%X count = %lX", i, (unsigned long)sgl0[idata][i].page,
				sgl0[idata][i].offset, sgl0[idata][i].length, count);
#endif

                  }

                  /* mark the last link in each scatter list as the end by setting DMA_CHANNEL_DESCRIPTOR_END */
                  sgl0[idata][i-1].Nxt_Desc_Ptr |= DMA_CHANNEL_DESCRIPTOR_END;	/* end of chain */ 
                  sgl1[idata][i-1].Nxt_Desc_Ptr |= DMA_CHANNEL_DESCRIPTOR_END;	/* end of chain */ 
             }
             else
             {
                  sgl0[idata][0].Nxt_Desc_Ptr |= DMA_CHANNEL_DESCRIPTOR_END;	/* end of chain */ 
                  sgl1[idata][0].Nxt_Desc_Ptr |= DMA_CHANNEL_DESCRIPTOR_END;	/* end of chain */
                  sgl0[idata][0].length = sgl1[idata][0].length = count;
             }

#ifdef KERNEL_DBG_OUTPUT
for (i=0; i < nr_pages[idata]; i++)
{
printk("\n\nsgl0[%X].PCI_address          = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].PCI_address, sgl0[idata][i].PCI_address);
printk("\nsgl0[%X].Local_address        = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].Local_address, sgl0[idata][i].Local_address);
printk("\nsgl0[%X].length address       = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].length, sgl0[idata][i].length);
printk("\nsgl0[%X].Nxt_Desc_Ptr addr    = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].Nxt_Desc_Ptr, sgl0[idata][i].Nxt_Desc_Ptr);
printk("\nsgl0[%X].page address         = %lX  Value = %lX",
		i,(unsigned long)&sgl0[idata][i].page, (unsigned long)sgl0[idata][i].page);
printk("\nsgl0[%X].offset address       = %lX  Value = %X\n",
		i,(unsigned long)&sgl0[idata][i].offset, sgl0[idata][i].offset);
}
#endif

           /* Return in user's arg[0] the address of the PCI to LOCAL transfer scatter/gather list */
           ldata = (unsigned long )&sgl0[idata][0].PCI_address;	/* get address of scatter/gather list */
           ldata = __pa(ldata);					/* convert kernel virtual address to physical address */
           put_user( ldata, (unsigned long *)arg );		/* update user data */

           /* Return in user's arg[1] the address of the LOCAL to PCI transfer scatter/gather list */
           ldata = (unsigned long )&sgl1[idata][0].PCI_address;	/* get address of scatter/gather list */
           ldata = __pa(ldata);					/* convert kernel virtual address to physical address */
           put_user( ldata, (unsigned long *)(arg+(1*(sizeof(unsigned long)))) );		/* update user data */
        return -OK;

        case 9:/* unmap and free scatter/gather list DMA buffer */
           gdata = (unsigned long ) arg;
           get_user( idata, (unsigned long *)gdata );		/* pickup instance index */
#ifdef KERNEL_DBG_OUTPUT
printk("\nindex =%X",(unsigned int)idata);
#endif
             if( pages[idata])					/* Release the mappings and exit *///vfxasdcioctl.c
             {
                  for (i=0; i < nr_pages[idata] ; i++)
                  {
#ifdef KERNEL_DBG_OUTPUT
printk("\n\nsgl0[%X].PCI_address          = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].PCI_address, sgl0[idata][i].PCI_address);
printk("\nsgl0[%X].Local_address        = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].Local_address, sgl0[idata][i].Local_address);
printk("\nsgl0[%X].length address       = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].length, sgl0[idata][i].length);
printk("\nsgl0[%X].Nxt_Desc_Ptr addr    = %lX  Value = %X",
		i,(unsigned long)&sgl0[idata][i].Nxt_Desc_Ptr, sgl0[idata][i].Nxt_Desc_Ptr);
printk("\nsgl0[%X].page address         = %lX  Value = %lX",
		i,(unsigned long)&sgl0[idata][i].page, (unsigned long)sgl0[idata][i].page);
printk("\nsgl0[%X].offset address       = %lX  Value = %X\n",
		i,(unsigned long)&sgl0[idata][i].offset, sgl0[idata][i].offset);
#endif
                      flush_dcache_page( sgl0[idata][i].page );
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
                      put_page( sgl0[idata][i].page );
#else
                      page_cache_release( sgl0[idata][i].page );
#endif
                      kunmap( sgl0[idata][i].page );
              }
              kfree(sgl1[idata]);
              kfree(sgl0[idata]);
              kfree(pages[idata]);
              pages[idata] = NULL;
         }
      return -OK;


        case VFX_LECVERSION : {
            /* Lecture de la version du pilote */

            struct vfxver *ve, zve;
            char *p, *q;
            ve = &zve;
    
            /* Copie du nom du pilote */
            p = VFX_NOM;
            q = ve->nom;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie de la version du pilote */
            p = __stringify(VFX_VERSION) "." __stringify(VFX_REVISION);
            q = ve->version;
            while (*p) { *q++ = *p++; }
            *q = '\0';

            /* Copie de la date du pilote */
            p = VFX_DATE;
            q = ve->date;
            while (*p) { *q++ = *p++; }
            *q = '\0';


            /* Copie des version et revision sous forme numerique */
            ve->vfxver = VFX_VERSION;
            ve->vfxrev = VFX_REVISION;

            /* Envoi a l'application de la structure renseignee */
            if (copy_to_user((void *)arg, (void *)ve, sizeof(struct vfxver))) {
                printk("vfx70 : Echec de copy_to_user !\n");
                return -EFAULT;
            }

            return -OK;
        }


        default :
           // printk("VFX70 ioctl(0x%X) ==> passage au suivant\n", cmd);

           /* Appel de la fonction ioctl utilisateur si elle existe */
           if (pvargs[unit]->up_isr) {
               return (*pvargs[unit]->up_ioctl)(unit,
                                                pvargs[unit]->userNum,
                                                fp, cmd, arg);
           }

           /* Si cette fonction n'existe pas : commande non trouvee ! */
           return -EINVAL;
        }

        /* Erreur qui ne peut pas arriver !!! */
        printk("vfx_ioctl : Ce message n'aurait jamais du etre ecrit !\n");
        return -ENOTTY;
}
