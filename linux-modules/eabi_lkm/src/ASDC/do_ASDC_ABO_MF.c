
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDC_ABO_MF et ASDC_ABO_MFX : Fonction ioctl de modification de la table des
                              filtres.
    ASDC_ABO_MF : permet de creer reponses avec erreur de parite
    ASDC_ABO_MFX : permet de creer des erreurs de parite transitoires

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
13/06/2014  YG   Rassemblement des includes dans un seul fichier


*/


#include "driverIncludes.h"



/*
 * Le parametre "asdcabomf" est utilise pour distinguer les fonctionnalites
 * ASDC_ABO_MF (asdcabomf=1) et ASDC_ABO_MFX (asdcabomf=0).
 */
static long
travail(int asdcabomf, int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    /*** Si asdcabomf = 1 : ASDC_ABO_MF                                     ***/
    /*** Modification du mot de la table des filtres associe a une voie RT  ***/

    /* Dans la structure asdcvoie en entree :                    */
    /*                                                           */
    /*    - les champs adresse, sous_adresse et direction        */
    /*      definissent la voie                                  */
    /*                                                           */
    /*    - les champs adrtamp et adrtamp2 definissent, pour     */
    /*      chacun des bits du filtre, les modifications a       */
    /*      effectuer :                                          */
    /*          adrtamp  adrtamp2            Action              */
    /*         --------  --------    --------------------------- */
    /*             0         0           Bit inchange            */
    /*             1         0           Bit mis a 0             */
    /*             0         1           Bit mis a 1             */
    /*             1         1           Bit modifie             */
    /*                                                           */

    /*** Si asdcabomf = 0 : ASDC_ABO_MFX                                    ***/
    /*** Modification du mot de la table des filtres associe a une voie RT  ***/
    /*** Fonction analogue a ASDC_ABO_MF, mais generation de plusieurs      ***/
    /*** (jusqu'a 32) erreurs de parite transitoires en fonction des bits   ***/
    /*** du champ ntamp.                                                    ***/

    /* Difference par rapport a ASDC_ABO_MF : le champ ntamp de  */
    /* la structure asdcvoie contient une liste de bits.         */
    
    struct asdcvoie *v, zv;
    unsigned int zd, cmd;
    int i, j, k, m;
    int liste;      /* Liste de bits pour ASDC_ABO_MFX */
    ITFLAGS(s);    /* Pour masquage IT pendant section critique */

    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->sous_adresse &= 0x1F;
    v->direction &= 1;
    v->mode &= RT_ETRANSIT;

    if (asdcabomf) liste = 1;   /* Si ASDC_ABO_MF, au plus 1 erreur */
              else liste = v->ntamp;

    /* La table des sous_adresses existe-t-elle ? */
    if ((i = L(L(ATPTR) + v->adresse + 32 * v->bus)) == 0)
      { cprintf("ASDC_ABO_MF: Echec 21\n");
        RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
      }

    /* Pointeur vers zone des donnees en memoire image */
    zd = LI(i + v->sous_adresse + (v->direction ? 32 : 0), 13);
    if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
      { cprintf("ASDC_ABO_MF: Echec zd (zd[0x%X] = 0x%X)\n",
                  i + v->sous_adresse + (v->direction ? 32 : 0), zd);
        RETURN(ESPIPE);           /* Memoire image anormale ! */
      }

    /* Le contenu de la memoire image semble-t-il correct ? */
    cmd =   (v->adresse << 11)
          | (v->direction ? 0x400 : 0)
          | (v->sous_adresse << 5);
    if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
      { cprintf("ASDC_ABO_MF: Echec 24 - ");
        cprintf("LI(zd+IRCMD)=0x%08lX   cmd=0x%08X\n",
                  LI(zd+IRCMD, 14) & 0xFFFFFFFF, cmd);

        RETURN(ESPIPE);           /* Memoire image anormale ! */
      }

    /* Mode de la voie (synchrone, asynchrone, etc...) */
    m = LI(zd + IRMODE, 15);

    /* On ne fait aucun test sur l'existence de la table des filtres */
    /* Si la table des sous-adresses existe, cette table DOIT        */
    /* exister aussi !                                               */

    /* Table des filtres */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    k = L(j);

    /* Calcul de l'adresse du filtre */
    k += v->sous_adresse + ( v->direction ? 32 : 0);

    /* La voie existe-t-elle ? */
    j = L(i + v->sous_adresse +  (v->direction ? 32 : 0));
    if ((j == 0) && (LI(k, 0) == 0) && (m != RT_VSYNC2))
      { cprintf("ASDC_ABO_MF: Echec 22 - ");
        cprintf(" j=%d m=%d\n", j, m);
        RETURN(EADDRNOTAVAIL); /* La voie n'a jamais ete declaree ! */
      }
    /* On considere que la voie existe si :
            - Un pointeur de tampons est defini
        ou - L'image du filtre est non nulle (cas d'une voie inhibee)
        ou - la voie est en mode synchrone 2
        ### Il faudrait sans doute rationaliser ce type      ###
        ### de test en utilisant un unique indicateur fiable ###
        ### (par exemple un fanion en memoire image).        ###
    */

    /* Modif. de chaque bit de opt en fonction des bits */
    /* correspondants des champs adrtamp et adrtamp2    */
    { int filtre, memofiltre;

      /* -------------------------------------------- */
      /* ---   Debut section critique   ----- vvvvvvv */
      DISABLE(s);

        /* Inhibition des ITs pour eviter pb si modification   */
        /* filtre (mem) et/ou memofiltre (image) simultanement */
        /* par ioctl(ASDC_ABO_MF) et fonction d'IT ...         */

        /* Memorisation de l'adresse du filtre en memoire image */
        /* (pour accelerer le traitement de l'interruption)     */
        EI((zd+IRAFILTRE), k, 0);

        /* Lecture filtre courant */
        filtre = memofiltre = L(k);

        /* Modif. de chaque bit de opt en fonction des bits */
        /* correspondants des champs adrtamp et adrtamp2    */
        filtre =   (v->adrtamp & v->adrtamp2 & ~filtre)
                  | ((filtre | v->adrtamp2) & ~v->adrtamp);

        /* Memorisation eventuelle des filtres en memoire d'image */
        if (v->mode)
          {
            /* Nombre total courant d'erreurs de parite */
            int nb_err = L(ERRPAR_NB);

            /* Memorisation des 2 filtres OK et Erreur */
            EI((zd+IRMEMOF), (   (memofiltre & 0xFFFF)
                                | 0x80000000
                                | ((nb_err << 16) & 0x7FFF0000) ), 0);
            EI((zd+IRMEMOE), (filtre & 0xFFFF), 0);

            /* La valeur du filtre courant depend du bit 0 de liste */
            if (liste & 1) E(k, filtre);
              else         E(k, memofiltre);
            EI((zd+IRTLET), liste, 0);
          }
        else
          { /* Nouvelle valeur imposee au filtre,  */
            /* inhibition des erreurs transitoires */
            E(k, filtre);
            EI((zd+IRMEMOF), 0, 0);
            EI((zd+IRMEMOE), 0, 0);
            EI((zd+IRTLET), 0, 0);
          }

        RESTORE(s);
        /* ---   Fin section critique   ------- ^^^^^^^ */
        /* -------------------------------------------- */
    }

    RETURN(OK);

  
  
  
  
}




/// TODO : Seuls arguments des fonctions ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!


/* Modification du mot de la table des filtres associe a une voie RT */
long
do_ASDC_ABO_MF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    return travail(1, vfxNum, asdcNum, dst, arg);
}

/* Modification du mot de la table des filtres associe a une voie RT */
/* Fonction analogue a ASDC_ABO_MF, mais generation de plusieurs     */
/* (jusqu'a 32) erreurs de parite transitoires en fonction des bits  */
/* du champ ntamp. Difference par rapport a ASDC_ABO_MF : le champ   */
/* ntamp de la structure asdcvoie contient une liste de bits.        */
long
do_ASDC_ABO_MFX(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    return travail(0, vfxNum, asdcNum, dst, arg);
}



