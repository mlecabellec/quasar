/*******************************************************************/
/*                                                                 */
/*   AFFICHAGE DES STRUCTURES DU DRIVER ASDC LIEES AUX FLUX BC     */
/*                                                                 */
/*                                                                 */
/*                            Y. Guillemot, le   26 octobre 2001   */
/*                                   modif. le   7 novembre 2001   */
/*                    modif. pour asdc v4.8 le   7 novembre 2002   */
/*                   modif. pour asdc v4.12 le       3 aout 2004   */
/*                Mise en conformite gcc v4 le   15 fevrier 2008   */
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "asdcctl.h"

#include "ln1.h"



static void ecrit_tampon();
static void print_bc(int sbc);


int file_desc;

#define RAM(X) 		(LL_get_ram(file_desc, X))
#define IMA(X) 		(LL_get_image(file_desc, X))

#define IMAL(X) 	(LL_get_image(file_desc, X) & 0xFFFF)
#define IMAH(X) 	((LL_get_image(file_desc, X) >> 16) & 0xFFFF)



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

  struct asdcetapil etatpil;

  if (argc!=2)
           { printf("\nSyntaxe :   %s  periph.\n", argv[0]);
             exit(-1);
           }
           
                 

  if ((file_desc = open(argv[1],O_RDWR)) < 0)
                         { perror("open");
                           printf("\n Impossible d'ouvrir '%s'\n",argv[1]);
                           exit(1);
                         }
 
  printf("\n\nFlux BC de '%s' : \n", argv[1]);
         
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
  printf("nb tampons fluxs =        %d\n", etatpil.nombre_tampons_flux);
  printf("nb tampons fluxs dispos = %d\n", etatpil.nb_tamp_flux_dispos);
  printf("premier flux =            0x%04X\n", etatpil.pflux & 0xFFFF);
  printf("dernier flux =            0x%04X\n", etatpil.dflux & 0xFFFF);
  printf("\n");
   
  
  
  
  
  printf("\nChaine des flux :\n");
  printf("-----------------------------------------\n");
  for (j=0, i=etatpil.pflux; i; j++, i=IMAH(i+IMFSD)) 
     { printf ("%3d - ", j);
       for (k=i; k; k=IMAL(k+IMSUIV))
          { if (k!=i) printf("      ");
            print_bc(k);
          }
       
       m = IMA(i+IMPZD);
       printf("\t\tnmte = %d\n", IMA(m+IMNBMTE));
       printf("\t\tncte = %d\n", IMA(m+IMNBCTE));
       printf("\t\tnlte = %d\n", IMA(m+IMNBLTE));
       printf("\t\tnmts = %d\n", IMA(m+IMNBMTS));
       printf("\t\tncts = %d\n", IMA(m+IMNBCTS));
       printf("\t\tnlts = %d\n", IMA(m+IMNBLTS));
       
       /* Affichage des erreurs flux memorisees */
       {  int i, j, zd, n;
         
          zd = m;
           n = LIMA(file_desc, zd + IMNBERR);
          j = LIMA(file_desc, zd + IMERRFLX);
          printf("\nNb erreurs flux = %d\n", n);
          if (n)
            { printf("   --------------\n");
              for (i=0; i < (n > IMNBMAXERR ? IMNBMAXERR : n); i++)
                 { printf(" %d Err=0x%04X BC=0x%04X #ExecBC=%d #TrameMin=%d",
                           i, LIMA(file_desc, j) & 0xFFFF,
                           (LIMA(file_desc, j)>>16) & 0xFFFF,
                           LIMA(file_desc, j+2), LIMA(file_desc, j+3));
                   printf(" Arg1=0x%04X Arg2=0x%04X\n", 
                            LIMA(file_desc, j+1) & 0xFFFF,
                            (LIMA(file_desc, j+1)>>16) & 0xFFFF);
                   j += IMNBMPERR;     
                 }
             }
          printf("\n");     
       }
       
       
       
       
       printf("-----------------------------------------\n");
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
            
            


