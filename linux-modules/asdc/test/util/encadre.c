/***************************************************************************/
/*                                                                         */
/*  MISE ENTRE 2 SEPARATEURS D'UNE CHAINE DE CARACTERES :                  */
/*  (Si la chaine initiale contient un separateur, celui-ci est double     */
/*  dans la chaine resultat)                                               */
/*                                                                         */
/*                                                                         */
/*  char *encadre(source, destination, taille_destination, separateur)     */
/*  char *source;                                                          */
/*  char *destination;                                                     */
/*  int taille_destination;                                                */
/*  char separateur;                                                       */
/*                                                                         */
/*  source et destination : pointeurs sur les chaines (terminees par '\0') */
/*                                                                         */
/*  taille_destination : taille allouee a la chaine destination (pour      */
/*                            controle eventuel debordement)               */
/*                                                                         */
/*  separateur : le caractere utilise pour encadrer la chaine              */
/*                                                                         */
/*  La fonction renvoie : - un pointeur sur destination si tout est OK     */
/*                        - NULL si destination est trop petite            */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*                                      Anonymized, le    4 avril 1991   */
/*                                    derniere modif. le                   */
/***************************************************************************/

#include <stdio.h>

#include "noms_ext.h"


char *encadre(source, destination, taille_d, separateur)
char *source, *destination;
int taille_d;
char separateur;
{  char *p, *q;
   int i;
   
   if (taille_d<3) return NULL;
   
   q = destination;
   *(q++) = separateur;
   
   for (p=source, i=1; *p!='\0'; p++)
      { if (*p == separateur) 
            { i += 2;
              if (i>=taille_d) return NULL;
              *(q++) = separateur;
              *(q++) = separateur;
            }
          else
            { i++;
              if (i>=taille_d) return NULL;
              *(q++) = *p;
            }
      }
      
   if((i+2) > taille_d) return NULL;
   
   *(q++) = separateur;
   *q = '\0';
   return destination;
}
