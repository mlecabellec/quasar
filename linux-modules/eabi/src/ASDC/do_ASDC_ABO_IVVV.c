
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

AASDC_ABO_IV et SDC_ABO_VV : Fonction ioctl de validation et d'inhibition
                             d'une sous-adresse.

AASDC_ABO_GV : Relecture de l'etat d'une voie

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
26/04/2013  YG   Ajout de la fonction de relecture.
13/06/2014  YG   Rassemblement des includes dans un seul fichier

*/


#include "driverIncludes.h"


/// TODO : Seuls arguments des fonctions ci-dessous qui sont sans doute
///        necessaires : dst et arg !!!



    /* Inhibition d'une "voie" (adr, sa, sens) d'un abonne */
long
do_ASDC_ABO_IV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    /* Dans la structure asdcvoie en entree :                    */
    /*                                                           */
    /*    - les champs bus, adresse, sous_adresse et direction   */
    /*      definissent la voie                                  */
    /*                                                           */

    struct asdcvoie *v, zv;
    int i, j, k, l, m;

    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->sous_adresse &= 0x1F;
    v->direction &= 1;

    /* Allocation si necessaire de la table des filtres */
    /* et initialisation de son image                   */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    k = L(j);
    if (k==0)
        { k = asdcalloc(dst, 64);
          if (k==-1) { RETURN(ENOMEM);     /* Plus de memoire */
                    }
          E(j, k);
          for (i=k; i<k+64; i++)
            { E(i, 0);  /* Init a 0 */
              EI(i, 0, 387);
            }
        }
    /*### ### ###
        Le code ci-dessus est inutile : Si la voie existe
        (test fait ci-dessous), la table des filtres
        existe forcement aussi !!!
      ### ### ###*/

    /* Calcul de l'adresse du filtre */
    k += v->sous_adresse + ( v->direction ? 32 : 0);

    /* Calcul de l'adresse de la table des sous-adresses */
    l = L(ATPTR) + v->adresse + 32 * v->bus;
    l = L(l);
    if (l == 0) { return(EADDRNOTAVAIL);  /* Abonne non defini ! */
                }

    /* Calcul de l'adresse du pointeur des tampons */
    l += v->sous_adresse + ( v->direction ? 32 : 0);

    /* Adresse des tampons */
    m = L(l);

    /* Si pointeur nul, voie inexistante ou deja inhibee */
    if (m == 0) { return(EADDRINUSE);  /* Voie non definie ! */
                }

    /* Sauvegarde adresse des tampons dans l'image */
    /* de la table des filtres (FTPTR)             */
    EI(k, m, 388);

    /* Inhibition voie */
    E(l, 0);
    usec_sleep(1);        /* Attente 1 us */
    E(l, 0);              /* Si acces concurrent par firmware */

    RETURN(OK);
}




    /* Revalidation d'une "voie" (adr, sa, sens) d'un abonne   */
long
do_ASDC_ABO_VV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    /* Dans la structure asdcvoie en entree :                    */
    /*                                                           */
    /*    - les champs bus, adresse, sous_adresse et direction   */
    /*      definissent la voie                                  */
    /*                                                           */

    struct asdcvoie *v, zv;
    int i, j, k, l, m;
    unsigned int zd, cmd;


    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->sous_adresse &= 0x1F;
    v->direction &= 1;

    /* Calcul de l'adresse de la table des sous-adresses */
    l = L(ATPTR) + v->adresse + 32 * v->bus;
    l = L(l);
    if (l == 0) { return(EADDRNOTAVAIL);  /* Abonne non defini ! */
                }
    i = l;

    /* Calcul de l'adresse du pointeur des tampons */
    l += v->sous_adresse + ( v->direction ? 32 : 0);

    /* Si pointeur non nul, voie non inhibee ! */
    if (L(l) != 0) { return(EADDRINUSE);  /* Voie non inhibee ! */
                    }


    /* Recherche de la table des filtres */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    k = L(j);
    if (k==0)
        { /* Table des filtres non initialisee                        */
          /*   ==> voie non definie, puisque pointeur tampons est nul */
          RETURN(EADDRNOTAVAIL);
        }

    /* Calcul de l'adresse du filtre */
    k += v->sous_adresse + ( v->direction ? 32 : 0);

    /* Recuperation de l'adresse des tampons */
    m = LI(k, 115);
    if (m == 0)
      { /* Adresse tampons sauvevardee est nulle ! */

        /* Si la voie est en mode synchrone 2, nous sommes */
        /* en presence d'une voie valide, et il n'y a rien */
        /* de plus a faire (retour du code EADDRINUSE)     */
        /* Sinon, la voie n'a pas ete definie (retour du   */
        /* code EADDRNOTAVAIL)                             */
        /*    ==> il faut determiner le mode de la voie    */

        /* Pointeur vers zone des donnees en memoire image */
        zd = LI(i + v->sous_adresse + (v->direction ? 32 : 0), 13);
        if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
          { cprintf("ASDC_ABO_VV: Echec zd (zd[0x%X] = 0x%X)\n",
                      i + v->sous_adresse + (v->direction ? 32 : 0), zd);
            RETURN(ESPIPE);               /* Memoire image anormale ! */
          }

        /* Le contenu de la memoire image semble-t-il correct ? */
        cmd =   (v->adresse << 11)
              | (v->direction ? 0x400 : 0)
              | (v->sous_adresse << 5);
        if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
          { cprintf("ASDC_ABO_VV: Echec 24 - ");
            cprintf("LI(zd+IRCMD)=0x%08lX   cmd=0x%08X\n",
                      LI(zd+IRCMD, 14) & 0xFFFFFFFF, cmd);

            RETURN(ESPIPE);               /* Memoire image anormale ! */
          }

        /* Mode de la voie (synchrone, asynchrone, etc...) */
        m = LI(zd + IRMODE, 15);

        /* Si mode "synchrone 2", la voie existe et est valide */
        if (m == RT_VSYNC2) RETURN(EADDRINUSE);

        /* Sinon, la voie est non definie, puisque le pointeur */
        /* des tampons est nul                                 */
        RETURN(EADDRNOTAVAIL);
      }

    /* Restauration adresse tampons */
    E(l, m);
    EI(k, 0, 389);

    RETURN(OK);
}




    /* Lecture de l'etat d'une "voie" (adr, sa, sens) d'un abonne   */
long
do_ASDC_ABO_GV(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    /* Dans la structure asdcvoie en entree :                    */
    /*                                                           */
    /*    - les champs bus, adresse, sous_adresse et direction   */
    /*      definissent la voie                                  */
    /*                                                           */
    /*                                                           */
    /* En sortie, le champ nmots definit le mode :               */
    /*    -  0 ==> SA non definie (mais RT defini)               */
    /*    -  1 ==> Inhibee                                        */
    /*    -  2 ==> Valide                                        */

    struct asdcvoie *v, zv;
    int i, j, k, l, m;
    unsigned int zd, cmd;


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

    /* Calcul de l'adresse de la table des sous-adresses */
    l = L(ATPTR) + v->adresse + 32 * v->bus;
    l = L(l);
    if (l == 0) { return(EADDRNOTAVAIL);  /* Abonne non defini ! */
                }

    /* Recherche de la table des filtres */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    k = L(j);
    if (k==0)
        { /* Table des filtres non initialisee  ==> abonne non defini  */
          /* Ou defini d'une facon anormale ...                        */
          RETURN(EADDRNOTAVAIL);
        }

    i = l;

    /* Calcul de l'adresse du pointeur des tampons */
    l += v->sous_adresse + ( v->direction ? 32 : 0);

    /* Calcul de l'adresse du filtre */
    k += v->sous_adresse + ( v->direction ? 32 : 0);


    if (L(l) != 0) {
        /* Si pointeur des tampons non nul, voie validee ! */
        v->nmots = 2;
    } else {
        if (LI(k, -1)) {
            /* Le pointeur de tampons est cache ici ==> voie inhibee */
            v->nmots = 1;
        } else {
            /* Voie quand meme definie si elle en mode synchrone 2 */

            /* Pointeur vers zone des donnees en memoire image */
            zd = LI(i + v->sous_adresse + (v->direction ? 32 : 0), 13);
            if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
              { cprintf("ASDC_ABO_GV: Echec zd (zd[0x%X] = 0x%X)\n",
                          i + v->sous_adresse + (v->direction ? 32 : 0), zd);
                RETURN(ESPIPE);               /* Memoire image anormale ! */
              }

            /* Le contenu de la memoire image semble-t-il correct ? */
            cmd =   (v->adresse << 11)
                  | (v->direction ? 0x400 : 0)
                  | (v->sous_adresse << 5);
            if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
              { cprintf("ASDC_ABO_GV: Echec 24 - ");
                cprintf("LI(zd+IRCMD)=0x%08lX   cmd=0x%08X\n",
                          LI(zd+IRCMD, 14) & 0xFFFFFFFF, cmd);

                RETURN(ESPIPE);               /* Memoire image anormale ! */
              }

            /* Mode de la voie (synchrone, asynchrone, etc...) */
            m = LI(zd + IRMODE, -1);

            /* Si mode "synchrone 2", la voie existe et est valide */
            if (m == RT_VSYNC2) {
                v->nmots = 2;
            } else {
                /* La voie n'existe pas */
                v->nmots = 0;
            }

        }
    }


    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));
    RETURN(OK);
}

