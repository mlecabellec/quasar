/************************************************************************
 * File             : labi.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *
 */

/* Ecriture dans la memoire image du driver de la carte ABI */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ln1.h"
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, v, n;
  int *tv;
  
  char *device;
  int adresse, nombre;
  char *p;
  
  nombre = 1;
  
  if (argc < 4) goto syntaxe;
  
  device = argv[1];
  
  adresse = strtol(argv[2], &p, 0);
  if (*p != '\0') goto syntaxe; 
  
  /* Allocation de la table des valeurs a ecrire */
  tv = (int *) calloc(argc - 3, sizeof(int));
  if (tv == NULL)
    { fprintf(stderr, "Impossible d'allouer memoire !\n");
      exit(-1);
    }
  
  /* Conversion en binaire des valeurs a ecrire */
  for (i=0; i<(argc-3); i++)
     { tv[i] = strtol(argv[i+3], &p, 0);
               if (*p != '\0') goto syntaxe;
     }
    
    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
 
  
  /* Ecriture des valeurs dans la carte */
  for (i=0; i<(argc-3); i++)
     { EIMAL(poignee, adresse++, tv[i]);
     }
      
  close(poignee);
  exit(0);
  
  
syntaxe : 
  fprintf(stderr, "Syntaxe : %s device adresse val1 [val2 val3 ...]\n",
                                                                  argv[0]);
  exit(-1);

}



