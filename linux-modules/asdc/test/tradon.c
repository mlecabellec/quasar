/************************************************************************
 * File             : tradon.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Traduction des donnees brutes (binaires) issues d'un flux et stockees
 * dans un fichier (pour experimentation uniquement)
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  3/11/01 creation                                       yg
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
  int i, j, a, v, n;
  int *tv;
  
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  struct asdcbc_tf tampon;
 
  
  
  if (argc != 1) 
    { fprintf(stderr, "Syntaxe : %s < fichier_e > fichier_s\n", argv[0]);
      exit(-1);
    }
  


  
  /* Relecture des donnees  */
  
  for (i=0; ; i++)
    {
      fread(&tampon, sizeof(struct asdcbc_tf), 1, stdin);
      
      if (feof(stdin)) break;	/* Fin du fichier ? */
  
      printf("%3d - ", i);
      if ((tampon.err & 0xFFFF) == 0xFFFF)
        { printf("   *** TAMPONS PERDUS ! ***\n");
        }
      else if (tampon.type == 0x5A5A)
        { printf("   =========================================\n");
          i--;
        }
      else
        {
        printf("%3d - ", *((int *) &tampon.date[0]));
          switch(tampon.type & 0xFF)
             { case BC_BCRT : printf("BCRT "); break;
               case BC_RTBC : printf("RTBC "); break;
               default : printf("\"Type 0x%04X inattendu\" ",
                                            tampon.type & 0xFFFF);
             }
          printf("RT%d,%d ", 
                  (tampon.cmd1 >> 11) & 0x1F, (tampon.cmd1 >> 5) & 0x1F);
          printf("n=%d ", tampon.cmd1 & 0x1F);
          printf("err=0x%04X ", tampon.err & 0xFFFF);
          printf("sts=0x%04X \n", tampon.sts1 & 0xFFFF);
          if (    (tampon.type == BC_RTBC)
               || (tampon.type == BC_BCRT)
               || (tampon.type == BC_BCRT_DI))
            { for (j=0; j<(tampon.cmd1 & 0x1F); j++)
                                    printf(" %04X", tampon.d[j] & 0xFFFF);
              printf("\n");
            }
        }
      printf("\n");
    }
    
          
      
  
  printf("\nC'est fini !\n");

      
  exit(0);
  
  
}



