
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_IMAGE_LIRE et ASDC_IMAGE_ECRIRE : Fonction ioctl d'acces a
la "memoire image" de la memoire d'echange du coupleur.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDC_IMAGE_LIRE(int vfxNum, int asdcNum,
                   struct asdc_varg *dst, unsigned long arg)
{

    struct sbs1553_ioctl *arguments, zarguments;

    long *long_ptr, zlong_ptr;
    int offset;


    /* DEBUT DE LA PARTIE SPECIFIQUE ASDC_IMAGE_LIRE */

    TDUTIL(arg, sizeof(struct sbs1553_ioctl));
    DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

    offset = arguments->offset;

    TVUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

    *long_ptr = LI(offset, 1);

    VUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

    RETURN(OK);
}



long
do_ASDC_IMAGE_ECRIRE(int vfxNum, int asdcNum,
                   struct asdc_varg *dst, unsigned long arg)
{

    struct sbs1553_ioctl *arguments, zarguments;

   long *long_ptr, zlong_ptr;
   int offset;


    /* DEBUT DE LA PARTIE SPECIFIQUE ASDC_IMAGE_ECRIRE */

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

        TDUTIL(arguments->buffer, sizeof(long));
        DUTIL(zlong_ptr, long_ptr, arguments->buffer, sizeof(long));

        EI(offset, *long_ptr, 300);

        RETURN(OK);

}

