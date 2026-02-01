/***************************************************************************/
/*   RECHERCHE DE L'INDICE D'UN MOT DANS UNE TABLE CONNAISSANT UNE         */
/*   "ABREVIATION" (C'EST A DIRE "LE DEBUT") DU DIT MOT                    */
/*                                                                         */
/*   Fonction "iabrev(mot, table)"                                         */
/*         - renvoie :   l'indice du mot trouve dans la table              */
/*                       -1 si aucun mot ne correspond                     */
/*                       -2 si l'abreviation est ambigue                   */
/*                                                                         */
/*         - char *mot    : le mot (eventuellement abrege) a chercher      */
/*         - char **table : la table des mots                              */
/*                          (elle s'acheve par le mot "")                  */
/*                                                                         */
/*         (si mot=="", l'indice de "" (fin de la table) est renvoye)      */
/*                                                                         */
/*                                         Y. Guilemot, le  2/02/1991      */
/*                                      derniere modif. le                 */
/***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "noms_ext.h"


int iabrev(mot, table)
char *mot;	/* Mot a chercher */
char **table;	/* Table des mots */
{
   int i, indice, l;
   int trouve = 0;
   int ambigu = 0;

   l = strlen(mot);

   if (l!=0)    /* Traitement particulier si mot="" */

      {	for (i=0; table[i][0]!=0; i++)
	   { if (!trouve) { trouve = !strncmp(mot, table[i], l);
                            indice = i;
                          }
		     else { if(ambigu = !strncmp(mot, table[i], l)) break;
			  }  		       
	   }

	return trouve ? (ambigu ? -2 : indice) : -1;
      }

     else      /* Cas de mot="" */

      { for (i=0; table[i][0]!=0; i++);   /* Recherche indice fin de table */
        return i;
      }

}
