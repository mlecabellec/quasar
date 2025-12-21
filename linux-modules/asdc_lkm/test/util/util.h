/*  modif. le 26 octobre 1990 */
/*  modif. le 4 avril 1991 */
/*  modif. le 10 avril 1991 */
/* derniere modif. le 15 avril 1991 */

#ifndef FILE
#include <stdio.h>
#endif

/**************************************************************/
/*   Fonctions generales   :   Definition des noms externes   */
/**************************************************************/

#define index            YG_UTIL1_index
#define trim_gauche      YG_UTIL1_trim_gauche
#define trim_droite      YG_UTIL1_trim_droite
#define trim             YG_UTIL1_trim
#define bina             YG_UTIL1_bina
#define hexa             YG_UTIL1_hexa
#define initanachai	 YG_UTIL1_initanachai
#define anachai		 YG_UTIL1_anachai
#define encadre		 YG_UTIL1_encadre
#define decadre		 YG_UTIL1_decadre
#define date		 YG_UTIL1_date
#define heure		 YG_UTIL1_heure
#define nfsuivant	 YG_UTIL1_nfsuivant
#define indice   	 YG_UTIL1_indice
#define iabrev   	 YG_UTIL1_iabrev
#define conversionl	 YG_UTIL1_conversionl
#define conversiond	 YG_UTIL1_conversiond
#define inicarsuiv	 YG_UTIL1_inicarsuiv
#define carsuiv		 YG_UTIL1_carsuiv
#define carsuiv_ligne	 YG_UTIL1_carsuiv_ligne
#define carsuiv_tamp	 YG_UTIL1_carsuiv_tamp
#define lecchaine	 YG_UTIL1_lecchaine
#define utilver		 YG_UTIL1_utilver

/*    Non portes sur VAX et SUN
#define cwrite           YG_UTIL1_cwrite
#define cw_ecrire        YG_UTIL1_cw_ecrire
#define cread            YG_UTIL1_cread
#define cherche          YG_UTIL1_cherche
*/



/********************************************/
/*   Fonctions generales   :   Prototypes   */
/********************************************/

#ifdef C_ANSI
   extern int index(char *, char *);
   extern void trim_gauche(char *);
   extern void trim_droite(char *);
   extern void trim(char *);
   extern char *bina(long int, int, char *);
   extern char *hexa(long int, int, char *);
   extern void initanachai(char *, char *, char *, int, int);
   extern int anachai(char *, char *);
   extern char *encadre(char *, char *, int, char);
   extern char *decadre(char *, char *, char);
   extern char *date(int);
   extern char *heure();
   extern char *nfsuivant(char *, char *, char *, int);
   extern int indice(char *, char **);
   extern int iabrev(char *, char **);
   extern int conversionl(char *, long int *);
   extern int conversiond(char *, double *);
   extern void inicarsuiv(FILE *);
   extern char carsuiv();
   extern int lecchaine(char *, int, char *);
   
/*    Non portes sur VAX et SUN
   extern int cherche(char *, int, char *);
*/
   
#else
   extern int index();
   extern void trim_gauche();
   extern void trim_droite();
   extern void trim();
   extern char *bina();
   extern char *hexa();
   extern void initanachai();
   extern int anachai();
   extern char *encadre();
   extern char *decadre();
   extern char *date();
   extern char *heure();
   extern char *nfsuivant();
   extern int indice();
   extern int iabrev();
   extern int conversionl();
   extern int conversiond();
   extern void inicarsuiv();
   extern char carsuiv();
   extern int lecchaine();
   
/*    Non portes sur VAX et SUN   
   extern int cherche();
*/
   
#endif



/**************************/
/*   Variables externes   */
/**************************/

extern int carsuiv_ligne;
extern char *carsuiv_tamp;

