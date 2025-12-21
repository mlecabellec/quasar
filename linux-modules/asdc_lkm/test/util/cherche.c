/* FONCTION "CHERCHE" : RECHERCHE DE FICHIERS DANS UN REPERTOIRE

     chaine   : specifie le (ou les) fichiers a rechercher (caracteres
                   "*" et "?" utilisables)
     phase    :  1 pour chercher le premier fichier
                 2 pour chercher les fichiers suivants
     resultat : nom du fichier trouve ou chaine vide

     valeur retournee : 0 si nom de fichier trouve
                        1 si aucun nom trouve
                        2 si parametre errone

                                   Y.G. le 17 juillet 1987
*/

  /* BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG - 30/8/88 */
   /*    ATTENTION : mauvais usage DTA :
                 il faut 1) faire un GET DTA
                         2) faire un SET DTA dans buffer local
                         3) faire operation
                         4) faire un SET DTA pour restaurer DTA initial
          ces operations sont a effectuer en phase 1 comme en phase 2

     BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG - 30/8/88

              Seul probleme : Il semble que le SET DTA reinitialise
           "quelquechose" dans le DTA .
              Conclusion : Ne faire aucun acces disque particulier
           avant la fin (ou l'interruption) d'une operation de
           recherche avec "wild-card" .                         */

 /*  BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG - 4/1/89    */


#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "noms_ext.h"

int cherche(chaine, phase, resultat)
int phase;
char chaine[], resultat[];
{static char dta[43];
 static union REGS rin, rout;
 int st;
 int i, j;

 switch(phase)
    {
     case 1 : rin.h.ah = 0x1A;
              rin.x.dx=&dta[0];    /* Indirection ! */
              int86(0x21, &rin, &rout); /* DTA imposee */

              rin.h.ah=0x4E;
              rin.x.cx=0x25;  /* Attributs des fichiers selectionnes :
                                  x  bit 0 : lecture seule
                                     bit 1 : fichier cache
                                  x  bit 2 : fichier systeme
                                     bit 4 : nom de volume
                                     bit 5 : repertoire
                                  x  bit 6 : fichier modifie       */

              rin.x.dx=chaine;     /*  Indirection ! */
              st=int86(0x21, &rin, &rout);

              if (rout.h.al==0)
                {for (i=30, j=0; dta[i]!=0; i++, j++)
                         resultat[j]=dta[i];
                 resultat[j]=0;
                 return(0);
                }
              else
                {resultat[0]=0;
                 return(1);
                }

     case 2 : rin.h.ah = 0x4F;
              int86(0x21, &rin, &rout);

              if (rout.h.al==0)
                {for (i=30, j=0; dta[i]!=0; i++, j++)
                         resultat[j]=dta[i];
                 resultat[j]=0;
                 return(0);
                }
              else
                {resultat[0]=0;
                 return(1);
                }

     default : perror("Phase innattendue dans cherche ! ");
               resultat[0]=0;
               return(2);
    }
}
