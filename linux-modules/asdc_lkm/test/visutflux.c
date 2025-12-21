/*******************************************************************/
/*                                                                 */
/*                 MISE AU POINT DU DRIVER ASDC :                  */
/*           AFFICHAGE DES TAMPONS DES FIFO D'UN FLUX BC           */
/*                                                                 */
/*                                                                 */
/*                            Y. Guillemot, le   31 octobre 2001   */
/*                Mise en conformite gcc v4 le   15 fevrier 2008   */
/*                                   modif. le                     */
/*                                 derniere le                     */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "asdcctl.h"

#include "ln1.h"



static void ecrit_tampon();
static void print_bc(int sbc);


int file_desc;

#define RAM(X) 	(LL_get_ram(file_desc, X))
#define IMA(X) 	(LL_get_image(file_desc, X))



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
  int ibc;                    /* Indice premier emplacement libre dans memobc */
  int flux;

  struct asdcbc_tfch		tamp, *p;
  struct asdcbc_tfch		*pe, *de, *ps, *ds;
  struct asdcbcflux_etat	etat;

  if (argc!=3)
           { printf("\nSyntaxe :   %s  periph. Addr_hexa_flux \n", argv[0]);
             exit(-1);
           }
           
  sscanf(argv[2], "%x", &flux);              

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
 
  printf("\n\nFlux BC 0x%04X de '%s' : \n", flux, argv[1]);
         
  etat.flux = flux;         
  if (ioctl(file_desc, ASDCBCFLUX_ETAT_COMPLET, &etat))
    { perror("ioctl(ASDCBCFLUX_ETAT_COMPLET) ");
      exit(-1);
    }
  
  pe = (void *) etat.ppte;
  de = (void *) etat.pdte;
  ps = (void *) etat.ppts;
  ds = (void *) etat.pdts;
  
  
  printf("\n");
  printf("Pointeurs : pe = 0x%08X\n", pe);
  printf("            de = 0x%08X\n", de);
  printf("            ps = 0x%08X\n", ps);
  printf("            ds = 0x%08X\n", ds);
  printf("\n\n");

  printf("FIFO en entree :\n\n");
  
  for (k=0, p=pe; p; k++)
    { tamp.s = p;
      if (ioctl(file_desc, ASDCBCFLUX_LTAMPON, &tamp))
        { perror("ioctl(ASDCFLUX_LTAMPON) ");
          exit(-1);
        }
      
      printf("%3d - 0x%08X\n", k, p);
      printf("   type=0x%04X   cmd1=0x%04X   sts1=0x%04X   err=0x%04X\n",
              tamp.tamp.type & 0xFFFF, tamp.tamp.cmd1 & 0xFFFF,
              tamp.tamp.sts1 & 0xFFFF, tamp.tamp.err & 0xFFFF);
      n = tamp.tamp.cmd1 & 0x1F;
      if (!n) n = 32;
      printf("   ");
      for (i=0; i<n; i++) printf("%04X ", tamp.tamp.d[i] & 0xFFFF);
      printf("\n\n");
      
      p = tamp.s;     
    }
     

  printf("\n\n");
  printf("FIFO en sortie :\n\n");
  
  for (k=0, p=ps; p; k++)
    { tamp.s = p;
      if (ioctl(file_desc, ASDCBCFLUX_LTAMPON, &tamp))
        { perror("ioctl(ASDCBCFLUX_LTAMPON) ");
          exit(-1);
        }
      
      printf("%3d - 0x%08X\n", k, p);
      printf("   type=0x%04X   cmd1=0x%04X   sts1=0x%04X   err=0x%04X\n",
              tamp.tamp.type & 0xFFFF, tamp.tamp.cmd1 & 0xFFFF,
              tamp.tamp.sts1 & 0xFFFF, tamp.tamp.err & 0xFFFF);
      n = tamp.tamp.cmd1 & 0x1F;
      if (!n) n = 32;
      printf("   ");
      for (i=0; i<n; i++) printf("%04X ", tamp.tamp.d[i] & 0xFFFF);
      printf("\n\n");
      
      p = tamp.s;     
    }
     
     
    
     
  printf("\n");   
     
     
     
  close(fd);
  exit(0);
} 





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




            
            


