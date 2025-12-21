#include <stdio.h>

/* 
 * Fonction pour lecture du fichier firmware SBS :
 *
 * Lecture d'1 mot (16 bits) code en ASCII hexa danf f (prealablement ouvert)
 *
 *   - Renvoi le mot si OK
 *   - Revoi -1 si probleme (dont "fin du fichier" atteinte)
 *
 */
int lec1mot(FILE *f)
{
  char tmp[100];
  int val;
  int i;
  
  fgets(tmp, sizeof tmp, f);
  if (feof(f)) return -1;
  
  if (strlen(tmp) != 6)
    { printf("LEC1MOT : n=%d   tmp=\"%s\"\n", strlen(tmp), tmp);
      exit(-1);
    }
    
  tmp[4] = '\0';
  i = sscanf(tmp, "%x%c", &val);
  
  if (i != 1)
    { printf("LEC1MOT : Mot bizarre = \"%s\"\n", tmp);
      exit(-1);
    }
  
  return val;
}



main(int argc, char **argv)
{

  FILE *f;
  short x;
  short nh, nb;
  long n;
  int i;
  
  if (argc != 2)
    { fprintf(stderr, "Syntaxe : %s firmware_file\n", argv[0]);
      exit(-1);
    }
  
  f = fopen(argv[1], "r");
    { if (f == NULL)
        { fprintf(stderr, "Impossible d'ouvrir \"%s\"\n", argv[1]);
          exit(-1);
        }
    }
  
  /* Lecture 16 premiers mots */
  printf("\nEn-tete :\n"); 
  for (i=0; i<16; i++)
    { x = lec1mot(f);
      if feof(f)
        { fprintf(stderr, "Fin de fichier inattendue en phase 1\n");
          exit(-1); 
        }
      printf(" %04X", x & 0xFFFF); fflush(stdout);
      if (i == 7) printf("\n");
    }
  printf("\n");
  
  
  
  /* Lecture du nombre de mots de la section DSP */
  nh = lec1mot(f);
  if feof(f)
    { fprintf(stderr, "Fin de fichier inattendue en phase 2a\n");
      exit(-1); 
    }
  nb = lec1mot(f);
  if feof(f)
    { fprintf(stderr, "Fin de fichier inattendue en phase 2b\n");
      exit(-1); 
    }
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nSection DSP :\n");
  printf("   nh=0x%04X   nb=0x%04X   n=0x%08X (n=%d)\n", nh, nb, n, n);
  
  /* Lecture de la section DSP */
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if feof(f)
        { fprintf(stderr, "Fin de fichier inattendue en phase 2c\n");
          exit(-1); 
        }
    }
    
    
    
    
  /* Lecture du nombre de mots de la section Altera */
  nh = lec1mot(f);
  if (feof(f))
    { fprintf(stderr, "Fin de fichier inattendue en phase 3a\n");
      exit(-1); 
    }
  nb = lec1mot(f);
  if (feof(f))
    { fprintf(stderr, "Fin de fichier inattendue en phase 3b\n");
      exit(-1); 
    }
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nSection Altera :\n");
  printf("   nh=0x%04X   nb=0x%04X   n=0x%08X (n=%d)\n", nh, nb, n, n);
  
  /* Lecture de la section Altera */
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if (feof(f))
        { fprintf(stderr, "Fin de fichier inattendue en phase 3c\n");
          exit(-1); 
        }
    }
  
  
  
  
  /* Controle fin du fichier atteinte */
  { x = lec1mot(f);
    if (!feof(f))
      { fprintf(stderr, "Il reste des mots dans le fichier !\n");
        exit(-1);
      }
  }
  
  printf("\nC'est fini !\n");
  exit(0);  
    
}
