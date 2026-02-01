
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

sbs1553_read et sbs1553_write : Fonctions ioctl d'acces a la memoire d'echange.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"


/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


long
do_sbs1553_read(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct sbs1553_ioctl *arguments, zarguments;
    int offset;
    int i;
    short *word_ptr;
    short zword_ptr;

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;
        i = arguments->device;	/* Instrumentation */

	TVUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        *word_ptr = LL(offset, i);

        VUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));
        RETURN(OK);
}


long
do_sbs1553_write(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct sbs1553_ioctl *arguments, zarguments;
    int offset;
    short *word_ptr;
    short zword_ptr;

        TDUTIL(arg, sizeof(struct sbs1553_ioctl));
        DUTIL(zarguments, arguments, arg, sizeof(struct sbs1553_ioctl));

        offset = arguments->offset;

        TDUTIL(arguments->buffer, sizeof(short));
        DUTIL(zword_ptr, word_ptr, arguments->buffer, sizeof(short));

        E(offset, *word_ptr);
        RETURN(OK);
}

