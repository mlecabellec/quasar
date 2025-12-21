
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCLIREMODE : Fonction ioctl de lecture du mode de fonctionnement d'un abonne.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
26/04/2013  YG   Version initiale.
29/04/2013  YG   Ajout du traitement "acceptation prise de gerance".
 3/05/2013  YG   Contrairement a l'ABI-PMC2, l'EMUABI a besoin d'une entree
                 dans sa table des status pour fonctionner en mode espion TR.
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/

      /* Programmation du mode de fonctionnement */
      /* d'un RT simule                          */





#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCLIREMODE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int mm;
    struct asdcvoie *v, zv;
    int sts;
    int proto;

    // printk("ASDCMODE : Debut\n");

    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;

    /* On verifie que les structures de donnees liees */
    /* au RT existent bien                            */

    /* La table des sous_adresses existe-t-elle ? */
    mm = L(L(ATPTR) + v->adresse + 32 * v->bus);
    if (mm == 0)
              { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
              }

    /* Mot de status programme */
    sts = L(L(SWTPTR) + v->adresse + 64 * v->bus) & 0xFFFF;

    /* Mot de protocole */
    proto = L(L(PROPTR) + v->adresse + 32 * v->bus) & 0xFFFF;

    /* Determination du mode de l'abonne */
    if (sts != 0) {
        if (proto & 2) {
            v->nmots = RT_ESPION;
        } else if (proto & 4) {
            v->nmots = RT_ABONNE_ACC;
        } else {
            v->nmots = RT_ABONNE;
        }
    } else {
        v->nmots = RT_INHIBE;
    }


    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));
    RETURN(OK);
}

