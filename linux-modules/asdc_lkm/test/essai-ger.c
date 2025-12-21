/************************************************************************
 *
 *   Pour essai"nouveau" coupleur ABI-PMC2-2 avec Pbs sur voie 2
 *
 */

/* Emission repetitive d'un "transfert individuel" par la carte ABI */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "asdcctl.h"
#include "ln1.h"
 
 
 
main(int argc, char **argv)
{

  int i, a, v, n;
  int *tv;
  
  int poignee;
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int erreur;
   
   
   if (argc != 2)
     { fprintf(stderr, "Syntaxe :   %s device\n", argv[0]);
       exit(-1);
     }


   device = argv[1];
   
   poignee = open(device, O_RDWR);
   if (poignee == -1)
     {
       perror("open ");
       exit(-1);
     } 


  
  
  /*********************************/
  /*   Declaration de la "trame"   */
  /*********************************/
    
  b.type = BC_BCRT;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  // b.options = 2;	/* Pas d'arret si erreur, mais IT erreur validee */
  b.options = 3;	/* Pas d'arret si erreur, IT erreur inhibee */
  b.chainage = 0;
  b.bus = 0;
  b.adresse = 5;
  b.sous_adresse = 7;
  b.nbmots = 1;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
  debut_trame = b.adrbbc;
 
  
  
  t.i = b.adrtamp;
  t.n = 1;
  t.d[0] = 0xcafe;
  
  
  
 
  /**************************************/
  /* Execution repetitive de la "trame" */
  /**************************************/
  
  for (i=0; i<100; i++)
    {
      if (ioctl(poignee, ASDCGOTBC, &debut_trame))
       { perror("ioctl(ASDCGOTBC) ");
         exit(-1);
       }
  
    
     /* Attente fin */
     // printf("Attente fin trame (ASDCFINTBC)\n"); fflush(stdout);
     if (ioctl(poignee, ASDCAFINTBC, 0))
       { perror("ioctl(ASDCAFINTBC) ");
         exit(-1);
       }
     // printf("Trame achevee !\n"); fflush(stdout);
     
    }
  
  
  
  
  
  /* Menage */
  
  if (ioctl(poignee, ASDCSUPTBC, &debut_trame))
    { perror("ioctl(ASDCSUPTBC) ");
      exit(-1);
    }
  
}



