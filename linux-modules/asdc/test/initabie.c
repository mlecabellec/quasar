/************************************************************************
 * File             : initabie.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 14/05/01 creation                                       yg
 *       2  9/11/01 Ajout arret des taches en attente              yg
 *       3 14/11/01 Version speciale espionnage (gros tampons)     yg
 *
 *
 */

/* Petite amorce d'un initabi "a la CMBA" */

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
  
  int csr;
  
  struct asdcparam par;   
  
  
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
    
  
  
  
  
  /* Tentative d'arret de toutes les taches en attente */
  
  printf("Tentative d'arret de toutes les taches ... \n");
  fflush(stdout);
  
  if (ioctl(poignee, ASDCAVORTE, (char *) 1))
    { perror("ioctl(ASDCAVORTE) : ");
      exit(-1);
    }
    
  printf("Sleep ...");
  fflush(stdout);
  
  sleep(2);
  
  printf(" ...\n");
  fflush(stdout);
    
  if (ioctl(poignee, ASDCAVORTE, (char *) 2))
    { perror("ioctl(ASDCAVORTE) : ");
      exit(-1);
    }
    
  printf("OK !\n");
  fflush(stdout);
  
  
  
  
    
    
  /* Lecture des parametres courants du driver */
  if (ioctl(poignee, ASDCLPAR, &par))
    { perror("ioctl(ASDCLPAR) : ");
      exit(-1);
    }
    
  printf("\nParametres initiaux :\n");
  printf("   Taille tampon ITs             = %d\n", par.iqnum);   
  printf("   Taille tampon espion          = %d\n", par.mbleng);   
  printf("   Masque pour IT sur status BC  = 0x%04X\n", par.bcsmsk & 0xFFFF);   
  printf("   Intermessage gap time pour BC = %d\n", par.bcigp);   
  printf("   Nbre de reprises pour BC      = %d\n", par.brtcnt);   
  printf("   Bus a utiliser pour reprises  = 0x%04X\n", par.brtbus & 0xFFFF);   
  printf("   Temps de reponse maxi RT      = %d\n", par.rspgpa);   
  printf("   Temps de reponse RT simules   = %d\n", par.rspgps);  
  
  
  /* Modification des tailles des tampons IT et d'espionnage */
  par.iqnum = 20;
  par.mbleng = 10000;
  
  printf("\nModification de ces parametres :\n");
  printf("   Taille tampon ITs             = %d\n", par.iqnum);   
  printf("   Taille tampon espion          = %d\n", par.mbleng);   
  
    
  /* Copie des nouveaux parametres dans le driver */
  if (ioctl(poignee, ASDCEPAR, &par))
    { perror("ioctl(ASDCEPAR) : ");
      exit(-1);
    }


  /* Demarrage firmware */
  if (ioctl(poignee, ABI_FIRMWARE_INIT, 0))
    { perror("ioctl(ABI_FIRMWARE_INIT) : ");
      exit(-1);
    }


  /* Initialisation memoire d'echange */
  if (ioctl(poignee, ASDCRAZ, 0))
    { perror("ioctl(ASDCRAZ) : ");
      exit(-1);
    }


  /* Demarrage traitement I/O */
  if (ioctl(poignee, ASDCGO, 0))
    { perror("ioctl(ASDCGO) : ");
      exit(-1);
    }


      
  /* Armement des IT */
  
  csr = LRAM(poignee, 0x40000);
  printf("CSR : 0x%04X --> ", csr & 0xFFFF);
  
  /* csr |= 4;	/* Master INT enabled */
  csr |= 8;     /* DSP INT enabled */
  
  ERAM(poignee, 0x40000, csr);  
  printf("0x%04X\n", csr & 0xFFFF);

  
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);
  
  
}



