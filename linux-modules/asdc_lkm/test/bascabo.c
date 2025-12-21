/************************************************************************
 * File             : bascabo.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Tache attendant un message sur une voie 1553 (devant avoir ete definie 
 * au prealable) et basculant l'abonne associe a la voie du mode abonne
 * au mode espion temps-reel et inversement a chaque reception d'un message.
 * 
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 29/11/01 creation                                        yg
 *
 *
 */
 
 
 
/* 
   Ce logiciel a besoin, en entree (sur la ligne de commande) :
   
     - Du "device" associe au coupleur ABI
     - De l'adresse de l'abonne converne
     - De la sous-adresse de la voie concernee
     - Du "sens" ('E' ou 'R') de la voie concernee
      
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
  int i, j, a, v, n;
  
  char *device;
  int adr, sa, sens;
  char *p;
  char tmp[80];
  

  
  
  
  /* Recuperation des parametres */
  
  if (argc != 5) 
    { fprintf(stderr, 
              "Syntaxe : %s device adresse sous_adresse {E|R}\n", 
              argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  
  adr = strtol(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de l'adresse est anormale.\n", argv[2]);
      exit(-1);
    }
  if ((adresse < 0) || (adresse > 31))
    { fprintf(stderr, "Adresse %d est hors bornes\n", adr);
      exit(-1);
    }
  
  sa = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de la sous-adresse est anormale.\n", 
              argv[3]);
      exit(-1);
    }
  if ((sa < 0) || (sa > 31))
    { fprintf(stderr, "Sous-adresse %d est hors bornes\n", sa);
      exit(-1);
    }

  
  if      (!strcmp(argv[4], "E")) 
    { sens = 1;
    }
  else if (!strcmp(argv[4], "R"))
    { sens = 0;
    }
  else
    { fprintf(stderr, "Sens \"%s\" est anormal\n", argv[4]);
      exit(-1);
    }
  
  
  
  
  
  
  /* Ouverture du peripherique */
  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    




   for (;;)
      {
         /* Attente d'une message */
         
         
         /* Passage de l'abonne en mode "espion temps reel" */
         
         
         /* Attente d'une message */
         
         
         /* Passage de l'abonne en mode "abonne simule" */
         
         
      }

}



