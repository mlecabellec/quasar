
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCDEF : Fonction ioctl de creation d'une sous-adresse.
          Si l'adresse n'existe pas, elle est egalement creee.

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 8/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
19/04/2013  YG   Initialisation a zero des buffers crees (contournement
                 d'un bug du F/W EMUABI).
12/06/2014  YG   Initialisation de la FIFO associee aux sous-adresses en
                 emission.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
27/01/2015  YG   Inhibition des interruptions pendant les acces a la FIFO des
                 dernieres donnees ecrites (et les acces au pool des tampons)
29/01/2015  YG   Utilisation d'un pointeur tampon suivant = -1 pour les voies
                 en emission en mode asynchrone.

*/



#include "driverIncludes.h"


/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCDEF(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int i, j, k, l, m;
    int afiltre;
    int nbta;
    int espion_tr; /* Indicateur RT est en mode "Espion Temps Reel" */
    int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */
    int tampapp;   /* Pour determiner le "pointeur de tampon application" */

    /* Definition d'une "voie 1553" */
    // printk("asdcioctl.c entree dans case ASDCDEF \n"); //ajout
    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));
    TVUTIL(zv, v, arg, sizeof(struct asdcvoie));

    /* En l'absence de l'instruction suivante, gcc affiche :        */
    /* "warning: ‘tmpi’ may be used uninitialized in this function" */
    /* Pour savoir si reel probleme, initialisation a une valeur    */
    /* esperee peu vraissemblable...                                */
    tmpi = 0xFFFFFFFF;

    /* En vue determination du "pointeur de tampon application" */
    tampapp = 0;

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    v->sous_adresse &= 0x1F;
    v->direction &= 0x01;
    v->nmots &= 0x1F;

    /* printk("ASDCDEF : bus=%d adr=%d sa=%d dir=%d nbmots=%d\n",
            v->bus, v->adresse, v->sous_adresse, v->direction, v->nmots);
    */

    /* Il faut reserver au moins un tampon */
    if ((v->ntamp==0) && (v->mode!=RT_VASYNC)) {
        cprintf("ASDCDEF ERREUR : ntamp=0 et mode!=ASYNC\n");
        RETURN(EINVAL);
    }

    /* On controle quand meme la validite du mode */
    if (    (v->mode != RT_VSYNC)
         && (v->mode != RT_VASYNC)
         && (v->mode != RT_VSTAT)
         && (v->mode != RT_VSYNC2)
       )
      { cprintf("ASDCDEF ERREUR : Mode 0x%0X inconnu\n", v->mode);
        RETURN(EINVAL);
      }

    /* Le mode "synchrone 2" est utilisable en transmission seulement */
    if ((v->mode == RT_VSYNC2) && (v->direction != 1))
      { cprintf("ASDCDEF ERREUR : Mode SYNC2 et RECEPTION\n");
        RETURN(EINVAL);
      }

    /* RT programme en "espion TR" ? */
    espion_tr = L(L(PROPTR) + v->adresse + 32 * v->bus) & 2;

    /* Nombre de tampons a allouer */
    if (v->mode == RT_VSYNC2)
      { if (v->adrtamp2 != 0)
          { nbta = v->adrtamp2;
          }
        else
          { nbta = v->ntamp * 2 + 2;
          }
      }
    else
      { nbta = v->ntamp;
      }

    /* Allocation si necessaire de la table des sous_adresses */
    j = L(ATPTR) + v->adresse + 32 * v->bus;
    if (L(j)==0)
       { int tcc;   /* Pointeur tampon des commandes codees */

         k = asdcalloc(dst, 128);
         if (k==-1) {
             cprintf("ASDCDEF ERREUR : Mem. echange pleine\n");
             RETURN(ENOMEM);     /* Plus de memoire */
         }
         E(j, k);
         for (i=k; i<k+128; i++) E(i, 0);

         /* Initialisation du temps de reponse a la valeur par defaut */
         E(j + 64, L(RSPGPS));

         /* Creation et initialisation a 0 du tampon des commandes codees */
         tcc = asdcalloc(dst, 34);
         if (tcc == -1) {
             cprintf("ASDCDEF ERREUR : Mem. echange pleine\n");
             RETURN(ENOMEM);     /* Plus de memoire */
         }
         for (i=tcc; i<tcc+34; i++) {
             E(i, 0);
             EI(i, 0, -1);     /* RAZ memoire image egalement */
         }

         /* Rebouclage du tampon sur lui-meme */
         E(tcc, tcc);

         /* Le meme tampon est affecte aux CC des sous-adresses 0 et 31 */
         E(k, tcc);
         E(k+31, tcc);
       }
      else k = L(j);  /* Preparation suite ... */

    /* Calcul de l'adresse du pointeur de tampon */
    k += (v->direction ? 32 : 0) + v->sous_adresse;

    /* La voie a pu etre deja declaree */
    if (L(k)!=0) {
        cprintf("ASDCDEF ERREUR : Redefinition de la voie %s%d,%d,%d\n",
                 v->direction ? "T" : "R",
                 v->bus, v->adresse, v->sous_adresse);
        RETURN(EADDRINUSE);      /* Voie deja declaree */
    }

    /* Programmation du nombre de mots legal */
    E(k + 64, v->nmots);


    /* k = adresse du ptr des tampons */


    /* Allocation si necessaire de la table des filtres */
    /* et de l'image de la table des filtres            */
    j = L(FTPTR) + v->adresse + 32 * v->bus;
    m = L(j);
    if (m==0)
       { m = asdcalloc(dst, 64);
         if (m==-1) { RETURN(ENOMEM);     /* Plus de memoire */
                    }
         E(j, m);
         for (i=m; i<m+64; i++)
            { E(i, 0);
              EI(i, 0, 383);
            }
       }

    /* Calcul de l'adresse du filtre */
    afiltre = m + v->sous_adresse + ( v->direction ? 32 : 0);

    /* La voie peut aussi avoir ete declaree, puis inhibee */
    if (LI(afiltre, 116) != 0) {
        cprintf("ASDCDEF ERREUR : Redefinition de la voie %s%d,%d,%d\n",
                 v->direction ? "T" : "R",
                 v->bus, v->adresse, v->sous_adresse);
         cprintf("                Cette voie est inhibee !!!\n");
         RETURN(EADDRINUSE);      /* Voie deja declaree */
    }


    { /* Des tampons sont maintenant (30/4/02) systematiquement alloues */
      int taille_alloc, ii;
      taille_alloc = (v->adrtamp == 0xFFFF) ? v->nmots : 32;
      if (taille_alloc == 0) taille_alloc = 32;

      /* Allocation du premier tampon */
      j = asdcalloc(dst, taille_alloc + 2);
      if (j==-1) {
          cprintf("ASDCDEF ERREUR (1) : Echec alloc. tampons pour"
                  " %s%d,%d,%d\n",
                  v->direction ? "T" : "R",
                  v->bus, v->adresse, v->sous_adresse);
          RETURN(ENOMEM);          /* Plus de memoire */
      }

      /* Initialisation a zero du tampon pour eviter pbs lies a mauvaise  */
      /* interpretation des bits de forts poids (16 a 31) par l'emetteur  */
      /* 1553 d'EMUABI. La meme initialisation est faite sur la memoire   */
      /* image a tout hasard...                                           */
      for (ii = j; ii < (j + taille_alloc + 2); ii++) {
          E(ii, 0);
          EI(ii, 0, -1);
      }

      EI(j+IRSEM, 0, 0);        /* Initialisation a 0 imperative */
      EI(j+IRMEMOF, 0, 0);      /* Initialisation a 0 imperative */
      EI(j+IRMEMOE, 0, 0);      /* Initialisation a 0 facultative */
      EI(j+IRTLET, 0, 0);       /* Initialisation a 0 facultative */
      EI(j+IRAFILTRE, 0, 0);    /* Initialisation a 0 facultative */
      E(j+1, TRT_LIBRE);
      EI(j + IRTPREC, 0, 1000);
      if (v->mode != RT_VSYNC2)
        { E(k, j);
        }
      else
        { E(k, RT_FIN2);
        }

      /* Renvoi de l'adresse choisie */
      v->adrtamp = j;

      /* Allocation du ou des tampons suivants */
      if (    (v->mode == RT_VSYNC)
           || (v->mode == RT_VSTAT)
           || (v->mode == RT_VSYNC2)
         )
        { /* Voie synchrone ou statique */
          for (i=2; i<=nbta; i++)
             { int ii;

               l = asdcalloc(dst, taille_alloc + 2);
               tmpi = l;
               if (l==-1) { cprintf("Echec 5\n");
                   cprintf("ASDCDEF ERREUR (2) : Echec alloc. "
                           "tampons pour %s%d,%d,%d\n",
                           v->direction ? "T" : "R",
                           v->bus, v->adresse, v->sous_adresse);
                   RETURN(ENOMEM);     /* Plus de memoire */
               }

               /* Initialisation a zero du tampon pour eviter pbs lies a mauvaise  */
               /* interpretation des bits de forts poids (16 a 31) par l'emetteur  */
               /* 1553 d'EMUABI. La meme initialisation est faite sur la memoire   */
               /* image a tout hasard...                                           */
               for (ii = l; ii < (l + taille_alloc + 2); ii++) {
                   E(ii, 0);
                   EI(ii, 0, -1);
               }

               EI(j+IRSEM, 0, 0);
               E(l+1, TRT_LIBRE);
               E(j, l);
               EI(l+IRTPREC, j, 303);
               j = l;
             }
          v->adrtamp2 = 0;

          /* Rebouclage de la chaine des tampons (sauf "asynchrone 2") */
          if (v->mode != RT_VSYNC2)
            { E(j, L(k));
              EI(L(k)+IRTPREC, j, 304);
            }
          else
            { /* Si "asynchrone 2", pas de rebouclage */
              E(j, 0);    // Terminaison par 0
              EI(L(k) + IRTPREC, 0, 390);
             }
        }
      else
        { int ii;

          /* Voie asynchrone */
          m = asdcalloc(dst, taille_alloc + 2);
          if (m==-1) { 
              cprintf("ASDCDEF ERREUR (3) : Echec alloc. "
                      "tampons pour %s%d,%d,%d\n",
                      v->direction ? "T" : "R",
                      v->bus, v->adresse, v->sous_adresse);
              RETURN(ENOMEM);     /* Plus de memoire */
          }

          /* Initialisation a zero du tampon pour eviter pbs lies a mauvaise  */
          /* interpretation des bits de forts poids (16 a 31) par l'emetteur  */
          /* 1553 d'EMUABI. La meme initialisation est faite sur la memoire   */
          /* image a tout hasard...                                           */
          for (ii = m; ii < (m + taille_alloc + 2); ii++) {
              E(ii, 0);
              EI(ii, 0, -1);
          }


          E(m+1, TRT_LIBRE);
          E(m , 0xFFFFFFFF);          /* Pour l'instant, tampons separes */
          EI(m+IRTPREC, j, 305);
          v->adrtamp2 = m;
          tampapp = m;

          /* "Rebouclage" de la chaine des tampons en memoire image */
          /* mais pas en memoire echange  (tampons separes)         */
          E(j, 0xFFFFFFFF);
          EI(L(k)+IRTPREC, m, 306);
        }
    }

    /* Programmation du filtre : Inhibition IT voie */
    E(afiltre, L(afiltre) & ~2);

    /* Si asynchrone :                                 */
    /*   si non espion_TR et transmission :            */
    /*   - alors les 2 tampons doivent etre separes    */
    /*   - dans tous les autres cas, les 2 tampons     */
    /*     doivent etre reboucles                      */
    if (v->mode == RT_VASYNC)
      { if (!(!espion_tr && v->direction))
          { /* Rebouclage des 2 tampons */
            E(v->adrtamp, v->adrtamp2);
            E(v->adrtamp2, v->adrtamp);
            tampapp = v->adrtamp;
          }
      }

    /* Initialisation de la memoire image */
    j = L(L(ATPTR) + v->adresse + 32 * v->bus);
    k = j + v->sous_adresse + ( v->direction ? 32 : 0);
    EI(k, v->adrtamp, 307);       /* Memorisation zone de donnees */
    /* printk("pSatable=0x%X  sa=%d  dir=%d \n", j, v->sous_adresse, v->direction); */
    /* printk("ZD : @0x%X contient 0x%X\n", k, v->adrtamp); */

    m = v->adrtamp;
    EI(m + IRCMD,   ((v->adresse << 11) & 0xF800)
                  | (v->direction ? 0x400 : 0)
                  | ((v->sous_adresse << 5) & 0x3E0), 308);
    EI(m + IRMODE, v->mode, 309);
    EI(m + IRTCAPP, tampapp ? tampapp : v->adrtamp, 310);
    EI(m + IRNBT, ((v->mode == RT_VASYNC) ? 2 : v->ntamp), 311);
    EI(m + IRSEM, 0, 312);        /* Semaphore voie */
    EI(m + IRCEVT, 0, 313);       /* Pas de connexion CEVT */

    /* Si mode "synchrone 2", initialisation particuliere */
    if (v->mode == RT_VSYNC2)
      {
        EI(m + IRTCH1, 0, 391);
        EI(m + IRTCH2, 0, 392);
        EI(m + IRTFCH1, 0, 393);
        EI(m + IRTFCH2, 0, 394);
        EI(m + IRTFCHINU, v->adrtamp, 395);
        EI(m + IRTCHINU, tmpi, 396);
        EI(m + IRTCACHE, 0, 397);
        EI(m + IRNBT, nbta, 1000);
        EI(m + IRTNMT, v->ntamp, 1000);
        EI(m + IRTNCH1, 0, 1000);
        EI(m + IRTNCH2, 0, 1000);

        /*####################################################*/
        /* Dans ce cas precis, si Espion TR :                 */
        /* pour le moment, RT non defini !                    */
        /* Si, ulterieurementy, RT defini, ecrire ici le code */
        /* de configuration des tampons en espion TR !        */
        /*####################################################*/
      }

    /* Initialisation eventuelle de la FIFO destinee a memoriser les */
    /* tampons d'une voie en emission.                               */
    if (v->direction) {
        unsigned long s;
        fifoTampons_t * pfifo;

        pfifo = &(dst->pTampEcr[v->bus][v->adresse][v->sous_adresse]);

        spin_lock_irqsave(&pfifo->lock, s);   /* Vraiment utile ici ??? */
            pfifo->demis = NULL;
            pfifo->p = NULL;
            pfifo->d = NULL;
            pfifo->nbt = 0;
        spin_unlock_irqrestore(&pfifo->lock, s);
    }


    /* Retour des donnees a l'appli appelante */
    VUTIL(zv, v, arg, sizeof(struct asdcvoie));


    /* On laisse le RT dans l'etat ou il etait avant l'appel */
    /* Inhibe (etat initial), espion_TR ou valide ...        */
    RETURN(OK);


    /*****************************************************************/
    /*   ATTENTION : Erreurs ENOMEM ci-avant : dans ces cas, on      */
    /*   devrait liberer la memoire allouee dans la meme operation   */
    /*   ou, autre methode, detecter l'erreur avant la premiere      */
    /*   allocation.                                                 */
    /*****************************************************************/
}

