/*******************************************************************/
/*                                                                 */
/*  AFFICHAGE DE LA STRUCTURE DE LA MEMOIRE ECHANGE DE L'AMI/ABI   */
/*              EQUIPE DU NOUVEAU MICROCODE DIT "SDC"              */
/*                                                                 */
/*     Suppose que la fonction mmap() soit disponible dans le      */
/*     driver du peripherique concerne ...                         */
/*                                                                 */
/*                              Anonymized, le 30 janvier 1991   */
/*                            derniere modif. le  4 fevrier 1991   */
/*                             adaptation SDC le    31 mars 1992   */
/*                            derniere modif. le     8 mars 1993   */
/*                    Carte SBS1553 ABI/PMC : le     5 juin 2001   */
/*                Mise en conformite gcc v4 : le 15 fevrier 2008   */
/*                            derniere modif. le                   */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/* #include <LOCAL/asdcctl.h> */

#include "ln1.h"

/* Constitution de la table des registres */

enum REGISTRES
{ CMD = 0x80,
  RESP,
  IQRSP,
  IQPTR1,
  IQPTR2,
  IQCNT1,
  IQCNT2,
  IQNUM,
  SWTPTR,
  ATPTR,
  FTPTR,
  SFMS,
  M1PTR,
  M2PTR,
  MBLEN,
  BCIPTR,
  BCCPTR,
  BCLPTR,
  BCSMSK,
  SCHIGH,
  SCLOW,
  RTPPTR,
  BITPTR,
  LCDPTR,
  TVWPTR,
  LSWPTR,
  LSYPTR,
  trou1,
  PROPTR,
  CCW,
  
  MIMPTR,
  MBFLG,
  
  trou2,
  trou3,
  trou4,
  trou5,
  trou6,
  trou7,
  trou8,
  trou9,
  trou10,
  trou11,
  trou12,
  SMBCNT,
  RSPGPA,
  RSPGPS,
  BCIGP,
  BRTCNT,
  BRTBUS,
  BRTCMD,
  BRTRTC,
  trou13,
  trou14,
  trou15,
  
  trou16,
  trou17,
  trou18,
  trou19,
  trou20,
  trou21,
  trou22,
  trou23,
  STUBSEL,
  MFPERFR,
  
  MFTVALH = 0xC0,
  MFTVALL,  
  MFTCNTL, 
  MFTEXEH,
  MFTEXEL,
  ASYNCBC,
  ASYNCBCT,
  
  trou24,  
  trou25,  
  trou26,  
  trou27,  
  trou28,  
  trou29,
  
  BCPAM,
  
  trou30,
  trou31,
  trou32,
  trou33,
  VVCTL,
  VVALUE
};




static void ecrit_tampon();


int file_desc;

#define RAM(X) 	(LL_get_ram(file_desc, X))



main(argc,argv)
int argc;
char *argv[];
{
  int rslt;
  int fd;
  int decalage;   /* Position zone a afficher par rapport debut mem. periph. */
  union masdc *a;
  caddr_t uuu;
  
  int i, j, k, m, l, n, o;

  if (argc!=2) { printf("\nSyntaxe :   %s  periph. \n", argv[0]);
                 exit(-1);
               }
                 

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
 
  printf("\nErreurs memorisees par '%s' : \n", argv[1]);
         
    
  for(i=0, k=0; i<34; i++)
     { if (RAM(0x101+2*i) || RAM(0x100+2*i))
            { printf("Erreur %2d  (A=0x%04x)\tNombre = %5d\tArg. = 0x%04x\n", 
                     i+1, 0x100+2*i, RAM(0x101+2*i), RAM(0x100+2*i));
              k++;
            }
     }
     
  if (!k) printf("Aucune erreur !\n");
  printf("\n");   
     
  close(file_desc);
  exit(0);
} 



