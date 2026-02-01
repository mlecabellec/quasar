   /****************************************************************/
   /*          COMPILATION D'UNE DESCRIPTION DE VOIES RTs A        */
   /*                SIMULER PAR UNE "CARTE" AMI/ABI               */
   /*                MUNIE DU MICROCODE MODIFIE SDC                */
   /*                                                              */
   /*                         Y. Guillemot, le     13 avril 1991   */
   /*                                modif. le        7 mai 1991   */
   /*                                modif. le       12 mai 1991   */
   /*                       derniere modif. le      21 juin 1991   */
   /*                        adaptation SDC le      31 mars 1992   */
   /*                                modif. le       15 mai 1992   */
   /*                                modif. le       27 mai 1992   */
   /*                                modif. le    7 juillet 1992   */
   /*                                modif. le      17 mars 1994   */
   /*                       derniere modif. le     14 avril 1994   */
   /*                                                              */
   /*    Adaptation a LynxOS et aux ABIPMC2 le       2 aout 2001   */
   /*   Adaptation ASDC4.1 (abo synchrones) le        6 mai 2002   */
   /*   Adaptation ASDC4.2 (abo synchrones) le       6 juin 2002   */
   /*    Adaptation ASDC4.5 (abo statiques) le 26 septembre 2002   */
   /*       Correction bug sur nbre tampons le   7 novembre 2002   */
   /*             Mise en conformite gcc v4 le   15 fevrier 2008   */
   /****************************************************************/



/*********************************************************/
/*  MODIFS ENVISAGEABLES :                               */
/*     fcontinu est inutile : a remplacer par nbcycles=0 */
/*     "rewind" fichiers possible ?                      */
/*     fichier temp. pour seq. vraiment necessaire ?     */
/*********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "util/util.h"
#include "asdcctl.h"

#define VERSION	"3.2"
#define DATE	"6 juin 2002"

/* Taille des tampons */
#define TATAMP 80
#define TATAMP1 250

/* Nom du fichier temporaire */
#ifdef __TURBOC__
static char ftempo[] = "cpl.tmp";
#else
static char ftempo[] = "vxisim_cpl.sequence.tmp";
#endif


static void lecparvoie();
static void lecparvalinh();
static void lecpartrep();
static int lecdonnees();
static void lecliste();
static void lectemps();

static void erreur();
static void erreur1();
static void panic();

/* Liste des commandes */
char *liste_com[] = { "RAZ" ,       	/* 0 : Remise a zero      	   */
		      "VOIE",       	/* 1 : Description d'une voie      */
		      "VALIDATION",	/* 2 : Validation de RTs	   */
		      "INHIBITION",	/* 3 : Inhibition de RTs	   */
		      "TEMPS_REPONSE",	/* 4 : Programmation temps reponse */
		      ""                /* 5 : Fin de la liste 		   */
		    };

/* Liste des parametres de description d'une voie */
char *liste_ptr[] = { "ADRESSE" ,        /* 0 : Adresse                    */
		      "BUS",	       	 /* 1 : Numero du bus              */
		      "SOUS_ADRESSE" ,   /* 2 : Sous-adresse               */
		      "SA",		 /* 3 : Synonyme pour sous_adresse */
		      "SENS", 		 /* 4 : Emission ou Reception      */
		      "NOMBRE" ,         /* 5 : Nombre de mots             */
		      "IDENTIFICATEUR" , /* 6 : Identificateur tampon      */
                      "TAMPONS",	 /* 7 : Nombre de tampons          */ 
		      "DONNEES" ,        /* 8 : Liste des donnees          */
		      ""                 /* 9 : Fin de la liste            */
		    };

/* Liste des mots utilises pour definir la direction d'un transfert */
char *liste_sens[] = { "RECEPTION" ,      /* 0 */
		       "TRANSMISSION",	  /* 1 */  
		       "EMISSION",        /* 2 */
		       ""                 /* 3 : Fin de la liste */
		     };

/* Liste des mots utilises pour definir une liste de RTs */
char *liste_liste[] = { "BUS" ,      /* 0 */
		        "RTs",	     /* 1 */  
			""           /* 2 : Fin de la liste */
		      };



/* Variables de description d'un RT */
static int adresse;		/* 0 a 31 */
static int bus;			/* 1 ou 2 */	
static int sous_adresse;	/* 0 a 31 */
static int nombre;		/* nombre de mots impose pour un echange */
static int sens;		/* 0 ==> Reception,   1 ==> Transmission */
#define TAIDENT 81
static char nom[TAIDENT];  	/* nom du buffer lie a la voie */
#define NTM 32	/* Nombre maxi de tampons (actuellement...) */
static int donnees[NTM][32];	/* zone de stockage des donnees */
static int nbtampons;		/* Nombre de tampons de donnees */
static int nbtcour;		/* Nombre tampons remplis courant -1         */
                                /* (pointe prochain tampon dans donnees[][]) */
static int nbdcour;             /* Nbr donnees courant ds tampon courant */
static char liste_rt[32];       /* Liste RT selectionnes (inh./val./Trep.) */
static double temps[32];        /* Liste Temps de reponse des RTs */

char * cioctl;			/* Nom commande ioctl pour message erreur */
 

/*===========================================================================*/


main(argc, argv)
int argc;
char **argv;
{       

FILE *entree;   /* Fichier source a traiter */
int fasdc;	/* Identificateur du fichier d'acces a la carte AMI */

char tampon[TATAMP];
char tampon1[TATAMP1];
int i, j;
int r;
char c;

struct asdcvoie v;  	/* Pour passer description voies au driver ASDC */
struct asdctampon t;	/* Pour passer donnees au driver ASDC */

   fprintf(stderr,"CPLASDC - V%s du %s\n", VERSION, DATE);

   /* Saisie nom du fichier source sur ligne de commande */   
   if(argc!=3)
	    { fprintf(stderr, "Syntaxe : %s periph. fichier_source\n", argv[0]);
	      exit(-1);
	    }
	       
	       
	       
   
   /* Ouverture du fichier source */
   if((entree=fopen(argv[2], "r")) == NULL)
               { fprintf(stderr, "Impossible d'ouvrir le fichier \"%s\"\n",
                                 argv[2]);
                 exit(-1);
               }
   inicarsuiv(entree);  /* Initialisation des fonctions de lecture */
   
   
    
   /* Ouverture du perpherique (ou "fichier destination") */
   if ((fasdc = open(argv[1], O_RDWR)) < 0) { perror("cplasdc - open asdc");
                                              exit(-1);
                                            }
   
   
   /* ----------------------------------- */
   /* ------- Debut du traitement ------- */
   /* ----------------------------------- */
   

   /* Partie initiale du fichier de commande (en sortie)  */
   
   /* Arret de la carte */	// Supprime le 26/9/02 (pour multitache)
   // cioctl = "ASDCSTOP";
   // ioctl(fasdc, ASDCSTOP, 0);   
 
   
   /*--------------------------*/
   /* --- Une seule passe  --- */
   /*--------------------------*/
   
  
   
   /* Lecture commande */
   r = leccom();
   while(r>=0)
       {
       switch(r)
         { case 0 : /* Remise a zero de la carte AMI/ABI */
                    /*** ATTENTION : Incompatible avec un usage multitache ***/
                    cioctl = "ASDCRAZ";
 		    ioctl(fasdc, ASDCRAZ, 0);   
		    break;
		    
	   case 1 : /* Description d'une voie */
		    /* assignation des parametres */
                    // v.bus = bus-1;
		    v.adresse = adresse;
		    v.sous_adresse = sous_adresse;
		    v.direction = sens;
		    v.nmots = nombre;
		    v.ntamp = nbtampons;
		    v.adrtamp = 0;    /* Au driver de faire l'allocation ... */
		    v.mode = RT_VSTAT;	/* Mode dedie a la simu statique */
		    /* appel ioctl() */
		    cioctl = "ASDCDEF";
		    if (ioctl(fasdc, ASDCDEF, &v)) goto erreur_ioctl;
		    /* Ecriture des eventuelles donnees dans les tampons */
		    if (sens)   /* Transmission seulement */
		       { for(i=0; 
		             i<=(nbtcour > nbtampons ? nbtampons : nbtcour);
		             i++)
		           { /* assignation des donnees */
		             // t.v.bus = bus-1;
		             t.v.adresse = adresse;
		             t.v.sous_adresse = sous_adresse;
		             t.f = (i==0) ? SDC_RAZ : SDC_NONBLOQ;
		             t.nbr = 32;   /* On transfert un tampon complet */
		             for (j=0; j<32; j++) t.t[j] = donnees[i][j];
		             /* appel ioctl(ECRITURE_TAMPON) */
		             cioctl = "ASDCECR";
		             if (ioctl(fasdc, ASDCECR, &t)) goto erreur_ioctl;
		           }
		       }
		       
		    /* Toute abonne dont une voie est definie est valide par */
		    /* defaut (compatibilite avec les versions anterieures)  */
		    v.adresse = adresse;
		    v.nmots = RT_ABONNE;
		    cioctl = "ASDCMODE";
		    if (ioctl(fasdc, ASDCMODE, &v)) goto erreur_ioctl;
		    
		    break;
		    
	   case 2 : /* Validation d'une liste de RTs */
                    /* assignation du parametre bus */
                    // v.bus = bus-1;
		    for (i=0; i<32; i++)
		       { if (liste_rt[i])
			    {  /* assignation i a adresse RT */
			       v.adresse = i;
			       v.nmots = RT_ABONNE;
			       /* appel ioctl(VALIDATION) */
		               cioctl = "ASDCMODE";
		               if (ioctl(fasdc, ASDCMODE, &v))
		                                          goto erreur_ioctl;
			    }
		       }
		    break;   
		    
	   case 3 : /* Inhibition d'une liste de RTs */
                    /* assignation du parametre bus */
                    // v.bus = bus-1;
		    for (i=0; i<32; i++)
		       { if (liste_rt[i])
			    {  /* assignation i a adresse RT */
			       v.adresse = i;
			       v.nmots = RT_INHIBE;
			       cioctl = "ASDCMODE";
		               if (ioctl(fasdc, ASDCMODE, &v))
		                                          goto erreur_ioctl;
			    }
		       }
		    break;   
		    
	   case 4 : /* Programmation temps de reponse d'une liste de RTs */
                    /* assignation du parametre bus */
                    // v.bus = bus-1;
		    for (i=0; i<32; i++)
		       { if (liste_rt[i])
			    {  /* assignation i a adresse RT */
			       // v.bus = bus-1;
			       v.adresse = i;
			       /* assignation temps_reponse[i] a Temps rep. */
			       /*   ---> T.Rep. mini supposee etre 10,2 us  */
#define T_REP_MINI 10.2	/* Temps de reponse minimum possible               */
#define PAS_TREP   0.25 /* Pas utilisable pour programmer temps de reponse */
                               if (temps[i] <= T_REP_MINI)
                                     v.nmots = 0;
                                else v.nmots = (temps[i]-T_REP_MINI)/PAS_TREP;
			       /* appel ioctl(TEMPS_REPONSE) */
			    // if (ioctl(fasdc, ASDCTR, &v)) goto erreur_ioctl;
			    }
		       }
		    break; 
		      
	 }     /* Fin du switch(r) */
          r = leccom();
       }




   /*----------------------------------------------------------*/
   /*--- Ici, ecriture des commandes globales du simulateur ---*/
   /*--- puis terminaison du fichier en sortie              ---*/
   /*----------------------------------------------------------*/
   
   /* Remise en marche du micro-code de la carte */
   //cioctl = "ASDCGO";	// Supprime le 26/9/02 (pour multitache)
   //ioctl(fasdc, ASDCGO, 0);   
   
   /* Fermeture du fichier d'acces a la carte */
   close(fasdc);

   exit(0);
   
   
   /* Traitement des erreurs drivers (ioctl a retourne une valeur non nulle) */
erreur_ioctl :
   perror("cplasdc - ioctl ");
   fprintf(stderr, "cmd ioctl = \"%s\"\n", cioctl);
   // fprintf(stderr, "v.bus =\t\t\t%d\n", v.bus);
   fprintf(stderr, "v.adresse =\t\t%d\n", v.adresse);
   fprintf(stderr, "v.sous_adresse =\t%d\n", v.sous_adresse);
   fprintf(stderr, "v.direction =\t%d\n", v.direction);
   fprintf(stderr, "v.nmots =\t\t\t%d\n", v.nmots);
   fprintf(stderr, "v.ntamp =\t\t\t%d\n", v.ntamp);
   exit(-1);
}



/*===========================================================================*/


   /* Lecture d'une commande  - renvoie -1 si EOF atteint,             */
   /*                                    le numero de la commade sinon */
int leccom()
{
   int r, com, i;
   char tampon[TATAMP];
   char c;

   r = lecchaine(tampon, TATAMP, "{");

   if (r==0) return -1;   /* -1 ==> Fin du fichier ! */

   if (r==-1) erreur("Delimiteur '{' absent", "", "");

   trim(tampon);
   com = iabrev(tampon, liste_com);
   if (com == -1) erreur("Commande \"", tampon, "\" inconnue");
   if (com == -2) erreur("Commande \"", tampon, "\" ambigue");
   switch(com)
      { case 0 :  /* RAZ : Pas de parametres */
		  do { c = carsuiv(); } while(c != '}');
                  break;
	case 1 :  /* Definition d'une voie : */
		  /* Initialisation des valeurs "par defaut" */
		  adresse = -1;  /* Comprendre : "non specifie" */
		  bus = 1;  /* Comprendre : "specifie" (car plus utilise) */
		  sous_adresse = -1;  /* Comprendre : "non specifie" */
		  nombre = -1;   /* Comprendre : "non specifie" */
		  sens = -1;  /* Comprendre : "non specifie" */
		  nom[0] = '\0';
		  nbtampons = 1;
		  nbtcour = -1;    /* Pas de tampon rempli */
		  
                  /* Lecture parametres de voie */
		  lecparvoie(); 

                  /* Tous les parametres necessaires sont-ils connus ? */
                  if(adresse==-1) erreur("Adresse non specifiee", "", "");  
                  if(bus==-1) erreur("Bus non specifiee", "", "");  
		  if(sous_adresse==-1) erreur("Sous_adresse non specifiee", "", ""); 
                  if(sens==-1) erreur("Sens du tranfert non specifie", "", "");  
                  if(nombre==-1) erreur("Nombre de mots non specifie", "", "");  
		  break;
                  
	case 2 :  /* Validation de RTs */

		  /* Valeurs initiales */
		  bus = 1;   /* Comprendre : Specifie (car plus utilise) */
		  for (i=0; i<32; i++) liste_rt[i] = 0;

		  lecliste();     /* Lecture liste de RTs a valider */
		  
                  /* Tous les parametres necessaires sont-ils connus ? */
		  if(bus==-1) erreur("Bus non specifie", "", "");
		  break;
	case 3 :  /* Inhibition de RTs */

		  /* Valeurs initiales */
		  bus = 1;   /* Comprendre : Specifie (car plus utilise) */
		  for (i=0; i<32; i++) liste_rt[i] = 0;

		  lecliste();     /* Lecture liste de RTs a valider */
		  
                  /* Tous les parametres necessaires sont-ils connus ? */
		  if(bus==-1) erreur("Bus non specifie", "", "");
		  break;
	case 4 :  /* Programmation des temps de reponse RTs */

		  /* Valeurs initiales */
		  bus = 1;   /* Comprendre : Specifie (car plus utilise) */
		  for (i=0; i<32; i++) liste_rt[i] = 0;

	          lectemps();     /* Lecture liste de Temps de reponse RTs */
		  break;
	case 5 :  erreur("Commande absente", "", "");
        default : panic("Numero de commande imprevue !");
      }

   return com;    /* com>=0 ==> Commande comprehensible trouvee */
}


/*---------------------------------------------------------------------------*/


   /* Lecture d'un parametre de description d'une voie */
   /* - fonction recursive jusqu'a epuisement des parametres */
static void lecparvoie()
{
   int r, parm, i;
   char tampon[TATAMP];

   r = lecchaine(tampon, TATAMP, "=;}");

   if (r==0) erreur("Fin inattendue du fichier !", "", "");

   if (r==-1) erreur("Delimiteur '}' absent", "", "");

   if (r!='=') erreur("Syntaxe erronee (", tampon, ")");

   trim(tampon);
   parm = iabrev(tampon, liste_ptr);
   if (parm == -1) erreur("Parametre de transfert \"", tampon, "\" inconnu");
   if (parm == -2) erreur("Parametre de transfert \"", tampon, "\" ambigu");
   switch(parm)
      { case 0 :  /* Definition adresse RT */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if(conversionl(tampon, &adresse)!=0)
                    erreur("Syntaxe anormale de l'adresse (", tampon, ")");
                  if ((adresse>31) || (adresse<0))
                     erreur("Adresse anormale (", tampon, ")");
                  break;
        case 1 :  /* Definition du bus */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if(conversionl(tampon, &bus)!=0)
                    erreur("Syntaxe anormale du bus (", tampon, ")");
                  if ((bus != 1) && (bus != 2))
                     erreur("Bus inexistant (", tampon, ")");
		  break;
	case 2 :
        case 3 :  /* Definition de la sous_adresse */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if(conversionl(tampon, &sous_adresse)!=0)
                    erreur("Syntaxe anormale sous_adresse (", tampon, ")");
                  if ((sous_adresse>31) || (sous_adresse<0))
		     erreur("Sous_adresse anormale (", tampon, ")");
		  break;
        case 4 :  /* Definition du sens de la transmission */
                  r = lecchaine(tampon, TATAMP, ";}");
                  trim(tampon);
		  i = iabrev(tampon, liste_sens);
		  switch(i) 
		     { case 0 :  /* Reception */
			     	 sens = 0;  
				 break;  
		       case 1 :
		       case 2 :  /* Emission */
				 sens = 1;
				 break; 
		       default : erreur("Syntaxe anormale sens (", tampon, ")");
                     }
		  break;
        case 5 :  /* Definition du nombre de mots */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if(conversionl(tampon, &nombre)!=0)
                    erreur("Syntaxe anormale du nombre de mots (", tampon, ")");
                  if ((nombre>32) || (nombre<1))
                     erreur("Nombre de mots anormal (", tampon, ")");
                  break;
        case 6 :  /* Definition de l'identificateur du tampon */
                  r = lecchaine(nom, TAIDENT, ";}");
                  if (r==-1) erreur("Identificateur trop long", "", "");
                  break;
        case 7 :  /* Definition du nombre de tampons */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if (conversionl(tampon, &nbtampons)!=0)
                    erreur("Syntaxe anormale nombre tampons (", tampon, ")");
		  if ((nbtampons>NTM) || (nbtampons<1))
                    erreur("Nombre de tampons interdit (", tampon, ")");
                  break;
        case 8 :  /* Definition des donnees */
		  nbtcour++;     /* Incrementation tampon courant */
                  nbdcour = -1;
		  r = lecdonnees();
                  /* Mise a zero partie tampon non remplie */
		  for (i=nbdcour+1; i<32; i++) donnees[nbtcour][i] = 0;
		  break;
	default : panic("Numero de parametre de transfert imprevu !");
      }

   if (r==0) erreur("Fin inattendue du fichier", "", "");

   if (r!='}') lecparvoie();     /* Lecture des parametres suivants */
}


/*---------------------------------------------------------------------------*/


   /* Lecture d'une liste de RTs (d'adresses) a valider ou a inhiber */
   /* - fonction recursive jusqu'a epuisement des parametres */
static void lecliste()
{
   int r, parm, i;
   char tampon[TATAMP];

   r = lecchaine(tampon, TATAMP, "=;}");

   if (r==0) erreur("Fin inattendue du fichier !", "", "");

   if (r==-1) erreur("Delimiteur '}' absent", "", "");

   if (r!='=') erreur("Syntaxe erronee (", tampon, ")");

   trim(tampon);
   parm = iabrev(tampon, liste_liste);
   if (parm == -1) erreur("Parametre \"", tampon, "\" inconnu");
   if (parm == -2) erreur("Parametre \"", tampon, "\" ambigu");
   switch(parm)
      { case 0 :  /* Definition du bus */
                  r = lecchaine(tampon, TATAMP, ";}");
                  if(conversionl(tampon, &bus)!=0)
                    erreur("Syntaxe anormale du bus (", tampon, ")");
                  if ((bus != 1) && (bus != 2))
                     erreur("Bus inexistant (", tampon, ")");
		  break;
	case 1 :  /* Definition des adresses RTs */
		  nbtcour=0;     /* Imposition tampon courant */
                  nbdcour = -1;
		  r = lecdonnees();
		  
		  /* Transfert des resultats sous forme d'une liste */
		  for (i=0; i<=nbdcour; i++) liste_rt[donnees[0][i]] = 1;
		  break;
	default : panic("Numero de parametre de transfert imprevu !");
      }

   if (r==0) erreur("Fin inattendue du fichier", "", "");

   if (r!='}') lecliste();     /* Lecture des parametres suivants */
}



/*---------------------------------------------------------------------------*/


   /* Lecture d'une liste de Temps de reponse associes a des adresses RT */
   /* - fonction recursive jusqu'a epuisement des parametres             */
   /*                                                                    */
   /*    Syntaxe attendue :                                              */
   /*       { BUS=1;   T(1)=23;  T(7)=12; . . .   }                      */
   /*                                                                    */
static void lectemps()
{
   int r, parm;
   char tampon[TATAMP];
   long l;
   int i;

   r = lecchaine(tampon, TATAMP, "=;}");

   if (r==0) erreur("Fin inattendue du fichier !", "", "");

   if (r==-1) erreur("Delimiteur '}' absent", "", "");

   if (r!='=') erreur("Syntaxe erronee (", tampon, ")");

   trim(tampon);

   if (    (!strcmp(tampon, "B"))
	|| (!strcmp(tampon, "BU"))
	|| (!strcmp(tampon, "BUS")) 
      )   {   /* Definition du bus */
	      r = lecchaine(tampon, TATAMP, ";}");
	      if(conversionl(tampon, &bus)!=0)
		    erreur("Syntaxe anormale du bus (", tampon, ")");
	      if ((bus != 1) && (bus != 2))
		     erreur("Bus inexistant (", tampon, ")");
	  }
   else { if (tampon[0]=='T')
	     { trim(&tampon[1]);
	       if ((tampon[1]!='(') || (tampon[strlen(tampon)-1]!=')'))
		    erreur("Syntaxe anormale : \"", tampon, "\"");
	       tampon[strlen(tampon)-1] = '\0';
               if(conversionl(&tampon[2], &l)!=0)
		    erreur("Syntaxe anormale d'une adresse RT", "", "");
	       i = l;
	       liste_rt[i] = 1;
	       r = lecchaine(tampon, TATAMP, ";}");
	       if(conversiond(tampon, &temps[i])!=0)
		    erreur("Syntaxe anormale du temps (", tampon, ")");
          /*   if ((temps[i]<...) || (temps[i]>...))
		    erreur("Valeur de temps de reponse interdite (", tampon, ")");
          */
	     }
	  else 
	    erreur("parametre incomprehensible ou inattendu : \"", tampon, "\"");
        }

   if (r==0) erreur("Fin inattendue du fichier", "", "");

   if (r!='}') lectemps();     /* Lecture des parametres suivants */
}



/*---------------------------------------------------------------------------*/



/* Fonction de lecture des mots de donnees (recursive) */
/* Renvoie le separateur qui a acheve la suite des donnees */
#define Ttmp 100
static int lecdonnees()
{
  static char tmp[Ttmp];
  static int r;		/* "static" tres important : sinon return errone */
  long int l;
  
  r = lecchaine(tmp, Ttmp, ",;}");
  if(conversionl(tmp, &l) == -1)
	erreur("Syntaxe anormale d'une donnee (", tmp, ")");

  nbdcour ++;
  if (nbdcour>31) erreur("Trop de donnees pour un meme tampon", "", ""); 

  donnees[nbtcour][nbdcour] = l & 0xFFFF;
  if(r == ',') lecdonnees();
  return r;
}


/*---------------------------------------------------------------------------*/

/*   Impression d'un message d'erreur (3 champs disponibles) accompage */
/* du numero de ligne du fichier source ou l'erreur a ete  detectee    */
static void erreur(ch1, ch2, ch3)
char *ch1, *ch2, *ch3;
{ fprintf(stderr, "\007\n%s%s%s a la ligne %d\n",
                  ch1, ch2, ch3, carsuiv_ligne, carsuiv_tamp);
  exit(-1);
}

/*---------------------------------------------------------------------------*/

/*   Impression d'un message d'erreur lie a un mauvais fonctionnement */
/* du programme (bug)                                                 */
/*   Un tel message ne devrait jamais etre imprime !                  */
static void panic(msg)
char *msg;
{ fprintf(stderr, "\007\nPanique : %s\n", msg);
  exit(-1);
}


/*---------------------------------------------------------------------------*/

/*   Impression d'un message d'erreur (un seul champs) ne pouvant etre */
/* associe a une ligne particuliere du fichier source                  */
static void erreur1(ch1)
char *ch1;
{ fprintf(stderr, "\007\n%s\n", ch1);
  exit(-1);
}

/*---------------------------------------------------------------------------*/
