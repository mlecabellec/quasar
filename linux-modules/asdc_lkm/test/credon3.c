/************************************************************************
 * File             : credon3.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Creation d'un fichier de donnees d'entree 
 * en vue d'une utilisation avec eflux3
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  2/11/01 creation                                       yg
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
  
  char *fichier;
  char *p, *nom;
  FILE *f;
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int nombre;
  
  struct asdcbcflux_etat e;
  
  struct asdcbc_tf tamp_s[1000];
  struct asdcbcflux flx;
  
  
  if (argc != 3) 
    { fprintf(stderr, "Syntaxe : %s nombre_cycles fichier_a_creer\n", argv[0]);
      exit(-1);
    }
  
  nombre = strtol(argv[1], &p, 0);
  if (*p != '\0')
    { fprintf(stderr,
              "Valeur \"%s\" du nombre de tampons est anormale.\n", 
              argv[0]);
      exit(-1);
    }

  fichier = argv[2];


    
  f = fopen(fichier, "w+");
  if (f == NULL)
    {
      fprintf(stderr, "Echec creation/ouverture de \"%s\"\n", fichier);
      exit(-1);
    } 
    
    
  
 
  
  
  /* Programmation des donnees en sortie du flux 2 */
  
  j = 0;
  for (i=0; i<nombre; i++)
    { tamp_s[0].type = BC_BCRT;
      tamp_s[0].cmd1 = (9 << 11) + (5 << 5) + 2;	/* RT9,5 n=2 */
      tamp_s[0].d[0] = j++;
      tamp_s[0].d[1] = j++;
      
      tamp_s[1].type = BC_BCRT;
      tamp_s[1].cmd1 = (9 << 11) + (6 << 5) + 3;	/* RT9,6 n=3 */
      
   // Decommenter ci-dessous pour creer une erreur de coherence flux/tampon   
   // if (i==52) tamp_s[1].cmd1 = (9 << 11) + (6 << 5) + 4;
   
      tamp_s[1].d[0] = j++;
      tamp_s[1].d[1] = j++;
      tamp_s[1].d[2] = j++;
    
      fwrite(tamp_s, sizeof(struct asdcbc_tf) , 2, f);
    }

  


      
  fclose(f);
  exit(0);
  
  
}



