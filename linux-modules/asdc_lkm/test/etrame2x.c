/************************************************************************
 * File             : etrame0.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Essai de creation et d'execution d'une trame repetitive
 * avec utilisation de blocs "schedule"
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  2/08/01 creation                                       yg
 *
 *
 */

/* Essai d'emission d'une trame par la carte ABI */

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
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int nombre_exec;
  unsigned long periode;
   
  
  
  if (argc != 4) 
    { fprintf(stderr, "Syntaxe : %s device periode nombre_exec\n", argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  
  periode = strtoul(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de periode est anormale.\n", argv[0]);
      exit(-1);
    }
  
  nombre_exec = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de nombre_exec est anormale.\n", argv[0]);
      exit(-1);
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
    
  
  
  /*******************************/
  /*   Declaration d'une trame   */
  /*******************************/
    
    
  /* 1 - Bloc schedule initial */
  
  b.type = BC_MINOR;
  b.adrbbc = 0;
  b.start = 0;
  b.reprate = 1;
  b.heure = 0;
  b.adrbbcsuiv = 0;
  b.chainage = 0;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
  /* Memorisation adresse debut trame */
  debut_trame = b.adrbbc;
 
    
printf("--- 1 ---\n"); fflush(stdout);    
    
    
    
  /* 2 - Transfert RTBC */
  
  b.type = BC_RTBC;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 7;
  b.sous_adresse = 7;
  b.nbmots = 7;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
 
printf("--- 2 ---\n"); fflush(stdout);    
  
  
  /* 3 - Second bloc schedule */
  
  b.type = BC_SCHED;
  b.adrbbc = 0;
  b.start = 0;
  b.reprate = 1;
  b.heure = 10000;	/* 10 ms */
  b.adrbbcsuiv = 0;
  b.chainage = 1;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
printf("--- 3 ---\n"); fflush(stdout);    

  
  
  /* 4 - Transfert BCRT */
  
  b.type = BC_BCRT;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 9;
  b.sous_adresse = 5;
  b.nbmots = 2;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_BCRT) ");
      exit(-1);
    }
    
printf("--- 4 ---\n"); fflush(stdout);    
  
  t.i = b.adrtamp;
  t.n = 2;
  t.d[0] = 0xCAFE;
  t.d[1] = 0xD0D0;
  
  if (ioctl(poignee, ASDCETAMPBC, &t))
    { perror("ioctl(ASDCETAMPBC) ");
      exit(-1);
    }
  
printf("--- 5 ---\n"); fflush(stdout);    
  
  /* 5 - Transfert RTBC et rebouclage de la trame */
  
  b.type = BC_RTBC;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = debut_trame;	/* Rebouclage */
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 9;
  b.sous_adresse = 4;
  b.nbmots = 4;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
printf("--- 6 ---\n"); fflush(stdout);    
    
    
  /* Programmation periodicite de la trame  */
  /* (100000 = 0x186A0)                     */
  ERAM(poignee, MFTVALH, (short)((periode >> 16) & 0xFFFF));
  ERAM(poignee, MFTVALL, (short)((periode) & 0xFFFF));
  
printf("--- 7 ---\n"); fflush(stdout);    
  
  /* Programmation du nombre de cycles a effectuer */
  ERAM(poignee, MFTEXEH, (short)((nombre_exec >> 16) & 0xFFFF));
  ERAM(poignee, MFTEXEL, (short)((nombre_exec) & 0xFFFF));
    
printf("--- 8 ---\n"); fflush(stdout);    
  
  
  /************************************************************/
  /*   Affichage adresse debut trame puis attente operateur   */
  /************************************************************/
  
  
  printf("\n"); 
  printf("Debut trame = 0x%04X\n", debut_trame);
  printf("Dernier bloc = 0x%04X\n", b.adrbbc); 
  printf("\n"); 
  printf("Periode = %d us\n", periode);
  printf("Cycles = %d\n", nombre_exec);
  printf("\n"); 
 
    
  /* Attente operateur */
  printf(" Presser CR pour executer la trame ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
 

 
    
  
  
  /***********************************************/
  /*   Lancement de l'execution 3 fois de suite  */
  /***********************************************/
  

  if (ioctl(poignee, ASDCGOTBC, &debut_trame))
    { perror("ioctl(ASDCGOTBC) ");
      exit(-1);
    }
    
  /*** Ici, usleep(10000) dans etrame2.c a ete supprime ! */  
    
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
    
  printf("*"); fflush(stdout); 
  
   

  if (ioctl(poignee, ASDCGOTBC, &debut_trame))
    { perror("ioctl(ASDCGOTBC) ");
      exit(-1);
    }
    
  /*** Ici, usleep(10000) dans etrame2.c a ete supprime ! */  
    
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
    
  printf("*"); fflush(stdout);    
  
  

  if (ioctl(poignee, ASDCGOTBC, &debut_trame))
    { perror("ioctl(ASDCGOTBC) ");
      exit(-1);
    }
    
  /*** Ici, usleep(10000) dans etrame2.c a ete supprime ! */  
    
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
  
  
  printf("C'est fini !\n");    
 
    
  
  
  /****************************************************************/
  /*   Liberation de la memoire d'echange utilisee par la trame   */
  /****************************************************************/

  if (ioctl(poignee, ASDCSUPTBC, &debut_trame))
    { perror("ioctl(ASDCSUPTBC) ");
      exit(-1);
    }


      
  close(poignee);
  exit(0);
  
  
}



