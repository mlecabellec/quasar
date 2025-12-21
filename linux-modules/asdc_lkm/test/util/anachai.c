/*********************************************************************/
/*                                                                   */
/*       ANALYSE D'UNE CHAINE DE CARACTERE AU FORMAT SUIVANT :       */
/*                                                                   */
/*      <nom1> <sep1> <valeur1> <sep2> <nom2> <sep1> <valeur2> ...   */
/*                       ... <sep2> <nomN> <sep1> <valeurN>          */
/*                                                                   */
/*   ou   <nom_i> represente le nom d'un parametre,                  */
/*        <valeur_i> la valeur associee a ce parametre,              */
/*        <sep1> et <sep2> des separateurs.                          */
/*                                                                   */
/*     <nom_i>, <valeur_i>, <sep1> et <sep2> sont des sous-chaines   */
/*   de caracteres.                                                  */
/*                                                                   */
/*                                                                   */
/*    Deux fonctions differentes sont definies :                     */
/*                                                                   */
/*       1) void initanachai(chaine, sep1, sep2, Tmax_nom, Tmax_val) */
/*               Initialise l'analyse et definit                     */
/*                       - La chaine a analyser (char *chaine)       */
/*                       - Les deux separateurs (char *sep1, *sep2)  */
/*                       _ Les tailles maxi (int Tmax_nom, Tmax_val) */
/*                         des chaines nom et valeur recherchees     */
/*                                                                   */
/*       2) int anachai(nom, valeur)                                 */
/*               Copie aux adresses passees en arguments les         */
/*             prochains nom et valeur obtenus en poursuivant        */
/*             l'analyse de chaine.                                  */
/*               Met des chaines nulles a ces adresses quand         */
/*             l'analyse est terminee                                */
/*               Renvoie :  0 si tout s'est bien passe               */
/*                         -1 quand l'analyse est terminee           */
/*                          1 si nom est trop long                   */
/*                          2 si valeur est trop long                */
/*                          3 si nom et valeur sont trop longs       */
/*               Si le nom ou la valeur est trop long, la chaine     */
/*             copiee est tronquee a la longueur specifiee a         */
/*             l'appel de initanachai()                              */
/*                                                                   */
/*                                                                   */
/*      ATTENTION : Les chaines sont terminees par un "nul" !        */
/*                  Les tailles donnees doivent inclure ce "nul" !   */ 
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                Y. Guillemot, le 26 octobre 1990   */
/*                              derniere modif. le 29 octobre 1990   */
/*                                                                   */
/*                   Mise en conformite gcc v4, le 15 fevrier 2008   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <TLPC/util.h> */

#include "noms_ext.h"

/* Variables statiques utilisees */
static char s1[21], s2[21];	/* Les separateurs */
static int ts1, ts2;		/* Taille des separateurs */
static int tn, tv;		/* Les tailles max. (sans le "nul" final) */
static char *p;			/* Position courante dans chaine */
static int init = 0;            /* drapeau initachai() deja appelee */
static char *chinit;		/* Pointeur chaine initiale */
static int tch;			/* Taille chaine initiale */
static int fini;                /* Drapeau "fin de la chaine atteinte" */


void initanachai(chaine, sep1, sep2, Tmax_nom, Tmax_val)
char *chaine;		/* chaine a analyser */
char *sep1, *sep2;	/* les separateurs */
int Tmax_nom;		/* Longueur maxi d'une sous-chaine nom */
int Tmax_val;		/* Longueur maxi d'une sous-chaine valeur */
{
   if ((strlen(sep1) > 20) || (strlen(sep2) > 20))
        { fprintf(stderr, "initanachai : Les separateurs sont trop longs !");
          exit(-1);
        } 

   if ((Tmax_nom > 80) || (Tmax_val > 80))
        { fprintf(stderr, "initanachai : Les tailles max. sont trop grandes !");
          exit(-1);
        } 

   strcpy(s1, sep1);
   strcpy(s2, sep2);
   ts1 = strlen(sep1);
   ts2 = strlen(sep2);
   tn = Tmax_nom - 1;
   tv = Tmax_val - 1;
   p = chaine;
   chinit = chaine;
   tch = strlen(chaine);
   init = 1;
   fini = 0;
}


int anachai(nom, valeur)
char *nom;		/* Zone ou le "nom" doit etre range */
char *valeur;		/* Zone ou la "valeur" doit etre rangee */
{
   int i, j;
   int ret = 0;	          /* Valeur a retourner (et son initialisation) */
   int zero_trouve = 0;   /* "nul" de fin de chaine trouve ! */

   if (!init) 
     { fprintf(stderr, 
               "anachai : Il faut appeler initanachai() avant anachai()\n");
       exit(-1);
     }

   /* Recherche de la premiere occurence de <sep1> */
   i = index(p, s1);

   /* Termine ? */
   if (i==0) goto plus_rien;

   /* Test depassement taille max. */
   if (i>tn+1) { j = tn;
                 ret = 1;
               }
          else { j = i-1;
               }

   /* Transfert du nom */
   strncpy(nom, p, j);
   nom[j] = 0;

   /* Increment du pointeur courant */
   p += i+ts1-1;


   /* Recherche de la premiere occurence de <sep2> */
   i = index(p, s2);


   /* Si plus de separateur <sep2>, la fin de la chaine en tient lieu ! */
   if (i==0)  { i = tch+1 - (p - chinit);
                zero_trouve = 1;                 /* Fin chaine atteinte */
              }

   /* Test depassement taille max. */
   if (i>tv+1) { j = tv;
                 ret |= 2;
               }
          else { j = i-1;
               }

   /* Transfert de la valeur */
   strncpy(valeur, p, j);
   valeur[j] = 0;

   /* Increment du pointeur courant */
   p += i+ts2-1;

   if (!fini) { if(zero_trouve) fini=1; /* La prochaine fois, termine ! */
                return (ret);
              }

   
plus_rien :
   init = 0;
   return (-1);

}
