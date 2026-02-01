
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCECR : Fonction ioctl d'ecriture dans un tampon abonne.


 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
31/05/2013  YG   Correction bug dans calcul zd si RT sur bus 2
12/06/2014  YG   Memorisation du tampon ecrit dans la FIFO associee aux
                 sous-adresses en emission.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
13/06/2014  YG   Rassemblement des includes dans un seul fichier
27/01/2015  YG   Inhibition des interruptions pendant les acces a la FIFO des
                 dernieres donnees ecrites
29/01/2015  YG   Utilisation d'un pointeur tampon suivant = -1 pour les voies
                 en emission en mode asynchrone.
 5/02/2015  YG   Apres ecriture du pointeur des tampons, on verifie la valeur
                 ecrite.
*/

 /*****************************************************/
 /* Ecriture dans un tampon :                         */
 /*  - Un tampon prepare (c'est a dire ecrit) et      */
 /*  non emis est reconnaissable par flag=TRT_NOUVEAU */
 /*  - Le nouveau tampon est ecrit a la suite de      */
 /*  tous les tampons "NOUVEAU" existants.            */
 /*  - Si tous les tampons existants sont "NOUVEAU",  */
 /*  rien n'est ecrit et une erreur ENOSR est         */
 /*  renvoyee.                                        */
 /*****************************************************/




#include "driverIncludes.h"

#ifdef DEBUG
#define dprintf PRINTF
#else
#define dprintf bidon
#endif


/// TODO : Seuls arguments sanss doute necessaires : dst et arg !!!

long
do_ASDCECR(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    struct asdctampon *t, zt;
    int i, j, k, l, m, mde;
    int prot2;
    unsigned int zd, cmd, z;
    long *oreiller;         /* Pointeur du semaphore ou dormir ... */
    /// ITFLAGS(s);             /* Pour masquage IT pendant section critique */
    int espion_tr;	    /* Indicateur RT est en mode "Espion Temps Reel" */
    int tmpi;   /* Memorisation tres temporaire adresse en memoire d'echange */
    int debordement;	 /* Indicateur debordement tampons E/S d'une voie RT */


/* Pour debug a l'oscilloscope */
#define settag0 ecriturePCIe(vfxNum, 0, lecturePCIe(vfxNum, 0, (int *)(pvargs[vfxNum]->base2 + (0x9094))) & 0xFFFFFFFD, (int *)(pvargs[vfxNum]->base2 + (0x9094)))
#define clrtag0 ecriturePCIe(vfxNum, 0, lecturePCIe(vfxNum, 0, (int *)(pvargs[vfxNum]->base2 + (0x9094))) | 0x00000002, (int *)(pvargs[vfxNum]->base2 + (0x9094)))

    // printk("asdcioctl.c entree dans case ASDCECR \n"); //ajout
    TDUTIL(arg, sizeof(struct asdctampon));
    DUTIL(zt, t, arg, sizeof(struct asdctampon));
    TVUTIL(zt, t, arg, sizeof(struct asdctampon));
    
    clrtag0;
//    printk("ct0\n");

    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    t->v.bus &= 0x01;
    t->v.adresse &= 0x1F;
    t->v.sous_adresse &= 0x1F;
    /* t->v.direction : Ecriture ==> imperativement TRANSMISSION */
    t->v.direction = 1;
    if (t->nbr > 32) t->nbr = 32;

    /* printk("ASDCECR : bus=%d adr=%d sa=%d dir=%d\n",
            t->v.bus, t->v.adresse, t->v.sous_adresse, t->v.direction);
    */
    
    /* La table des sous_adresses existe-t-elle ? */
    if ((i = L(L(ATPTR) + t->v.adresse + 32 * t->v.bus)) == 0) {
        cprintf("ASDCECR ERREUR : RT %s%d,%d,%d non defini\n",
                 t->v.direction ? "T" : "R",
                 t->v.bus, t->v.adresse, t->v.sous_adresse);
        RETURN(EADDRNOTAVAIL);   /* Le RT n'a jamais ete declare ! */
      }

    /* Pointeur vers zone des donnees en memoire image */
    zd = LI(i + t->v.sous_adresse + 32, 13);
    if ((zd < DEBUT_RAM) || (zd > FIN_RAM))
      { printk("ASDCECR ERREUR : Echec zd (zd[0x%X] = 0x%X)\n",
                 i + t->v.sous_adresse + 32, zd);
        /* Correspond a une tentative d'ecriture de donnees */
        /* dans une voie en lecture !                       */
        RETURN(ESPIPE);         /* Memoire image anormale ! */
      }


    /* Mode de la voie (synchrone, asynchrone, etc...) */
    m = LI(zd + IRMODE, 15);

    /* La voie existe-t-elle ? */
    j = L(i + t->v.sous_adresse + 32);
    if ((j == 0) && (m != RT_VSYNC2))
      { cprintf("Echec 2\n");
        RETURN(EADDRNOTAVAIL);      /* La voie n'a jamais ete declaree ! */
      }


    /* Le contenu de la memoire image semble-t-il correct ? */
    cmd = (t->v.adresse << 11) | 0x400 | (t->v.sous_adresse << 5);
    if ((LI(zd+IRCMD, 14) & 0xFFE0) != cmd)
      { cprintf("Echec 4\n");
        RETURN(ESPIPE);            /* Memoire image anormale ! */
      }


    /* Validation, si necessaire, des ITs associees a la voie */
    /* Depuis l'association d'une FIFO soft a chaque sous-adresse en */
    /* emission, l'interruption doit TOUJOURS etre validee pour      */
    /* permettre de disposer, en sortie de la FIFO, du dernier       */
    /* emis sur le bus.                                              */
    /* if (t->f == SDC_ATTENDRE) */
      { int ef;

        /* Pointeur table des filtres associee a adresse */
        ef = L(L(FTPTR) + t->v.adresse + 32 * t->v.bus);

        /* Adresse du filtre */
        ef += t->v.sous_adresse + ( t->v.direction ? 32 : 0);

        /* Programmation du filtre : Validation IT voie */
        E(ef, L(ef) | 2);
      }



    /* RT en mode espion ? */
    espion_tr = L(L(PROPTR) + t->v.adresse + 32 * t->v.bus) & 2;

    /* Si oui, operation invalide ! */
    if (espion_tr)
      { cprintf("Echec 5\n");
        RETURN(EXDEV);
      }




    /* Si mode "synchrone 2", traitement particulier */
    if (m == RT_VSYNC2)
      { int pch;
        int ttrait;	/* Indicateur "tampon traite" */
                          /* (et, donc, recyclable)     */

        /* Memorisation d'un debordement potentiel */
        debordement = (j == RT_FIN2);

        /* Extraction du bit SDC_PROT2 */
        prot2 = t->f & SDC_PROT2;
        t->f &= ~SDC_PROT2;

        switch (t->f)
          {
            case SDC_RAZRAZ :
            case SDC_RAZ :
              /* Normalement, on devrait regarder si un traitement */
              /* n'est pas en cours : on fait l'impasse sur cette  */
              /* eventualite ...                                   */

              /*** RAZ :                                           ***/
              /*** Tous les tampons sont mis dans la chaine des    ***/
              /*** tampons disponibles (c'est a dire "inutilises") ***/

              /*####################################
                   MODIF. A PREVOIR :
                   Pour eviter les pbs, tous les
                   tampons devraient avoir leur
                   ptr "suivant" en mem. echange
                   pointant vers 0 ou vers RT_FIN2
                ####################################*/

              if (!prot2)
                { /* Accrochage chaine 2 aux tampons inutilises */
                  pch = LI(zd + IRTCH2, 1000);
                  if (pch)
                    { EI( LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                      LI(zd + IRTFCH2, 1000), 1000);
                      EI(zd + IRTFCHINU, LI(zd + IRTCH2, 1000), 1000);
                      EI(zd + IRTCH2, 0, 1000);
                      EI(zd + IRTFCH2, 0, 1000);
                      EI(zd + IRTNCH2, 0 , 1000);
                    }
                }

              /* Accrochage chaine 1 aux tampons inutilises */
              pch = LI(zd + IRTCH1, 1000);
              if (pch)
                { EI( LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                  LI(zd + IRTFCH1, 1000), 1000);
                  EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                  EI(zd + IRTCH1, 0, 1000);
                  EI(zd + IRTFCH1, 0, 1000);
                  EI(zd + IRTNCH1, 0 , 1000);
                  E(i + t->v.sous_adresse + 32, RT_FIN2);
                }

              /* Si SDC_RAZRAZ, on a termine ! */
              if (t->f == SDC_RAZRAZ) RETURN(OK);

              /* On est dans le cas SDC_RAZ        */
              /*      ==> reste a ecrire un tampon */


            case SDC_NONBLOQ :
            case SDC_ATTENDRE :
               // printk("entree dans case SDC_ATTENDRE switch "
               //        "(t->f)ASDCECR \n"); //ajout
               ttrait = 0;

              /* Y a-t-il des tampons deja lus a recycler ?  */
              tmpi = L(i + t->v.sous_adresse + 32);
              z = LI(zd + IRTCH1, 1000);
              if (tmpi != z)
                { /* Il y a des tampons a recycler ! */
                  ttrait = 1;
                }
              else
                {
                  /* Peut-on encore ajouter un tampon a la chaine ? */
                  if (    (LI(zd + IRTNCH1 , 1000)
                                >=  LI(zd + IRTNMT , 1000))
                       || (LI(zd + IRTCHINU, 1000) == 0)
                     )
                    { /* Chaine pleine ou plus de tampons disponibles */
                      if (t->f == SDC_NONBLOQ)
                        { RETURN(ENOSR);
                        }

                      /* Attente de la liberation d'un tampon */

                      /* Semaphore ou attendre */
                      oreiller = &LI(zd + IRSEM, 1000);

                      /* -------------------------------------------- */
                      /* ---   Debut section critique   ----- vvvvvvv */
                      /// DISABLE(s);    TODO ???

                      tmpi = 0; /* Pour pouvoir passer tests  */
                                /* en sortie section critique */
                                /* si swait non appele        */

                      /* Tampon (de nouveau) disponible */
                      /* pour recyclage ?               */
                      z = LI(zd + IRTCH1, 1000);
                      if (z == L(i + t->v.sous_adresse + 32))
                        { /* Plus de tampon dispo ! */
                          /* Demasquage eventuel IT sur ABI */
                          if ((++dst->nombre_it)==1)
                            {    /***********************************/
                                 /* DEMASQUAGE IT ABI doit etre ICI */
                                 /***********************************/
                            }

                          // dprintf("ASDC : j'attend l'IT !\n");
                          // printk("ASDC : j'attend l'IT !\n"); //ajout
                          tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
                          // printk("ASDC : IT arrivee !\n");
                          //ajout
                          if (!(--dst->nombre_it))
                            {  /*********************************/
                               /* MASQUAGE IT ABI doit etre ICI */
                               /*********************************/
                            }

                        }  /* Fin du second "Si tampons-traites" */

                      /// RESTORE(s);   TODO ???
                      /* ---   Fin section critique   ------- ^^^^^^^ */
                      /* -------------------------------------------- */

                      if (tmpi) /* Passage semaphore du a un signal ? */
                        { RETURN(EINTR);
                        }

                      if (dst->raz) /* Passage semaphore du a RAZ ? */
                        { RETURN(ENETRESET);
                        }

                      ttrait = 1;	  /* Il y a des tampons a recycler */
                    }
                  }	/* Fin du premier "Si tampons-traites" */


              /* Recyclage eventuel des tampons deja ecrits */
              if (ttrait)
                { int tcour;	/* Adr. tampon courant */

                  tcour = L(i + t->v.sous_adresse + 32);
                  if (tcour == RT_FIN2)
                    { /* La chaine 1 a ete entierement traitee */
                      EI(LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                    LI(zd + IRTFCH1, 1000), 1000);
                      EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                      EI(zd + IRTCH1, 0, 1000);
                      EI(zd + IRTFCH1, 0, 1000);
                      EI(zd + IRTNCH1, 0, 1000);
                    }
                  else
                    { /* La chaine 1 n'est pas encore */
                      /* entierement traitee          */

                      int tprec;	/* Adr. tampon precedent */
                      int n1;

                      tprec = LI(tcour + IRTPREC, 1000);
                      EI(tcour + IRTPREC, 0, 1000);
                      { /* Comptage des tampons a recycler */
                        int p;
                        n1 = 0;
                        p = tprec;
                        while(p)
                          { n1++;
                            p = LI(p + IRTPREC, 1000);
                          }
                      }

                      EI(LI(zd + IRTFCHINU, 1000) + IRTPREC,
                                               tprec, 1000);
                      EI(zd + IRTFCHINU, LI(zd + IRTCH1, 1000), 1000);
                      EI(zd + IRTCH1, tcour, 1000);
                      EI(tcour + IRTPREC, 0, 1000);
                      EI(zd + IRTNCH1, LI(zd + IRTNCH1, 1000) - n1,
                                                                1000);
                    }
                }


              /* Il reste a ecrire le tampon                        */
              /* (et on est maintenant certain de pouvoir le faire) */

              /* Retrait d'un tampon de la chaine des t. inutilises */
              l = LI(zd + IRTCHINU, 1000);
              EI (zd + IRTCHINU, LI(l + IRTPREC, 1000), 1000);

              /* Ecriture du tampon */
              for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
              E(l+1, TRT_NOUVEAU);
              E(l, RT_FIN2);

              /* Insertion du tampon dans chaine 1 */
              /* Chainage direct : */
              E(l, 0);
              if (LI(zd + IRTCH1, 1000) == RT_FIN2)
                { /* Chaine 1 est vide ! */
                  EI(zd + IRTCH1, l, 1000);
                  EI(zd + IRTFCH1, l, 1000);
                  EI(l + IRTPREC, 0, 1000);
                  E(i + t->v.sous_adresse + 32, l);
                }
              else
                { /* Chaine 1 non vide */
                  E(LI(zd + IRTFCH1, 1000), l);
                  EI(l + IRTPREC, LI(zd + IRTFCH1, 1000), 1000);
                  EI(zd + IRTFCH1, l, 1000);
                }

              /* Incrementation du nombre des tampons utilises */
              EI(zd + IRTNCH1, LI(zd + IRTNCH1 , 1000) + 1, 1000);

              /* On n'a aucun moyen de savoir si un debordement */
              /* a vraiment eu lieu (il est peut-etre en train  */
              /* d'avoir lieu, mais on n'en sait rien encore    */
              /* car le firmware ne permet pas de savoir si un  */
              /* transfert est en cours ...)                    */
              /* On avertit donc si un debordement est          */
              /* potentiellement possible (si le tampon courant */
              /* est le dernier de la chaine) ...               */
              /* Ce n'est donc pas parce que le fanion          */
              /* "debordement" est monte qu'un debordement a eu */
              /* lieu.                                          */
              /* Un vrai debordement se traduit par un bit      */
              /* erreur monte dans le status.                   */
              /*                                                */
              /* Remarquons que le fanion "debordement" sera    */
              /* toujours monte lors de la premiere ecriture    */
              /* dans la voie !                                 */

              if (debordement) t->f |= SDC_DEBORD;
              VUTIL(zt, t, arg, sizeof(struct asdctampon));

              RETURN(OK);

            default :
              RETURN(ENXIO);
          }
      }



      /* Ci-dessous, modes autres que "synchrone 2" */

      /* Tampons separes ? */
      z = ((LI(zd + IRNBT, 16) == 2) && (L(j) == 0xFFFFFFFF));
                                             /* z vrai si t. separes */

      /* Traitement de l'appel si tampons separes */
      if (z)
        {
          unsigned long s;
          int pb;

          fifoTampons_t * pfifo;
          l = LI(zd + IRTCAPP, 17);	/* Adr. tampon appli courant */
          pb = 0;

          /* Il arrive parfois, si la sous-adresse est tres sollicitee, */
          /* que j pointe le mauvais tampon.                            */
          /* Le code ci-dessous detecte cette situation et la corrige.  */
          /* Les barrieres ont ete mise en place a un moment ou, en     */
          /* phase de debug, le message etait attendu et n'apparaissait */
          /* pas. Il est tres possible qu'elles soient inutiles...      */
          barrier();
          if (j==l) { 
            printk("ASDC: Correction perte pointeur (AG) bus=%d adr=%d sa=%d\n",
                   t->v.bus, t->v.adresse, t->v.sous_adresse);
            j = LI(l + IRTPREC, 444);   /* Relecture adr. de l'autre tampon */
            pb = 1;
          }
          barrier();

          /* Copie des donnees */
          for (k=0; k<t->nbr; k++) {
               E(l+2+k, t->t[k]);
               if (pb) printk("d:%04x ", t->t[k]); 
          }
          if (pb) printk("f\n");
          E(l+1, TRT_NOUVEAU);

          /* Memorisation des donnees pour relecture du dernier tampon emis */
          pfifo = &(dst->pTampEcr[t->v.bus][t->v.adresse][t->v.sous_adresse]);
          spin_lock_irqsave(&pfifo->lock, s);
            if (pfifo->p == NULL) pfifo->p = prendre();
            for (k=0; k<t->nbr; k++) pfifo->p->cargo[k+2] = t->t[k];
          spin_unlock_irqrestore(&pfifo->lock, s);
          
          
/* Remarque : Initialement, le code utilise ci-dessous etait reduit a :
 * 
 *           EI(zd + IRTCAPP, j, 315);
 *           E(i + t->v.sous_adresse + 32, l);
 *           usec_sleep(1);
 *           E(i + t->v.sous_adresse + 32, l);
 *
 * Les branches "Si echec, on recommence" et les barrieres associees ont
 * ete ajoutees quand le bon fonctionnement de la memoire double port a
 * ete mis en doute.
 * Depuis, cette memoire n'est plus utilisee que comme une memoire simple port.
 * Ces branches pourraient donc (devraient !) etre supprimees.
 * La double ecriture initiale (cf. ci-dessus) est suffisante pour forcer
 * la mise a jour correcte du pointeur, meme si le coupleur effectue lui
 * aussi une mise a jour au meme moment.
 */

          /* Forcage du basculement des tampons */
          EI(zd + IRTCAPP, j, 315);
          E(i + t->v.sous_adresse + 32, l);
	  barrier();
	  /* Si echec, on recommence */
	  if (L(i + t->v.sous_adresse + 32) != l) {
	      E(i + t->v.sous_adresse + 32, l);
	      printk("Reecriture (1)  l=0x%X\n", l);
	  }
          usec_sleep(1);
          E(i + t->v.sous_adresse + 32, l);
	  barrier();
	  /* Si echec, on recommence */
	  if (L(i + t->v.sous_adresse + 32) != l) {
	      E(i + t->v.sous_adresse + 32, l);
	      printk("Reecriture (2) l=0x%X\n", l);
	  }

          VUTIL(zt, t, arg, sizeof(struct asdctampon));
          settag0;
//          printk("st0\n");
          RETURN(OK);
        }

      /* Traitement selon le mode de l'appel si tampons chaines */
      debordement = 0;

      /* Mode de l'ecriture :                          */
      /*    SDC_ATTENDRE est remplace par SDC_NON_BLOQ */
      /*    si la voie est "statique"                  */
      if ((t->f == SDC_ATTENDRE) && (m == RT_VSTAT))
        { mde = SDC_NONBLOQ;
        }
      else
        { mde = t->f;
        }

      switch (mde)
        { case SDC_RAZRAZ :
          case SDC_RAZ :
             /* Remise a "zero" de tous les tampons */
             tmpi = j;
             for (l=0; l<LI(zd+IRNBT, 18); l++)
               { E(tmpi+1, TRT_LIBRE);
                 tmpi = L(tmpi);
               }
             /* Mise en coherence des pointeurs firmware et appli */
             EI(zd + IRTCAPP, j, 316);
             
             /* Supprimer les tampons memorises, sauf le dernier emis */
             viderFifo(&(dst->pTampEcr[t->v.bus]
                                         [t->v.adresse][t->v.sous_adresse]));

             /* Fin si RAZRAZ */
             if (t->f == SDC_RAZRAZ)
               {  RETURN(OK);    /* VUTIL inutile : rien a remonter ! */
               }

          case SDC_NONBLOQ :
             l = LI(zd + IRTCAPP, 19);   /* Adr. tampon appli courant */

             /* Si voie statique, on ignore les eventuels debordements */
             if (m != RT_VSTAT)
               { debordement =  ((L(l + 1) & 0xFFFF) != TRT_LIBRE);

                 /* Tampon dispo ? */
                 if ((L(L(l) + 1) & 0xFFFF) == TRT_NOUVEAU)
                   { /* Plus de tampon dispo ! */
                     if (debordement) t->f |= SDC_DEBORD;
                     VUTIL(zt, t, arg, sizeof(struct asdctampon));
                     RETURN(ENOSR);
                   }
               }

             /* Ecriture du tampon */
             for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
             E(l+1, TRT_NOUVEAU);

             /* Mise a jour du pointeur application */
             tmpi = L(l);
             EI(zd + IRTCAPP, tmpi, 317);
             E(tmpi + 1, TRT_LIBRE);
             
             /* Memorisation du tampon dans la FIFO soft */
             {   tampon_t * pt;
                 pt = prendre();
                 if (pt != NULL) {  /* Si plus de tampons, on ignore le Pb */
                     pt->cargo[0] = 0xFFFF;
                     pt->cargo[1] = 0xFFFF;
                     for (i=0; i<t->nbr; i++) pt->cargo[i+2] = t->t[i];
                     ecrireFifo(&(dst->pTampEcr[t->v.bus][t->v.adresse]
                                                    [t->v.sous_adresse]), pt);
                 } else {
                     printk("ASDCECR: ERREUR: Plus de tampons dans le pool\n");
                 }
             }

             if (debordement) t->f |= SDC_DEBORD;
             VUTIL(zt, t, arg, sizeof(struct asdctampon));
             RETURN(OK);


          case SDC_ATTENDRE :
                 // printk("entree dans case SDC_ATTENDRE"
                 // " switch(mde) ASDCECR \n"); //ajout
                 l = LI(zd + IRTCAPP, 20);	/* Adr. tamp. appli courant */

             debordement =  ((L(l + 1) & 0xFFFF) != TRT_LIBRE);

             /* Semaphore ou attendre */
             oreiller = &LI(zd + IRSEM, 21);

             dprintf("sleep : oreiller RT(%d,%d,%c)\n",
                     t->v.adresse, t->v.sous_adresse,
                     t->v.direction ? 'T' : 'R');

             /* ---------------------------------------------------- */
             /* ---   Debut section critique   ------------- vvvvvvv */
             /// DISABLE(s);   TODO ???

             tmpi = 0;	/* Pour pouvoir passer tests en sortie  */
                        /* section critique si swait non appele */

             if ((L(L(l) + 1) & 0xFFFF) == TRT_NOUVEAU)
                                     /* Tampon disponible ? */
               { /* Plus de tampon dispo ! */

                 /* Demasquage eventuel IT sur ABI */
                 if ((++dst->nombre_it)==1)
                   {    /***********************************/
                        /* DEMASQUAGE IT ABI doit etre ICI */
                        /***********************************/
                   }

                 dprintf("Oreiller = I(%d) ==> 0x%X\n",
                          zd + IRSEM, oreiller);
                 // dprintf("ASDC : j'attend l'IT !\n");
               // printk("ASDC : j'attend l'IT !\n"); //ajout
                 tmpi = SWAIT_IM(oreiller, SEM_SIGABORT);
               // printk("ASDC : apr�s le \"j'attend l'IT\" !\n"); //ajout
                 if (!(--dst->nombre_it))
                   {  /*********************************/
                      /* MASQUAGE IT ABI doit etre ICI */
                      /*********************************/
                   }
               }

             /// RESTORE(s);    TODO ???
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


             /* Ecriture du tampon */
             for (k=0; k<t->nbr; k++) E(l+2+k, t->t[k]);
             E(l+1, TRT_NOUVEAU);

             /* Mise a jour du pointeur application */
             tmpi = L(l);
             EI(zd + IRTCAPP, tmpi, 318);
             E(tmpi + 1, TRT_LIBRE);
             
             /* Memorisation du tampon dans la FIFO soft */
             {   tampon_t * pt;
                 pt = prendre();
                 if (pt != NULL) {  /* Si plus de tampons, on ignore le Pb */
                     pt->cargo[0] = 0xFFFF;
                     pt->cargo[1] = 0xFFFF;
                     for (i=0; i<t->nbr; i++) pt->cargo[i+2] = t->t[i];
                     ecrireFifo(&(dst->pTampEcr[t->v.bus][t->v.adresse]
                                                    [t->v.sous_adresse]), pt);
                 } else {
                     printk("ASDCECR: ERREUR: Plus de tampons dans le pool\n");
                 }
             }

             if (debordement) t->f |= SDC_DEBORD;
             VUTIL(zt, t, arg, sizeof(struct asdctampon));
             RETURN(OK);


          default :
             RETURN(ENXIO);
        }
}

