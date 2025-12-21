/*******************************************************************/
/*            VISUALISATION (POUR DEBUG) DES STRUCTURES            */
/*               D'ALLOCATION MEMOIRE DU PILOTE ASDC               */
/*                                                                 */
/*      CONTROLE DE LA COHERENCE DU CONTENU DE CES STRUCTURES      */
/*                                                                 */
/*                            Y. Guillemot, le       16 mai 1994   */
/*                 Carte SBS1553 ABI/PMC2 : le   6 novembre 2000   */
/*              Mise en conformite gcc v4 : le   15 fevrier 2008   */
/*                          derniere modif. le                     */
/*******************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "asdcctl.h"


main(argc, argv)
int argc;
char *argv[];
{  int fd;
   struct asdcememo mem;
   struct asdcblibre blibre;
   
   struct asdcblibre *memo;
   int *t;
   int *memoire;
   
   int i, j, k, l;
   
#define TTREC 50
   int ttrec, trec[TTREC];   /* Table pour limiter affichage si recouvrements */

   /* Initialisation */
   ttrec = 0;
   
   if (argc<2) { printf("Usage : %s device\n", argv[0]);  
                 exit(-1);
               }
               
   
   if ((fd = open(argv[1],O_RDWR)) < 0) { perror("open");
                                          exit(-1);
                                        }
                                                               
  
   
   if(ioctl(fd, ASDCLECEMEMO, &mem) == -1)
                      { perror("ioctl(ASDCLECEMEMO)");
                        exit(-1);
                      }
                      
                      
   printf("\n");
   printf("Taille table de gestion memoire du pilote : %d\n", mem.tmemo); 
   printf("Indice plus petit bloc libre :              %d\n", mem.ipremier); 
   printf("Indice plus grand bloc libre :              %d\n", mem.idernier); 
   
   /* Allocation memoire pour reconstituer table interne au pilote */
   memo = (struct asdcblibre *) calloc(mem.tmemo, sizeof(struct asdcblibre));
   if (memo == NULL) { printf("Impossible d'allouer memoire pour memo[] !\n");  
                       exit(-1);
                     }
                     
   
   /* Lecture table interne au pilote */
   for(i=0; i<mem.tmemo; i++)
      { blibre.i = i;
        if(ioctl(fd, ASDCLECMEMO, &blibre) == -1)
                      { perror("ioctl(ASDCLECMEMO)");
                        exit(-1);
                      }
         memo[i] = blibre;
      }
      
      
   /* Creation liste des entrees utilisees et controle linearite de la chaine */
   t = (int *) calloc(mem.tmemo, sizeof(int));
   if (t == NULL) { printf("Impossible d'allouer memoire pour t[] !\n");  
                    exit(-1);
                  }
   for (i=0; i<mem.tmemo; i++) t[i] = 0;
   for (i=mem.ipremier ;i!=-1; i=memo[i].s)
     { if (t[i])
         { printf("Erreur : la chaine des blocs libre est bouclee !\n");
           printf("   Dump de la table de gestion memoire d'echange :\n");
           for (i=0; i<mem.tmemo; i++)
             { printf("i=%03d (%03d)   a=0x%04X   l=0x%04x [%05d]",
                       i, memo[i].i, memo[i].a&0xFFFF, 
                          memo[i].l&0xFFFF, memo[i].l&0xFFFF);
               printf("    s=%03d  p=%03d\n", memo[i].s, memo[i].p);
             }
           exit(-1);
         }
       t[i] = 1;
     }
     

   /* Controle coherence du chainage */
   i = mem.ipremier;
   if ((i == -1) && (mem.idernier != -1))
      printf("Incoherence entre premier=%d et dernier=%d\n", i, mem.idernier);
    else if (memo[i].p != -1) printf("Erreur : memo[premier].p = %d != -1\n",
                                      memo[i].p);
   
      
   for( ;i!=-1; i=memo[i].s)
     { j = memo[i].s;
       if (j != -1)
          { if (memo[j].p != i)
              { printf("Erreur chainage : memo[%d].s = %d\n", i, memo[i].s);
                printf("                  memo[%d].p = %d\n\n", j, memo[j].p);
              }
          }
     }
   
   /* Controle validite du tri */
   for (i=mem.ipremier ;i!=-1; i=memo[i].s)
     { j = memo[i].s;
       if (j != -1)
          { if (memo[j].l < memo[i].l)
              { printf("Erreur tri : memo[%d].l = %d > %d = memo[%d].l\n",
                        i, memo[i].l&0xFFFF, memo[j].l&0xFFFF, j);
              }
          }
     }

   
   /* Controle marquage des entrees inutilisees */
   for (i=0; i<mem.tmemo; i++)
      { if ((!t[i]) && (memo[i].l != 0xFFFF))
          { printf("Erreur : L'entree memo[%d] est hors chaine ", i);
            printf("mais memo[%d].l=%d != -1\n", i, memo[i].l&0xFFFF);
          }
      }

   
   /* Affichage liste des blocs libres */
   printf("\nBlocs libres :\n");
   printf("   Indice    Debut      Fin    Taille\n");
   /* Fmt "    9999    0xXXXX    0xXXXX    99999" */
   for (i=mem.ipremier ;i!=-1; i=memo[i].s)
     { j = memo[i].s;
       printf("    %4d    0x%04X    0x%04X    %5d\n",
               i, memo[i].a&0xFFFF, 
               (memo[i].a&0xFFFF) + (memo[i].l&0xFFFF) - 1, memo[i].l&0xFFFF);
     }
   
   
   /* Reconstitution memoire et controle de non recouvrement des blocs libres */
   memoire = (int *) calloc(0xFFFF, sizeof(int));
   if (memoire == NULL)
               { printf("Impossible d'allouer memoire pour memoire[] !\n");  
                 exit(-1);
               }
   for (i=0; i<=0xFFFF; i++) memoire[i] = -1;   /* Memoire utilisee */
   for (i=mem.ipremier ;i!=-1; i=memo[i].s)
     { for(j=memo[i].a&0xFFFF; j<(memo[i].a&0xFFFF)+(memo[i].l&0xFFFF); j++)
         { if (memoire[j] != -1)
              { k = ((i & 0xFFF) << 16) + (memoire[i]&0xFFF);
                for (l=0; l<ttrec; l++)
                   { if (trec[l] == k) break;
                   }
                if (l == ttrec)
                   { printf("Recouvrement des blocs %d et %d !\n",
                             i, memoire[j]);
                     if (ttrec < TTREC) trec[ttrec++] = k;
                   }
              }
            else 
              { memoire[j] = i;                /* Memoire libre */
              }
          }
     }
   
   
   /* Affichage cartographie memoire d'echange */
   printf("\n\n\nCarte de la memoire d'echange : \n\n");
   printf("     0.. 1.. 2.. 3.. 4.. 5.. 6.. 7.. 8.. 9..");
   printf(" A.. B.. C.. D.. E.. F..\n");
   for (i=0; i<16; i++)
      { printf("%1X... ", i);
        for (j=0; j<64; j++)
           { for (k=0, l=0; k<64; k++)     
                { if (memoire[i*0x1000+j*0x40+k] == -1) l |= 1;
                                                   else l |= 2;
                }
             switch(l) { case 0 : printf("?"); break; /* Anormal ! */
                         case 1 : printf("O"); break; /* Zone utilisee */
                         case 2 : printf("-"); break; /* Zone libre */
                         case 3 : printf("x"); break; /* Zone en partie libre */
                         default : printf("A l'aide !   [i=%d j=%d l=%d]\n",
                                           i, j, l); /* Encore plus anormal ! */
                       }
           }
        printf("\n");
      }
      
   printf("     Chaque caractere represente 64 mots : O ==> Occupes\n");
   printf("                                           ");
   printf("x ==> Partiellement occupes\n");
   printf("                                           ");
   printf("- ==> Libres\n");
      
                      
   exit(0);
 
}   
   
