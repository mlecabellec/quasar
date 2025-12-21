/***************************************************************************/
/*   LECTURE D'UN FICHIER (PREALABLEMENT OUVERT) CARACTERE PAR CARACTERE   */
/*   EN SAUTANT LES COMMENTAIRES                                           */
/*                                                                         */
/*   Fonction "inicarsuiv()" : doit etre appelee une fois avant carsuiv()  */
/*    - fd est le descripteur du fichier a lire                            */
/*                                                                         */
/*   Fonction "carsuiv()" : - renvoie le "caractere suivant"               */
/*                          - renvoie le caractere 0 en fin de fichier     */
/*                                                                         */
/*   Deux variables globales externes sont definies :                      */
/*    - carsuiv_ligne est le numero de la ligne courante                   */
/*    - carsuiv_tamp est le tampon contenant la ligne courante             */
/*      (carsuiv_ligne et carsuiv_tamp sont destinees a l'elaboration des  */
/*       messages d'erreur)                                                */
/*                                                                         */
/*   Definition des commentaires :                                         */
/*         Premiere sorte :                                                */
/*            Il commence par '/' suivi de '*' et s'acheve par '*' suivi   */
/*            de '/'                                                       */
/*         Seconde sorte :                                                 */
/*            Il commence par '/' suivi de '/' et s'acheve a la fin de la  */
/*            ligne                                                        */
/*                                                                         */
/*   Une chaine de caractere commence et s'acheve par '"'                  */
/*   Un commentaire ne peut pas etre inclu dans une chaine                 */
/*                                                                         */
/*                                        Y. Guillemot, le 22/11/1990      */
/*                                      derniere modif. le                 */
/***************************************************************************/

#include <stdio.h>
#include "noms_ext.h"

/* Taille du tampon de ligne */
#define TATAMP 250

/* Synonyme pour carsuiv_tamp */
#define tamp carsuiv_tamp

int carsuiv_ligne;		/* Numero ligne courante du fichier */
char carsuiv_tamp[TATAMP];	/* Tampon de stockage ligne courante */

static FILE *fich;		/* Memorisation descripteur du fichier a lire */

static int chaine;		/* Indicateur chaine de caracteres */
static int ic;			/* Indice du prochain caractere dans tamp */

static int blanc;		/* Indic. caractere blanc a ete lu */

static int avance;		/* Indic. caract. lu en avance par carsuiv() */ 



/* Initialisation des fonctions de lecture */
void inicarsuiv(fd)
FILE *fd;		/* Descripteur du fichier a lire */
{  chaine = 0;
   tamp[0] = 0;
   ic = 0;
   blanc = 0;
   avance = 0;
   fich = fd;
}




/************************************************/
/* Hierarchie des fonctions de lecture :        */
/*     carsuiv() appelle carsuiv1()             */
/*     carsuiv1() appelle carsuiv0()            */
/*     carsuiv0() appelle fgets()               */
/*                                              */
/* BUG : Le filtrage des blancs est fait AVANT  */
/* le filtrage des commentaires - Si plusieurs  */
/* commentaires se suivent, il se peut que plu- */
/* sieurs blancs se suivent en sortie           */
/*   Le remede consisterait a intervertir les   */
/* fonctions carsuiv() et carsuiv1() ...        */
/************************************************/



/* Fonction de lecture "de base" : lit le fichier et met a jour les tampons */
/*   sans se preoccuper des commentaires                                    */
static char carsuiv0()
{  
   if(feof(fich)) return '\0';
   while(tamp[ic]==0) 
           { fgets(tamp, TATAMP, fich);
             if(feof(fich)) return '\0';
             carsuiv_ligne++;
             ic=0;
           }
   return tamp[ic++];
}






/* Fonction de lecture "de niveau 1" :                              */
/*    - Fait appel a carsuiv0() et filtre les CR, LF et tabulations */
/*      qui sont remplacees par des blancs                          */
/*    - Plusieurs blancs consecutifs sont remplaces par un seul     */
/*    - Ces fonctions de filtrage ne sont pas actives a l'interieur */
/*      des chaines de caracteres                                   */
static char carsuiv1()
{  char c;
   
   c=carsuiv0();
   
   /* Si chaine en cours, pas de filtrage */
   if(chaine) return c;
   
   /* Sinon, comportement different pour premier caractere blanc */
   /* (ou assimile) et pour les suivants                         */        
   if(!blanc) 
            { if((c==' ') || (c=='\t') || (c==0xA) || (c==0xD))
                     { blanc = 1;
                       return ' ';   /* Le premier blanc est renvoye */
                     } 
                else { blanc = 0;
                       return c;     /* Un non blanc est renvoye */
                     }
            }
       else { /* On attend le premier non blanc pour le renvoyer */
              while((c==' ') || (c=='\t') || (c==0xA) || (c==0xD)) c=carsuiv0();
              blanc = 0;
              return c;
            }
}



/* Fonction de lecture avec filtrage des commentaires */
char carsuiv()
{
   char c; 		/* Caractere lu courant */ 
   static char c1;	/* Caractere lu en avance (si necessaire) */
   
   while (1)   /* On ne sort de cette boucle qu'en executant "return" */
     { 
       /* Lecture caractere courant */             
       if (avance) { c=c1; avance=0; }             
            else   c = carsuiv1();  
       
       /* Entree ou sortie d'une chaine ? */
       if(c=='"') chaine = !chaine;              
           
       /* Si pas de commentaire possible : termine ! */
       if ((c!='/') || chaine) return c;
   
       /* On n'est pas dans une chaine et c=='/' :    */
       /*   Il faut tester si commentaire commence    */
       avance = 1;
       c1 = carsuiv1();
   
       switch(c1)
           {  case '*' : /* Debut d'un commentaire de type 1 */
                         do {  /* Recherche de la prochaine '*' */
                               do { c=carsuiv1();
                                    if(c==0) return 0; /* Fin de fichier */
                                  }
                               while(c!='*');
                                 
                               /* Si plusieurs '*', recherche du premier */
                               /* caractere different                    */
                               do { c=carsuiv1();
                                    if(c==0) return 0; /* Fin de fichier */
                                  }
                               while(c=='*');
                                 
                            }
                         /* Fin boucle seulement si fin de commentaire */
                         while(c!='/');
                         avance=0;
                         break;
                     
              case '/' : /* Debut d'un commentaire de type 2 */
                         /* On force la lecture de la ligne suivante */
                         /* au prochain appel de carsuiv1()          */
                         tamp[0]=0;
                         ic=0;
                         avance=0;
                         break;
                     
              default :  /* Ce n'est pas un commentaire */
                         return c;      /* avance reste a 1 */
           }
     }  
}   
      
