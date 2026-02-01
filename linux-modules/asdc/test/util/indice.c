/***************************************************************************/
/*   RECHERCHE DE L'INDICE D'UN MOT DANS UNE TABLE                         */
/*                                                                         */
/*   Fonction "indice(mot, table)"                                         */
/*         - renvoie l'indice du mot dans la table ou -1 si pas trouve     */
/*                                                                         */
/*         - char *mot    : le mot a chercher                              */
/*         - char **table : la table des mots                              */
/*                          (elle s'acheve par le mot "")                  */
/*                                                                         */
/*         (si mot=="", l'indice de "" (fin de la table) est renvoye)      */
/*                                                                         */
/*                                         Y. Guilemot, le 24/11/1990      */
/*                                      derniere modif. le                 */
/***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "noms_ext.h"

int indice(mot, table)
char *mot;	/* Mot a chercher */
char **table;	/* Table des mots */
{
   int i;
   int trouve=0;

   for (i=0; table[i][0]!=0; i++)
      { if (trouve = !strcmp(mot, table[i])) break;
      }

   /* Ligne suivante pour renvoyer indice !=-1 si mot=="" */
   if(mot[0]==0) trouve = 1;

   return trouve ? i : -1;
}
