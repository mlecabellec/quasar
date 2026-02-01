
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCGO et ASDCSTOP : Fonctions ioctl de demarrage et d'arret du "firmware"
                     du coupleur 1553 EMUSCI.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier
11/02/2015  YG   Remplacement L() et E() par LX() et EX()
26/02/2015  YG   Le partage materiel du port droit de la SRAM est maintenant
                 fonctionnel. Le bon fonctionnement du powerPC n'etant plus
                 necessaire pour acceder a la RAM depuis le bus PCI, les macros
                 LX() et EX() peuvent de nouveau etre remplacees par L() et E().
*/



#include "driverIncludes.h"



/// TODO : Seuls arguments des fonction ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!




    /* Demarrage du micro-code de la carte AMI */
long
do_ASDCGO(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    /* Lancement du micro-code */
    E(CMD, 1);

    /* Une petite attente (1 ms) pour laisser au firmware */
    /* le temps de s'initialiser                          */
    if (asdc_sleep(dst, 1))
      { RETURN(EAGAIN);       /* Pas de timer disponible */
      }

    /* Reinitialisation de quelques parametres, vraisemblablement */
    /* ecrases par des valeurs par defaut au lancement du         */
    /* firmware                                                   */
    E(BCSMSK, dst->asdcdef.bcsmsk); /* BC Status Word Mask         */
    E(BCIGP, dst->asdcdef.bcigp);   /* BC Inter message Gap Time   */
    E(BRTCNT, dst->asdcdef.brtcnt); /* Bc ReTry CouNT              */
    E(BRTBUS, dst->asdcdef.brtbus); /* Bc ReTry BUS                */
    E(RSPGPS, dst->asdcdef.rspgps); /* Temps de reponse RT simules */

    /* RSPGPA n'est pas reprogramme ici, car ce registre ne peut */
    /* etre initialise qu'avant le debut du traitement des I/O.  */
    /* Toute modification ulterieure est sans effet.             */

    RETURN(OK);
}

    /* Arret du micro-code de la carte AMI */
long
do_ASDCSTOP(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    /* Arret du micro-code */
    E(CMD, 0);

    RETURN(OK);
}

