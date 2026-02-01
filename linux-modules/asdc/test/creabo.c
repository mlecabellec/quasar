/************************************************************************
 * File             : simabol.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Creation d'un abonne simule (ou, plutot, d'une voie)
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 12/09/02 creation                                       yg
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
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, n;
  int *tv;
  
  char *device;
  int adr, sa, sens, nombre;
  char *p;
  char tmp[80];
  
  struct asdcvoie v;  
  
  int cevt; 
  
  
  if ((argc != 5) && (argc != 6))
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

  cevt = 0;
  if (argc == 6)
    { 
      cevt = strtol(argv[5], &p, 0);
      if (*p != '\0')
        { fprintf(stderr, "Valeur \"%s\" du numero de CEVT est anormale.\n",
                      argv[0], argv[5]);
          goto syntaxe;
        }
    
    }

  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    


  v.adresse = adr;
  v.sous_adresse = sa;
  v.direction = sens;
  v.adrtamp = 0;
  v.ntamp = 5;		/* 5 tampons */
  v.mode = 0;		/* Voie synchrone */

  if (ioctl(poignee, ASDCDEF, &v) == -1)
    { perror("ioctl(ASDCDEF) ");
      fprintf(stderr, "Echec acces au RT%d,%d (%s)\n",
                      v.adresse, v.sous_adresse, 
                      v.direction ? "T" : "R");
      exit(-1);
    }


  /* Connexion eventuelle a un CEVT */
  if (cevt)
    {
      v.adresse = adr;
      v.sous_adresse = sa;
      v.direction = sens;
      v.adrtamp = cevt;
      
      if (ioctl(poignee, ASDCEVT_ABO_AJOUTER, &v) == -1)
        { perror("ioctl(ASDCEVT_ABO_AJOUTER) ");
          fprintf(stderr, "Echec connexion RT%d,%d (%s) au CEVT%d\n",
                           v.adresse, v.sous_adresse, 
                           v.direction ? "T" : "R",
                           cevt);
          exit(-1);
        }
      
    
    }
    
    
  /* Validation de l'abonne cree */
  v.adresse = adr;
  v.nmots = RT_ABONNE;		/* Validation de l'abonne */

  if (ioctl(poignee, ASDCMODE, &v) == -1)
    {
      perror("ioctl(ASDCMODE) ");
      fprintf(stderr, "Echec validation du RT%d\n", v.adresse);
      exit(-1);
    }
  
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);



syntaxe :
  fprintf(stderr, "\nSyntaxe : %s device adr sa {R|T} [num_CEVT]\n\n", argv[0]);
  exit(-1);
  
  
}



