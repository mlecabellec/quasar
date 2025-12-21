
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_RT_GETSTS et ASDC_RT_SETSTS : Fonctions ioctl d'acces au mot
                                   de status 1553 d'un abonne.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
22/04/2013  YG   Version initiale
26/04/2013  YG   Correction d'une erreur d'indirection
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"




/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


/* Lecture du mot de status d'un abonne simule     */
/* Le champ asdcvoie.nmots est utilise pour passer */
/* ce mot de status                                */
long
do_ASDC_RT_GETSTS(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int j, k;

    // printk("asdcioctl.c entree dans case ASDC_RT_GETTREP\n");
    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;

    /* Table des sous-adresses */
    j = L(ATPTR) + v->adresse + 32*v->bus;

    /* Pointeur status */
    k = L(SWTPTR) + v->adresse + 64*v->bus;

    if ((L(j) == 0) || (L(k) == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Lecture du mot de status */
    v->nmots = L(k);
    v->nmots &= 0xFFFF;       /* C'est un mot de 16 bits */

    printk("GETSTS: k=0x%X  sts=0x%X ==> 0x%X\n", k, L(k), v->nmots);

    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));


    RETURN(OK);
}



/* Programmation du mot de status d'un abonne simule */
/* Le champ asdcvoie.nmots est utilise pour passer   */
/* ce mot de status                                  */
long
do_ASDC_RT_SETSTS(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int j, k;

    // printk("asdcioctl.c entree dans case ASDC_RT_GETTREP\n");
    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->nmots &= 0xFFFF;       /* Mot de status : 16 bits */

    /* Table des sous-adresses */
    j = L(ATPTR) + v->adresse + 32*v->bus;

    /* Pointeur status */
    k = L(SWTPTR) + v->adresse + 64*v->bus;

    if ((L(j) == 0) || (L(k) == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Ecriture du mot de status                                    */
    /* (on force a 1 le bit erreur pour avoir mot de status non nul */
    E(k, v->nmots | 0x0400);

    printk("SETSTS: k=0x%X  sts=0x%X\n", k, L(k));

    RETURN(OK);
}

