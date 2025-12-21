
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

ASDCDEF : Fonction ioctl de creation d'une sous-adresse.
          Si l'adresse n'existe pas, elle est egalement creee.

 QUAND      QUI   QUOI
---------- ----  ------------------------------------------------
 9/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
 2/05/2013  YG   Ajout flag "jamais_initialise".
 3/06/2013  YG   Ajout de la troisieme option "arg = 7"
13/06/2014  YG   Rassemblement des includes dans un seul fichier
11/02/2015  YG   Remplacement L() et E() par LX() et EX()
26/02/2015  YG   Le partage materiel du port droit de la SRAM est maintenant
                 fonctionnel. Le bon fonctionnement du powerPC n'etant plus
                 necessaire pour acceder a la RAM depuis le bus PCI, les macros
                 LX() et EX() peuvent de nouveau etre remplacees par L() et E().
*/


    /* Interruption de toutes les taches en attente               */
    /*                                                            */
    /* Le premier appel de la fonction - avec arg=1 - leve le     */
    /* fanion raz et fait un sreset sur tous les semaphores       */
    /*                                                            */
    /* Le second appel de la fonction - avec arg=2 - rabaisse le  */
    /* fanion raz                                                 */
    /*                                                            */
    /* Pour faire une RAZ de la carte, l'application doit :       */
    /*     - 1) appeler ASDCAVORTE avec arg=1                     */
    /*     - 2) attendre (sleep) un temps suffisant pour donner   */
    /*          a toutes les taches reveillees le temps de se     */
    /*          terminer                                          */
    /*     - 3) appeler ASDCAVORTE avec arg=2                     */
    /*     - 4) appeler ABI_FIRMWARE_INIT                         */
    /*     - 5) appeler ASDCRAZ, etc....                          */
    /*                                                            */
    /*                                                            */
    /* Certains utilitaires (en particulier "testemuabi") peuvent */ 
    /* laisser des donnees aleatoires dans la memoire d'echange.  */
    /* Si l'appel de ces utilitaires fait suite a une premiere    */
    /* execution de "einitabi", le flag "jamais_initialise" est a */
    /* 0 mais la memoire n'est pas "propre". Le prochain appel de */
    /* "einitabi" peut alors entrainer un crash du systeme.       */
    /* Pour eviter ce probleme, les utilitaires en question       */
    /* doivent avoir la possibilite de remettre a 1 le flag       */
    /* "jamais_initialise".                                       */
    /* Cette remise a zero peut etre obtenue en appelant (une     */
    /* seule fois) ASDCAVORTE avec arg=77. (La valeur 77 a ete    */
    /* choisie pour ne pas risquer de remettre le flag a 1 par    */
    /* erreur).                                                   */
    

#include "driverIncludes.h"

#ifdef CEVT
#include "../CEVT/cevtctl.h"
#endif   /* CEVT */


/// TODO : Seuls arguments sans doute necessaires : dst et arg !!!

long
do_ASDCAVORTE(int vfxNum, int asdcNum, struct asdc_varg *dst, unsigned long arg)
{

    int i, j, l, m;
    unsigned int zd;

#ifdef CEVT
    int k, mm;
#endif

    /* printk("ASDCAVORTE arg=%ld jamais_init=%d\n", arg, dst->jamais_initialise); */
    
    /* L'execution de cette fonction sur un driver+coupleur non initialise */
    /* se traduit generalement par un crash.                               */
    if (dst->jamais_initialise) RETURN(OK);

    switch ((int) arg)
      { case 1 : dst->raz = 1;
                 kkprintf("RAZ ASDC en cours ...\n");
                 kkprintf("   ==> Reset de tous les semaphores\n");

#ifdef POUR_LE_MOMENT_MODE_BC_NON_IMPLEMENTE
                 /* Reveil de toutes les attentes sur flux BC */
                 /* (En parcourant la chaine des flux)        */
                 /*    -> L'utilisation de i est destinee a   */
                 /*       sortir du piege d'un chainage       */
                 /*       reboucle                            */
                 /*    -> L'hypothese d'un nombre de flux BC  */
                 /*       superieur a 100 n'est pas serieuse  */
                 for (i=0, j=dst->pflux;
                      ((j >= DEBUT_RAM) && (j < FIN_RAM) && (i < 100));
                      i++, j=LIH(j+IMFSD, 3))
                    { cprintf("   flux BC sreset : j=%d (0x%0X)\n", j, j);
                      SRESET_IM(&LI(j+IMSEMFLX, 4));
                    }

                 /* Reveil de toutes les attentes trame */
                 /// SRESET(&dst->sem_finbc);
                 /// SRESET(&dst->sem_gotbc);

                 /* Reveil de toutes les attentes execution bloc BC */
                 /// SRESET(&dst->sem_exbc);
#endif  /* POUR_LE_MOMENT_MODE_BC_NON_IMPLEMENTE */

                 /* Reveil de toutes les attentes sur abonnes */
                 /* Balayage des adresses :                     */
                 /* On balaye d'un seul coup les 32 adresses du */
                 /* bus 0 et les 32 adresses du bus 1.          */
                 for (i=0; i<64; i++) /* Balayage adresses RT */
                   {
                     /* Table des sous-adresses */
                     l = L(L(ATPTR) + i);
                     /* L'abonne est-il defini ? */
                     if (l == 0) continue;

                     /* Balayage des sous-adresses :                     */
                     /* On balaye d'un seul coup les 32 sous-adresses en */
                     /* reception et les 32 sous-adresses en emission.   */
                     for (j=0; j<64; j++)
                       {
                         /* Pointeur premier tampon */
                         m = L(l + j);
                         /* La voie est-elle definie ? */
                         if (m == 0) continue;

                         zd = LI(l + j, 5);
                         /* Si adresse zd anormale, on abandonne ... */
                         if ((zd < DEBUT_RAM) || (zd >= FIN_RAM))
                                                            continue;

                         /* Reveil eventuelle tache en attente */
                         /* sur cette voie                     */
                         cprintf("   sreset B%d RT%d,%d %c",
                                 (i>31) ? 1 : 0, i % 32, j % 32,
                                  (j>31) ? 'T' : 'C');
#ifdef __LP64__
                         cprintf("   IMA[0x%04X] = 0x%16lX\n",
                                          zd + IRSEM, LI(zd + IRSEM, 6));
#else  /* __LP64__ */
                         cprintf("   IMA[0x%04X] = 0x%08lX\n",
                                          zd + IRSEM, LI(zd + IRSEM, 6));
#endif /* __LP64__ */
                         /* Si aucun SWAIT_IM n'a encore ete effectue  */
                         /* sur la voie, le pointeur de wait-queue est */
                         /* nul. Il faut alors ne pas l'utiliser !     */
                         /* (sous peine de segmentation fault !)       */
                         if (LI(zd + IRSEM, 6) != 0)
                           { SRESET_IM(&LI(zd + IRSEM, 6));
                           }

#ifdef CEVT
                         /* La voie est-elle connectee a un CEVT ? */
                         mm = LI(zd + IRCEVT, 7);
                         if (mm && (cevt_signaler != NULL))
                           {
                             (*(cevt_signaler)) (mm, CEVT_SDCABO,
                                                 dst->signal_number, 0,
                                                 CEVT_AVORTE, 0);
                           }
#endif

                       }
                   }

                /* printk("ASDCAVORTE : Avant balayage de tous les CEVTs\n"); */


#ifdef CEVT
                /* Reveil de tous les CEVT en attente de CC */

                /* Balayage bus */
                for (i=0; i<2; i++) {
                    /* Balayage adresses RT */
                    for (j=0; j<32; j++) {
                        /* Balayage des commandes codees */
                        for (k=0; k < (COCO_MAX + 1); k++) {
                            /* Un CEVT est-il connecte ? */
                            mm = dst->cocor[i][j][k].cevt;
                            if (mm) {
                                /* si oui, signaler la RAZ */
                                (*(cevt_signaler)) (mm, CEVT_SDCABO,
                                                    dst->signal_number, 0,
                                                    CEVT_AVORTE, 0);
                            }
                        }
                    }
                }
#endif


                 /* Reveil de toutes les attentes sur espion seq. */
                 /// SRESET(&dst->semmon);

                 /* Reveil de toutes les attentes sur horloge IRIG */
                 /// SRESET(&dst->semirig);

                 /* printk("ASDCAVORTE : FIN DE LA PHASE 1\n"); */
                 break;

        case 2 : dst->raz = 0;
                 /* printk("ASDCAVORTE :  PHASE 2\n"); */
                 break;


        case 77 : dst->jamais_initialise = 1;
                  /* printk("ASDCAVORTE :  PHASE 77\n"); */
                  break;

        default : RETURN(EINVAL);
       }

    RETURN(OK);
}

