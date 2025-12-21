
/*

 DRIVER EMUABI

Ce fichier contient les definitions de fonctions utilitaires diverses utilisees
par le driver ASDC en general et par ses fonctions ioctl en particulier.



 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier


*/

#include "driverIncludes.h"



/**********************/
/***   PROVISOIRE   ***/
/**********************/
/* Fonction ci-dessous pour temporiser le demarrage firmware */
/* (en attendant une autre methode ...)                      */
/*   Note : L'argument inutilise "*dst" est un reste de la   */
/*          version LynxOS de cette fonction.                */
int asdc_sleep(struct asdc_varg *dst, int duree_ms)
{
  unsigned long heure;
  int nbre_ticks;

  nbre_ticks = (duree_ms * HZ) / 1000;
  if (!nbre_ticks) nbre_ticks = 1;   /* Attente d'1 tick au minimum ... */

  heure = jiffies + nbre_ticks;

  while (jiffies < heure) schedule();

  return 0;
}



