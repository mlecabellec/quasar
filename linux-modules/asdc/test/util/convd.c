   /**************************************************************/
   /*   Conversion d'une chaine de caracteres en reel "double"   */
   /*                                                            */
   /*         Renvoie 0 si tout s'est bien passe, -1 autrement.  */
   /*                                                            */
   /*                         Anonymized, le 2 janvier 1991    */
   /*                                                            */
   /*            Mise en conformite gcc v4, le     15/02/2008    */
   /*                       derniere modif. le                   */
   /**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "noms_ext.h"



int conversiond(tampon, presultat)
char *tampon;		/* Chaine a convertir */
double *presultat;	/* Resultat de la conversion */
{   char *p, *q;
    int n, m;
    
    trim(tampon);
    p=tampon;
    
    if((*p=='+')||(*p=='-')) p++;     /* Signe mantisse */
    n = 0;
    m = 0;
    while(isdigit(*p))
            { p++; /* Debut mantisse */
              n++; /* n : nombre de chiffres */
            }
            
    if (*p=='.')  /* Point decimal eventuel */
        { p++;
          while(isdigit(*p))
            { p++; /* Fin mantisse */
              m++; /* m : nombre de chiffres */
            }
        }
        
            
    if((m+n)==0) return(-1); /* Pas de chiffre dans la mantisse ! */
    
    if((*p=='e')||(*p=='E'))    /* Indicateur d'exposant */
       { p++;
         n=0;
         if((*p=='+')||(*p=='-')) p++;     /* Signe exposant */
         while(isdigit(*p))
            { p++;    /* Exposant */
              n++;    /* n : nombre de chiffres */
            }
         
         if(n==0) return -1;   /* Pas de chiffres dans l'exposant */
       }
       
    if(*p!='\0') return -1;    /* Caracteres imprevus dans la chaine */
    
    /* Conversion */    
    sscanf(tampon, "%le", presultat);
    
    return(0);
}
