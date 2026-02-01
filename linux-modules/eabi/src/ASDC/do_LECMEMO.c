
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCLECMEMO et ASDCLECEMEMO : Fonctions ioctl de relecture (pour debug)
          de l'etat du systeme d'allocation en memoire d'echange.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


    /*   Lecture de l'environement table memo[]          */
    /*   Cette fonction est uniquement destinee au debug */
    /*   de l'allocation en memoire d'echange            */

long
do_ASDCLECEMEMO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcememo *emem, zemem;

    TVUTIL(zemem, emem, arg, sizeof(struct asdcememo));
    asdclecememo(dst, emem);
    VUTIL(zemem, emem, arg, sizeof(struct asdcememo));
    RETURN(OK);
}


    /*   Lecture d'une entree de la table memo[]         */
    /*   Cette fonction est uniquement destinee au debug */
    /*   de l'allocation en memoire d'echange            */

long
do_ASDCLECMEMO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcblibre *bl, zbl;

    TDUTIL(arg, sizeof(struct asdcblibre));
    DUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
    TVUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
    if (asdclecmemo(dst, bl))
      { RETURN(EINVAL);
      }
    else
      {
         VUTIL(zbl, bl, arg, sizeof(struct asdcblibre));
         RETURN(OK);
      }
}
