/*******************************************************************/
/*   MONABI : Logiciel d'espionnage 1553 utilisant une carte ABI   */
/*                                                                 */
/*      Affichage "en clair" du fichier brut fourni par MONABI     */
/*      ------------------------------------------------------     */
/*                                                                 */
/*         Gestion de l'impression sur la sortie standard          */
/*            (impression conditionnelle selon erreurs             */
/*                rencontrees et options specifiees)               */
/*                                                                 */
/*                       Anonymized (EXAM v2.5), le 20/05/1997   */
/*                             modif. (EXAM v2.6), le 21/05/1997   */
/*******************************************************************/

#include <stdio.h>
#include "exam.h"

#define NB_MAX_LIGNES	100
	/* Remarque les "Lignes" dont il est question ci-dessus sont     */
	/* en fait des chaines de caracteres (ou "unites d'impression")  */
	/* pas forcement achevees par \n et qui peuvent donc etre        */
	/* relativement nombreuses.                                      */

static char *tampon1;
static char *tampon2;
static int nbc1, nbc2;	      /* Nbre car. utilises ds tampon1 et tampon2 */
static int num_tmp;	      /* Numero tampon couramment associe a tmp */

/* Allocation memoire : a appeler une fois, en phase d'initialisation */
void allocation()
{ int i;

  tampon1 = (char *) calloc(T_MAX_TMP_S, sizeof(char));
  if (tampon1 == NULL) goto erreur_alloc;
  *tampon1 = '\0';

  tampon2 = (char *) calloc(T_MAX_TMP_S, sizeof(char));
  if (tampon2 == NULL) goto erreur_alloc;
  *tampon2 = '\0';
     
  nbc1 = nbc2 = 0;
  
  num_tmp = 1;
  tmp = tampon1;
  pnctmp = &nbc1;
     
  return;
  
  
erreur_alloc :
  fprintf(stderr, "Impossible allouer memoire !\n");
  exit(-1);
}   


/* Impression d'un tampon */
impression(tampon, nbc)
char *tampon;
int nbc;
{ 
  if (!nbc) return;
  
  strcpy(tampon + nbc, "\n");
  fwrite (tampon, 1, nbc + 1, stdout);
}


/* Changement de tampon : a appeler quand fin de decodage d'un message */
bascule()
{
  switch(num_tmp)
    { case 1 :  nbc2 = 0;
                tmp = tampon2;
                pnctmp = &nbc2;
                num_tmp = 2;
                break;
                
      case 2 :  nbc1 = 0;
                tmp = tampon1;
                pnctmp = &nbc1;
                num_tmp = 1;
                break;
                
      default : fprintf(stderr, "Erreur interne : num_tmp=%d\n", num_tmp);
                fflush(stderr);
                abort();
    }
}




/*
** Vidage des tampons : a appeler quand condition adequate rencontree

*/
vidage_avant_dernier()
{ int i;

  switch(num_tmp)
    { case 1 :  impression(tampon2, nbc2);
                nbc2 = 0;
                break;
                
      case 2 :  impression(tampon1, nbc1);
                nbc1 = 0;
                break;
                
      default : fprintf(stderr, "Erreur interne : num_tmp=%d\n", num_tmp);
                fflush(stderr);
                abort();
    }
}


vidage_dernier()
{ int i;

  switch(num_tmp)
    { case 1 :  impression(tampon1, nbc1);
                nbc1 = 0;
                tmp = tampon1;
                break;
                
      case 2 :  impression(tampon2, nbc2);
                nbc2 = 0;
                tmp = tampon2;
                break;
                
      default : fprintf(stderr, "Erreur interne : num_tmp=%d\n", num_tmp);
                fflush(stderr);
                abort();
    }
}
