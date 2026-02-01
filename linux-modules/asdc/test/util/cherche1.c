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
                         derniere modif le 14 janvier 1989
*/


#include <dos.h>
#include <string.h>
#include <stdlib.h>

int cherche(chaine, phase, resultat)
int phase;
char chaine[], resultat[];
{static char dta[43];
 static union REGS rin, rout;
 static struct SREGS sr;
 unsigned int adta;         /* Pour memorisation adresse DTA */
 unsigned int sdta;         /* initial et de son segment    */

 int st;
 int i, j;

 void far *pdta;            /* Pour permettre emploi FP_OFF et FP_SEG */
 pdta=&dta[0];

 rin.h.ah = 0x2F;     /* Lecture adresse DTA courante */
 int86x(0x21, &rin, &rout, &sr);
 adta=rin.x.bx;       /* Sauvegarde adresse */
 sdta=sr.es;

 rin.h.ah = 0x1A;     /* Programmation nouvelle DTA */
 rin.x.dx=FP_OFF(pdta);
 sr.es=FP_SEG(pdta);
 int86x(0x21, &rin, &rout, &sr); /* DTA imposee */

 switch(phase)
    {
     case 1 : rin.h.ah=0x4E;
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

 rin.h.ah = 0x1A;     /* Restauration DTA initiale */
 rin.x.dx=adta;
 sr.es=sdta;
 int86x(0x21, &rin, &rout, &sr); /* DTA imposee */

}
