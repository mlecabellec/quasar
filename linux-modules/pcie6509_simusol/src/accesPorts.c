/**************************************************************************
 *                                                                        *
 *      Driver pour carte NI PCIe-6509 (96 E/S TOR)                       *
 *     ---------------------------------------------                      *
 *                                                                        *
 * accesPorts.c : Fonctions d'acces aux ports (8 bits) a partir des       *
 *                registres 32 bits de la carte.                          *
 *                                                                        *
 *                                 ************ - ************ - ************
 *Anonymized
 **
 **************************************************************************/

/*
   QUAND    QUI   QUOI
---------- ----- ---------------------------------------------------------------
14/12/2013   YG  Version initiale
24/03/2012   YG  Modification des niveaux logiques des E/S en fonction
                 de la variable globale "logiquePositive".

*/

#include "DriverIncludes.h"

/* Lecture d'un port en entree */
uint8_t lirePortEntree(pcie6509Statics_t *pstat, int port) {
  uint8_t r;
  r = readl(pstat->bvba + iadr[port]) >> decalage[port];

  r = (logiquePositive ? r : ~r) & 0xFF;

  return r;
}

/* Ecriture d'un port en sortie */
void ecrirePortSortie(pcie6509Statics_t *pstat, int port, uint8_t valeur) {
  uint32_t data;
  int i;
  int p;
  int tmp;

  valeur = logiquePositive ? valeur : ~valeur;

  /* Image du registre a ecrire avec les bits a modifier positionnes */
  data = (valeur << decalage[port]) & masque[port];

  /* Ajout a  cette image des bits inchanges (ceux des autres ports) */
  tmp = listep[port];
  for (i = 0; i < (listep[port] & 0xF); i++) {
    tmp >>= 4;
    p = tmp & 0xF;
    data |= ((pstat->bit[p] << decalage[p]) & masque[p]);
  }

  printLog(DRV_DEBUG, "S: port=%d @reg=0x%X data=0x%02X\n", port, oadr[port],
           data);

  /* Ecriture de l'image a l'adresse appropriee */
  writel(data, pstat->bvba + oadr[port]);

  /* Et memorisation des bits modifies */
  pstat->bit[port] = valeur;
}

/* Ecriture des directions des bits d'un port */
void ecrirePortDirection(pcie6509Statics_t *pstat, int port, uint8_t valeur) {
  uint32_t data;
  int i;
  int p;
  int tmp;

  /* Image du registre a ecrire avec les bits a modifier positionnes */
  data = (valeur << decalage[port]) & masque[port];

  /* Ajout a  cette image des bits inchanges (ceux des autres ports) */
  tmp = listep[port];
  for (i = 0; i < (listep[port] & 0xF); i++) {
    tmp >>= 4;
    p = tmp & 0xF;
    data |= ((pstat->dir[p] << decalage[p]) & masque[p]);
  }

  printLog(DRV_DEBUG, "DIR: port=%d @reg=0x%X data=0x%02X\n", port, dadr[port],
           data);

  /* Ecriture de l'image a l'adresse appropriee */
  writel(data, pstat->bvba + dadr[port]);

  /* Et memorisation des directions modifiees */
  pstat->dir[port] = valeur;
}
