/*
   Mise de l'horloge systeme a une date du XXI siecle
   
   Ce logiciel supplee aux carences de l'utilitaire date (1) qui 
   peut lire des dates du XXI siecle, mais ne peut remettre l'horloge
   a la date qu'avant l'an 2000.
   
   ===> Il faut etre superuser ou pouvoir le devenir !
   
                                Y. Guillemot, le    11 juin 1997
                               Remis en forme le 26 fevrier 1998
                                  et complete le    17 mars 1998
                                  
                                Modif. LynxOS le     30 mai 2002
   
*/


/*

   		- Prise en compte correcte du "Day Saving Time Flag" :
   		
   		  Actuellement, solution de contournement :
   		     - Mise a la date/heure souhaitee
   		     - Si heure correcte : OK, mise a l'heure achevee !
   		     - Sinon (erreur d'une heure), effectuer une nouvelle
   		       mise a la date/heure
   		     - L'heure doit maintenant etre correcte
   		  
   		  
   ===> L'annee maximale utilisable avec SUNOS_4.1 est 2037
        (la limite ultime est entre le 15 et le 20 janvier 2038)
   		
*/


/*
** Definir symbole(s) ci-dessous en fonction de la nature du systeme pour 
** specifier les utilisateurs autorises a changer la date et l'heure.     
**                                                       
**     - Si CMBA est defini : root (0) et adm (500) sont autorises          
**     - Si UMINI et UMAXI sont definis, seuls les utilisateurs compris
**       entre UMINI et UMAXI sont autorises
**     - Si SANS_RESTRICTION est defini tous les utilisateurs sont autorises
**     - Si aucun symbole n'est defini, seul root (0) est autorise
**
** Pour definir ces symboles, plutot que de modifier le source ci-dessous,
** il est plus pratique d'appeler le compilateur cc avec l'option -D ou
** d'appeler la makefile presente dans le repertoire courant en specifiant
** sur la ligne de commande   UTIL="-D ... "   (c.f. commentaires dans la
** makefile).
*/
/* #define CMBA 	    */
/* #define UMINI	500 */
/* #define UMAXI	600 */
/* #define SANS_RESTRICTION */


#define NOM		"DATE21"
#define VERSION		"1"
#define REVISION	"2"
#define DATE		"30/5/2002"


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

extern long entrer_e();
int jours_par_annee();
int jours_par_mois();


main(argc, argv)
int argc;
char **argv;
{


   int jour, mois, annee, heure, minute;
   int def_jour, def_mois, def_annee, def_heure, def_minute;
   
   int i, j;
   long int horloge, horloge1;
   
   time_t tloc;
   struct tm *tloc_tm;
   
   struct timeval tp;
   struct timezone tzp;
   
   struct tm *tcour;
   time_t horloge_cour;
   
   char *utilisateurs;
   char memo_util[80];
   
   
/*
** Elaboration d'une chaine de caracteres pour afficher les numeros des
** utilisateurs autorises a modifier l'heure et la date du systeme
*/
   utilisateurs = "{???}";
#ifndef SANS_RESTRICTION
#ifdef CMBA
   utilisateurs = "{0,500}";
#else CMBA
#ifdef UMINI
#ifdef UMAXI
   sprintf(memo_util, "{0,%d-%d}", UMINI, UMAXI);
   utilisateurs = memo_util;
#endif UMAXI
#else UMINI
   utilisateurs = "{0}";
#endif UMINI
#endif CMBA
#else SANS_RESTRICTION      
   utilisateurs = "{Tous}";
#endif SANS_RESTRICTION      
   
   
   
/*
** Page de publicite
*/
   fprintf(stdout, "\n%s - v%s.%s du %s - %s\n\n", 
                    NOM, VERSION, REVISION, DATE, utilisateurs);



/*
** Controle de l'utilisateur et suid eventuel en superuser 
*/

#ifndef SANS_RESTRICTION
#ifdef CMBA
   if ((getuid() != 500) && (getuid() != 0))
#else CMBA
#ifdef UMINI
#ifdef UMAXI
   if (((getuid() < UMINI) || (getuid() > UMAXI)) && (getuid() != 0))
#endif UMAXI
#else UMINI
   if (getuid() != 0)
#endif UMINI
#endif CMBA
      { fprintf(stderr, 
           "Seul l'administrateur du systeme peut utiliser cette commande !\n");
        exit(1);
      }
#endif SANS_RESTRICTION      

   if (setuid(0)) { fprintf(stderr, "Impossible de devenir superuser !\n");
                    exit(2);
                  }
            
                  
           
/*
** Lecture heure et date courantes pour initialiser les valeurs par defaut
*/
   horloge_cour = time(0);
   tcour = localtime(&horloge_cour);
   
   def_jour = tcour->tm_mday;
   def_mois = tcour->tm_mon + 1;
   def_annee = tcour->tm_year + 1900;
   def_heure = tcour->tm_hour;
   def_minute = tcour->tm_min;                
                  
           
           
                  

/*
** Entree des parametres
*/
   annee = entrer_e("Annee ? %s= ", "[%d] ", 1970, 2038, def_annee);
   mois = entrer_e("Mois ? %s= ", "[%d] ", 1, 12, def_mois);
   jour = entrer_e("Jour ? %s= ", "[%d] ", 
                            1, jours_par_mois(def_mois, def_annee), def_jour);
   heure = entrer_e("Heure ? %s= ", "[%d] ", 0, 23, def_heure);
   minute = entrer_e("Minute ? %s= ", "[%d] ", 0, 59, def_minute);



/*
** Affichage heure et date courante et heure et date souhaitees
*/
   fprintf(stdout, "\nDate initiale   : %02d:%02d %d/%d/%d\n",
                    def_heure, def_minute, def_jour, def_mois, def_annee);
   fprintf(stdout, "Date souhaitee  : %02d:%02d %d/%d/%d\n",
                    heure, minute, jour, mois, annee);



/*
** Calcul du nombre de jours depuis le 01/01/1970 
*/
  
   /* Initialisation du compteur */
   j = 0;
   
   /* Calcul sur le nombre d'annees pleines */
   for (i=1970; i<annee; i++) j += jours_par_annee(i);
   
   /* Calcul du nombre de jours dans les mois passes */
   switch (mois)
     { case 12 : j += 30;					 /* Novembre */
       case 11 : j += 31;					 /* Octobre */
       case 10 : j += 30;					 /* Septembre */
       case  9 : j += 31;					 /* Aout */
       case  8 : j += 31;					 /* Juillet */
       case  7 : j += 30;					 /* Juin */
       case  6 : j += 31;					 /* Mai */
       case  5 : j += 30;					 /* Avril */
       case  4 : j += 31;					 /* Mars */
       case  3 : j += (jours_par_annee(annee) == 366) ? 29 : 28; /* Fevrier */
       case  2 : j += 31;					 /* Janvier */
       case  1 : ;
     }

   /* Calcul du nombre de jours passes dans le mois courant */
   j += jour-1;
   
   
   
   
/*
** Calcul du nombre de secondes depuis le 01/01/1970 a 0h
*/
   
   /* Conversion du nombre de jours en secondes */
   horloge = j * 24 * 3600;

   /* Ajout du nombre de secondes ecoulees dans le jour courant */
   horloge += heure * 3600;
   horloge += minute * 60;
   
   /* A-t-on depasse la date maximale de ce systeme ? */
   if (horloge < 0)
     { fprintf(stderr, 
               "L'horloge de ce systeme ne peut atteindre cette date !\n");
       exit(-1);
     }


 
/*
** Mise a l'heure
*/  

   /* Lecture de l'heure courante pour en deduire le decalage */
   /* entre heure locale et heure GMT                         */
   tloc = time(NULL);
   tloc_tm = localtime(&tloc);
   
#ifndef LYNXOS
   /* Correction de la variable horloge par le decalage GMT */
   horloge1 = horloge - tloc_tm->tm_gmtoff;
#else
   horloge1 = horloge;
#endif
   
   /* Lecture heure courante pour initilisation convenable des structures */
   if (gettimeofday(&tp, &tzp))
     { perror("gettimeofday ");
       exit(-1);
     }
   
   /* Modification de la structure timeval en fonction de la nouvelle date */
   tp.tv_sec = horloge1;
   tp.tv_usec = 0;

   /* Mise a l'heure effective */
   if (settimeofday(&tp, &tzp))
     { perror("settimeofday ");
       exit(-1);
     }
     
     
     
/*
** Relecture heure et date courantes pour controle
*/
   horloge_cour = time(NULL);
   tcour = localtime(&horloge_cour);
   
   def_jour = tcour->tm_mday;
   def_mois = tcour->tm_mon + 1;
   def_annee = tcour->tm_year + 1900;
   def_heure = tcour->tm_hour;
   def_minute = tcour->tm_min;  
   
   
   
   
/*
** Comparaison date et heure courantes a date et heure attendues
*/              
   if (    (jour == def_jour)     
        && (mois == def_mois)
        && (annee == def_annee)
        && (minute == def_minute))
        	{  /* Tout s'est bien passe ! */
                   fprintf(stdout, 
                           "Date finale     : %02d:%02d %d/%d/%d\n",
                           def_heure, def_minute, 
                           def_jour, def_mois, def_annee);
                   exit(0);
               	}
                                       
                                       
                                       
/*
** Tout ne s'est pas bien passe : c'est sans doute un probleme
** lie a l'heure d'ete/hiver ("Day Saving Time Flag"), une nouvelle
** operation de mise a l'heure devrait, en principe, corriger le defaut.
*/

   fprintf(stdout, "Date provisoire : %02d:%02d %d/%d/%d\n",
                    def_heure, def_minute, def_jour, def_mois, def_annee);

   /* Lecture de l'heure courante pour en deduire le decalage */
   /* entre heure locale et heure GMT                         */
   tloc = time(NULL);
   tloc_tm = localtime(&tloc);
   
#ifndef LYNXOS
   /* Correction de la variable horloge par le decalage GMT */
   horloge1 = horloge - tloc_tm->tm_gmtoff;
#else
   horloge1 = horloge;
#endif
   
   /* Lecture heure courante pour initilisation convenable des structures */
   if (gettimeofday(&tp, &tzp))
     { perror("gettimeofday ");
       exit(-1);
     }
     
   /* Modification de la structure timeval en fonction de la nouvelle date */
   tp.tv_sec = horloge1;
   tp.tv_usec = 0;

   /* Mise a l'heure effective */
   if (settimeofday(&tp, &tzp))
     { perror("settimeofday ");
       exit(-1);
     }
     
     
     
     
     
     
     
     
/*
** Relecture heure et date courantes pour controle
*/
   horloge_cour = time(NULL);
   tcour = localtime(&horloge_cour);
   
   def_jour = tcour->tm_mday;
   def_mois = tcour->tm_mon + 1;
   def_annee = tcour->tm_year + 1900;
   def_heure = tcour->tm_hour;
   def_minute = tcour->tm_min;  

   fprintf(stdout, "Date finale     : %02d:%02d %d/%d/%d\n",
                    def_heure, def_minute, def_jour, def_mois, def_annee);
     

}



/* Determination du nombre de jours dans une annee donnee */
int jours_par_annee(an)
int an;
{
   if (!(an % 400))
       { /* Annee quadriseculaire ==> bissextile */
         return 366;
       }
   else if (!(an % 100))
       { /* Annee seculaire ==> non bissextile */
         return 365;
       }
   else if (!(an % 4))
       { /* Annee bissextile */
         return 366;
       }
   else
       { /* Annee ordinaire */
         return 365;
       }
}



/* Determination du nombre de jours dans un mois d'une annee donnee */
int jours_par_mois(mois, an)
int mois;
int an;
{
   switch (mois)
     { case 1 : return 31;
       case 2 : return (jours_par_annee(an) == 366) ? 29 : 28;
       case 3 : return 31;
       case 4 : return 30;
       case 5 : return 31;
       case 6 : return 30;
       case 7 : return 31;
       case 8 : return 31;
       case 9 : return 30;
       case 10 : return 31;
       case 11 : return 30;
       case 12 : return 31;
       default : fprintf(stderr, "Erreur interne : Numero mois = %d\n\n", mois);
                 abort();
     }
}
