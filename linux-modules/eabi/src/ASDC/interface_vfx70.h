
/*

 DRIVER EMUABI

Ce fichier donne acces aux prototypes des fonctions permettant utilisees
comme interfaces avec le driver de la carte VFX70, sur laquelle est placee
le FPGA qui supporte l'emulation de coupleur 1553 EMUSCI


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 8/04/2013  YG   Version initiale
 8/01/2015  YG  Adaptation a la compilation d'un driver monobloc EMUABI
                incluant dans un seul module les drivers VFX70, ASDC et CEVT.


*/


#ifndef EMUABI_INTERFACE_VFX70_H
#define EMUABI_INTERFACE_VFX70_H



/* J'aurais prefere specifier le repertoire dans la makefile plutot   */
/* que dans l'include ci-dessous mais, pour le moment, je ne sais pas */
/* faire.                                                             */
#include "../VFX70/vfxExport.h"

// Pour debug en utilisant "tag()"
#include "../VFX70/vfx_ctl.h"
#include "../VFX70/vfxvarg.h"



#endif   /* EMUABI_INTERFACE_VFX70_H */
