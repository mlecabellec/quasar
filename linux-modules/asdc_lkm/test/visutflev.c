/*******************************************************************/
/*                                                                 */
/*    AFFICHAGE DES TAMPONS D'UN FLUX D'EVENEMENTS RT              */
/*                                                                 */
/*                                                                 */
/*                            Y. Guillemot, le      10 juin 2002   */
/*                                   modif. le                     */
/*                                 derniere le                     */
/*******************************************************************/

#include <stdio.h>
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
  int flux;
  
  int i, j, k, m, l, n, o;
  int sbc;	/* "adreSse bloc BC" ou "Suivant bloc BC" */
  int tybc;	/* TYpe bloc BC */
  int abbcs;	/* Adresse Bloc Bc Suivant */
  int ibc;                    /* Indice premier emplacement libre dans memobc */
  
  struct asdcev_tfch *p, *q;
  struct asdcev_tfch tfch;
  
  struct asdcetapil etatpil;

  if (argc!=3)
           { printf("\nSyntaxe :   %s  periph.  adresse_flux_evt\n", argv[0]);
             exit(-1);
           }
           
                 

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
                         
  flux = strtol(argv[2], NULL, 0);
 
  printf("\n\nFlux Evt 0x%04X de '%s' : \n", flux & 0xFFFF, argv[1]);
         
  if (ioctl(file_desc, ASDCLETAT, &etatpil))
    { perror("ioctl(ASDCLETAT) ");
      exit(-1);
    }
  
  printf("\n");
  printf("nb open =                 %d\n", etatpil.asdc_nbopen);
  printf("nb ITs =                  %d\n", etatpil.asdc_nombre_it);
  printf("monok =                   %d\n", etatpil.asdc_monok);
  printf("fin bc =                  %d\n", etatpil.asdc_fin_bc);
  printf("stocke bc =               %d\n", etatpil.asdc_stocke_bc);
  printf("\n");
  printf("nb tampons flux evt =     %d\n", etatpil.nombre_tampons_flux_ev);
  printf("nb tampons flux dispos =  %d\n", etatpil.nb_tamp_flux_ev_dispos);
  printf("premier flux evt =        0x%04X\n", etatpil.pfluxev & 0xFFFF);
  printf("dernier flux evt =        0x%04X\n", etatpil.dfluxev & 0xFFFF);
  printf("\n");
    
 
  if (IMA(flux + IRFLXE) != flux)
    {
      fprintf(stderr, "0x%04X ne correspond pas a un flux \n", flux);
      exit(-1);
    } 

  printf("Tampons disponibles : %d sur %d\n", 
         IMA(flux + IRNBCTEV), IMA(flux + IRNBMTEV));
         
         printf("IRPPTEV=%d\n", IRPPTEV);  fflush(stdout);
            
  /* Parcours de la chaine des tampons */  
  k = IMA(flux + IRPPTEV);      
  printf("PPTEV = 0x%X\n", k); fflush(stdout);
  
/*
  for (i=0; k; i++)
    {
       printf("%3d : ", i); fflush(stdout);
       
       (int) tfch.s = k;
       
       if (ioctl(file_desc, ASDCRTEFLUX_LTAMPON, &tfch) == -1)
         { perror("ioctl(ASDCRTEFLUX_LTAMPON) ");
           exit(-1);
         }
              
       printf("cmd=0x%04X  err=0x%X\n", tfch.tamp.cmd, tfch.tamp.err); 
       fflush(stdout);
       
       k = (int) tfch.s;
    }
*/    
    
  (int) tfch.s = k;
  if (ioctl(file_desc, ASDCRTEFLUX_LTAMPON, &tfch) == -1)
    { perror("ioctl(ASDCRTEFLUX_LTAMPON) ");
      exit(-1);
    }
     
     
  printf("\n");   
     
     
     
  close(fd);
  exit(0);
} 



static void ecrit_tampon(p)
int p;
{  int i, j;
   unsigned short int q;
   p = p & 0xFFFF;  /* Au cas ou ... */
   q = p;
   do { printf("\t0x%04X 0x%04X\n", 0xFFFF&RAM(q), 0xFFFF&RAM(q+1));
        for(i=q+2; i<q+27; i+=8)
           { printf("\t");
             for(j=0; j<8; j++) printf("0x%04X ", RAM(i+j)&0xFFFF);
             printf("\n");
           }
        q=RAM(q);
      } while(q!=p);
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




/* Affichage d'un bloc BC */
void print_bc(int sbc)
    { 
          int tybc;
                   
          tybc = RAM(sbc);
          switch(tybc & 0xFF)
            { case 0 : printf("0x%04X : 0x%04X ==> Commande codee\n", 
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande  (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
              case 1 : printf("0x%04X : 0x%04X ==> BC->RT\n",
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande   (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
              case 2 : printf("0x%04X : 0x%04X ==> RT->BC\n",
                                                  sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande   (",
                                                   RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
              case 3 : printf("0x%04X : 0x%04X ==> RT->RT\n",
                                           sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande 1  (",
                                                   RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
                       
              case 5 : printf("0x%04X : 0x%04X ==> Diffusion BC->RT\n",
                                             sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande  (",
                                                     RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
              case 6 : printf("0x%04X : 0x%04X ==> Diffusion RT->RT\n",
                                              sbc & 0xFFFF, tybc & 0xFFFF);
                       printf("         0x%04X  commande 1  (",
                                                      RAM(sbc+1) & 0xFFFF);
                       decode_comm(RAM(sbc+1));
                       printf(")\n");
                       break;
                       
                       
              default : printf("0x%04X : 0x%04X ==> *** CODE ANORMAL ***\n",
                                                                    sbc, tybc);
                       break;
            }
    }            
            
            


