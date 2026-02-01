/***************************************************************************/
/*                                                                         */
/*  EXTRACTION D'UNE CHAINE DE CARACTERES PLACEE ENTRE 2 SEPARATEURS:      */
/*  (Un caractere separateur etant double dans la chaine de facon a        */
/*  etre reconnu)                                                          */
/*                                                                         */
/*      Cette fonction est analogue a decadre(), mais elle n'impose pas    */
/*    a la chaine source d'etre la totalite de la chaine encadree.         */
/*      (le "p" de decadrep signifie "Partiel")                            */
/*                                                                         */
/*      Avant l'appel, un pointeur est positionne sur la chaine source.    */
/*                                                                         */
/*      Les blancs et tabs eventuels presents dans la chaine source sont   */
/*    sautes jusqu'a la decouverte du separateur de tete de la chaine      */
/*    encadree.                                                            */
/*                                                                         */
/*      La chaine est ensuite extraite jusqu'a decouverte d'un separateur  */
/*    unique indiquant la fin de la chaine encadree.                       */
/*                                                                         */
/*      Apres l'appel, le pointeur de la chaine source est positionne sur  */
/*    le premier caractere suivant la chaine encadree.                     */
/*                                                                         */
/*  char *decadrep(psource, destination, taille_destin, separateur)        */
/*  char **psource;                                                        */
/*  char *destination;                                                     */
/*  int  taille_destin;                                                    */
/*  char separateur;                                                       */
/*                                                                         */
/*  psource : pointe le pointeur sur la chaine source                      */
/*                                                                         */
/*  destination : pointeur sur la chaine resultat (terminee par '\0')      */
/*                                                                         */
/*  taille_destin : taille allouee a la chaine resultat                    */
/*                                                                         */
/*  separateur : le caractere utilise pour encadrer la chaine              */
/*                                                                         */
/*  La fonction renvoie - un pointeur sur destination si tout est OK       */
/*                      - NULL en cas d'encadrement anormal ou de resultat */
/*                        trop grand pour la zone destination              */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                      Y. Guillemot, le 28 octobre 1991   */
/*                                                                         */
/*                         Mise en conformite gcc v4, le     15/02/2008    */
/*                                    derniere modif. le                   */
/***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "noms_ext.h"


char *decadrep(psource, destination, taille_destin, separateur)
char **psource, *destination;
int taille_destin;
char separateur;
{  char *p, *q;
   int i;
   
   p = *psource;
   while(strchr("\040\011\012\015", *p) != NULL) p++;
   
   if (*(p++) != separateur) { *psource = p;
                               return NULL;   /* Le premier caract. non blanc */
                                              /* n'est pas le separateur !    */
                             }
   
   i = 0;
   for (q = destination; i<taille_destin; q++, i++)
      { if (*p == separateur) 
            { p++;
              if (*p == separateur) { p++;
                                      *q = separateur;
                                    }
                   else { *q = '\0';
                          *psource = p;
                           return destination;   /* Fin correcte */
                        }
            }
           else if (*p=='\0')  { *psource = p;
                                 return NULL;    /* Chaine mal encadree */
                               }  
                 else *q = *(p++);
      }
      
   *psource = p;
   return NULL;		/* Debordement de la chaine destination */
}                 
