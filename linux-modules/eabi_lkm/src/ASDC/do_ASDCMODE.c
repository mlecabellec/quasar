
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCMODE : Fonction ioctl de selection du mode de fonctionnement d'un abonne.

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
29/04/2013  YG   Ajout de l'option "acceptation prise de gerance".
 3/05/2013  YG   Contrairement a l'ABI-PMC2, l'EMUABI a besoin d'une entree
                 dans sa table des status pour fonctionner en mode espion TR.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
 3/02/2015  YG   Utilisation d'un pointeur tampon suivant = -1 pour les voies
                 en emission en mode asynchrone.
 5/05/2015  YG   Correction bug sur adresse du tampon courant quand on quitte
                 le mode espion temps-reel.
*/

      /* Programmation du mode de fonctionnement */
      /* d'un RT simule                          */





#include "driverIncludes.h"



/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCMODE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int i, j, mm;
    unsigned int zd;
    struct asdcvoie *v, zv;
    int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */
    int accger;

    // printk("ASDCMODE : Debut\n");

    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;

    accger = 0;   /* Par defaut, refus de la prise de gerance */

    /* On verifie que les structures de donnees liees */
    /* au RT existent bien                            */

    /* La table des sous_adresses existe-t-elle ? */
    mm = L(L(ATPTR) + v->adresse + 32 * v->bus);
    if (mm == 0)
              { RETURN(EADDRNOTAVAIL);   /* RT n'a jamais ete declare */
              }

    /* On ne cherche pas a savoir quel est l'etat initial du RT */
    /* On applique directement le nouveau mode demande          */
    switch (v->nmots)
      { case RT_INHIBE :
              /* Inhibition du RT dans la table des Mots d'Etat */
              E(L(SWTPTR) + v->adresse + 64 * v->bus, 0);

              /* Inhibition de l'espion temps reel et de l'acceptation */
              /* GESDYN dans la table des protocoles                   */
              tmpi = L(PROPTR) + v->adresse + 32 * v->bus;
              E(tmpi, L(tmpi) & ~6);

              break;

        case RT_ESPION :
              /* Validation du RT dans la table des Mots d'Etat */
              if (v->adresse)
                { /* Si adresse != 0, "status 1553 standard" */
                  E(L(SWTPTR) + v->adresse + 64 * v->bus, v->adresse << 11);
                }
              else
                { /* Si adresse = 0, on met bit ME a 1            */
                  /* (en principe, ca n'affecte pas le mot d'etat */
                  /*  renvoye...)                                 */
                  E(L(SWTPTR) + v->adresse + 64 * v->bus, 0x400);
                }

              /* Validation de l'espion temps reel dans la */
              /* table des protocoles                      */
              tmpi = L(PROPTR) + v->adresse + 32 * v->bus;
              E(tmpi, L(tmpi) | 2);
              E(tmpi, L(tmpi) & ~4);

              /* Balayage des voies ASYNC et des voies SYNC2  */
              /* en transmission pour eventuelle modif. de la */
              /* structure des tampons                        */
              for (i=0; i<32; i++)
                 { int modevoie;

                   /* mm pointe table des SA, i balaye les SA, 32 ==> Transm. */
                   j = L(mm + i + 32);
                   if (j == 0) continue;

                   /* Voie ASYNC ou SYNC2 ? */
                   zd = LI(mm + i + 32, 8);
                   modevoie = LI(zd+IRMODE, 9);

                   if (modevoie == RT_VASYNC)
                     { /* Les 2 tampons sont-ils separes ? */
                       if (L(j) == 0xFFFFFFFF)
                         { /* Oui ==> on les enchaine */
                           tmpi = LI(zd+IRTCAPP, 10);
                           E(j, tmpi);
                           E(tmpi, j);
                         }
                     }

                   if (modevoie == RT_VSYNC2)
                     { /* Memorisation adresse courante */
                       EI(zd+IRTCACHE, j, 397);
                       /* Et inhibition RT (provisoire ...?) */
                       E(mm + i + 32, 0);
                       usec_sleep(1);
                       E(mm + i + 32, 0);
                     }
                 }

              break;

        case RT_ABONNE_ACC :   /* Abonne avec acceptation de prise de gerance */
              accger = 1;

        case RT_ABONNE :       /* Abonne avec refus de prise de gerance */
              /* Validation du RT dans la table des Mots d'Etat */
              if (v->adresse)
                { /* Si adresse != 0, "status 1553 standard" */
                  E(L(SWTPTR) + v->adresse + 64 * v->bus, v->adresse << 11);
                }
              else
                { /* Si adresse = 0, on met bit ME a 1            */
                  /* (en principe, ca n'affecte pas le mot d'etat */
                  /*  renvoye...)                                 */
                  E(L(SWTPTR) + v->adresse + 64 * v->bus, 0x400);
                }

              /* Inhibition de l'espion temps reel dans la */
              /* table des protocoles                      */
              tmpi = L(PROPTR) + v->adresse + 32 * v->bus;
              E(tmpi, L(tmpi) & ~2);

              /* Validation ou refus de l'acceptation GESDYN dans la */
              /* table des protocoles                                */
              if (accger) {
                  E(tmpi, L(tmpi) | 4);
              } else {
                  E(tmpi, L(tmpi) & ~4);
              }

              /* Balayage des voies ASYNC en transmission pour */
              /* eventuelle modif. de la structure des tampons */
              for (i=0; i<32; i++)
                 { int modevoie;

                   /* mm pointe table des SA, i balaye les SA, 32 ==> Transm. */
                   j = L(mm + i + 32);
                   if (j == 0) continue;

                   /* Voie ASYNC ou SYNC2 ? */
                   zd = LI(mm +i + 32, 11);
                   modevoie = LI(zd+IRMODE, 12);

                   if (modevoie == RT_VASYNC)
                     { /* Les 2 tampons sont-ils separes ? */
                       if (L(j) != 0xFFFFFFFF)
                         { /* Non ==> on les separe */
                           EI(zd+IRTCAPP, LI(j+IRTPREC, 314), 314);
                           E(j, 0xFFFFFFFF);
                         }
                     }

                   if (modevoie == RT_VSYNC2)
                     { int cache;
                       /* Une chaine de tampons est-elle cachee ? */
                       cache = LI(zd+IRTCACHE, 112);
                       if (cache)
                         { /* Oui ==> on la restore */
                           E(mm + i + 32, cache);
                           EI(zd+IRTCACHE, 0, 212);
                         }
                     }
                 }

              break;

        default :
              RETURN(ENXIO);
      }

    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));
    RETURN(OK);
}

