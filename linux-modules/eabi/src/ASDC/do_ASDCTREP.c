
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_RT_GETTREP et ASDC_RT_SETTREP : Fonctions ioctl d'acces au temps de
          reponse d'un abonne.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
15/04/2013  YG   Version initiale
26/04/20123 YG   Correction d'une erreur d'indirection
 4/06/20123 YG   Correction d'une autre erreur d'indirection !
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"




/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


/* Lecture du temps de reponse d'un abonne simule  */
/* Le champ asdcvoie.nmots est utilise pour passer */
/* le temps en unites de 100 ns                    */
long
do_ASDC_RT_GETTREP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
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

    /* Table des filtres */
    k = L(FTPTR) + v->adresse + 32*v->bus;

    if ((L(j) == 0) || (L(k) == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Lecture du temps de reponse */
    v->nmots = L(j + 64);
    v->nmots &= 0x3FF;       /* Temps de reponse limite a 12 bits */

    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));


    RETURN(OK);
}



/* Programmation du temps de reponse d'un abonne simule  */
/* Le champ asdcvoie.nmots est utilise pour passer le    */
/* temps en unites de 100 ns                             */
long
do_ASDC_RT_SETTREP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
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
    v->nmots &= 0x3FF;       /* Temps de reponse : limite a 12 bits */

    /* Table des sous-adresses */
    j = L(ATPTR) + v->adresse + 32*v->bus;

    /* Table des filtres */
    k = L(FTPTR) + v->adresse + 32*v->bus;

    if ((L(j) == 0) || (L(k) == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Ecriture du temps de reponse */
    E(j + 64, v->nmots);
    RETURN(OK);
}

