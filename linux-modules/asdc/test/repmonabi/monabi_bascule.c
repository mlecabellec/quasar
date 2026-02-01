






/* Syntaxe :        swap [pseud0_fichier]

   Force une carte ABI a basculer ses tampons d'espionnage sequentiel
   
   Le pseudo-device est "/dev/asdc1" par defaut
   
   				                    Anonymized, le 6/01/1994
							   modif. le 4/10/1994
				     Adaptation LynxOS et ABIPMC2 le 3/08/2001
						  derniere modif. le
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "asdcctl.h"
#include "ln1.h"


char *device = "/dev/asdc1";

main(argc, argv)
int argc;
char **argv;
{




  /* Variables liees a la carte ABI */
  int fasdc;		/* Manivelle d'acces */
  union masdc *a;	/* Pointeur memoire d'echange */
  caddr_t uuu;		/* Utilisation temporaire */

  /* Premier argument permet de passer nom de device autre que le defaut */
  if (argc > 1) device = argv[1];


  printf("SWAP : Force le basculement des tampons \"moniteur sequentiel\"\n");
  printf("       de la carte ABI \"%s\"\n", device);

  /* Ouverture du peripherique */
  if ((fasdc=open(device,O_RDWR)) < 0)
			 { perror("open");
			   printf("\n Impossible d'ouvrir '%s'\n",device);
			   exit(1);
			 }


  /* "mappage" du peripherique en memoire
  uuu = (caddr_t)mmap((caddr_t)0, ASDC_TAILLEMAX*2, PROT_READ|PROT_WRITE,
			   MAP_SHARED, fasdc, 0);
  if (uuu == (caddr_t) -1)
			 { perror("mmap");
			   exit(1);
			 }

  a = (union masdc *) uuu;
  */


  /* Affichage de la version du microcode */
  printf("\nVersion microcode (significatif seulement apres initialisation)\n");

  /* printf("NUMVER = %d   NUMODI = %d\n\n", a->r.numver, a->r.numodi); */
  printf("Version provisoire ...\n");


   /* Forcage du basculement avec IT */
   if (ioctl(fasdc, ASDCESPSBASC, 0))
     { perror("ioctl(ASDCESPSBASC) ");
       exit(-1);
     }


   exit(0);

}

