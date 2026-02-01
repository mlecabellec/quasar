/*******************************************************************/
/*   MONABI : Logiciel d'espionnage 1553 utilisant une carte ABI   */
/*                                                                 */
/*      Affichage "en clair" du fichier brut fourni par MONABI     */
/*      ------------------------------------------------------     */
/*                                                                 */
/*                                   Y. Guillemot, le 16/12/1993   */
/*                                         modifie le  3/01/1994   */
/*                                         modifie le  6/01/1994   */
/*                                         modifie le  7/01/1994   */
/*                                         modifie le 13/01/1994   */
/*                                         modifie le 29/03/1994   */
/*                                         modifie le 31/03/1994   */
/*                                                                 */
/*                V2.1 pour carte ABI-V3           le 20/06/1994   */
/*                V2.2                             le  4/08/1994   */
/*                V2.3 davantage d'affichages ...  le 12/09/1994   */
/*                V2.4 correction bug si T<>32us   le  7/03/1995   */
/*                V2.5 Ajout options diverses      le 20/05/1997   */
/*                V2.6 Augmentation vitesse        le 21/05/1997   */
/*******************************************************************/


/* Pour l'instant, fonctionnement en "filtre" */


#define NOM	"EXAM"
#define VERSION_MAJEUR	3
#define VERSION_MINEUR	0
#define DATE    "3/8/2001"

#define MODULE_PRINCIPAL


#include <stdio.h>
#include <signal.h>
#include "monabi.h"
#include "exam.h"




/* Pour permettre d'utiliser exam en "tuyau" avec monabi, on interdit     */
/* l'interruption par le premier Ctrl-C : il doit etre transmis a monabi. */
/* Par contre, on laisse a l'operateur la possibilite d'interrompre une   */
/* longue conversion en frappant 2 fois Ctrl-C.                           */
/*   --> Cette fonction est utilisee si l'option -p est utilisee.         */
void exec_ctrlc() 
   { static int une_fois_deja = 0;
   
     if (une_fois_deja) exit(1);
                   else une_fois_deja = 1;
   }



fin_de_fichier_inattendue(fin)
int fin;
{ if (fin) { fprintf(stderr, "Fin de fichier inattendue !\n");
             exit(-1);
           }
}

int lec_mot(fin)
int fin;
{ short int x;
  if (!fread(&x, sizeof(x), 1, stdin)) fin_de_fichier_inattendue(fin);
  octet_courant += sizeof(x);
  return x & 0xFFFF;
}

int lec_2mots(fin)
int fin;
{ int x;
  if (!fread(&x, sizeof(x), 1, stdin)) fin_de_fichier_inattendue(fin);
  octet_courant += sizeof(x);
  return x;
}




main(argc, argv)
int argc;
char **argv;
{  
   int m, i, j, id; 
   char c;
   extern char *optarg;
   extern int optind;
   int tirig;
   
   
   printf("\n%s - Version %d.%d du %s\n\n", 
                   NOM, VERSION_MAJEUR, VERSION_MINEUR, DATE);




   /*
   ** Decodage des options eventuelles
   */
   
   affderok = fok = ctrlc_2 = infos = 0;
   visutamp = vnumtamp = 1;
   
   while ((c = getopt(argc, argv, "ENSepv")) != -1)
      switch(c)
         { case 'e' : affderok = 1;
           case 'E' : fok = 1;
                      break;
           case 'N' : vnumtamp = 0;
                      break;
           case 'S' : visutamp = 0;
                      break;
           case 'p' : ctrlc_2 = 1;
                      break;
           case 'v' : infos = 1;
                      break;
           case '?' : goto syntaxe;
         }

   if (optind != argc) goto syntaxe;
   
   
   if (infos)
     { printf("\n");
       if (fok) printf("- Filtrage des messages OK\n");
       if (affderok) printf("- Impression du dernier message correct\n");
       if (ctrlc_2) printf("- Interception de SIGINT\n");
       if (visutamp) printf("- Affichage des infos sur tampons ABI\n");
       printf("\n");
     }



   
   /* Lien eventuel de la fonction exec_ctrlc() au signal SIGINT */
   if (ctrlc_2) signal(SIGINT, exec_ctrlc);
   
   
   
   /* Allocation des tampons de sortie */
   allocation();


   
   /* Initialisation des compteurs */
   octet_courant = 0;
   nb_msg = 0;
   spurious = 0;
   nb_err = 0;
   nb_msg1 = spurious1 = nb_err1 = 0;	/* Sur bus 1 */
   nb_msg2 = spurious2 = nb_err2 = 0;	/* Sur bus 2 */
   
   /* Periode de datation par defaut */
   T_datation = 32;	
   
   /* Premier numero de tampon attendu (si ABI-V3) */
   numero_tampon = 1;
   
   
   /* Lecture du "bloc identification" */
   i = lec_mot(1);
   if (i != MAGIQUE_IRIG) 
           { printf("IDENTIFICATEUR DE FICHIER ANORMAL : 0x%04x\n", i);
             exit(-1);
           }
           
   j = lec_mot(1);
   if (j < 6)  
      { printf("TAILLE ANORMALE DU BLOC_IDENTIFICATION : %d\n", j); 
        exit(-1);
      }
     else
      { m = lec_mot(1);
        printf("Logiciel MONABI V%d.%d\n", (m >> 8) & 0xFF, m & 0xFF);
        m = lec_mot(1);
        printf("Carte ABI : Microcode V%d.%d\n", (m >> 8) & 0xFF, m & 0xFF);
        if (j>=4)
          { m = lec_mot(1);
            printf("Carte ABI : Microcode SBS numero  0x%04X\n", m & 0xFFFF);
          }
        if (j>=5)
          { m = lec_mot(1);
            printf("Carte ABI : Microcode SBS version 0x%04X\n", m & 0xFFFF);
          }
        if (j>=6)
          { m = lec_mot(1);
            tirig = m & BIT_IRIG;
            printf("Carte ABI Version 0x%04X%s\n", 
            m & 0xFFFF & ~BIT_IRIG, tirig ? " - IRIG" : "");
          }
        for (i=6; i<j; i++) lec_mot(1);
      } 
   
   
   for (;;)
     { id = lec_mot(0);
     
       if (feof(stdin))
           { printf("---   C'est fini !   ---\n");	/*** FIN NORMALE ! ***/
             printf("\n");
             printf("                             \tBus 1\tBus 2\tTotal\n");
             printf("Nombre de messages :         \t%d\t%d\t%d\n",
                      nb_msg1, nb_msg2, nb_msg);
             printf("Nombre de \"spurious data\" :\t%d\t%d\t%d\n", 
                     spurious1, spurious2, spurious);
             printf("Nombre d'erreurs :           \t%d\t%d\t%d\n",
                      nb_err1, nb_err2, nb_err);
                      
             /* Recopie eventuelle sur sortie erreur : permet d'afficher */
             /* les statistiques meme si sortie standard est redirigee   */
             /* dans un fichier.                                         */
             if (infos)
               { fprintf(stderr,"\n");
                 fprintf(stderr,
                     "                             \tBus 1\tBus 2\tTotal\n");
                 fprintf(stderr, 
                     "Nombre de messages :         \t%d\t%d\t%d\n",
                      nb_msg1, nb_msg2, nb_msg);
                 fprintf(stderr, 
                     "Nombre de \"spurious data\" :\t%d\t%d\t%d\n", 
                     spurious1, spurious2, spurious);
                 fprintf(stderr, 
                     "Nombre d'erreurs :           \t%d\t%d\t%d\n",
                      nb_err1, nb_err2, nb_err);
               }
                      
             exit(0);
           }
           
        switch(id)  
           { case DEBUT :    printf("DEBUT ESPIONNAGE :\n");
                             examheure();
                             break;
                             
             case FIN :      printf("FIN ESPIONNAGE :\n");
                             examheure();
                             break;
                             
             case ESPION :   examespionirig();
                             break;
                             
             case OPTION :	/* Blocs non pris en compte pour le moment */
             case IRIG :	/* puisque non produits par les versions   */
                                /* actuelles de monabi                     */
                             
             default :       printf("BLOC #0x%04x inconnu (octet # 0x%lx) !\n",
                                    id, octet_courant);
                             j = lec_mot(1);
                             printf("    taille = %d mots\n", j);
                             for (i=1; i<j; i++) lec_mot(1);
           } 
     }        
     
   /* On ne devrait jamais sortir de cette boucle par la fin ! */
   abort();
   
   
syntaxe :
   fprintf(stderr,
           "Syntaxe d'appel : \n");
   fprintf(stderr, 
           " %s   [options]   < source   > destination\n",
           argv[0]);
   fprintf(stderr, "\n");
   fprintf(stderr, "source : Fichier d'espionnage ABI-V3 a convertir\n");
   fprintf(stderr, "destination : Fichier texte obtenu\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Options  -E : Seules les erreurs sont affichees\n");
   fprintf(stderr, "         -e : Idem -E + impression dernier message OK\n");
   fprintf(stderr, "         -p : Inhibition arret sur Ctrl-C unique\n");
   fprintf(stderr, "         -v : Validation ecriture infos internes\n");
   fprintf(stderr, "         -S : Inhibition ecriture infos sur tampons ABI\n");
   fprintf(stderr, "         -N : Inhibition verif. numeros des tampons ABI\n");

   exit(1);
}   
