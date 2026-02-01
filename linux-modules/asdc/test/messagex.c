/************************************************************************
 * File             : etrame0.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  9/04/01 creation                                       yg
 *
 *
 */

/* Essai d'emission de "transferts individuels" par la carte ABI */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"
#include "ln1-bc.h"
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, k;
  int a1, s1, v1, n1;
  int a2, s2, v2, n2;
  int a3, s3, v3, n3;
  int *tv;
  
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  short status1, erreur1, d1[32];
  short status2, erreur2, d2[32];
  short status3, erreur3, d3[32];
   
  
  
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
    
  
  
  /********************************************/
  /*   Declaration et execution d'une trame   */
  /********************************************/
  
#define ERR1 (erreur1 ? "ERR" : "   ")
#define ERR2 (erreur2 ? "ERR" : "   ")
#define ERR3 (erreur3 ? "ERR" : "   ")


  
  /*** Premier transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a1=18; s1=1; n1=20;

  /* Remplissage du tampon par les donnees a emettre */
  for(i=0; i<20; i++) d1[i] = 1 + 2 * i;

  transfert(poignee, BC_BCRT, a1, s1, n1, &status1, &erreur1, d1);

  // usleep(10000);	/* Attente de 10 ms */


  
  /*** Second transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a2=9; s2=4; n2=4;

  /* RAZ du tampon */
  for(i=0; i<32; i++) d2[i] = 0;

  transfert(poignee, BC_RTBC, a2, s2, n2, &status2, &erreur2, d2);


  // usleep(10000);	/* Attente de 10 ms */


  
  /*** Troisieme transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a3=9; s3=4; n3=4;

  /* RAZ du tampon */
  for(i=0; i<32; i++) d3[i] = 0;

  transfert(poignee, BC_RTBC, a3, s3, n3, &status3, &erreur3, d3);



  /*** Affichage des resultats **********************************************/

  printf("%s RT%d,%d (n=%d) <-- S=%04X", ERR1, a1, s1, n1, status1 & 0xFFFF);
  if (erreur1) printf("\t Erreur=0x%04X", erreur1);
  printf("\n");

  printf("%s RT%d,%d (n=%d) --> S=%04X", ERR2, a2, s2, n2, status2 & 0xFFFF);
  for (i=0; i<n2; i++) printf(" %04X", d2[i] & 0xFFFF);
  if (erreur2) printf("\t Erreur=0x%04X", erreur2);
  printf("\n");

  printf("%s RT%d,%d (n=%d) --> S=%04X", ERR3, a3, s3, n3, status3 & 0xFFFF);
  for (i=0; i<n3; i++) printf(" %04X", d3[i] & 0xFFFF);
  if (erreur3) printf("\t Erreur=0x%04X", erreur3);
  printf("\n");


  
  
  /* Fin de l'essai */
  
  
  printf("C'est fini !\n");    
  
  close(poignee);
  exit(0);
  
  
}



