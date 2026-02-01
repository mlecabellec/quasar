
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCEPAR et SDCLPAR : Fonctions ioctl d'acces aux parametres du driver.


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


    /* Ecriture de la table des parametres du driver   */
    /*   Cette fonction ecrit directement la structure */
    /*   asdcparam                                     */
    /* ATTENTION : Les parametres ainsi ecrits ne      */
    /*             seront pris en compte qu'a la       */
    /*             prochaine execution de              */
    /*             ioctl(ASDCRAZ)                      */

long
do_ASDCEPAR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcparam *p, zp;

    /* DEBUT DE LA PARTIE SPECIFIQUE ASDCEPAR */

    TDUTIL(arg, sizeof(struct asdcparam));
    DUTIL(zp, p, arg, sizeof(struct asdcparam));

    dst->asdcdef = *p;

    RETURN(OK);
}



    /* Lecture de la table des parametres du driver  */
    /*   Cette fonction lit directement la structure */
    /*   asdcparam                                   */

long
do_ASDCLPAR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcparam *p, zp;

    /* DEBUT DE LA PARTIE SPECIFIQUE ASDCLPAR */

    TVUTIL(zp, p, arg, sizeof(struct asdcparam));

    *p = dst->asdcdef;

    /* Retour des donnees a l'appli appelante */
    VUTIL(zp, p, arg, sizeof(struct asdcparam));
    RETURN(OK);
}

