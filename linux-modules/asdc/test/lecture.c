/************************************************************************
 * File             : lecture.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 15/05/02 creation                                       yg
 *
 *
 */

/* Emission repetitive de "transferts individuels" en transmission 
 * par la carte ABI (mode gerant) et affichage des donnees recues
 */


#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"
#include "ln1-bc.h"
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, j, k, a, s, v, n;
  int *tv;
  
  char *device;
  int adr, sa, nbre, nbcycles;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  short status, erreur, d[32];
   
  
  
  if (argc != 6) goto syntaxe;
  
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
  
  nbre = strtol(argv[4], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" du nombre est anormale.\n", 
                      argv[0], argv[4]);
      goto syntaxe;
    }

  
  nbcycles = strtol(argv[5], &p, 0);
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
    
    
  /* Demarrage firmware */
  
  /*** Ici, il faudrait une fonction LN1 pour pouvoir affirmer ***/
  /*** que le coupleur est OK (firmware demarre, etc...)       ***/
    
  
  
  /**************************************************************/
  /*   Declaration et execution d'une trame "pilotee par l'UC"  */
  /**************************************************************/
  
#define ERR (erreur ? "ERR" : "   ")


  
  for (j=0; (!nbcycles) || (j < nbcycles); j++)
     { 
       /* Vidage du tampon */
       for(i=0; i<32; i++) d[i] = 0;

       transfert(poignee, BC_RTBC, adr, sa, nbre, &status, &erreur, d);
       printf("%s RT%d,%d (n=%d) <-- S=%04X", ERR, adr, sa, nbre, status & 0xFFFF);
       if (erreur) printf("\t Erreur=0x%04X", erreur);
       printf("\n");

       for (i=0; i<nbre; i++)
         { printf("%04X ", d[i] & 0xFFFF);
         }
       printf("\n");


       usleep(10000);	/* Attente de 10 ms */
     }

  
  
  /* Fin de l'essai */
  
  
  printf("C'est fini !\n");    
  
  close(poignee);
  exit(0);


syntaxe :
 fprintf(stderr, "Syntaxe : %s device adr sa nbre nbcycles\n", argv[0]);
 exit(-1);
}



