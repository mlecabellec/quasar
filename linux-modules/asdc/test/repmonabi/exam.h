/*******************************************************************/
/*   EXAM : Logiciel d'espionnage 1553 utilisant une carte ABI     */
/*          (Affichage "en clair" des donnees espionnees)          */
/*                                                                 */
/*                  Definition des variables globales :            */
/*                  -----------------------------------            */
/*                                                                 */
/*                       Y. Guillemot (V2.5), le     20 mai 1997   */
/*                             modif. (V2.6), le     21 mai 1997   */
/*******************************************************************/

#ifdef MODULE_PRINCIPAL
#define GLOBAL
#else
#define GLOBAL extern
#endif

/* Dimensionnement des tampons de sortie */
#define T_MAX_LIGNE	132
#define T_MAX_TMP_S	10000

GLOBAL int affderok;	/* Valide affichage dernier message OK (option -e) */
GLOBAL int fok;		/* Valide filtrage des messages OK (option -E et -e) */
GLOBAL int ctrlc_2;	/* Inhibe arret sur Ctrl-C unique (option -p) */
GLOBAL int infos;	/* Valide affichage informations internes */
GLOBAL int visutamp;	/* Inhibe affichage infos sur tampons */
GLOBAL int vnumtamp;	/* Inhibe la verification des numeros de tampons ABI */


/* Compteurs divers ... */
GLOBAL long int octet_courant, nb_msg, spurious, nb_err;
GLOBAL long int nb_msg1, spurious1, nb_err1;	/* Sur bus 1 */
GLOBAL long int nb_msg2, spurious2, nb_err2;	/* Sur bus 2 */

GLOBAL int T_datation;		/* Periode datation en us */


GLOBAL char *tmp;	/* Pointeur dans tampon de sortie courant a remplir */
GLOBAL int *pnctmp;	/* Pointe taille courante du tampon courant */

GLOBAL int numero_tampon;	/* Pour controle succession des tampons ABI */



/* Macros pour remplir le tampon de sortie courant */

#define MAJTMP		lt = strlen(temporaire);	\
			strcpy(tmp, temporaire);	\
			tmp += lt;			\
			*pnctmp += lt;

#define AJTMP(cte)	lt = sizeof(cte) - 1;		\
			strcpy(tmp, cte);		\
			tmp += lt;			\
			*pnctmp += lt;
					
