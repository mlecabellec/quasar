/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*                       FONCTIONS DIVERSES                           */
/*                                                                    */
/*                                            Y.G., le 3/9/1992       */
/**********************************************************************/



#include <stdio.h>

#include "ptc.glob.h"








/*********************************************************************/
/* Fonctions renvoyant au PASCAL le nombre de lignes et le nombre de */
/* colonnes disponibles sur l'ecran utilise                          */
/*********************************************************************/
int nombre_lignes()   
{ if (!termcap_deja_initialise) init_termcap(1);
  return li; 
}

int nombre_colonnes() 
{ if (!termcap_deja_initialise) init_termcap(1);
  return co; 
}




/******************************************************************/
/* Specification du fichier ou l'operation doit etre effectue     */
/*                                                                */
/* ATTENTION : si appel depuis PASCAL, il faut donner comme arg.  */
/* un pointeur de fichier C, soit utiliser la syntaxe :           */
/*   sortie_termcap(getfile(fichier_pascal), var_nom_term);       */
/*                                                                */
/* L'argument "nom" doit etre utilise pour passer le type du      */
/* terminal, puisque ce dernier ne peut plus etre extrait de la   */
/* la variable d'environnement TERM                               */
/*                                                                */
/* Si l'argument "nom" est une chaine vide, le nom du terminal    */
/* est malgre tout extrait de l'environnement                     */
/******************************************************************/
void sortie_termcap(fp, nom)
FILE *fp;
struct ch80 *nom;
{ 
  int i;
  
  /* Memorisation de l'identificateur du fichier */
  fich = fp;
  
  /* Stockage de nom en variable globale term_impose */
  if (nom->t > 80)
       { nom->t = 80;
         fprintf(stderr, "\007sortie_termcap : troncature nom terminal !\n");
       }
  for (i=0; i<nom->t; i++) term_impose[i] = nom->c[i];
  term_impose[i] = '\0';
       
  /* Interrogation du fichier printcap */     
  if (nom->t) init_termcap(0);
         else init_termcap(1);  
}




