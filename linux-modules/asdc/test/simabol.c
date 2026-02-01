/************************************************************************
 * File             : simabol.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Essai de simulation d'un abonne : affichage des tampons d'une voie
 * a la reception de chaque message.
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 14/05/02 creation                                       yg
 *
 *
 */


#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"
#include "ln1.h"
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, v, n;
  int *tv;
  
  char *device;
  int adr, sa, sens, nombre;
  char *p;
  char tmp[80];
  
  struct asdctampon t;   
  
  
  if (argc != 5) 
    { goto syntaxe;
    }
  
  device = argv[1];
  
  adr = strtol(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de l'adresse est anormale.\n",
                      argv[0], argv[2]);
      goto syntaxe;
    }
  
  sa = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de la sous-adresse est anormale.\n", 
                      argv[0], argv[3]);
      goto syntaxe;
    }
    
  if      (!strcmp(argv[4], "R"))
    { sens = RT_RECEPTION;
    }
  else if (!strcmp(argv[4], "T"))
    { sens = RT_EMISSION;
    }
  else
    { fprintf(stderr, "Valeur \"%s\" du sens des transferts est anormale.\n", 
                      argv[0], argv[4]);
      goto syntaxe;
    }

  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    


  t.v.adresse = adr;
  t.v.sous_adresse = sa;
  t.v.direction = sens;
  t.f = SDC_RAZ;

  if (ioctl(poignee, ASDCLEC, &t) == -1)
    { perror("ioctl(ASDCLEC -0-) ");
      fprintf(stderr, "Echec acces au RT%d,%d (%s)\n",
                      t.v.adresse, t.v.sous_adresse, 
                      t.v.direction ? "T" : "R");
      exit(-1);
    }








    

  t.v.adresse = adr;
  t.v.sous_adresse = sa;
  t.v.direction = sens;
  t.f = SDC_ATTENDRE;
  

  printf("--- Debut attente ---\n"); fflush(stdout);   
  
  for (;;)
     { int i, a_la_ligne;
     
       /* Boucle de simulation */
       if (ioctl(poignee, ASDCLEC, &t) == -1)
         { perror("ioctl(ASDCLEC) ");
           fprintf(stderr, "Echec acces au RT%d,%d (%s)\n",
                           t.v.adresse, t.v.sous_adresse, 
                           t.v.direction ? "T" : "R");
           exit(-1);
         }
         
       printf("RT%d,%d (%s) : n=%d\n", adr, sa, argv[4], t.nbr);
       for (i=0; i<t.nbr; i++)
         { printf(" %04X", t.t[i]);
           if (((i+1) % 8) == 0)
             { printf("\n");
               a_la_ligne = 1;
             }
           else
             {
               a_la_ligne = 0;
             }
         }
         
       if (a_la_ligne == 0) printf("\n");
        printf("\n");
     } 
  
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);



syntaxe :
  fprintf(stderr, "\nSyntaxe : %s device adr sa {R|T}\n\n", argv[0]);
  exit(-1);
  
  
}



