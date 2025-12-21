
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCDEF : Fonction ioctl d'allocation en memoire d'echange.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
12/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"


/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCALLOUE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int *int_ptr, zint_ptr;
    int n;

    TDUTIL(arg, sizeof(int));
    DUTIL(zint_ptr, int_ptr, arg, sizeof(int));
    TVUTIL(zint_ptr, int_ptr, arg, sizeof(int));

    /* Les 16 bits de poids faible pointes par int_ptr contiennent */
    /* la taille du bloc a allouer                                 */

    n = asdcalloc(dst, *int_ptr & 0xFFFF);

    /* L'allocation s'est elle correctement deroulee ? */
    if ( n == -1) RETURN(ENOMEM);

    /* Il n'y a pas eu de probleme ! */

    *int_ptr = n & 0xFFFF;
    /* Les 16 bits de poids faible pointes par int_ptr contiennent */
    /* l'adresse du bloc alloue                                    */

    TVUTIL(zint_ptr, int_ptr, arg, sizeof(int));
    RETURN(OK);
}

