
/*

 DRIVER EMUABI

Ce fichier contient la description des commandes codees autorisees.



 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
15/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "asdc_statics.h"



/* Table de description des commandes codees autorisees */
        /* Indice = code de la commande                            */
        /* Mot 1 (def) :  bit 0 = bit T/R associe a la commande             */
        /*                bit 1 = 1 si commande definie (autorisee)         */
        /* Mot 2 (hmim) : Bit correspondant dans mot de poids fort du MIM   */
        /* Mot 3 (lmim) : Bit correspondant dans mot de poids faible du MIM */
const struct sdcoco asdc_coco[] =
  { { 3, 0x0000, 0x0001 }, /*  0 : GESDYN (Dynamic Bus Control) */
    { 3, 0x0000, 0x0002 }, /*  1 : SYNC ("Synchronize) */
    { 3, 0x0000, 0x0004 }, /*  2 : DSTATUS (Transmit Status) */
    { 3, 0x0000, 0x0008 }, /*  3 : TEST (Initiate Self Test) */
    { 3, 0x0000, 0x0010 }, /*  4 : TS (Transmitter Shutdown) */
    { 3, 0x0000, 0x0020 }, /*  5 : OTS (Override Transmitter Shutdown) */
    { 3, 0x0000, 0x0040 }, /*  6 : ITFB (Inhibit Terminal Flag Bit) */
    { 3, 0x0000, 0x0080 }, /*  7 : OITFB (Override Inhibit Terminal Flag Bit) */
    { 3, 0x0000, 0x0100 }, /*  8 : REINIT (Reset RT) */
    { 3, 0x0000, 0x0200 }, /*  9 : LP (Lancement Programme) */
    { 3, 0x0000, 0x0400 }, /* 10 : ABORT (Arrete Programme) */
    { 3, 0x0000, 0x0800 }, /* 11 : FLASH (Simulation Flash) */
    { 3, 0x0000, 0x1000 }, /* 12 : WMSO (Autorisation Ecriture en MSO) */
    { 3, 0x0000, 0x2000 }, /* 13 : VALID (Valide la Fonction Gerant) */
    { 3, 0x0000, 0x4000 }, /* 14 : INHIB (Inhibe la Fonction Gerant) */
    { 3, 0x0000, 0x8000 }, /* 15 : MODGER (Autorise Changement Mode gerant) */

    { 3, 0x0001, 0x0000 }, /* 16 : VECTEUR (Transmit Vector Word) */
    { 2, 0x0002, 0x0000 }, /* 17 : SD (Synchronize (with data word)) */
    { 3, 0x0004, 0x0000 }, /* 18 : DCDE (Transmit Last Command) */
    { 3, 0x0008, 0x0000 }, /* 19 : BIT (Transmit Built In Test Word) */
    { 2, 0x0010, 0x0000 }, /* 20 : STS (Selected Transmitter Shutdown) */
    { 2, 0x0020, 0x0000 }, /* 21 : OSTS (Override Select. Transm. Shutdown) */
    { 2, 0x0040, 0x0000 }, /* 22 : RES (Reserve) */
    { 0, 0x0000, 0x0000 }, /* 23 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 24 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 25 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 26 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 27 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 28 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 29 : Commande invalide */
    { 0, 0x0000, 0x0000 }, /* 30 : Commande invalide */
    { 0, 0x0000, 0x0000 }  /* 31 : Commande invalide */
  };



