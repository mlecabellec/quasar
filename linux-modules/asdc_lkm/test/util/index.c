
                    /* Fonction INDEX non fournie avec C sous VMS  */
                    /*   --> Imitation de la fonction INDEX du PL1 */
                    /* et du PASCAL/VMS                            */
                    /*                                             */
                    /* index(str1,str2) renvoi un entier (position */
                    /* de la chaine STR2 dans la chaine STR1, 0 si */
                    /* STR2 n'est pas trouvee) .                   */
                    /*                                             */
                    /* Peut-etre aurait-il mieux valu renvoyer un  */
                    /* pointeur et non une "position" ?            */
                    /*                                             */
                    /*                     Y. Guillemot - 1/8/1988 */
                    /*                                             */
                    /*    Mise en conformite gcc v4, le 15/02/2008 */


        /* Fonction INDEX(str1,str2) : renvoi la position (en caracteres)
           de la premiere occurence de str2 dans str1 (0 si aucune occurence) */
 
#include <string.h>
#include "noms_ext.h"

   int index(str1, str2)
   char *str1, *str2;
   {  int diff;
      char *p1;
      char *i, *j;
      for (p1=str1 ; (*p1!=0) && (strlen(p1)>=strlen(str2)); p1++)
             {   diff =0;            
                 for (i=p1, j=str2; (*j!=0) && !diff; i++, j++) 
                        {  if (*i != *j) diff=1;
                        }                 
                 if (!diff) return((int)(p1-str1+1));
             }
      return(0);
   }
