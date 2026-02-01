/*******************************************************************/
/*   MONABI : Logiciel d'espionnage 1553 utilisant une carte ABI   */
/*                                                                 */
/*                                                                 */
/*                              Anonymized, le 13 janvier 1994   */
/*                                    modifie le    30 mars 1994   */
/*                            derniere modif. le    31 mars 1994   */
/*                                                                 */
/*   Version 2.1 : Adaptation aux cartes ABI-V3 le  22 juin 1994   */
/*   Version 2.2 : Accepte sans erreur des temps                   */
/*        de reponse abonnes jusqu'a 22us     le 27 juillet 1994   */
/*                            derniere modif. le  4 octobre 1994   */
/*                                                                 */
/*   Version 2.3 : - Horloge datation variable                     */
/*                 - Initialisation carte ABI par initabi          */
/*                   (carte ABI-V3 et ucode >= V4.1 obligatoires)  */
/*                                     modif. le     7 mars 1995   */
/*                            derniere modif. le     9 mars 1995   */
/*                                                                 */
/*   Version 2.4 : - Possibilite choix du microcode via option -h  */
/*                                     modif. le 18 juillet 1995   */
/*                                                                 */
/*   Version 2.5 : - Correction bug sur valeur de RSPGPA           */
/*                                     modif. le    10 mars 1997   */
/*                            derniere modif. le                   */
/*                                                                 */
/*   Version 3.1 : - Adaptation a abi-pmc2 sous LynxOS/PowerPC     */
/*                                     modif. le     18 mai 2001   */
/*                            derniere modif. le     3 aout 2001   */
/*******************************************************************/


/* Dans cette version, INITABI n'est plus jamais appele automatiquement 
   par MONABI  ==>  Ceci permet plus de souplesse et de clarte dans le 
                    fonctionnement multitache.
               ==>  En revanche, il faut penser a appeler explicitement
                    initabi en mode monotache ...
*/


/* Syntaxe :       
               MONABI [-d device] [-f fich_params] [-b] [-s] 
					      [-v] [-C] [-h] [fichier]
   
                    -d : Permet de specifier la carte a utiliser
  ---> Remplacer par un numero de voie ou mnemonique en arg. (pas en option)
                    
                    -f : Permet de specifier le fichier de parametres devant
                         etre lu par initabi
  ---> Option a supprimer
                    
                    -b : Ecriture par fonctions E/S bas niveau (appels systemes)
                         (par defaut : ecriture avec bufferisation)
                         
                    -s : Ecriture sur la sortie standard et non pas dans fichier
                    
		    -v : Affichage de messages d'information en cours de travail
                    
                    -V : Idem -v avec, en plus, appel de initabi avec option -v
  ---> Option a supprimer
                    
                    -C : Conservation de l'heure (pas de RAZ horloge datation)
  ---> Option a supprimer (espion utilise temps IRIG)
                    
                    -h : Passage option -h a initabi (microcode "haut")
  ---> Option a supprimer
                    
                    fichier : fichier a ecrire (par defaut : "espion.dat")
                    
   ==> Options a ajouter :
             - Affichage version du logiciel
             - Choix de la priorite d'execution
                    
*/


#define NOM	"MONABI"
#define VERSION_MAJEUR	3
#define VERSION_MINEUR	0
#define DATE    "3/8/2001"

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "asdcctl.h"
#include "ln1.h"

#include "monabi.h"

/* Noms des logiciels auxilliaires */
#define LBASCULE	"monabi_bascule"

/* Variables globales : */
char *device;	  /* Nom du pseudo-fichier d'acces au pilote de la carte  */
char *fprm;	  /* Nom du fichier de parametrage carte ABI (via initabi) */
int v;	    /* Indic. : 0 ==> Pas de messages, 1 ==> messages d'information */
int vv;	    /* Idem, mais pour le logiciel d'initialisation ABI */
int razh;   /* Indic. RAZ horloge datation a effectuer */
int mc_haut;	/* Option "microcode haut" */
int vidange = 0;    /* Indicateur "lecture de tampon en cours" */
int terminer = 0;   /* Indicateur "fin de l'espionnage demande" */

/* Options pour traiter les messages des logiciels auxilliaires : */
char *opt_msg = "";
char *opt_rien = "> /dev/null";
  




/* Fonction executee quand le signal SIGINT est recu */
/* (c'est a dire quand la touche ^C est pressee)     */
void exec_ctrlc()
{      
  char tmp[200];
  
  /* Creation d'une commande shell pour swapper les tampons */
  /*   - La commande est executee dans un sous-process      */
  /*   - Elle est precedee d'une temporisation              */
  /*       ==> Ceci pour eviter qu'IT ne survienne avant    */
  /*           retour du prog. principal dans l'ioctl       */
  /*           d'attente IT (ce qui donne une erreur        */
  /*           de debordement des tampons)                  */
  sprintf(tmp, "(sleep 1 ; %s %s %s ) &",
                LBASCULE, device, v ? opt_msg : opt_rien);  

  if (v) printf("appel shell : \"%s\"\n", tmp);
                   
  /* Leve l'indicateur demandant la sortie de la boucle */
  terminer = 1;
  
  /* Lance le sous-processus qui doit forcer un basculement du tampon */
  /* (pour permettre la lecture des derniers messages recus) si une   */
  /* lecture n'est pas en cours                                       */
  /* [si une lecture etait en cours, le basculement entrainerait une  */
  /*  mauvaise lecture (melange entre les 2 tampons)]                 */
  if (!vidange) system(tmp);
}









main(argc, argv)
int argc;
char **argv;
{

  extern char *optarg;
  extern int optind, opterr;
  
  char *fichier;  /* Nom du fichier de sortie */
  FILE *sortie2;  /* Descripteur du fichier a utiliser en sortie (defaut) */
  int sortie1;    /* Manivelle du fichier a utiliser en sortie (option -b) */
  int nes;	  /* Niveau d'E/S (1 ou 2); */
  int fo;	  /* 0 ==> Usage sortie standard, 1 ==> fichier a ouvrir */
  
  int c, r;
  
  /* Variables liees a la carte ABI */
  int fasdc;		/* Manivelle d'acces */
  union masdc *a;	/* Pointeur memoire d'echange */
  caddr_t uuu;		/* Utilisation temporaire */
  struct asdcparam parm;
  int atampon;          /* Adresse (dans la carte ABI) du tampon a vider */

  MOT *zone;            /* Pointe la zone (memoire UNIX) ou ranger le tampon */
  MOT *p1, *p2;         /* Pointeurs "de travail" */
  int i, n;             /* Variables "de travail" */
  
  /* Initialisation des indicateurs et options a "non encore impose" */
  device = fprm = fichier = NULL;
  sortie2 = NULL;
  sortie1 = 0;
  nes = 0;
  v = vv = 0;
  razh = 1;
  mc_haut = 0;

  while ((c = getopt(argc, argv, "CVbd:f:hsv")) != -1)
     switch(c)
	{ case 'C' : if (!razh) goto erreur_syntaxe;
		     razh = 0;
		     break;
	  case 'b' : if (nes) goto erreur_syntaxe;
		     nes = 1;
		     break;
	  case 'd' : if (device != NULL) goto erreur_syntaxe;
		     device = optarg;
		     break;
	  case 'f' : if (fprm != NULL) goto erreur_syntaxe;
		     fprm = optarg;
		     break;
	  case 'h' : if (mc_haut) goto erreur_syntaxe;
		     mc_haut = 1;
		     break;
	  case 's' : if (fichier != NULL) goto erreur_syntaxe;
		     fichier = "<stdout>";
		     sortie2 = stdout;
		     sortie1 = fileno(stdout);
		     break;
	  case 'v' : if (v) goto erreur_syntaxe;
                     v = 1;
                     break;
          case 'V' : if (vv) goto erreur_syntaxe;
                     vv = v = 1;
                     break;
          case '?' : goto syntaxe;
        }

  fo = 0;  
  
  if (optind < argc)
    { if (fichier != NULL) goto erreur_syntaxe;
      fichier = argv[optind];
      fo = 1;	/* Indique qu'il faut ouvrir un fichier */
      optind++;
    }

  if (optind != argc) goto erreur_syntaxe;
  
  if (device == NULL) device = "/dev/asdc1";
  if (fprm == NULL) fprm = "";
  if (fichier == NULL) 
       { fichier = "espion.dat";
         fo = 1;	/* Indique qu'il faut ouvrir un fichier */
       }
  if (!nes) nes = 2;
   
  fprintf(stderr, "\n%s - Version %d.%d du %s\n\n", 
                   NOM, VERSION_MAJEUR, VERSION_MINEUR, DATE);
  fprintf(stderr, "   ---> device =  \"%s\"\n", device);
  fprintf(stderr, "   ---> fichier = \"%s\"\n", fichier);
  if (v) { fprintf(stderr, "   ---> nes =     %d\n", nes);
           fprintf(stderr, "   ---> sortie1 = %d\n", sortie1);
           fprintf(stderr, "   ---> fichie2 = 0x%x\n", sortie2);
	 }
  
  /* Mise en marche de la carte (et armement des ITs) :   */
  /* --> Carte supposee correctement demaree par initabi  */
  
  /* Ouverture du peripherique */
  if ((fasdc=open(device,O_RDWR)) < 0)
                         { perror("open");
                           fprintf(stderr, 
                                   "\n Impossible d'ouvrir '%s'\n",device);
                           exit(1);
                         }
                         
  /* Ouverture du fichier de sortie */
  if (fo) 
    { if (nes == 1)
           { fprintf(stderr, "E/S bas niveau non traitees pour le moment !\n");
             exit(-1);
           }
        else
           { sortie2 = fopen(fichier, "w+");
             if (sortie2 == NULL)
               { fprintf(stderr, "Impossible d'ouvrir \"%s\"\n", fichier);
                 exit(-1);
               }
           }
    }
        
  
  /* Affichage de la version du microcode */
  /******* ATTENTION : Num. version significatif seulement si *******/
  /*******             phase initialisation du microcode a    *******/
  /*******             deja ete executee !                    *******/
  fprintf(stderr, 
         "\nCarte ABIPMC2, firmware %04X/%04X\n", 
           LRAM(fasdc, FWPROC) & 0xFFFF, LRAM(fasdc, FWVERC) & 0xFFFF);




   /* Validation de l'espionnage sequentiel pour toutes les adresses */
   /* et sous-adresses                                               */
   { struct asdcvoie v;
     int b, a, s, d;
   
     for (a=0; a<32; a++)        /* Adresse */
         for (s=0; s<32; s++)     /* Sous-adresse */
             for (d=0; d<2; d++)   /* Direction du transfert */
                  { v.adresse = a;
                    v.sous_adresse = s;
                    v.direction = d;
                    if(ioctl(fasdc, ASDCVALESPS, &v))
                       { perror("ioctl(ASDCVALESPS)");
                         fprintf(stderr, "   Bus = %d", b);
                         fprintf(stderr, "   RT = %d,%d", a, s);
                         fprintf(stderr, "   Sens = %d\n", d);
                         exit(-1);
                       }
                  }
   }


   /* Allocation memoire de la zone tampon                */
   /* (taille suffisante pour plus grand tampon possible) */
   zone = (MOT *) calloc(32768, sizeof(MOT));
   if (zone == NULL)
      { fprintf(stderr, "Impossible d'allouer memoire\n");
        exit(-1);
      }
                         
  /* Ecriture du "bloc identification" du fichier de sortie */
  if (nes == 1)
           { fprintf(stderr, "E/S bas niveau non traitees pour le moment !\n");
             exit(-1);
           }
        else
	   { /* zone[0] = abiv3 ? MAGIQUE_V3 : MAGIQUE; */
	     zone[0] = MAGIQUE_IRIG;
	     zone[1] = 6;		/* Nombre de mots */
             zone[2] = (VERSION_MAJEUR << 8) + VERSION_MINEUR;
             /* zone[3] = (a->r.numver << 8) + a->r.numodi; */
             zone[3] = 0x0;		/* Pas de modif. EADS du ucode */
             zone[4] = LRAM(fasdc, FWPROC) & 0xFFFF;
             zone[5] = LRAM(fasdc, FWVERC) & 0xFFFF;
             zone[6] = 0x1012;		/* Carte PMC2-IRIG */
             /* zone[7] = 1;		   Periode H sans objet pour IRIG */
 
             fwrite (zone, (zone[1] + 1) * sizeof(MOT), 1, sortie2);
             fflush(sortie2);
           }
                         
  /* Ecriture du "bloc debut" du fichier de sortie */
  if (nes == 1)
           { fprintf(stderr, "E/S bas niveau non traitees pour le moment !\n");
             exit(-1);
           }
        else
           { struct tm *t;
             time_t horloge;
  
             horloge = time(0);
             t = localtime(&horloge);
            
             zone[0] = DEBUT;
             zone[1] = 8;
             zone[2] = 1900 + t->tm_year;		/* Annee */
             zone[3] = t->tm_mon;			/* Mois */
             zone[4] = t->tm_mday;			/* Jour du mois */
             zone[5] = t->tm_wday;			/* Jour de la semaine */
             zone[6] = t->tm_hour;			/* Heure */
             zone[7] = t->tm_min;			/* Minute */
             zone[8] = t->tm_sec;			/* Seconde */

             fwrite (zone, 9 * sizeof(MOT), 1, sortie2);
             fflush(sortie2);
           }
           
   /* Il est souhaitable que le disque ait termine son travail de la veille */
   sync();
        
   /* Lien de la fonction exec_ctrlc() au signal SIGINT */
   signal(SIGINT, exec_ctrlc);



   /* Initialisation de l'espionnage */
   if (r = ioctl(fasdc, ASDCESPS, 0))
     { perror("ioctl(ASDCESPS) ");
       exit(-1);
     }
   if (v) fprintf(stderr, "ioctl(ASDCESPS) = 0x%x\n", r);
   fflush(stdout);



   /* Espionnage */
   while (!terminer)
      {
        /* Attente de la disponibilite d'un tampon */
        /* Utilisation boucle car SIGINT (Ctrl-C) force la fin de l'attente */
        /* sur le semaphore noyau avant l'arrivee de l'IT ...               */
        for (;;)
          { 
            fprintf(stderr, " -1- "); fflush(stderr);
            r = ioctl(fasdc, ASDCESPSIT, 0);
            if (r) 
              { if (errno != EINTR)
                  { perror("ioctl(ASDCESPSIT) ");
                    exit(-1);
                  }
                else 
                  { fprintf(stderr, " -2- "); fflush(stderr);
                    continue;
                  }
              }
            else break;
          }
        fprintf(stderr, " -3- "); fflush(stderr);
        vidange = 1;
        /*************************************************/
        /* cf commentaire dans fonction exec_ctrlc() sur */
        /* utilisation de la variable vidange            */
        /*************************************************/
        

        /* Lecture de l'adresse du tampon moniteur sequentiel plein */
	atampon = LRAM(fasdc, M2PTR);

        if (v) fprintf(stderr, 
                       "ioctl(ASDCESPSIT) = 0x%X       atampon=0x%04X\n", 
                       r, atampon);
                                         
        if (r) /* Alors : debordement du tampon ! */
            { /* Ecriture d'un drapeau "debordement" dans la zone de travail */
              *(zone + 1) = 2;
              *(zone + 2) = 0xFFFF;
              n = 2;
              
              /* Ajout de l'emplacement du numero de tampon */
              n = 3;
              *(zone + 3) = 0;
            }
          else
            { /* Lecture de la premiere adresse et du nombre de mots */
              n = LRAM(fasdc, atampon);
              p2 = zone + 1;
     
/* Pour debug */              
printf("SMBCNT = 0x%04X\n", LRAM(fasdc, SMBCNT) & 0xFFFF);              
printf("Compteur bloc : Numtamp = 0x%04X\n", 
                               LRAM(fasdc, (atampon + n)) & 0xFFFF);

              /* Lecture du numero du tampon */
              n++;
                            
              /* Lecture du tampon courant de l'ABI dans la zone de travail */
              {
                 struct sbs1553_ioctl ioctl_arguments;
                 
                ioctl_arguments.buffer = p2;
                ioctl_arguments.length = n;
                ioctl_arguments.offset = atampon;
            
                if (ioctl(fasdc, ABI_LIRE_BLOC, (char *)&ioctl_arguments) == -1)
                  {
   	             printf("ABI_LIRE_BLOC call failed...");
   	             exit(-1);
                  }
              }
            }
            
        /* Mise en place de l'identificateur de bloc */
        zone[0] = ESPION;
        
/* Pour debug */              
printf("Compteur bloc : zone[n] = 0x%04X\n", zone[n] & 0xFFFF); fflush(stdout);
        
        /* Ecriture de la zone de travail sur le disque */
        if (nes == 1)
           { fprintf(stderr, "E/S bas niveau non traitees pour le moment !\n");
             exit(-1);
           }
        else
           { fwrite (zone, (n+1) * sizeof(MOT), 1, sortie2);
             fflush(sortie2);
             sync();
           }
                       
        fflush(stdout);
        vidange = 0;
      }





                         
  /* Ecriture du "bloc fin" du fichier de sortie */
  if (nes == 1)
           { fprintf(stderr, "E/S bas niveau non traitees pour le moment !\n");
             exit(-1);
           }
        else
           { struct tm *t;
             time_t horloge;
  
             horloge = time(0);
             t = localtime(&horloge);
            
             zone[0] = FIN;
             zone[1] = 8;
             zone[2] = 1900 + t->tm_year;		/* Annee */
             zone[3] = t->tm_mon;			/* Mois */
             zone[4] = t->tm_mday;			/* Jour du mois */
             zone[5] = t->tm_wday;			/* Jour de la semaine */
             zone[6] = t->tm_hour;			/* Heure */
             zone[7] = t->tm_min;			/* Minute */
             zone[8] = t->tm_sec;			/* Seconde */

             fwrite (zone, 9 * sizeof(MOT), 1, sortie2);
             fflush(sortie2);
           }
  
  
   /* Fin normale du programme ! */
   exit(0);






erreur_syntaxe :
   fprintf(stderr, "\n%s : Syntaxe anormale !\n", NOM);
   
syntaxe :
   fprintf(stderr, "\n%s - Version %d.%d du %s\n\n", 
                   NOM, VERSION_MAJEUR, VERSION_MINEUR, DATE);
   fprintf(stderr, 
	      "   Syntaxe : %s [-d device] [-bhsvVC] [fichier]",
                  argv[0]);
   fprintf(stderr, "\n\n");
   
}
   
