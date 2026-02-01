/************************************************************************
 * File             : etrame0.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Essai de creation et d'execution d'une trame repetitive
 * du meme type que celle souaitee pour les essais PPIL 
 * avec definition d'un flux associe ...
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  5/11/01 creation                                       yg
 *
 *
 */

/* Essai d'emission d'une trame liee a un flux BC par la carte ABI */

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
  
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int nombre_exec;
  unsigned long periode;
  
  struct asdcbcflux_aj f;
  struct asdcbcflux_etat e;
  
  struct asdcbc_tf tamp_s[1000], tamp_e1[1000], tamp_e2[1000];
  struct asdcbcflux flx;
 
  int tra1, tra2, tra3;  
  int ttra1;  

  
  if (argc != 4) 
    { fprintf(stderr, "Syntaxe : %s device periode nombre_exec\n", argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  
  /* periode = 1250; */
  
  periode = strtol(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de periode est anormale.\n", argv[0]);
      exit(-1);
    }
    
  nombre_exec = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de nombre_exec est anormale.\n", argv[0]);
      exit(-1);
    }
    
  printf("\n    periode = %d\n", periode);
  printf("nombre_exec = %d ==> duree trame = %g s\n", nombre_exec,
         (double) periode * 1E-6 * (double) nombre_exec);

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
 
        
    
  /* 2 - Transfert BCRT */
  
  b.type = BC_BCRT;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 4;
  b.sous_adresse = 7;
  b.nbmots = 2;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    
  /* Memorisation adresse bloc BC et adresse tampon BC */
  tra1 = b.adrbbc;
  ttra1 = b.adrtamp;


  
  /* 3 - Second bloc schedule */
  
  b.type = BC_SCHED;
  b.adrbbc = 0;
  b.start = 0;
  b.reprate = 1;
  b.heure = 250;	/* 250 us */
  b.adrbbcsuiv = 0;
  b.chainage = 1;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    

  
  
  /* 4 - Transfert RTBC */
  
  b.type = BC_RTBC;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = 0;
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 4;
  b.sous_adresse = 8;
  b.nbmots = 15;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_BCRT) ");
      exit(-1);
    }
    
  /* Memorisation adresse transfert */
  tra2 = b.adrbbc;      


  
  /* 5 - Troisieme bloc schedule */
  
  b.type = BC_SCHED;
  b.adrbbc = 0;
  b.start = 0;
  b.reprate = 1;
  b.heure = 750;	/* 750 us */
  b.adrbbcsuiv = 0;
  b.chainage = 1;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_RTBC) ");
      exit(-1);
    }
    

  
  
  /* 6 - Transfert RTBC et rebouclage de la trame */
  
  b.type = BC_RTBC;
  b.adrbbc = 0;
  b.adrtamp = 0;
  b.adrbbcsuiv = debut_trame;	/* Rebouclage */
  b.options = 2;	/* Pas d'arret si erreur */
  b.chainage = 1;
  b.bus = 0;
  b.adresse = 4;
  b.sous_adresse = 9;
  b.nbmots = 15;
  
  if (ioctl(poignee, ASDCBBC, &b))
    { perror("ioctl(ASDCBBC, BC_BCRT) ");
      exit(-1);
    }
    
  /* Memorisation adresse transfert */
  tra3 = b.adrbbc;      
  
  
  
  
    
printf("--- Fin definition des tranferts ---\n"); fflush(stdout);    
    
    
  /* Programmation periodicite de la trame  */
  ERAM(poignee, MFTVALH, (short)((periode >> 16) & 0xFFFF));
  ERAM(poignee, MFTVALL, (short)((periode) & 0xFFFF));
  
  /* Programmation du nombre de cycles a effectuer */
  ERAM(poignee, MFTEXEH, (short)((nombre_exec >> 16) & 0xFFFF));
  ERAM(poignee, MFTEXEL, (short)((nombre_exec) & 0xFFFF));
    
printf("--- Fin programmation periodicite de la trame ---\n"); fflush(stdout);    
  
  
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
  printf(" Presser CR pour definir les fluxs ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
  
  
  /* Creation flux */
  
  f.flux = tra1;
  f.bc = tra1;
  f.nbte = 500;
  f.nbts = 500;
  if (ioctl(poignee, ASDCBCFLUX_AJOUTER, &f))
    { perror("ioctl(ASDCBCFLUX_AJOUTER) - 1 ");
      exit(-1);
    }
    
  printf("Adresse flux #1 = 0x%04X\n", tra1 & 0xFFFF);
  
  f.flux = tra1;
  f.bc = tra2;
  if (ioctl(poignee, ASDCBCFLUX_AJOUTER, &f))
    { perror("ioctl(ASDCBCFLUX_AJOUTER) - 2 ");
      exit(-1);
    }
  
  f.flux = tra1;
  f.bc = tra3;
  if (ioctl(poignee, ASDCBCFLUX_AJOUTER, &f))
    { perror("ioctl(ASDCBCFLUX_AJOUTER) - 3 ");
      exit(-1);
    }
    
  /* Programmation flux */
  e.flux = tra1;
  e.nblte = 100;
  e.nblts = 100;
  e.evt = 0;	/* Aucun reveil de l'appli n'est prevu ! */
  if (ioctl(poignee, ASDCBCFLUX_REGLER, &e))
    { perror("ioctl(ASDCBCFLUX_REGLER) - 1 ");
      exit(-1);
    }
  
  
    
  
  
  
  
    
  /* Attente operateur */
  printf(" Presser CR pour chargement preliminaire des tampons materiels ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
 
  
  
  /* Chargement initial des tampons materiels en sortie du flux */
  
  t.i = ttra1;
  t.n = 2;
  t.d[0] = 0xCAFE;
  t.d[1] = 0xDECA;
  if (ioctl(poignee, ASDCETAMPBC, &t))
    { perror("ioctl(ASDCETAMPBC) - 1 ");
      exit(-1);
    }
  
  
  
  

    
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
    
    
    
  /***********************************************************************/
  /*   CORRECTION (provisoire ?) d'un probleme a etudier ...             */
  /*                                                                     */
  /*   Sans le usleep ci-dessous :                                       */
  /*        -> "etrame2 /dev/asdc1 1000000 10" fonctionne parfaitement   */
  /*            au premier essai                                         */
  /*                                                                     */
  /*        -> sort sans attendre la fin de la trame au second essai     */
  /*           (puis bloque le coupleur en effacant sa trame ...)        */
  /*           avec un message console :                                 */
  /*              "Ioctl(ASDCAFINTBC) : (BCCPTR == 0) && (BCLPTR != 0)   */
  /*               BCCPTR=0x19E0   BCLPTR=0x0000"                        */
  /*                                                                     */
  /*   Questions : - Combien de temps faut-il attendre entre             */
  /*                  l'ecriture de BCIPTR et le debut de la trame ?     */
  /*               - Cette duree est-elle constante ?                    */
  /*                                                                     */
  /*   Le choix d'un usleep de 10 ms a ete fait parce que, sous LynxOS,  */
  /*   dans la config. systeme utilisee, tout usleep de moins de 10 ms   */
  /*   dure 10 ms !                                                      */
  /***********************************************************************/
 
  usleep(10000);
    
  
  
  /*******************************/
  /*   Attente fin de la trame   */
  /*******************************/
    
    
  if (ioctl(poignee, ASDCAFINTBC, 0))
    { perror("ioctl(ASDCAFINTBC) ");
      exit(-1);
    }
  
  
  printf("C'est fini !\n");    
 
 
  
  
  
  /* Attente operateur */
  printf(" Presser CR pour detruire les fluxs et la trame ...");
  fflush(stdout);
  fgets(tmp, sizeof(tmp), stdin);
    
  
  
  /*************************************/
  /*   Suppression des fluxs definis   */
  /*************************************/

  f.flux = tra1;
  if (ioctl(poignee, ASDCBCFLUX_SUPPRIMER, &f))
    { perror("ioctl(ASDCBCFLUX_SUPPRIMER) - 1 ");
      exit(-1);
    }

    
  
  
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



