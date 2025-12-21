/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*   RECHERCHE DES "CAPACITES" DISPONIBLES - version 2 arguments      */
/*                                                                    */
/*                                            Y.G., le 4/9/1992       */
/**********************************************************************/



#include <stdio.h>

#include "ptc.glob.h"







/*****************************************************/
/* Copie d'une chaine source s au format C dans une  */
/* chaine destination d au format PASCAL             */
/*****************************************************/
static void copie(d, s)
struct ch80 *d;
char *s;
{
  int i, l;
  
  l = strlen(s);
  if (l>80) { l = 80;
              fprintf(stderr, "termcap-copie : debordement !\n");
            }
  for (i=0; i<l; i++) d->c[i] = s[i];
  d->t = l;
}



/*****************************************************/
/* Concatenation d'une chaine source s au format C a */
/* une chaine destination d au format PASCAL         */
/*****************************************************/
static void concat(d, s)
struct ch80 *d;
char *s;
{
  int i, j, l;
  
  l = strlen(s);
  if ((l + d->t) > 80) { l = 80 - d->t;
                         fprintf(stderr, "termcap-concat : debordement !\n");
                       }
  for (i=d->t, j=0; j<l; i++, j++) d->c[i] = s[j];
  d->t += l;
}






/*****************************************************************/
/* Variable globale a ce module utilisee par decodpad() et out() */
/*****************************************************************/
static char *pout;


/*******************************************************/
/* Fonction de pseudo-ecriture d'un caractere utilisee */
/* pour tromper tputs() dans la fonction dcodpad()     */
/*                                                     */
/* Utilise la variable globale pout                    */
/*******************************************************/
static int out(c)
char c;
{ *(pout++) = c;
}



/*******************************************************/
/* Decodage du "padding" inclu dans une "capacite" seq */
/* et rangement dans rslt[] du resultat obtenu         */
/*                                                     */
/* Utilise la variable globale pout                    */
/*******************************************************/
decodpad(rslt, seq)
char * seq;
char *rslt;
{ pout = rslt;
  tputs(seq, 1, out);
  *pout = '\0';
}






/************************************************************************/
/* Cette fonction est destinee a etre appelee par le PASCAL             */
/*   - Elle renvoie 1 (true) si la capacite specifiee (commande) existe */
/*                  0 (false) dans le cas contraire                     */
/*   - La chaine donnee en argument est remplie par la chaine de codage */
/*     de la capacite (par une chaine vide si la capacite n'existe pas) */
/************************************************************************/
char termcap2(commande, chaine)
int commande;
struct ch80 *chaine;
{ char *x;
  char tempo[128];

  if (!termcap_deja_initialise) init_termcap(1);
  
  switch(commande)
     { case HOME :
              decodpad(tempo, home);
              copie(chaine, tempo);
              if (*home) return 1; else return 0;
       case DROITE :
              decodpad(tempo, right);
              copie(chaine, tempo);
              if (*right) return 1; else return 0;
       case GAUCHE :
              decodpad(tempo, left);
              copie(chaine, tempo);
              if (*left) return 1; else return 0;
       case HAUT :
              decodpad(tempo, up);
              copie(chaine, tempo);
              if (*up) return 1; else return 0;
       case BAS :
              decodpad(tempo, down);
              copie(chaine, tempo);
              if (*down) return 1; else return 0;
       case CUR_ALLUME :
              if (*ve) x = ve; else x = vs;
              decodpad(tempo, x);
              copie(chaine, tempo);
              if (*x) return 1; else return 0;
       case CUR_ETEINT :
              decodpad(tempo, vi);
              copie(chaine, tempo);
              if (*vi) return 1; else return 0;
       case INIT :
              if (*is) x = is; else x = ti;
              decodpad(tempo, x);
              copie(chaine, tempo);
              if (*is && *ti) { decodpad(tempo, ti);
                                concat(chaine, tempo);
                              }  
              if (*x) return 1; else return 0;
       case EFFACE_ECR :
              decodpad(tempo, cl);
              copie(chaine, tempo);
              if (*cl) return 1; else return 0;
       case FIN :
              decodpad(tempo, te);
              copie(chaine, tempo);
              if (*te) return 1; else return 0;
       case CLIGNOTE :
              decodpad(tempo, mb);
              copie(chaine, tempo);
              if (*mb) return 1; else return 0;
       case INVERSE :
              decodpad(tempo, mr);
              copie(chaine, tempo);
              if (*mr) return 1; else return 0;
       case SURINTENSITE :
              decodpad(tempo, md);
              copie(chaine, tempo);
              if (*md) return 1; else return 0;
       case NORMAL :
              decodpad(tempo, me);
              copie(chaine, tempo);
              if (*me) return 1; else return 0;
       case EFFACE_DLI :
              decodpad(tempo, cb);
              copie(chaine, tempo);
              if (*cb) return 1; else return 0;
       case EFFACE_FLI :
              decodpad(tempo, ce);
              copie(chaine, tempo);
              if (*ce) return 1; else return 0;
       case POUET :
              decodpad(tempo, bl);
              copie(chaine, tempo);
              if (*bl) return 1; else return 0;
       case GOTO :
              decodpad(tempo, cm);
              copie(chaine, tempo);
              if (*cm) return 1; else return 0;
       case TERMINAL :
              copie(chaine, term);
              return 1;
       default :
              fprintf(stderr, "termcap2 : commande %d inconnue\n", commande);
              return 0;
     }
}




