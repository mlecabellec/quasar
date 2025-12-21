#include <strings.h>
#include <stdlib.h>
#include "util.h"


struct ch80 { int  t;
              char c[80];
            };

int appel_shell(x)
struct ch80 *x;
{ char tampon[81];
  
  int i, r;
  if (x->t>80) x->t=80;  /* Ce n'est pas cense se produire..., */
                         /* mais sait-on jamais !              */
       
  /* Recopie chaine et mise en place du 0 final */
  for(i=0; i<x->t; i++) tampon[i] = x->c[i];
  tampon[i] = '\0';
  
  /* Lancement de la commande */
  system(tampon);
}
