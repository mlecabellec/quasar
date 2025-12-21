/**********************************************************************/
/*                                                                    */
/*                Fonctions "termcap" pour PASCAL                     */
/*                -------------------------------                     */
/*                                                                    */
/*                          INITIALISATION                            */
/*                                                                    */
/*                                            Y.G., le 4/9/1992       */
/**********************************************************************/



#include <stdio.h>

#define INIT_TERMCAP
#include "ptc.glob.h"





/*****************************************************/
/* Extraction des donnees du fichier /etc/printcap : */
/*                                                   */
/*   - Si on utilise la sortie standard, le nom du   */
/*     terminal est extrait de l'environnement et    */
/*     la fonction doit etre appelee avec le param 1 */
/*   - Dans le cas contraire, le nom du terminal     */
/*     doit avoir ete place dans la chaine           */
/*     terminal_impose->c et le param 0 doit etre    */
/*     utilise                                       */
/*****************************************************/
init_termcap(autom)
int autom;	/* 1 si nom terminal extrait de l'environnement    */
                /* 0 si nom terminal est dans variable term_impose */
{
	register char *nom_term;
	register char *tptr;
	char *tbufptr, *nul;
	int rslt;

        /* Controle initialisation */
        termcap_deja_initialise = 1;
        
        /* Allocation memoire du tampon termcap */
	tptr = calloc(1024, sizeof(char));
	tbufptr = tbuf;
	
	/* Creation d'une entree nulle dans tbuf[] pour les */
	/* capacites non definies dans termcap              */
	nul = tbufptr;
	*nul = '\0';
	tbufptr++;

        /* Determination du type de terminal en fonction indicateur autom */
        if (autom)  { if(!(nom_term = getenv("TERM")))
		       { fprintf(stderr, 
		           "\007Variable d'environnement TERM non definie !\n");
		         exit(-1);
		       }  
		    }
               else { nom_term = term_impose;
                    }
		
        /* Lecture du tampon termcap */
	rslt = tgetent(tptr,nom_term);
	if(rslt < 0)
		{ fprintf(stderr, "\007Impossible ouvrir fichier termcap !\n");
		  exit(-1);
		}  
	if(rslt == 0)
		{ fprintf(stderr, "\007Terminal %s inconnu !\n", nom_term);
		  exit(-1);
		}  
		
	/* Stockage du nom du terminal dans le buffer */
	strcpy(tbufptr, nom_term);
	term = tbufptr;
	tbufptr += strlen(nom_term) + 1;
		
		
		
	/* Decodage des capacites qui nous interessent */
	
        if(!(left = tgetstr("le",&tbufptr))) left = nul;
	if(!(right = tgetstr("ri",&tbufptr))) right = nul;
	if(!(up = tgetstr("up",&tbufptr))) up = nul;
	if(!(down = tgetstr("do",&tbufptr))) down = nul;
	if(!(home = tgetstr("ho",&tbufptr))) home = nul;
	if(!(cm = tgetstr("cm",&tbufptr))) cm = nul;
	
	if(!(is = tgetstr("is",&tbufptr))) is = nul;
	if(!(ti = tgetstr("ti",&tbufptr))) ti = nul;
	if(!(te = tgetstr("te",&tbufptr))) te = nul;	
	if(!(cl = tgetstr("cl",&tbufptr))) cl = nul;
	
	if(!(mb = tgetstr("mb",&tbufptr))) mb = nul;
	if(!(md = tgetstr("md",&tbufptr))) md = nul;
	if(!(me = tgetstr("me",&tbufptr))) me = nul;
	if(!(mr = tgetstr("mr",&tbufptr))) mr = nul;
	
	if (!(vi = tgetstr("vi", &tbufptr))) vi = nul;
	if (!(ve = tgetstr("ve", &tbufptr))) ve = nul;
	if (!(vs = tgetstr("vs", &tbufptr))) vs = nul;
	
	if (!(cb = tgetstr("cb", &tbufptr))) cb = nul;
	if (!(ce = tgetstr("ce", &tbufptr))) ce = nul;
	
	if (!(bl = tgetstr("bl", &tbufptr))) 
	          { bl = tbufptr;		/* Si la capacite bl n'est */
	            *(tbufptr++) = '\007';	/* pas definie, on impose  */
	            *(tbufptr++) = '\0';	/* '^G' (soit '\007')      */
	          }  
	
	co = tgetnum("co");
	li = tgetnum("li");
	
	
	/* Liberation du tampon termcap */
	free(tptr);
}
