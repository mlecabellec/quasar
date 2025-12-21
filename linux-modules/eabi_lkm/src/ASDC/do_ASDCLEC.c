
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCLEC : Fonction ioctl de lecture d'un tampon abonne.

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
12/06/2014  YG   Si sous-adresse en ecriture, lecture du dernier tampon emis
                 (en utilisant la FIFO associee aux sous-adresses en emission).
13/06/2014  YG   Rassemblement des includes dans un seul fichier
27/01/2015  YG   Inhibition des interruptions pendant les acces a la FIFO des
                 dernieres donnees ecrites (et les acces au pool des tampons)

*/

    /****************************************************/
    /* Lecture d'un tampon :                            */
    /*  - Un tampon n'ayant pas recu de donnees depuis  */
    /*  sa derniere lecture est caracterise par         */
    /*  flag=TRT_LIBRE .                                */
    /*  - Le premier tampon non "LIBRE" est lu, puis il */
    /*  est marque "TRT_LIBRE" .                        */
    /*  - Si tous les tampons existants sont "LIBRE",   */
    /*  rien n'est lu et une erreur ENOSR est           */
    /*  renvoyee.                                       */
    /****************************************************/






#include "driverIncludes.h"



#ifdef DEBUG
#define dprintf PRINTF
#else
#define dprintf bidon
#endif

/// TODO : Seuls arguments sanss doute necessaires : dst et arg !!!

long
do_ASDCLEC(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdctampon *t, zt;
    int i, j, k, l, m, mde;
    unsigned int zd, cmd, z;
    int espion_tr;	/* Indicateur RT est en mode "Espion Temps Reel" */
    int tmpi; /* Memorisation tres temporaire adresse en memoire d'echange */
    int debordement;	/* Indicateur debordement tampons E/S d'une voie RT */
    long *oreiller;   /* Pointeur du semaphore ou dormir ... */
    /// ITFLAGS(s);    /* Pour masquage IT pendant section critique */


    /* DEBUT DE LA PARTIE SPECIFIQUE ASDCLEC */

    // printk("asdcioctl.c entree dans case ASDCLEC \n"); //ajout
    TDUTIL(arg, sizeof(struct asdctampon));
    DUTIL(zt, t, arg, sizeof(struct asdctampon));
    TVUTIL(zt, t, arg, sizeof(struct asdctampon));

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    t->v.bus &= 0x01;
    t->v.adresse &= 0x1F;
    t->v.sous_adresse &= 0x1F;
    t->v.direction &= 1;

    /* printk("ASDCLEC : bus=%d adr=%d sa=%d dir=%d\n",
            t->v.bus, t->v.adresse, t->v.sous_adresse, t->v.direction);
    */
    
    /* La "mise en forme" de t->v.direction est reportee */
    /* a un peu plus loin...                             */

    /* La table des sous-adresses existe-t-elle ? */
    if ((i = L(L(ATPTR) + t->v.adresse + t->v.bus * 32)) == 0) {
        RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
    }

    /* La table des mots d'etat est elle bien declaree ? */
    if (L(SWTPTR) == 0) {
        RETURN(EXDEV);          /* (Ne devrait jamais se produire !) */
    }

    /* Le RT est-il defini dans la table des status words ? */
    if (L(L(SWTPTR) + t->v.adresse + t->v.bus * 64) == 0) {
        cprintf("ASDCLEC (bus=%d adr=%d) ERREUR : "
                "Entree nulle dans table des Status Words\n",
                t->v.bus, t->v.adresse);
        RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
    }

    /* Le RT est-il simule ou en mode espion TR ? */
    if (L(L(PROPTR) + t->v.adresse + t->v.bus * 32) & 2)
      { /* Le RT est espionne ==> la voie peut-etre en EMISSION */
        /* comme en RECEPTION !                                 */
        espion_tr = 1;
      }
    else
      { /* Le RT est simule */
        espion_tr = 0;
      }

    /* La voie existe-t-elle ? */
    if ((j = L(i + t->v.sous_adresse + (t->v.direction ? 32 : 0))) == 0) {
        RETURN(EADDRNOTAVAIL); /* La voie n'a jamais ete declaree ! */
    }


    /* Le contenu de la memoire image semble-t-il correct ? */

    zd = LI(i + t->v.sous_adresse + (t->v.direction ? 32 : 0), 22);
    if ((zd < DEBUT_RAM) || (zd >= FIN_RAM )) {
        printk("EMUABI: Pb ASDCLEC: zd(@0x%X)=%d [0x%X]\n",
                i + t->v.sous_adresse + (t->v.direction ? 32 : 0), zd, zd);
        RETURN(ESPIPE);      /* Memoire image anormale ! */
    }

    cmd =   (t->v.adresse << 11)
          | (t->v.direction ? 0x400 : 0)
          | (t->v.sous_adresse << 5);

    if ((LI(zd+IRCMD, 23) & 0xFFE0) != cmd) {
        /* Memoire image anormale ! */
        cprintf("LI(zd+IRCMD)=0x%lX\n", LI(zd+IRCMD, 24));
        cprintf("cmd=0x%X\n", cmd);
        cprintf("---2---\n");
        RETURN(ESPIPE);
    }



    /* Validation, si necessaire, des ITs associees a la voie */
    if (t->f == SDC_ATTENDRE)
      { int ef;

        /* Pointeur table des filtres associee a adresse */
        ef = L(L(FTPTR) + t->v.adresse + 32 * t->v.bus);

        /* Adresse du filtre */
        ef += t->v.sous_adresse + ( t->v.direction ? 32 : 0);

        /* Programmation du filtre : Validation IT voie */
        E(ef, L(ef) | 2);
      }


    /* Si non espion TR et voie en emission, le dernier tampon emis (qui */
    /* a ete memorise) doit etre renvoye.                                */
    if (t->v.direction && !espion_tr)
        {
            unsigned long s;
            
            /* REMARQUE : Les option ASDC_RAZ, ASDC_ATTENDRE, etc... sont */
            /* toujours ignorees. Est-ce correct ???                      */
            fifoTampons_t * pfifo;
            pfifo = &(dst->pTampEcr[t->v.bus][t->v.adresse][t->v.sous_adresse]);
            
            spin_lock_irqsave(&pfifo->lock, s);
                if (pfifo->demis == NULL) {
                    spin_unlock_irqrestore(&pfifo->lock, s);
                    RETURN(ENOSR);            /* Pas de donnees disponibles */
                } else {
                    cmd = pfifo->demis->cargo[1];
                    t->nbr = cmd & 0x1F;
                    if (t->nbr == 0) t->nbr = 32;
                    for (k=0; k<t->nbr; k++) t->t[k] = pfifo->demis->cargo[k+2];
                }
            spin_unlock_irqrestore(&pfifo->lock, s);
            
            /* Envoi des donnees a l'utilisateur */
            VUTIL(zt, t, arg, sizeof(struct asdctampon));
            RETURN(OK);
        }

    /* Voie en mode synchrone, asynchrone ou statique ? */
    m = LI(zd + IRMODE, 25);

    /* La voie ne devrait pas etre en mode "synchrone 2"   */
    /* (sauf, peut-etre, si espion TR, mais ce type de     */
    /* fonctionnement n'est pas implemente pour le moment) */
    if (m == RT_VSYNC2) RETURN(ENOEXEC);

    /* si la voie est statique :                               */
    /*    - Le mode SDC_ATTENDRE est remplace par SDC_NONBLOQ  */
    /*    - La voie est alors traitee comme une voie synchrone */
    mde = t->f;
    if (m == RT_VSTAT)
      { m = RT_VSYNC;
        if (t->f == SDC_ATTENDRE)
          { mde = SDC_NONBLOQ;
          }
      }

    /* Tampons separes ? */
    z = ((LI(zd + IRNBT, 26) == 2) && (L(j) == j));
                                     /* z vrai si t. separes */

    /* Les tampons a lire par l'appli ne devraient pas etre separes */
    /* 	==> Pb ...                                                */
    if (z)
      { RETURN(EDOM);	/* Erreur interne ... */
      }


    /* Traitement des tampons chaines en mode asynchrone */
    if (m)
      {
        int autret, memoj, cmd;

        /* Traitement RAZ eventuelle et controle validite du mode */
        switch (mde)
          { case SDC_RAZRAZ :
            case SDC_RAZ :        /* "Remise a zero" des tampons */
                                  E(j+1, TRT_LIBRE);
                                  j = L(j);
                                  E(j+1, TRT_LIBRE);
                                  return OK;

            case SDC_NONBLOQ :
            case SDC_ATTENDRE :   break;

            default :             RETURN(ENXIO);
          }

        /* Adresse tampon 2 */
        autret = L(j);
        memoj = j;

        /* Lecture tampon 2 */
        cmd = L(autret+1);
        t->nbr = cmd & 0x1F;
        if (t->nbr == 0) t->nbr = 32;
        for (k=0; k<t->nbr; k++) t->t[k] = L(autret+2+k);

        /* Relecture du pointeur "firmware" */
        j = L(i + t->v.sous_adresse + (t->v.direction ? 32 : 0));

        if (j != memoj)
          {
            /* Il faut recommencer ! */
            autret = L(j);

            /* Lecture tampon 2 */
            cmd = L(autret+1);
            t->nbr = cmd & 0x1F;
            if (t->nbr == 0) t->nbr = 32;
            for (k=0; k<t->nbr; k++) t->t[k] = L(autret+2+k);
          }

        if (cmd == TRT_LIBRE) {
            RETURN(ENOSR);
        }

        VUTIL(zt, t, arg, sizeof(struct asdctampon));
        RETURN(OK);
      }

    /* Traitement des tampons chaines en mode synchrone */
    switch (mde)
      { case SDC_RAZRAZ :
        case SDC_RAZ :
           /* Remise a "zero" de tous les tampons */
           tmpi = j;
           for (l=0; l<LI(zd+IRNBT, 27); l++)
             { E(tmpi+1, TRT_LIBRE);
               tmpi = L(tmpi);
             }
           /* Mise en coherence des pointeurs firmware et appli */
           EI(zd + IRTCAPP, j, 319);

           /* Fin de la RAZ */
           RETURN(OK);

        case SDC_NONBLOQ :
           l = LI(zd + IRTCAPP, 28);	/* Adr. tampon appli courant */

           /* Debordement ? */
           debordement =
                    ((L(LI(l+IRTPREC, 29) + 1) & 0xFFFF) != TRT_LIBRE);

           if ((L(l + 1) & 0xFFFF) == TRT_LIBRE) {  /* Donnees dispo ? */
               /* Plus de donnees dispo ! */
               if (debordement) t->f |= SDC_DEBORD;
               VUTIL(zt, t, arg, sizeof(struct asdctampon));
               RETURN(ENOSR);
           }

           /* Lecture du tampon */
           t->nbr = L(l+1) & 0x1f;
           if (t->nbr == 0) t->nbr = 32;
           for (k=0; k<t->nbr; k++) t->t[k] = L(l+2+k);
           E(l+1, TRT_LIBRE);

           /* Mise a jour du pointeur application */
           EI(zd + IRTCAPP, L(l), 320);

           if (debordement) t->f |= SDC_DEBORD;
           VUTIL(zt, t, arg, sizeof(struct asdctampon));
           RETURN(OK);


        case SDC_ATTENDRE :
           // printk("entree dans case SDC_ATTENDRE"
           //        "  switch (mde) ASDCLEC\n");"

           l = LI(zd + IRTCAPP, 30);	/* Adr. tampon appli courant */

           /* Debordement ? */
           debordement =
                   ((L(LI(l+IRTPREC, 31) + 1) & 0xFFFF) != TRT_LIBRE);

           /* Semaphore ou attendre */
           oreiller = &LI(zd + IRSEM, 32);

           dprintf("sleep : oreiller RT(%d,%d,%c)\n",
                   t->v.adresse, t->v.sous_adresse,
                   t->v.direction ? 'T' : 'R');

           /* ---------------------------------------------------- */
           /* ---   Debut section critique   ------------- vvvvvvv */
           /// DISABLE(s);   TODO ???

           tmpi = 0;	/* Pour pouvoir passer tests en sortie  */
                        /* section critique si swait non appele */

           if ((L(l + 1) & 0xFFFF) == TRT_LIBRE) /* Donnees dispo ? */
             { /* Plus de donnees dispo ! */

               /* Demasquage eventuel IT sur ABI */
               if ((++dst->nombre_it)==1)
                 {    /***********************************/
                      /* DEMASQUAGE IT ABI doit etre ICI */
                      /***********************************/
                 }


               // dprintf("ASDC : j'attend l'IT !\n");
               // printk("ASDC : j'attend l'IT !\n"); //ajout
               tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
               //dprintf("ASDC : IT arrivee !\n");
               // printk("ASDC : IT arrivee !\n"); //ajout
               if (!(--dst->nombre_it))
                 {  /*********************************/
                    /* MASQUAGE IT ABI doit etre ICI */
                    /*********************************/
                 }
             }

           /// RESTORE(s);   TODO ???
           /* ---   Fin section critique   --------------- ^^^^^^^ */
           /* ---------------------------------------------------- */

           if (tmpi)	/* Passage semaphore du a un signal ? */
             { if (debordement) t->f |= SDC_DEBORD;
               VUTIL(zt, t, arg, sizeof(struct asdctampon));
               RETURN(EINTR);
             }

           if (dst->raz) /* Passage du semaphore du a une RAZ ? */
             { if (debordement) t->f |= SDC_DEBORD;
               VUTIL(zt, t, arg, sizeof(struct asdctampon));
               RETURN(ENETRESET);
             }


           /* Lecture du tampon */
           t->nbr = L(l+1) & 0x1F;
           if (t->nbr == 0) t->nbr = 32;
           for (k=0; k<t->nbr; k++) t->t[k] = L(l+2+k);
           E(l+1, TRT_LIBRE);

           /* Mise a jour du pointeur application */
           EI(zd + IRTCAPP, L(l), 321);

           if (debordement) t->f |= SDC_DEBORD;
           VUTIL(zt, t, arg, sizeof(struct asdctampon));
           RETURN(OK);


        default :
           RETURN(ENXIO);
      }
}

