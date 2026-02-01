
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_SA_GETNBM et ASDC_SA_SETNBM : Fonctions ioctl d'acces au nombre
          de mots legal d'une sous-adresse.

 QUAND      QUI   QUOI
---------- ----  -------------------------------------------------------------
15/04/2013  YG   Version initiale
26/04/2013  YG   Correction erreur dans position du champ "nombre de mots"
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"




/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


/* Lecture du nombre de mots legal d'une sous-adresse simulee  */
/* Le champ asdcvoie.nmots est utilise pour passer ce nombre   */
/* Attention : La valeur 0 correspond a 32 mots                */
long
do_ASDC_SA_GETNBM(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
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
    v->sous_adresse &= 0x1F;
    v->direction &= 0x01;

    /* Table des sous-adresses */
    j = L(L(ATPTR) + v->adresse + 32*v->bus);

    /* Table des filtres */
    k = L(L(FTPTR) + v->adresse + 32*v->bus);

    if ((j == 0) || (k == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Lecture du nombre de mots */
    v->nmots = L(j + v->sous_adresse + (v->direction ? 32 : 0) + 64);
    v->nmots &= 0x1F;

    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));


    RETURN(OK);
}




/* Programmation du nombre de mots legal d'une sous-adresse simulee  */
/* Le champ asdcvoie.nmots est utilise pour passer ce nombre         */
/* Attention : La valeur 0 correspond a 32 mots                      */
long
do_ASDC_SA_SETNBM(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
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
    v->nmots &= 0x1F;

    /* Table des sous-adresses */
    j = L(L(ATPTR) + v->adresse + 32*v->bus);

    /* Table des filtres */
    k = L(L(FTPTR) + v->adresse + 32*v->bus);

    if ((j == 0) || (k == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

    /* Ecriture du nombre de mots */
    E(j + v->sous_adresse + (v->direction ? 32 : 0) + 64, v->nmots);

    RETURN(OK);
}

