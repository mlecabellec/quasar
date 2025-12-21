/************************************************************************
 * File             : simabol.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * Connexion des CC recues par un abonne simule a un CEVT
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  4/10/02 creation                                       yg
 *
 *
 */


#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"




struct cocos { char  trbit;	/* Bit T/R 	(2 si non defini) */
               char  bra;	/* Broadcast allowed (2 si commande illegale) */
               char * mnemo;	/* Surnom de la commande */
               char * nom;	/* Nom "en clair" de la commande */
             };



struct cocos sdc_coco[]
  = { { 1, 0, "GESDYN",  "Dynamic Bus Control" },		/* Code 0x00 */
      { 1, 1, "SYNC",    "Synchronize" },			/* Code 0x01 */
      { 1, 0, "DSTATUS", "Transmit Status" },			/* Code 0x02 */
      { 1, 1, "TEST",    "Initiate Self Test" },		/* Code 0x03 */
      { 1, 1, "TS",      "Transmitter Shutdown" },		/* Code 0x04 */
      { 1, 1, "OTS",     "Override Transmitter Shutdown" },	/* Code 0x05 */
      { 1, 1, "ITFB",    "Inhibit Terminal Flag Bit" },		/* Code 0x06 */
      { 1, 1, "OITFB",   "Override Inhibit Terminal Flag Bit" },/* Code 0x07 */
      { 1, 1, "REINIT",  "Reset RT" },				/* Code 0x08 */
      { 1, 0, "LP",      "Lancement Programme" },		/* Code 0x09 */
      { 1, 0, "ABORT",   "Arrete Programme" },			/* Code 0x0A */
      { 1, 1, "FLASH",   "Simulation Flash" },			/* Code 0x0B */
      { 1, 0, "WMSO",    "Autorisation Ecriture en MSO" },	/* Code 0x0C */
      { 1, 0, "VALID",   "Valide la Fonction Gerant" },		/* Code 0x0D */
      { 1, 0, "INHIB",   "Inhibe la Fonction Gerant" },		/* Code 0x0E */
      { 1, 0, "MODGER",  "Autorise Changement Mode gerant" },	/* Code 0x0F */
      { 1, 0, "VECTEUR", "Transmit Vector Word" },		/* Code 0x10 */
      { 0, 1, "SD",      "Synchronize (with data word)" },	/* Code 0x11 */
      { 1, 0, "DCDE",    "Transmit Last Command" },		/* Code 0x12 */
      { 1, 0, "BIT",     "Transmit Built In Test Word" },	/* Code 0x13 */
      { 0, 1, "STS",     "Selected Transmitter Shutdown" },	/* Code 0x14 */
      { 0, 1, "OSTS",    "Override Selected Transmitter Shutdown" }, /* 0x15 */
      { 0, 1, "RES",     "Reserve" },				/* Code 0x16 */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x17 */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x18 */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x19 */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x1A */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x1B */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x1C */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x1D */
      { 2, 2, "?????",   "Commande Illegale" },			/* Code 0x1E */
      { 2, 2, "?????",   "Commande Illegale" }			/* Code 0x1F */
    };       


 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, j, k, a, n;
  int *tv;
  
  char *device;
  int adr, sa, sens, nombre;
  char *p;
  char tmp[80];
  
  struct asdcvoiecc v;  
  
  int cevt; 
  
  int coco[32];
  
  
  if (argc < 5)
    { goto syntaxe;
    }
  
  device = argv[1];
  
  cevt = strtol(argv[2], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" du numero de CEVT est anormale.\n",
                      argv[0], argv[2]);
      goto syntaxe;
    }
  
  adr = strtol(argv[3], &p, 0);
  if (*p != '\0')
    { fprintf(stderr, "Valeur \"%s\" de l'adresse est anormale.\n", 
                      argv[0], argv[3]);
      goto syntaxe;
    }
    
  /* RAZ de la table des CC a traiter */  
  for (i=0; i<32; i++) coco[i] = 0;
   
  /* Saisie des CC a traiter */
  for (i=4, j=0; i<argc; i++, j++)
    { 
      k = strtol(argv[i], &p, 0);
      if (*p != '\0')
        { fprintf(stderr, "Valeur \"%s\" d'un numero de CC est anormale.\n",
                      argv[0], argv[i]);
          goto syntaxe;
        }
        
      coco[k & 0x1F]++;
    }

  /* Remise au complet des codes CC */
  for (i=0; i<32; i++)
     { if (coco[i])
         { coco[i] = i | ((sdc_coco[i].trbit & 1) ? 0x400 : 0);
         }
     }


  
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    


  /* Connexions au CEVT */
  for (i=0; i<32; i++)
     { if (coco[i])
         {
           v.adresse = adr;
           v.coco = coco[i];
           v.cevt = cevt;
           
           if (ioctl(poignee, ASDCEVT_CC_AJOUTER, &v) == -1)
             { perror("ioctl(ASDCEVT_CC_AJOUTER) ");
               fprintf(stderr, "Echec acces RT%d, CC=0x%X (%s), CEVT%d\n",
                       adr, v.coco, sdc_coco[i].mnemo, cevt);
               exit(-1);
             }
             
            printf(" %s", sdc_coco[i].mnemo);
         }
     }
  printf("\n");


  
  
  printf("C'est fini !\n");    
 
      
  close(poignee);
  exit(0);



syntaxe :
  fprintf(stderr,
          "\nSyntaxe : %s device num_CEVT rt cc1 [cc2 [cc3 ...]]\n\n", 
          argv[0]);
  fprintf(stderr,
          "   Il suffit de donner les codes numeriques (5 bits) des CC\n");
  exit(-1);
  
  
}



