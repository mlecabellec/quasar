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
  int i, k, a, s, v, n;
  int *tv;
  
  char *device;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  short status, erreur, d[32];
   
  
  
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
  
#define ERR (erreur ? "ERR" : "   ")


  
  /*** Premier transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a=18; s=1; n=20;

  /* Remplissage du tampon par les donnees a emettre */
  for(i=0; i<20; i++) d[i] = 1 + 2 * i;

  transfert(poignee, BC_BCRT, a, s, n, &status, &erreur, d);
  printf("%s RT%d,%d (n=%d) <-- S=%04X", ERR, a, s, n, status & 0xFFFF);
  if (erreur) printf("\t Erreur=0x%04X", erreur);
  printf("\n");


  usleep(10000);	/* Attente de 10 ms */


  
  /*** Second transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a=9; s=4; n=4;

  /* RAZ du tampon */
  for(i=0; i<32; i++) d[i] = 0;

  transfert(poignee, BC_RTBC, a, s, n, &status, &erreur, d);
  printf("%s RT%d,%d (n=%d) --> S=%04X", ERR, a, s, n, status & 0xFFFF);
  for (i=0; i<n; i++) printf(" %04X", d[i] & 0xFFFF);
  if (erreur) printf("\t Erreur=0x%04X", erreur);
  printf("\n");


  usleep(10000);	/* Attente de 10 ms */


  
  /*** Troisieme transfert *************************************************/

  /* Adresse, sous-adresse et nombre de mots du transfert */
  a=9; s=4; n=4;

  /* RAZ du tampon */
  for(i=0; i<32; i++) d[i] = 0;

  transfert(poignee, BC_RTBC, a, s, n, &status, &erreur, d);
  printf("%s RT%d,%d (n=%d) --> S=%04X", ERR, a, s, n, status & 0xFFFF);
  for (i=0; i<n; i++) printf(" %04X", d[i] & 0xFFFF);
  if (erreur) printf("\t Erreur=0x%04X", erreur);
  printf("\n");





  
  
  /* Fin de l'essai */
  
  
  printf("C'est fini !\n");    
  
  close(poignee);
  exit(0);
  
  
}



