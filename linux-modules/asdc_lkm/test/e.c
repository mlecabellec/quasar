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

  
  
  device = argv[1];
  
  /* periode = 1250; */
  

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
 printf("Avant ASDCECR0_n"); fflush(stdout);   


{ struct asdctampon t;


  
  t.nbr = 2;
  t.t[0] = 0xCAFE;
  t.t[1] = 0xDECA;
  t.v.adresse = 1;
  t.v.sous_adresse = 1;
  t.v.direction = 0;
  
  
  if (ioctl(poignee, ASDCECR0, &t))
    { perror("ioctl(ASDCECR0) - 1 ");
      exit(-1);
    }
  
  
}  
  



      
  close(poignee);
  exit(0);
  
  
}



