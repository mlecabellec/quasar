/***************************************************************************/
/*                                                                         */
/*  EXTRACTION D'UNE CHAINE DE CARACTERES PLACEE ENTRE 2 SEPARATEURS:      */
/*  (Un caractere separateur etant double dans la chaine de facon a        */
/*  etre reconnu)                                                          */
/*                                                                         */
/*                                                                         */
/*  char *decadre(source, destination, separateur)                         */
/*  char *source;                                                          */
/*  char *destination;                                                     */
/*  char separateur;                                                       */
/*                                                                         */
/*  source et destination : pointeurs sur les chaines (terminees par '\0') */
/*                                                                         */
/*  separateur : le caractere utilise pour encadrer la chaine              */
/*                                                                         */
/*  La fonction renvoie - un pointeur sur destination si tout est OK       */
/*                      - NULL en cas d'encadrement anormal                */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                      Anonymized, le    4 avril 1991   */
/*                                                                         */
/*                         Mise en conformite gcc v4, le     15/02/2008    */
/*                                    derniere modif. le                   */
/***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "noms_ext.h"


char *decadre(source, destination, separateur)
char *source, *destination;
char separateur;
{  char *p, *q;
   int i;
   
   p = source;
   
   if (*(p++) != separateur) return NULL;
   
   for (q = destination; ; q++)
      { if (*p == separateur) 
            { p++;
              if (*p == separateur) { p++;
                                      *q = separateur;
                                    }
                   else if(*p == '\0')
                                 { *q = '\0';
                                   return destination;
                                 }
                            else  { *q = '\0';
                                    return NULL;
                                  }
            }
           else if (*p=='\0')  return NULL;
                 else *q = *(p++);
      }
}                 
