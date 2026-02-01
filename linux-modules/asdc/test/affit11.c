/************************************************************************
 * File             : affit11.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Affichage liste des appels IT code 0x11 pour debug fluxs BC
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 13/11/01 creation                                       yg
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
  int *di, *dc, *dp;
  
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
  if (ioctl(poignee, FLUX_IDBG, &n))
    { perror("ioctl(FLUX_IDBG) ");
      exit(-1);
    }
    
  fprintf(stderr, "N = %d\n", n); fflush(stderr);
    
  /* Allocation memoire */
  di = (int *) calloc(n, sizeof(int));
  dc = (int *) calloc(n, sizeof(int));
  dp = (int *) calloc(n, sizeof(int));
  
  fprintf(stderr, "--- Phase 4 ---\n"); fflush(stderr);
  
  if ((di==NULL) || (dc==NULL) || (dp==NULL))
    { fprintf(stderr, "Impossible d'allouer memoire !\n");
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 5 ---\n"); fflush(stderr);
    
    
  /* lecture table des compteurs BC */  
  if (ioctl(poignee, FLUX_DBG_I, di))
    { perror("ioctl(FLUX_DBG_I) ");
      exit(-1);
    }
  
  fprintf(stderr, "--- Phase 6 ---\n"); fflush(stderr);
    
  /* lecture table des commandes BC */  
  if (ioctl(poignee, FLUX_DBG_C, dc))
    { perror("ioctl(FLUX_DBG_C) ");
      exit(-1);
    }
    
  /* lecture table des adresses tampons */  
  if (ioctl(poignee, FLUX_DBG_P, dp))
    { perror("ioctl(FLUX_DBG_P) ");
      exit(-1);
    }
    
    
  /* Affichage des tables */
  for (i=0; i<n; i++)
    { printf("%4d  %4d  %04X  %08X\n",
              i, di[i], dc[i] & 0xFFFF, dp[i]);
    }
 
}
