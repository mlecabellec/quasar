
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCEVT_CC_AJOUTER : Fonction ioctl de connexion d'une commande codee a un CEVT.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"




/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCEVT_CC_AJOUTER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    struct asdcvoiecc ze, *e;
    int code;
    int mim;
    unsigned short hmim, lmim;
    int k;

    TDUTIL(arg, sizeof(struct asdcvoiecc));
    DUTIL(ze, e, arg, sizeof(struct asdcvoiecc));
    TVUTIL(ze, e, arg, sizeof(struct asdcvoiecc));


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    e->bus &= 0x01;
    e->adresse &= 0x1F;
    e->coco &= 0x41F;


    /* La voie a connecter au CEVT existe-t-elle ? */

    /* On verifie que les structures de donnees liees */
    /* au RT existent bien                            */

    /* La table des sous_adresses existe-t-elle ? */
    k = L(L(ATPTR) + e->adresse + 32 * e->bus);
    if (k == 0)
      { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
      }


    /* Le CEVT existe-t-il ? */
    if (cevt_existence == NULL)
      { RETURN(ENOMEM);   /* Fonctions CEVT non importees */
      }
    else
      {
        if (!(*cevt_existence)(e->cevt))
          { RETURN(EXDEV);   /* Le numero de CEVT est invalide */
          }
      }


    /* Code "de base" de la commande */
    code = e->coco & 0x1F;

    /* Commande existante ? */
    if ((!(asdc_coco[code].def & 2)) || (code > COCO_MAX))
      {
        RETURN(ENOENT);   /* Non */
      }



    spin_lock(&dst->cevt_lock); /* Debut section critique */

        /* La voie ne doit pas etre deja connectee a un CEVT */
        if (dst->cocor[e->bus][e->adresse][code].cevt) {
            spin_unlock(&dst->cevt_lock);    /* Fin section critique */
            RETURN(EEXIST);   /* La voie est deja connectee a un CEVT */
        }

        /* Connexion de la voie */
        dst->cocor[e->bus][e->adresse][code].cevt = e->cevt;   /* Numero CEVT */




        /* Validation des interruptions si message destine a la voie */

        mim = L(MIMPTR);                        /* Adresse de la table des MIM */

        mim += 2 * e->adresse + 64 * e->bus;    /* Adresse du MIM de la voie */

        hmim = asdc_coco[code].hmim;
        lmim = asdc_coco[code].lmim;



        if (lmim) E(mim, L(mim) | lmim);
        if (hmim) E(mim + 1, L(mim + 1) | hmim);

    spin_unlock(&dst->cevt_lock);    /* Fin section critique */




    /*
    *  REMARQUE : Les interruptions sur reception CC sont validees
    *             en differents endroits (dont ci-dessus), mais ne sont
    *             sans doute pas toujours inhibees (par exemple, en cas
    *             de suppression d'un CEVT). [En effet, quand un CEVT
    *             est supprime, rien permet d'affirmer qu'une tache n'est
    *             pas alors en train d'effectuer une lecture bloquante
    *             sur la voie concernee ...].
    *             ==> La strategie actuellement employee consiste a
    *                 considerer qu'il vaut mieux laisser une IT validee
    *                 (ce qui introduit seulement une perte de quelques
    *                 us de termps UC en cas d'arrivee du message associe
    *                 a la voie) plutot qu'inhiber une IT qui est
    *                 utilisee par ailleurs.
    *             ==> Cette strategie pourra toujours etre revue
    *                 ulterieurement, si le besoin s'en fait sentir ...
    *
    *                                     Y. Guillemot, le 5/6/2002
    */

    /* Fin normale de ASDCEVT_CC_AJOUTER */
    RETURN(OK);

}

