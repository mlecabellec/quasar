/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*    RECHERCHE DES "CAPACITES" DISPONIBLES - version 1 argument      */
/*                                                                    */
/*                                            Y.G., le 4/9/1992       */
/**********************************************************************/



#include <stdio.h>

#include "ptc.glob.h"









/************************************************************************/
/* Cette fonction est destinee a etre appelee par le PASCAL             */
/*   - Elle renvoie 1 (true) si la capacite specifiee (commande) existe */
/*                  0 (false) dans le cas contraire                     */
/************************************************************************/
char termcap1(commande)
int commande;
{ char *x;

  if (!termcap_deja_initialise) init_termcap(1);
  
  switch(commande)
     { case HOME :
              if (*home) return 1; else return 0;
       case DROITE :
              if (*right) return 1; else return 0;
       case GAUCHE :
              if (*left) return 1; else return 0;
       case HAUT :
              if (*up) return 1; else return 0;
       case BAS :
              if (*down) return 1; else return 0;
       case CUR_ALLUME :
              if (*x) return 1; else return 0;
       case CUR_ETEINT :
              if (*vi) return 1; else return 0;
       case INIT :
              if (*is) x = is; else x = ti;
              if (*x) return 1; else return 0;
       case EFFACE_ECR :
              if (*cl) return 1; else return 0;
       case FIN :
              if (*te) return 1; else return 0;
       case CLIGNOTE :
              if (*mb) return 1; else return 0;
       case INVERSE :
              if (*mr) return 1; else return 0;
       case SURINTENSITE :
              if (*md) return 1; else return 0;
       case NORMAL :
              if (*me) return 1; else return 0;
       case EFFACE_DLI :
              if (*cb) return 1; else return 0;
       case EFFACE_FLI :
              if (*ce) return 1; else return 0;
       case POUET :
              if (*bl) return 1; else return 0;
       case GOTO :
              if (*cm) return 1; else return 0;
       case TERMINAL :
              return 1;
       default :
              fprintf(stderr, "termcap1 : commande %d inconnue\n", commande);
              return 0;
     }
}




