/************************************************************************
 * File             : alimflux.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Tache alimentant la sortie d'un flux BC en donnees (depuis un fichier)
 * et recuperant les donnees d'entrees de ce flux (vers un autre fichier)
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1 2/11/01 creation                                        yg
 *
 *
 */
 
 
 
/* 
   Ce logiciel doit etre lance apres la definition de la trame et du flux,
   mais avant le debut de l'execution de la trame.
   
   Il a besoin, en entree (sur la ligne de commande) :
   
     - Du "device" associe au coupleur ABI
     - De l'adresse du flux dans le coupleur
     - Du nom du fichier a lire (donnees en sortie du flux)
     - Du nom du fichier a creer/ecrire (donnees en entree du flux)
     
*/


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
  int flux;
  char *p;
  char tmp[80];
  char *fich_lire, *fich_ecrire;
  
  FILE *flire, *fecrire;
  
  struct asdcbbc b;
  struct asdctampbc t;
  MOT debut_trame;
  
  int nombre_exec;
  unsigned long periode;
  
  struct asdcbcflux_aj f;
  struct asdcbcflux_etat e;
  
  struct asdcbc_tf *tampon;
  struct asdcbcflux flx;
 
  int nbmte, nbmts, nbcte, nbcts, nblte, nblts;  

  
  
  
  /* Recuperation des parametres */
  
  if (argc != 5) 
    { fprintf(stderr, 
              "Syntaxe : %s device adresse_hexa_flux fich_lire fich_ecrire\n", 
              argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  
  sscanf(argv[2], "%x", &flux);
  
  fich_lire = argv[3];
  fich_ecrire = argv[4];
  
  
  
  
  
  /* Ouverture des fichiers et peripheriques */
  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
  flire = fopen(fich_lire, "r");
  if (flire == NULL)
    {
      fprintf(stderr, "Echec ouverture de \"%s\"\n", fich_lire);
      exit(-1);
    } 
    
  fecrire = fopen(fich_ecrire, "w+");
  if (fecrire == NULL)
    {
      fprintf(stderr, "Echec creation/ouverture de \"%s\"\n", fich_ecrire);
      exit(-1);
    } 
    
  
  
  
  /* Lecture des caracteristiques du flux */    
    
    
  e.flux = flux;         
  if (ioctl(poignee, ASDCBCFLUX_ETAT, &e))
    { perror("ioctl(ASDCBCFLUX_ETAT) ");
      exit(-1);
    }
    
  nbmte = e.nbmte;
  nbmts = e.nbmts;
  
  nbcte = e.nbcte;  
  nbcts = e.nbcts;  
  
  printf("Nombre de tampons dispos : %d en entree et %d en sortie\n",
          nbmte, nbmts);
          
  if (nbcte || nbcts)
    { fprintf(stderr, "\n");
      fprintf(stderr, "Nbre courant tampons en entree = %d\n", nbcte);
      fprintf(stderr, "Nbre courant tampons en sortie = %d\n", nbcts);
      fprintf(stderr, "\nCes 2 valeurs auraient du etre nulles !\n\n");
      exit(-1);
    }
    
  if (nbmte < 5)
    { fprintf(stderr, "Trop peu de tampons en entree !\n");
      exit(-1);
    }  
    
  if (nbmts < 5)
    { fprintf(stderr, "Trop peu de tampons en sortie !\n");
      exit(-1);
    }  
    



  /* Allocation d'un tampon de taille suffisante pour les E/S */
  n = (nbmte > nbmts) ? nbmte : nbmts;
  tampon = (struct asdcbc_tf *) calloc(n, sizeof(struct asdcbc_tf));
  if (tampon == NULL)
    { fprintf(stderr, "Echec allocation %d x %d octets en memoire !\n",
                       n, sizeof(struct asdcbc_tf));
      exit(-1);
    }
    
    
    
    
  /* Avertissement */
  
  printf("\n");             
  printf("ATTENTION : Les tampons materiels des transferts BCRT de ce flux\n");
  printf("            doivent avoir ete initialises par l'application ayant\n");
  printf("            ce flux !\n");
  printf("\n");            
  printf("            Dans le cas contraire, le premier message emis par\n");
  printf("            chacun des blocs BC de type BCRT contiendra des \n");
  printf("            donnees imprevisibles ...\n");
  printf("\n");             
  
  
  
  /* Parametrage des modes de reveil du flux */  
    
  e.flux = flux;
  e.nblte = nbmte / 2;
  e.nblts = nbmts / 2;
  e.evt = FLX_EVTLIME | FLX_EVTLIMS;	/* Tous types de reveils ! */
  e.evt |= FLX_ATSERR;			/* Forcage arret trame si erreur flux */
  if (ioctl(poignee, ASDCBCFLUX_REGLER, &e))
    { perror("ioctl(ASDCBCFLUX_REGLER) ");
      exit(-1);
    }
    
   
  
  
  /* Ecriture de tous les tampons possibles en sortie du flux */
  /* (sauf les tampons materiels : normalement, cette tache   */
  /*  devrait aussi initialiser les tampons materiels, mais,  */
  /*  comme elle ne dispose pas de la description detaillee   */
  /*  flux, elle ne le fait pas. Ce travail est laisse a la   */
  /*  charge de l'application ayant defini la trame et le     */
  /*  flux)                                                   */
  
  
  /* Lecture de nbmts tampons dans le fichier fich_lire */
  n = fread(tampon, sizeof(struct asdcbc_tf), nbmts, flire);
     
  if (n < nbmts) { /* Fin du fichier fich_lire */
                   fclose(flire);
                   flire = NULL;     /* Indicateur fin du fichier atteinte */  
                 }
    
  /* Ecriture dans le flux */
  flx.flux = flux;  
  flx.nbtt = n;
  flx.z = tampon;
  if (ioctl(poignee, ASDCBCFLUX_ECRIRE, &flx))
    { perror("ioctl(ASDCBCFLUX_ECRIRE) - 1 ");
      exit(-1);
    }
    
  
  
 
  /* Avertissement operateur */
  printf(" Initialisation achevee, pret a traiter ! ...\n");
  fflush(stdout);
  
  /* Attente debut trame */
  if (ioctl(poignee, ASDCAGOTBC, 0))
    { perror("ioctl(ASDCAGOTBC) ");
      exit(-1);
    }
  printf("On y va !\n"); fflush(stdout);
 
 
  /* Boucle de traitement */
  for (;;)
     {
       /* Attente evenement */
       e.flux = flux;  
       if (ioctl(poignee, ASDCBCFLUX_ATTENDRE, &e))
         { perror("ioctl(ASDCBCFLUX_ATTENDRE)");
           exit(-1);
         }

       // printf("\n");
       // printf("evt = 0x%04X\n", e.evt & 0xFFFF);
       // printf("\n");
       // printf("nbcte = %d\n", e.nbcte);
       // printf("nblte = %d\n", e.nblte);
       // printf("\n");
       // printf("nbcts = %d\n", e.nbcts);
       // printf("nblts = %d\n", e.nblts);
       // printf("\n");
       
       if (e.evt & FLX_ERRF)
         { 
           /* Affichage des erreurs flux memorisees */
           int i, j, zd, n;
         
           fprintf(stderr, "--- ERREUR FLUX ---\n");
           zd = LIMA(poignee, flux + IMPZD);
           n = LIMA(poignee, zd + IMNBERR);
           j = LIMA(poignee, zd + IMERRFLX);
           printf("%d Erreur%s\n", n, n>1 ? "s" : "");
           for (i=0; i < (n > IMNBMAXERR ? IMNBMAXERR : n); i++)
              { printf(" %d Err=0x%04X BC=0x%04X #ExecBC=%d #TrameMin=%d",
                        i, LIMA(poignee, j) & 0xFFFF,
                        (LIMA(poignee, j)>>16) & 0xFFFF,
                        LIMA(poignee, j+2), LIMA(poignee, j+3));
                printf(" Arg1=0x%04X Arg2=0x%04X\n", 
                         LIMA(poignee, j+1) & 0xFFFF,
                         (LIMA(poignee, j+1)>>16) & 0xFFFF);
                j += IMNBMPERR;     
              }
            exit;
         }
       
       /* Traitement eventuel des entrees */
       if ((e.evt & FLX_LIME) || (e.evt & FLX_FTRAME))
         { n = e.nbcte;
         
           /* Lecture dans flux de tous les tampons disponibles */
           flx.flux = flux;  
           flx.nbtt = n;
           flx.z = tampon;
           if (ioctl(poignee, ASDCBCFLUX_LIRE, &flx))
             { perror("ioctl(ASDCBCFLUX_LIRE) ");
               exit(-1);
             }
             
       // printf("lus %d --> %d\n", n, flx.nbtet);
       
           /* Ecriture sur le disque des tampons lus */
           fwrite(tampon, sizeof(struct asdcbc_tf), n, fecrire);
           
           
      /* Marquage pour debug ... 
         tampon[0].type = 0x5A5A;
         fwrite(tampon, sizeof(struct asdcbc_tf), 1, fecrire);
      */
      
           
           // printf("flux --> fichier\n"); fflush(stdout);
         }
         
       /* Traitement eventuel des sorties */
       if ((flire != NULL) && (e.evt & FLX_LIMS))
         { n = nbmts - e.nbcts;
         
           /* Lecture de n tampons dans le fichier fich_lire */
           i = fread(tampon, sizeof(struct asdcbc_tf), n, flire);
     
           if (i < n) { /* Fin du fichier fich_lire */
                        fclose(flire);
                        flire = NULL;  /* Indicateur fin du fichier atteinte */
                        n = i;  
                      }
    
           /* Ecriture dans le flux */
           flx.flux = flux;  
           flx.nbtt = n;
           flx.z = tampon;
           if (ioctl(poignee, ASDCBCFLUX_ECRIRE, &flx))
             { perror("ioctl(ASDCBCFLUX_ECRIRE) - 2 ");
               exit(-1);
             }
             
           // printf("fichier --> flux\n"); fflush(stdout);
          }
        
       /* Fin de la boucle de traitement si trame achevee */
       if (e.evt & FLX_FTRAME) break;
       
     }
 


  if (flire) fclose(flire);
  fclose(fecrire);
  close(poignee);
  
  printf("C'est fini !\n");
      
  exit(0);
}



