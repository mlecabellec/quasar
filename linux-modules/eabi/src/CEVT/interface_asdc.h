
/*

 DRIVER EMUABI

Ce fichier donne acces aux prototypes des fonctions utilisees pour se connecter
au driver ASDC.


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
11/04/2013  YG   Version initiale
 8/01/2015  YG  Adaptation a la compilation d'un driver monobloc EMUABI
                incluant dans un seul module les drivers VFX70, ASDC et CEVT.


*/


#ifndef EMUABI_INTERFACE_ASDC_H
#define EMUABI_INTERFACE_ASDC_H



/* J'aurais prefere specifier le repertoire dans la makefile plutot   */
/* que dans l'include ci-dessous mais, pour le moment, je ne sais pas */
/* faire.                                                             */
#include "../ASDC/asdcExport.h"



#endif   /* EMUABI_INTERFACE_ASDC_H */
