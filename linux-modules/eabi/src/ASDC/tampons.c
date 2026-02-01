
/*

 DRIVER ASDC POUR EMUABI, PLACE AU-DESSUS DU DRIVER VFX70

Fonctions d'acces aux tampons utilises par les FIFOs qui memorisent
les donnees ecrites par les applications dans les sous-adresses en
mode emission.



 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 3/04/2014  YG   Version initiale - incluse dans le driver d'un espion RS422
12/06/2014  YG   Adaptation au driver EMUABI
13/06/2014  YG   Rassemblement des includes dans un seul fichier
27/01/2015  YG   Inhibition des interruptions pendant les acces a la FIFO des
                 dernieres donnees ecrites (et les acces au pool des tampons)

*/



#include "driverIncludes.h"



/* Reinitialisation complete d'une FIFO */
void
viderCompletementFifo(fifoTampons_t * fifo)
{
    tampon_t * pt;

    for (;;) {
        pt = lireFifo(fifo);
        if (pt == NULL) break;
        /* printk("viderCompletementFifo, avant rendre(pt), pt=%p\n", pt); */
        rendre(pt);
    }
}


/* Reinitialisation d'une FIFO en conservant le dernier tampon emis */
void
viderFifo(fifoTampons_t * fifo)
{
    tampon_t * pt;

    for (;;) {
        pt = lireFifoPrincipale(fifo);
        if (pt == NULL) break;
        /* printk("viderFifo, avant rendre(pt), pt=%p\n", pt); */
        rendre(pt);
    }
}


/* Prendre un nouveau tampon dans le pool des tampons libres. */
/* Renvoi l'adresse du tampon ou NULL si plus de tampons.     */
tampon_t *
prendre(void)
{
    tampon_t * p;
    unsigned long s;

    spin_lock_irqsave(pasdcPoolLock, s);
        if (asdcFreePool == NULL) {
            p = NULL;
        } else {
            p = asdcFreePool;
            asdcFreePool = p->s;
            p->s = NULL;
            asdcNbrePool--;
        }
    spin_unlock_irqrestore(pasdcPoolLock, s);
    return p;
}


/* Remettre un tampon dans le pool des tampons libres. */
/* Un parametre null est ignore.                       */
void
rendre(tampon_t * tampon)
{
    unsigned long s;
    
    if (tampon == NULL) return;
    spin_lock_irqsave(pasdcPoolLock, s);
        tampon->s = asdcFreePool;
        asdcFreePool = tampon;
        asdcNbrePool++;
    spin_unlock_irqrestore(pasdcPoolLock, s);
}


/* Prendre le premier tampon disponible dans une FIFO.     */
/* "fifo" pointe la structure d'ancrage de la FIFO.        */
/* Le tampon lu vient remplacer le "dernier tampon emis".  */
/* La fonction renvoie l'adresse du "dernier tampon emis"  */
/* precedent ou NULL s'il n'existait pas.                  */
tampon_t *
lireFifo(fifoTampons_t * fifo)
{
    tampon_t *p, *q;
    unsigned long s;

    spin_lock_irqsave(&fifo->lock, s);
        if (fifo->p == NULL) {
            p = NULL;
        } else {
            p = fifo->p;
            fifo->p = p->s;
            if (p->s == NULL) fifo->d = NULL;
            p->s = NULL;
            fifo->nbt--;
        }

        q = fifo->demis;
        fifo->demis = p;
    spin_unlock_irqrestore(&fifo->lock, s);
    return q;
}


/* Prendre le premier tampon disponible dans une FIFO en   */
/* ignorant le "dernier tampon emis".                      */
/* "fifo" pointe la structure d'ancrage de la FIFO.        */
/* Le tampon lu vient remplacer le "dernier tampon emis".  */
/* La fonction renvoie l'adresse du tampon lu ou NULL s'il */
/* n'existait pas.                                         */
/* Le "dernier tampon emis" reste inchange.                */
tampon_t *
lireFifoPrincipale(fifoTampons_t * fifo)
{
    tampon_t * p;
    unsigned long s;

    spin_lock_irqsave(&fifo->lock, s);
        if (fifo->p == NULL) {
            p = NULL;
        } else {
            p = fifo->p;
            fifo->p = p->s;
            if (p->s == NULL) fifo->d = NULL;
            p->s = NULL;
            fifo->nbt--;
        }
    spin_unlock_irqrestore(&fifo->lock, s);
    return p;
}


/* Prendre le premier tampon disponible dans une FIFO.     */
/* "fifo" pointe la structure d'ancrage de la FIFO.        */
/* Si le tampon lu n'est pas nul, il vient remplacer le    */
/* "dernier tampon emis" et c'est l'adresse de ce dernier  */
/* qui est renvoyee par la fonction.                       */
/* Si le tampon lu est nul, le "dernier tampon emis" reste */
/* inchange et la fonction renvoie NULL.                   */
/*                                                         */
/* ATTENTION : Cette fonction est destinee mettre a jour   */
/*             "demis" dans l'isr. Pour cette raison, elle */
/*             ne fait pas appel au lock de la FIFO, qui   */
/*             est cense avoir ete utilise par la fonction */
/*             appelante.                                  */
tampon_t *
decalerFifo_unlocked(fifoTampons_t * fifo)
{
    tampon_t *p, *q;

    if (fifo->p == NULL) {
        p = NULL;
    } else {
        p = fifo->p;
        fifo->p = p->s;
        if (p->s == NULL) fifo->d = NULL;
        p->s = NULL;
        fifo->nbt--;
    }

    if (p != NULL) {
        q = fifo->demis;
        fifo->demis = p;
    } else {
        q = p;
    }
    return q;
}


/* Placer un tampon à la fin d'une  FIFO.           */
/* "fifo" pointe la structure d'ancrage de la FIFO. */
void
ecrireFifo(fifoTampons_t * fifo, tampon_t * tampon)
{
    unsigned long s;

    spin_lock_irqsave(&fifo->lock, s);
        tampon->s = NULL;
        if (fifo->p == NULL) {
            fifo->d = tampon;
            fifo->p = tampon;
        } else {
            fifo->d->s = tampon;
            fifo->d = tampon;
        }
        fifo->nbt++;
    spin_unlock_irqrestore(&fifo->lock, s);
}


/* Cette fonction essaye de reprendre le dernier tampon d'une FIFO */
/* afin de completer son contenu.                                  */
/* Cette operation n'est autorisee que si la FIFO contient au      */
/* moins deux tampons (pour etre certain qu'une eventuelle tache   */
/* de lecture va bien trouver au moins un tampon) et que si cet    */
/* eventuel dernier tampon n'est pas totalement plein.             */
/* Si aucun tampon ne peut etre rappele, la fonction renvoi NULL.  */
#ifdef PLUS_TARD_PEUT_ETRE
tampon_t *
rappelerFifo(fifoTampons_t * fifo)
{
    tampon_t * p;

    spin_lock_irqsave(&fifo->lock, s);
    if (fifo->nbt < 2) {
        p = NULL;
    } else {
        if (fifo->d->nbre == TCARGO) {
            p = NULL;
        } else {
            p = fifo->d;
            fifo->d = p->p;   /// NE FONCTIONNE PAS : IL FAUDRAIT UN DOUBLE
                              /// CHAINAGE QUI N'EST PAS IMPLEMENTE
                              /// ACTUELLEMENT !
            p->p = NULL;
            fifo->nbt--;
        }
    }
    spin_unlock_irqrestore(&fifo->lock, s);
    return p;
}
#endif  /* PLUS_TARD_PEUT_ETRE */






