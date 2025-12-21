/************************************************************************
 * File             : etrame0.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  11/04/01 creation                                       yg
 *       2   7/11/01 Utilisation symbole IMERR pour acces a erreur  yg
 *       3  14/12/01 Modif. pour traquer un retard a la mise a 0  
 *                   de BCCPTR par le firmware                      yg
 *
 *       4  14/12/01 Suppresion de la modif. ci-dessus              yg
 *
 */

/* Emission d'un "transfert individuel" par la carte ABI */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "asdcctl.h"
#include "ln1.h"
 
 
 
void transfert(int poignee, int type, int adr, int sa, int nbre, 
                            short *pstatus, short *perr, short *donnees)
{

  int i, a, v, n;
  int *tv;
  
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int erreur;
   
  
  
  /*********************************/
  /*   Declaration de la "trame"   */
  /*********************************/
    
  b.type = type;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur, mais IT erreur validee */
  b.chainage = 0;
  b.bus = 0;
  b.adresse = adr;
  b.sous_adresse = sa;
  b.nbmots = nbre;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
  debut_trame = b.adrbbc;
 
  
  
  if (type == BC_BCRT)    
    { 
      t.i = b.adrtamp;
      t.n = nbre;
      for (i=0; i<nbre; i++) t.d[i] = donnees[i];
  
      if (ioctl(poignee, ASDCETAMPBC, &t))
        { perror("ioctl(ASDCETAMPBC) ");
          exit(-1);
        }
    }
  
  
  
 
  /* Execution de la "trame" */
  
   if (ioctl(poignee, ASDCGOTBC, &debut_trame))
    { perror("ioctl(ASDCGOTBC) ");
      exit(-1);
    }
  
  /* Pour traquer retard a la mise a 0 de BCCPTR ...
  { int cmpt = 0;
    while (ioctl(poignee, ASDCGOTBC, &debut_trame))
         { cmpt++;
           if (errno != EBUSY)
             { perror("ioctl(ASDCGOTBC) ");
               exit(-1);
             }
           if (cmpt > 10)
             { fprintf(stderr, "ASDCGOTBC : Echec avec cmpt = %d\n", cmpt);
               exit(-1);
             }
         }
     printf("cmpt=%d\n", cmpt);
  }   
  */  
    
  /* Attente fin */
  // printf("Attente fin trame (ASDCFINTBC)\n"); fflush(stdout);
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
  // printf("Trame achevee !\n"); fflush(stdout);
  
  
  
  /* Recuperation des resultats */
  
  *pstatus = LRAM(poignee, debut_trame + 3);
  erreur = LIMA(poignee, debut_trame + IMERR);
  *perr = erreur;
  
  
  if (type == BC_RTBC)    
    { 
      t.i = b.adrtamp;
      t.n = nbre;
  
      if (ioctl(poignee, ASDCLTAMPBC, &t))
        { perror("ioctl(ASDCETAMPBC) ");
          exit(-1);
        }
        
      for (i=0; i<nbre; i++)  donnees[i] = t.d[i];
    }
  
  
  /* Menage */
  
  if (ioctl(poignee, ASDCSUPTBC, &debut_trame))
    { perror("ioctl(ASDCSUPTBC) ");
      exit(-1);
    }
  
}



