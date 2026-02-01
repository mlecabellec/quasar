/************************************************************************
 * File             : simaboe.c
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
  int i, k, a, v, n;
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
  
  nombre = strtol(argv[4], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" du nombre est anormale.\n", 
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
  t.f = SDC_RAZRAZ;

  if (ioctl(poignee, ASDCECR, &t) == -1)
    { perror("ioctl(ASDCECR -0-) ");
      fprintf(stderr, "Echec acces au RT%d,%d (%s)\n",
                      t.v.adresse, t.v.sous_adresse, 
                      t.v.direction ? "T" : "R");
      exit(-1);
    }


    

  t.v.adresse = adr;
  t.v.sous_adresse = sa;
  t.f = SDC_ATTENDRE;
  

  printf("--- Debut Boucle ---\n"); fflush(stdout);  
  k = 0; 
  
  for (;;)
     { int i, a_la_ligne;

       t.nbr = nombre;
       for (i=0; i<nombre; i++)
         { t.t[i] = k++;
         }

     
       if (ioctl(poignee, ASDCECR, &t) == -1)
         { perror("ioctl(ASDCECR) ");
           fprintf(stderr, "Echec acces au RT%d,%d (T)\n",
                           t.v.adresse, t.v.sous_adresse);
           exit(-1);
         }
         
        printf("------------ %d ---\n", k);
     } 
  
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);



syntaxe :
  fprintf(stderr, "\nSyntaxe : %s device adr sa nbr_mots\n\n", argv[0]);
  exit(-1);
  
  
}



