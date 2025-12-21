
/*

 DRIVER LINUX POUR LE COUPLEUR EMUABI

isr_asdc.c : Fonction de traitement des interruptions du coupleur EMUSCI

 QUAND      QUI   QUOI
---------- ----  --------------------------------------------------------------
10/04/2013  YG   Version initiale (extraite du driver ASDC v4.22).
 3/06/2013  YG   Suppression d'un acces a un registre CSR obsolete.
13/06/2014  YG   - Rassemblement des includes dans un seul fichier
                 - Traitement de la memorisation du dernier tampon emis
                   dans le cas d'une sous-adrese en emission.
13/04/2015  YG   Correction d'un crash si un message AG est recu par une
                 sous-adresse pour laquelle aucun tampon n'a ete rempli
                 (pointeur nul dans la structure pTampEcr[][][]).
*/


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	
#endif


/* Le symbole ci-dessous est utilisee pour distinguer les appels des     */
/* macros E() et L() dans et hors d'une fonction d'interruption.         */
/*    ==> Il faut imperativement avoir defini ce symbole AVANT d'inclure */
/*        les fichiers .h locaux (sinon, risque de hangup).              */
#define FONCTION_INTERRUPTION

#include "driverIncludes.h"
#include "rdtsc.h"
#include "interface_vfx70.h"


#ifdef CEVT
#include "../CEVT/cevtctl.h"
#endif   /* CEVT */

struct file;
struct inode;
struct pt_regs;


// #define zzprintf printk
#define zzprintf bidon

// #define dprintf printk
#define dprintf bidon

/* Macros pour instrumentation eventuelle du code */
#define L2(a)            asdc_lecture_brute(dst->pcibar2, (a))
#define E2(a, v)         asdc_ecriture_brute(dst->pcibar2, (a), (v))

/* Pour debug a l'oscilloscope - x pouvant varier de 0 a 7 */
#define tag(x) ecriturePCIe(asdc_p[unit]->numVfx, 1, 0xF000 | (((~(x)) << 1) & 0xE), (int *)(pvargs[asdc_p[unit]->numVfx]->base2 + (0x9094)))


/*
 * Cette fonction est appelee par le driver de la carte VFX70 quand
 * une interruption recue par cette derniere concerne le coupleur EMUSCI.
 * Le parametre unit est le numero du coupleur ASDC.
 *
 */

void
isr_asdc(int unit)
{

    int cnt, icnt, i, tmp;
    unsigned int ptr;
    int itbc;	/* Indicateur arrivee ITs du mode BC */

    unsigned long long tsc;

    struct asdc_varg *dst;
    dst = asdc_p[unit];


   // E(FIN_RAM - 1, 0x5A5A);   /* Marqueur debug pour VMETRO */

   dst->dit = 1;
   dst->dphit = 1;

   // kkprintf("{%d", dst->signal_number);
   // printk("--- IT ASDC (unit=%d) ---\n", unit);

   zzprintf("EMUABI-ASDC-IT : U=%d SN=%d\n"
            "                 IQRSP=0x%04X  ATPTR=0x%04X\n",
            unit, dst->signal_number, L(IQRSP), L(ATPTR));


   /* Le coupleur courant est-il bien a l'origine de l'IT ? */
   /* Cette verification doit avoir ete deja faite par le   */
   /* driver de la carte VFX70.                             */

   /* Attente du passage a zero de IQRSP */
   /* [Remarque : Ci-dessous, (tmp=L(IQRSP)) est bien un assignement, */
   /*             pas une comparaison !]                              */
   for (i=0; (tmp=L(IQRSP)); i++)
      { if (i>10)
          { printk("ASDC : IT et IQRSP=0x%04X\n", tmp);
            printk("SORTIE ANTICIPE de la fonction d'IT pour IQRSP!=0\n");
            return;
          }
        usec_sleep(1);         /*** A Supprimer si possible ... ***/
      }



   /* Bloquage de la queue des comptes rendus d'IT */
   E(IQRSP, 0xFFFF);

   usec_sleep(2);                 /***####### Toujours OK ??? #######***/

   /* Position de la queue */
   cnt = L(IQCNT1);
   ptr = L(IQPTR1);


   if (cnt==0) { /* L'IT ne provenait pas de l'EMUABI ! */
                 /// printk("ASDC : IT recue et IQCNT1=0 ==> ???\n");

                 /* Debloquage de la queue */
                 E(IQRSP, 1);

                 // kkprintf("]");
                 dst->dphit = -1;
                 return;
               }

    /* Instrumentation pour mesures perfos : montee d'une voie TOR */
    /* Mise a un de la sortie TOR SSYNC3                        */
    /* E2(0x9094, L2(0x9094) | 0x00000004);                     */

   if (cnt < 0) { printk("ASDC : compte d'ITs = %d < 0 !!!\n", cnt);
                  /* Au point ou on en est, on continue ... */

                  /*###########################################
                     Ici, il faudrait positionner un flag pour
                     indiquer le probleme rencontre, puis
                     reveiller toutes les applications en
                     attente semaphores noyaux.
                     ==> Pour cela, il faudrait fonction adaptee,
                     qui pourrait etre appelee la ou on a besoin
                     d'elle (par exemple dans ioctl(ASDCAVORTE))...
                    ###########################################*/
                }

   /* Mise a jour de l'histogramme des nombres d'ITs dans la table */
   /* (pour permettre le reglage de la taille de la table des ITs) */
   dst->nbhistoit++;
   if (cnt >= ASDCTHISTOIT) dst->deborde++;
   else dst->histoit[cnt]++;

   itbc = 0;

   for (icnt=0; icnt<cnt; icnt++)
      {  int bus, cmd, commande, adresse, sous_adresse, direction;
         long *oreiller;
         unsigned int aadr, zd;
         unsigned int flx;
         int memofiltre, afiltre;
         int liste, erreur;
         int espion_tr;

         /* zzprintf("QIT(%d) : 0x%X 0x%X 0x%X 0x%X\n",
                 dst->signal_number, L(ptr),  L(ptr+1), L(ptr+2), L(ptr+3)); */
         /* printk("QIT(%d) : 0x%X 0x%X 0x%X 0x%X\n",
                 dst->signal_number, L(ptr),  L(ptr+1), L(ptr+2), L(ptr+3)); */
         dst->dit = L(ptr); /* Code d'interruption : */
                            /* mot1 des 4 mots pointes par IQPTR1 */
         switch(L(ptr))
            {  case 0x1 :  zzprintf("ASDC : Timer Data Ready\n");
//                // E(FIN_RAM - 1, 0x8888);   /* Marqueur debug pour VMETRO */
//                          /* On reveille la (les ???) tache(s) en attente */
//                          SRESET_OLD(&dst->semirig);
                         break;

               case 0x2 :  zzprintf("ASDC : Monitor Buffer Swap\n");
//                          /* On indique qu'un buffer est disponible */
//                          dst->monok = 1;
//                          /* On reveille tache associee a espion sequentiel */
//                          /* qui est censee dormir sur semmon               */
//                          SRESET_OLD(&dst->semmon);
                         break;

               case 0x3 :  /* Message received */
                         zzprintf("ASDC : Filter Table Bit Set\n");
                         tag(2);
 // printk("tag(2)\n");

                         /* Instrumentation pour mise en evidence instant */
                         /* IT relativement au message 1553 :             */
                         /*  ==> Incrementation du mot 0 de la mem. image */
                         EI(0, LI(0, 0) + 1, 0);

                          /* Ici, il faut rechercher l'adresse du RT(A,SA)  */
                          /* dans l'ABI et reveiller les taches qui dorment */
                          /* dessus                                         */
                         if(L(ptr+2)) KPRINTF("ATTENTION : word2 = 0x%x\n",
                                              L(ptr+2));
                             /* Devrait entrainer un traitement de la     */
                             /* 2eme cmd identique a celui de la premiere */

                         cmd = commande = L(ptr+1);
                         bus = L(ptr+3) & 1;
                         commande >>= 5;
                         sous_adresse = commande & 0x1F;
                         commande >>= 5;
                         direction = commande & 1;     /* 1 ==> T ;  0 ==> R */
                         commande >>= 1;
                         adresse = commande & 0x1F;

                         /* Dans ce qui suit, on ne fait aucun test.    */
                         /* On suppose que si une IT s'est produite les */
                         /* structures de donnees ABI existent.         */

                         aadr = L(L(ATPTR) + adresse + bus * 32);
                         zd = LI(aadr + sous_adresse + direction * 32, 200);
                         oreiller = &LI(zd + IRSEM, 201);

                         dprintf("zd=0x%X   oreiller : 0x%X 0x%0X\n",
                                   zd, zd+IRSEM, oreiller);

                         /* On fait le meme traitement (un reveil) pour */
                         /* une emission comme pour une reception !     */

                         dprintf(
                              "wake up : oreiller(0x%0X) a=%d sa=%d  sens=%d\n",
                               oreiller, adresse, sous_adresse, direction);

                         /* Reveil de la tache endormie sur l'adresse du */
                         /* pointeur du (des) tampon(s) de donnees       */
                         SRESET_IM(oreiller);

                         /* Traitement eventuel des flux d'evenements RT */
                         /* (c'est a dire des connexions CEVT)           */
#ifdef CEVT
                         /* Une connexions CEVT est-elle liee a la voie ? */
                         flx = LI(zd + IRCEVT, 202);
                         if (flx && (cevt_signaler_date != NULL))
                           { /* Oui ! */

                             /* Lecture du TSC */
                             tsc = liretsc();

                             /* ==> Transmission evenement au flux concerne */
                             (*(cevt_signaler_date))(flx, CEVT_SDCABO,
                                                dst->signal_number, bus,
                                                cmd & 0xFFFF, 0, tsc);
                           }
#endif	/* CEVT	*/

                         /* Si la sous-adresse est en emission et n'est */
                         /* pas en mode espion temps reel, mise a jour  */
                         /* du tampon soft.                             */
                         espion_tr = L(L(PROPTR) + adresse + 32 * bus) & 2;
                         if (direction && !espion_tr) {
                             fifoTampons_t * pfifo;
                             tampon_t *pt;
                             
                             pfifo = &(dst->pTampEcr[bus][adresse]
                                                       [sous_adresse]);
                             
                             spin_lock(&pfifo->lock);
                                /* Retour au pool du plus ancien tampon */
                                /* de la FIFO associee a la voie.       */
                                /* Fonctionne en mode SYNC et ASYNC     */
                                pt = decalerFifo_unlocked(pfifo);
                                
                                /* Ecriture du mot de commande dans le */
                                /* nouveau "dernier tampon emis" s'il  */
                                /* existe.                             */
                                if (pfifo->demis) pfifo->demis->cargo[1] = cmd;
                             spin_unlock(&pfifo->lock);
                             
                              /* printk("isr_asdc, avant rendre(pt), pt=%p\n", pt); */
                             rendre(pt);
                        }

                        /* Des erreurs transitoires (multishot errors) */
                         /* ont elle ete demandees ?                    */
                         memofiltre = LI((zd+IRMEMOF), 0);
                            /* Si aucune erreur transitoire en cours : */
                            /* fin du traitement de l'IT !             */
                         if (!(memofiltre & 0x80000000)) break;

                            /* Dans le cas contraire, le traitement */
                            /* est effectue ci-dessous.             */

                         liste = LI((zd+IRTLET), 0);
                         afiltre = LI((zd+IRAFILTRE), 0);

                         /* Erreur demandee ou non, arrivee ou non ? */
                         erreur = liste & 1;
                         if (erreur)
                           {
                             /* Une erreur etait demandee */
                             int msg_err;

                             /* Une erreur de parite doit s'etre produite */
                             int nb_err = L(ERRPAR_NB) & 0x7FFF;
                             int nb_err0 = (LI(zd+IRMEMOF, 0) >> 16) & 0x7FFF;
                             /* si non : fin du traitement ! */
                             if (nb_err == nb_err0) break;

                             /* Derniere err. par. doit concerner cette voie */
                             msg_err = L(ERRPAR_DC);
                             /* si non : fin du traitement ! */
                             if (msg_err != cmd) break;
                           }

                         /* Decalage de la liste */
                         liste = (liste >> 1) & 0x7FFFFFFF;
                         EI((zd+IRTLET), liste, 0);

                         /* Preparation prochain message :            */
                         /* (inutile de modifier le filtre si le mode */
                         /*  erreur ou OK courant est conserve)       */

                         /* Passage de OK a erreur ? */
                         if ((!erreur) & (liste & 1))
                           { /* Programmation erreur sur prochain msg */
                             /* (on force a 1 le bit IT au cas ou...) */
                             E(afiltre, (LI((zd+IRMEMOE), 0) | 2) & 0xFFFF);
                           }
                         /* Passage d'erreur a OK ? */
                         if (erreur & !(liste & 1))
                           { /* Suppression erreur sur prochain msg   */
                             /* (on force a 1 le bit IT au cas ou...) */
                             E(afiltre, (LI(zd+IRMEMOF, 0) | 2) & 0xFFFF);
                           }

                         /* Mise a jour de memofiltre si erreur */
                         if (liste & 1)
                           {
                             int nb_err = L(ERRPAR_NB);
                             memofiltre =   (memofiltre & 0xFFFF)
                                          | 0x80000000
                                          | ((nb_err << 16) & 0x7FFF0000);
                             EI((zd+IRMEMOF), memofiltre, 0);
                           }

                         /* Si erreurs restent dans la sequence, */
                         /* fin du traitement.                   */
                         if (liste) break;

                         /* Sinon, la sequence est terminee et le */
                         /* flag "erreur multishot" peut etre     */
                         /* supprime                              */
                         EI((zd+IRMEMOF), 0, 0);

                         break;  /* Fin du "case 0x3" */

               case 0x4 :  KPRINTF("ASDC : Reserved\n");
                         break;

//                case 0x5 :  /* RT response error (programmation BC) */
//                          {
//                            unsigned int bc, err;
// 
//                            bc = L(ptr+1);	/* Bloc BC concerne */
//                            err = L(ptr+2);	/* Code erreur */
// 
//                            /* Stockage du code erreur dans image[] */
//                            zzprintf("ASDC : BC Program Error\n");
//                            EIL(bc+IMERR, err, 500);
// 
//                            /* Traitement d'un eventuel tampon en attente */
//                            if (dst->tf_attente)
//                              {
//                                /* Si erreur concerne le bloc BC du tampon */
//                                /* en attente, memorisation erreur dans ce */
//                                /* tampon                                  */
//                                if (dst->tf_attente == bc)
//                                  { dst->tf_pta->tamp.err = err;
//                                  }
// 
//                                tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta,
//                                         dst->tf_flux, dst->tf_zd, &dst->pbcfl);
// 
//                                /* Si tmp, traitement de l'erreur */
//                                if (tmp)
//                                  { bcflux_err(dst, dst->tf_attente,
//                                           dst->tf_flux, dst->tf_zd, tmp, 0, 0);
//                                  }
// 
//                                /* Abaissement indicateur */
//                                dst->tf_attente = 0;
//                              }
// 
//                          }
//                          break;
// 
//                case 0x6 :  /* KPRINTF("ASDC : BC Program End\n"); */
//                          dst->fin_bc = 1;
// 
//                          /* Provisoire : Pour experimentation
//                            { // int s, ns;
//                              // ns = nanotime(&s);
//                              extern void heure(long int *phaut, long int *pbas);
//                              heure(&dst->dbg_s, &dst->dbg_ns);
//                              //dst->dbg_ns = ns;
//                              //dst->dbg_s = s;
//                            }
//                          */
// 
// 
// 
// 
//                          /* Traitement d'un eventuel tampon en attente */
//                          if (dst->tf_attente)
//                            {
//                              tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta,
//                                         dst->tf_flux, dst->tf_zd, &dst->pbcfl);
// 
//                              /* Si tmp, traitement de l'erreur */
//                              if (tmp)
//                                { bcflux_err(dst, dst->tf_attente,
//                                           dst->tf_flux, dst->tf_zd, tmp, 0, 0);
//                                }
// 
//                              /* Abaissement indicateur */
//                              dst->tf_attente = 0;
//                            }
// 
//                          /* Indication "Trame achevee" */
//                          EIH(dst->idtrame + IMERR, 1, 1000);
// 
// 
//                          /* Reveil tache qui attend fin de la trame */
//                          SRESET_OLD(&dst->sem_finbc);
// 
//                          /* Reveil des taches en attente sur des fluxs BC */
//                          /* (En parcourant la chaine des fluxs */
//                          for (j=dst->pflux; j; j=LIH(j+IMFSD, 203))
//                             { SRESET_IM(&LI(j+IMSEMFLX, 204));
//                             }
// 
//                          /* Faut-il enchainer une nouvelle trame ? */
//                          tmp = LIH(dst->idtrame + IMSUIV, 1000);
//                          EIH(dst->idtrame + IMSUIV, 0, 1000);
//                          if (tmp)
//                            { /* Oui */
//                              dst->idtrame = dst->descr_trame[tmp].idtrame;
//                              dst->fin_bc = 0;
//                              E(MFTVALH,
//                                  ((dst->descr_trame[tmp].periode >> 16)
//                                                                     & 0xFFFF));
//                              E(MFTVALL,
//                                  ((dst->descr_trame[tmp].periode) & 0xFFFF));
//                              E(MFPERFR, ((dst->descr_trame[tmp].minpmaj)
//                                                                     & 0xFFFF));
//                              E(MFTEXEH, ((dst->descr_trame[tmp].cycles >> 16)
//                                                                     & 0xFFFF));
//                              E(MFTEXEL, ((dst->descr_trame[tmp].cycles)
//                                                                     & 0xFFFF));
//                              EIH(tmp + IMERR, 0, 1000); /* exec. en cours */
//                              E(BCIPTR, dst->idtrame);   /* Lancement trame */
//                              SRESET_OLD(&dst->sem_gotbc);  /* Reveil taches ... */
//                            }
//                          else
//                            { /* Non */
//                              dst->idtrame = 0;
//                            }
// 
//                          /* Reveil tache qui lit les trames BC */
//                    /***      wakeup((caddr_t) &asdc_ntbc[asdcunit]); ***/
//                          break;
// 
//                case 0x7 :  zzprintf("ASDC : BC Program Masked Status\n");
//                          break;

               case 0x8 :  printk("ASDC : Interrupt Overflow\n");

//                          /* La trame est-elle terminee ? */
//                          /* (l'IT perdue pouvait indiquer une fin de trame) */
//                          if ((!L(BCCPTR)) && (L(BCLPTR)))
//                            { /* Indicateur "trame finie" */
//                              dst->fin_bc = 1;
// 
//                              /* Reveil tache qui attend fin de la trame */
//                              SRESET_OLD(&dst->sem_finbc);
//                            }
// 
//                          /* Parcours de la chaine des fluxs pour */
//                          /* remonter l'erreur (code 3)           */
//                          for (j=dst->pflux; j; j=LIH(j+IMFSD, 205))
//                             { int zd;
//                               zd = LI(j + IMPZD, 206);
//                               bcflux_err(dst, j, j, zd, 3, 0, 0);
// 
//                               /* Si trame terminee, reveil (eventuel) flux */
//                               if (dst->fin_bc == 1)
//                                            SRESET_IM(&LI(j+IMSEMFLX, 207));
//                             }

                         break;

               case 0x9 : zzprintf("ASDC : MIM Interrupt\n");
                         {
                           unsigned short cmd;
                           int code, rt, mot, cevt;

                           cmd = L(ptr+1);
                           bus = L(ptr+3) & 1;

                           code = cmd & 0x1F;
                           rt = (cmd >> 11) & 0x1F;

                           /* Traitement eventuel des flux d'evenements RT */
                           /* (c'est a dire des connexions CEVT)           */
#ifdef CEVT
                           /* Une connexions CEVT est-elle liee a la CC ? */
                           cevt = dst->cocor[bus][rt][code].cevt;
                           if (cevt && (cevt_signaler_date != NULL))
                             { /* Oui ! */

                               /* Recuperation du mot de donnee eventuel */
                               switch (code)
                                 { case  2 :	/* Transmit last status */
                                       mot = L(L(LSWPTR) + rt + 32 * bus);
                                       break;

                                   case 16 :	/* Transmit vector word */
                                       mot = L(L(TVWPTR) + rt + 32 * bus);
                                       break;

                                   case 17 :	/* Synchronize (with data) */
                                       mot = L(L(LSYPTR) + rt + 32 * bus);
                                       break;

                                   case 18 :	/* Transmit last command */
                                       mot = L(L(LCDPTR) + rt + 32 * bus);
                                       break;

                                   case 19 :	/* Transmit Built In Test */
                                       mot = L(L(BITPTR) + rt + 64 * bus);
                                       break;

                                   /// For M51 only
                                   // case 22 :	/* RESERVE */
                                   //    mot = L(L(RESPTR) + rt + 32 * bus);
                                   //    break;

                                   default :
                                       mot = 0;
                                       break;
                                 }
                               /* Lecture du TSC */
                               tsc = liretsc();

                               /* ==> Transmission evenement au cevt concerne */
                    /// PENSER A AJOUTER LE BUS !!!
                               (*(cevt_signaler_date))(cevt, CEVT_SDCABO,
                                                  dst->signal_number, bus,
                                                  cmd & 0xFFFF, mot & 0xFFFF,
                                                  tsc);
                             }
#endif	/* CEVT */

                         }
                         break;

               case 0x10 : /* Sequential monitor swap on message */
                         zzprintf("ASDC : Monitor Buffer Swap on Message\n");
                         break;

//                case 0x11 : /* BC bloc complete */
//                          { unsigned int bc, flux;
//                            unsigned int zd, ad;
//                            int nbmte, nbcte, nblte;
//                            int nbmts, nbcts, nblts;
//                            unsigned int evt;
//                            struct asdcbc_tf *t;
//                            int cm, tb, n;
//                            int x; /* Memorisation temp. erreurs internes */
// 
//                            wait_queue_head_t * semexbc;
// 
// 
//                            /* Traitement d'un eventuel tampon en attente */
//                            if (dst->tf_attente)
//                              {
//                                tmp = asdc_fluxbctamp_ajouter(dst, dst->tf_pta,
//                                         dst->tf_flux, dst->tf_zd, &dst->pbcfl);
// 
//                                /* Si tmp, traitement de l'erreur */
//                                if (tmp)
//                                  { bcflux_err(dst, dst->tf_attente,
//                                           dst->tf_flux, dst->tf_zd, tmp, 0, 0);
//                                  }
// 
//                                /* Abaissement indicateur */
//                                dst->tf_attente = 0;
//                              }
// 
// 
// 
// 
// 
//                            bc = L(ptr+1);   	        /* Bloc BC */
//                            flux = LIL(bc+IMFLUX, 208);   /* Flux eventuel */
// 
// 
// 
// 
//                            /* Traitement d'une eventuelle attente de */
//                            /* la fin d'execution du bloc BC courant  */
//                            semexbc = (wait_queue_head_t *)
//                                                   LI(bc + IMSEMBC, 231);
//                            if (semexbc)
//                              {
//                                SRESET_OLD(semexbc);
//                              }
// 
// 
// 
// 
//                            /* Pas de traitement si pas de flux */
//                            if (!flux) break;
// 
//                            /* Parametres du flux */
//                            zd = LI(flux + IMPZD, 209);
//                            nbmte = LI(zd + IMNBMTE, 210);
//                            nbcte = LI(zd + IMNBCTE, 211);
//                            nblte = LI(zd + IMNBLTE, 212);
//                            nbmts = LI(zd + IMNBMTS, 213);
//                            nbcts = LI(zd + IMNBCTS, 214);
//                            nblts = LI(zd + IMNBLTS, 215);
//                            evt = LI(zd + IMEVT, 216);
// 
//                            /* Pointeur bloc de donnees */
//                            ad = L(bc + 6);
// 
//                            /* Comptage des executions */
//                            EI(bc+IMCPTR, LI(bc+IMCPTR, 217) + 1, 501);
// 
// 
//                            /******************************************/
//                            /* Traitement flux entree (bus --> appli) */
//                            /******************************************/
// 
//                            /* Recuperation tampon dispo a remplir */
//                            x = asdc_fluxbctamp_nouveau
//                                                (&dst->pbcfl, &dst->tf_pta);
// 
//                            /* Si x, traitement de l'erreur */
//                            if (x)
//                              { bcflux_err(dst, bc, flux, zd, x, 0, 0);
//                              }
//                            else
//                              {
//                                /* Mise a jour du tampon */
//                                t = &((dst->tf_pta)->tamp);
//                                t->type = L(bc);
//                                t->err = LIL(bc+IMERR, 218);
//                                EIL(bc+IMERR, 0, 502);	/* RAZ erreur 1553 */
//                                t->cmd1 = L(bc+1);
//                                t->cmd2 = L(bc+2);
//                                t->sts1 = L(bc+3);
//                                t->sts2 = L(bc+4);
//         /* Pour test */    *((int *)(&t->date[0])) = LI(bc+IMCPTR, 219);
//                                /* Enregistrement eventuel des donnees */
//                                if (    (t->type == RTBC)
//                                     || (t->type == BCRT)
//                                     || (t->type == BCRT_DI))
//                                  { n = t->cmd1 & 0x1F;
//                                    if (!n) n = 32;
//                                    for (i=0; i<n; i++) t->d[i] = L(ad+i);
//                                  }
//                                else if (t->type == COCO)
//                                  { /* y-a-t-il une donnee ? */
//                                    if ((t->cmd1 & 0x410) == 0x410)
//                                      { t->d[0] = L(ad);
//                                      }
//                                  }
// 
//                                /* Memorisation des elements lies au tampon */
//                                /* et leve indicateur "tampon en attente"   */
//                                dst->tf_attente = bc;
//                                dst->tf_zd = zd;
//                                dst->tf_flux = flux;
//                              }
// 
//                            /******************************************/
//                            /* Traitement flux sortie (appli --> bus) */
//                            /******************************************/
//                            tb = L(bc) & 0xFF;       /* Type du bloc */
//                            cm = L(bc+1);	    /* Mot de commande */
//                            /* Le filtrage par 0xFF elimine les bits */
//                            /* susceptibles d'avoir ete modifies par */
//                            /* le firmware                           */
// 
//                            /* Si transferts sans donnees en sortie : fini */
//                            if (    (tb==BCRT)
//                                 || ((tb==COCO) && !(cm & 0x400))
//                                 || (tb==BCRT_DI)
//                               )
//                              {
//                               /* Recherche du tampon a vider */
//                               if (!nbcts)
//                                 {
//                                   /* Plus de tampons disponibles ! */
//                                   /* Traitement erreur flux (code 1) */
//                                   bcflux_err(dst, bc, flux, zd, 1, 0, 0);
//                                 }
//                               else
//                                 { /* Tampons encore disponibles */
// 
//                                   /* Mise a jour du tampon en mem. echange */
//                                   t = &((struct asdcbc_tfch *)
//                                                     LI(zd+IMPPTS, 220))->tamp;
// 
//                                   if (    (t->type != tb)
//                                        || (t->cmd1 != cm))
//                                     {
//                                       /* Incoh�rence entre trame et flux ! */
//                                       /* Traitement erreur flux (code 2) */
//                                       bcflux_err(dst, bc, flux, zd, 2,
//                                                        t->type, t->cmd1);
// 
//      // kkprintf("bc=0x%04X : Incoherence entre trame et donnees du flux S !\n",
//      //              bc);
//      // kkprintf("   IT -   p : 0x%X\n" , LI(zd+IMPPTS, 221));
//      // kkprintf("   tb=%X t->type=%X   cmd=%X t->cmd1=%X\n",
//      //                       tb, t->type, L(bc+1), t->cmd1);
// 
//                                     }
//                                   else
//                                     { /* Copie des donnees dans le coupleur */
//                                       if (tb == COCO)
//                                         { /* Commande codee */
//                                           E(ad, t->d[0]);
//                                         }
//                                       else
//                                         { /* Transfert BC-RT */
//                                           n = t->cmd1 & 0x1F;
//                                           if (!n) n = 32;
//                                           for (i=0; i<n; i++)
//                                             { E(ad+i, t->d[i]);
//                                             }
//                                         }
//                                     }
// 
//                                   /* Suppression du tampon lu */
//                                   x = asdc_fluxbctamp_retirer(
//                                                 (struct asdcbc_tfch **)
//                                                           &LI(zd+IMPPTS, 222),
//                                                                   &dst->pbcfl);
//                                   nbcts--;
//                                   EI(zd + IMNBCTS, nbcts, 503);
// 
// 
//                                   /* Si plus de tampons, pointeur */
//                                   /* dernier tampon = NULL        */
//                                   if (nbcts == 0) EI(zd + IMPDTS,
//                                                               (long) NULL, 504);
// 
//                                   /* Si x, traitement de l'erreur */
//                                   if (x)
//                                     { bcflux_err(dst, bc, flux, zd, x, 0, 0);
//                                     }
//                                 }
//                              }
// 
//                            /* Reveil eventuel de la tache utilisateur */
//                                               j = 0;
//                            if (nbcte > nblte) j |= FLX_LIME;
//                            if (nbcts < nblts) j |= FLX_LIMS;
//                            if ((j >> 8) & evt)
//                                          SRESET_IM(&LI(flux + IMSEMFLX, 223));
//                          }
//                          break;

               case 0x20 : /* RT block Xfer complete */
               case 0x21 : /* RT block Xfer no data */
               case 0x41 : /* General interrupt */
                         KPRINTF("ASDC : Code IT %d non traite !\n", L(ptr));
                         break;

               default : KPRINTF("ASDC : Code IT %d inconnu !\n", L(ptr));
            }
         ptr += 4;
      }

   // zzprintf("ASDC/IT2\n");

   /* Reveil tache qui attend liste des ITs */
   /* if (itbc) wakeup((caddr_t) &asdc_n_bc_it[asdcunit]); */

   /* Debloquage de la queue */
   E(IQRSP, 1);

   udelay(3);   /// Le systeme hote est trop rapide,
                    ///  ==> Il faut laisser au PPC le temps de remettre
                    ///      a zero la demande d'IT avant de rearmer les
                    ///      interruptions (ce rearmement sera effectue
                    ///      par le driver dans l'une des fonctions qui
                    ///      ont appele isr_asdc()).

   // KPRINTF("}");

   /* Pour tentative de debug en cas de crash dans KDB */
   dst->dphit = -1;

    /* Instrumentation pour mesures perfos : descente d'une voie TOR */
    /* Mise a un de la sortie TOR SSYNC4                        */
    /* E2(0x9094, L2(0x9094) | 0x00000008);                     */

 tag(4);
// printk("tag(4)\n");
}





