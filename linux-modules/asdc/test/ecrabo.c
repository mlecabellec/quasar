/************************************************************************
 * File             : ecrabo.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Ecriture d'un tampon dans d'un abonne 
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 17/05/02 creation                                       yg
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
  int i, a, n;
  int *tv;
  
  char *device;
  int adr, sa, sens, nombre;
  char *p;
  char tmp[80];
  
  struct asdctampon t; 
  
  int mode, nbt;  
  int d[32];
  
  if ((argc < 5) || (argc > 36)) 
    { goto syntaxe;
    }
  
  device = argv[1];
  
  adr = strtol(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de l'adresse est anormale.\n",
                       argv[2]);
      goto syntaxe;
    }
  
  sa = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de la sous-adresse est anormale.\n", 
                      argv[3]);
      goto syntaxe;
    }
    
  sens = RT_EMISSION;


  for (i=4; i<argc; i++)
     {
       d[i-4] = strtol(argv[i], &p, 0);
       if (*p != '\0')
         { fprintf(stderr, "Valeur \"%s\" de la donnee %d est anormale.\n",
                           argv[i], i-3);
           goto syntaxe;
         }
     } 
  n = argc - 4;

    
  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    



    

  t.v.adresse = adr;
  t.v.sous_adresse = sa;
  t.v.direction = sens;
  t.f = SDC_NONBLOQ;
  t.nbr = n;
  for (i=0; i<n; i++) t.t[i] = d[i];

  if (ioctl(poignee, ASDCECR, &t) == -1)
         { perror("ioctl(ASDCECR) ");
           exit(-1);
         }
         
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);



syntaxe :
  fprintf(stderr,
          "\nSyntaxe : %s device adr sa d1 [d2 d3 ...]\n\n", 
          argv[0]);
  exit(-1);
  
  
}



