/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*             TRANSMISSION DES COMMANDES AU TERMINAL                 */
/*                                                                    */
/*                                            Y.G., le 4/9/1992       */
/**********************************************************************/



#include <stdio.h>

#include "ptc.glob.h"








/*************************************************************/
/* Fonction utilisee pour ecrire les caracteres a la console */
/*************************************************************/
static int outc(c)
char c;
{ fputc(c, fich);
}




/*******************************************/
/* Transmission d'une commande au terminal */
/* (sauf deplacement absolu du curseur)    */
/*******************************************/
void terminal(commande)
int commande;
{
  if (!termcap_deja_initialise) init_termcap(1);
  
  switch(commande)
     { case HOME : 
              if(*home) tputs(home, (co > 0) ? co : 1, outc);
              return;
       case DROITE :
              if(*right) tputs(right, 1, outc);
              return;
       case GAUCHE :
              if(*left) tputs(left, 1, outc);
              return;
       case HAUT :
              if(*up) tputs(up, 1, outc);
              return;
       case BAS :
              if(*down) tputs(down, 1, outc);
              return;
       case CUR_ALLUME :
              if(*ve)         tputs(ve, 1, outc);
                else if(*vs)  tputs(vs, 1, outc);
              return;
       case CUR_ETEINT :
              if(*vi) tputs(vi, 1, outc);
              return;
       case INIT :
              if(*is) tputs(is, 1, outc);
              if(*ti) tputs(ti, 1, outc);
              return;
       case EFFACE_ECR :
              if(*cl) tputs(cl, 1, outc);
              return;
       case FIN :
              if(*te) tputs(te, 1, outc);
              return;
       case CLIGNOTE :
              if(*mb) tputs(mb, 1, outc);
              return;
       case INVERSE :
              if(*mr) tputs(mr, 1, outc);
              return;
       case SURINTENSITE :
              if(*md) tputs(md, 1, outc);
              return;
       case NORMAL :
              if(*me) tputs(me, 1, outc);
              return;
       case EFFACE_DLI :
              if(*cb) tputs(cb, 1, outc);
              return;
       case EFFACE_FLI :
              if(*ce) tputs(ce, 1, outc);
              return;
       case POUET :
              if(*bl) tputs(bl, 1, outc);
              return;
       default :
              fprintf("terminal : commande %d inconnue\n", commande);
              return;
     }
}





/*********************************/
/* Deplacement absolu du curseur */
/*********************************/
void curseur_goto(ligne, colonne)
int ligne, colonne;
{  
  if (!termcap_deja_initialise) init_termcap(1);
  
  if (*cm) tputs(tgoto(cm, colonne, ligne), 1, outc);
}  



