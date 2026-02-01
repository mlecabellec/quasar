   /**************************************************************/
   /*    Conversion d'une chaine de caracteres en entier long    */
   /*                                                            */
   /*           Syntaxes autorisees : 	dddddd                   */
   /*					0xhhhhh			 */
   /*					$hhhhhh			 */
   /*					"cccc"			 */
   /*                                                            */
   /*         Renvoie 0 si tout s'est bien passe, -1 autrement.  */
   /*                                                            */
   /*                         Anonymized, le 2 janvier 1991    */
   /*                       derniere modif. le 1 fevrier 1991    */
   /*                                                            */
   /*            Mise en conformite gcc v4, le     15/02/2008    */
   /**************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "noms_ext.h"

/* bcopy() ne figure pas dans les bibliotheques de Turbo-C */
#ifdef __TURBOC__
#define bcopy(s, d, n) memcpy(d, s, n)
#endif


int conversionl(tampon, presultat)
char *tampon;		/* Chaine a convertir */
long int *presultat;	/* Resultat de la conversion */
{   char *p, *q;
    int hexa;    /* Drapeau mis a 1 si codage hexa */
    
    trim(tampon);
    p=tampon;
    
    /* L'entier est il code en hexadecimal ? */
    hexa = 0;
    if(!strncmp(tampon, "0x", 2)) { hexa = 1;
                                   p += 2;
                                 }
    else if(!strncmp(tampon, "0X", 2)) { hexa = 1;
                                        p += 2;
                                      }
    else if(tampon[0] == '$') { hexa = 1;
                                p++;
                              }
    
    /* Traitement de la chaine si hexadecimal */
    if(hexa)
       { if(p=='\0') return(-1);
         
         /* Caracteres non hexa presents ? */
         q = p;
         while(*q)
            { if(!isxdigit(*q)) return(-1);
              q++;
            }
         sscanf(p, "%lx", presultat);
         return(0);
       }
       
       
       
    /* L'entier est il code en caracteres ? - Si oui : traitement */
    if(tampon[0]=='"')
      { char t[4];
        int i;
        
        for(i=0; i<4; i++)
           { p++;
             if(*p=='\0') return -1;
             if (*p=='"')    /* Cas d'un '"' double entre les guillemets */
                { p++;
                  if (*p!='"') return -1;
                  t[i] = '"';
                }
             else t[i]=*p;
           }
        p++;
        if(*p!='"') return -1;
        p++;
        if(*p!='\0') return -1;
        bcopy(t, presultat, 4);
        return 0;
      }
        


    
    /* Seul cas restant : l'entier est code en decimal ! */
    
    /* Caracteres non decimaux presents ? */
    q = p;
    if((*q=='+')||(*q=='-')) q++;  /* Saut du signe eventuel */
    while(*q)
        { if(!isdigit(*q)) return(-1);
          q++;
        }
    sscanf(p, "%ld", presultat);
    return(0);
}
       
       
