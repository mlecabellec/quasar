
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_ABO_GF : Fonction ioctl de relecture de la table des filtres.


 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
26/04/2013  YG   Version initiale
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#include "driverIncludes.h"



/* La valeur du filtre est remontee dans le champ nmots */
long
do_ASDC_ABO_GF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int i, j, k;

    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->sous_adresse &= 0x1F;
    v->direction &= 1;

    /* La table des sous_adresses existe-t-elle ? */
    if ((i = L(L(ATPTR) + v->adresse + 32 * v->bus)) == 0)
      { cprintf("ASDC_ABO_MF: Echec 21\n");
        RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
      }

    /* On ne fait aucun test sur l'existence de la table des filtres */
    /* Si la table des sous-adresses existe, cette table DOIT        */
    /* exister aussi !                                               */

    /* Table des filtres */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    k = L(j);

    if (k == 0) {
        /* La table des filtres n'existe pas : le RT n'a sans doute */
        /* jamais ete cree.                                         */
        RETURN(EADDRNOTAVAIL);
    }

    /* Calcul de l'adresse du filtre et recupertaion de son contenu */
    v->nmots = L(k + v->sous_adresse + ( v->direction ? 32 : 0)) & 0xFFFF;


    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));
    RETURN(OK);
}


