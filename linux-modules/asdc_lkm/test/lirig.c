/************************************************************************
 * File             : lirig.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        By
 * ------- -------- ---------------------------------------------- ---
 *       1 22/08/02 created                                        yg
 *       2  5/02/03 lecture horloges "IRIG" et "non IRIG"          yg
 *
 *
 */

/* Lectures simultanee des horloges IRIG et non IRIG d'une carte ABI */
/* (Bien sur, une seule des 2 est valide ... )                       */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"


/* Prototype fonction de conversion date IRIG */
static __inline void calcul_irig(struct asdcirig *irig,
                        int *j, int *h, int *m,
                        int *s, int *ms, int *us,
                        long *totals, long *totalus,
                        int *synchro);
 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, v, n;
  int *tv;
  
  char *device;
  
  struct asdcirig d1, d2;
  
  int j, h, m, s, ms, us;
  long totals, totalus;
  int synchro;
  
  
  if (argc != 2) goto syntaxe;
  
  device = argv[1];    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 

  /* Demande lecture heure */
  if (ioctl(poignee, ASDCLDATE, 0) == -1 )
    {
      perror("ioctl(ASDCLDATE) ");
      exit(-1);
    }
    
 
  /* Acquisition de l'heure lue */
 if (ioctl(poignee, ASDCLIRIG, &d1) == -1 )
   {
     perror("ioctl(ASDCLIRIG) ");
     exit(-1);
   }

  /* Demande lecture heure */
  if (ioctl(poignee, ASDCLDATE, 0) == -1 )
    {
      perror("ioctl(ASDCLDATE) ");
      exit(-1);
    }
    
 
  /* Acquisition de l'heure lue */
 if (ioctl(poignee, ASDCLIRIG, &d2) == -1 )
   {
     perror("ioctl(ASDCLIRIG) ");
     exit(-1);
   }
   

 /* Conversion et affichage de l'heure lue */ 
 
 printf("HI=0x%04X LO=0x%04X   ", d1.schigh, d1.sclow);
 calcul_irig(&d1, &j, &h, &m, &s, &ms, &us, &totals, &totalus, &synchro);
 printf("%c %03d %02d:%02d:%02d.%03d.%03d\n",
         synchro ? 'S' : ' ', j, h, m, s, ms, us);
         
 printf("HI=0x%04X LO=0x%04X   ", d2.schigh, d2.sclow);
 calcul_irig(&d2, &j, &h, &m, &s, &ms, &us, &totals, &totalus, &synchro);
 printf("%c %03d %02d:%02d:%02d.%03d.%03d\n",
         synchro ? 'S' : ' ', j, h, m, s, ms, us);
  
      
  close(poignee);
  exit(0);
  
  
syntaxe : 
  fprintf(stderr, "Syntaxe : %s device\n", argv[0]);
  exit(-1);

}







/********************************************************************/
/* Mise en forme d'une date IRIG                                    */
/*	Forme 1 : "jour - heure - minute - seconde | ms - us"       */  
/*      Forme 2 : "total secondes | total us"                       */
/*      synchro : vrai si horloge non en "roue libre"               */
/********************************************************************/
static __inline void calcul_irig(struct asdcirig *irig,
                        int *j, int *h, int *m,
                        int *s, int *ms, int *us,
                        long *totals, long *totalus,
                        int *synchro)
{
  int hi, mi, li;
  
  hi = irig->schighi;
  mi = irig->scmidi;
  li = irig->sclowi;
  
  /* printf("0x%04X 0x%04X 0x%04X\n", hi, mi, li); */

  *j = (hi >> 5) & 0x1FF;
  *h = hi & 0x1F; 
  *m = (mi >> 10) & 0x3F;
  *s = (mi >> 4) & 0x3F;

  *totalus = (li & 0xFFFF) | ((mi << 16) & 0xF0000);
  
  *us = *totalus % 1000;
  *ms = *totalus / 1000;

  *totals = *s + 60 * *m + 3600 * *h + 86400 * *j;
  
  *synchro = (hi & 0x8000) ? 0 : 1;
}
