
/* Essai d'allocation memoire d'echange via ASDCALLOUE */
/*                     Y. Guillemot, le 31 octobre 2001 */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, k, a, s, v, n;
  int *tv;
  
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  short status, erreur, d[32];
   
  
  
  if (argc != 3) goto syntaxe;
  
  device = argv[1];
  
  nombre = strtol(argv[2], &p, 0);
  if (*p != '\0') goto syntaxe;
  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
    
  n = nombre;
  
  if (ioctl(poignee, ASDCALLOUE, &n))
    { perror("ioctl(ASDCALLOUE) ");
      exit(-1);
    }
  
  printf("Adresse bloc = 0x%04X\n", n); 
    
  
  
  /* Fin de l'essai */
  
  close(poignee);
  exit(0);
  
  
  
syntaxe :  
  fprintf(stderr, "Syntaxe : %s device nombre_mots\n", argv[0]);
  exit(-1);
  
  
}



