/************************************************************************
 * File             : lver.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1 22/08/02 created                                        yg
 *
 *
 */

/* Lecture de la version du pilote ASDC d'une carte ABI */

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
  int i, a, n;
  int *tv;
  
  char *device;
  
  struct asdcver v;
  
  
  if (argc != 2) goto syntaxe;
  
  device = argv[1];    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 

  /* Demande lecture version */
  if (ioctl(poignee, ASDCLVER, &v) == -1 )
    {
      perror("ioctl(ASDCLVER) ");
      exit(-1);
    }
    
 
   

 /* Affichage de la version lue */ 
  printf("Pilote %s v%s du %s\n", v.nom, v.version, v.date);
  printf("DSP :      0x%04X 0x%04X\n", v.vdsp & 0xFFFF, v.rdsp & 0xFFFF);
  printf("Firmware : 0x%04X 0x%04X\n", v.vfirm & 0xFFFF, v.rfirm & 0xFFFF);
  
  printf("\n");
  
  /* Affichage du numero du device et des adresses du coupleur */
  printf("Device 1553 numero %d\n", v.signal_number);
  printf("Adresse de base physique =  0x%08X\n", v.bpba);
  printf("Adresse de base virtuelle = 0x%08X\n", v.bvba);
      
  close(poignee);
  exit(0);
  
  
syntaxe : 
  fprintf(stderr, "Syntaxe : %s device\n", argv[0]);
  exit(-1);

}







