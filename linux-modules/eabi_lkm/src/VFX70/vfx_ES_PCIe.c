
/* 

Ce fichier definit des fonctions d'E/S PCIe protegees par un verrou (le
"bigEmuabiLock") de facon a interdire les E/S PCIe simultanees vers la
memoire d'echange.

Le but recherche est de comprendre, et, mieux, de corriger, le crash observe
sur l'ISF en decembre 2014.

Le probleme suppose etant vraisemblablement lie aux acces PCIe plutot qu'aux
acces a la memoire partages, tous les acces PCIe seront proteges.
Comme ces acces sont pratiquement tous des E/S 32 bits, seules des fonctions
d'acces 32 bits sont definies ci-dessous.

L'exception : quelques acces 16 bits sont effectues dans le handler
d'interruptions isr_pmcvfx(). Ces acces seront proteges directement (et,
si possible, d'une facon globale) dans cette fonction.


QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
12/01/2015  YG   Version initiale
26/02/2015  YG   Acces a la memoire double port via PCI-BAR2 + 0x200000 en
                 utilisant un nouveau composant VHDL qui donne acces au "port
                 droit" de la memoire.
*/


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


#include "vfx_ctl.h"
#include "vfxvarg.h"
#include "vfxExport.h"



/* L'argument "it" des fonctions ci-dessous et un indicateur "interruption */
/* en cours" qui permet de selectionner le mode de lock approprie.         */

/* Ce symbole valide l'affichage d'un message sur le log systeme chaque fois */
/* qu'un acces au "port gauche" de la RAM d'echange est effectue.            */
/* Depuis le firmware v2.1, la RAM est utilisee en mode simple port : son    */
/* "port droit" est partage entre le bus PCI et le powerPC au travers d'un   */
/* composant VHDL qui implemente un arbitre de bus (priorite au powerPC).    */
/* Le "port gauche" (accessible via PCI-BAR1) ne doit donc plus etre utilise */
/* (sauf, le cas echeant, pour investiguer sur des problemes materiels) et   */
/* l'apparition d'un de ces messages indique l'oubli, quelque part dans le   */
/* logiciel, de l'application de la nouvelle methode d'acces a la RAM.       */
#define MONTRER_ACCES_LEFT_PORT

unsigned int
lecturePCIe(int vfxNum, int it, void * adresse)
{
    int valeur;
    vfxvarg_t * varg;
    
    varg = pvargs[vfxNum];
    
    if (it) {
        valeur = readl(adresse);
    } else {
        spin_lock_irqsave(&varg->bigEmuabiLock, varg->bigLockFlags);

        valeur = readl(adresse);
    
        spin_unlock_irqrestore(&varg->bigEmuabiLock, varg->bigLockFlags);
    }

#ifdef MONTRER_ACCES_LEFT_PORT
    if (    ((unsigned long)adresse >= (unsigned long)varg->base1)
         && ((unsigned long)adresse < (unsigned long)varg->base1 + 0x200000)) {
        printk("lecturePCIe @RAM(0x%0lX) --> 0x%0X\n",
               ((unsigned long)adresse - (unsigned long)varg->base1) >> 2, valeur);
    }
#endif

    return valeur;
}


void
ecriturePCIe(int vfxNum, int it, unsigned int valeur, void * adresse)
{
    vfxvarg_t * varg;

    varg = pvargs[vfxNum];

#ifdef MONTRER_ACCES_LEFT_PORT
    if (   ((unsigned long)adresse >= (unsigned long)varg->base1)
        && ((unsigned long)adresse < (unsigned long)varg->base1 + 0x200000)) {
        printk("ecriturePCIe 0x%0X --> @RAM(0x%0lX)\n",
               valeur, ((unsigned long)adresse - (unsigned long)varg->base1) >> 2);
    }
#endif

    if (it) {
        writel(valeur, adresse);
    } else {
        spin_lock_irqsave(&varg->bigEmuabiLock, varg->bigLockFlags);

        writel(valeur, adresse);

        spin_unlock_irqrestore(&varg->bigEmuabiLock, varg->bigLockFlags);
    }
}


unsigned int
lectureRamPCIe(int vfxNum, int it, unsigned int adresse)
{
    unsigned int valeur;
    vfxvarg_t * varg;
    
    varg = pvargs[vfxNum];

    if (!it) {
        spin_lock_irqsave(&varg->bigEmuabiLock, varg->bigLockFlags);
    }

        /* TODO : Ce spinlock a-t-il encore une utilite ?
         *   ==> Apparemment oui : le supprimer entraine un crash kernel.
         */

    /* "adresse" est l'indice du mot dans une table d'entier.
     * Il faut la convertir en adressage "par octet", d'ou le decalage de
     * deux bits.
     */
    valeur = readl((void *) varg->base2 + 0x200000 + (adresse << 2));
        
    if (!it) {
        spin_unlock_irqrestore(&varg->bigEmuabiLock, varg->bigLockFlags);
    }
    
//    printk("ram(0x%X) : 0x%08X\n", adresse, valeur);
    return valeur;
}


void
ecritureRamPCIe(int vfxNum, int it, unsigned int valeur, unsigned int adresse)
{
    vfxvarg_t * varg;

    varg = pvargs[vfxNum];
    
    if (!it) {
        spin_lock_irqsave(&varg->bigEmuabiLock, varg->bigLockFlags);
    }

        /* TODO : Ce spinlock a-t-il encore une utilite ?
         *   ==> Apparemment oui : le supprimer entraine un crash kernel.
         */
 
    /* "adresse" est l'indice du mot dans une table d'entier.
     * Il faut la convertir en adressage "par octet", d'ou le decalage de
     * deux bits.
     */
    writel(valeur, (void *) varg->base2 + 0x200000 + (adresse << 2));
    
    if (!it) {
        spin_unlock_irqrestore(&varg->bigEmuabiLock, varg->bigLockFlags);
    }
}





