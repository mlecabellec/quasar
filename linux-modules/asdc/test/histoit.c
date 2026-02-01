/*******************************************************************/
/*                                                                 */
/*     AFFICHAGE DE L'HISTOGRAMME DU NOMBRE D'IT DANS LA TABLE     */
/*                                                                 */
/*                                                                 */
/*                             Anonymized, le 27 novembre 2001   */
/*                  Mise en conformite gcc v4 le 15 fevrier 2008   */
/*                           derniere modif. le                    */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "asdcctl.h"
#include "ln1.h"



int file_desc;



main(argc,argv)
int argc;
char *argv[];
{
  struct asdchistoit h;
  int fd;
  
  int i, j, k, m, l, n, o;

  if ((argc!=2) && (argc!=3)) goto syntaxe;
                 

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
 
  if ((argc==3) && !strcmp(argv[2], "RAZ")) 
    { if (ioctl(file_desc,  ASDCRAZHISTO, 0))
        { perror("ioctl(ASDCRAZHISTO) ");
          exit(-1);
        }  
        
      printf("RAZ histogramme des ITs effectue !\n");
      exit(0);  
    }     
    
    
    if (ioctl(file_desc,  ASDCLECHISTO, &h))
        { perror("ioctl(ASDCLECHISTO) ");
          exit(-1);
        }  
        
    
    
    
  
  n =  LRAM(file_desc, IQNUM);
  
  printf("Taille du tampon des ITs = %d\n", n);
  
  printf("Nombre d'ITs physiques enregistrees = %d\n\n", h.nombre);

/* 
  if (n <= (ASDCTHISTOIT-1))
    { for (i=1; i<=n; i++)
        { printf("  %4d  %5d\n", i, h.histo[i]);
        }
      printf(" >%4d  %5d\n", n, h.histo[0]);
    }
  
  
  if (n > (ASDCTHISTOIT-1))
    { for (i=1; i<ASDCTHISTOIT; i++)
        { printf("  %4d  %5d\n", i, h.histo[i]);
        }
      printf(" >%4d  %5d\n", ASDCTHISTOIT, h.histo[0]);
    }
*/

    { for (i=0; i<ASDCTHISTOIT; i++)
        { printf("  %4d  %5d\n", i, h.histo[i]);
        }
      printf("Debordement : %d\n", h.deborde);
    }


  printf("\n\n");   
     
  close(file_desc);
  exit(0);
  
  
syntaxe :
  fprintf(stderr, "\nSyntaxe :   %s  periph. [RAZ]\n", argv[0]);
  exit(-1);
  
} 



