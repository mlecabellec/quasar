/************************************************************************
 * File             : labi.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *
 *
 */

/* Lecture de la memoire image de la carte ABI */

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
  
  char *device;
  int adresse, nombre;
  char *p;
  
  nombre = 1;
  
  switch(argc)
    { case 4 : nombre = strtol(argv[3], &p, 0);
               if (*p != '\0') goto syntaxe;
      case 3 : adresse = strtol(argv[2], &p, 0);
               if (*p != '\0') goto syntaxe;
               break;
               
      default : goto syntaxe;
    }
    
  device = argv[1];
    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
 
  
  /* Dump de nombre mots a partir de adresse */
  for (i=0, n=0, a=adresse; n<nombre; a++, n++)
     { if (!i) printf("\n%08X : ", a);

       v = LIMAH(poignee, a);
       printf(" %08X", v);
       
       if (++i == 8) i=0;
     }
   
  printf("\n");  
      
  close(poignee);
  exit(0);
  
  
syntaxe : 
  fprintf(stderr, "Syntaxe : %s device adresse [nombre]\n", argv[0]);
  exit(-1);

}



