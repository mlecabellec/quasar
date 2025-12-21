   /**************************************************************/
   /*                                                            */
   /*      RECHERCHE D'UN NOM DE FICHIER NON ENCORE UTILISE      */
   /*                                                            */
   /*                         Y. Guillemot, le    10 avril 1991  */
   /*                       derniere modif. le    12 avril 1991  */
   /**************************************************************/


/* 
   Un nom de fichier non encore utilise dans un repertoire donne est recherche.
   
   Le nom a une structure du type :    "DDDNNNFFF"
   
        ou  "DDD"  est une chaine de caracteres donnee (debut),
            "FFF"  est aussi une chaine de caracteres donnee (fin) et
            "NNN"  est une valeur numerique.
            
    La fonction essaye toutes les valeurs numeriques (depuis la valeur pointee
    par pnuminit passee en argument) jusqu'a la decouverte d'un fichier 
    inexistant.
    
    Elle renvoie un pointeur sur ce nom (ou NULL en cas d'erreur).
    
    L'entier pointe par pnuminit est mis a la valeur trouvee pour creer le
    premier nom de fichier inutilise.
    
    rep est limite a 80 caracteres
    debut est limite a 40 caracteres
    fin est limite a 20 caracteres
    
    Si le parametre rep (repertoire) est NULL, la recherche est effectuee 
    dans le repertoire courant.
*/
            
       

#include <stdio.h>
#include <string.h>

#include "noms_ext.h"

char *nfsuivant(rep, debut, fin, pnuminit)
char *rep;	/* Repertoire ou doit etre place le nouveau fichier */
char *debut;	/* Debut du nom du fichier */
char *fin;	/* Fin du nom du fichier */
int *pnuminit;	/* Premiere valeur numerique utilisee dans la recherche */
{  static char nom[160];  
   int i;
   char *p;
   int trouve;
   FILE *f;
                    
   if (rep == NULL) strncpy(nom, debut, 40);
               else { if (strlen(rep)>80) 
                         { fprintf(stderr, "nfsuivant : rep trop long\n");
                           return NULL;
                         }
                      strcpy(nom, rep);
                      strcat(nom, "/");
                      strncat(nom, debut, 40);
                    }
   
   p = &nom[strlen(nom)];  /* p pointe la position du nombre */

   i = *pnuminit;
   trouve = 0;
   do { sprintf(p, "%d", i);
        strncat(p, fin, 20); 
        
        f = fopen(nom, "r");
        if (f==NULL) trouve = 1;
                else { fclose(f);
                       i++;
                     }
      } while(!trouve);
      
   *pnuminit = i;
   return nom;
}   
