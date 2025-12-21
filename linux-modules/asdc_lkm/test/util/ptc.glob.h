/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*       Definition des types, constantes et variables globales       */
/*                                                                    */
/*                                            Y.G., le 4/9/1992       */
/**********************************************************************/

/* Definition de "noms compliques" pour les noms externes            */
/* (pour eviter les collisions avec les programmes de l'utilisateur) */
#define tbuf			CMBA_SUNUTIL1_termcap_tbuf
#define termcap_deja_initialise	CMBA_SUNUTIL1_termcap_deja_initialise
#define term_impose		CMBA_SUNUTIL1_term_impose
#define left			CMBA_SUNUTIL1_termcap_left
#define right			CMBA_SUNUTIL1_termcap_right
#define up			CMBA_SUNUTIL1_termcap_up
#define down			CMBA_SUNUTIL1_termcap_down
#define home			CMBA_SUNUTIL1_termcap_home
#define cm			CMBA_SUNUTIL1_termcap_cm
#define is			CMBA_SUNUTIL1_termcap_is
#define ti			CMBA_SUNUTIL1_termcap_ti
#define te			CMBA_SUNUTIL1_termcap_te
#define cl			CMBA_SUNUTIL1_termcap_cl
#define mb			CMBA_SUNUTIL1_termcap_mb
#define md			CMBA_SUNUTIL1_termcap_md
#define me			CMBA_SUNUTIL1_termcap_me
#define mr			CMBA_SUNUTIL1_termcap_mr
#define vi			CMBA_SUNUTIL1_termcap_vi
#define ve			CMBA_SUNUTIL1_termcap_ve
#define vs			CMBA_SUNUTIL1_termcap_vs
#define co			CMBA_SUNUTIL1_termcap_co
#define li			CMBA_SUNUTIL1_termcap_li
#define cb			CMBA_SUNUTIL1_termcap_cb
#define ce			CMBA_SUNUTIL1_termcap_ce
#define bl			CMBA_SUNUTIL1_termcap_bl
#define term			CMBA_SUNUTIL1_termcap_term
#define fich			CMBA_SUNUTIL1_termcap_fich
#define init_termcap		CMBA_SUNUTIL1_termcap_init_termcap
	


/* Structure d'une chaine de caracteres PASCAL */
struct ch80 { int  t;
              char c[80];
            };

/* Tampon pour memoriser les chaines de controle */
char tbuf[512];	

/* Stockage nom terminal si stdout non utilisee */
char term_impose[81];

/* Pointeurs des chaines C */
char *left, *right, *up, *down, *home, *cm;	/* Deplacement curseur  */
char *is, *ti, *te, *cl;			/* Initialisation ecran */
char *mb, *md, *me, *mr;			/* Attributs caracteres */
char *vi, *ve, *vs;				/* Visibilite curseur   */
char *cb, *ce;					/* Effacement ligne     */
char *bl;					/* Sonnette             */
char *term;					/* Nom du terminal      */

/* Valeurs numeriques */
int co, li;					/* Dimensions ecran     */


/* Variables globales avec initialisation : */
/* ---------------------------------------- */
#ifdef INIT_TERMCAP
int termcap_deja_initialise = 0;	/* Indicateur initialisation */
FILE *fich = stdout;	/* Fichier de controle ecran (stdout par defaut) */
#else
extern int termcap_deja_initialise;	/* Indicateur initialisation */
extern FILE *fich;			/* Fichier de controle ecran */
#endif

/* Mots clefs (arguments des procedures PASCAL) */
enum {HOME, DROITE, GAUCHE, HAUT, BAS, 			/* mouvements curseur */
      CUR_ALLUME, CUR_ETEINT,				/* visibilite curseur */
      INIT, EFFACE_ECR, FIN,				/* commandes ecran    */
      CLIGNOTE, INVERSE, SURINTENSITE, NORMAL,		/* attributs caract.  */
      EFFACE_DLI, EFFACE_FLI,				/* effacement ligne   */
      POUET,						/* sonnette           */
      GOTO,						/* deplacement curs.  */
      TERMINAL						/* nom du terminal    */
     };

/* Prototypes des fonctions C utilisees */
extern char *tgetstr(), *tgoto(), *getenv();
extern char *calloc();



