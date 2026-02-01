/********************************************************************/
/* Driver Linux pour carte E/S TOR PCIe-6509 de National Instrument */
/*                                                                  */
/*                                                                  */
/*                             ************ - ************ - ************
 * Anonymized */
/********************************************************************/

/*
  La liste ci-dessous decrit les evolutions successives du pilote :

V1.0 : 16/01/2014
       -Version initiale
V1.1 : 25/03/2014
       - Ajout option pour choisir logique positive ou negative

*/

/* ATTENTION : Les tailles des chaines PCIE6509_NOM et PCIE6509_DATE  */
/*             doivent etre inferieures a 40 caracteres !!!           */
/*             (ou plus exactement, inferieures a TMAXSTRVER)         */

#ifndef PCIE6509_VERSION_H
#define PCIE6509_VERSION_H

#define PCIE6509_NOM "PCIE6509"
#define PCIE6509_VERSION 1
#define PCIE6509_REVISION 1
#define PCIE6509_DATE "25 mars 2014"

#endif /* PCIE6509_VERSION_H */
