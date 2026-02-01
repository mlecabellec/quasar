   /**************************************************************/
   /*      Introduction par l'operateur d'une valeur entiere     */
   /*                                                            */
   /*          question : chaine affichee sur stdout pour        */
   /*                     interroger l'operateur                 */
   /*          vmini : Valeur minimale autorisee comme reponse   */
   /*          vmaxi : Valeur maximale autorisee comme reponse   */
   /*                                                            */
   /*            --> Si vmini >= vmaxi, tout entier est autorise */
   /*                                                            */
   /*          La question est posee a l'operateur jusqu'a       */
   /*          l'obtention d'une reponse correcte (syntaxe et    */
   /*          bornes)                                           */
   /*                                                            */
   /*          La fonction renvoie la valeur fournie par         */
   /*          l'operateur                                       */
   /*                                                            */
   /*                           Y. Guillemot, le 12 mars 1992    */
   /*                         derniere modif. le                 */
   /**************************************************************/

#include <stdio.h>
#include <string.h>
#include "noms_ext.h"

#define TAILLE_TAMPON 100


long int entrer_e(question, vmini, vmaxi)
char *question;
long int vmini, vmaxi;
{  
   char tampon[TAILLE_TAMPON];
   int r, i;
   long int x;
   
   r=-1;
   while (r!=0) 
     { if (question!=NULL) fprintf(stdout, "%s", question);
       fflush(stdout);
       fgets(tampon, TAILLE_TAMPON, stdin);
       
       /* Elimination du retour chariot malencontreux que laisse fgets() */
       i = strlen(tampon);
       if (i==0) { r = -1;
                   continue;
                 }
            else { i--;
                   if (tampon[i] == '\n') { if (i==0) continue;
                                            tampon[i] = '\0';
                                          }
                 }
        
       r = conversionl(tampon, &x);
       if (r!=0) { fprintf(stdout , "\007Ceci n'est pas un entier !\n");
                   continue;
                 }
       if (vmini<vmaxi)
          { if ((vmini>x) || (vmaxi<x))
               { fprintf
                  (stdout , 
                   "\007Cette valeur doit etre comprise entre %ld et %ld !\n",
                   vmini, vmaxi
                  );
                 r = -1;
               }
          }
     }
   return x;
}                 
                  
   



