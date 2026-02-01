/*******************************************************************/
/*                                                                 */
/*  AFFICHAGE DE LA STRUCTURE DE LA MEMOIRE ECHANGE DE L'AMI/ABI   */
/*              EQUIPE DU NOUVEAU MICROCODE DIT "SDC"              */
/*                                                                 */
/*     Suppose que la fonction mmap() soit disponible dans le      */
/*     driver du peripherique concerne ...                         */
/*                                                                 */
/*                            Y. Guillemot, le   30 janvier 1991   */
/*                          derniere modif. le    4 fevrier 1991   */
/*                           adaptation SDC le      31 mars 1992   */
/*                                   modif. le       8 mars 1993   */
/*                                   modif. le     15 avril 1994   */
/*                                   modif. le 19 septembre 1994   */
/*                                   modif. le  25 novembre 1996   */
/*                  Carte SBS1553 ABI/PMC : le   6 novembre 2000   */
/*                                   modif. le       15 mai 2001   */
/*                                   modif. le   25 octobre 2001   */
/*                                   modif. le       28 mai 2002   */
/*  ASDC v4.6 : affichage connexions CEVT : le 11 septembre 2002   */
/* Affichage RESPTR (CC "RESERVE" du M51) : le    7 octobre 2002   */
/*      Affichage mode "STATIQUE" des voies le   6 novembre 2002   */
/*   Affichage mode "Synchrone 2" des voies le    8 juillet 2004   */
/*    Affichage adresses et chainages voies le     14 avril 2005   */
/*                Mise en conformite gcc v4 le   15 fevrier 2008   */
/*                          derniere modif. le                     */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/* #include <LOCAL/asdcctl.h> */

#include "ln1.h"

#define VERSION	"du 14 avril 2005"

/* Autorise le traitement des donnees de la "memoire image" */
#define IMAGE


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
  
  STUBSEL = 0xBE,
  MFPERFR,
  
  MFTVALH = 0xC0,
  MFTVALL,  
  MFTCNTH, 
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
  
  RESPTR = 0xCF,
  BCPAM,
  
  trou30,
  trou31,
  trou32,
  trou33,
  VVCTL,
  VVALUE
};

enum IMA_RT
{ IRCMD = 0,
  IRMODE,
  IRTCAPP,
  IRNBT,
  IRSEM,
  IRBIDON,
  IRCEVT,
  IRBIDON2,
  IRTCH1,
  IRTCH2,
  IRTCHINU,
  IRTFCH2,
  IRTFCHINU,
  IRTCACHE,
  IRTBIDON3,
  IRTPREC,
  
  IRTNCH1 = 30,
  IRTNCH2,
  IRTNMT  
};
#define IRTFCH1 IRTCAPP

enum IMA_BC
{ IMFLUX = 0,
  IMERR,
  IMPZD,
  IMSEMFLX,
  IMSUIV,
  IMSEMBC,
  IMFSD,
  IMCPTR
};

enum MODE_VOIE
{ RT_VSYNC = 0,
  RT_VASYNC,
  RT_VSTAT,
  RT_VSYNC2
};


static void ecrit_tampon(int p, int papp);

#ifdef IMAGE
static void ecrit_tampon_sync2(int z, int pfw);
#endif // IMAGE

int file_desc;

#define RAM(X) 	(LL_get_ram(file_desc, X))
#define IMA(X)	(LL_get_image(file_desc, X))




char * nom_mode_voie(int mode)
{
  switch (mode)     
    { case RT_VSYNC :	return "SYNC";
      case RT_VASYNC :	return "ASYNC";
      case RT_VSTAT :	return "STAT"; 
      case RT_VSYNC2 :	return "SYNC2"; 
      default :		return "?????";
    }
}





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
  int sbc;	/* "adreSse bloc BC" ou "Suivant bloc BC" */
  int tybc;	/* TYpe bloc BC */
  int abbcs;	/* Adresse Bloc Bc Suivant */
  unsigned short int *memobc; /* Pour se souvenir des blocs deja vus */
  int ibc;                    /* Indice premier emplacement libre dans memobc */
  char *etat;
  int ptrappli;
  
  printf("%s - Version %s", argv[0], VERSION);
#ifdef IMAGE
  printf("   --> Traitement \"memoire image\" valide !\n");
#else
  printf("\n");
#endif // IMAGE

  if (    (argc!=2)
       && (argc!=3)) 
           { printf("\nSyntaxe :   %s  periph. [Addr_hexa_seq_BC] \n", argv[0]);
             exit(-1);
           }
           
  if (argc==3) { if (sscanf(argv[2], "%x", &sbc) != 1)
                   { printf("\nAdresse sequence BC anormale !\n", argv[0]);
                     exit(-1);
                   }
                 sbc &= 0xFFFF;
                 
                 /* Alloc. memoire table des adresses de blocs */
                 memobc = (unsigned short int *)
                               calloc(8000, sizeof(unsigned short int));
                 if (memobc == NULL)
                   { printf("\nImpossible d'allouer memoire !\n", argv[0]);
                     exit(-1);
                   }
                 ibc = 0;   /* Table est vide */
               } 
                 

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
 
  printf("\n\nMemoire d'echange de '%s' : \n", argv[1]);
         
  
  
  printf("\nVersion microcode \n");
  printf("Firmware : Ver = 0x%04X  Rev = 0x%04X",
                   LL_get_ram(file_desc, 0x3E) & 0xFFFF,
                   LL_get_ram(file_desc, 0x3F) & 0xFFFF);
  printf("     DSP : Ver = 0x%04X  Rev = 0x%04X\n",
                   LL_get_ram(file_desc, 0x40) & 0xFFFF,
                   LL_get_ram(file_desc, 0x41) & 0xFFFF);
  
  printf("CMD = 0x%04X\t\t\n", RAM(CMD) & 0xFFFF);
  
  printf("\n");   
     
  printf("SWTPTR = 0x%04X\n", j = RAM(SWTPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("ATPTR = 0x%04X\n", j = RAM(ATPTR));
  for(i=0; i<32; i++, j++)
     { if((k=RAM(j))==0) continue;
       
       /* Determination de l'etat du RT */ 
       { int sw, pro;
         sw = RAM(RAM(SWTPTR) + i);
         pro = RAM(RAM(PROPTR) + i);
         if      (sw  && !(pro & 2)) etat = "VALIDE";
         else if (!sw &&  (pro & 2)) etat = "ESPION_TR";
         else if (!sw && !(pro & 2)) etat = "INHIBE";
         else if (sw  &&  (pro & 2)) etat = "ANORMAL";
       }
       printf("------- RT%d %s -------\n", i, etat);
       
       
       for(l=0; l<32; l++, k++)
          { if((m=RAM(k))==0) continue;
            printf("   R : RT%02d,%02d", i, l);
#ifdef IMAGE
            { int zd, cevt;
              zd = IMA(k);
              printf("   %s NbT=%d (Cmd=%04X)", 
                      nom_mode_voie(IMA(zd + IRMODE)),
                      IMA(zd + IRNBT),
                      IMA(zd + IRCMD) & 0xFFFF);
              ptrappli = IMA(zd + IRTCAPP);
              
              cevt = IMA(zd + IRCEVT);
              if (cevt) printf("   ===> CEVT%d", cevt);
              
              printf("\n");
            }
#else
            printf("\n");
            ptrappli = 0;
#endif // IMAGE
            ecrit_tampon(m, ptrappli);
          }
          
       for(l=0; l<32; l++, k++)
          { m=RAM(k);
#ifdef IMAGE
            { int zd, cevt;
              zd = IMA(k);
              if ((IMA(zd + IRMODE) != RT_VSYNC2) && (m == 0)) continue;
              printf("   T : RT%02d,%02d", i, l);
              printf("   %s NbT=%d (Cmd=%04X)", 
                      nom_mode_voie(IMA(zd + IRMODE)),
                      IMA(zd + IRNBT),
                      IMA(zd + IRCMD) & 0xFFFF);
              ptrappli = IMA(zd + IRTCAPP);
              
              cevt = IMA(zd + IRCEVT);
              if (cevt) printf("   ===> CEVT%d", cevt);
              
              printf("\n");
            }
#else
            if(m==0) continue;
            printf("   T : RT%02d,%02d", i, l);
            printf("\n");
            ptrappli = 0;
#endif // IMAGE

#ifdef IMAGE
            { int zd;
              zd = IMA(k);
              if (IMA(zd + IRMODE) == RT_VSYNC2)
                { ecrit_tampon_sync2(zd, m);
                }
              else
                { ecrit_tampon(m, ptrappli);
                }
            }
#else
            ecrit_tampon(m, ptrappli);
#endif // IMAGE
            
          }
     }
     
  printf("\n");   
     
  printf("FTPTR = 0x%04X\n", j = RAM(FTPTR));
  for(i=0; i<32; i++, j++)
     { if((k=RAM(j))==0) continue;
       for(l=0; l<32; l++, k++)
          { if((m=RAM(k))==0) continue;
            printf("   R : RT%02d,%02d : 0x%04X\n", i, l, m&0xFFFF);
          }
       for(l=0; l<32; l++, k++)
          { if((m=RAM(k))==0) continue;
            printf("   T : RT%02d,%02d : 0x%04X\n", i, l, m&0xFFFF);
          }
     }
     
  printf("\n");   
     
  printf("MIMPTR = 0x%04X\n", j = RAM(MIMPTR));
  for(i=0; i<32; i++, j++, j++)
     { k=RAM(j);
       l=RAM(j+1);
       if((k==0) && (l==0)) continue;
       printf("   RT%02d : LO=0x%04X HI=0x%04X\n",
               i, k&0xFFFF, l&0xFFFF);
     }
     
  printf("\n");   
     
  printf("RTPPTR = 0x%04X\n", j = RAM(RTPPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("BITPTR = 0x%04X\n", j = RAM(BITPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("LCDPTR = 0x%04X\n", j = RAM(LCDPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("TVWPTR = 0x%04X\n", j = RAM(TVWPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("LSWPTR = 0x%04X\n", j = RAM(LSWPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("LSYPTR = 0x%04X\n", j = RAM(LSYPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);
  
  printf("\n");   
     
  printf("PROPTR = 0x%04X\n", j = RAM(PROPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);   
  
  printf("\n");   
     
  printf("RESPTR = 0x%04X\n", RESPTR, j = RAM(RESPTR) & 0xFFFF);
  for(i=0; i<32; i++, j++)
     if(RAM(j)!=0) printf("   RT%02d : 0x%04X\n", i, RAM(j) & 0xFFFF);   
     
  printf("\n\n");
  printf("IQRSP = 0x%04X\tIQNUM = %d\n", RAM(IQRSP) & 0xFFFF, RAM(IQNUM));  
  printf("\n");
  printf("IQPTR1 = 0x%04X   IQCNT1 = %d\t\tIQPTR2 = 0x%04X   IQCNT2 = %d\n",
          j = RAM(IQPTR1) & 0xFFFF, RAM(IQCNT1),
          k = RAM(IQPTR2) & 0xFFFF, RAM(IQCNT2));
  for(i=0, l=0; l<RAM(IQNUM); l++, i += 4)
     {  printf("0x%04X 0x%04X 0x%04X 0x%04X\t\t0x%04X 0x%04X 0x%04X 0x%04X\n",
               RAM(j+i) & 0xFFFF, RAM(j+i+1) & 0xFFFF,
               RAM(j+i+2) & 0xFFFF, RAM(j+i+3) & 0xFFFF,
               RAM(k+i) & 0xFFFF, RAM(k+i+1) & 0xFFFF,
               RAM(k+i+2) & 0xFFFF, RAM(k+i+3) & 0xFFFF);
     }  
          
          
  /* Affichage d'une sequence BC */
  if (argc==3)
    { printf("\n\n\nSequence BC en 0x%04X :\n\n", sbc & 0xFFFF);
      while(sbc != 0)
        { tybc = RAM(sbc);
          switch(tybc & 0xFF)
            { case 0 : printf("0x%04X : 0x%04X ==> Commande codee\n", 
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande  (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X  status ", RAM(sbc+3) & 0xFFFF);
#ifdef IMAGE
                          printf("   Err = 0x%04X", IMA(sbc+IMERR) & 0xFFFF);
#endif
                       printf("\n");
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions",
                                                      RAM(sbc+5) & 0xFFFF);
#ifdef IMAGE
                          printf("   Cpt = %d", IMA(sbc+IMCPTR));
#endif
                       printf("\n");
                       printf("         0x%04X  adr. tampon\n",
                                                      RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                              abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 1 : printf("0x%04X : 0x%04X ==> BC->RT\n",
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande   (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X  status ", RAM(sbc+3) & 0xFFFF);
#ifdef IMAGE
                          printf("   Err = 0x%04X", IMA(sbc+IMERR) & 0xFFFF);
#endif
                       printf("\n");
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions",
                                                      RAM(sbc+5) & 0xFFFF);
#ifdef IMAGE
                          printf("   Cpt = %d", IMA(sbc+IMCPTR));
#endif
                       printf("\n");
                       printf("         0x%04X  adr. tampon\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                           abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 2 : printf("0x%04X : 0x%04X ==> RT->BC\n",
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande   (",
                                                   RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X  status ", RAM(sbc+3) & 0xFFFF);
#ifdef IMAGE
                          printf("   Err = 0x%04X", IMA(sbc+IMERR) & 0xFFFF);
#endif
                       printf("\n");
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions",
                                                      RAM(sbc+5) & 0xFFFF);
#ifdef IMAGE
                          printf("   Cpt = %d", IMA(sbc+IMCPTR));
#endif
                       printf("\n");
                       printf("         0x%04X  adr. tampon\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                           abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 3 : printf("0x%04X : 0x%04X ==> RT->RT\n",
                                           sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande 1  (",
                                                   RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X  commande 2  (",
                                                   RAM(sbc+2) & 0xFFFF);
                       decode_comm(RAM(sbc+2));
                       printf(")\n");
                       printf("         0x%04X  status 1\n",
                                                   RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X  status 2\n",
                                                   RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions\n",
                                                   RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X  adr. tampon\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                           abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 4 : printf("0x%04X : 0x%04X ==> Delai %d us\n", 
                               sbc & 0xFFFF, tybc, RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+1) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions\n",
                                                   RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X  duree = %d\n", 
                               RAM(sbc+6) & 0xFFFF, RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                            abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 5 : printf("0x%04X : 0x%04X ==> Diffusion BC->RT\n",
                                             sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande  (",
                                                     RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions\n",
                                                   RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X  adr. tampon\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                           abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 6 : printf("0x%04X : 0x%04X ==> Diffusion RT->RT\n",
                                              sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande 1  (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       printf("         0x%04X  commande 2  (",
                                                      RAM(sbc+2) & 0xFFFF);
                       decode_comm(RAM(sbc+2));
                       printf(")\n");
                       printf("         0x%04X  status\n", RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X  fanions\n",
                                                   RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X  adr. tampon\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                           abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
                       
              case 128 : printf("0x%04X : 0x%04X ==> SCHEDULE",
                                           sbc & 0xFFFF, tybc & 0xFFFF);
                       if (RAM(sbc) & 0x8000) printf(" - MINOR FRAME ENABLE");
                       printf("\n");
                       printf("         0x%04X  start#\n", RAM(sbc+1) & 0xFFFF);
                       printf("         0x%04X  rep rate\n",
                                                           RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X  sched. time MSW\n",
                                                   RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  sched. time LSW\n",
                                                   RAM(sbc+7) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+8) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+9) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+10) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+11) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+12) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+13) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+14) & 0xFFFF);
                       printf("         0x%04X  bloc suivant\n", 
                                             abbcs = RAM(sbc+15) & 0xFFFF);
                       break;
                       
              default : printf("0x%04X : 0x%04X ==> *** CODE ANORMAL ***\n",
                                                                    sbc, tybc);
                       printf("         0x%04X\n", RAM(sbc+1) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+2) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+3) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+4) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+5) & 0xFFFF);
                       printf("         0x%04X\n", RAM(sbc+6) & 0xFFFF);
                       printf("         0x%04X  bloc suivant ???\n",
                                                 abbcs = RAM(sbc+7) & 0xFFFF);
                       break;
            }
            
          memobc[ibc++] = sbc;   /* Enregistrement du bloc lu */
          
          sbc = abbcs;  /* Bloc suivant */
          
          /* A-t-il deja ete lu ? */
          for (i=0; i<ibc; i++)
             { if (memobc[i] == sbc) { sbc = 0;  /* Force arret affichage BC */
                                       break;
                                     }
             }
          printf("\n");
        } 
    }
    
    
    
  /* Affichage des structures de definition des fluxs */
   
     
     
     
  close(fd);
  exit(0);
} 



static void ecrit_tampon(int p, int papp)
{  int i, j;
   unsigned short int q;
   p = p & 0xFFFF;  /* Au cas ou ... */
   q = p;
   do { 
#ifdef IMAGE
   	/* Si tampon est pointe par "l'appli", impression d'une marque */
   	if (papp != -1)
   	  { if (q == (papp & 0xFFFF))
   	      { printf("    ==>");
   	      }
   	    else 
   	      { printf("       ");
   	      }
   	  }
#else  // IMAGE
   printf("       "); 
#endif // IMAGE
        /* Impression d'un tampon */
        printf("[0x%04X]", q);
#ifdef IMAGE
   printf(" -> 0x%04X\t", IMA(q + IRTPREC) & 0xFFFF);
#else  // IMAGE
   printf("          \t");
#endif // IMAGE
        printf(" 0x%04X 0x%04X\n", 0xFFFF&RAM(q), 0xFFFF&RAM(q+1));
        for(i=q+2; i<q+27; i+=8)
           { printf("\t");
             for(j=0; j<8; j++) printf("0x%04X ", RAM(i+j)&0xFFFF);
             printf("\n");
           }
           
        q=RAM(q);
      } while(q!=p);
}             



#ifdef IMAGE
static void ecrit_tampon_sync2(int z, int pfw)
{
   int i, j;
   unsigned short int q;
   int nbtch1, nbtch2, nbttot;
   
   nbtch1 = IMA(z + IRTNCH1);
   nbtch2 = IMA(z + IRTNCH2);
   nbttot = IMA(z + IRNBT);
  
   printf("\tNbre Tampons :  ch1=%d  ch2=%d  dispos=%d   Total=%d\n",
          nbtch1, nbtch2, nbttot - nbtch1 - nbtch2, nbttot);  
   printf("\tNbre limite tampons par chaine = %d\n", IMA(z + IRTNMT));

   if (pfw == 0) printf("    ==>[0x0000]\n");

   printf("\t------- Chaine 1 :\n");
   q = IMA(z + IRTCH1) & 0xFFFF;  
   while (q)
      { 
   	/* Si tampon est pointe comme dernier, impression d'une marque */
   	if (q == (IMA(z + IRTFCH1) & 0xFFFF))
   	  { printf("XXX ");
   	  }
   	else
   	  { printf("    ");
   	  }
   	/* Si tampon est pointe par "le firmware", impression d'une marque */
   	  if (q == (pfw & 0xFFFF))
   	    { printf("==>");
   	    }
   	  else
   	    { printf("   ");
   	    }
        /* Impression d'un tampon */
        printf(" [0x%04X] -> 0x%04X", 0xFFFF & q, 0xFFFF & IMA(q+IRTPREC));
        printf("\t0x%04X 0x%04X\n", 0xFFFF&RAM(q), 0xFFFF&RAM(q+1));
        for(i=q+2; i<q+27; i+=8)
           { printf("\t");
             for(j=0; j<8; j++) printf("0x%04X ", RAM(i+j)&0xFFFF);
             printf("\n");
           }
           
        q=RAM(q);
      } 

   printf("\t------- Chaine 2 :\n");
   q = IMA(z + IRTCH2) & 0xFFFF;  
   while (q)
      { 
   	/* Si tampon est pointe comme dernier, impression d'une marque */
   	if (q == (IMA(z + IRTFCH2) & 0xFFFF))
   	  { printf("XXX ");
   	  }
   	else
   	  { printf("    ");
   	  }
   	/* Si tampon est pointe par "le firmware", impression d'une marque */
   	  if (q == (pfw & 0xFFFF))
   	    { printf("==>");
   	    }
   	  else
   	    { printf("   ");
   	    }
        /* Impression d'un tampon */
        printf(" [0x%04X] -> 0x%04X", 0xFFFF & q, 0xFFFF & IMA(q+IRTPREC));
        printf("\t0x%04X 0x%04X\n", 0xFFFF&RAM(q), 0xFFFF&RAM(q+1));
        for(i=q+2; i<q+27; i+=8)
           { printf("\t");
             for(j=0; j<8; j++) printf("0x%04X ", RAM(i+j)&0xFFFF);
             printf("\n");
           }
           
        q=RAM(q);
      }

   printf("\t------- Tampons disponibles :\n");
   q = IMA(z + IRTCHINU) & 0xFFFF;  
   while (q)
      { 
   	/* Si tampon est pointe par "le firmware", impression d'une marque */
   	/* (sur cette chaine, ce serait tout a fait anormal !...)          */
   	  if (q == (pfw & 0xFFFF))
   	    { printf("    ==>");
   	    }
   	  else
   	    { printf("       ");
   	    }
        /* Impression d'un tampon */
        printf(" [0x%04X] -> 0x%04X", 0xFFFF & q, 0xFFFF & IMA(q+IRTPREC));
        printf("\t0x%04X 0x%04X\n", 0xFFFF&RAM(q), 0xFFFF&RAM(q+1));
        for(i=q+2; i<q+27; i+=8)
           { printf("\t");
             for(j=0; j<8; j++) printf("0x%04X ", RAM(i+j)&0xFFFF);
             printf("\n");
           }
           
        q=IMA(q + IRTPREC) & 0xFFFF;
      }

}
#endif // IMAGE






decode_comm(c)
int c;
{  int adresse, sous_adresse, emission, nombre;

   adresse = (c >> 11) & 0x1F; 
   emission = c & 0x400; 
   sous_adresse = (c >> 5) & 0x1F;
   nombre = c & 0x1F;
   
   if ((sous_adresse != 0) && (sous_adresse != 31))
     { if (emission)
         { printf("RT%d,%d --> BC    N=%d", adresse, sous_adresse, nombre);
         }
       else 
         { printf("BC --> RT%d,%d    N=%d", adresse, sous_adresse, nombre);
         }
     }
   else
     { printf("CC ");
       if (emission)
         { switch(nombre)
            { case 0 : printf("Dynamic Bus Control"); 
                       break;
              case 1 : printf("Synchronize"); 
                       break;
              case 2 : printf("Transmit Status Word"); 
                       break;
              case 3 : printf("Initiate Self-Test"); 
                       break;
              case 4 : printf("Transmitter Shutdown"); 
                       break;
              case 5 : printf("Override Transmitter Shutdown"); 
                       break;
              case 6 : printf("Inhibit Terminal Flag Bit"); 
                       break;
              case 7 : printf("Override Inhibit Terminal Flag Bit"); 
                       break;
              case 8 : printf("Reset RT"); 
                       break;
              case 16 : printf("Transmit Vector Word"); 
                        break;
              case 18 : printf("Transmit Last Command"); 
                        break;
              case 19 : printf("Transmit Bit Word"); 
                        break;
              default : printf("inconnue T/R=1 N=%d", nombre);
            }
         }
       else
         { switch(nombre)
            { case 17 : printf("Synchronize"); 
                        break;
              case 20 : printf("Selected Transmitter Shutdown"); 
                        break;
              case 21 : printf("Override Selected Transmitter Shutdown"); 
                        break;
              default : printf("inconnue T/R=0 N=%d", nombre);
            }
         }
       printf(" RT%d", adresse);
     }
}





/*

Fragment pour tests ...

printf("\n");
printf("zd=0x%X\n", zd & 0xFFFF);
printf("0:%08X 1:%08X 2:%08X 3:%08X 4:%08X 5:%08X 6:%08X 7:%08X",
        IMA(zd+0), IMA(zd+1), IMA(zd+2), IMA(zd+3), IMA(zd+4),
        IMA(zd+5), IMA(zd+6), IMA(zd+7));
printf("\n");

*/
