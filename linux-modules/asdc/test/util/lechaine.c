/***************************************************************************/
/*   LECTURE D'UNE CHAINE ACHEVEE PAR UN TERMINATEUR DONNE DANS UN FICHIER */
/*   AVEC FILTRAGE DES COMMENTAIRES VIA LA FONCTION "carsuiv()"            */
/*                                                                         */
/*   - Le fichier doit avoir ete ouvert par fopen() et la fonction         */
/*     inicarsuiv() doit avoir ete appelee                                 */
/*                                                                         */
/*   - lecchaine(chaine, n, term)                                          */
/*      - chaine pointe la chaine de caracteres ou le resultat est range,  */
/*        termine par un 0                                                 */
/*      - n represente la taille de chaine, la fonction renverra au plus   */
/*        n-1 caracteres suivis d'un 0 (soit n caracteres)                 */
/*      - term est une chaine de caracteres contenant les terminateurs     */
/*        possibles pour la chaine lue - '\0' est toujours un terminateur  */
/*        possible et n'a donc pas (heureusement ...) a figurer dans term  */
/*      - la fonction renvoie un entier qui represente                     */
/*              - Le code du terminateur trouve (s'il a ete trouve ...)    */
/*              - La valeur -1 si la taille de la chaine cherchee est      */
/*                superieure a n-1 caracteres                              */
/*                                                                         */
/*                                         Y. Guilemot, le 22/11/1990      */
/*                                      derniere modif. le                 */
/***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "noms_ext.h"

extern char carsuiv();


int lecchaine(chaine, n, term)
char *chaine;
int n;
char *term;
{  
   int i;
   char c, *k;
   
   for(i=0; i<n-1; i++)     /* Execution boucle n-1 fois */
      { chaine[i] = c = carsuiv();
        if((k=strchr(term, c))!=NULL) break;
      }
      
   if(k==NULL) { chaine[n-1]=0;   /* Alors, arret pour cause de debordement */
                 return(-1);
               }
               
   /* Sinon, la decouverte d'un terminateur est cause de l'arret */
   chaine[i]=0;            /* Caractere nul de fin de chaine */
   return (c & 0xFF);      /* Code du terminateur */
}
