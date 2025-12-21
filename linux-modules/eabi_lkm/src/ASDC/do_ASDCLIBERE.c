
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCDEF : Fonction ioctl de liberation de la memoire d'echange.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
12/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCLIBERE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    int *int_ptr, zint_ptr;
    int n, a;

    TDUTIL(arg, sizeof(int));
    DUTIL(zint_ptr, int_ptr, arg, sizeof(int));

    /* - Les 16 bits de poids fort pointes par int_ptr contiennent   */
    /*   la taille du bloc a liberer                                 */
    /* - Les 16 bits de poids faible pointes par int_ptr contiennent */
    /*   l'adresse du bloc a liberer                                 */

    a = *int_ptr & 0xFFFF;              /* Adresse du bloc */
    n = (*int_ptr >> 16) & 0xFFFF;      /* Taille du bloc */

    if (asdclibere(dst, a, n) == -1) RETURN(EADDRNOTAVAIL);

    /* Il n'y a pas eu de probleme ! */
    RETURN(OK);  
}

