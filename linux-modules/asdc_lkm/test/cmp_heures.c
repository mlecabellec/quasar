/************************************************************************
 * File             : cmp_heures.c
 * Operating System : GNU/Linux (CMB seulement)
 *
 * History
 *
 * Edition Date     Commentaire                                    Par
 * ------- -------- ---------------------------------------------- ---
 *       1  7/01/05 creation                                        yg
 *       
 *
 *
 */

/* Lectures simultanee des horloges IRIG et non IRIG d'une carte ABI */
/* (Bien sur, une seule des 2 est valide ... )                       */

#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"


/* Prototype fonction de conversion date IRIG */
static __inline void calcul_irig(struct asdchcalibr *irig,
                        int *j, int *h, int *m,
                        int *s, int *ms, int *us,
                        long *totals, long *totalus,
                        int *synchro);

                        
/* Pour recuperer frequence UC, utilisee pour transformer en unites */
/* temporelles standards les valeurs du TSC                         */ 
double cpufreqhz(void)
{
  FILE * f;

  char tampon[100];
  int i;
  char *p, *q;
  double freq;
  
  f = fopen("/proc/cpuinfo", "r");
  if (f == NULL)
    { perror("cpufreqhz: fopen(\"/proc/cpuinfo\", ...) ");
      exit(-1);
    }
    
  for (i=0; i<7; i++)
    {
      fgets(tampon, sizeof(tampon), f);
    }
  // printf(">>>%s<<<\n", tampon);
  
  p = strrchr(tampon, '\n');
  if (p == NULL) goto pbcpuinfo;
  *p = '\0';
  
  p = strrchr(tampon, ':');
  
  if (p == NULL) goto pbcpuinfo;
  p++;
  freq = strtod(p, &q);
  
  if (*q != '\0') goto pbcpuinfo;
  
  // printf("Freq = %lg MHz\n", freq);  
  
  return freq * 1.E6;
  
pbcpuinfo :
  fprintf(stderr, "cpufreqhz: Echec recup. freq. UC dans /proc/cpuinfo\n");
  fprintf(stderr, "   Ligne = \"%s\"", tampon);
  exit(-1);
  
}



 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, a, v, n;
  int *tv;
  
  char *device;
  struct asdchcalibr d1, d2;
  
  int j, h, m, s, ms, us;
  long totals, totalus;
  int synchro;
  
  double periode_tsc;
  double d_tsc_h, d_tsc_l, d_tsc; 
  
  if (argc != 2) goto syntaxe;
  
  device = argv[1];    

  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 

  /* Demande lecture des heures */
  if (ioctl(poignee, ASDCHCALIBR, &d1) == -1 )
    {
      perror("ioctl(ASDCHCALIBR (1)) ");
      exit(-1);
    }
    
 

  /* On recommence ! */
  if (ioctl(poignee, ASDCHCALIBR, &d2) == -1 )
    {
      perror("ioctl(ASDCHCALIBR (2)) ");
      exit(-1);
    }
    
 
 periode_tsc = 1 / cpufreqhz();
   

 /* Conversion et affichage des heures lues */ 
 
 printf("HI=0x%04X LO=0x%04X   ", d1.schigh, d1.sclow);
 
 calcul_irig(&d1, &j, &h, &m, &s, &ms, &us, &totals, &totalus, &synchro);
 
 printf("%c %03d %02d:%02d:%02d.%03d.%03d\n",
         synchro ? 'S' : ' ', j, h, m, s, ms, us);
 printf("Tsys : %lu.%06lu\n", d1.sys_s, d1.sys_ns);
 printf("TSC :               %lu.%06lu\n", d1.tsc1_h, d1.tsc1_l);
 printf("                    %lu.%06lu\n", d1.tsc2_h, d1.tsc2_l);
 printf("                    %lu.%06lu\n", d1.tsc3_h, d1.tsc3_l);
 //printf("TSC :               %08lx.%08lx\n", d1.tsc1_h, d1.tsc1_l);
 //printf("                    %08lx.%08lx\n", d1.tsc2_h, d1.tsc2_l);
 //printf("                    %08lx.%08lx\n", d1.tsc3_h, d1.tsc3_l);
 
 d_tsc_h = d1.tsc2_h - d1.tsc1_h;
 d_tsc_l = d1.tsc2_l - d1.tsc1_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition heure IRIG :    %lg us\n", d_tsc * 1E6);
  
 d_tsc_h = d1.tsc3_h - d1.tsc2_h;
 d_tsc_l = d1.tsc3_l - d1.tsc2_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition heure systeme : %lg us\n", d_tsc * 1E6);
 
 d_tsc_h = d1.tsc3_h - d1.tsc1_h;
 d_tsc_l = d1.tsc3_l - d1.tsc1_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition totale  :       %lg us\n", d_tsc * 1E6);
         
 printf("\n");
 
 printf("HI=0x%04X LO=0x%04X   ", d2.schigh, d2.sclow);
 
 calcul_irig(&d2, &j, &h, &m, &s, &ms, &us, &totals, &totalus, &synchro);
 
 printf("%c %03d %02d:%02d:%02d.%03d.%03d\n",
         synchro ? 'S' : ' ', j, h, m, s, ms, us);
 printf("Tsys : %lu.%06lu\n", d2.sys_s, d2.sys_ns);
 printf("TSC :               %lu.%06lu\n", d2.tsc1_h, d2.tsc1_l);
 printf("                    %lu.%06lu\n", d2.tsc2_h, d2.tsc2_l);
 printf("                    %lu.%06lu\n", d2.tsc3_h, d2.tsc3_l);
 //printf("TSC :               %08lx.%08lx\n", d2.tsc1_h, d2.tsc1_l);
 //printf("                    %08lx.%08lx\n", d2.tsc2_h, d2.tsc2_l);
 //printf("                    %08lx.%08lx\n", d2.tsc3_h, d2.tsc3_l);
 
 d_tsc_h = d2.tsc2_h - d2.tsc1_h;
 d_tsc_l = d2.tsc2_l - d2.tsc1_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition heure IRIG :    %lg us\n", d_tsc * 1E6);
  
 d_tsc_h = d2.tsc3_h - d2.tsc2_h;
 d_tsc_l = d2.tsc3_l - d2.tsc2_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition heure systeme : %lg us\n", d_tsc * 1E6);
 
 d_tsc_h = d2.tsc3_h - d2.tsc1_h;
 d_tsc_l = d2.tsc3_l - d2.tsc1_l;
 d_tsc = periode_tsc * d_tsc_h;
 d_tsc *= 4294967296.0;               /* 2**32 = 4294967296 */
 d_tsc += periode_tsc * d_tsc_l;
 printf("Duree acquisition totale  :       %lg us\n", d_tsc * 1E6);
  
      
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
static __inline void calcul_irig(struct asdchcalibr *irig,
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
