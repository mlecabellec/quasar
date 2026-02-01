/************************************************************************
 * File             : initabi.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 21/11/01 creation                                       yg
 *
 *
 */

/* Autotest du coupleur */

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
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  int csr;
  
  struct asdcparam par;   
  
  
  if (argc != 2) 
    { fprintf(stderr, "Syntaxe : %s device\n", argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
  

  /***   Chargement du firmware   ***/
    
  /* Controle acces a la memoire (utile ?) */
  ERAM(poignee, 0xFF, 0x1234);
  if (LRAM(poignee, 0xFF) != 0x1234)
    { printf(" Impossible d'acceder a la memoire d'echange ...\n");
      exit(2);
    }
            
  /* Chargement du firmware depuis la flash */
  ERAM(poignee, CSR, 1);
  
  /* Attente 1 s (???) */
  sleep(1);
    
  ERAM(poignee, PCODE, 0xFFFF);
  ERAM(poignee, CSR, 0);
  
  /* Attente 1 s (???) */
  sleep(1);
    
  /***   Debut des autotests   ***/  
    
  /* 0 --> BIT Total Error Count */
  ERAM(poignee, 0x3C, 0);
    
  /* 0x48 --> Selected Test */
  ERAM(poignee, TEST_SELECT_REG, 0x48);
    
  /* 0x2 --> BIT Control */
  ERAM(poignee, 0x3A, 0x2);
    
  /* Attente 0 dans BIT Control */
  for (i=0; ; i++)
     { a = LRAM(poignee, 0x3A);
       if (!a) break;
       /* printf("%04X ", a & 0xFFFF); fflush(stdout); */
       if (i > 20) 
         { fprintf(stderr, "Time-Out : Echec de l'autotest !!!\n");
           exit(-1);
         }
       sleep(1);
     }
  printf("\n");
   
  /* Si OK, BIT Total Error Count doit etre 0 */
  v = LRAM(poignee, 0x3C);  
  if (!v) 
    { printf(" ---   Tests OK   ---\n");
      exit(0);
    }
    
  /* Si pas OK, Affichage des erreurs : */
  printf("\n");
  printf("   Nombre d'erreurs du test RAM = %d\n", LRAM(poignee, RAM_ERR));
  printf("   Nombre d'erreurs du test codeur/decodeur = %d\n", 
                                               LRAM(poignee, ENCODER_ERR));
                                               
  exit(1);
  
      
  close(poignee);
  exit(0);
  
  
}



