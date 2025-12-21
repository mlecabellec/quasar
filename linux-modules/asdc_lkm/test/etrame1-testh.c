/************************************************************************
 * File             : etrame0.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Essai de creation et d'execution d'une trame repetitive
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  9/04/01 creation                                       yg
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
    
    
  /* Demarrage firmware */
  
  /*** Ici, il faudrait une fonction LN1 pour pouvoir affirmer ***/
  /*** que le coupleur est OK (firmware demarre, etc...)       ***/
    
  
  
  /*******************************/
  /*   Declaration d'une trame   */
  /*******************************/
    
  /* 1 - Transfert RTBC */
  
  b.type = BC_RTBC;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 0;
  b.bus = 0;
  b.adresse = 7;
  b.sous_adresse = 7;
  b.nbmots = 7;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
  /* Memorisation adresse debut trame */
  debut_trame = b.adrbbc;
 
  
  
  /* 2 - Delai */
  
  b.type = BC_DELAI;
  b.adrbbc = 0;
  b.adrbbcsuiv = 0;
  b.chainage = 1;
  b.retard = 20000;	/* 20 ms ??? */
     /*** Attention : "b.retard=30000" bloque l'execution de la trame !!? ***/ 
    
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_DELAI) ");
      exit(-1);
    }

  
  
  /* 3 - Transfert BCRT */
  
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
    
  
  t.i = b.adrtamp;
  t.n = 2;
  t.d[0] = 0xCAFE;
  t.d[1] = 0xD0D0;
  
  if (ioctl(poignee, ASDCETAMPBC, &t))
    { perror("ioctl(ASDCETAMPBC) ");
      exit(-1);
    }
  
  
  /* 4 - Transfert RTBC et rebouclage de la trame */
  
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
    
 
    
  
  
  /************************************************************/
  /*   Affichage adresse debut trame puis attente operateur   */
  /************************************************************/
  
  
  printf("\n"); 
  printf("Debut trame = 0x%04X\n", debut_trame);
  printf("Dernier bloc = 0x%04X\n", b.adrbbc); 
  printf("\n"); 
 
    
  /* Attente operateur */
  printf(" Presser CR pour executer la trame ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
 

 
    
  
  
  /********************************/
  /*   Lancement de l'execution   */
  /********************************/
  

  if (ioctl(poignee, ASDCGOTBC, &debut_trame))
    { perror("ioctl(ASDCGOTBC) ");
      exit(-1);
    }
    
    
  
  
  /*************************/
  /*   Attente operateur   */
  /*************************/   

  printf(" Presser CR pour arreter ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
  
  
  
    
    
  
  
  /*************************/
  /*   Arret de la trame   */
  /*************************/
    
  if (ioctl(poignee, ASDCSTOPTBC, 0))
    { perror("ioctl(ASDCSTOPTBC) ");
      exit(-1);
    }
    
    
  
  
  /*******************************/
  /*   Attente fin de la trame   */
  /*******************************/
      
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
    
    
  /* Calcul temps de transfert noyau --> Application */
  {  
    struct timespec time2;
    struct asdcdbg hnoyau;
    int delta, delta1;
    long int h2, h1;

    extern void heure(long int *phaut, long int *pbas);
  
    /* Lecture heure arrivee dans application 
    if (clock_gettime(CLOCK_REALTIME, &time2) == -1)
				{
        			  perror("clock_gettime() :");
        			  exit(-1);
    				}
    */
    heure(&h2, &h1);
    

    /* Lecture heure depart du noyau
    if (ioctl(poignee, ASDC_TESTH, &hnoyau))
      { perror("ioctl(ASDC_TESTH) ");
        exit(-1);
      }
      
    /* Calcul et affichage du delta T
    printf("Noyau : %d.%09d   ", hnoyau.s, hnoyau.ns);
    printf("Appli : %d.%09d   ", time2.tv_sec, time2.tv_nsec);
    
    delta = time2.tv_sec - hnoyau.s;
    delta *= 1000000000;
    delta1 = (time2.tv_nsec) - (hnoyau.ns);
    delta += delta1;
    
    printf("Delta = %d ns\n", delta);
    */
    printf("Noyau : %d   ", hnoyau.ns);
    printf("Appli : %d   ", h1);
    printf("Delta = %d ??\n", h1 - hnoyau.ns);

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



