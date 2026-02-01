/************************************************************************
 * File             : affit11.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Affichage liste des transferts tampons extremes pour debug fluxs BC
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 13/11/01 creation                                       yg
 *       2 14/11/01 adaptation a l'enregistrement de tous les 
 *                  transferts                                     yg
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
  int i, n;
  int *dn, *dp1, *dp2, *di1, *di2;
  
  char *device;
  
  fprintf(stderr, "--- Phase 1 ---\n"); fflush(stderr);
  
  
  if (argc != 2) 
    { fprintf(stderr, "Syntaxe : %s device\n", argv[0]);
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 2 ---\n"); fflush(stderr);
    
  device = argv[1];
  

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
  
  fprintf(stderr, "--- Phase 3 ---\n"); fflush(stderr);
    
    
  /* Lecture taille tables */
  if (ioctl(poignee, FLUX_IIDBG, &n))
    { perror("ioctl(FLUX_IIDBG) ");
      exit(-1);
    }
    
  fprintf(stderr, "N = %d\n", n); fflush(stderr);
    
  /* Allocation memoire */
  dn = (int *) calloc(n, sizeof(int));
  dp1 = (int *) calloc(n, sizeof(int));
//  dp2 = (int *) calloc(n, sizeof(int));
  di1 = (int *) calloc(n, sizeof(int));
//  di2 = (int *) calloc(n, sizeof(int));
  
  fprintf(stderr, "--- Phase 4 ---\n"); fflush(stderr);
  
  if ((dn==NULL) || (dp1==NULL) || (dp2==NULL) || (di1==NULL) || (di2==NULL))
    { fprintf(stderr, "Impossible d'allouer memoire !\n");
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 5 ---\n"); fflush(stderr);
    
    
    
  /* lecture table des numeros d'appel ioctl */  
  if (ioctl(poignee, FLUX_IDBG_N, dn))
    { perror("ioctl(FLUX_IDBG_N) ");
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 6 ---\n"); fflush(stderr);
    
  /* lecture table des adresses tampons */  
  if (ioctl(poignee, FLUX_IDBG_P1, dp1))
    { perror("ioctl(FLUX_IDBG_P1) ");
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 7 ---\n"); fflush(stderr);
    
  /* lecture table des adresses derniers tampons  
  if (ioctl(poignee, FLUX_IDBG_P2, dp2))
    { perror("ioctl(FLUX_IDBG_P2) ");
      exit(-1);
    }
  */
   
  fprintf(stderr, "--- Phase 8 ---\n"); fflush(stderr);
  
    
  /* lecture table des numeros IT BC des tampons */  
  if (ioctl(poignee, FLUX_IDBG_I1, di1))
    { perror("ioctl(FLUX_IDBG_I1) ");
      exit(-1);
    }
  
  
  fprintf(stderr, "--- Phase 9 ---\n"); fflush(stderr);
    
  /* lecture table des numeros IT derniers tampons   
  if (ioctl(poignee, FLUX_IDBG_I2, di2))
    { perror("ioctl(FLUX_IDBG_I2) ");
      exit(-1);
    }
  */
  
  fprintf(stderr, "--- Phase 10 ---\n"); fflush(stderr);
    
    
  /* Affichage des tables */
  
//  for (i=0; i<n; i++)
//    { printf("%4d  %4d  %4d-%08X  %4d-%08X\n",
//              i, dn[i], di1[i], dp1[i], di2[i], dp2[i]);
//    }
    
  for (i=0; i<n; i++)
    { printf("%4d  %4d  %4d  %08X\n",
              i, dn[i], di1[i], dp1[i]);
    }
 
}
