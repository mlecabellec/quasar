
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCDEF : Fonction ioctl de creation d'une sous-adresse.
          Si l'adresse n'existe pas, elle est egalement creee.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
12/06/2014  YG   Prise en compte de la FIFO associee aux sous-adresses en
                 emission.
13/06/2014  YG   Rassemblement des includes dans un seul fichier
13/06/2014  YG   Rassemblement des includes dans un seul fichier
27/06/2014  YG   Correction bug sur liberation table des SA quand bus=1

*/


#include "driverIncludes.h"


#ifdef CEVT
#include "../CEVT/cevtctl.h"
#endif   /* CEVT */


static int liberer_tamp(struct asdc_varg *dst, unsigned short ptamp);




/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDC_ABO_LIBERER(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{
    struct asdcvoie *v, zv;
    int i, j, k, mm;
    unsigned int zd, cmd, iz;
    int tampapp;        /* Pour determiner le "pointeur de tampon application" */
    int vliee;
    int espion_tr;      /* Indicateur RT est en mode "Espion Temps Reel" */


    TDUTIL(arg, sizeof(struct asdcvoie));
    DUTIL(zv, v, arg, sizeof(struct asdcvoie));

  /* Ce qu'il faut faire :

          1 - Controle existence abonne
          2 - inhibition abonne (dans SWTPTR)
          3 - Pour toutes les CC :
                  - Si connexion CEVT :
                          - envoi message "suppression abonne"
                          - deconnexion CEVT
          4 - Liberation du tampon des commandes codees
          5 - Pour toutes les autres voies existantes :
                  - Envoi signal "reset" sur semaphore associe a la voie
                  - Si connexion CEVT :
                          - envoi message "suppression abonne"
                          - deconnexion CEVT
                  - Si voie inhibee, restauration de la chaine des tampons
                  - Si voie liee, suppression du lien
                  - Mise a 0 du pointeur des tampons
                  - Mise a zero de la memoire image associee
                  - Liberation des tampons
          6 - Liberation de la table des sous-adresses
          7 - NE PAS LIBERER la table des filtres (sert a l'espion)
          8 - Vidage des FIFO des voies en emission (si elles existent)
          9 - Autres tables a liberer ???
  */


    /* Controle de la validite des parametres :                 */
    /*   - Pour le moment, pas de controle a proprement parler, */
    /*  on se contente de supprimer tous les bits inutilises    */
    v->bus &= 0x01;
    v->adresse &= 0x1F;
    
    /* RT programme en "espion TR" ? */
    espion_tr = L(L(PROPTR) + v->adresse + 32*v->bus) & 2;

    /* Inhibition du RT via la table des status */
    E(L(SWTPTR) + v->adresse + 64 * v->bus, 0);

    /* Table des sous-adresses */
    j = L(L(ATPTR) + v->adresse + 32*v->bus);

    /* Table des filtres */
    k = L(L(FTPTR) + v->adresse + 32*v->bus);

    if ((j == 0) || (k == 0))  /* Table inexistante ==> pas d'abonne */
      { RETURN(EADDRNOTAVAIL);
      }

#ifdef CEVT
    /* Squelette d'un mot de commande CC */
    cmd = ((v->adresse << 11) & 0xF800) | 0x3E0;

    /* Parcours des commandes codees pour signaler la suppression */
    /* de l'abonne aux eventuels CEVT connectes aux CC            */
    for (i=0; i < (COCO_MAX + 1); i++)
        {
          /* Un CEVT est-il connecte ? */
          mm = dst->cocor[v->bus][v->adresse][i].cevt;
          if (mm)
            {
              /* si oui, signaler la RAZ (si possible) */
              if (cevt_signaler != NULL)
                        (*(cevt_signaler))
                            (mm, CEVT_SDCABOSUPPR,
                              dst->signal_number, v->bus,
                              cmd
                              | (((asdc_coco[i].def & 1) << 10) & 0x400)
                              | (i & 0x1F),
                              0);

              /* puis effectuer la deconnexion */
              dst->cocor[v->bus][v->adresse][i].cevt = 0;
            }
        }
#endif

    /* Liberation du tampon des commandes codees (commun aux SA 0 et 31) */
    if (L(j) != L(j+31)) {
        printk("ASDC %d : Liberation RT%d sur bus %d : \n"
               " ERREUR tampons differents pour CC(SA=0) et CC(SA=31) :"
               " 0x%X et 0x%X\n",
               asdcNum, v->adresse, v->bus, L(j), L(j+1));
       /* Dans le doute, on ne libere pas ces tampons ... */
    } else {
        if (asdclibere(dst, L(j), 34) == -1) {
            printk("ASDC %d : Liberation RT%d sur bus %d : \n"
                   " ERREUR liberation tampon CC : 0x%X\n",
                   asdcNum, v->adresse, v->bus, L(j));
        }
        E(j, 0);
        E(j + 31, 0);
    }

    /* Parcours des sous-adresses    *** Sauf 0 et 31 (CC) *** */
    for (i=1; i<31; i++)
        {
          /*****************************/
          /* Sous-adresse en reception */
          /*****************************/

          /* La voie existe-t-elle ?                          */
          /* L'image de la table des filtres doit etre testee */
          /* au meme titre que le pointeur de tampons         */
          tampapp = LI(k + i, 0);
          if (tampapp == 0) tampapp = L(j + i);
          if (tampapp)
            { /* La voie existe !                            */
              /* et tampapp pointe la chaine des tampons que */
              /* la voie soit inhibee ou non.                */

              /* Inhibition ITs */
              E(k + i, L(k + i) & ~0x7A);

              zd = LI(j + i, 0);

              /* Si adresse zd anormale, on abandonne ... */
              if ((zd < DEBUT_RAM) || (zd >= FIN_RAM)) RETURN(EDOM);


              /* Envoi signal RESET sur semaphore associe a la voie  */
              /* (Reveil eventuelle tache en attente sur cette voie) */
              // cprintf("   sreset RT%d,%d %c",
              //              i, j % 2, (j>31) ? 'T' : 'C');

              // cprintf("   IMA[0x%04X] = 0x%08X\n",
              //                     zd + IRSEM, LI(zd + IRSEM, 6));
              /* Si aucun SWAIT_IM n'a encore ete effectue  */
              /* sur la voie, le pointeur de wait-queue est */
              /* nul. Il faut alors ne pas l'utiliser !     */
              /* (sous peine de segmentation fault !)       */
              if (LI(zd + IRSEM, 6) != 0)
                { SRESET_IM(&LI(zd + IRSEM, 6));
                }



#ifdef CEVT
              /* On n'utilise pas le mutex habituel pour proteger */
              /* le code ci-dessous car il semble deraisonnable   */
              /* qu'une tache tente de connecter une voie a un    */
              /* CEVT pendant qu'une autre tache supprime cette   */
              /* voie : un minimum de synchronisation doit etre   */
              /* fourni par les applications elles-memes !        */

              /* La voie est-elle connectee a un CEVT ? */
              mm = LI(zd + IRCEVT, 0);
              if (mm)
                {
                  /* On signale, si possible, la deconnexion au CEVT */
                  if (cevt_signaler != NULL)
                      (*(cevt_signaler)) (mm, CEVT_SDCABOSUPPR,
                                          dst->signal_number, v->bus,
                                          ((v->adresse << 11) & 0xF800)
                                          | ((i << 5) & 0x3E0),
                                          0);

                  /* Deconnexion de la voie */
                  EI(zd + IRCEVT, 0, 324);
                }
#endif

              /* La voie est-elle liee ?                   */
              /* Si c'est le cas, le mot de commande place */
              /* dans la zone zd est celui d'une voie en   */
              /* emission (bien que la voie courante soit  */
              /* en reception)                             */
              vliee = LI(zd + IRCMD, 0) & 0x400;

              /* Deconnexion voie courante de */
              /* la chaine des tampons        */
              EI(k + i, 0, 0);
              E(j + i, 0);
              usec_sleep(1);
              E(j + i, 0);


              /* Si la voie est liee, on laisse a la voie en */
              /* emission le soin de liberer le tampon       */
              if (!vliee)
                {
                  /* Pour une voie en reception, la structure */
                  /* des tampons ne depend pas du mode de la  */
                  /* voie                                     */
                  if (liberer_tamp(dst, tampapp))
                                              RETURN(EADDRNOTAVAIL);

                  /* Mise a zero memoire image */
                  for (iz=0; iz<34; iz++)
                    { EI(zd + iz, 0, 0);

                      /*** ATTENTION ***   #########################
                        Sous Linux, il faudrait sans doute   #######
                        liberer ici la wait_queue eventuelle ... ###
                      ##############################################
                      TODO
                      */

                    }

                  /* RAZ du pointeur de zd */
                  EI(j + i, 0, 0);
                }

            }


          /****************************/
          /* Sous-adresse en emission */
          /****************************/

          /* La voie existe-t-elle ?                          */
          /* L'image de la table des filtres doit etre testee */
          /* au meme titre que le pointeur de tampons         */
          tampapp = LI(k + i + 32, 0);
          if (tampapp == 0) tampapp = L(j + i + 32);
          if (tampapp)
            { /* La voie existe !                            */
              /* et tampapp pointe la chaine des tampons que */
              /* la voie soit inhibee ou non.                */

              /* Inhibition ITs */
              E(k + i + 32, L(k + i + 32) & ~0x7A);

              zd = LI(j + i + 32, 0);

              /* Si adresse zd anormale, on abandonne ... */
              if ((zd < DEBUT_RAM) || (zd >= FIN_RAM)) RETURN(EDOM);


              /* Envoi signal RESET sur semaphore associe a la voie  */
              /* (Reveil eventuelle tache en attente sur cette voie) */
              // cprintf("   sreset RT%d,%d %c",
              //              i, j % 2, (j>31) ? 'T' : 'C');
#ifdef LINUX
              // cprintf("   IMA[0x%04X] = 0x%08X\n",
              //                     zd + IRSEM, LI(zd + IRSEM, 6));
              /* Si aucun SWAIT_IM n'a encore ete effectue  */
              /* sur la voie, le pointeur de wait-queue est */
              /* nul. Il faut alors ne pas l'utiliser !     */
              /* (sous peine de segmentation fault !)       */
              if (LI(zd + IRSEM, 6) != 0)
                { SRESET_IM(&LI(zd + IRSEM, 6));
                }
#else
              // cprintf("\n");
              SRESET_IM(&LI(zd + IRSEM, 6));
#endif   /* LINUX */


#ifdef CEVT
              /* On n'utilise pas le mutex habituel pour proteger */
              /* le code ci-dessous car il semble deraisonnable   */
              /* qu'une tache tente de connecter une voie a un    */
              /* CEVT pendant qu'une autre tache supprime cette   */
              /* voie : un minimum de synchronisation doit etre   */
              /* fourni par les applications elles-memes !        */

              /* La voie est-elle connectee a un CEVT ? */
              mm = LI(zd + IRCEVT, 0);
              if (mm)
                {
                  /* On signale, si possible, la deconnexion au CEVT */
                  if (cevt_signaler != NULL)
                      (*(cevt_signaler)) (mm, CEVT_SDCABOSUPPR,
                                          dst->signal_number, v->bus,
                                          ((v->adresse << 11) & 0xF800)
                                          | 0x400 | ((i << 5) & 0x3E0),
                                          0);

                  /* Deconnexion de la voie */
                  EI(zd + IRCEVT, 0, 324);
                }
#endif


              /* Si la voie etait liee, le retrait du lien */
              /* a ete fait au cours du traitement de la   */
              /* voie en reception associee                */

              /* Deconnexion voie courante de */
              /* la chaine des tampons        */
              EI(k + i + 32, 0, 0);
              E(j + i + 32, 0);
              usec_sleep(1);
              E(j + i + 32, 0);


              /* Pour une voie en emission, la structure des */
              /* tampons depend du mode de la voie           */
              switch (LI(zd + IRMODE, 0))
                {
                  case RT_VSYNC :
                  case RT_VASYNC :
                      if (liberer_tamp(dst, tampapp))
                                              RETURN(EADDRNOTAVAIL);
                      break;

                  case RT_VSTAT :
                      /* Tampons separes ? */
                      /*   --> Ils sont quand meme lies par IRTPREC */
                      if (liberer_tamp(dst, tampapp))
                                              RETURN(EADDRNOTAVAIL);
                      break;

                  case RT_VSYNC2 :
                      if (liberer_tamp(dst, LI(zd + IRTCHINU, 0)))
                                              RETURN(EADDRNOTAVAIL);
                      if (liberer_tamp(dst, LI(zd + IRTFCH1, 0)))
                                              RETURN(EADDRNOTAVAIL);
                      if (liberer_tamp(dst, LI(zd + IRTFCH2, 0)))
                                              RETURN(EADDRNOTAVAIL);
                      break;

                  default : RETURN(EBADF);
                }


              /* Remise a zero memoire image */
              for (iz=0; iz<34; iz++)
                { EI(zd + iz, 0, 0);

                      /*** ATTENTION ***
                        Sous Linux, il faudrait sans doute
                        liberer ici la wait_queue eventuelle ...
                      */

                }

              /* RAZ du pointeur de zd */
              EI(j + i, 0, 0);

            }
        }



  /* Liberation de la table des sous-adresses */
  if (asdclibere(dst, j, 128) == -1) RETURN(EADDRNOTAVAIL);
  E(L(ATPTR) + v->adresse + 32*v->bus, 0);

  /* On ne doit pas liberer la table des filtres !!! */
  /* (sert a l'espion sequentiel)                    */
  /* if (asdclibere(dst, k, 64) == -1) RETURN(EADDRNOTAVAIL); */
  /* E(L(FTPTR) + v->adresse, 0); */

  /* Mais, par securite, on remet a zero */
  /* la memoire image correspondante     */
  for (iz=0; iz<64; iz++) EI(k+iz, 0, 0);
  
  
  /* Et on vide les FIFOS associees aux voies en emission */
  for (i=0; i<32; i++) {
        fifoTampons_t * pfifo;
        pfifo = &(dst->pTampEcr[v->bus][v->adresse][i]);
        viderCompletementFifo(pfifo);
  }


  /* Fin normale de ASDC_ABO_LIBERER */
  RETURN(OK);
}




/* Liberation d'une chaine de tampons abonne (bouclee ou non)      */
/*     dst : pointeur vers le contexte general                     */
/*     ptamp : pointeur vers le premier tampon de la chaine        */
/*                                                                 */
/*  ATTENTION : Utilise le chainage inverse (en memoire d'echange) */
/*              pour etre compatible du plus grand nombre possible */
/*              de chaines                                         */
/*                                                                 */
static int liberer_tamp(struct asdc_varg *dst, unsigned short ptamp)
{
  int ptamp0;
  int ptampini;
  int premiere_fois;

//  /* Chainage direct : non utilise ici */
//  while ((ptamp != RT_FIN2) && (ptamp != 0))
//       {
//         ptamp0 = L(ptamp);
//         E(ptamp, RT_FIN2, 0);  /* Pour briser la boucle */
//         if (asdclibere(dst, ptamp, 34)) return -1;
//         ptamp = ptamp0;
//       }


  /* Chainage inverse */
  ptampini = ptamp;
  premiere_fois = 1;
  while ((ptamp != RT_FIN2) && (ptamp != 0))
       {
         /* Detection de la boucle */
         if (!premiere_fois && (ptamp == ptampini)) break;

         ptamp0 = LI(ptamp + IRTPREC, 0);
         if (asdclibere(dst, ptamp, 34)) return -1;
         ptamp = ptamp0;
         premiere_fois = 0;
       }

  return 0;
}



