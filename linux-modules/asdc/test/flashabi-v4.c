/************************************************************************
 * File             : initabi.c
 * Operating System : LynxOS/PowerPC V3.1
 *
 * History
 *
 * Edition Date     Comment                                        Par
 * ------- -------- ---------------------------------------------- ---
 *       1  3/12/01 creation                                       yg
 *
 *	   23/01/03 Inhibition du controle adressage en memoire
 *                  d'echange (flag "vflash")                      yg
 */

/* Chargement d'un fichier firmware SBS dans la memoire flash du coupleur */

#include <stdio.h> 
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "asdcctl.h"
#include "ln1.h"
 



/* 
 * Fonction pour lecture du fichier firmware SBS :
 *
 * Lecture d'1 mot (16 bits) code en ASCII hexa danf f (prealablement ouvert)
 *
 *   - Renvoi le mot si OK
 *   - Renvoi -1 si probleme (dont "fin du fichier" atteinte)
 *
 */
int lec1mot(FILE *f)
{
  char tmp[100];
  int val;
  int i;
  
  fgets(tmp, sizeof tmp, f);
  if (feof(f)) return -1;
  
  // printf("---%s", tmp);
  
  if (strlen(tmp) != 6)
    { printf("LEC1MOT : n=%d   tmp=\"%s\"\n", strlen(tmp), tmp);
      exit(-1);
    }
    
  tmp[4] = '\0';
  i = sscanf(tmp, "%x%c", &val);
  
  // printf("=== val : %08X   (%d)\n", val, val);
  
  if (i != 1)
    { printf("LEC1MOT : Mot bizarre = \"%s\"\n", tmp);
      exit(-1);
    }
  
  return val;
}



/*
 * Fonction pour arret sur probleme grave 
 *
 */
panique(char *msg)
{
  printf("\n\nFLASHABI : Panique %s\n\n");
  exit(-1);
}

 
 
/* 
 * Fonction pour effacer un secteur de la memoire flash du coupleur SBS :
 *
 *   - Renvoi 0 si OK
 *   - Renvoi -1 si probleme
 *
 */
int effacer_flash(int maniv, int secteur)
{
  int offset;
  int i, x;
  
   
  if ((secteur < 0) || (secteur > 3)) 
    { fprintf(stderr, "Appel de effacer_flash avec secteur=%d\n", secteur);
      return -1;
    }
  offset = secteur * 0x8000;
  
  /* Reset flash : on essaye de faire betement ce qui est dans la doc */
  ERAM(maniv, 0, 0xF);
  ERAM(maniv, 0x555, 0xAA);
  ERAM(maniv, 0x2AA, 0x55);
  ERAM(maniv, 0x555, 0x80);
  ERAM(maniv, 0x555, 0xAA);
  ERAM(maniv, 0x2AA, 0x55);
  ERAM(maniv, offset, 0x30);
  
  /* Verification du reset */
  for (;;)
     { x = LRAM(maniv, offset);
       if (x & 0x80) break;	/* Bit 7 est monte */
       else
         { if (x & 0x20)	/* Bit 5 est monte */
             { x = LRAM(maniv, offset);
               if (x & 0x80) break;	/* Bit 7 est monte */
               else { fprintf(stderr,
                              "Echec effacement secteur %d\n",
                              secteur);
                      return -1;
                    }
             }
           else
             { /* On recommence ... */
             }
         }
     }
   
  /* Verification mot a mot /
  for (i=0; i<0x8000; i++)
     { x = LRAM(maniv, offset + i) & 0xFFFF;
       if (x != 0xFFFF)
         { fprintf(stderr,
                   "Echec effacement secteur %d : sect(0x%X) = 0x%X\n",
                    secteur, i, x);
           return -1;
         }
     }
     
  /* Tout s'est bien passe */
  return 0;
}

 



/* 
 * Fonction de base pour ecrire un mot dans la memoire flash du coupleur SBS :
 *
 *   - Renvoi 0 si OK
 *   - Renvoi -1 si erreur
 *   - Renvoi 421 si time-out
 *
 */
int ecrire_flash_base(int maniv, int adresse, int mot)
{
  int offset, adr_rel, secteur;
  int i, j, x;
  
  secteur = adresse / 0x8000;
   
  if ((secteur < 0) || (secteur > 3)) 
    { fprintf(stderr, "Appel de effacer_flash avec secteur=%d\n", secteur);
      return -1;
    }
  offset = secteur * 0x8000;
  adr_rel = adresse % 0x8000;
  
  /* Ecriture mot dans flash :                          */
  /* On essaye de faire betement ce qui est dans la doc */
  ERAM(maniv, 0, 0xF);
  ERAM(maniv, 0x555, 0xAA);
  ERAM(maniv, 0x2AA, 0x55);
  ERAM(maniv, 0x555, 0xA0);
  ERAM(maniv, adresse, mot);
  
  
  /* Verification de l'ecriture */
  for (j=0; ; j++)
     { x = LRAM(maniv, adresse);
       if ((x & 0x80) ==(mot & 0x80)) break;	/* Bit 7 OK */
       else
         { if (x & 0x20)	/* Bit 5 est monte */
             { x = LRAM(maniv, adresse);
               if ((x & 0x80) ==(mot & 0x80)) break;	/* Bit 7 OK */
               else { fprintf(stderr,
                              "Echec ecriture 0x%X (secteur %d)\n",
                              adresse, secteur);
                      return -1;
                    }
             }
           else
             { /* On recommence ... */
             }
         }
         
       /* Time-out de second niveau */
       if (j>1000) 
         { 
           /* On refait un controle, au cas ou ... */
           usleep(20000);
           x = LRAM(maniv, adresse) & 0xFFFF;
           if (x != mot) return 421;
           
           /* On s'est miraculeusement rattrape !!!??? */
           printf("!"); fflush(stdout);
           return 0; 
         }
     }
   
  /* Verification par relecture du mot */
  // usleep(1000);		/* Petit delai, sinon probleme ... */
  x = LRAM(maniv, adresse) & 0xFFFF;
  if (x != mot)
    { fprintf(stderr,
              "Echec ecriture de 0x%X en 0x%X (relecture de 0x%X)\n",
              mot, adresse, x);
      return -1;
    }
    
  // printf(" %d", j); fflush(stdout);
     
  /* Tout s'est bien passe */
  return 0;
}







/* 
 * Fonction pour ecrire un mot dans la memoire flash du coupleur SBS :
 *
 *   - Renvoi 0 si OK
 *   - Renvoi -1 si probleme
 *
 */
int ecrire_flash(int maniv, int adresse, int mot)
{    int j, r;
 
     for (j=0; ; j++)
        { r = ecrire_flash_base(maniv, adresse, mot);
        
          if (r == 0) break;
          
          if (j > 100)
            { fprintf(stderr, "Time-out de niveau 2 : ");
              fprintf(stderr, "adr=0x%0X val=0x%0X\n", adresse, mot);
              break;
            }
        
          if (r == 421) printf("*"); 
             else       printf("%%");
          fflush(stdout); 
          usleep(1000);		/* Au cas ou ... */            
        }
        
     return r;
}
 




 
 
 
main(int argc, char **argv)
{

  int poignee;   
  int i, j, r, a, v;
  int *tv;
  
  char *device, *fichier;
  int adresse, nombre;
  char *p;
  char tmp[80];
  
  int csr;
  
  struct asdcparam par;  
  
   
  FILE *f;
  int x;
  short nh, nb;
  long n;
  
  
  if (argc != 3) 
    { fprintf(stderr, "Syntaxe : %s device fichier_firmware\n", argv[0]);
      exit(-1);
    }
  
  device = argv[1];
  fichier = argv[2];
  
  
  /* Ouverture du device */
  poignee = open(device, O_RDWR);
  if (poignee == -1)
    {
      perror("open ");
      exit(-1);
    } 
    
  
  /* Ouverture du fichier */
  f = fopen(fichier, "r");
    { if (f == NULL)
        { fprintf(stderr, "Impossible d'ouvrir \"%s\"\n", argv[1]);
          exit(-1);
        }
    }
  




  /*****************************************************/
  /***   Controle prealable de l'aspect du fichier   ***/
  /*****************************************************/
  
  /* Lecture 16 premiers mots */
  printf("\nEn-tete :\n"); 
  for (i=0; i<16; i++)
    { x = lec1mot(f);
      if (x<0)
        { fprintf(stderr, "Fin de fichier inattendue en phase 1\n");
          exit(-1); 
        }
      printf(" %04X", x & 0xFFFF); fflush(stdout);
      if (i == 7) printf("\n");
    }
  printf("\n");
  
  
  
  /* Lecture du nombre de mots de la section DSP */
  nh = lec1mot(f);
  if (nh<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 2a\n");
      goto pbfichier;
    }
  nb = lec1mot(f);
  if (nb<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 2b\n");
      goto pbfichier;
    }
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nSection DSP :\n");
  printf("   nh=0x%04X   nb=0x%04X   n=0x%08X (n=%d)\n", nh, nb, n, n);
  
  /* Lecture de la section DSP */
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if (x<0)
        { fprintf(stderr, "Fin de fichier inattendue en phase 2c\n");
          goto pbfichier;
        }
    }
    
    
    
    
  /* Lecture du nombre de mots de la section Altera */
  nh = lec1mot(f);
  if (nh<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 3a\n");
      goto pbfichier;
    }
  nb = lec1mot(f);
  if (nb<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 3b\n");
      goto pbfichier;
    }
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nSection Altera :\n");
  printf("   nh=0x%04X   nb=0x%04X   n=0x%08X (n=%d)\n", nh, nb, n, n);
  
  /* Lecture de la section Altera */
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if (x<0)
        { fprintf(stderr, "Fin de fichier inattendue en phase 3c\n");
          goto pbfichier;
        }
    }
  
  
  
  
  /* Controle fin du fichier atteinte */
  { x = lec1mot(f);
    if (x>=0)
      { fprintf(stderr, "Il reste des mots dans le fichier !\n");
        goto pbfichier;
      }
  }
  
  
  
  
  /**********************************/
  /***   Chargement de la flash   ***/  
  /**********************************/
  
  /* Rembobinage du fichier */
  fseek(f, 0, SEEK_SET);


  /* Inhibition du controle de la plage d'adressage dans la */
  /* memoire d'echange (pour eviter des milliers de lignes  */
  /* de messages d'erreur sur la console ...                */
  if (ioctl(poignee, ASDCVFLASH, 0) == -1)
    {
      perror("ioctl(ASDCVFLASH) ");
      fprintf(stderr,
              "Le driver ASDC ne serait-il pas anterieur a v4.9 ?\n");
      fprintf(stderr, "On continue quand meme ...\n\n");
    }

            
  /* Selection acces a la flash */
  ERAM(poignee, CSR, 0x3);
  usleep(10000000);		/* Attente 10 ms */
  printf("\n---   Acces flash ouvert   ---\n");
  
  
  /* Lecture (et saut) des 16 premiers mots (en-tete) */
  for (i=0; i<16; i++)
    { x = lec1mot(f);
      if (x<0) panique("(A)");
    }
    

  /* Lecture du nombre de mots de la section DSP */
  nh = lec1mot(f);
  if (nh<0) panique("(B)");
  nb = lec1mot(f);
  if (nb<0) panique("(C)");
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nEcriture section DSP : n=%d\n", n);
  printf("Effacement secteur 1 de la flash ..."); fflush(stdout);
    

  /* Effacement du secteur 0 de la flash */
  if (effacer_flash(poignee, 0))
    { fprintf(stderr, "Abandon de l'operation !\n");
      exit(-1);
    }
    
  printf(" OK\n"); fflush(stdout);
  

  /* Lecture de la section DSP et ecriture dans la flash */
  printf("Ecriture des donnees "); fflush(stdout);
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if (x<0) panique("(D)");
      r = ecrire_flash(poignee, i, x);
            
      if (r)
        { fprintf(stderr, "Abandon de l'operation !\n");
          exit(-1);
        }
               
      /* Pour faire patienter l'operateur ... */
      if (!(i%250)) { printf("."); fflush(stdout); }
    }
    
    
  printf(" OK\n"); fflush(stdout);
    
    
    
  /* Lecture du nombre de mots de la section Altera */
  nh = lec1mot(f);
  if (nh<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 3a\n");
      goto pbfichier;
    }
  nb = lec1mot(f);
  if (nb<0)
    { fprintf(stderr, "Fin de fichier inattendue en phase 3b\n");
      goto pbfichier;
    }
  n = ((nh & 0x0000FFFF) << 16) + (nb & 0x0000FFFF);
  
  printf("\nEcriture section Altera : n=%d\n", n);
  printf("Effacement secteurs 1 a 3 de la flash ..."); fflush(stdout);
  
  /* Effacement des secteurs 1 a 3 de la flash */
  if (effacer_flash(poignee, 1))
    { fprintf(stderr, "Abandon de l'operation !\n");
      exit(-1);
    }
  if (effacer_flash(poignee, 2))
    { fprintf(stderr, "Abandon de l'operation !\n");
      exit(-1);
    }
  if (effacer_flash(poignee, 3))
    { fprintf(stderr, "Abandon de l'operation !\n");
      exit(-1);
    }
    
  printf(" OK\n"); fflush(stdout);
  
  
  
  /* Lecture de la section Altera et ecriture dans la flash */
  printf("Ecriture des donnees ..."); fflush(stdout);
  for (i=0; i<n; i++)
    { x = lec1mot(f);
      if (x < 0) panique("(E)");
      r = ecrire_flash(poignee, i + 0x8000, x);
        
      if (r)
        { fprintf(stderr, "Abandon de l'operation !\n");
          exit(-1);
        }
       
      /* Pour faire patienter l'operateur ... */
      if (!(i%250)) { printf("."); fflush(stdout); }
    }
  
    
  printf(" OK\n"); fflush(stdout);
  
  
  
  /* Controle fin du fichier atteinte */
  { x = lec1mot(f);
    if (x>=0) panique("(F)");
  }
  
  
 /* Fermeture acces a la flash */
 r = LRAM(poignee, CSR);
 ERAM(poignee, CSR, r | 0xFFFD);
   
 printf("\n---   Acces flash ferme   ---\n");
 
 
 printf("\nChargement de la flash termine !\n");
 
 printf("\nATTENTION : IL FAUT IMPERATIVEMENT METTRE LE SYSTEME HORS\n");
 printf(  "            TENSION AVANT DE LE REBOUTER POUR POUVOIR\n");
 printf(  "            UTILISER LE FIRMWARE NOUVELLEMENT CHARGE !\n");
 
 exit(0); 
  
  
  
pbfichier :
  fprintf(stderr, "Le fichier firmware est anormal !\n");
  fprintf(stderr, "   ==> Abandon de l'operation !\n");
  exit(-1);
}



