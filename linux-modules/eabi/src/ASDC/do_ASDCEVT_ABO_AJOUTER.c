
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCEVT_ABO_AJOUTER : Fonction ioctl de connexion d'une sous-adresse a un CEVT.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
12/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
18/04/20123 YG   Ajout de la gestion du bus
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/



#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCEVT_ABO_AJOUTER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int zd;
    struct asdcvoie *v, zv;
    int k, l, m;

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


    /* La voie a connecter au CEVT existe-t-elle ? */

    /* On verifie que les structures de donnees liees */
    /* au RT existent bien                            */

    /* La table des sous_adresses existe-t-elle ? */
    k = L(L(ATPTR) + v->adresse + 32 * v->bus);
    if (k == 0)
      { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
      }

    /* Zone de donnees en mem. image de la voie */
    m = k + v->sous_adresse + v->direction * 32;
    zd = LI(m, 33);



    /* Des tampons de donnees existent-t-ils ? (sauf si mode sync 2) */
    if (LI(zd+IRMODE, 0) != RT_VSYNC2)
      { l = L(m);
        if (l == 0)
          { RETURN(EADDRNOTAVAIL);   /* La voie n'a jamais ete declaree */
          }
      }


    /* Le CEVT existe-t-il ? */
    if (cevt_existence == NULL)
      { RETURN(ENOMEM);   /* Fonctions CEVT non importees */
      }
    else
      {
        if (!(*cevt_existence)(v->adrtamp))
          { RETURN(EXDEV);   /* Le numero de CEVT est invalide */
          }
      }


    spin_lock(&dst->cevt_lock); /* Debut section critique */

        /* La voie ne doit pas etre deja connectee a un CEVT */
        if (LI(zd + IRCEVT, 34))
          { 
              spin_unlock(&dst->cevt_lock);    /* Fin section critique */
              RETURN(EEXIST);   /* La voie est deja connectee a un CEVT */
          }

        /* Connexion de la voie */
        EI(zd + IRCEVT, v->adrtamp, 323);  /* Numero CEVT */

    spin_unlock(&dst->cevt_lock);    /* Fin section critique */


    /* Validation des interruptions si message destine a la voie */
    { int ef;

      /* Pointeur table des filtres associee a adresse */
      ef = L(L(FTPTR) + v->adresse + 32 * v->bus);

      /* Adresse du filtre */
      ef += v->sous_adresse + ( v->direction ? 32 : 0);

      /* Programmation du filtre : Validation IT voie */
      E(ef, L(ef) | 2);
    }

    /*
    *  REMARQUE : Les interruptions sur reception message sont validees
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

    /* Fin normale de ASDCEVT_ABO_AJOUTER */
    RETURN(OK);

}

